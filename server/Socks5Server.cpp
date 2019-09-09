// simulate socks5-server. coding by wefree in 2019.08
#include "Socks5Server.h"
using namespace std;
// server start
void Socks5Server::start(){
	//this->ndn_listen_local("/localhost/nfd/vpn/server") ;
	this->ndn_listen_local("/aaa/nfd/vpn/server") ;
	this->ndn_forever() ;
	mndn_socket.close() ;
};

void Socks5Server::ndn_listen_local(string prefix){
	this->listen_prefix= prefix ;
	mndn_socket.listen(prefix.data()) ;
}

string Socks5Server::ndn_accept_local(string new_prefix) {
	char buff[1000] ;
	memset(buff,0,1000) ;
	int read_n = mndn_socket.read(buff,1000) ;
	cout << "recv_n = " << read_n << endl ;
	cout << buff << endl ;

	string client_prefix = buff ;
	cout << "client_prefix : " << client_prefix << endl ;
	
	mndn_socket.set_daddr(buff) ;

	mndn_socket.write(new_prefix) ;
	//mndn_socket.
	return client_prefix ;
}

int Socks5Server::ndn_negotShake(Ndn_socket &ndn_socket){
	int flag=0;
	//Get 2 bytes of information from the client 
	// - socks version number, total number of negotiation methods
	char buf1[1];
	char buf2[2];
	cout << "ndn_negotShake before read" << endl ;
	if (ndn_socket.read(buf2 , 2) < 0) {
		cout<<"cannot get version-nmethod"<<endl;
		flag=1;
		return flag;
	}	
	if ((uint8_t)buf2[0] != this->socks_vision) {
		cout<<"socks version is wrong"<<endl;
		flag=1;
		return flag;
	}

	if ((uint8_t)buf2[1] <= 0) {
		cout<<"nmethod is wrong"<<endl;
		flag=1;
		return flag;
	}
	cout<<"version-nmethod: ";
	printf("%x ", buf2[0]);
	printf("%x \n", buf2[1]);

	//negotiation methods
	int nmethods = (uint8_t)buf2[1];
	// the method contains 0 when the m_flag==1
	int m_flag = 0;	
	for (int i = 0; i < nmethods; i++) {
		ndn_socket.read(buf1,1) ;
		if ((uint8_t)(buf1[0]) == 0) {
			m_flag = 1;
		}
		cout<<"nmethods: ";
		printf("%x\n", buf1[0]);
	}
	if (m_flag == 0) {
		cout<<"methods donot have 0"<<endl;
		flag=1;
		return flag;
	}

	//Return request reply (no sub-negotiation required)
	uint8_t msg2[2];
	msg2[0] = this->socks_vision;
	msg2[1] = this->ui0;
	if (ndn_socket.write(msg2,2) < 0) {
		cout<<"cannot send socks-methond-reply"<<endl;
		flag=1;
		return flag;
	}

	return flag;
}

pair<int,int> Socks5Server::ndn_connShake(Ndn_socket &ndn_socket){
	// right flag, the loop is going on when th flag==0
	int flag=0;
	char buf4[4];
	if (ndn_socket.read(buf4,4) < 0 ) {
		cout<<"cannot get version-cmd-_-addressType"<<endl;
		flag=1;
		return make_pair(-1,flag);
	}
	cout<<"version-cmd-_-addressType: ";
	printf("%x ", buf4[0]);
	printf("%x ", buf4[1]);
	printf("%x \n", buf4[3]);
	if ((uint8_t)buf4[0] != 5) {
		cout<<"version is wrong. version-cmd-_-addressType: ";
		printf("%x ", buf4[0]);
		printf("%x ", buf4[1]);
		printf("%x \n", buf4[3]);
		flag=1;
		return make_pair(-1,flag);
	}
	uint8_t cmd = (uint8_t)buf4[1];
	if(cmd!=1){
		cout<<"cmd is not 1"<<endl;
		flag=1;
		return make_pair(-1,flag);
	}
	uint8_t addressType = (uint8_t)buf4[3];
	if(addressType!=1){
		cout<<"address is not ipv4"<<endl;
		flag=1;
		return make_pair(-1,flag);
	}

	//Get the IP address of the destination server of URL request from the client
	char* r_address;
	char* domin_address;
	int remote_addr_length=4;
	if (addressType == 1) {//ipv4
		remote_addr_length=4;
		r_address=new char[4];
		if (ndn_socket.read(r_address,4) < 0) {
			cout<<"cannot get the ipv4\n"<<endl;
			flag=1;
			return make_pair(-1,flag);
		}
		printf("ipv4: %d %d",(uint8_t)r_address[0],(uint8_t)r_address[1]);
		printf(" %d %d\n",(uint8_t)r_address[2],(uint8_t)r_address[3]);
	}else{
		cout<<"address is not ipv4"<<endl;
		flag=1;
		return make_pair(-1,flag);
	}

	//Listener port of destination server for URL request from client
	char r_port[2];
	//if (recv(newsockfd, r_port, 2, 0) == -1) {
	if (ndn_socket.read(r_port,2) < 0) {
		cout<<"cannot get the port\n"<<endl;
		flag=1;
		return make_pair(-1,flag);
	}
	printf("port: %d %d\n", (uint8_t)r_port[0],(uint8_t)r_port[1]);

	//When the connection to the remote server is successful, 
	// remote server
	int remote_sockfd;
	try
	{
		if (cmd == 1) {
			if (addressType == 1){//ipv4
				//Connect to remote destination server
				struct sockaddr_in remote_addr;
				remote_sockfd = socket(AF_INET, SOCK_STREAM, 0);
				bzero((char *)&remote_addr, sizeof(remote_addr));
				remote_addr.sin_family = AF_INET;
				memcpy(&(remote_addr.sin_addr.s_addr),r_address,remote_addr_length);
				memcpy(&(remote_addr.sin_port),r_port,2);
				cout<<"connecting to ipv4..."<<endl;
				int res_c=connect(remote_sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
				if(res_c==-1){
					cout<<"connect error"<<endl;
					flag=1;
					return make_pair(remote_sockfd,flag);
				}else{
					cout<<"connected ipv6"<<endl;
					printf("Connected to %s:%d\n",inet_ntoa(remote_addr.sin_addr),remote_addr.sin_port);
				}
			}else{
				//...
			}	

		}
		else {
			printf("cmd is not connection\n");
			flag=1;
			return make_pair(remote_sockfd,flag);
		}

		//Return connection information to client
		uint8_t *connRrt;
		if(addressType==1){//ipv4
			//getsocketname()
			sockaddr_in cs_addr;
			memset(&cs_addr,0,sizeof(cs_addr));
			int cs_len=sizeof(cs_addr);
			int res_sn=getsockname(remote_sockfd,(struct sockaddr *)&cs_addr,(socklen_t *)&cs_len);
			cout<<"Current connection:"<<inet_ntoa(cs_addr.sin_addr)<<":"<<ntohs(cs_addr.sin_port)<<endl;
			// return info
			connRrt=new uint8_t[10];
			connRrt[0]=socks_vision;
			connRrt[1]=ui0;
			connRrt[2]=ui0;
			connRrt[3]=addressType;
			memcpy(&connRrt[4],&(cs_addr.sin_addr.s_addr),remote_addr_length);
			memcpy(&connRrt[8],&(cs_addr.sin_port),2);
			cout<<"the reply ip:"<<endl;
			for(int i=0;i<4;i++){
				printf("%d ",(uint8_t)connRrt[4+i]);	
			}
			cout<<endl;
			cout<<"the reply port:"<<endl;
			for(int i=0;i<2;i++){
				printf("%d ",(uint8_t)connRrt[8+i]);	
			}
			cout<<endl;
			if (ndn_socket.write(connRrt,10) < 0) {
				printf("cannot send url-reply\n");
				flag=1;
				return make_pair(remote_sockfd,flag);
			}
		}else{}

	}
	catch (exception& e)
	{
		uint8_t err[6];
		err[0] = socks_vision;
		err[1] = ui5;
		err[2] = ui0;
		err[3] = addressType;
		err[4] = ui0;
		err[5] = ui0;
		//if (send(newsockfd, err, 6, 0) == -1) {
		if (ndn_socket.write(err,6) < 0) {
			printf("cannot send url-err-reply\n");
			flag=1;
			return make_pair(remote_sockfd,flag);
		}
	}
	return make_pair(remote_sockfd,flag);

};

void Socks5Server::ndn_transData(string new_prefix , string client_prefix){

	printf("Data transmission is beginning.\n");


	pid_t fpid1;
	while ( ( fpid1 = fork() ) < 0 ){
		cout << "create child process 1 failled. Retry..." << endl;
	}
	if ( 0 == fpid1 ) {
		cout<<"child process 1 beginning: "<<endl;
		Ndn_socket ndn_socket ;
		ndn_socket.listen(new_prefix.data()) ;
		ndn_socket.set_daddr(client_prefix.data()) ;

		if(this->ndn_negotShake(ndn_socket)>0){
			cout<<"negotShake error"<<endl;
			ndn_socket.close() ;
			exit(0) ;
		} 
		pair<int,int> pairval= ndn_connShake(ndn_socket) ;
		if(pairval.second>0){
			cout<<"connShake error"<<endl;
			if(pairval.first>0){
				close(pairval.first);
			}
			ndn_socket.close() ;
			exit(0);
		}
		int remote_sockfd = pairval.first;

		void *sockfds[2] = {&ndn_socket,&remote_sockfd};
		pthread_t ptid1, ptid2 ;
		pthread_create(&ptid1 , NULL , ndn_thread1 , sockfds) ;
		pthread_create(&ptid2 , NULL , ndn_thread2 , sockfds) ;

		pthread_join(ptid1,NULL) ;
		pthread_join(ptid2,NULL) ;
		cout<<"child process 1 exit()"<<endl;
		// close the fd in the son process
		close(this->sockfd);
		// exit the son process
		exit(0);
	}
};

void Socks5Server::ndn_forever(){
	while(true){
		string new_prefix = this->listen_prefix+ to_string(p_port_seq++) ;
		string client_prefix = this->ndn_accept_local(new_prefix) ;

		ndn_transData(new_prefix,  client_prefix ) ;
		// close the fds in main process
	}
};
void *Socks5Server::ndn_thread1(void *val){
	Ndn_socket *ndn_socketp = *((Ndn_socket**)val) ;
	int remote_sockfd = **((int**)val + 1) ;
	char data[MAX_SIZE];
	int datalen;
	int send_len = 0 ;
	while(true){
		cout << "before recv" << endl; 
		datalen = ndn_socketp->read(data , sizeof(data)) ;
		cout << "recv from ndn_socket datalen = " << datalen << endl ;
		if (datalen<= 0) {
			break ;
		}else if(datalen==10){
			string data_len10(data,data+10);
			if(data_len10=="closesocks"){
				break;
			}else{
				send_len = send(remote_sockfd, data, datalen, 0) ;
				cout << "send to remote_sockfd len = " << send_len << endl ;
			}
		}else{  // send to remote server
			send_len = send(remote_sockfd, data, datalen, 0) ;
			cout << "send to remote_sockfd len = " << send_len << endl ;
		}		
	}
	ndn_socketp->close() ;
	close(remote_sockfd) ;
	return NULL;
}

void *Socks5Server::ndn_thread2(void *val){
	Ndn_socket *ndn_socketp = *((Ndn_socket**)val) ;
	int remote_sockfd = **((int**)val + 1) ;
	char data[MAX_SIZE];
	int datalen;
	int send_len = 0 ;
	while(true){
		cout << "before recv" << endl; 
		datalen=recv(remote_sockfd, data, sizeof(data), 0);
		cout << "recv from remote_sock datalen = " << datalen << endl ;
		if (datalen <= 0) {
			char closedata[10];
			string closedata_s="closesocks";
			strcpy(closedata,closedata_s.c_str());
			send_len = ndn_socketp->write(closedata,10) ;
			cout << "send to client_sockfd the 'closesocks', len = " << send_len << endl ;
			break;
		}else{
			send_len = ndn_socketp->write(data, datalen) ;
			cout << "send to local_sockfd len = " << send_len << endl ;
		}
	}
	close(remote_sockfd) ;
	return NULL;
}
