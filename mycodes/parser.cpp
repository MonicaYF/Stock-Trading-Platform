/*
#include<cassert>
#include<cstdlib>
#include<new>
#include<exception>
#include"rapidxml.hpp"
#include"rapidxml_print.hpp"
#include<iostream>
using namespace rapidxml;
using namespace std;
*/
#include "parser.h"
#include <string.h>
#include <mutex> 
//extern std::unordered_map<std::string,std::string> mymap;
mutex mtx1;
int uid = 0;

void parser(connection * C, mybase * database, xml_document<> &create, string text) {
  
  //  mymap.insert(pair<string,string>("abc","def")); 
  xml_document<> doc;
  doc.parse<0>(&(text[0]));
  string describe = " ";
  xml_node<> * create_or_transaction = doc.first_node();
  
  while(1) {
    
    try{
      if (strcmp(create_or_transaction -> name(),"create") == 0) {
	//    xml_document<> create;
	xml_node<> * result = create.allocate_node(node_element, "results");
	create.append_node(result);
	xml_node<> * account_or_symbol = create_or_transaction -> first_node();
	
	while (1) {
	  
	  try{
	    if (strcmp(account_or_symbol -> name(), "account") == 0) {
	      string account = account_or_symbol -> first_attribute() -> value();
	      int accountid = stoi(account);
	      cout << "account id = " << accountid<<'\n';
	      string balance = account_or_symbol -> first_attribute() -> next_attribute() -> value();
	      double balancevalue = stod(balance);
	      cout << "balance = " << balance <<'\n';
	      if (database -> createAccount(C,describe,accountid,balancevalue)) {
		xml_node<> * createAccount = create.allocate_node(node_element, "created");
		result -> append_node(createAccount);
		char *ID_value = create.allocate_string(account.c_str());
		xml_attribute<> *accountAttri = create.allocate_attribute("ID", ID_value);
		createAccount -> append_attribute(accountAttri);
		//	    cout<<"this is after create account:" <<'\n'<<create<<'\n';
	      }
	      else {
		char * describe_s = create.allocate_string(describe.c_str());
		xml_node<> * createAccount = create.allocate_node(node_element, "error", describe_s);
		result -> append_node(createAccount);
		char * account_s = create.allocate_string(account.c_str());
		xml_attribute<> *accountAttri= create.allocate_attribute("id", account_s);
		createAccount-> append_attribute(accountAttri);
	      }
	    }
	    else if (strcmp(account_or_symbol -> name(), "symbol") == 0) {
	      string symbolname = account_or_symbol -> first_attribute() -> value();
	      cout <<"the name of this symbol is " << symbolname <<'\n';
	      xml_node<> * accountnode = account_or_symbol ->first_node();
	      
	      while (1) {
		try{
		  string account = accountnode -> first_attribute() -> value();
		  int accountid = stoi(account);
		  cout << "account id = " << accountid<<'\n';
		  string amount = accountnode -> value();
		  double amountValue = stod(amount);
		  cout << "amount = " << amountValue<<'\n';
		  
		  
		  if (database -> createSymbol(C, describe, accountid,amountValue, symbolname)) {
		    xml_node<> * createAccount = create.allocate_node(node_element, "created");
		    result -> append_node(createAccount);
		    char * account_s =  create.allocate_string(account.c_str());
		    xml_attribute<> *accountAttri1= create.allocate_attribute("ID", account_s);
		    createAccount-> append_attribute(accountAttri1);
		    char * symbolname_s = create.allocate_string(symbolname.c_str());
		    xml_attribute<> *accountAttri2= create.allocate_attribute("SYM", symbolname_s);
		    createAccount-> append_attribute(accountAttri2);
		  }
		  else {
		    char * describe_s = create.allocate_string(describe.c_str());
		    xml_node<> * createAccount = create.allocate_node(node_element, "error", describe_s);
		    result -> append_node(createAccount);
		    char * symbol_s = create.allocate_string(symbolname.c_str());
		    xml_attribute<> *accountAttri= create.allocate_attribute("sym", symbol_s);
		    createAccount-> append_attribute(accountAttri);
		  }
		}catch (...) {
		  xml_node<> * createAccount = create.allocate_node(node_element, "error", "formate error!'\n'");
		  result -> append_node(createAccount);
		}
		
		if (accountnode == account_or_symbol -> last_node()) break;
		else {
		  accountnode = accountnode -> next_sibling();
		}
	      }
	    }
	    else {
	      cout << "you can only create with an account or an symbol";
	    }
	  }catch(...){
	    xml_node<> * createAccount = create.allocate_node(node_element, "error", "Format error!");
	    result -> append_node(createAccount);
	  }
	  if (account_or_symbol == create_or_transaction -> last_node()) break;
	  else account_or_symbol = account_or_symbol -> next_sibling();
	}
      }
      else if (strcmp(create_or_transaction -> name(), "transactions") == 0) {
	xml_node<> * result = create.allocate_node(node_element, "results");
	create.append_node(result);
	
	string accountid = create_or_transaction -> first_attribute() -> value();
	int account_id = stoi(accountid);
	xml_node<> * transactionnode = create_or_transaction ->first_node();
	while(1) {
	  
	  try{
	    if (strcmp(transactionnode -> name(), "order") == 0) {
	      xml_attribute<> * sym = transactionnode -> first_attribute();
	      string symbolname = sym -> value();
	      string amount = sym -> next_attribute() -> value();
	      double amount_value = stod(amount);
	      cout<< "the amount of this order is : " << amount_value<<'\n';
	      string limit = sym -> next_attribute() -> next_attribute() -> value();
	      double price = stod(limit);
	      cout << "the price is" << price<<'\n';
	      mtx1.lock();
	      uid++;
	      mtx1.unlock();
	      
	      if (stod(amount) > 0) {
		
		if (database -> buy(C, describe, uid, account_id, amount_value, symbolname,price)) {
		  cout << "finished buying" << '\n';
		  xml_node<> * transaction = create.allocate_node(node_element, "opened");
		  result -> append_node(transaction);
		  char * symbolname_s = create.allocate_string(symbolname.c_str());
		  xml_attribute<> *Attri1= create.allocate_attribute("SYM", symbolname_s);
		  transaction-> append_attribute(Attri1);
		  char * amount_s = create.allocate_string(amount.c_str());
		  xml_attribute<> *Attri2= create.allocate_attribute("amount", amount_s);
		  transaction-> append_attribute(Attri2);
		  char * limit_s = create.allocate_string(limit.c_str());
		  xml_attribute<> *Attri3= create.allocate_attribute("limit", limit_s);
		  transaction-> append_attribute(Attri3);
		  char * uid_s = create.allocate_string(to_string(uid).c_str());
		  xml_attribute<> *Attri4= create.allocate_attribute("uid", uid_s);
		  transaction-> append_attribute(Attri4);
		  cout << "finished buying sql" << '\n';	      
		}
		else {
		  cout << "failed buying" << '\n';
		  char * describe_s = create.allocate_string(describe.c_str());
		  xml_node<> * transaction = create.allocate_node(node_element, "error", describe_s);
		  result -> append_node(transaction);
		  char * symbolname_s = create.allocate_string(symbolname.c_str());
		  xml_attribute<> *Attri1= create.allocate_attribute("SYM", symbolname_s);
		  transaction-> append_attribute(Attri1);
		  char * amount_s =create.allocate_string(amount.c_str());
		  xml_attribute<> *Attri2= create.allocate_attribute("amount", amount_s);
		  transaction-> append_attribute(Attri2);
		  char * limit_s = create.allocate_string(limit.c_str());
		  xml_attribute<> *Attri3= create.allocate_attribute("limit", limit_s);
		  transaction-> append_attribute(Attri3);
		  
		}
	      }
	      else {
		
		if (database -> sell(C, describe, uid, account_id, 0 - amount_value, symbolname, price)) {
		  cout << "finished selling " << '\n';
		  xml_node<> * transaction = create.allocate_node(node_element, "opened");
		  result -> append_node(transaction);
		  char * symbolname_s = create.allocate_string(symbolname.c_str());
		  xml_attribute<> *Attri1= create.allocate_attribute("SYM", symbolname_s);
		  transaction-> append_attribute(Attri1);
		  char * amount_s =create.allocate_string(amount.c_str());
		  xml_attribute<> *Attri2= create.allocate_attribute("amount", amount_s);
		  transaction-> append_attribute(Attri2);
		  char * limit_s = create.allocate_string(limit.c_str());
		  xml_attribute<> *Attri3= create.allocate_attribute("limit", limit_s);
		  transaction-> append_attribute(Attri3);
		  char * uid_s = create.allocate_string(to_string(uid).c_str());
		  xml_attribute<> *Attri4= create.allocate_attribute("uid", uid_s);
		  transaction-> append_attribute(Attri4);
		  cout << "finished selling sql" << '\n';
		}
		else {
		  cout << "failed selling" << '\n';
		  char * describe_s = create.allocate_string(describe.c_str());
		  xml_node<> * transaction = create.allocate_node(node_element, "error", describe_s);
		  result -> append_node(transaction);
		  char * symbolname_s = create.allocate_string(symbolname.c_str());
		  xml_attribute<> *Attri1= create.allocate_attribute("SYM", symbolname_s);
		  transaction-> append_attribute(Attri1);
		  char * amount_s =create.allocate_string(amount.c_str());
		  xml_attribute<> *Attri2= create.allocate_attribute("amount", amount_s);
		  transaction-> append_attribute(Attri2);
		  char * limit_s = create.allocate_string(limit.c_str());
		  xml_attribute<> *Attri3= create.allocate_attribute("limit", limit_s);
		  transaction-> append_attribute(Attri3);
		}
	      }//sell
	    }//order
	    else if (strcmp(transactionnode -> name(),"query") == 0){
	      std::vector<std::string> status;
	      std::vector<std::string> shares;
	      std::vector<std::string> times;
	      std::vector<std::string> prices;
	      
	      int uid = stoi(transactionnode -> first_attribute() -> value());
	      cout<<"uid of query is " << uid;
	      //	      xml_node<> * result = create.allocate_node(node_element, "result");
	      //create.append_node(result);
	      
	      if (database -> query(C, describe, uid, status, shares, times, prices)) {
		cout<< "finished query" << '\n';
		
		xml_node<> * query = create.allocate_node(node_element, "status");
		result -> append_node(query);
	      
		for (size_t i = 0; i < status.size(); i++) {
		  
		  if (status[i] == "OPEN") {
		    xml_node<> * transaction = create.allocate_node(node_element, "open");
		    query -> append_node(transaction);
		    char* share_s = create.allocate_string(shares[i].c_str());
		    xml_attribute<> *Attri1= create.allocate_attribute("shares", share_s);
		    transaction-> append_attribute(Attri1);
		  }
		  else if (status[i] == "CANCEL") {
		    xml_node<> * transaction = create.allocate_node(node_element, "canceled");
		    query -> append_node(transaction);
		    char* share_s = create.allocate_string(shares[i].c_str());
		    xml_attribute<> *Attri1= create.allocate_attribute("shares", share_s);
		    transaction-> append_attribute(Attri1);
		    char * time_s = create.allocate_string(times[i].c_str());
		    xml_attribute<> *Attri2= create.allocate_attribute("time", time_s);
		    transaction-> append_attribute(Attri2);
		  }
		  else if (status[i] == "EXECUTED") {
		    xml_node<> * transaction = create.allocate_node(node_element, "executed");
		    query -> append_node(transaction);
		    char* share_s = create.allocate_string(shares[i].c_str());
		    xml_attribute<> *Attri1= create.allocate_attribute("shares", share_s);
		    transaction-> append_attribute(Attri1);
		    char * price_s = create.allocate_string(prices[i].c_str());
		    xml_attribute<> *Attri2= create.allocate_attribute("price", price_s);
		    transaction-> append_attribute(Attri2);
		    char * time_s =create.allocate_string(times[i].c_str());
		    xml_attribute<> *Attri3= create.allocate_attribute("time", time_s);
		    transaction-> append_attribute(Attri3);
		  }
		}
	      }
	      else {
		cout << "failed query" << '\n';
		char * describe_s =  create.allocate_string(describe.c_str());
		xml_node<> * query = create.allocate_node(node_element, "error",describe_s);
		result -> append_node(query);
	      }
	    }//query
	    else if (strcmp(transactionnode -> name(), "cancel") == 0) {
	      std::vector<std::string> status;
	      std::vector<std::string> shares;
	      std::vector<std::string> times;
	      std::vector<std::string> prices;
	      int uid = stoi(transactionnode -> first_attribute() -> value());
	      if (database -> cancel(C, describe, uid, status, shares, times, prices)) {
		cout << "finised caneling" << '\n';
		xml_node<> * cancel = create.allocate_node(node_element, "canceled");
		result -> append_node(cancel);
		
		for (size_t i = 0; i < status.size(); i++) {
		  
		  if (status[i] == "CANCEL") {
		    xml_node<> * transaction = create.allocate_node(node_element, "canceled");
		    cancel -> append_node(transaction);
		    char* share_s = create.allocate_string(shares[i].c_str());	
		    xml_attribute<> *Attri1= create.allocate_attribute("shares", share_s);
		    transaction-> append_attribute(Attri1);
		    char * time_s =create.allocate_string(times[i].c_str());
		    xml_attribute<> *Attri2= create.allocate_attribute("time", time_s);
		    transaction-> append_attribute(Attri2);
		  }
		  else if (status[i] == "EXECUTED") {
		    xml_node<> * transaction = create.allocate_node(node_element, "executed");
		    cancel -> append_node(transaction);
		    char* share_s = create.allocate_string(shares[i].c_str());
		    xml_attribute<> *Attri1= create.allocate_attribute("shares", share_s);
		    transaction-> append_attribute(Attri1);
		    char * price_s = create.allocate_string(prices[i].c_str());
		    xml_attribute<> *Attri2= create.allocate_attribute("price", price_s);
		    transaction-> append_attribute(Attri2);
		    char * time_s =create.allocate_string(times[i].c_str());
		    xml_attribute<> *Attri3= create.allocate_attribute("time", time_s);
		    transaction-> append_attribute(Attri3);
		  }
		}
		cout << "finished cancling sql" << '\n';
	      
	      }
	      else {
		cout << "failed cancling" << '\n';
		char * describe_s =  create.allocate_string(describe.c_str());
		xml_node<> * query = create.allocate_node(node_element, "error",describe_s);
		result -> append_node(query);
	      }
	    }
	  }catch(...) {
	    xml_node<> * createAccount = create.allocate_node(node_element, "error", "format error!");
	    result -> append_node(createAccount);
	  }
	  if (transactionnode == create_or_transaction ->last_node()) break;
	  else transactionnode = transactionnode -> next_sibling();
	  cout << "end of transaction" << '\n';
	}
      }//transaction
    }catch(...) {
      xml_node<> * createAccount = create.allocate_node(node_element, "error", "format error!");
      create.append_node(createAccount);
    }
    
    if (create_or_transaction == doc.last_node()) break;
    else create_or_transaction = create_or_transaction -> next_sibling();
  }
}


