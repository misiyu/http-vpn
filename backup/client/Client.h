#ifndef SOCKS_H_INCLUDED
#define SOCKS_H_INCLUDED

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/time.h>

#include "ccn_p2p/ndn_socket.h"

#define SERV_TCP_PORT 8888 /* Local listening port number */
#define MAX_SIZE 1024*7120

// the address of socks server
#define SOCKS5_SERVER_IP "127.0.0.1"
#define SOCKS5_SERVER_PORT 9011

using namespace std;
using std::pair;

class Client{
	private:
		// sockfd
		int sockfd;
		// sockaddr
		struct sockaddr_in serv_addr;
		/* Local listening port number */
		int port;

		Ndn_socket mndn_socket ;
		string listen_prefix ;
		unsigned int p_port_seq ;
		string server_prefix ;

	public:
		// construction function
		Client(){
			this->port=SERV_TCP_PORT;
			this->server_prefix = "/localhost/nfd/vpn/server" ;
			cout << server_prefix << endl ;
		}
		Client(int port){
			this->port=port;
			this->server_prefix = "/localhost/nfd/vpn/server" ;
			cout << server_prefix << endl ;
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

		void ndn_listen_local(string prefix) ;
		Ndn_socket * ndn_connSockes5Server();
		// data transmission shake
		void ndn_transData(Ndn_socket &ndn_socket , int remote_sockfd);
		// forever listenner function
		void ndn_forever();
		void *ndn_thread1(void *val);
		void *ndn_thread2(void *val);
};

#endif 
