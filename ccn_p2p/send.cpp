#include <iostream>
#include "ndn_socket.h"
#define BUFF_SZ 8000
using namespace std;
int main()
{  // send
	Ndn_socket ndn_socket ;
	ndn_socket.listen("/localhost/nfd/send") ;
	ndn_socket.set_daddr("/localhost/nfd/recv") ;
	char buff[BUFF_SZ] ;
	memset(buff, 'a', BUFF_SZ) ;
	ndn_socket.close() ;
	cout << endl ;
	return 0;
}
