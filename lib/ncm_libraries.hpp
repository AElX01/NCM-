#ifndef NCM_LIBRARIES

#define NCM_LIBRARIES


#include <iostream>
#include <libssh/libssh.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <set>
#include <chrono>
#include <ctime>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <filesystem>
#include <vector>
#include <iterator>
#include <algorithm>
#include <map>

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using nlohmann::json;
using std::ifstream;
using std::set;
using std::ofstream;
using std::system;
using std::vector;
using std::size_t;
using std::equal;
using std::istreambuf_iterator;
using std::ios;
using std::unordered_map;
using std::system;

namespace fs = std::filesystem;

#endif