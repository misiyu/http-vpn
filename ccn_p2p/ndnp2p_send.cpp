#include <iostream>
#include "ndn_socket.h"

using namespace std;

int main()
{
	Ndn_socket ndn_socket ;
	ndn_socket.listen("/localhost/nfd/send") ;
	ndn_socket.set_daddr("/localhost/nfd/recv") ;
	string buff = "hello i am a student\n" ;
	ndn_socket.write(buff.data() , buff.size()) ;
	ndn_socket.close() ;

	//sleep(9) ;
	return 0;
}
