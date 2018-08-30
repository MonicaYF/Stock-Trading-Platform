#include<cassert>
#include<cstdlib>
#include<new>
#include<exception>
#include"rapidxml.hpp"
#include"rapidxml_print.hpp"
#include<iostream>
#include<string>
#include<unordered_map>
#include"database.h"
using namespace rapidxml;
using namespace std;

extern map<string,string> mymap;

void parser(connection * c,mybase* database, xml_document<> &create, string text);
