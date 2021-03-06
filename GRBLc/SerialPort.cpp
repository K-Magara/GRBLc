#include "stdafx.h"

#include <vector>
#include <algorithm>

#include "boost/asio.hpp"
#include "boost/bind/bind.hpp"	// boost 1.73
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
using namespace boost::placeholders;	// boost 1.73
using namespace std;

///
/// boostのインターフェースを隠蔽するためのクラス
///
class SerialPort::serial_impl
{
// コンストラクタ、デストラクタ----------------------------
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

	
// 属性----------------------------------------------------
public:
	// shared_ptr
	vector< SerialObserver* > ptrList;

	// thread
	boost::thread ioThread;
	boost::condition recv_condition;
	boost::mutex recv_sync;

	// シリアルの設定系統
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
* @brief コンストラクタ
*/
SerialPort::SerialPort() : impl( new serial_impl() )
{
//	impl = new serial_impl;
	is_connect_ = false;
}


/**
* @brief : デストラクタ
*/
SerialPort::~SerialPort()
{
	close();
//	delete	impl;
}


/**
* @brief		: ポートのオープン
* @param[in]	: comポート
* @param[in]	: 1バイトのビット数
* @param[in]	: パリティを指定
* @param[in]	: ストップビット指定
* @return		: 成功判定
*/
bool SerialPort::open(const string &com,
		int baudrate, int cs, int parity, int stopbits, int flow)
{
	if ( is_connect_ ) return false;

	boost::system::error_code ret;

	// ポートのオープン
	impl->serial_port = new serial_port( impl->io );
	impl->serial_port->open( com, ret );

	if( ret ) {
#ifdef _DEBUG
		cout << "resial_port open() error " << ret << endl;
#endif
		return false;
	}

	// 接続フラグ
	is_connect_ = true;

	// パリティ値の設定
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

	// 設定値の取得
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

	// 読み込み用の関数を設定
	impl->serial_port->async_read_some( 
		boost::asio::buffer(rBuffer, MAX_RECIVE_BUFFER), 
		boost::bind(&SerialPort::read_ok, this, _1, _2) );

	// IOサービスの開始
//	impl->ioThread = boost::thread( boost::bind(&boost::asio::io_service::run, &impl->io) );
	impl->ioThread = boost::thread( &SerialPort::runThread, this );

	return true;
}


/**
* @brier	: オブジェクトの登録を行う
* @param[in]: 登録を行うオブジェクト
* @return	: 成功判定
*/
bool SerialPort::attach(SerialObserver *ob)
{
	vector<SerialObserver*>::iterator it = find( impl->ptrList.begin(), impl->ptrList.end(), ob );

	// 登録されていなかったら、オブザーバーを登録
	if ( it != impl->ptrList.end() ) return false;
	impl->ptrList.push_back(ob);
	return true;
}


/**
* @brier	: オブジェクトの破棄を行う
* @param[in]: 破棄を行うオブジェクト
* @return	: 成功判定
*/
bool SerialPort::detach(SerialObserver *ob)
{
	vector<SerialObserver*>::iterator it = find( impl->ptrList.begin(), impl->ptrList.end(), ob );

	// 登録されていなかったら、オブザーバーを登録
	if ( it == impl->ptrList.end() ) return false;
	impl->ptrList.erase( it );
	return true;
}


/**
* @brief	: 状態の更新を通知する
* @param[in]: 受信文字列
*/
void SerialPort::notifyAll( const string& str )
{
	// 全てのオブザーバーに通知
	if ( is_connect_ ) {
		BOOST_FOREACH( SerialObserver* ob, impl->ptrList ) ob->notify(str);
	}
	// コンディション解除
	boost::mutex::scoped_lock lk( impl->recv_sync );
	readData = str;
	impl->recv_condition.notify_all();
}


/**
* @brief	: ポートのクローズ
* @return	: 成功判定
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

	// 全て終了してから close() しないと
	// impl->ioThread が終了しなくなる
	impl->serial_port->close();
#ifdef _DEBUG
	cout << "impl->serial_port->close() ok\n";
#endif

	return true;
}


/*
* @brief		: データリード
* @return		: 成功判定
* @param[out]	: 受信データ
* @param[in]	: タイムアウト[ms]
*/
bool SerialPort::receive( string& str, double timeout )
{

	// 接続判定
	if ( !is_connect_ ) return false;

	boost::mutex::scoped_lock lk( impl->recv_sync );

	// 受信待ち
	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC_);
	xt.nsec += static_cast<int>(timeout*1000.0);

	// 受信待ち失敗
	if ( !impl->recv_condition.timed_wait(lk,xt) ) return false;

	// 受信文字列を格納
	str = this->readData;
	return true;
}


/**
/* @brief	: 文字列の送信関数
/* @return	: 成功判定
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

	// 受信処理
	string str(rBuffer, rBuffer+size);

	// 更新処理
	notifyAll( str );

	// 読み込みが完了したので、再度設定
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
