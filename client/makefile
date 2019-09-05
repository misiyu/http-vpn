client: Client.o ClientMain.o  
	g++ -o client ClientMain.o Client.o
Client.o : Client.cpp Client.h
	g++ -c Client.cpp
ClientMain.o : ClientMain.cpp
	g++ -c ClientMain.cpp

server: Socks5Server.o ServerMain.o
	g++ -o server  ServerMain.o Socks5Server.o
Socks5Server.o : 
	g++ -c Socks5Server.cpp 
ServerMain.o :
	g++ -c ServerMain.cpp

clean :
	rm -rf *.o client server
