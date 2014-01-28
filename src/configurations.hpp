/*
 * configurations.hpp
 *
 *  Created on: 25-Jan-2014
 *      Author: sriram
 */

#ifndef CONFIGURATIONS_HPP_
#define CONFIGURATIONS_HPP_

#include <map>
#include <list>
#include <vector>
#include "mem_model.hpp"

/*
 * TableName -> { AttributeName , Attribute DataType }
 */
typedef std::map<std::string, std::map<std::string, std::string> > TableMap;
typedef std::map<std::string, std::map<std::string, std::string> >::iterator TableMapIterator;
typedef std::pair<std::string, std::string> StringMapEntry;
typedef std::pair<std::string, std::map<std::string, std::string> > TableMapEntry;

typedef std::map<std::string, std::map< int,int > > TableBlockMap;
typedef std::map<std::string, std::map< int,int > >::iterator TableBlockMapIterator;

typedef std::map<int,int> BlockLineNumberMap;
typedef std::map<int,int>::iterator BlockLineNumberMapIterator;

class Configurations {
public:
	static const std::string CONFIG_FILE_PATH;

	static const std::string TAG_PAGESIZE;
	static const std::string TAG_NUM_PAGES;
	static const std::string TAG_PATH_FOR_DATA;
	static const std::string TAG_BEGIN_TABLE;
	static const std::string TAG_END_TABLE;

	static int pageSize;
	static int numberOfPages;
	static std::string pathForData;
	static TableMap tables;			// key: tableName value: map<columnName, dataType>
	static TableBlockMap tableBlocks;
	
};

#endif /* CONFIGURATIONS_HPP_ */
