#include <iostream>
#include "ndn_socket.h"

#define BUFF_SZ 8000

using namespace std;


int main()
{
	Ndn_socket ndn_socket ;
	ndn_socket.listen("/localhost/nfd/send") ;
	ndn_socket.set_daddr("/localhost/nfd/recv") ;
	char buff[BUFF_SZ] ;
	for (int i = 0; i < 1; i++) {
		memset(buff, 'a'+i , BUFF_SZ) ;
		ndn_socket.write(buff , BUFF_SZ) ;
	}
	ndn_socket.close() ;
	cout << endl ;

	//sleep(9) ;
	return 0;
}
