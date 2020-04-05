#include "stdafx.h"

#include <vector>
#include <algorithm>

#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/foreach.hpp"
#include "boost/thread/win32/mfc_thread_init.hpp"
#include "boost/thread.hpp"
#include "boost/thread/condition.hpp"

#include "SerialPort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace serial;
using namespace boost::asio;
using namespace std;

///
/// boost�̃C���^�[�t�F�[�X���B�����邽�߂̃N���X
///
class SerialPort::serial_impl
{
// �R���X�g���N�^�A�f�X�g���N�^----------------------------
public:
	serial_impl() : serial_port(NULL)
		, br( serial_port_base::baud_rate(115200) )
		, cs( serial_port_base::character_size( 8 ) )
		, fc( serial_port_base::flow_control::none )
		, parity( serial_port_base::parity::none )
		, sb( serial_port_base::stop_bits::one )
		, com( "COM1" )
	{}

	virtual ~serial_impl()
	{
		if ( serial_port ) delete serial_port;
		serial_port = NULL;
	}

	
// ����----------------------------------------------------
public:
	// shared_ptr
	vector< SerialObserver* > ptrList;

	// thread
	boost::thread ioThread;
	boost::condition recv_condition;
	boost::mutex recv_sync;

	// �V���A���̐ݒ�n��
	io_service		io;
	string			com;
	serial_port*	serial_port;
	serial_port_base::baud_rate br;
	serial_port_base::character_size cs;
	serial_port_base::flow_control::type fc;
	serial_port_base::parity::type parity;
	serial_port_base::stop_bits::type sb;
};


/**
* @brief �R���X�g���N�^
*/
SerialPort::SerialPort()// : impl( new serial_impl() )
{
	impl = new serial_impl;
	is_connect_ = false;
}


/**
* @brief : �f�X�g���N�^
*/
SerialPort::~SerialPort()
{
	close();
	delete	impl;
}


/**
* @brief		: �|�[�g�̃I�[�v��
* @param[in]	: com�|�[�g
* @param[in]	: 1�o�C�g�̃r�b�g��
* @param[in]	: �p���e�B���w��
* @param[in]	: �X�g�b�v�r�b�g�w��
* @return		: ��������
*/
bool SerialPort::open(const string &com,
		int baudrate, int cs, int parity, int stopbits, int flow)
{
	if ( is_connect_ ) return false;

	boost::system::error_code ret;

	// �|�[�g�̃I�[�v��
	impl->serial_port = new serial_port( impl->io );
	impl->serial_port->open( com, ret );

	if( ret ) {
#ifdef _DEBUG
		cout << "resial_port open() error " << ret << endl;
#endif
		return false;
	}

	// �ڑ��t���O
	is_connect_ = true;

	// �p���e�B�l�̐ݒ�
	serial_port_base::parity::type parity_t = serial_port_base::parity::none;
	if ( parity == Parity::Even ) parity_t = serial_port_base::parity::even;
	else if ( parity == Parity::Odd ) parity_t = serial_port_base::parity::odd;

	// Stop Bists
	serial_port_base::stop_bits::type stopbit_t = serial_port_base::stop_bits::one;
	if ( stopbits == StopBits::Two ) stopbit_t = serial_port_base::stop_bits::two;
	else if ( stopbits == StopBits::OnePointFive ) stopbit_t = serial_port_base::stop_bits::onepointfive;

	// flow control
	serial_port_base::flow_control::type flow_t = serial_port_base::flow_control::none;
	if ( flow == FlowControl::Hardware ) flow_t = serial_port_base::flow_control::hardware;
	else if ( flow == FlowControl::Software ) flow_t = serial_port_base::flow_control::software;

	// �ݒ�l�̎擾
	impl->com = com;
	impl->br = serial_port_base::baud_rate( baudrate );
	impl->cs = serial_port_base::character_size( cs );
	impl->parity = parity_t;
	impl->sb = stopbit_t;
	impl->fc = flow_t;

	impl->serial_port->set_option( impl->br );
	impl->serial_port->set_option( serial_port_base::parity(parity_t) );
	impl->serial_port->set_option( serial_port_base::character_size( cs ) );
	impl->serial_port->set_option( serial_port_base::stop_bits(stopbit_t) );
	impl->serial_port->set_option( serial_port_base::flow_control(flow_t) );

	// �ǂݍ��ݗp�̊֐���ݒ�
	impl->serial_port->async_read_some( 
		boost::asio::buffer(rBuffer, MAX_RECIVE_BUFFER), 
		boost::bind(&SerialPort::read_ok, this, _1, _2) );

	// IO�T�[�r�X�̊J�n
//	impl->ioThread = boost::thread( boost::bind(&boost::asio::io_service::run, &impl->io) );
	impl->ioThread = boost::thread( &SerialPort::runThread, this );

	return true;
}


/**
* @brier	: �I�u�W�F�N�g�̓o�^���s��
* @param[in]: �o�^���s���I�u�W�F�N�g
* @return	: ��������
*/
bool SerialPort::attach(SerialObserver *ob)
{
	vector<SerialObserver*>::iterator it = find( impl->ptrList.begin(), impl->ptrList.end(), ob );

	// �o�^����Ă��Ȃ�������A�I�u�U�[�o�[��o�^
	if ( it != impl->ptrList.end() ) return false;
	impl->ptrList.push_back(ob);
	return true;
}


/**
* @brier	: �I�u�W�F�N�g�̔j�����s��
* @param[in]: �j�����s���I�u�W�F�N�g
* @return	: ��������
*/
bool SerialPort::detach(SerialObserver *ob)
{
	vector<SerialObserver*>::iterator it = find( impl->ptrList.begin(), impl->ptrList.end(), ob );

	// �o�^����Ă��Ȃ�������A�I�u�U�[�o�[��o�^
	if ( it == impl->ptrList.end() ) return false;
	impl->ptrList.erase( it );
	return true;
}


/**
* @brief	: ��Ԃ̍X�V��ʒm����
* @param[in]: ��M������
*/
void SerialPort::notifyAll( const string& str )
{
	// �S�ẴI�u�U�[�o�[�ɒʒm
	if ( is_connect_ ) {
		BOOST_FOREACH( SerialObserver* ob, impl->ptrList ) ob->notify(str);
	}
	// �R���f�B�V��������
	boost::mutex::scoped_lock lk( impl->recv_sync );
	readData = str;
	impl->recv_condition.notify_all();
}


/**
* @brief	: �|�[�g�̃N���[�Y
* @return	: ��������
*/
bool SerialPort::close()
{
	if ( !is_connect_ ) return false;

	is_connect_ = false;
	impl->recv_condition.notify_all();

	impl->io.stop();
	if ( !impl->ioThread.timed_join(boost::posix_time::milliseconds(EVENT_TIMEOUT)) ) {
#ifdef _DEBUG
		cout << "impl->ioThread end timeout\n";
#endif
		impl->ioThread.detach();
		impl->ioThread.interrupt();
		impl->ioThread.join();	// intrruption point
	}

	// �S�ďI�����Ă��� close() ���Ȃ���
	// impl->ioThread ���I�����Ȃ��Ȃ�
	impl->serial_port->close();
#ifdef _DEBUG
	cout << "impl->serial_port->close() ok\n";
#endif

	return true;
}


/*
* @brief		: �f�[�^���[�h
* @return		: ��������
* @param[out]	: ��M�f�[�^
* @param[in]	: �^�C���A�E�g[ms]
*/
bool SerialPort::receive( string& str, double timeout )
{

	// �ڑ�����
	if ( !is_connect_ ) return false;

	boost::mutex::scoped_lock lk( impl->recv_sync );

	// ��M�҂�
	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC_);
	xt.nsec += static_cast<int>(timeout*1000.0);

	// ��M�҂����s
	if ( !impl->recv_condition.timed_wait(lk,xt) ) return false;

	// ��M��������i�[
	str = this->readData;
	return true;
}


/**
/* @brief	: ������̑��M�֐�
/* @return	: ��������
*/
bool SerialPort::send( const string& s )
{
	return write( s.c_str(), s.size() );
}

bool SerialPort::send( char c )
{
	return write( &c, 1 );
}

bool SerialPort::send( const char* c, size_t size )
{
	return write( c, size );
}

bool SerialPort::write( const char* str, size_t n )
{
	if ( !is_connect_ ) return false;

	boost::system::error_code ret;
	impl->serial_port->write_some( boost::asio::buffer(str, n), ret );
	if ( ret ) {
#ifdef _DEBUG
		cout << "serial_port::write_some() return = " << ret << endl;
#endif
		return false;
	}

	return true;
}

void SerialPort::read_ok( const boost::system::error_code& e, size_t size )
{
	if ( e ) {
#ifdef _DEBUG
		cout << "read_some() Error = " << e << endl;
#endif
		return;
	}

	// ��M����
	string str(rBuffer, rBuffer+size);

	// �X�V����
	notifyAll( str );

	// �ǂݍ��݂����������̂ŁA�ēx�ݒ�
	if ( is_connect_ ) {
		impl->serial_port->async_read_some(
			boost::asio::buffer(rBuffer, MAX_RECIVE_BUFFER),
			boost::bind(&SerialPort::read_ok, this, _1, _2) );
	}
}

void SerialPort::runThread()
{
#ifdef _DEBUG
	cout << "SerialPort::runThread() start\n";
#endif
	try {
		impl->io.run();
	}
#ifdef _DEBUG
	catch (boost::system::system_error& e) {
		cout << "impl->io.run() error code=" << e.what() << endl;
#else
	catch (boost::system::system_error&) {
#endif
	}
#ifdef _DEBUG
	cout << "SerialPort::runThread() end\n";
#endif
}
