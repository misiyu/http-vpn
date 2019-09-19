#include "Client.h"
using namespace std;

// simulate socks5-client
// usage:
// 1. "g++ -c Client.cpp"
// 2. "g++ -c ClientMain.cpp"
// 3. "g++ -o client ClientMain.o Client.o"
// 4. "./client"
// 5. test:
// 		"curl -v --socks5 127.0.0.1:8888  https://www.baidu.com" => ok
// 		make firefox browser port 8888 => ok
//
void alert_usage(){
		cout << "Usage : ./client -s /aaa/nfd/vpn/server -c /aaa/nfd/vpn/client" << endl ;
		exit(0) ;
}

void parse_arg(string &server_prefix , string &m_prefix , int argc , char **argv){
	if(argc < 5){
		alert_usage() ;
	}
	string cmd1 = argv[1] ;
	string cmd2 = argv[3] ;
	if(cmd1 == "-s"){
		server_prefix = argv[2] ;
	}else if(cmd1 == "-c"){
		m_prefix = argv[2] ;
	}else{
		alert_usage() ;
	}
	if(cmd2 == "-s"){
		server_prefix = argv[4] ;
	}else if(cmd2 == "-c"){
		m_prefix = argv[4] ;
	}else{
		alert_usage() ;
	}
	if(server_prefix == "" || m_prefix == "") alert_usage() ;
}

int main(int argc , char **argv){

	string server_prefix = "/aaa/nfd/vpn/server" ;
	string client_prefix = "/aaa/nfd/vpn/client" ;

	parse_arg(server_prefix , client_prefix, argc, argv) ;

    Client *client=new Client(server_prefix,client_prefix);
    client->start();
    delete client;
}

// makefile:
// client: Client.o ClientMain.o  
// 	g++ -o client ClientMain.o Client.o
// Client.o : Client.cpp Client.h
// 	g++ -c Client.cpp
// ClientMain.o : ClientMain.cpp
// 	g++ -c ClientMain.cpp
