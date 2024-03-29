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
	struct timeval start  ;
	gettimeofday(&start , NULL) ;
	this->start_ts = to_string(start.tv_sec) ;
	pthread_create(&(this->m_tid) , NULL , run, (void*)&m_face) ;
	this->state = true ;
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
string Ndn_socket::get_daddr(){
	return this->daddr ;
}
int Ndn_socket::write(const string& data) { 
	return this->write(data.data() , data.length() , this->maddr) ;
}
int Ndn_socket::write(const char * data , int len ) { 
	return this->write(data , len , this->maddr) ;
}
int Ndn_socket::write(const uint8_t * data , int len) {
	return this->write((char*)data, len , this->maddr) ;
}
int Ndn_socket::write(const uint8_t * data , int len , string dname_base) {
	return this->write((char*)data, len , dname_base) ;
}

// brief : 往socket中写入数据
// param : data 数据
//			len 数据长度
//			dname_base  数据包的前缀名称 

int Ndn_socket::write(const char * data , int len , string dname_base ) {
	if(dname_base[dname_base.length()-1] != '/') {
		dname_base += ("/"+this->start_ts+"/") ;
	}
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

	string pre_iname = daddr +"/"+ this->start_ts + "/" + to_string(seq) ;
	Interest pre_int(Name(pre_iname.data())) ;
	pre_int.setInterestLifetime(1_s) ;
	pre_int.setMustBeFresh(true) ;

	Block app_param = makeBinaryBlock(tlv::AppPrivateBlock1+1,
			pre_payload.data(), pre_payload.length());
	pre_int.setParameters(app_param) ;

	// 发送预请求兴趣包
	this->m_face.expressInterest(pre_int , 
			bind(&Ndn_socket::onData_pre,this,_1,_2),
			bind(&Ndn_socket::onNack_pre,this,_1,_2),
			bind(&Ndn_socket::onTimeout_pre,this,_1));
	//cout << "pre I>> : " << pre_int.getName() << endl; 
	return len ;
}

int Ndn_socket::read(char *data , int buf_sz) {
	r_queue.wait4data() ;
	if(this->state == false) return -1 ;
	int data_len = r_queue.get_data_len() ;
	if(data_len > buf_sz) data_len = buf_sz ;
	r_queue.get_ndata(data , data_len) ;
	r_queue.rmv_n(data_len) ;
	return data_len ;
}

void Ndn_socket::onInterest(const InterestFilter& filter, 
		const Interest& interest) {

	//cout << "onInterest : " << interest.getName() << endl ;
	if(interest.hasParameters()){
		uint8_t p_type = 0 ;
		memcpy(&p_type , interest.getParameters().value() , 1) ;
		if(p_type == tlv::AppPrivateBlock1 + 1) {  // 预请求包
			Block dname_block(interest.getParameters().value() ,
					interest.getParameters().value_size()) ;
			string datas_name((char*)dname_block.value() , 
					dname_block.value_size()) ;
			//cout << "datas_name : "  <<datas_name << endl ;
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
				//cout << "I>> : " <<  request_int.getName() << endl ;
			}
		}
	}

	//if(){     // 预请求包

	//}else{    // 请求数据兴趣包

	//}
}

void Ndn_socket::onData(const Interest& interest , const Data& data){
	int data_sz = data.getContent().value_size() ;
	r_queue.push_ndata(reinterpret_cast<const char*>(data.getContent().value()),
			data_sz) ;
	//cout << "D<< :" << data.getName() << " sz = " << data_sz << endl ;
}
void Ndn_socket::onData_pre(const Interest& interest , const Data& data){
	//cout << "pre D<< :" << data.getName() << " sz = " << 
		//data.getContent().value_size() << endl ;
}


void Ndn_socket::onNack(const Interest& interest, const Nack& nack){
	cout << "Nack : "<< interest.getName() << endl ;
	long lifetime = interest.getInterestLifetime().count() ;
	if(lifetime > 3000) return ;
	if(this->state == false) return ;
	sleep(1) ;
	Interest interest_new(interest.getName());
	interest_new.setMustBeFresh(true) ;
	boost::chrono::milliseconds new_lifetime(lifetime+200) ;
	interest_new.setInterestLifetime(new_lifetime);
	this->m_face.expressInterest(interest_new,
			bind(&Ndn_socket::onData,this,_1,_2),
			bind(&Ndn_socket::onNack,this,_1,_2),
			bind(&Ndn_socket::onTimeout,this,_1));
	cout << "I>> : " <<  interest_new.getName() << endl ;
	//this->m_face.shutdown() ;
}

void Ndn_socket::onNack_pre(const Interest& interest, const Nack& nack){
	cout << "pre Nack : "<< interest.getName() << endl ;
	cout << "listen : " << this->maddr << endl ;
	long lifetime = interest.getInterestLifetime().count() ;
	if(lifetime > 3000 || this->state == false ) return ;
	sleep(1) ;
	Interest interest_new(interest.getName());
	interest_new.setMustBeFresh(true) ;
	boost::chrono::milliseconds new_lifetime(lifetime+200) ;
	interest_new.setInterestLifetime(new_lifetime);
	interest_new.setParameters(interest.getParameters());
	this->m_face.expressInterest(interest_new,
			bind(&Ndn_socket::onData_pre,this,_1,_2),
			bind(&Ndn_socket::onNack_pre,this,_1,_2),
			bind(&Ndn_socket::onTimeout_pre,this,_1));
	cout << "pre I>> : " <<  interest_new.getName() << endl ;
}

void Ndn_socket::onTimeout(const Interest& interest) {
	cout << "Time out " << interest.getName() << endl ;
	long lifetime = interest.getInterestLifetime().count() ;
	if(lifetime > 3000) return ;
	if(this->state == false) return ;
	Interest interest_new(interest.getName());
	boost::chrono::milliseconds new_lifetime(lifetime+200) ;
	interest_new.setInterestLifetime(new_lifetime);
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
	if(this->state == false ) return 0 ;
	this->state = false ;
	char c_flag = 'c' ;
	r_queue.push_ndata(&c_flag,1) ;
	while(m_face.getNPendingInterests() > 0){
		usleep(10000) ;
	}
	this->m_face.shutdown() ;
	return 0 ;
}

void *Ndn_socket::run(void *param){
	Face *face_p = (Face*)param ;
	face_p->processEvents(time::milliseconds::zero(), true) ;
}
