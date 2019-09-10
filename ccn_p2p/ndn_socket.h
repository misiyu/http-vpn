#ifndef _NDN_SOCKET_H_
#define _NDN_SOCKET_H_
// 功能 ： 启动一个face，并往固定的前缀发送数据
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <ndn-cxx/face.hpp>
#include <errno.h>
#include <exception>
#include "r_queue.h"
#include <sys/time.h>

using std::string ;
using ndn::Face ;
using ndn::KeyChain ;
using ndn::Interest ;
using ndn::InterestFilter ;
using ndn::Data ;
using ndn::Name ;
using ndn::lp::Nack ;

class Ndn_socket
{
public:
	Ndn_socket();
	~Ndn_socket();
	int listen(const char * prefix) ;   // 监听一个前缀
	int set_daddr(const char * prefix) ;   // 设置一个目标地址
	string get_daddr() ;
	int write(const string & data) ;	// 发送数据
	int write(const char * data , int len) ;
	int write(const char * data , int len , string dname_base) ;
	int write(const uint8_t * data , int len) ;
	int write(const uint8_t * data , int len , string dname_base) ;
	//int read(char *data ) ;			// 接收数据
	int read(char *data , int buf_sz) ;
	int close() ;	// 关闭线程-processEvent
private:
	void onInterest(const InterestFilter& filter, const Interest& interest) ;
	void onData(const Interest& interest , const Data& data);
	void onData_pre(const Interest& interest , const Data& data);
	void onNack(const Interest& interest, const Nack& nack);
	void onNack_pre(const Interest& interest, const Nack& nack);
	void onTimeout(const Interest& interest) ;
	void onTimeout_pre(const Interest& interest) ;
	void onRegisterFailed(const Name& prefix, const std::string& reason) ;
	static void *run(void *param) ;

private:
	/* data */
	pthread_t m_tid ;
	char *send_data ;   // 要发送的数据
	int data_len ;		// 发送的数据
	int recv_n ;		// 接收到的数据长度
	int send_n ;		// 发送的数据长度
	string daddr ;		// 推送数据到的目的地址
	string maddr ;		// 本机的地址
	char *read_buf ;	// 接收到的数据暂存的缓冲区
	pthread_mutex_t recv_mutex ;
	pthread_cond_t has_recv ;
	unsigned int seq ;
	R_Queue r_queue ;
	string start_ts ;

	bool state ;
	

	Face m_face ;
	KeyChain m_keyChain ;

};

#endif 
