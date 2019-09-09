// simulate socks5-client. coding by wefree in 2019.08
#include "Client.h"
using namespace std;

// listen to the local port
void Client::listenLocal(){
	this->sockfd=socket(AF_INET, SOCK_STREAM, 0);
	/* bind the local address, so that the cliend can send to server */
	bzero((char *)&(this->serv_addr), sizeof(this->serv_addr));
	(this->serv_addr).sin_family = AF_INET;
	(this->serv_addr).sin_addr.s_addr = htonl(INADDR_ANY);
	(this->serv_addr).sin_port = htons(this->port);	
	// bind
	while( bind(this->sockfd, (struct sockaddr *) &(this->serv_addr), sizeof(this->serv_addr)) < 0){
		sleep(1) ;
		cout << "bind fail" << endl ;
	};
	listen(this->sockfd,10);
};
// waiting for the local port's connection 
int Client::acceptLocal(){
	int newsockfd;
	struct sockaddr_in cli_addr;
	int cli_len=sizeof(cli_addr);
	cout<<"Waitting connection [client] ..."<<endl;
	newsockfd = accept(this->sockfd, (struct sockaddr *) &cli_addr, 
			(socklen_t *)&cli_len);
	// error
	while(newsockfd<0){
		cout<<"can't accept from local address"<<endl;
		close(this->sockfd);
		this->listenLocal();
		newsockfd = accept(this->sockfd, (struct sockaddr *) &cli_addr, 
				(socklen_t *)&cli_len);
	}
	cout<<"Acceptted a connection [client]"<<endl;

	return newsockfd;
};


// server start
void Client::start(){
	/*
	 *this->listenLocal();
	 *this->forever();
	 *close(this->sockfd);
	 */

	this->ndn_listen_local("/aaa/nfd/vpn/client") ;
	this->ndn_forever() ;
	close(this->sockfd) ;
};

void Client::ndn_listen_local(string prefix){
	this->listenLocal() ;
	this->listen_prefix = prefix ;
	this->mndn_socket.listen(prefix.data()) ;
}

void Client::ndn_connSockes5Server(Ndn_socket &ndn_socket , string new_prefix){
	cout << "new_prefix : " << new_prefix << endl ;
	ndn_socket.write(new_prefix) ;
	char daddr[1000] ;
	memset(daddr , 0 , 1000) ;
	ndn_socket.read(daddr,1000) ;
	ndn_socket.set_daddr(daddr) ;
}
void Client::ndn_transData(Ndn_socket &ndn_socket , int newsockfd){

	printf("Data transmission is beginning.\n");
	cout << "newsockfd's daddr :" << ndn_socket.get_daddr() << endl ;
	void* sockfds[2] = {&ndn_socket,&newsockfd};
	pthread_t ptid1 , ptid2 ;
	pthread_create(&ptid1 , NULL , ndn_thread1 , sockfds) ;
	pthread_create(&ptid2 , NULL , ndn_thread2 , sockfds) ;

	pthread_join(ptid1 , NULL) ;
	pthread_join(ptid2 , NULL) ;

	cout<<" child process 1 exit()"<<endl;
	// close the fd in the son process
	close(this->sockfd);
	// exit the son process
	exit(0);
}

string Client::get_newprefix(){
	return (listen_prefix+to_string(p_port_seq++)) ;
	if(listen_prefix.at(listen_prefix.size()-1) != '/'){
		return (listen_prefix+"/"+to_string(p_port_seq++)) ;
	}else{
		return (listen_prefix+to_string(p_port_seq++)) ;
	}
}
// forever listenner function
void Client::ndn_forever(){
	while(true){
		int newsockfd = this->acceptLocal();        
		string new_prefix = this->get_newprefix();

		pid_t fpid1;
		while ( ( fpid1 = fork() ) < 0 ){
			cout << "create child process 1 failled. Retry..." << endl;
		}
		if ( 0 == fpid1 ) { // child process
			Ndn_socket ndn_socket ;
			ndn_socket.listen(new_prefix.data()) ;
			ndn_socket.set_daddr(server_prefix.data()) ;
			cout << "create ndn_socket in child process" << endl ;

			ndn_connSockes5Server(ndn_socket, new_prefix) ;
			ndn_transData(ndn_socket, newsockfd) ;
		}else{
			// close the fds in main process
			close(newsockfd);
		}
	}
}

void *Client::ndn_thread1(void *val){
	
	Ndn_socket *ndn_socketp = *((Ndn_socket**)val) ;
	int remote_sockfd = **((int**)val + 1) ;
	char data[MAX_SIZE];
	int datalen;
	int send_len = 0 ;

	while(true){
		datalen=recv(remote_sockfd, data, sizeof(data), 0);
		cout << "recv from browser_sock datalen = " << datalen << endl ;
		if (datalen <= 0) {
			char closedata[10];
			string closedata_s="closesocks";
			strcpy(closedata,closedata_s.c_str());
			send_len = ndn_socketp->write(closedata,10) ;
			cout << "send to client_sockfd the 'closesocks', len = " << send_len << endl ;
			break;
		}else{
			cout << "before send to ndn_socket" << endl ;
			cout << "ndn_socketp->daddr = " << ndn_socketp->get_daddr() << endl ;
			send_len = ndn_socketp->write(data, datalen) ;
			cout << "send to ndn_socket len = " << send_len << endl ;
		}
	}
	close(remote_sockfd) ;
	return NULL;
}

// transfer data form socks5 server to local port
void *Client::ndn_thread2(void *val){

	Ndn_socket *ndn_socketp = *((Ndn_socket**)val) ;
	int remote_sockfd = **((int**)val + 1) ;
	char data[MAX_SIZE];
	int datalen;
	int send_len = 0 ;
	while(true){
		datalen = ndn_socketp->read(data , sizeof(data)) ;
		cout << "recv from ndn_socket datalen = " << datalen << endl ;
		if (datalen<= 0) {
			break ;
		}else if(datalen==10){
			string data_len10(data,data+10);
			if(data_len10=="closesocks"){
				break;
			}else{
				//sleep(10) ;
				send_len = send(remote_sockfd, data, datalen, 0) ;
				cout << "send to remote_sockfd len = " << send_len << endl ;
			}
		}else{  // send to remote server
			send_len = send(remote_sockfd, data, datalen, 0) ;
			cout << "send to browser_sockfd len = " << send_len << endl ;
		}		
	}
	ndn_socketp->close() ;
	close(remote_sockfd) ;
	return NULL;
}
