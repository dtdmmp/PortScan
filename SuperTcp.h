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

#define PREPARE_SOCKET_COUNT 1000  //常备SOCKET数
#define MAX_SOCKET_COUNT  100000   //最大10w连接
#define MAX_SEND_QUEUE	  64	   //最大发送队列

#define SIGNAL_NULL  0
#define SIGNAL_EXIT -1

#define MIN_PORT_VALUE 1025
#define MAX_PORT_VALUE 65534

#define SCAN_CYCLE   (60*1000L)  //管理线程扫描周期

#define CLIENT_TYPE_NORMAL 0x01
#define CLIENT_TYPE_SCANER 0x02

typedef enum _SOCKET_OPERATION_TYPE
{
	OP_ACPT,                     // 标志投递的Accept操作
	OP_SEND,                     // 标志投递的是发送操作
	OP_RECV,                     // 标志投递的是接收操作
	OP_CONN,					 // 标志投递的是连接操作
	OP_NULL						 // 用于初始化，无意义
}SOCKET_OPERATION_TYPE;

typedef struct _OVERLAPPEDEX
{
	OVERLAPPED				Overlapped;
	SOCKET_OPERATION_TYPE	Type;
}OVERLAPPEDEX, *POVERLAPPEDEX;

typedef std::queue<WSABUF*> SEND_QUEUE;

typedef enum _SOCKET_STAT
{
	ss_init,		//socket已初始化
	ss_buzy,		//socket正在工作
	ss_down			//socket已经关闭
}SOCKET_STAT;

typedef struct _SOCK_CONTEXT
{
	SOCKET					Socket;                                 //这个网络操作所使用的Socket
	SOCKET_STAT				Status;									//当前SOCKET状态
	SOCKADDR_IN				LocalAddr;								//本地地址
	SOCKADDR_IN				RemoteAddr;								//远端地址	
	time_t					ConnectedTime;							//连接时间	
	time_t					LastActive;								//最后活动时间	
	unsigned __int64		TotleSendBytes;							//发送字节总数	
	unsigned __int64		TotleRecvBytes;							//接收字节总数	
}SOCK_CONTEXT;

typedef struct _PER_IO_CONTEXT
{	
	OVERLAPPEDEX			Overlapped1;							//重叠结构,ACCEPT,CONNECT,SEND共用
	OVERLAPPEDEX			Overlapped2;							//RECV专用
	SOCK_CONTEXT			SockCtx;								//Socket相关信息
	WSABUF					BuffRecv;                               //WSA类型的缓冲区，用于给重叠操作传参数的
	char					Data[IO_BUFF_SIZE];                     //这个是WSABUF里具体存字符的缓冲区	
	CRITICAL_SECTION		SendQueueLock;							//发送队列锁
	SEND_QUEUE*				QueueSend;								//发送队列
	WSABUF					BuffSend;								//用于0字节发送
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
	//取得当前cpu核数 
	DWORD GetProcessorCoreCount();

	//启动iocp socket模型
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
		std::vector<unsigned short>&& PortList,		   //端口列表
		pfnOnConnect OnConnect = nullptr,			   //连接成功回调
		pfnOnDisconnect OnDisconnect = nullptr,        //连接关闭回调
		pfnOnRecvComplete OnRecvComplete = nullptr,    //接收完成回调
		pfnScanComplete OnScanComplete=nullptr,        //扫描完成回调
		pfnShowMessage OnMsg = nullptr,
		unsigned long uConnCount=10000,                //并发数
		const char* lpszStartIp="127.0.0.1",           //起始IP
		const char* lpszStopIp="127.0.0.1",            //终止IP
		const char* lpszLocalIp="0.0.0.0");            //本地地址



	//停止iocp socket模型
	void Stop();
	
protected:
	//是否本实例初始化的网络
	BOOL m_bStartup = FALSE;

	//主线程
	static unsigned int __stdcall _MainThread(LPVOID lParam);
	void RunMain();
	//主线程事件通知 
	HANDLE m_hEvtStopped=nullptr;

	//工作者线程
	static unsigned int __stdcall _WorkerThread(LPVOID lParam);
	void RunWorker();
	//每个线程结束事件通知
	std::map<DWORD,HANDLE> m_mapTidStopEvent;

	//是否正在运行
	BOOL m_bRunning = FALSE;

	//完成端口
	HANDLE m_hIOCP = nullptr;

	//工作模式
	BOOL m_bWorkAsServer = TRUE;

	//server模式参数
	//server ip
	std::string m_strLocalAddr="0.0.0.0"; 
	//server 端口
	unsigned short m_uLocalPort = 8086;
	//当前空闲socket数
	long m_lFreeSock = 0;

	//client模式参数,client模式又分几种：批量连接模式和扫描模式
	int m_nClientType = CLIENT_TYPE_NORMAL;
	//并发连接数
	unsigned long m_uConnCount = 1;

	//1-批量连接模式，对同一个ip和端口发出一定数量的链接，并保持有效链接数量，可用于压力测试，也可以用于普通链接
	//要连接的ip
	std::string m_strRemoteAddr = "127.0.0.1";
	//要连接的端口
	unsigned short m_uRemotePort = 8086;
	long m_lValidSock = 0;
	BOOL m_bComplete = FALSE;

	//2-扫描模式，对范围ip和范围端口进行链接尝试
	struct in_addr m_addrFrom = {127,0,0,1};
	struct in_addr m_addrTo = { 127,0,0,1 };
	std::vector<unsigned short> m_vecPorts;
	long m_lBurstSock = 0;



	//所有socket及其数据的map,增删都在主线程中完成，不需要锁
	std::map<SOCKET, PER_IO_CONTEXT*> m_mapSocketToContex;

	pfnOnConnect cbOnConnect= nullptr;
	pfnOnRecvComplete cbOnRecvComplete=nullptr;
	pfnOnDisconnect cbOnDisconnect = nullptr;
	pfnScanComplete cbOnScanComplete = nullptr;
	pfnShowMessage cbShowMsg = nullptr;

	void ResetContex(PER_IO_CONTEXT* pContex);

	//批量连接
	LPFN_CONNECTEX m_pConnectEx = nullptr;

	unsigned short BindSocket(SOCKET s);

	void RunAsServer();
	void RunAsClient();
	void RunAsScaner();

public:
	BOOL SendTcpData(SOCKET sSocket,DWORD dwLen,char* pData);	
	void KillSocket(SOCKET sSocket);
};

