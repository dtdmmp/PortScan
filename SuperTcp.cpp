#include "stdafx.h"
#include "SuperTcp.h"

CSuperTcp::CSuperTcp()
{

}

CSuperTcp::~CSuperTcp()
{
	Stop();
}

DWORD CSuperTcp::GetProcessorCoreCount()
{
#if (_WIN32_WINNT < 0x0600) //�Ͱ汾��Windows SDKû�ж��� RelationProcessorPackage �ȳ���
#define RelationProcessorPackage 3
#define RelationGroup 4
#endif
	typedef BOOL(WINAPI *LPFN_GLPI)(
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
		PDWORD);
	LPFN_GLPI glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
	if (NULL == glpi)
		return 0;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	DWORD returnLength = 0;
	DWORD processorCoreCount = 0;
	while (true)
	{
		DWORD rc = glpi(buffer, &returnLength);
		if (FALSE == rc)
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				if (buffer)
				{
					free(buffer);
				}
				buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
				if (NULL == buffer)
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
		else
		{
			break;
		}
	}
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
	DWORD byteOffset = 0;
	while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
	{
		switch (ptr->Relationship)
		{
		case RelationProcessorCore:
			++processorCoreCount;
			break;
		default:
			break;
		}
		byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		++ptr;
	}

	if (NULL != buffer)
	{
		free(buffer);
	}
	return processorCoreCount;
}

BOOL CSuperTcp::StartListen(pfnOnConnect OnConnect/*=nullptr*/, 
	pfnOnDisconnect OnDisconnect/*=nullptr*/, 
	pfnOnRecvComplete OnRecvComplete/*=nullptr*/, 
	unsigned short uPort /*= 8086*/, 
	const char* lpszIpAddr /*= "0.0.0.0"*/)
{
	auto ClearHandle = [](HANDLE& h){
		if (h)
		{
			CloseHandle(h);
			h = nullptr;
		}
	};

	cbOnConnect = OnConnect;
	cbOnDisconnect = OnDisconnect;
	cbOnRecvComplete = OnRecvComplete;

	m_bWorkAsServer = TRUE;
	if (m_bWorkAsServer)
	{
		m_strLocalAddr = lpszIpAddr;
		m_uLocalPort = uPort;
	}
	else
	{
		m_strRemoteAddr = lpszIpAddr;
		m_uRemotePort = uPort;
	}

	do 
	{
		//��ʼ��socket��
		WSADATA       wsd;
		int nRet = WSAStartup(MAKEWORD(2, 2), &wsd);
		if (NO_ERROR != nRet)
		{
			break;
		}
		m_bStartup = TRUE;
	
		//������ɶ˿�
		m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (nullptr==m_hIOCP)
		{
			break;
		}

		//�������߳�
		m_hEvtStopped = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		if (nullptr == m_hEvtStopped)
		{
			break;
		}

		unsigned int uTid;
		HANDLE hMainThread = (HANDLE)_beginthreadex(nullptr, 0, _MainThread, this, CREATE_SUSPENDED, &uTid);
		if (nullptr == hMainThread)
		{
			break;
		}

		//�����������߳���
		DWORD dwThreadCount = GetProcessorCoreCount();
		dwThreadCount++;

		//�����������߳�
		std::vector<HANDLE> vecThreadHandles;
		for (DWORD dwCount = 0; dwCount < dwThreadCount;dwCount++)
		{
			HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
			if (hEvent==nullptr)
			{
				break;
			}

			HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, _WorkerThread, this, CREATE_SUSPENDED, &uTid);
			if (hThread==nullptr)
			{
				CloseHandle(hEvent);
				break;
			}
			vecThreadHandles.push_back(hThread);
			m_mapTidStopEvent[uTid]=hEvent;
		}

		if (m_mapTidStopEvent.size()==0)
		{
			break;
		}

		//���������߳�
		m_bRunning = TRUE;
		
		for (size_t i = 0; i < vecThreadHandles.size();i++)
		{
			HANDLE hThread = vecThreadHandles[i];
			ResumeThread(hThread);
			CloseHandle(hThread);
		}

		ResumeThread(hMainThread);
		CloseHandle(hMainThread);

		return TRUE;

	} while (0);	

	ClearHandle(m_hEvtStopped);
	ClearHandle(m_hIOCP);

	if (m_bStartup)
	{
		WSACleanup();
		m_bStartup = FALSE;
	}


	return FALSE;
}



BOOL CSuperTcp::StartConnect(pfnOnConnect OnConnect /*= nullptr*/, 
	pfnOnDisconnect OnDisconnect /*= nullptr*/, 
	pfnOnRecvComplete OnRecvComplete /*= nullptr*/, 
	unsigned long uConnCount/*=10000*/, 
	unsigned short uPort /*= 8086*/, 
	const char* lpszIpAddr /*= "127.0.0.0"*/,
	const char* lpszLocalIp /*= "127.0.0.0"*/)
{
	auto ClearHandle = [](HANDLE& h) {
		if (h)
		{
			CloseHandle(h);
			h = nullptr;
		}
	};

	m_nClientType = CLIENT_TYPE_NORMAL;

	cbOnConnect = OnConnect;
	cbOnDisconnect = OnDisconnect;
	cbOnRecvComplete = OnRecvComplete;

	m_bWorkAsServer = FALSE;
	m_strLocalAddr = lpszLocalIp;
	m_uLocalPort = 0;
	m_strRemoteAddr = lpszIpAddr;
	m_uRemotePort = uPort;

	m_uConnCount = uConnCount;

	do
	{
		//��ʼ��socket��
		WSADATA       wsd;
		int nRet = WSAStartup(MAKEWORD(2, 2), &wsd);
		if (NO_ERROR != nRet)
		{
			break;
		}
		m_bStartup = TRUE;

		//������ɶ˿�
		m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (nullptr == m_hIOCP)
		{
			break;
		}

		//�������߳�
		m_hEvtStopped = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		if (nullptr == m_hEvtStopped)
		{
			break;
		}

		unsigned int uTid;
		HANDLE hMainThread = (HANDLE)_beginthreadex(nullptr, 0, _MainThread, this, CREATE_SUSPENDED, &uTid);
		if (nullptr == hMainThread)
		{
			break;
		}

		//�����������߳���
		DWORD dwThreadCount = GetProcessorCoreCount();
		dwThreadCount++;

		//�����������߳�
		std::vector<HANDLE> vecThreadHandles;
		for (DWORD dwCount = 0; dwCount < dwThreadCount; dwCount++)
		{
			HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
			if (hEvent == nullptr)
			{
				break;
			}

			HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, _WorkerThread, this, CREATE_SUSPENDED, &uTid);
			if (hThread == nullptr)
			{
				CloseHandle(hEvent);
				break;
			}
			vecThreadHandles.push_back(hThread);
			m_mapTidStopEvent[uTid] = hEvent;
		}

		if (m_mapTidStopEvent.size() == 0)
		{
			break;
		}

		//���������߳�
		m_bRunning = TRUE;

		for (size_t i = 0; i < vecThreadHandles.size(); i++)
		{
			HANDLE hThread = vecThreadHandles[i];
			ResumeThread(hThread);
			CloseHandle(hThread);
		}

		ResumeThread(hMainThread);
		CloseHandle(hMainThread);

		return TRUE;

	} while (0);

	ClearHandle(m_hEvtStopped);
	ClearHandle(m_hIOCP);

	if (m_bStartup)
	{
		WSACleanup();
		m_bStartup = FALSE;
	}


	return FALSE;
}



BOOL CSuperTcp::StartScaner(pfnOnConnect OnConnect /*= nullptr*/, /*���ӳɹ��ص� */ 
	pfnOnDisconnect OnDisconnect /*= nullptr*/, /*���ӹرջص� */ 
	pfnOnRecvComplete OnRecvComplete /*= nullptr*/, /*������ɻص� */ 
	pfnScanComplete OnScanComplete/*=nullptr*/, /*ɨ����ɻص� */ 
	unsigned long uConnCount/*=10000*/, /*������ */ 
	const char* lpszStartIp/*="127.0.0.1"*/, /*��ʼIP */ 
	const char* lpszStopIp/*="127.0.0.1"*/, /*��ֹIP */ 
	unsigned short uStartPort/*=1*/, /*��ʼ�˿� */ 
	unsigned short uStopPort/*=65535*/, /*��ֹ�˿� */ 
	const char* lpszLocalIp/*="127.0.0.1"*/)
{
	auto ClearHandle = [](HANDLE& h) {
		if (h)
		{
			CloseHandle(h);
			h = nullptr;
		}
	};

	m_nClientType = CLIENT_TYPE_SCANER;

	cbOnConnect = OnConnect;
	cbOnDisconnect = OnDisconnect;
	cbOnRecvComplete = OnRecvComplete;
	cbOnScanComplete = OnScanComplete;

	m_bWorkAsServer = FALSE;
	m_strLocalAddr = lpszLocalIp;
	m_uLocalPort = 0;

	m_addrFrom.S_un.S_addr = inet_addr(lpszStartIp);
	m_addrTo.S_un.S_addr = inet_addr(lpszStopIp);
	m_uPortFrom = uStartPort;
	m_uPortTo = uStopPort;

	m_uConnCount = uConnCount;

	do
	{
		//��ʼ��socket��
		WSADATA       wsd;
		int nRet = WSAStartup(MAKEWORD(2, 2), &wsd);
		if (NO_ERROR != nRet)
		{
			break;
		}
		m_bStartup = TRUE;

		//������ɶ˿�
		m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (nullptr == m_hIOCP)
		{
			break;
		}

		//�������߳�
		m_hEvtStopped = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		if (nullptr == m_hEvtStopped)
		{
			break;
		}

		unsigned int uTid;
		HANDLE hMainThread = (HANDLE)_beginthreadex(nullptr, 0, _MainThread, this, CREATE_SUSPENDED, &uTid);
		if (nullptr == hMainThread)
		{
			break;
		}

		//�����������߳���
		DWORD dwThreadCount = GetProcessorCoreCount();
		dwThreadCount++;

		//�����������߳�
		std::vector<HANDLE> vecThreadHandles;
		for (DWORD dwCount = 0; dwCount < dwThreadCount; dwCount++)
		{
			HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
			if (hEvent == nullptr)
			{
				break;
			}

			HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, _WorkerThread, this, CREATE_SUSPENDED, &uTid);
			if (hThread == nullptr)
			{
				CloseHandle(hEvent);
				break;
			}
			vecThreadHandles.push_back(hThread);
			m_mapTidStopEvent[uTid] = hEvent;
		}

		if (m_mapTidStopEvent.size() == 0)
		{
			break;
		}

		//���������߳�
		m_bRunning = TRUE;

		for (size_t i = 0; i < vecThreadHandles.size(); i++)
		{
			HANDLE hThread = vecThreadHandles[i];
			ResumeThread(hThread);
			CloseHandle(hThread);
		}

		ResumeThread(hMainThread);
		CloseHandle(hMainThread);

		return TRUE;

	} while (0);

	ClearHandle(m_hEvtStopped);
	ClearHandle(m_hIOCP);

	if (m_bStartup)
	{
		WSACleanup();
		m_bStartup = FALSE;
	}


	return FALSE;
}

void CSuperTcp::Stop()
{
	m_bRunning = FALSE;

	//vista���Ժ�Ĳ���ϵͳ���ر�iocp�����GetQueuedCompletionStatus�����Ϸ��أ���vista��ǰ��ϵͳȴ����
	//Ϊ�˼��ݣ��˴�ͳһ�����˳��źŵ������߳�
	for (size_t i = 0; i < m_mapTidStopEvent.size();i++)
	{
		PostQueuedCompletionStatus(m_hIOCP, 0, SIGNAL_EXIT, nullptr);
	}
	
	//�ر���ɶ˿�
	CloseHandle(m_hIOCP);
	m_hIOCP = nullptr;

	//�ȴ�worker�߳̽���
	auto it = m_mapTidStopEvent.begin();
	while (it!=m_mapTidStopEvent.end())
	{
		HANDLE hEvent = it->second;
		if (hEvent)
		{
			while (true)
			{
				DWORD dwWait=WaitForSingleObject(hEvent, 0);
				if (dwWait==WAIT_OBJECT_0)
				{
					break;
				}

				//������Ϣ��ʹ�ò����ڿ���
				MSG msg;
				while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			
			CloseHandle(hEvent);
		}

		it++;
	}

	//�����߳�ID-ֹͣ�¼�map
	decltype(m_mapTidStopEvent) mapTemp;
	m_mapTidStopEvent.swap(mapTemp);

	//�ȴ����߳̽���
	if (m_hEvtStopped)
	{
		while (true)
		{
			DWORD dwWait = WaitForSingleObject(m_hEvtStopped, 0);
			if (dwWait==WAIT_OBJECT_0)
			{
				break;
			}

			//������Ϣ��ʹ�ò����ڿ���
			MSG msg;
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		CloseHandle(m_hEvtStopped);
		m_hEvtStopped = nullptr;
	}

	//����sockets
	auto pairSocketContex = m_mapSocketToContex.begin();
	while (pairSocketContex != m_mapSocketToContex.end())
	{
		auto pContex = pairSocketContex->second;
		if (pContex)
		{
			if (pContex->SockCtx.Socket!=INVALID_SOCKET)
			{
				closesocket(pContex->SockCtx.Socket);
			}

			EnterCriticalSection(&pContex->SendQueueLock);
			if (pContex->QueueSend)
			{
				while (pContex->QueueSend->size())
				{
					WSABUF* pBuff = pContex->QueueSend->front();
					if (pBuff)
					{
						delete[](char*)pBuff;
					}
					pContex->QueueSend->pop();
				}

				delete pContex->QueueSend;
				pContex->QueueSend = nullptr;
			}
			LeaveCriticalSection(&pContex->SendQueueLock);

			DeleteCriticalSection(&pContex->SendQueueLock);

			delete pContex;
		}

		pairSocketContex++;
	}

	decltype(m_mapSocketToContex) mapTemp1;
	m_mapSocketToContex.swap(mapTemp1);

	m_lFreeSock = 0;

	if (m_bStartup)
	{
		WSACleanup();
		m_bStartup = FALSE;
	}
}

unsigned int __stdcall CSuperTcp::_MainThread(LPVOID lParam)
{
	CSuperTcp* pThis = static_cast<CSuperTcp*>(lParam);
	if (pThis)
	{
		if (pThis->m_hEvtStopped)
		{
			ResetEvent(pThis->m_hEvtStopped);
		}

		pThis->RunMain();

		if (pThis->m_hEvtStopped)
		{
			SetEvent(pThis->m_hEvtStopped);
		}
	}

	return 0;
}

void CSuperTcp::RunAsServer()
{
	SOCKET sListenSock = INVALID_SOCKET;

	//��ʼ������socket
	sListenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (sListenSock != INVALID_SOCKET)
	{
		//������ɶ˿�
		if (CreateIoCompletionPort((HANDLE)sListenSock, m_hIOCP, sListenSock, 0) != nullptr)
		{
			//����bind
			struct sockaddr_in sa;
			sa.sin_family = AF_INET;
			sa.sin_addr.s_addr = inet_addr(m_strLocalAddr.c_str());
			sa.sin_port = htons(m_uLocalPort);

			int nRet = bind(sListenSock, (struct sockaddr *)&sa, sizeof(sa));
			if (nRet != SOCKET_ERROR)
			{
				nRet = listen(sListenSock, SOMAXCONN);
				if (nRet != INVALID_SOCKET)
				{
					DWORD dwTick = GetTickCount();
					while (m_bRunning)
					{
						//���Ͷ�ݵ�SOCKET�Ƿ��ѱ�����,����1000������socket
						if (m_lFreeSock < PREPARE_SOCKET_COUNT && m_mapSocketToContex.size() < MAX_SOCKET_COUNT)
						{
							SOCKET sNewSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
							if (sNewSock != INVALID_SOCKET)
							{
								PER_IO_CONTEXT* pContex = nullptr;

								BOOL bIsReUse = FALSE;
								//socket���ܻḴ��
								auto it = m_mapSocketToContex.find(sNewSock);
								if (it != m_mapSocketToContex.end())
								{
									pContex = it->second;
									bIsReUse = TRUE;
								}
								else
								{
									pContex = new PER_IO_CONTEXT;
								}

								if (pContex)
								{
									//���Ǹ��õĽṹ����ʼ��
									if (!bIsReUse)
									{
										memset(pContex, 0x00, sizeof(PER_IO_CONTEXT));
										InitializeCriticalSection(&pContex->SendQueueLock);
										pContex->QueueSend = new SEND_QUEUE;

										m_mapSocketToContex[sNewSock] = pContex;
									}
									//�Ǹ��õĽṹ�����ͷŷ��Ͷ���
									else
									{
										EnterCriticalSection(&pContex->SendQueueLock);
										if (pContex->QueueSend)
										{
											while (pContex->QueueSend->size())
											{
												WSABUF* pBuff = pContex->QueueSend->front();
												if (pBuff)
												{
													delete[](char*)pBuff;
												}
												pContex->QueueSend->pop();
											}
										}
										LeaveCriticalSection(&pContex->SendQueueLock);
									}

									pContex->SockCtx.Status = ss_init;
									pContex->SockCtx.Socket = sNewSock;
									pContex->Overlapped1.Type = OP_ACPT;
									pContex->BuffRecv.len = sizeof(pContex->Data);
									pContex->BuffRecv.buf = pContex->Data;

									DWORD dwBytesRecv;
									BOOL bRet = AcceptEx(sListenSock, sNewSock, pContex->BuffRecv.buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytesRecv, &pContex->Overlapped1.Overlapped);
									if (!bRet)
									{
										if (ERROR_IO_PENDING != GetLastError())
										{
											DWORD dwErr = GetLastError();
											printf("AcceptEx with error code=%08x\n", dwErr);
											ResetContex(pContex);
											continue;
										}
									}

									InterlockedIncrement(&m_lFreeSock);
								}
							}
						}
						else
						{
							//���е�ʱ��ͳ������
							DWORD dwNewTick = GetTickCount();
							if (dwNewTick - dwTick > SCAN_CYCLE)
							{
								int nFree = 0;
								int nShutdown = 0;
								int nConnected = 0;
								auto pairSocketContex = m_mapSocketToContex.begin();
								while (pairSocketContex != m_mapSocketToContex.end())
								{
									SOCKET s = pairSocketContex->first;
									PER_IO_CONTEXT* pContex = pairSocketContex->second;

									if (pContex)
									{
										if (pContex->SockCtx.Status == ss_down)
										{
											nShutdown++;

											m_mapSocketToContex.erase(pairSocketContex++);

											if (pContex->SockCtx.Socket != INVALID_SOCKET)
											{
												closesocket(pContex->SockCtx.Socket);
											}

											EnterCriticalSection(&pContex->SendQueueLock);
											if (pContex->QueueSend)
											{
												while (pContex->QueueSend->size())
												{
													WSABUF* pBuff = pContex->QueueSend->front();
													if (pBuff)
													{
														delete[](char*)pBuff;
													}
													pContex->QueueSend->pop();
												}

												delete pContex->QueueSend;
												pContex->QueueSend = nullptr;
											}
											LeaveCriticalSection(&pContex->SendQueueLock);

											DeleteCriticalSection(&pContex->SendQueueLock);

											delete pContex;
										}
										else
										{
											if (pContex->SockCtx.Status == ss_init)
											{
												nFree++;
											}

											if (pContex->SockCtx.Status == ss_buzy)
											{
												nConnected++;
											}

											pairSocketContex++;
										}
									}

								}

								printf("current lFree=%d, free=%d,connected=%d,expired=%d\n", m_lFreeSock, nFree, nConnected, nShutdown);

								//ʹ��nFree����m_lFreeSock
								InterlockedExchange(&m_lFreeSock, nFree);


								dwTick = GetTickCount();
							}
							else
							{
								Sleep(100);
							}
						}
					}
				}
			}
		}
		closesocket(sListenSock);
	}
}

void CSuperTcp::RunAsClient()
{
	DWORD dwBytesRet;
	//���Ū��socket,��ΪWSAIoctlҪ�õ�
	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (s != INVALID_SOCKET)
	{
		//ȡ��ConnectEx����ָ��
		GUID guidConnectEx = WSAID_CONNECTEX;
		int nRet = WSAIoctl(s,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guidConnectEx,
			sizeof(guidConnectEx),
			&m_pConnectEx,
			sizeof(m_pConnectEx),
			&dwBytesRet,
			nullptr,
			nullptr);

		closesocket(s);
	}

	if (m_pConnectEx == nullptr)
	{
		printf("Can not get ConnectEx pointer\n");
		return;
	}

	//��ʼ����
	while (m_bRunning)
	{
		DWORD dwTick = GetTickCount();
		while (m_bRunning)
		{
			//����Ƿ���Ͷ���㹻������
			if (m_lValidSock < (long)m_uConnCount && m_mapSocketToContex.size() < MAX_SOCKET_COUNT)
			{
				SOCKET sNewSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
				if (sNewSock == INVALID_SOCKET)
				{
					printf("create new socket fail %d\n", WSAGetLastError());
					break;
				}

				//������ɶ˿�
				if (!CreateIoCompletionPort((HANDLE)sNewSock, m_hIOCP, sNewSock, 0))
				{
					closesocket(sNewSock);
					continue;
				}

				//����bind
				unsigned short uPortBind = BindSocket(sNewSock);
				if (uPortBind==-1)
				{
					printf("bind new socket fail %d\n", WSAGetLastError());
					closesocket(sNewSock);
					continue;
				}

				PER_IO_CONTEXT* pContex = nullptr;

				BOOL bIsReUse = FALSE;
				//socket���ܻḴ��
				auto it = m_mapSocketToContex.find(sNewSock);
				if (it != m_mapSocketToContex.end())
				{
					pContex = it->second;
					bIsReUse = TRUE;
				}
				else
				{
					pContex = new PER_IO_CONTEXT;
					if (!pContex)
					{
						closesocket(sNewSock);
						break;
					}
				}

				//���Ǹ��õĽṹ����ʼ��
				if (!bIsReUse)
				{
					memset(pContex, 0x00, sizeof(PER_IO_CONTEXT));
					InitializeCriticalSection(&pContex->SendQueueLock);
					pContex->QueueSend = new SEND_QUEUE;

					m_mapSocketToContex[sNewSock] = pContex;
				}
				//�Ǹ��õĽṹ�����ͷŷ��Ͷ���
				else
				{
					EnterCriticalSection(&pContex->SendQueueLock);
					if (pContex->QueueSend)
					{
						while (pContex->QueueSend->size())
						{
							WSABUF* pBuff = pContex->QueueSend->front();
							if (pBuff)
							{
								delete[](char*)pBuff;
							}
							pContex->QueueSend->pop();
						}
					}
					LeaveCriticalSection(&pContex->SendQueueLock);
				}

				pContex->SockCtx.Status = ss_init;
				pContex->SockCtx.Socket = sNewSock;
				pContex->Overlapped1.Type = OP_CONN;
				struct sockaddr_in sa;
				sa.sin_family = AF_INET;
				sa.sin_addr.s_addr = inet_addr(m_strRemoteAddr.c_str());
				sa.sin_port = htons(m_uRemotePort);
				memcpy(&pContex->SockCtx.RemoteAddr, &sa, sizeof(pContex->SockCtx.RemoteAddr));

				pContex->SockCtx.LocalAddr.sin_addr.S_un.S_addr = inet_addr(m_strLocalAddr.c_str());
				pContex->SockCtx.LocalAddr.sin_port = uPortBind;

				dwBytesRet = 0;
				BOOL bRet = m_pConnectEx(sNewSock,
					(struct sockaddr *)&sa,
					sizeof(sa),
					nullptr,
					0,
					&dwBytesRet,
					&pContex->Overlapped1.Overlapped);
				if (!bRet)
				{
					if (ERROR_IO_PENDING != GetLastError())
					{
						DWORD dwErr = GetLastError();
						printf("ConnectEx with error code=%d\n", dwErr);
						ResetContex(pContex);
						continue;
					}
				}

				InterlockedIncrement(&m_lValidSock);

			}
			else
			{
				//���е�ʱ��ͳ������
				DWORD dwNewTick = GetTickCount();
				if (dwNewTick - dwTick > SCAN_CYCLE)
				{
					int nFree = 0;
					int nShutdown = 0;
					int nConnected = 0;
					int nValid = 0;
					auto pairSocketContex = m_mapSocketToContex.begin();
					while (pairSocketContex != m_mapSocketToContex.end())
					{
						SOCKET s = pairSocketContex->first;
						PER_IO_CONTEXT* pContex = pairSocketContex->second;

						if (pContex)
						{
							if (pContex->SockCtx.Status == ss_down)
							{
								nShutdown++;

								m_mapSocketToContex.erase(pairSocketContex++);

								if (pContex->SockCtx.Socket != INVALID_SOCKET)
								{
									closesocket(pContex->SockCtx.Socket);
								}

								EnterCriticalSection(&pContex->SendQueueLock);
								if (pContex->QueueSend)
								{
									while (pContex->QueueSend->size())
									{
										WSABUF* pBuff = pContex->QueueSend->front();
										if (pBuff)
										{
											delete[](char*)pBuff;
										}
										pContex->QueueSend->pop();
									}

									delete pContex->QueueSend;
									pContex->QueueSend = nullptr;
								}
								LeaveCriticalSection(&pContex->SendQueueLock);

								DeleteCriticalSection(&pContex->SendQueueLock);

								delete pContex;
							}
							else
							{
								if (pContex->SockCtx.Status == ss_init)
								{
									nFree++;
									nValid++;
								}

								if (pContex->SockCtx.Status == ss_buzy)
								{
									nConnected++;
									nValid++;
								}

								pairSocketContex++;
							}
						}

					}

					printf("current  Connecting=%d,Connected=%d,expired=%d\n", nFree, nConnected, nShutdown);

					//ʹ��nValid����m_lValidSock�����ǳ�ʼ�����߳ɹ�����״̬��socket������Ч��
					InterlockedExchange(&m_lValidSock, nValid);

					dwTick = GetTickCount();
				}
				else
				{
					Sleep(100);
				}
			}
		}
	}
}

void CSuperTcp::RunAsScaner()
{
	DWORD dwBytesRet;
	//���Ū��socket,��ΪWSAIoctlҪ�õ�
	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (s != INVALID_SOCKET)
	{
		//ȡ��ConnectEx����ָ��
		GUID guidConnectEx = WSAID_CONNECTEX;
		int nRet = WSAIoctl(s,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guidConnectEx,
			sizeof(guidConnectEx),
			&m_pConnectEx,
			sizeof(m_pConnectEx),
			&dwBytesRet,
			nullptr,
			nullptr);

		closesocket(s);
	}

	if (m_pConnectEx == nullptr)
	{
		printf("Can not get ConnectEx pointer\n");
		return;
	}

	auto CheckIP = [](const struct in_addr& addr)->BOOL {
		if (addr.S_un.S_un_b.s_b1>223)
		{
			return FALSE;
		}
		if (addr.S_un.S_un_b.s_b2==255)
		{
			return FALSE;
		}
		if (addr.S_un.S_un_b.s_b3==255)
		{
			return FALSE;
		}
		
		if (addr.S_un.S_un_b.s_b4==255 || addr.S_un.S_un_b.s_b4==0)
		{
			return FALSE;
		}

		return TRUE;
	};

	auto IncreaseIP = [](struct in_addr& addr)->BOOL {
		
		if (addr.S_un.S_un_b.s_b4<254)
		{
			addr.S_un.S_un_b.s_b4++;
		}
		else
		{
			addr.S_un.S_un_b.s_b4 = 1;

			if (addr.S_un.S_un_b.s_b3<254)
			{
				addr.S_un.S_un_b.s_b3++;
			}
			else
			{
				addr.S_un.S_un_b.s_b3 = 0;
				
				if (addr.S_un.S_un_b.s_b2<254)
				{
					addr.S_un.S_un_b.s_b2++;
				}
				else
				{
					addr.S_un.S_un_b.s_b2 = 0;

					if (addr.S_un.S_un_b.s_b1<223)
					{
						addr.S_un.S_un_b.s_b1++;
					}
					else
					{
						addr.S_un.S_un_b.s_b1++;
						return FALSE;
					}
				}
			}

			return TRUE;
		}

		return TRUE;
	};

	auto CompareIP = [](const struct in_addr& addr1, const struct in_addr& addr2)->int {
		if (addr1.S_un.S_un_b.s_b1==addr2.S_un.S_un_b.s_b1)
		{
			if (addr1.S_un.S_un_b.s_b2==addr2.S_un.S_un_b.s_b2)
			{
				if (addr1.S_un.S_un_b.s_b3 == addr2.S_un.S_un_b.s_b3)
				{
					if (addr1.S_un.S_un_b.s_b4 == addr2.S_un.S_un_b.s_b4)
					{
						return 0;
					}
					else
					{
						return addr1.S_un.S_un_b.s_b4 < addr2.S_un.S_un_b.s_b4 ? -1 : 1;
					}
				}
				else
				{
					return addr1.S_un.S_un_b.s_b3 < addr2.S_un.S_un_b.s_b3 ? -1 : 1;
				}
			}
			else
			{
				return addr1.S_un.S_un_b.s_b2 < addr2.S_un.S_un_b.s_b2 ? -1 : 1;
			}
		}
		else
		{
			return addr1.S_un.S_un_b.s_b1 < addr2.S_un.S_un_b.s_b1 ? -1 : 1;
		}
	};

	DWORD dwTick = GetTickCount();
	//��һ��ѭ��:IP
	struct in_addr adCurrent;
	adCurrent.S_un.S_addr = m_addrFrom.S_un.S_addr;
	do
	{
		if (!CheckIP(adCurrent))
		{
			printf("Invalid ip address %d.%d.%d.%d\n",adCurrent.S_un.S_un_b.s_b1,adCurrent.S_un.S_un_b.s_b2,adCurrent.S_un.S_un_b.s_b3,adCurrent.S_un.S_un_b.s_b4);
			if (cbOnScanComplete)
			{
				cbOnScanComplete();
			}
			break;
		}

		//printf("processing %d.%d.%d.%d...\n", adCurrent.S_un.S_un_b.s_b1, adCurrent.S_un.S_un_b.s_b2, adCurrent.S_un.S_un_b.s_b3, adCurrent.S_un.S_un_b.s_b4);

		//�ڶ���ѭ��:�˿�
		unsigned short uCurrentPort = m_uPortFrom;
		do 
		{			
			if (uCurrentPort==0)
			{
				break;
			}

			//����Ƿ���Ͷ���㹻������
			if (m_lBurstSock < (long)m_uConnCount && m_mapSocketToContex.size() < MAX_SOCKET_COUNT)
			{
				SOCKET sNewSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
				if (sNewSock == INVALID_SOCKET)
				{
					printf("create new socket fail %d\n", WSAGetLastError());
					break;
				}

				//������ɶ˿�
				if (!CreateIoCompletionPort((HANDLE)sNewSock, m_hIOCP, sNewSock, 0))
				{
					DWORD oerr = GetLastError();

					closesocket(sNewSock);
					continue;
				}

				//����bind
				unsigned short uPortBind = BindSocket(sNewSock);
				if (uPortBind==-1)
				{
					printf("bind new socket fail %d\n", WSAGetLastError());
					closesocket(sNewSock);
					continue;
				}

				PER_IO_CONTEXT* pContex = nullptr;

				BOOL bIsReUse = FALSE;
				//socket���ܻḴ��
				auto it = m_mapSocketToContex.find(sNewSock);
				if (it != m_mapSocketToContex.end())
				{
					pContex = it->second;
					bIsReUse = TRUE;
				}
				else
				{
					pContex = new PER_IO_CONTEXT;
					if (!pContex)
					{
						closesocket(sNewSock);
						break;
					}
				}

				//���Ǹ��õĽṹ����ʼ��
				if (!bIsReUse)
				{
					memset(pContex, 0x00, sizeof(PER_IO_CONTEXT));
					InitializeCriticalSection(&pContex->SendQueueLock);
					pContex->QueueSend = new SEND_QUEUE;

					m_mapSocketToContex[sNewSock] = pContex;
				}
				//�Ǹ��õĽṹ�����ͷŷ��Ͷ���
				else
				{
					EnterCriticalSection(&pContex->SendQueueLock);
					if (pContex->QueueSend)
					{
						while (pContex->QueueSend->size())
						{
							WSABUF* pBuff = pContex->QueueSend->front();
							if (pBuff)
							{
								delete[](char*)pBuff;
							}
							pContex->QueueSend->pop();
						}
					}
					LeaveCriticalSection(&pContex->SendQueueLock);
				}

				pContex->Overlapped1.Type = OP_CONN;
				pContex->SockCtx.Status = ss_init;
				pContex->SockCtx.Socket = sNewSock;
				struct sockaddr_in sa;
				sa.sin_family = AF_INET;
				sa.sin_addr.s_addr = adCurrent.S_un.S_addr;
				sa.sin_port = htons(uCurrentPort);
				memcpy(&pContex->SockCtx.RemoteAddr, &sa, sizeof(pContex->SockCtx.RemoteAddr));

				pContex->SockCtx.LocalAddr.sin_addr.S_un.S_addr= inet_addr(m_strLocalAddr.c_str());
				pContex->SockCtx.LocalAddr.sin_port = uPortBind;

			
				dwBytesRet = 0;
				BOOL bRet = m_pConnectEx(sNewSock,
					(struct sockaddr *)&sa,
					sizeof(sa),
					nullptr,
					0,
					&dwBytesRet,
					&pContex->Overlapped1.Overlapped);
				if (!bRet)
				{
					if (ERROR_IO_PENDING != GetLastError())
					{
						DWORD dwErr = GetLastError();
						printf("ConnectEx with error code=%d\n", dwErr);
						ResetContex(pContex);
						continue;
					}
				}
				InterlockedIncrement(&m_lBurstSock);

			}
			else
			{
				//���е�ʱ��ͳ������
				DWORD dwNewTick = GetTickCount();
				if (dwNewTick - dwTick > SCAN_CYCLE)
				{
					int nFree = 0;
					int nShutdown = 0;
					int nConnected = 0;
					int nValid = 0;
					auto pairSocketContex = m_mapSocketToContex.begin();
					while (pairSocketContex != m_mapSocketToContex.end())
					{
						SOCKET s = pairSocketContex->first;
						PER_IO_CONTEXT* pContex = pairSocketContex->second;

						if (pContex)
						{
							if (pContex->SockCtx.Status == ss_down)
							{
								nShutdown++;

								m_mapSocketToContex.erase(pairSocketContex++);

								if (pContex->SockCtx.Socket != INVALID_SOCKET)
								{
									closesocket(pContex->SockCtx.Socket);
								}

								EnterCriticalSection(&pContex->SendQueueLock);
								if (pContex->QueueSend)
								{
									while (pContex->QueueSend->size())
									{
										WSABUF* pBuff = pContex->QueueSend->front();
										if (pBuff)
										{
											delete[](char*)pBuff;
										}
										pContex->QueueSend->pop();
									}

									delete pContex->QueueSend;
									pContex->QueueSend = nullptr;
								}
								LeaveCriticalSection(&pContex->SendQueueLock);

								DeleteCriticalSection(&pContex->SendQueueLock);

								delete pContex;
							}
							else
							{
								if (pContex->SockCtx.Status == ss_init)
								{
									nFree++;
									nValid++;
								}

								if (pContex->SockCtx.Status == ss_buzy)
								{
									nConnected++;
									nValid++;
								}

								pairSocketContex++;
							}
						}

					}

					printf("current  Connecting=%d,Connected=%d,expired=%d\n", nFree, nConnected, nShutdown);

					//ʹ��nValid����m_lValidSock�����ǳ�ʼ�����߳ɹ�����״̬��socket������Ч��
					InterlockedExchange(&m_lBurstSock, nValid);

					dwTick = GetTickCount();
				}
				else
				{
					Sleep(100);
				}
			}


			if (uCurrentPort<m_uPortTo)
			{
				uCurrentPort++;
			}
			else
			{
				break;
			}
			
		} while (m_bRunning);



		if (CompareIP(adCurrent,m_addrTo)<0)
		{
			IncreaseIP(adCurrent);
		}
		else
		{
			if (cbOnScanComplete)
			{
				cbOnScanComplete();
			}
			break;
		}

	} while (m_bRunning);
	
}

void CSuperTcp::RunMain()
{
	SOCKET sListenSock=INVALID_SOCKET;

	auto SleepEx = [this](const DWORD dwTime){
		int nCount = dwTime / 100;
		for (int i = 0; i <= nCount && m_bRunning; i++)
		{
			Sleep(100);
		}
	};

	//��Ϊ����������
	if (m_bWorkAsServer)
	{
		RunAsServer();
	}
	else
	{
		if (m_nClientType==CLIENT_TYPE_NORMAL)
		{
			RunAsClient();
		}
		else if(m_nClientType==CLIENT_TYPE_SCANER)
		{
			RunAsScaner();
		}
	}
}

unsigned int __stdcall CSuperTcp::_WorkerThread(LPVOID lParam)
{
	CSuperTcp* pThis = static_cast<CSuperTcp*>(lParam);
	if (pThis)
	{
		DWORD dwThreadId = GetCurrentThreadId();
		
		auto pairTidEvent = pThis->m_mapTidStopEvent.find(dwThreadId);
		if (pairTidEvent!=pThis->m_mapTidStopEvent.end())
		{
			HANDLE hEvent = pairTidEvent->second;
			if (hEvent)
			{
				ResetEvent(hEvent);
			}

			pThis->RunWorker();

			if (hEvent)
			{
				SetEvent(hEvent);
			}
		}
	}

	return 0;
}

void CSuperTcp::RunWorker()
{
	while (m_bRunning)
	{
		DWORD dwBytesTransferred = 0;
		ULONG_PTR CompletionKey = 0;
		OVERLAPPED* pOverlapped = nullptr;
		OVERLAPPEDEX* pOverlappedEx = nullptr;
		PER_IO_CONTEXT* pContex = nullptr;
		SOCKET_OPERATION_TYPE Type = OP_NULL;

		BOOL bRet = GetQueuedCompletionStatus(m_hIOCP, &dwBytesTransferred, &CompletionKey,&pOverlapped,INFINITE);

		if (SIGNAL_EXIT == CompletionKey)
		{
			break;
		}

		if (pOverlapped)
		{
			pOverlappedEx = CONTAINING_RECORD(pOverlapped, OVERLAPPEDEX, Overlapped);
			Type = pOverlappedEx->Type;
			if (Type==OP_RECV)
			{
				pContex = CONTAINING_RECORD(pOverlappedEx, PER_IO_CONTEXT, Overlapped2);
			}
			else
			{
				pContex = CONTAINING_RECORD(pOverlappedEx, PER_IO_CONTEXT, Overlapped1);
			}
			
		}
		else
		{
			continue;
		}

		if (bRet)
		{
			//�յ�0�ֽ����ݣ���ʱҪ�жϴ���
			if (dwBytesTransferred == 0 && OP_RECV == Type)
			{
				//���ǹ���Ͷ�ݵ�0�ֽڽ���
				if (pContex->BuffRecv.len != 0)
				{
					ResetContex(pContex);
					continue;
				}
			}

			if (OP_RECV == Type)
			{

				pContex->SockCtx.LastActive = time(nullptr);
				pContex->SockCtx.TotleRecvBytes += dwBytesTransferred;

				//�����ݣ���������
				if (dwBytesTransferred)
				{
					if (cbOnRecvComplete)
					{
						SOCK_CONTEXT TmpSockCtx;
						memcpy(&TmpSockCtx, &pContex->SockCtx, sizeof(TmpSockCtx));
						cbOnRecvComplete(pContex->SockCtx.Socket, dwBytesTransferred, pContex->Data,TmpSockCtx);
					}
				}

				//׼����һ�ν��� 
				pContex->Overlapped2.Type = OP_RECV;

				//������һ�ν��ճ���
				//�������0�ֽڣ�Ͷ��һҳ����
				if (dwBytesTransferred == 0)
				{					
					pContex->BuffRecv.len = sizeof(pContex->Data);
				}
				//�������һҳ��ҲͶ��һҳ����
				else if (dwBytesTransferred == IO_BUFF_SIZE)
				{
					pContex->BuffRecv.len = sizeof(pContex->Data);
				}
				//����ղ���һҳ��˵��û�������ˣ�Ͷ��0�ֽڽ���
				else 
				{					
					pContex->BuffRecv.len = 0;
				}
				pContex->BuffRecv.buf = pContex->Data;

				//����Ͷ����һ�ν���

				DWORD dwBytes = 0;
				DWORD dwFlag = 0;
				int nRet = WSARecv(pContex->SockCtx.Socket, &pContex->BuffRecv, 1, &dwBytes, &dwFlag, &pContex->Overlapped2.Overlapped, nullptr);
				if (nRet==SOCKET_ERROR)
				{
					int nErr = WSAGetLastError();
					if (WSA_IO_PENDING != nErr)
					{
						if (nErr!= WSAENOTSOCK) //socket�Ѿ��ͷ�
						{
							printf("WSARecv error with %d on socket #%d\n", nErr,pContex->SockCtx.Socket);
						}
						ResetContex(pContex);
					}
				}				
			}
			else if (OP_SEND == Type)
			{
				pContex->SockCtx.TotleSendBytes += dwBytesTransferred;
				pContex->SockCtx.LastActive = time(nullptr);

				
				EnterCriticalSection(&pContex->SendQueueLock);
				if (pContex->QueueSend)
				{
					if (pContex->QueueSend->size())
					{
						//ȡ����һ�����ݽ��з���
						if (pContex->QueueSend->size())
						{
							WSABUF* pBuff = pContex->QueueSend->front();
							if (pBuff)
							{
								pContex->Overlapped1.Type = OP_SEND;
								DWORD dwBytes = 0;
								DWORD dwFlag = 0;

								int nRet = WSASend(pContex->SockCtx.Socket, pBuff, 1, &dwBytes, dwFlag, &pContex->Overlapped1.Overlapped, nullptr);
								if (nRet==SOCKET_ERROR)
								{
									int nErr = WSAGetLastError();
									if (WSA_IO_PENDING != nErr)
									{
										if (nErr != WSAENOTSOCK) //socket�Ѿ��ͷ�
										{
											printf("WSASend error with %d on socket #%d\n", nErr, pContex->SockCtx.Socket);
										}
										ResetContex(pContex);
									}
								}

								//ע�⣬�˴��ǹؼ������ܷ��ͳɲ��ɹ�������֮�󶼽�����ɾ��
								//���nRet=0�������Ѿ�������ϣ�ɾ������û������
								//���nRet!=0��WSAGetLastError()==WSA_IO_PENDING��˵�������ѻ��棬pBuff�ᱻ�����ɷǷ�ҳ�ڴ棬ɾ��û������
								//���nRet!=0, WSAGetLastError()!=WSA_IO_PENDING��˵�����ͳ�������Ҳ��Ҫ�ͷ�
								delete[] (char*)pBuff;
							}			
							pContex->QueueSend->pop();
						}
					}
				}
				LeaveCriticalSection(&pContex->SendQueueLock);
			}
			else if (OP_ACPT == Type)
			{
				//Accept�ɹ�������socket����
				InterlockedDecrement(&m_lFreeSock);

				//��ӡ���ص�ַ��Զ�˵�ַ
				sockaddr* pLocalAddr = nullptr;
				int nLocal = 0;
				sockaddr* pRemoteAddr = nullptr;
				int nRemote = 0;
				GetAcceptExSockaddrs(pContex->Data, dwBytesTransferred, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &pLocalAddr, &nLocal, &pRemoteAddr, &nRemote);
				memcpy(&pContex->SockCtx.LocalAddr, pLocalAddr, sizeof(pContex->SockCtx.LocalAddr));
				memcpy(&pContex->SockCtx.RemoteAddr, pRemoteAddr, sizeof(pContex->SockCtx.RemoteAddr));

				//����ʱ��
				pContex->SockCtx.ConnectedTime = time(nullptr);
				pContex->SockCtx.LastActive = time(nullptr);

				//����״̬
				pContex->SockCtx.Status = ss_buzy;

				//��socket������IOCP
				HANDLE hIocp = CreateIoCompletionPort((HANDLE)pContex->SockCtx.Socket, m_hIOCP, 0, 0);
				if (!hIocp)
				{
					printf("combine accepted socket with iocp fail %d\n",GetLastError());
					ResetContex(pContex);
					continue;
				}

				//Ͷ��0�ֽڽ���
				DWORD dwBytes = 0;
				DWORD dwFlag=0;
				pContex->Overlapped2.Type = OP_RECV;
				pContex->BuffRecv.len = 0;
				pContex->BuffRecv.buf = pContex->Data;
				int nRet=WSARecv(pContex->SockCtx.Socket, &pContex->BuffRecv, 1, &dwBytes, &dwFlag, &pContex->Overlapped2.Overlapped, nullptr);
				if (nRet==SOCKET_ERROR)
				{
					if (WSA_IO_PENDING != WSAGetLastError())
					{
						printf("recv from socket %d error,the socket will be closed\n",pContex->SockCtx.Socket);
						ResetContex(pContex);
						continue;
					}
				}

				if (cbOnConnect)
				{
					SOCK_CONTEXT tmpSockCtx;
					memcpy(&tmpSockCtx, &pContex->SockCtx, sizeof(SOCK_CONTEXT));

					cbOnConnect(pContex->SockCtx.Socket, tmpSockCtx);
				}
			}
			else if (OP_CONN == Type)
			{
				//����ʱ��
				pContex->SockCtx.ConnectedTime = time(nullptr);
				pContex->SockCtx.LastActive = time(nullptr);

				//����״̬
				pContex->SockCtx.Status = ss_buzy;

				if (cbOnConnect)
				{
					SOCK_CONTEXT tmpSockCtx;
					memcpy(&tmpSockCtx, &pContex->SockCtx, sizeof(SOCK_CONTEXT));
					cbOnConnect(pContex->SockCtx.Socket, tmpSockCtx);
				}

				//Ͷ��0�ֽڽ���
				DWORD dwBytes = 0;
				DWORD dwFlag = 0;
				pContex->Overlapped2.Type = OP_RECV;
				pContex->BuffRecv.len = 0;
				pContex->BuffRecv.buf = pContex->Data;
				int nRet = WSARecv(pContex->SockCtx.Socket, &pContex->BuffRecv, 1, &dwBytes, &dwFlag, &pContex->Overlapped2.Overlapped, nullptr);
				if (nRet == SOCKET_ERROR)
				{
					if (WSA_IO_PENDING != WSAGetLastError())
					{
						//printf("recv from socket %d error,the socket will be closed\n",pContext->Socket);
						ResetContex(pContex);
						continue;
					}
				}
			}
		}
		else
		{
			DWORD dwErr = GetLastError();

			ResetContex(pContex);

			if (!m_bWorkAsServer)
			{
				if (m_nClientType == CLIENT_TYPE_NORMAL)
				{
					InterlockedDecrement(&m_lValidSock);
				}
				else if (m_nClientType == CLIENT_TYPE_SCANER)
				{
					InterlockedDecrement(&m_lBurstSock);
				}
			}

			if (dwErr!=ERROR_CONNECTION_REFUSED &&       //���ӱ��ܾ�
				dwErr!=ERROR_NETNAME_DELETED &&          //�Է��ر�����
				dwErr!=ERROR_OPERATION_ABORTED &&		 //���عر�����
				dwErr!=ERROR_SEM_TIMEOUT)                //������ʱ
			{
				LPVOID lpMsgBuf;
				FormatMessageA(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					dwErr,
					MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
					(LPSTR)&lpMsgBuf,
					0,
					NULL
					);

				printf("GetQueuedCompletionStatus return with error=%d: %s\n", dwErr, (LPSTR)lpMsgBuf);

				LocalFree(lpMsgBuf);
			}
		}		
	}
}

void CSuperTcp::ResetContex(PER_IO_CONTEXT* pContex)
{
	if (pContex)
	{
		if (pContex->SockCtx.Socket!=INVALID_SOCKET)
		{
			if (cbOnDisconnect)
			{
				SOCK_CONTEXT tmpCtx;
				memcpy(&tmpCtx, &pContex->SockCtx, sizeof(SOCK_CONTEXT));

				cbOnDisconnect(pContex->SockCtx.Socket,tmpCtx);
			}

			closesocket(pContex->SockCtx.Socket);
			//shutdown(pContex->SockCtx.Socket, SD_BOTH);
			pContex->SockCtx.Socket = INVALID_SOCKET;
			pContex->SockCtx.Status = ss_down;
		}
	}
}

unsigned short CSuperTcp::BindSocket(SOCKET s)
{
	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = inet_addr(m_strLocalAddr.c_str());	

	static unsigned short uPort = MIN_PORT_VALUE;

	if (uPort>=MAX_PORT_VALUE)
	{
		uPort = MIN_PORT_VALUE;
	}

	//�Ӵ���1024�Ķ˿����ҳ����еĶ˿ڰ�socket
	for (; uPort < MAX_PORT_VALUE && m_bRunning;uPort++)
	{
		local.sin_port = htons(uPort);


		if (bind(s, (struct sockaddr *)&local, sizeof(local))!=SOCKET_ERROR)
		{
			return local.sin_port;
		}
		else
		{
			DWORD dwErr = WSAGetLastError();
			if (WSAEADDRINUSE/*10048*/ == dwErr || WSAEACCES == dwErr)
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}


	return -1;
}





BOOL CSuperTcp::SendTcpData(SOCKET sSocket, DWORD dwLen, char* pData)
{
	if (sSocket != INVALID_SOCKET && dwLen &&pData)
	{
		auto it = m_mapSocketToContex.find(sSocket);

		if (it != m_mapSocketToContex.end())
		{
			PER_IO_CONTEXT* pContex = it->second;
			if (pContex)
			{
				if (pContex->QueueSend)
				{
					//Ϊ�˱���Ƿ�ҳ�ڴ�Ĺ⣬��Ҫ�ְ�����				
					int nCount = dwLen / IO_BUFF_SIZE;
					if (dwLen%IO_BUFF_SIZE)
					{
						nCount++;
					}
	
					EnterCriticalSection(&pContex->SendQueueLock);
					
					char* pRemain = pData;
					DWORD dwRemain = dwLen;
	
					while (dwRemain)
					{
						WSABUF* pBuff = (WSABUF*)new char[sizeof(WSABUF)+IO_BUFF_SIZE];
						if (pBuff)
						{
							pBuff->buf = ((char*)pBuff + sizeof(WSABUF));
	
							if (dwRemain>IO_BUFF_SIZE)
							{
								pBuff->len = IO_BUFF_SIZE;
							}
							else
							{
								pBuff->len = dwRemain;
							}
	
							memcpy(pBuff->buf, pRemain, pBuff->len);
	
							pRemain += pBuff->len;
							dwRemain -= pBuff->len;
	
							pContex->QueueSend->push(pBuff);
						}
					}
	
					LeaveCriticalSection(&pContex->SendQueueLock);
	
					//����һ��0�ֽڰ��Դ�����������
					pContex->BuffSend.buf = nullptr;
					pContex->BuffSend.len = 0;
					pContex->Overlapped1.Type = OP_SEND;
					DWORD dwBytes=0;
					DWORD dwFlag = 0;
					//ZeroMemory(&pContex->SendOverLapped.Overlapped, sizeof(pContex->SendOverLapped.Overlapped));
					int nRet = WSASend(pContex->SockCtx.Socket, &pContex->BuffSend, 1, &dwBytes, dwFlag, &pContex->Overlapped1.Overlapped, nullptr);
	
					return nRet!=SOCKET_ERROR;
				}
			}
		}		
	}

	return FALSE;
}

void CSuperTcp::KillSocket(SOCKET sSocket)
{
	auto it = m_mapSocketToContex.find(sSocket);
	if (it!=m_mapSocketToContex.end())
	{
		PER_IO_CONTEXT* pContex = it->second;
		if (pContex)
		{
			if (pContex->SockCtx.Socket != INVALID_SOCKET)
			{
				closesocket(pContex->SockCtx.Socket);
				//shutdown(pContex->SockCtx.Socket, SD_BOTH);
			}
		}
	}
}



