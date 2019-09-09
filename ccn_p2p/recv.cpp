#include <iostream>
#include "ndn_socket.h"

#define BUFF_SZ

using namespace std;

void *ndn_socket(void *parm){
	//Ndn_socket *ndn_socket = (Ndn_socket)
}

int main()
{  // receive
	Ndn_socket ndn_socket ;
	ndn_socket.listen("/localhost/nfd/recv") ;
	char buff[8000] ;
	ndn_socket.read(buff,8000) ;
	cout << buff << endl ;
	return 0;
}
