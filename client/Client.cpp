// simulate socks5-client. coding by wefree in 2019.08
#include "Client.h"
using namespace std;

// local port listenner thread
void* thread_fun1(void *val);
// remote server listenner thread
void* thread_fun2(void *val);

// listen to the local port
void Client::listenLocal(){
	this->sockfd=socket(AF_INET, SOCK_STREAM, 0);
	/* bind the local address, so that the cliend can send to server */
	bzero((char *)&(this->serv_addr), sizeof(this->serv_addr));
	(this->serv_addr).sin_family = AF_INET;
	(this->serv_addr).sin_addr.s_addr = htonl(INADDR_ANY);
	(this->serv_addr).sin_port = htons(this->port);	
	// bind
	while( bind(this->sockfd, (struct sockaddr *) &(this->serv_addr),
				sizeof(this->serv_addr)) < 0){
		sleep(1) ;
		cout << "bind fail" << endl ;
	};
	listen(this->sockfd,100);
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

// data transmission shake
void Client::transData(pair<int,int> pairval){
	int newsockfd = pairval.first;
	int remote_sockfd = pairval.second;
	printf("Data transmission is beginning.\n");

	int sockfds[2] = {newsockfd,remote_sockfd};

	pid_t fpid1;
	while ( ( fpid1 = fork() ) < 0 ){
		cout << "create child process 1 failled. Retry..." << endl;
	}
	if ( 0 == fpid1 ) {
		cout<<" child process 1 beginning: "<<endl;
		thread_fun1(sockfds);
		cout<<" child process 1 exit()"<<endl;
		// close the fd in the son process
		close(this->sockfd);
		// exit the son process
		exit(0);
	}

	pid_t fpid2;
	while ( ( fpid2 = fork() ) < 0 ){
		cout << "create child process 2 failled. Retry..." << endl;
	}	
	if ( 0 == fpid2 ) {
		cout<<" child process 2 beginning: "<<endl;
		thread_fun2(sockfds);
		cout<<" child process 2 exit()"<<endl;
		// close the fd in the son process
		close(this->sockfd);
		// exit the son process
		exit(0);
	}	
};

int Client::connSockes5Server(){
	//Connect to remote destination server
	struct sockaddr_in remote_addr;
	int socks5_sockfd; 
	socks5_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero((char *)&remote_addr, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr=inet_addr(SOCKS5_SERVER_IP);
	remote_addr.sin_port=htons(SOCKS5_SERVER_PORT);
	cout<<"connecting to socks5server..."<<endl;
	int res_c=connect(socks5_sockfd, (struct sockaddr *)&remote_addr, 
			sizeof(struct sockaddr));
	while(res_c<0){
		cout<<"can not connect to the socks server...."<<endl;
		sleep(1);       
		res_c=connect(socks5_sockfd, (struct sockaddr *)&remote_addr, 
				sizeof(struct sockaddr)); 
	}
	cout<<"connected socks5server"<<endl;
	printf("Connected to %s:%d\n",inet_ntoa(remote_addr.sin_addr), 
			remote_addr.sin_port);
	return socks5_sockfd;
}

// forever listenner function
void Client::forever(){
	while(true){
		int newsockfd = this->acceptLocal();        
		int socks5_sockfd=this->connSockes5Server() ; 
		this->transData(make_pair(newsockfd,socks5_sockfd));
		// close the fds in main process
		close(newsockfd);
		close(socks5_sockfd);
	}
};

// server start
void Client::start(){
	this->listenLocal();
	this->forever();
	close(this->sockfd);
};

// transfer data form local port to socks5 server
void * thread_fun1(void *val){
	//pair<int,int> pairval=*((pair<int,int>*)val);
	int local_sockfd = ((int*)val)[0];
	int socks_sockfd = ((int*)val)[1];

	char data[MAX_SIZE];
	int datalen;
	int send_len = 0 ;
	while(true){
		datalen=recv(local_sockfd, data, sizeof(data), 0);
		cout << "recv from local_sock datalen = " << datalen << endl ;
		if (datalen<= 0) {
			// 发送一个特殊数据到代理服务器，提醒其关闭此次连接=>也可以设置心跳包来实现
			// 由于这里正常情况下传输的是http格式数据，因此这种特殊数据不会跟不同数据混同
			char closedata[10];
			string closedata_s="closesocks";
			strcpy(closedata,closedata_s.c_str());
			send_len = send(socks_sockfd, closedata, 10, 0) ;
			cout << "send to socks_sockfd the 'closesocks', len = " << send_len << endl ;
			break ;
		}else{  // send to socks5 server
			send_len = send(socks_sockfd, data, datalen, 0) ;
			cout << "send to socks_sockfd len = " << send_len << endl ;
		}		
	}
	close(local_sockfd) ;
	close(socks_sockfd) ;
	return NULL;
}

// transfer data form socks5 server to local port
void * thread_fun2(void *val){
	//pair<int,int> pairval=*((pair<int,int>*)val);
	int local_sockfd = ((int*)val)[0];
	int socks_sockfd = ((int*)val)[1];
	char data[MAX_SIZE];
	int datalen;
	int send_len = 0 ;
	while(true){
		datalen=recv(socks_sockfd, data, sizeof(data), 0);
		cout << "recv from socks_sock datalen = " << datalen << endl ;
		if (datalen <= 0) {
			//nothing to do
			break;
		}else if(datalen==10){
			// cout<<"data[10]: "<<data<<endl;
			string data_len10(data,data+10);
			if(data_len10=="closesocks"){
				break;
			}else{
				// send to localhost
				send_len = send(local_sockfd, data, datalen, 0) ;
				cout << "send to local_sockfd len = " << send_len << endl ;	
			}
		}else{
			// send to localhost
			send_len = send(local_sockfd, data, datalen, 0) ;
			cout << "send to local_sockfd len = " << send_len << endl ;
		}
	}
	close(local_sockfd) ;
	close(socks_sockfd) ;
	return NULL;
}
