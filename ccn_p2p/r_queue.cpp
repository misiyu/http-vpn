#include <iostream>
#include "r_queue.h"

using namespace std;
R_Queue::R_Queue(){
	this->head = 0 ;
	this->rear = 0 ;
	pthread_cond_init(&has_data , NULL) ;
	pthread_cond_init(&has_space , NULL) ;
	pthread_mutex_init(&m_mutex , NULL) ;
}
R_Queue::~R_Queue(){

}
int R_Queue::get_head(){
	int ret = 0 ;
	pthread_mutex_lock(&m_mutex) ;
	ret = head ;
	pthread_mutex_unlock(&m_mutex) ;
	return head ;
}
int R_Queue::get_rear(){
	return rear ;
}
bool R_Queue::is_full(){
	return ((this->rear+1)%QUEUE_SZ == this->head) ;
}
bool R_Queue::is_empty(){
	bool ret = false ;
	pthread_mutex_lock(&m_mutex) ;
	ret = (this->head == this->rear) ;
	pthread_mutex_unlock(&m_mutex) ;
	return ret ;
}
int R_Queue::add_n(int n){
	bool need_signal = false ;
	pthread_mutex_lock(&m_mutex) ;
	need_signal = (head == rear) ;
	rear = (rear + n) % QUEUE_SZ ;
	pthread_mutex_unlock(&m_mutex) ;
	if(need_signal) pthread_cond_signal(&has_data) ;
}
int R_Queue::rmv_n(int n){
	bool need_signal = false ;
	pthread_mutex_lock(&m_mutex) ;
	need_signal = ((rear+1)%QUEUE_SZ == head) ;
	head = (head + n) % QUEUE_SZ ;
	pthread_mutex_unlock(&m_mutex) ;
	if(need_signal) pthread_cond_signal(&has_space) ;
}
char *R_Queue::get_head_p(){
	return &(buff[this->head]);
}
char *R_Queue::get_rear_p(){
	return &(buff[this->rear]) ;
}
// 得到所有数据的长度
int R_Queue::get_data_len(){
	int ret =  0 ;
	pthread_mutex_lock(&m_mutex) ;
	ret = (rear - head + QUEUE_SZ)%QUEUE_SZ ;
	pthread_mutex_unlock(&m_mutex) ;
	return ret ;
}
// 得到空余长度
int R_Queue::get_free_space(){
	return (head-rear-1+QUEUE_SZ)%QUEUE_SZ ;
}
// get the length of a continuous free space
// 获得从rear开始往后的连续剩余空间的长度 
int R_Queue::get_cfree_space(){
	int space = 0 ;
	if(head <= rear ) {
		space = QUEUE_SZ - rear ;
		if(head == 0) space -= 1 ;
	}else{
		space = head - rear - 1 ;
	}
	return space ;
}
// begin at head , get the continuous data block len in ring queue
int R_Queue::get_cdata_len(){
	int len = 0;
	if(rear < head){
		len = QUEUE_SZ - head ;
	}else{
		len = rear-head ;
	}
	return len ;
}

// copy n Byte data from buff to data 
// 从start开始，往后取n字节数据
int R_Queue::get_ndata(int start , char *data ,int n ){
	//printf("&&&&&&&&&&&& = start = %d , n = %d \n",start , n );
	if(start + n >= QUEUE_SZ){
		memcpy(data,&buff[start],QUEUE_SZ-start);
		memcpy(&data[QUEUE_SZ-start],buff,n+start-QUEUE_SZ);
	}else{
		memcpy(data,&buff[start],n);
	}
}
// 队列中加入n字节数据
int R_Queue::push_ndata(char *data , int n){
	//if(get_free_space() < n) return -1 ;	
	while(get_free_space() < n ) ;
	int clen ;
	while(n > 0){
		clen = get_cfree_space() ;
		if(clen > n) clen = n;
		memcpy(&buff[rear],data,clen);
		add_n(clen);
		n -= clen ;
	}
}

// 等待数据，若队列为空，当前进程等待在 has_data变量下
void R_Queue::wait4data() {
	pthread_mutex_lock(&m_mutex) ;
	while(head == rear) {
		pthread_cond_wait(&has_data , &m_mutex) ;
	}
	pthread_mutex_unlock(&m_mutex) ;
}
void R_Queue::wait4space() {
	pthread_mutex_lock(&m_mutex) ;
	while( (rear+1)%QUEUE_SZ == head) {
		pthread_cond_wait(&has_space , &m_mutex) ;
	}
	pthread_mutex_unlock(&m_mutex) ;
}
