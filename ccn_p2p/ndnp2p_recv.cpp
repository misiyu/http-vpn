#include <iostream>
#include "ndn_socket.h"

using namespace std;

void *ndn_socket(void *parm){
	//Ndn_socket *ndn_socket = (Ndn_socket)
}

int main()
{
	Ndn_socket ndn_socket ;
	ndn_socket.listen("/localhost/nfd/recv") ;
	char buff[5000] ;
	memset(buff, 0 , 5000) ;
	int read_n = ndn_socket.read(buff) ;
	cout << endl ;
	cout << buff << endl ;
	cout << "recv len = " << read_n << endl ;


	return 0;
}
