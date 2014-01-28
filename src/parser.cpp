/*
 * config_parser.cpp
 *
 *  Created on: 25-Jan-2014
 *      Author: sriram
 */

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include "parser.hpp"
#include "configurations.hpp"

using namespace std;

void ConfigParser::parse(string configFileName) {
	ifstream fin(configFileName.c_str());

	string tag;
	while (fin >> tag) {
		if (tag.compare(Configurations::TAG_PAGESIZE) == 0) {
			fin >> Configurations::pageSize;

		} else if (tag.compare(Configurations::TAG_NUM_PAGES) == 0) {
			fin >> Configurations::numberOfPages;

		} else if (tag.compare(Configurations::TAG_PATH_FOR_DATA) == 0) {
			fin >> Configurations::pathForData;

		} else if (tag.compare(Configurations::TAG_BEGIN_TABLE) == 0) {
			string tableName;
			fin >> tableName;

			map<string, string> tableAttributes;
			while (1) {
				string line;
				fin >> line;

				if (line.compare(Configurations::TAG_END_TABLE) == 0)
					break;

				istringstream iss(line);
				string columnName, dataType;
				std::getline(iss, columnName, ',');
				std::getline(iss, dataType, ',');
				tableAttributes.insert(StringMapEntry(columnName, dataType));
			}

			Configurations::tables.insert(TableMapEntry(tableName,
					tableAttributes));
		}
	}
}

ConfigParser *ParserFactory::newParserInstance() {
	return (new ConfigParser());
}

//int main() {
//	ConfigParser *parser = ParserFactory::newParserInstance();
//	parser->parse(Configurations::CONFIG_FILE_PATH);
//	return 0;
//}
