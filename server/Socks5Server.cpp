// simulate socks5-server. coding by wefree in 2019.08
#include "Socks5Server.h"
using namespace std;

// local port listenner thread
void* thread_fun1(void *val);
// remote server listenner thread
void* thread_fun2(void *val);

// listen to the local port
void Socks5Server::listenLocal(){
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
	listen(this->sockfd,10);
};

// waiting for the local port's connection 
int Socks5Server::acceptLocal(){
	int newsockfd;
	struct sockaddr_in cli_addr;
	int cli_len=sizeof(cli_addr);
	cout<<"Waitting connection [socks5 server] ..."<<endl;
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
	cout<<"Acceptted a connection [socks5 server]"<<endl;

	return newsockfd;
};

// a headshake for the sub-negotiation method
int Socks5Server::negotShake(int val){
	// right flag, the loop is going on when th flag==0
	int flag=0;
	// newsockfd
	int newsockfd = val;
	//Get 2 bytes of information from the client 
	// - socks version number, total number of negotiation methods
	char buf1[1];
	char buf2[2];
	if (recv(newsockfd, buf2, 2, 0) == -1) {
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
		recv(newsockfd, buf1, 1, 0);
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
	if (send(newsockfd, msg2, 2, 0) == -1) {
		cout<<"cannot send socks-methond-reply"<<endl;
		flag=1;
		return flag;
	}

	return flag;
};

// a headshake for the connection information 
// 		to remote server
pair<int,int> Socks5Server::connShake(int val){
	// right flag, the loop is going on when th flag==0
	int flag=0;
	// newsockfd
	int newsockfd = val;
	//Get the first four bytes of information
	//  such as the address type of URL request 
	// from the client
	char buf4[4];
	if (recv(newsockfd, buf4, 4, 0) == -1) {
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
		if (recv(newsockfd, r_address, 4, 0) == -1) {
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
	if (recv(newsockfd, r_port, 2, 0) == -1) {
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
			if (send(newsockfd, connRrt, 10, 0) == -1) {
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
		if (send(newsockfd, err, 6, 0) == -1) {
			printf("cannot send url-err-reply\n");
			flag=1;
			return make_pair(remote_sockfd,flag);
		}
	}
	return make_pair(remote_sockfd,flag);

};

// data transmission shake
void Socks5Server::transData(pair<int,int> pairval){
	int newsockfd = pairval.first;
	int remote_sockfd = pairval.second;
	printf("Data transmission is beginning.\n");

	int sockfds[2] = {newsockfd,remote_sockfd};

	pid_t fpid1;
	while ( ( fpid1 = fork() ) < 0 ){
		cout << "create child process 1 failled. Retry..." << endl;
	}
	if ( 0 == fpid1 ) {
		cout<<"child process 1 beginning: "<<endl;
		thread_fun1(sockfds);
		cout<<"child process 1 exit()"<<endl;
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
		cout<<"child process 2 beginning: "<<endl;
		thread_fun2(sockfds);
		cout<<"child process 2 exit()"<<endl;
		// close the fd in the son process
		close(this->sockfd);
		// exit the son process
		exit(0);
	}	
};

// forever listenner function
void Socks5Server::forever(){
	while(true){
		int newsockfd = this->acceptLocal();
		if(this->negotShake(newsockfd)>0){
			cout<<"negotShake error"<<endl;
			close(newsockfd);
			continue;
		} 
		pair<int,int> pairval=connShake(newsockfd);
		if(pairval.second>0){
			cout<<"connShake error"<<endl;
			close(newsockfd);
			if(pairval.first>0){
				close(pairval.first);
			}
			continue;
		}
		int remote_sockfd = pairval.first;
		this->transData(make_pair(newsockfd,remote_sockfd));
		// close the fds in main process
		close(newsockfd);
		close(remote_sockfd);
	}
};

// server start
void Socks5Server::start(){
	this->listenLocal();
	this->forever();
	close(this->sockfd);
};

// transfer data form local port to remote server
void * thread_fun1(void *val){
	//pair<int,int> pairval=*((pair<int,int>*)val);
	int local_sockfd = ((int*)val)[0];
	int remote_sockfd = ((int*)val)[1];

	// cout << local_sockfd << " ==== " << remote_sockfd << endl;

	char data[MAX_SIZE];
	int datalen;
	int send_len = 0 ;
	while(true){
		datalen=recv(local_sockfd, data, sizeof(data), 0);
		cout << "recv from local_sock datalen = " << datalen << endl ;
		if (datalen<= 0) {
			// nothing to do
			break ;
		}else if(datalen==10){
			// cout<<"data[10]: "<<data<<endl;
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
	close(local_sockfd) ;
	close(remote_sockfd) ;
	return NULL;
}

// transfer data form remote server to local port
void * thread_fun2(void *val){
	//pair<int,int> pairval=*((pair<int,int>*)val);
	int local_sockfd = ((int*)val)[0];
	int remote_sockfd = ((int*)val)[1];
	char data[MAX_SIZE];
	int datalen;
	int send_len = 0 ;
	while(true){
		datalen=recv(remote_sockfd, data, sizeof(data), 0);
		cout << "recv from remote_sock datalen = " << datalen << endl ;
		if (datalen <= 0) {
			// 发送一个特殊数据到客户端，提醒其关闭此次连接=>也可以设置心跳包来实现
			// 由于这里正常情况下传输的是http格式数据，因此这种特殊数据不会跟不同数据混同
			char closedata[10];
			string closedata_s="closesocks";
			strcpy(closedata,closedata_s.c_str());
			send_len = send(local_sockfd, closedata, 10, 0) ;
			cout << "send to client_sockfd the 'closesocks', len = " << send_len << endl ;
			break;
		}else{
			// send to localhost
			send_len = send(local_sockfd, data, datalen, 0) ;
			cout << "send to local_sockfd len = " << send_len << endl ;
		}
	}
	close(local_sockfd) ;
	close(remote_sockfd) ;
	return NULL;
}
