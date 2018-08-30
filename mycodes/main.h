#ifndef WEB_PROXY_PROXY_H
#define WEB_PROXY_PROXY_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <thread>
#include <iostream>
#include "pthread.h"
#include <map>
#include "parser.h"
#include <ctime>

//extern map<string,string> mymap;
int initialize(int port_num);
#endif //WEB_PROXY_PROXY_H
