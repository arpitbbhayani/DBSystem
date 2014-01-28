#ifndef _MEM_MODEL_H
#define _MEM_MODEL_H

#include <list>
#include <map>
#include <string>

#include "dbconstants.hpp"

#define PAGE_NOT_REPLACED -1
#define PAGE_SIZE_EXCEEDED -2

struct TableBlockId {
	std::string tableName; //data from this table
	long tableBlockNumber; //table file block number (unspanned)
	
	TableBlockId(std::string name, int number) {
		tableName = name;
		tableBlockNumber = number;
	}
};

class MemoryModel {
public:
	struct Page {
		int pageNumber; //page number

		TableBlockId *tableBlockId;

		char *data; //actual data
		int dataLength; //total occupied data size

		bool isModified;
	};

	typedef std::list<Page *> PageList;
	typedef std::map<std::string, Page *> PageMap;

private:
	int _capacity;
	int _currentSize;
	int _pageSize;

	PageList _pageList;
	PageList _freePageList;
	PageMap _tableToPagesMap;

	inline MemoryModel::Page * newPage(int, int);
	std::string tableBlockIdToKey(TableBlockId *);

	static MemoryModel *mm;
	MemoryModel(int, int);

public:
	static MemoryModel *getInstance(int, int);
	~MemoryModel();

	//Check if the given table file block is present in the memory
	inline bool isPresent(TableBlockId *);
	
	//Add page. Returns replaced page number if any otherwise -1
	int allocate(TableBlockId *, const char *, int);
	
	std::string getRecordForBlock(TableBlockId *, int, int);
	std::string getBlockData(TableBlockId *);
	
	//Touch the page number to move to front
	void touch(TableBlockId *);
	void traverseList();
};

#endif
