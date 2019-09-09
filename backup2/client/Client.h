#ifndef SOCKS_H_INCLUDED
#define SOCKS_H_INCLUDED

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/time.h>

#define SERV_TCP_PORT 8888 /* Local listening port number */
#define MAX_SIZE 1024*7120

// the address of socks server
#define SOCKS5_SERVER_IP "127.0.0.1"
#define SOCKS5_SERVER_PORT 9011

using namespace std;

class Client{
        private:
            // sockfd
            int sockfd;
            // sockaddr
            struct sockaddr_in serv_addr;
            /* Local listening port number */
            int port;
        public:
                // construction function
                Client(){
                    this->port=SERV_TCP_PORT;
                }
                Client(int port){
                    this->port=port;
                }
                ~Client(){
                }
                // listen to the local port
                void listenLocal();
                // waiting for the local port's connection 
                int acceptLocal();
                // connect to the socks server
                int connSockes5Server();
                // data transmission shake
                void transData(pair<int,int> pairval);
                // forever listenner function
                void forever();
                // server start
                void start();
};

#endif 
