
using namespace std;

#include <string>

#include "configurations.hpp"

int Configurations::pageSize;
int Configurations::numberOfPages;
string Configurations::pathForData;
TableMap Configurations::tables;
TableBlockMap Configurations::tableBlocks;

const string Configurations::CONFIG_FILE_PATH = "./config/config.txt";

const string Configurations::TAG_PAGESIZE = "PAGE_SIZE";
const string Configurations::TAG_NUM_PAGES = "NUM_PAGES";
const string Configurations::TAG_PATH_FOR_DATA = "PATH_FOR_DATA";
const string Configurations::TAG_BEGIN_TABLE = "BEGIN";
const string Configurations::TAG_END_TABLE = "END";