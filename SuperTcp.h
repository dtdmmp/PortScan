#pragma once
#include <windows.h>
#include <Mswsock.h>
#include <WinSock2.h>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>


#define IO_BUFF_SIZE  4096

#define PREPARE_SOCKET_COUNT 1000  //����SOCKET��
#define MAX_SOCKET_COUNT  100000   //���10w����
#define MAX_SEND_QUEUE	  64	   //����Ͷ���

#define SIGNAL_NULL  0
#define SIGNAL_EXIT -1

#define MIN_PORT_VALUE 1025
#define MAX_PORT_VALUE 65534

#define SCAN_CYCLE   (60*1000L)  //�����߳�ɨ������

#define CLIENT_TYPE_NORMAL 0x01
#define CLIENT_TYPE_SCANER 0x02

typedef enum _SOCKET_OPERATION_TYPE
{
	OP_ACPT,                     // ��־Ͷ�ݵ�Accept����
	OP_SEND,                     // ��־Ͷ�ݵ��Ƿ��Ͳ���
	OP_RECV,                     // ��־Ͷ�ݵ��ǽ��ղ���
	OP_CONN,					 // ��־Ͷ�ݵ������Ӳ���
	OP_NULL						 // ���ڳ�ʼ����������
}SOCKET_OPERATION_TYPE;

typedef struct _OVERLAPPEDEX
{
	OVERLAPPED				Overlapped;
	SOCKET_OPERATION_TYPE	Type;
}OVERLAPPEDEX, *POVERLAPPEDEX;

typedef std::queue<WSABUF*> SEND_QUEUE;

typedef enum _SOCKET_STAT
{
	ss_init,		//socket�ѳ�ʼ��
	ss_buzy,		//socket���ڹ���
	ss_down			//socket�Ѿ��ر�
}SOCKET_STAT;

typedef struct _SOCK_CONTEXT
{
	SOCKET					Socket;                                 //������������ʹ�õ�Socket
	SOCKET_STAT				Status;									//��ǰSOCKET״̬
	SOCKADDR_IN				LocalAddr;								//���ص�ַ
	SOCKADDR_IN				RemoteAddr;								//Զ�˵�ַ	
	time_t					ConnectedTime;							//����ʱ��	
	time_t					LastActive;								//���ʱ��	
	unsigned __int64		TotleSendBytes;							//�����ֽ�����	
	unsigned __int64		TotleRecvBytes;							//�����ֽ�����	
}SOCK_CONTEXT;

typedef struct _PER_IO_CONTEXT
{	
	OVERLAPPEDEX			Overlapped1;							//�ص��ṹ,ACCEPT,CONNECT,SEND����
	OVERLAPPEDEX			Overlapped2;							//RECVר��
	SOCK_CONTEXT			SockCtx;								//Socket�����Ϣ
	WSABUF					BuffRecv;                               //WSA���͵Ļ����������ڸ��ص�������������
	char					Data[IO_BUFF_SIZE];                     //�����WSABUF�������ַ��Ļ�����	
	CRITICAL_SECTION		SendQueueLock;							//���Ͷ�����
	SEND_QUEUE*				QueueSend;								//���Ͷ���
	WSABUF					BuffSend;								//����0�ֽڷ���
} PER_IO_CONTEXT, *PPER_IO_CONTEXT;

typedef struct _PKT
{
	unsigned short uLen;
	char		   szData[2];
}PKT,*PPKT;

typedef std::queue<PPKT>  packet_queue;

typedef void (*pfnOnConnect)(SOCKET sSocket, const SOCK_CONTEXT&  Context);
typedef void (*pfnOnRecvComplete)(SOCKET sSocket, DWORD dwLen,char* pData,const SOCK_CONTEXT& Context);
typedef void (*pfnOnSendComplete)(SOCKET sSocket, DWORD dwLen, const SOCK_CONTEXT& Context);
typedef void (*pfnOnDisconnect)(SOCKET sSocket, const SOCK_CONTEXT& Context);
typedef void (*pfnScanComplete)(void);
typedef void (*pfnShowMessage)(const char* pszMsg);


class CSuperTcp
{
public:
	CSuperTcp();
	virtual ~CSuperTcp();

public:
	//ȡ�õ�ǰcpu���� 
	DWORD GetProcessorCoreCount();

	//����iocp socketģ��
	BOOL StartListen(pfnOnConnect OnConnect = nullptr,
		pfnOnDisconnect OnDisconnect = nullptr,
		pfnOnRecvComplete OnRecvComplete = nullptr,
		pfnShowMessage OnMsg=nullptr,
		unsigned short uPort = 8086, 
		const char* lpszIpAddr = "0.0.0.0");
	BOOL StartConnect(pfnOnConnect OnConnect = nullptr,
		pfnOnDisconnect OnDisconnect = nullptr,
		pfnOnRecvComplete OnRecvComplete = nullptr,
		pfnShowMessage OnMsg = nullptr,
		unsigned long  uConnCount=10000,
		unsigned short uPort = 8086,
		const char* lpszIpAddr = "127.0.0.1",
		const char* lpszLocalIp = "0.0.0.0");

	BOOL StartScaner(
		std::vector<unsigned short>&& PortList,		   //�˿��б�
		pfnOnConnect OnConnect = nullptr,			   //���ӳɹ��ص�
		pfnOnDisconnect OnDisconnect = nullptr,        //���ӹرջص�
		pfnOnRecvComplete OnRecvComplete = nullptr,    //������ɻص�
		pfnScanComplete OnScanComplete=nullptr,        //ɨ����ɻص�
		pfnShowMessage OnMsg = nullptr,
		unsigned long uConnCount=10000,                //������
		const char* lpszStartIp="127.0.0.1",           //��ʼIP
		const char* lpszStopIp="127.0.0.1",            //��ֹIP
		const char* lpszLocalIp="0.0.0.0");            //���ص�ַ



	//ֹͣiocp socketģ��
	void Stop();
	
protected:
	//�Ƿ�ʵ����ʼ��������
	BOOL m_bStartup = FALSE;

	//���߳�
	static unsigned int __stdcall _MainThread(LPVOID lParam);
	void RunMain();
	//���߳��¼�֪ͨ 
	HANDLE m_hEvtStopped=nullptr;

	//�������߳�
	static unsigned int __stdcall _WorkerThread(LPVOID lParam);
	void RunWorker();
	//ÿ���߳̽����¼�֪ͨ
	std::map<DWORD,HANDLE> m_mapTidStopEvent;

	//�Ƿ���������
	BOOL m_bRunning = FALSE;

	//��ɶ˿�
	HANDLE m_hIOCP = nullptr;

	//����ģʽ
	BOOL m_bWorkAsServer = TRUE;

	//serverģʽ����
	//server ip
	std::string m_strLocalAddr="0.0.0.0"; 
	//server �˿�
	unsigned short m_uLocalPort = 8086;
	//��ǰ����socket��
	long m_lFreeSock = 0;

	//clientģʽ����,clientģʽ�ַּ��֣���������ģʽ��ɨ��ģʽ
	int m_nClientType = CLIENT_TYPE_NORMAL;
	//����������
	unsigned long m_uConnCount = 1;

	//1-��������ģʽ����ͬһ��ip�Ͷ˿ڷ���һ�����������ӣ���������Ч����������������ѹ�����ԣ�Ҳ����������ͨ����
	//Ҫ���ӵ�ip
	std::string m_strRemoteAddr = "127.0.0.1";
	//Ҫ���ӵĶ˿�
	unsigned short m_uRemotePort = 8086;
	long m_lValidSock = 0;
	BOOL m_bComplete = FALSE;

	//2-ɨ��ģʽ���Է�Χip�ͷ�Χ�˿ڽ������ӳ���
	struct in_addr m_addrFrom = {127,0,0,1};
	struct in_addr m_addrTo = { 127,0,0,1 };
	std::vector<unsigned short> m_vecPorts;
	long m_lBurstSock = 0;



	//����socket�������ݵ�map,��ɾ�������߳�����ɣ�����Ҫ��
	std::map<SOCKET, PER_IO_CONTEXT*> m_mapSocketToContex;

	pfnOnConnect cbOnConnect= nullptr;
	pfnOnRecvComplete cbOnRecvComplete=nullptr;
	pfnOnDisconnect cbOnDisconnect = nullptr;
	pfnScanComplete cbOnScanComplete = nullptr;
	pfnShowMessage cbShowMsg = nullptr;

	void ResetContex(PER_IO_CONTEXT* pContex);

	//��������
	LPFN_CONNECTEX m_pConnectEx = nullptr;

	unsigned short BindSocket(SOCKET s);

	void RunAsServer();
	void RunAsClient();
	void RunAsScaner();

public:
	BOOL SendTcpData(SOCKET sSocket,DWORD dwLen,char* pData);	
	void KillSocket(SOCKET sSocket);
};

