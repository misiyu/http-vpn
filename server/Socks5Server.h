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

#define SERV_TCP_PORT 9011 /* Local listening port number */
#define MAX_SIZE 1024*7120
#define SOCKS5_VERSION 5

using namespace std;

class Socks5Server{
	private:
		// sockfd
		int sockfd;
		// sockaddr
		struct sockaddr_in serv_addr;
		/* Local listening port number */
		int port;
		// socks version
		uint8_t socks_vision;
		// 2 commom numbers
		uint8_t ui0;
		uint8_t ui5;

		Ndn_socket mndn_socket ;
		string listen_prefix ;
		unsigned int p_port_seq ;

	public:
		// construction function
		Socks5Server(){
			this->port=SERV_TCP_PORT;
			this->ui0=0;
			this->ui5=5;
			this->socks_vision=SOCKS5_VERSION;
			p_port_seq = 0 ;
		}
		Socks5Server(int port){
			this->port=port;
			this->ui0=0;
			this->ui5=5;
			this->socks_vision=SOCKS5_VERSION;
		}
		~Socks5Server(){
		}
		// listen to the local port
		void listenLocal();
		// waiting for the local port's connection 
		int acceptLocal();
		// a headshake for the sub-negotiation method
		int negotShake(int val);
		// a headshake for the connection information 
		// 		to remote server
		pair<int,int> connShake(int val);
		// data transmission shake
		void transData(pair<int,int> pairval);
		// forever listenner function
		void forever();
		// server start
		void start();

		// ndn 监听线程
		void ndn_listen_local(string prefix) ;
		Ndn_socket* ndn_accept_local() ;
		int ndn_negotShake(Ndn_socket &ndn_socket);
		pair<int,int> ndn_connShake(Ndn_socket &ndn_socket);
		void ndn_transData(Ndn_socket &ndn_socket , int remote_sockfd);
		void *ndn_thread1(void *val);
		void *ndn_thread2(void *val);
		void ndn_forever();
};

#endif 
