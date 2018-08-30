#include "main.h"

int serverSocketFD;

int initialize(int port_num) {
  int proxySocketFD;
  try {//socket
    proxySocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (proxySocketFD < 0) {
      throw proxySocketFD;
    }
  } catch (int proxySocketFD) {
    std::cout << "Error : Cannot create socket for master : \n";
  }
  try {//set socket to be reusable when restart (port and IP)
    int flag = 1;
    int socOpt = setsockopt(proxySocketFD, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
    if (socOpt < 0) {
      throw socOpt;
    }
  } catch (int socOpt) {
    std::cout << "Error : SockOpt : \n";
  }
  //set an sockaddr for bind
  
  struct sockaddr_in serverAddr;
  memset((void *)&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET; //ipv4
  serverAddr.sin_addr.s_addr = INADDR_ANY; //don't need to bind with a specific IP
  serverAddr.sin_port = htons(port_num);

  try {//bind
    int bound = bind(proxySocketFD, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    if (bound < 0) {
      throw bound;
    }
  } catch (int bound) {
    std::cout << " ??EError : Cannot bind socket : \n";
  }
  try {//listen
    int listening = listen(proxySocketFD, 10000);
    if (listening < 0) {
      throw listening;
    }
  } catch (int listening) {
    std::cout << "Error : Cannot listen to socket : \n";
  }
  return proxySocketFD;
}

void handler(int clientFD,std::string IP){
  clock_t start,end;
  start = clock();

  mybase* database = new mybase();
  // database->createTable();
  connection *C;
  try{
    C = new connection("dbname = postgres user = postgres host=my_server port=5432 ");
    if (C->is_open()) {
      std::cout << "Opened database successfully: " << C->dbname() << std::endl;
    } else {
      std::cout << "Can't open database" << std::endl;
      return;
    }
  } catch (const std::exception &e){
    std::cerr << e.what() << std::endl;
    return;
  }
  
  ssize_t dataRecv = 0;
  ssize_t dataTotal = 0;
  int find;
  int text = 0;
  std::vector<char> buffer(10000);
  std::vector<char> recv_container(10000);
  try {
    int i = 0;
    while (true) {
      dataRecv = recv(clientFD,&recv_container.data()[0], 10000, 0);
      dataTotal += dataRecv;
      i++;
      if (dataRecv < 0) {
	throw dataRecv;
      }
      if(i == 1){
	buffer.assign(recv_container.begin(), recv_container.end());        //copy recv_container to buffer
	std::string aaa(buffer.begin(),buffer.end());
        find = aaa.find_first_of("\n");
	aaa = aaa.substr(0,find-2);
	text = atoi(aaa.c_str());
       } else{                                                              //append recv_container to buffer
	buffer.insert(std::end(buffer), std::begin(recv_container), std::end(recv_container));
      }
      if (dataTotal >= text){ break;}
      recv_container.clear();
    }
  } catch (int dataRecv) {
    std::cout << "Error : Cannot receive data : \n";
  }
  std::string whole_receive(buffer.begin(),buffer.end());
  whole_receive = whole_receive.substr(find,whole_receive.size());
  
  std::cout <<  whole_receive << std::endl;
  xml_document<> create;
  parser(C, database, create,whole_receive);
  std::stringstream ss;
  ss<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  ss<<create;
  std::cout<<"The result is \n"<< ss.str();  
  send(clientFD ,(const void *)ss.str().c_str(),ss.str().size(), 0);
  close(clientFD);
  C->disconnect();
  delete database;
  end = clock();
  std::cout <<" Time for this request is :"<<(double)(end - start)/CLOCKS_PER_SEC <<std::endl;
}

int main(){  
  
 mybase* database = new mybase();
  database->createTable();
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
  C -> disconnect();
  delete database;
  /*
  std::string describe="";
  std::string symbol = "SPY";

  database->createAccount(C,describe,12345,20);
  //std::cout<<"1:" <<describe<<std::endl;

  //database->createAccount(C,describe,12346,2000);
  //std::cout<<"2:" <<describe<<std::endl;

  //database->createAccount(C,describe,12347,20);
  //std::cout<<"3:" <<describe<<std::endl;
  
  database->createSymbol(C,describe,12345,100,symbol);
  std::cout << mymap["aaa"]<<"\n";
  //std::cout<<"4:"<<describe<<std::endl;
  
    database->createSymbol(C,describe,12345,10,symbol);
  //  std::cout<<"4:"<<describe<<std::endl;
  
  //database->createSymbol(C,describe,12347,10,symbol);
  //std::cout<<"5:"<<describe<<std::endl;

  //database->sell(C,describe,1,12347,20.123,symbol,12.0);
  //std::cout<<"7:"<<describe<<std::endl;

  database->buy(C,describe,2,12345,10.123,symbol,10.0);
  std::cout<<"8:"<<describe<<std::endl;
  
  // database->cancel(C,describe,2);
  
  //database->buy(C,describe,3,12345,9.123,symbol,13.0);
  //std::cout<<"9:"<<describe<<std::endl;
  
  std::vector<std::string> status;
  std::vector<std::string> shares;
  std::vector<std::string> times;
  std::vector<std::string> price;
  database->query(C,describe,1,status,shares,times,price);
  for(unsigned int i = 0;i<status.size();i++){
    std::cout<< status[i] <<std::endl;
  }
  for(unsigned int i = 0;i<shares.size();i++){
    std::cout<<shares[i] <<std::endl;
  }
  
  //std::cout << status;
  */
  serverSocketFD = initialize(12345);
  while(true) {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int clientFD = accept(serverSocketFD, (struct sockaddr *) &client, &client_len);
    std::string IP = inet_ntoa(client.sin_addr);
    std::thread handler_thread(handler, clientFD,IP);
    handler_thread.detach();
  }
  return 0;
}
