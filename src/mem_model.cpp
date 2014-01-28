#include <cstdio>
#include <iostream>
#include <cstring>
#include <sstream>

#include "mem_model.hpp"

using namespace std;

typedef map<string, MemoryModel::Page *>::iterator PageMapIterator;
typedef list<MemoryModel::Page *>::iterator PageListIterator;
typedef pair<string, MemoryModel::Page *> PageMapEntry;

MemoryModel *MemoryModel::mm= NULL;

MemoryModel * MemoryModel::getInstance(int capacity, int pageSize) {
	if (mm == NULL)
		mm = new MemoryModel(capacity, pageSize);

	return mm;
}

////////////////////// private methods ///////////////////////

inline MemoryModel::Page * MemoryModel::newPage(int pageNumber, int pageSize) {
	Page *p = new Page();
	p->pageNumber = pageNumber;
	p->tableBlockId = NULL;
	p->data = new char[pageSize + 1];
	p->dataLength = 0;
	p->isModified = false;
	return p;
}

string MemoryModel::tableBlockIdToKey(TableBlockId *tid) {
	int digitCount = 1;
	long blockNumber = tid->tableBlockNumber;
	while (blockNumber >= 10) {
		blockNumber /= 10;
		digitCount++;
	}

	string key;
	key.append(tid->tableName);
	key.append("_");
	char *number = new char[digitCount + 1];
	sprintf(number, "%ld", tid->tableBlockNumber);
	key.append(number);

	delete[] number;

	return key;
}

inline bool MemoryModel::isPresent(TableBlockId *tid) {
	return (_tableToPagesMap.find(tableBlockIdToKey(tid))
			!= _tableToPagesMap.end());
}

void MemoryModel::touch(TableBlockId *tid) {
	PageMapIterator it = _tableToPagesMap.find(tableBlockIdToKey(tid));

	if (it == _tableToPagesMap.end())
		return;

	//TODO: write the page to memory when touched.

	// TODO: See if this can be done efficiently
	_pageList.remove(it->second);
	_pageList.push_back(it->second);
}

////////////////////// public methods ///////////////////////

MemoryModel::MemoryModel(int capacity, int pageSize) :
	_capacity(capacity), _pageSize(pageSize) {

	int i;
	for (i = 0; i < _capacity; i++)
		_freePageList.push_back(newPage(i, _pageSize));
}

void deleteList(MemoryModel::PageList &list) {
	PageListIterator it = list.begin();
	while (it != list.end()) {
		MemoryModel::Page *p = *it;
		delete p->tableBlockId;
		delete[] p->data;
		delete p;
		it = list.erase(it);
	}
}

void MemoryModel::traverseList() {
	PageListIterator it = _pageList.begin() ;
	cout << "LRU : ";
	while (it != _pageList.end()) {
		MemoryModel::Page *p = *it;
		cout << p->tableBlockId->tableBlockNumber << " ";
		it++;
	}
	cout << endl;
}

MemoryModel::~MemoryModel() {
	if (!_freePageList.empty()) {
		deleteList(_freePageList);
	}

	if (!_pageList.empty()) {
		deleteList(_pageList);
	}

	_tableToPagesMap.clear();
}

int MemoryModel::allocate(TableBlockId *tid, const char *content,
		int contentLength) {
	if (contentLength > _pageSize) {
		cout << "File block length is more than allowed page size." << endl;
		return PAGE_SIZE_EXCEEDED;
	}

	//Checking if the page is already present in the allocated list.
	if (isPresent(tid)) {
		//TODO: change the data if it has been changed

		cout << "Found in block number : " << tid->tableBlockNumber << endl;
		touch(tid);
		return PAGE_NOT_REPLACED;
	}

	// Freelist is empty then allocate a page from that
	if (!_freePageList.empty()) {
		Page *p = _freePageList.front();
		_freePageList.pop_front();

		p->tableBlockId = tid;
		p->dataLength = contentLength;
		memcpy(p->data, content, contentLength);
		p->data[contentLength] = '\0';

		//New page mapping added
		//cout << "MISS " << p->pageNumber << endl;
		_tableToPagesMap.insert(PageMapEntry(tableBlockIdToKey(tid), p));
		_pageList.push_back(p);

		return p->pageNumber;
	}

	//Replacing a page cause freelist is empty
	Page *p = _pageList.front();
	//cout << "MISS " << p->pageNumber << endl;

	//Erase previous data
	_tableToPagesMap.erase(tableBlockIdToKey(p->tableBlockId));
	delete (p->tableBlockId);

	//Write new data to the page
	p->isModified = false;
	p->dataLength = contentLength;
	memcpy(p->data, content, contentLength);
	p->data[contentLength] = '\0';
	p->tableBlockId = tid;

	//Insert new mapping to map and move the page to the end of the list
	_tableToPagesMap.insert(PageMapEntry(tableBlockIdToKey(tid), p));
	touch(tid);

	return p->pageNumber;
}

string MemoryModel::getRecordForBlock(TableBlockId *tableBlock, int recordId,
		int firstBlockRecordId) {
	PageMapIterator pageIterator =
			_tableToPagesMap.find(tableBlockIdToKey(tableBlock));
	Page *pageToChange = pageIterator->second;
	string recordsString(pageToChange->data);

	istringstream iss(recordsString);
	string record;
	while(std::getline(iss, record, '\n')) {
		if(recordId == firstBlockRecordId)
			return record;
		firstBlockRecordId++;
	}
	
	return "";
}

string MemoryModel::getBlockData(TableBlockId *tableBlock) {
	PageMapIterator pageIterator =
			_tableToPagesMap.find(tableBlockIdToKey(tableBlock));
	Page *pageToChange = pageIterator->second;
	string recordsString(pageToChange->data);
	
	return recordsString;
}

/*int main() {
 MemoryModel *mm = MemoryModel::getInstance(3, 1024);

 mm->allocate(newBlock(7), (char *) "content", 7);
 mm->allocate(newBlock(0), (char *) "content", 7);
 mm->allocate(newBlock(1), (char *) "content", 7);
 mm->allocate(newBlock(2), (char *) "content", 7);
 mm->allocate(newBlock(0), (char *) "content", 7);
 mm->allocate(newBlock(3), (char *) "content", 7);
 mm->allocate(newBlock(0), (char *) "content", 7);
 mm->allocate(newBlock(4), (char *) "content", 7);
 mm->allocate(newBlock(2), (char *) "content", 7);
 mm->allocate(newBlock(3), (char *) "content", 7);
 mm->allocate(newBlock(0), (char *) "content", 7);
 mm->allocate(newBlock(3), (char *) "content", 7);
 mm->allocate(newBlock(2), (char *) "content", 7);
 mm->allocate(newBlock(1), (char *) "content", 7);
 mm->allocate(newBlock(2), (char *) "content", 7);
 mm->allocate(newBlock(0), (char *) "content", 7);
 mm->allocate(newBlock(1), (char *) "content", 7);
 mm->allocate(newBlock(7), (char *) "content", 7);
 mm->allocate(newBlock(0), (char *) "content", 7);
 mm->allocate(newBlock(1), (char *) "content", 7);

 delete mm;

 return 0;
 }*/
