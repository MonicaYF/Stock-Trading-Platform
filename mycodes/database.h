#include <iostream>
#include <pqxx/pqxx>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <math.h>

using namespace pqxx;

//#include <map>

//extern std::map<std::string,std::string> mymap;


class mybase{
 public:
  int createTable();
  bool createAccount(connection* C, std::string &describe,int account_id, double balance);
  bool createSymbol(connection* C, std::string &describe,int account_id,double amount,std::string symbol);
  bool buy(connection* C,std::string &describe,int uid,int account_id,double amount,std::string symbol,double price);
  bool sell(connection* C,std::string &describe,int uid,int account_id,double amount,std::string symbol,double price);
  bool query(connection* C,std::string &describe,int uid,std::vector<std::string> &status,std::vector<std::string> &shares,std::vector<std::string> &time,std::vector<std::string> &price);
  bool cancel(connection* C,std::string &describe,int uid,std::vector<std::string> &status,std::vector<std::string> &shares,std::vector<std::string> &time,std::vector<std::string> &price);
};
