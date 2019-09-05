#include <iostream>
#include "ndn_socket.h"

using namespace std;
using namespace ndn;

Ndn_socket::Ndn_socket(){
	this->send_n = 0 ;
	this->recv_n = 0 ;
	this->seq = 0 ;
	pthread_cond_init(&has_recv , NULL) ;
	pthread_mutex_init(&recv_mutex , NULL) ;

	pthread_create(&(this->m_tid) , NULL , run, (void*)&m_face) ;
}

Ndn_socket::~Ndn_socket(){

}

int Ndn_socket::listen(const char *prefix){
	this->maddr = prefix ;
	//cout << "set filter " << prefix << endl ;
	m_face.setInterestFilter(prefix, 
			bind(&Ndn_socket::onInterest, this, _1, _2) ,
			RegisterPrefixSuccessCallback() , 
			bind(&Ndn_socket::onRegisterFailed, this, _1, _2)) ;
	return 0 ;
}

int Ndn_socket::set_daddr(const char * prefix){
	this->daddr = prefix ;
	return 0 ;
}
int Ndn_socket::write(const char * data , int len ) { 
	this->write(data , len , this->maddr) ;
}

// brief : 往socket中写入数据
// param : data 数据
//			len 数据长度
//			dname_base  数据包的前缀名称 

int Ndn_socket::write(const char * data , int len , string dname_base ) {
	if(dname_base[dname_base.length()-1] != '/') dname_base += "/" ;
	int pkt_n = len/8000 ;
	if(len%8000 != 0) pkt_n ++ ;
	string pre_payload = dname_base + to_string(seq) + "-" + to_string(seq+pkt_n) ;
	for (int i = 0; i < pkt_n; i++) {
		Data data_pkt ;
		data_pkt.setName(dname_base + to_string(seq)) ;
		seq ++ ;
		int c_len = 8000 ;
		if(len - i*8000 < 8000) c_len = len - i*8000 ;
		data_pkt.setContent(reinterpret_cast<const uint8_t*>(data+i*8000), c_len) ;
		this->m_keyChain.sign(data_pkt) ;
		this->m_face.put(data_pkt) ;
	}

	string pre_iname = daddr +"/"+ to_string(seq) ;
	Interest pre_int(Name(pre_iname.data())) ;
	pre_int.setInterestLifetime(1_s) ;
	pre_int.setMustBeFresh(true) ;

	Block app_param = makeBinaryBlock(tlv::AppPrivateBlock1+1,
			pre_payload.data(), pre_payload.length());
	pre_int.setParameters(app_param) ;

	this->m_face.expressInterest(pre_int , 
			bind(&Ndn_socket::onData,this,_1,_2),
			bind(&Ndn_socket::onNack,this,_1,_2),
			bind(&Ndn_socket::onTimeout_pre,this,_1));
	//cout << "pending interest num = " << this->m_face.getNPendingInterests() << endl ;
	return 0 ;
}
int Ndn_socket::read(char *data ) {
	this->read_buf = data ;
	pthread_mutex_lock(&recv_mutex) ;
	while(recv_n == 0){
		pthread_cond_wait(&has_recv , &recv_mutex) ;
	}
	pthread_mutex_unlock(&recv_mutex) ;

	return recv_n ;
}


void Ndn_socket::onInterest(const InterestFilter& filter, 
		const Interest& interest) {

	if(interest.hasParameters()){
		uint8_t p_type = 0 ;
		memcpy(&p_type , interest.getParameters().value() , 1) ;
		if(p_type == tlv::AppPrivateBlock1 + 1) {  // 预请求包
			Block dname_block(interest.getParameters().value() ,
					interest.getParameters().value_size()) ;
			string datas_name((char*)dname_block.value() , 
					dname_block.value_size()) ;
			// format : /ndn/edu/pkusz/node11/vpn/5-9
			int idx1 = datas_name.rfind('/')+1 ;
			int first, last ;
			sscanf(datas_name.data()+idx1 , "%d-%d" , &first , &last) ;
			string iname_base = datas_name.substr(0, idx1) ;
			for (int i = first; i < last; i++) {
				string interest_name = iname_base+ std::to_string(i) ;
				Interest request_int(Name(interest_name.data())) ;
				request_int.setInterestLifetime(1_s) ;
				
				this->m_face.expressInterest(request_int , 
						bind(&Ndn_socket::onData,this,_1,_2),
						bind(&Ndn_socket::onNack,this,_1,_2),
						bind(&Ndn_socket::onTimeout,this,_1));
			}
		}
	}

	//if(){     // 预请求包

	//}else{    // 请求数据兴趣包

	//}
}

void Ndn_socket::onData(const Interest& interest , const Data& data){

	int data_sz = data.getContent().value_size() ;
	pthread_mutex_lock(&recv_mutex) ;
	memcpy(read_buf+recv_n , (char*)(data.getContent().value()) , data_sz ) ;
	recv_n += data_sz ;
	pthread_mutex_unlock(&recv_mutex) ;
	if(recv_n == data_sz) pthread_cond_signal(&has_recv) ;
}

void Ndn_socket::onNack(const Interest& interest, const Nack& nack){
	cout << "connect to remote node fail" << endl ;
	this->m_face.shutdown() ;
}

void Ndn_socket::onTimeout(const Interest& interest) {
	cout << "Time out " << interest.getName() << endl ;
	Interest interest_new(interest.getName());
	interest_new.setInterestLifetime(1_s);
	this->m_face.expressInterest(interest_new,
			bind(&Ndn_socket::onData,this,_1,_2),
			bind(&Ndn_socket::onNack,this,_1,_2),
			bind(&Ndn_socket::onTimeout,this,_1));
}

void Ndn_socket::onTimeout_pre(const Interest& interest) {

}

void Ndn_socket::onRegisterFailed(const Name& prefix, const std::string& reason)
{
	std::cerr << "ERROR: Failed to register prefix \""
		<< prefix << "\" in local hub's daemon (" << reason << ")"
		<< std::endl;
	m_face.shutdown();
}

int Ndn_socket::close(){
	while(m_face.getNPendingInterests() > 0){
		usleep(10000) ;
	}
	this->m_face.shutdown() ;
}

void *Ndn_socket::run(void *param){
	Face *face_p = (Face*)param ;
	face_p->processEvents(time::milliseconds::zero(), true) ;
}
