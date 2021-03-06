#include "database.h"
#include <iomanip>
#include<mutex>

std::mutex mtx;

//std::map<std::string,std::string> mymap;

int mybase::createTable(){
  std::string sql;
  connection *C;
  try{
    C = new connection("dbname = postgres user = postgres host=my_server port=5432");
    if (C->is_open()) {
      std::cout << "Opened database successfully: " << C->dbname() << std::endl;
    } else {
      std::cout << "Can't open database" << std::endl;
      return 1;
    }
  } catch (const std::exception &e){
    std::cerr << e.what() << std::endl;
    return 1;
  }
  sql = "DROP TABLE IF EXISTS ORDERS;";
  work W(*C);
  W.exec(sql);
  sql ="DROP TABLE IF EXISTS POSITION;";
  W.exec(sql);
  sql ="DROP TABLE IF EXISTS ACCOUNT;";
  W.exec(sql);
  sql = "CREATE TABLE ACCOUNT(" \
        "ACCOUNT_ID INT PRIMARY KEY NOT NULL," \
        "BALANCE DECIMAL NOT NULL CHECK(BALANCE>=0));";
  W.exec(sql);

  sql = "CREATE TABLE POSITION(" \
        "SYMBOL TEXT NOT NULL," \
        "ACCOUNT_ID INT NOT NULL,"\
        "CONSTRAINT account_pkey FOREIGN KEY(ACCOUNT_ID) REFERENCES ACCOUNT(ACCOUNT_ID) ON DELETE CASCADE ON UPDATE CASCADE,"\
        "AMOUNT REAL NOT NULL CHECK(AMOUNT>=0),"\
        "CONSTRAINT SYMBOL_ACCOUNT UNIQUE(ACCOUNT_ID,SYMBOL));";
  
  W.exec(sql);
  //need time
  sql = "CREATE TABLE ORDERS(" \
        "ID INT NOT NULL," \
        "TYPE TEXT NOT NULL," \
	"ACCOUNT_ID INT NOT NULL,"\
	"CONSTRAINT account_pkey FOREIGN KEY(ACCOUNT_ID) REFERENCES ACCOUNT(ACCOUNT_ID) ON DELETE CASCADE ON UPDATE CASCADE,"\
	"PRICE DECIMAL NOT NULL CHECK(PRICE>0),"\
	"SYMBOL TEXT NOT NULL,"\
	"AMOUNT REAL NOT NULL CHECK(AMOUNT>0),"\
        "TIME TIMESTAMP NOT NULL,"\
        "STATUS TEXT NOT NULL);";
   W.exec(sql);

   sql = "CREATE INDEX orders_tss ON ORDERS(TYPE,STATUS,SYMBOL);";
   W.exec(sql);
   sql = "CREATE INDEX position_tss ON POSITION(ACCOUNT_ID,SYMBOL);";
   W.exec(sql);
   sql = "CREATE INDEX ACCOUNT_tss ON ACCOUNT(ACCOUNT_ID);";
   W.exec(sql);
   W.commit();
   return 0;
 
}

bool mybase::createAccount(connection*C, std::string &describe, int account_id, double balance){
  if (balance<0){
    describe = "Balance should not be negative";
    return false;
  }
  if (account_id<0){
    describe = "Account should not be negative";
    return false;
  }
  
  std::string sql = "SELECT * FROM ACCOUNT WHERE (ACCOUNT_ID ="+ to_string(account_id)  +") FOR UPDATE";
  //  mtx.lock();
  work W(*C);
  result R = W.exec(sql);
  result::const_iterator c = R.begin();
  if(c != R.end())
  {
    describe = "Account already exist";
    W.commit();
    return false;
   }else{
    std::string sql ="INSERT INTO ACCOUNT(ACCOUNT_ID,BALANCE) "           \
      "VALUES(" + to_string(account_id) + ","+ to_string(balance) + ");";
      W.exec(sql);
     }
   W.commit();
   //mtx.unlock();
  describe = "Successfully create account";
  return true;
}
//I learn how to do this from:https://bbs.csdn.net/topics/390880436?list=lz
double Round(double dVal, short iPlaces) {
  double dRetval;
  double dMod = 0.0000001;
  if (dVal<0.0) dMod=-0.0000001;
  dRetval=dVal;
  dRetval+=(5.0/pow(10.0,iPlaces+1.0));
  dRetval*=pow(10.0,iPlaces);
  dRetval=floor(dRetval+dMod);
  dRetval/=pow(10.0,iPlaces);
  return(dRetval);
}

bool cp_ifnothave(work &W,connection* C,int account_id,std::string symbol,double amount){
  std::string sql;
  sql = "SELECT * FROM POSITION WHERE (ACCOUNT_ID =" + to_string(account_id) + ") AND (SYMBOL ='" + symbol  + "') FOR UPDATE";
  result Q = W.exec(sql);
  result::const_iterator c = Q.begin();
  if (c == Q.end()){
    sql = "INSERT INTO POSITION(SYMBOL,ACCOUNT_ID,AMOUNT)"\
      "VALUES('" + symbol + "'," + to_string(account_id) + "," + to_string(amount) +  ");";
    W.exec(sql);
    return true;
  }else{
    return false;
  }
}

void update_position(work &W,connection* C,int account_id,std::string symbol,double amount){
  std::string sql;
  amount = Round(amount,2);
  sql = "SELECT * FROM POSITION WHERE(ACCOUNT_ID = " + to_string(account_id) +") AND ( symbol = '" + symbol + "') FOR UPDATE;";
  W.exec(sql);
  sql = "UPDATE POSITION SET AMOUNT = AMOUNT + " + to_string(amount) + "WHERE(ACCOUNT_ID = " + to_string(account_id) +") AND ( symbol = '" + symbol + "');";
  W.exec(sql);
}

bool mybase::createSymbol(connection* C,std::string &describe,int account_id,double amount,std::string symbol){
  if(amount<=0.0) {
    describe = "Amount should be positive";
    return false;
  }
    std::string sql;
    sql = "SELECT * FROM ACCOUNT WHERE(ACCOUNT_ID = " + to_string(account_id) + ") FOR UPDATE;";
    work W(*C);
    result R = W.exec(sql);
    result::const_iterator c = R.begin();
    if(c == R.end())
    {
    describe = "Account does not exist";
    W.commit();
    return false;
    }else{
      bool flag = cp_ifnothave(W,C,account_id,symbol,amount);
      if(flag) {
      }else{   //if account and symbol already exist
       try{
	 update_position(W,C,account_id,symbol,amount);
	 describe = "Successfully add amount into position";
       }catch(...){
	 describe = "Fail to add amount into position";
         W.commit();
	 return false;
       }
       }
    }
    W.commit();
    return true;
}

void refresh_order(work &W,connection* C, bool flag, std::string type, int account_id, std::string symbol,double amount, std::string status,double price ,int uid){
  std::string sql;
  amount = Round(amount,2);
  if (flag == true){   //flag = true, update exist order status
    if (!status.compare("OPEN")){
      sql = "SELECT * FROM ORDERS WHERE ( ID = " + to_string(uid) + ") AND ( status = 'OPEN') FOR UPDATE;";
      W.exec(sql);
      sql = "UPDATE ORDERS SET AMOUNT = "+ to_string(amount)  +" WHERE ( ID = " + to_string(uid) + ") AND ( status = 'OPEN');"; 
      W.exec(sql);
      // W.commit();    
    }else if (!status.compare("EXECUTED")){
      sql = "SELECT * FROM ORDERS WHERE ( ID = " + to_string(uid) + ") AND ( status = 'OPEN' ) FOR UPDATE ;";
      W.exec(sql);
      sql = "UPDATE ORDERS SET AMOUNT = " + to_string(amount) + ", PRICE ="+ to_string(price) + ", STATUS = 'EXECUTED', TIME = CURRENT_TIMESTAMP"  + " WHERE ( ID = " + to_string(uid) + ") AND ( status = 'OPEN' ) ;";
      W.exec(sql);
      // W.commit();
    }else if (!status.compare("CANCEL")){
      sql = "SELECT * FROM ORDERS WHERE ( ID = " + to_string(uid) + ") AND ( status = 'OPEN' ) FOR UPDATE;";
      W.exec(sql);
      sql = "UPDATE ORDERS SET STATUS = 'CANCEL', TIME = CURRENT_TIMESTAMP WHERE ( ID = " + to_string(uid) + ") AND ( status = 'OPEN' );";
      // work W(*C);
      W.exec(sql);
      // W.commit();
    }
  }else{    //flag = false, create new order status
     sql = "INSERT INTO ORDERS(ID,TYPE,ACCOUNT_ID,PRICE,SYMBOL,AMOUNT,TIME,STATUS)"\
       "VALUES(" + to_string(uid)+ ",'"+ type + "'," + to_string(account_id) + "," + to_string(price) + ",'" + symbol + "'," + to_string(amount) + "," + " CURRENT_TIMESTAMP ,'"   + status + "')";
     // work W(*C);
     W.exec(sql);
     // W.commit();
  }
}

void update_balance(work &W,connection* C,int account_id,double balance){
  std::string sql;
  sql = "SELECT * FROM ACCOUNT WHERE(ACCOUNT_ID = " + to_string(account_id) + ") FOR UPDATE;";
  W.exec(sql);
  sql = "UPDATE ACCOUNT SET BALANCE = BALANCE +" + to_string(balance) + "WHERE(ACCOUNT_ID = " + to_string(account_id) + ");";
  W.exec(sql);
}

bool mybase::buy(connection* C,std::string &describe,int uid,int account_id,double amount,std::string symbol,double price){
  if(price <= 0){
    describe = "Price should be positive";
    return false;
  }
  std::string sql;
  sql = "SELECT BALANCE FROM ACCOUNT WHERE(ACCOUNT_ID = " + to_string(account_id) + ");";
  work W(*C);
  result R = W.exec(sql);
  W.commit();
  result::const_iterator c = R.begin();
  if(c == R.end()){   //if account doesn't exist
      describe = "Account does not exist";
      return false;
  }else{
    amount = Round(amount,2);
    double balance = c[0].as<double>();
    double can_buy = balance - ( price * amount);
    if(can_buy < 0){  //if balance doesn't enough
      describe = "Balance not enough";
      return false;
    }else{ //put the buy transaction into orders 
      work R(*C);
      refresh_order(R,C,false,"BUY",account_id,symbol,amount,"OPEN",price,uid);
      update_balance(R,C,account_id,- price * amount);
      cp_ifnothave(R,C,account_id,symbol,0.0);
      R.commit();
      sql = "SELECT * FROM ORDERS WHERE(TYPE = 'SELL') AND (SYMBOL = '" + symbol + "') AND (STATUS = 'OPEN') ORDER BY PRICE ASC, TIME ASC FOR UPDATE;";   //select all the sells
      work N(*C);
      result Q = N.exec(sql);
      for(result::const_iterator c = Q.begin();c != Q.end();++c){   //match one by one
        double sell_amount = c[5].as<double>();
	int sell_id =c[0].as<int>();
	int sell_account = c[2].as<int>();
	double sell_price = c[3].as<double>();
	
	if(sell_price > price) break;             //sell price > buy price cannot match and break
	if(amount == 0.0)    break;                         //buy amount is zero, break
	if(sell_amount >= amount ) {                   // if sell amount is larger than buy amount
	  refresh_order(N,C,true,"BUY",account_id,symbol,amount,"EXECUTED",sell_price,uid); //update buy
	  if(sell_amount > amount ){
	    refresh_order(N,C,true,"SELL",sell_account, symbol ,sell_amount - amount,"OPEN",sell_price,sell_id); //update sell
	    refresh_order(N,C,false,"SELL",sell_account,symbol,amount,"EXECUTED",sell_price,sell_id); //create executed sell
          }else{     //sell_amount equals to buy amount
            refresh_order(N,C,true,"SELL",sell_account,symbol,amount,"EXECUTED",sell_price,sell_id); 
	  }
	  if(price > sell_price){   //if buy price is larger than sell price, update balance
            update_balance(N,C,account_id,(price - sell_price) * amount);
	  }
	  update_balance(N,C,sell_account,sell_price * amount);
	  update_position(N,C,account_id,symbol,amount);
	  //  mtx.unlock();
	  break; }	
	else if ( sell_amount < amount ) {
	  refresh_order(N,C,true,"SELL",sell_account ,symbol,amount,"EXECUTED",sell_price,sell_id);
	  refresh_order(N,C,false,"BUY",account_id,symbol,amount,"EXECUTED",sell_price,uid); 
          refresh_order(N,C,true,"BUY",account_id,symbol,amount - sell_amount,"OPEN",price,uid);
	  update_balance(N,C,account_id, (price - sell_price) * sell_amount);
          update_balance(N,C,sell_account,sell_price * sell_amount);
	  amount -=sell_amount;
	  update_position(N,C,account_id,symbol,sell_amount);
	}   
      }//for
      N.commit();
    }//else
    }//else
  return true;    
}

bool mybase::sell(connection* C,std::string &describe,int uid,int account_id,double amount,std::string symbol,double price){
  if(price <= 0){
    describe = "Price should be positive";
    return false;
  }
  std::string sql;
  sql = "SELECT BALANCE FROM ACCOUNT WHERE(ACCOUNT_ID = " + to_string(account_id) + ");";
  nontransaction N(*C);
  result R(N.exec(sql));
  result::const_iterator c = R.begin();
  if(c == R.end())
    {
      describe = "Account does not exist";
      N.commit();
      return false;
    }else{
    amount = Round(amount,2);
    sql = "SELECT AMOUNT FROM POSITION WHERE(ACCOUNT_ID = " + to_string(account_id) + ") AND ( SYMBOL ='" + symbol + "');";
    result Q(N.exec(sql));
    N.commit();
    c = Q.begin();
    if(c == Q.end()){   
      describe = "The account doesn't have this symbol";
      return false;
    }else{   //account has that symbol
      if(amount > c[0].as<double>()){
	describe = "The account doesn't have that much amount to sell";
        return false;
      }else{ //can sell
       work W(*C);
       refresh_order(W,C,false,"SELL",account_id,symbol,amount,"OPEN",price,uid);
       update_position(W,C,account_id,symbol,- amount);
       W.commit();
       work E(*C);
       sql = "SELECT * FROM ORDERS WHERE(TYPE = 'BUY') AND (SYMBOL = '" + symbol + "') AND (STATUS = 'OPEN') ORDER BY PRICE DESC, TIME ASC FOR UPDATE;";   //select all the sells
       result Q = E.exec(sql);
       for(result::const_iterator c = Q.begin();c != Q.end();++c){   //match one by one
	 double buy_amount = c[5].as<double>();
	 int buy_id =c[0].as<int>();
	 int buy_account = c[2].as<int>();
	 double buy_price = c[3].as<double>();
         if(price > buy_price) break;             //sell price > buy price cannot match and break
	 if(amount == 0.0)    break;       //no available amount to sell
         if(buy_amount >= amount ) {                   // if sell amount is larger than buy amount
           cp_ifnothave(E,C,buy_account,symbol,0.0);
	   refresh_order(E,C,true,"SELL",account_id,symbol,amount,"EXECUTED",buy_price,uid); //update buy
	   if(buy_amount > amount ){
	     refresh_order(E,C,true,"BUY",buy_account, symbol ,buy_amount - amount,"OPEN",buy_price,buy_id); //update sell
	     refresh_order(E,C,false,"BUY",buy_account,symbol,amount,"EXECUTED",buy_price,buy_id); //create executed sell
	   }else{     //sell_amount equals to buy amount
	     refresh_order(E,C,true,"BUY",buy_account,symbol,amount,"EXECUTED",buy_price,buy_id);
	   }
	   update_balance(E,C,account_id,amount * buy_price);
	   update_position(E,C,buy_account,symbol,amount);
	   // mtx.unlock();
	   break; }
	 else if ( buy_amount < amount ) {
	   cp_ifnothave(E,C,buy_account,symbol,0.0);
	   refresh_order(E,C,true,"BUY",buy_account ,symbol,buy_amount,"EXECUTED",buy_price,buy_id);
	   refresh_order(E,C,false,"SELL",account_id,symbol,buy_amount,"EXECUTED",buy_price,uid);
	   refresh_order(E,C,true,"SELL",account_id,symbol,amount - buy_amount,"OPEN",price,uid);
	   update_position(E,C,buy_account,symbol,buy_amount);
	   amount -= buy_amount;
	   update_balance(E,C,account_id,buy_amount * buy_price);
	   // mtx.unlock();
	 }
       }//for
          E.commit();
      }
    }//else, c!=R.end
  }
    return true;
}

bool mybase::query(connection*C,std::string &describe,int uid, std::vector<std::string> &status,std::vector<std::string> &shares,std::vector<std::string> &time,std::vector<std::string> &price){
  std::string sql;
  sql = "SELECT STATUS,AMOUNT,EXTRACT(EPOCH FROM TIME),PRICE FROM ORDERS WHERE (ID =" + to_string(uid) + ");";
  // mtx.lock();
  nontransaction N(*C);
  result Q(N.exec(sql));
  N.commit();
  // mtx.unlock();
  result::const_iterator c = Q.begin();
  if(c == Q.end()){
    describe = "Transaction doesn't exist";
    return false;
  }
  for (c = Q.begin();c != Q.end();++c){
    status.push_back(c[0].as<std::string>());
    shares.push_back(c[1].as<std::string>());
    time.push_back(c[2].as<std::string>());
    price.push_back(c[3].as<std::string>());
  }
  return true;
}

bool mybase::cancel(connection* C,std::string &describe,int uid,std::vector<std::string> &status,std::vector<std::string> &shares,std::vector<std::string> &time,std::vector<std::string> &price){
  std::string sql;
  sql = "SELECT TYPE,ACCOUNT_ID,SYMBOL,AMOUNT,PRICE FROM ORDERS WHERE (ID =" + to_string(uid) + ") AND (STATUS = 'OPEN') FOR UPDATE;";
  // mtx.lock();
  work N(*C);
  result Q = N.exec(sql);
  // mtx.unlock();
  result::const_iterator c = Q.begin();
  if(c == Q.end()){
    describe = "Transaction doesn't exist";
    return false;
  }else{
    std::string type = c[0].as<std::string>();
    int account_id = c[1].as<int>();
    std::string symbol = c[2].as<std::string>();
    double amount = c[3].as<double>();
    double price = c[4].as<double>();
    if(!type.compare("BUY")){
      double balance = amount * price;
      //  mtx.lock(); 
      update_balance(N,C,account_id,balance);
      //mtx.unlock();
    }else if(!type.compare("SELL")){
      //mtx.lock();
      update_position(N,C, account_id, symbol, amount); 
      //mtx.unlock();
    }
  }
  //mtx.lock();
  refresh_order(N,C,true,"",0,"",0,"CANCEL",0.0,uid);
  //mtx.unlock();
  N.commit();
  return query(C,describe,uid,status,shares,time,price);
}
