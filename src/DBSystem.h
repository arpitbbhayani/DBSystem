/*
 * DBSystem.h
 *
 *  Created on: 25-Jan-2014
 *      Author: arpit
 */

#ifndef DBSYSTEM_H_
#define DBSYSTEM_H_

#include <string>

class DBSystem
{
public:
	DBSystem();
	virtual ~DBSystem();
	
	void populateDBInfo();
	void populateDBInfo(std::string);
	void readConfig(std::string);
	void insertRecord(std::string,std::string);
	std::string getRecord(std::string,int);
	
private:
	bool canLineBeAdded(std::string,int); 
	
};

#endif /*DBSYSTEM_H_*/
