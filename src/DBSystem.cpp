/*
 * DBSystem.cpp
 *
 *  Created on: 25-Jan-2014
 *      Author: arpit
 */

#include "DBSystem.h"

#include "mem_model.hpp"
#include "parser.hpp"
#include "configurations.hpp"

#include <iostream>
#include <fstream>

using namespace std;

DBSystem::DBSystem() {
}

DBSystem::~DBSystem() {
}

bool DBSystem::canLineBeAdded(string tableName, int length) {
	return true;
}

void DBSystem::readConfig(string filePath) {
	ConfigParser *parser = ParserFactory::newParserInstance();
	parser->parse(filePath);

	//cout << "Page Size : " << Configurations::pageSize << endl;
	//cout << "Number of pages : " << Configurations::numberOfPages << endl;

}

string DBSystem::getRecord(string tableName, int recordId) {

	//cout << "GET : " << tableName << " and record : " << recordId << endl;
	
	MemoryModel *mm = MemoryModel::getInstance(Configurations::numberOfPages,
			Configurations::pageSize);

	TableBlockMapIterator tableBlockIterator =
			Configurations::tableBlocks.find(tableName);
	BlockLineNumberMap m = (BlockLineNumberMap) tableBlockIterator->second;
	BlockLineNumberMapIterator blockLineItr = m.begin();

	int previousPageNumber = blockLineItr->first;
	int blockNumberToGetIntoMemory = -1;

	if (recordId == blockLineItr->second) {
		blockNumberToGetIntoMemory = previousPageNumber;
	}

	for (blockLineItr++; blockNumberToGetIntoMemory == -1 && blockLineItr
			!= m.end(); blockLineItr++) {
		int currentPageNumber = blockLineItr->first;
		int currentLineNumber = blockLineItr->second;

		if (recordId == currentLineNumber) {
			blockNumberToGetIntoMemory = blockLineItr->first;
		} else if (recordId < currentLineNumber) {
			blockNumberToGetIntoMemory = previousPageNumber;
		} else {
			previousPageNumber = currentPageNumber;
		}
	}

	if (blockNumberToGetIntoMemory == -1) {
		blockNumberToGetIntoMemory = previousPageNumber;
	}

	TableBlockId * tableBlockId = new TableBlockId(tableName,
			blockNumberToGetIntoMemory);
	if (!mm->isPresent(tableBlockId)) {

		//if required block is not found in memory (LRU)
		int currentLineNumber = 0;
		string filePath = Configurations::pathForData + "/" + tableName
				+ ".csv";

		//cout << "Input to file : " << filePath << endl;
		ifstream dataFile(filePath.c_str());

		int blockStartLineNumber =
				tableBlockIterator->second[blockNumberToGetIntoMemory];

		//TODO: change the line number to offset of the file in the map
		string blockDataString;
		std::string line;

		//Skipping all the lines before the required block
		while (currentLineNumber != blockStartLineNumber) {
			getline(dataFile, line);
			currentLineNumber++;
		}

		int endLineNumber =
				tableBlockIterator->second[blockNumberToGetIntoMemory + 1];

		while (getline(dataFile, line) > 0 && currentLineNumber
				!= endLineNumber) {
			blockDataString.append(line + "\n");
			currentLineNumber++;
		}

		blockDataString.resize(blockDataString.length()-1);

		int result = mm->allocate(tableBlockId, blockDataString.c_str(),
				blockDataString.length());

		dataFile.close();

		switch (result) {
		case PAGE_SIZE_EXCEEDED:
			return "";
		case PAGE_NOT_REPLACED:
			//cout << "HIT " << tableBlockId->tableBlockNumber << endl;
			cout << "HIT" << endl;
			break;
		default:
			//cout << "MISS " << tableBlockId->tableBlockNumber << endl;
			cout << "MISS " << result << endl;
			break;
		}
	} else {
		//cout << "HIT " << tableBlockId->tableBlockNumber << endl;
		cout << "HIT" << endl;
		mm->touch(tableBlockId);
	}
	
	
	//mm->traverseList();
	
	return mm->getRecordForBlock(tableBlockId, recordId,
			tableBlockIterator->second[blockNumberToGetIntoMemory]);
}

void DBSystem::insertRecord(string tableName, string record) {

	//cout << "Insert : " << tableName << " and record : " << record << endl;
	MemoryModel *mm = MemoryModel::getInstance(Configurations::numberOfPages,
			Configurations::pageSize);

	string filePath = Configurations::pathForData + "/" + tableName + ".csv";

	//cout << "Ouput to file : " << filePath << endl;
	std::ofstream outfile;
	outfile.open(filePath.c_str() , std::ios_base::app);
	outfile << record << "\n";
	outfile.flush();

	TableBlockMapIterator tableBlockIterator =
			Configurations::tableBlocks.find(tableName);

	Configurations::tableBlocks.erase(tableBlockIterator);
	populateDBInfo(tableName);

	tableBlockIterator = Configurations::tableBlocks.find(tableName);
	int lastBlockNumber = Configurations::tableBlocks[tableName].size()-1;
	int blockStartLineNumber = tableBlockIterator->second[lastBlockNumber];

	//cout << "Last block of table is numbered : " << lastBlockNumber << endl;
	//cout << "Start line numb " << blockStartLineNumber << endl;

	TableBlockId * tableBlockId = new TableBlockId(tableName,
			lastBlockNumber);

	if (mm->isPresent(tableBlockId) ) {
		//cout << "Block already present in memory." << endl;
		string blockData = mm->getBlockData(tableBlockId);
		mm->allocate(tableBlockId, blockData.c_str() , blockData.length());
	} else {
		//cout << "Block not present in memory." << endl;
		// get the last block from file create the block of data and then allocate

		int currentLineNumber = 0;
		ifstream dataFile(filePath.c_str());

		int blockStartLineNumber = tableBlockIterator->second[lastBlockNumber];

		//TODO: change the line number to offset of the file in the map
		string blockDataString;
		std::string line;

		//Skipping all the lines before the required block
		while (currentLineNumber != blockStartLineNumber) {
			getline(dataFile, line);
			currentLineNumber++;
		}

		int endLineNumber = tableBlockIterator->second[lastBlockNumber + 1];

		while (getline(dataFile, line) > 0 && currentLineNumber
				!= endLineNumber) {
			blockDataString.append(line + "\n");
			currentLineNumber++;
		}

		blockDataString.resize(blockDataString.length()-1);

		//cout << "Block to be allocated " << blockDataString << endl;
		//cout << "Block to be allocated - length " << blockDataString.length() << endl;

		dataFile.close();

		//mm->touch(new TableBlockId(tableName , lastBlockNumber-1));
		mm->allocate(tableBlockId, blockDataString.c_str(),
				blockDataString.length());

	}
	
	//mm->traverseList();

}

void DBSystem::populateDBInfo(string tableName) {

	string filePath = Configurations::pathForData + "/" + tableName + ".csv";

	int count_of_bytes = 0;
	int current_block_number = 0;
	int current_line_number = 0;

	map<int, int> tableBlockLineNumberMap;

	ifstream file(filePath.c_str());
	string str;
	while (std::getline(file, str)) {

		int countNewLine = 1;
		int count = str.length();

		// If counting newline make record go to next page and removing it make it get appended the current page
		// then include the newline.
		// otherwise exclude it.
		if ((count_of_bytes + count + countNewLine) > Configurations::pageSize
				&& (count_of_bytes + count - countNewLine)
						<= Configurations::pageSize) {
			count = count;
		} else {
			count += countNewLine;
		}

		//int count = str.length() + 1;
		/*int count = str.length();*/

		if ((count_of_bytes + count) <= Configurations::pageSize) {
			if (tableBlockLineNumberMap.find(current_block_number)
					== tableBlockLineNumberMap.end()) {
				tableBlockLineNumberMap[current_block_number]
						= current_line_number;
			}

			count_of_bytes += count;

		} else {
			current_block_number++;
			count_of_bytes = count;
			tableBlockLineNumberMap[current_block_number] = current_line_number;
		}
		current_line_number++;
	}
	Configurations::tableBlocks[tableName] = tableBlockLineNumberMap;
}

void DBSystem::populateDBInfo() {

	for (TableMapIterator tableMapEntry = Configurations::tables.begin(); tableMapEntry
			!= Configurations::tables.end(); tableMapEntry++) {

		populateDBInfo((string) tableMapEntry->first);
	}

	for (TableBlockMapIterator iterator = Configurations::tableBlocks.begin() ; iterator
			!= Configurations::tableBlocks.end() ; iterator++) {
		cout << "Table Name : " << iterator->first << endl;
		map<int,int> m = iterator->second;
		for (map<int,int>::iterator itr = m.begin() ; itr != m.end() ; itr++) {
			cout << "Block number : " << itr->first << " line number : "
					<< itr->second << endl;
		}
	}
}

/*int main() {

	DBSystem dbSystem;
	dbSystem.readConfig(Configurations::CONFIG_FILE_PATH);

	dbSystem.populateDBInfo();

	dbSystem.getRecord("countries", 0);
	dbSystem.getRecord("countries", 1);
	dbSystem.getRecord("countries", 2);
	dbSystem.getRecord("countries", 1);
	dbSystem.getRecord("countries", 2);
	dbSystem.getRecord("countries", 2);
	dbSystem.getRecord("countries", 3);
	dbSystem.getRecord("countries", 9);
	dbSystem.getRecord("countries", 39);
	dbSystem.getRecord("countries", 28);
	dbSystem.getRecord("countries", 1);
	dbSystem.getRecord("countries", 30);
	dbSystem.getRecord("countries", 38);
	dbSystem.getRecord("countries", 39);
	dbSystem.getRecord("countries", 31);
	dbSystem.insertRecord("countries", "record");
	dbSystem.getRecord("countries", 42);
	dbSystem.getRecord("countries", 28);

	return 0;
}*/