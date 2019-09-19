#include "Socks5Server.h"
using namespace std;

// simulate socks5-server
// usage:
// 1. "g++ -c Socks5Server.cpp"
// 2. "g++ -c ServerMain.cpp"
// 3. "g++ -o server  ServerMain.o Socks5Server.o"
// 4. "./server"
// 5. test:
// 		"curl -v --socks5 127.0.0.1:9011  https://www.baidu.com" => ok
// 		make firefox browser port 9011 => ok

void alert_usage(){
		cout << "Usage : ./server -s /aaa/nfd/vpn/server " << endl ;
		exit(0) ;
}

void parse_arg(string &server_prefix , int argc , char **argv){
	if(argc < 3){
		alert_usage() ;
	}
	string cmd1 = argv[1] ;
	if(cmd1 == "-s"){
		server_prefix = argv[2] ;
	}else{
		alert_usage() ;
	}
	if(server_prefix == "" ) alert_usage() ;
}

int main(int argc , char **argv){
	string server_prefix ;
	parse_arg(server_prefix , argc , argv) ;
    Socks5Server *ss_server=new Socks5Server(server_prefix);
    ss_server->start();
    delete ss_server;
}


// makefile:
// server : Socks5Server.o ServerMain.o  
// 	g++ -o server  ServerMain.o Socks5Server.o
// Socks5Server.o : Socks5Server.cpp Socks5Server.h
// 	g++ -c Socks5Server.cpp
// ServerMain.o : ServerMain.cpp
// 	g++ -c ServerMain.cpp
