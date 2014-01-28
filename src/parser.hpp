/*
 * cofig_parser.hpp
 *
 *  Created on: 25-Jan-2014
 *      Author: sriram
 */

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <string>

class ConfigParser {
public:
	void parse(std::string configFileName);
};

class ParserFactory {
public:
	static ConfigParser *newParserInstance();
};

#endif /* PARSER_HPP_ */
