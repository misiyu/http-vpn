LFLAGS := -lpthread -lpcap -lndn-cxx -lboost_system -ljsoncpp

main: send recv

send : ndnp2p_send.o ndn_socket.o
	g++ -o send ndnp2p_send.o ndn_socket.o  $(LFLAGS)

recv : ndnp2p_recv.o ndn_socket.o
	g++ -o recv ndnp2p_recv.o ndn_socket.o  $(LFLAGS)

ndn_socket.o : ndn_socket.cpp ndn_socket.h
	g++ -c -std=c++14 ndn_socket.cpp 

ndnp2p_recv.o : ndnp2p_recv.cpp 
	g++ -c -std=c++14 ndnp2p_recv.cpp 
ndnp2p_send.o : ndnp2p_send.cpp 
	g++ -c -std=c++14 ndnp2p_send.cpp

clean :
	rm -rf *.o send recv
