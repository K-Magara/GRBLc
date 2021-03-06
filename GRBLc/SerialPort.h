#pragma once

#include <string>
#include "boost/smart_ptr.hpp"

namespace boost{ namespace system{ class error_code; }; };
#define	MAX_RECIVE_BUFFER		256

//	参考URL
//	http://chicklab.blog84.fc2.com/blog-entry-29.html

namespace serial
{
	/**
	* @brief：シリアルポートのオプションを設定するためのEnumクラス群
	*/
	// ボーレート設定用
	class BaudRate {
	public:
		enum {
			B300=300, B1200=1200, B2400=2400, B4800=4800, B9600=9600,
			B19200=19200, B38400=38400, B57600=57600, B74880=74880,
			B115200=115200, B230400=230400,
			B250000=250000, B500000=500000,
			B1000000=1000000, B2000000=2000000
		};
	};

	// パリティ設定用
	class Parity {
	public:
		enum {
			None=0,
			Odd,
			Even
		};
	};

	// ストップビット設定用
	class StopBits {
	public:
		enum {
			One=0,
			OnePointFive,
			Two
		};
	};

	// データのバイトサイズ設定用
	class ByteSize{
	public:
		enum{
			B7 = 7,
			B8 = 8
		};
	};

	// フローコントロール用の設定
	class FlowControl{
	public:
		enum{
			None=0,
			Hardware,
			Software
		};
	};

	class SerialPort;
	/**
	* @brief : observerを使ってコールバックを実装するためのインターフェースクラス
	*/
	class SerialObserver 
	{
		friend SerialPort;
	public:
		SerialObserver(){}
		virtual ~SerialObserver(){}

	protected:
		virtual void notify( const std::string& str )=0;

	};


	/**
	* @brief : boostを使ったシリアル通信クラス
	*          イベントで動くようにしたのため
	*          通信に余分なリソースをさく必要がなくなります。
	*/
	class SerialPort
	{
	// コンストラクタ、デストラクタ------------------------
	public:
		SerialPort();
		virtual ~SerialPort();


	// 操作------------------------------------------------
	public:
		/*
		* @brief      : ポートのオープン
		* @param[in]  : comポート
		* @param[in]  : 1バイトのビット数
		* @param[in]  : パリティを指定
		* @param[in]  : ストップビット指定
		* @return     : 成功判定
		*/
		bool open( 
			const std::string& com = "COM1",
			int baudrate = BaudRate::B115200,
			int bytesize = ByteSize::B8,
			int parity = Parity::None,
			int stopbits = StopBits::One,
			int flowcontrol = FlowControl::None
		);

		/**
		* @brief	: ポートのクローズ
		* @return	: 成功判定
		*/
		bool close();

		/**
		* @brief	: 文字列を送信する関数
		* @param[in]: 送信する文字列
		*/
		bool send( const std::string& s );

		/**
		* @brief	: 文字列を送信する関数
		* @param[in]: 送信する文字
		*/
		bool send( char c );

		/**
		* @brief	: 文字列を送信する関数
		* @param[in]: 送信する文字列
		* @param[in]: 文字列のサイズ
		*/
		bool send( const char *c, size_t size );

		/**
		* @brief	: observerを追加する関数
		* @param[in]: observerクラス 
		* @return	: 成功判定
		*/
		bool attach( SerialObserver* ob );

		/**
		* @brief	: observerを削除する関数
		* @param[in]: observerクラス
		* @return	: 成功判定
		*/
		bool detach( SerialObserver* ob );

		/**
		* @brief		: データを受信する関数
		* @param[out]	: 取得データ
		* @return		: 残りのデータ数
		*/
		bool receive( std::string& str, double timeout );

		/**
		* @brief	: 接続確認
		* @return	: 接続状況
		*/
		bool isConnect() const { return is_connect_; }



	private:
		// 更新関数
		virtual void notifyAll( const std::string& str );

		// データ書き込み
		bool write( const char* str, size_t n );

		// IO service
		void runThread();

	// 属性------------------------------------------------
	private:
		class serial_impl;
		boost::shared_ptr<serial_impl> impl;
//		serial_impl* impl;

		// 受信用バッファ
		char rBuffer[MAX_RECIVE_BUFFER];
		
		void read_ok( const boost::system::error_code& e, size_t size );
		bool is_connect_;

		// 最新版の受信データ
		std::string readData;
	};

};
