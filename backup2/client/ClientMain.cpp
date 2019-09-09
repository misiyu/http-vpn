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

int main(){
    Client *client=new Client();
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