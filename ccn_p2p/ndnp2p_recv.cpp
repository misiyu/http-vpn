#include <iostream>
#include "ndn_socket.h"

#define BUFF_SZ

using namespace std;

void *ndn_socket(void *parm){
	//Ndn_socket *ndn_socket = (Ndn_socket)
}

int main()
{
	Ndn_socket ndn_socket ;
	ndn_socket.listen("/localhost/nfd/recv") ;
	ndn_socket.listen("/localhost/nfd/send") ;
	char buff[8000] ;
	int read_n = 0 ;
	int recv_count = 0 ;
	while((read_n = ndn_socket.read(buff,8000)) > 0){
		cout << "count = " << ++recv_count << endl ;
		cout << "buff[0] = " << buff[0] << endl ;
		cout << "recv len = " << read_n << endl ;
	}
	cout << endl ;
	cout << buff << endl ;


	return 0;
}
