#pragma once

#include <string>
#include "boost/smart_ptr.hpp"

namespace boost{ namespace system{ class error_code; }; };
#define	MAX_RECIVE_BUFFER		256

//	�Q�lURL
//	http://chicklab.blog84.fc2.com/blog-entry-29.html

namespace serial
{
	/**
	* @brief�F�V���A���|�[�g�̃I�v�V������ݒ肷�邽�߂�Enum�N���X�Q
	*/
	// �{�[���[�g�ݒ�p
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

	// �p���e�B�ݒ�p
	class Parity {
	public:
		enum {
			None=0,
			Odd,
			Even
		};
	};

	// �X�g�b�v�r�b�g�ݒ�p
	class StopBits {
	public:
		enum {
			One=0,
			OnePointFive,
			Two
		};
	};

	// �f�[�^�̃o�C�g�T�C�Y�ݒ�p
	class ByteSize{
	public:
		enum{
			B7 = 7,
			B8 = 8
		};
	};

	// �t���[�R���g���[���p�̐ݒ�
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
	* @brief : observer���g���ăR�[���o�b�N���������邽�߂̃C���^�[�t�F�[�X�N���X
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
	* @brief : boost���g�����V���A���ʐM�N���X
	*          �C�x���g�œ����悤�ɂ����̂���
	*          �ʐM�ɗ]���ȃ��\�[�X�������K�v���Ȃ��Ȃ�܂��B
	*/
	class SerialPort
	{
	// �R���X�g���N�^�A�f�X�g���N�^------------------------
	public:
		SerialPort();
		virtual ~SerialPort();


	// ����------------------------------------------------
	public:
		/*
		* @brief      : �|�[�g�̃I�[�v��
		* @param[in]  : com�|�[�g
		* @param[in]  : 1�o�C�g�̃r�b�g��
		* @param[in]  : �p���e�B���w��
		* @param[in]  : �X�g�b�v�r�b�g�w��
		* @return     : ��������
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
		* @brief	: �|�[�g�̃N���[�Y
		* @return	: ��������
		*/
		bool close();

		/**
		* @brief	: ������𑗐M����֐�
		* @param[in]: ���M���镶����
		*/
		bool send( const std::string& s );

		/**
		* @brief	: ������𑗐M����֐�
		* @param[in]: ���M���镶��
		*/
		bool send( char c );

		/**
		* @brief	: ������𑗐M����֐�
		* @param[in]: ���M���镶����
		* @param[in]: ������̃T�C�Y
		*/
		bool send( const char *c, size_t size );

		/**
		* @brief	: observer��ǉ�����֐�
		* @param[in]: observer�N���X 
		* @return	: ��������
		*/
		bool attach( SerialObserver* ob );

		/**
		* @brief	: observer���폜����֐�
		* @param[in]: observer�N���X
		* @return	: ��������
		*/
		bool detach( SerialObserver* ob );

		/**
		* @brief		: �f�[�^����M����֐�
		* @param[out]	: �擾�f�[�^
		* @return		: �c��̃f�[�^��
		*/
		bool receive( std::string& str, double timeout );

		/**
		* @brief	: �ڑ��m�F
		* @return	: �ڑ���
		*/
		bool isConnect() const { return is_connect_; }



	private:
		// �X�V�֐�
		virtual void notifyAll( const std::string& str );

		// �f�[�^��������
		bool write( const char* str, size_t n );

		// IO service
		void runThread();

	// ����------------------------------------------------
	private:
		class serial_impl;
		boost::shared_ptr<serial_impl> impl;
//		serial_impl* impl;

		// ��M�p�o�b�t�@
		char rBuffer[MAX_RECIVE_BUFFER];
		
		void read_ok( const boost::system::error_code& e, size_t size );
		bool is_connect_;

		// �ŐV�ł̎�M�f�[�^
		std::string readData;
	};

};
