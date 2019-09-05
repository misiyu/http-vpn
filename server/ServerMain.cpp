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

int main(){
    Socks5Server *ss_server=new Socks5Server();
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
