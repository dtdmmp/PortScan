
// PortScanDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PortScan.h"
#include "PortScanDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPortScanDlg �Ի���

CPortScanDlg* CPortScanDlg::m_pDlg = nullptr;

CPortScanDlg::CPortScanDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PORTSCAN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPortScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ipAddrFrom);
	DDX_Control(pDX, IDC_IPADDRESS2, m_ipAddrTo);
	DDX_Control(pDX, IDC_IPADDRESS3, m_ipAddrLocal);
	DDX_Control(pDX, IDC_EDIT1, m_editPortRange);
	DDX_Control(pDX, IDC_EDIT2, m_editBurstCount);
	DDX_Control(pDX, IDC_CHECK1, m_chkSendData);
	DDX_Control(pDX, IDC_EDIT3, m_editSendText);
	DDX_Control(pDX, IDC_EDIT4, m_editRecvText);
	DDX_Control(pDX, IDC_LIST2, m_lstResult);
	DDX_Control(pDX, IDC_BUTTON1, m_btnStart);
	DDX_Control(pDX, IDC_BUTTON2, m_btnStop);
	DDX_Control(pDX, IDOK, m_btnExit);
}

BEGIN_MESSAGE_MAP(CPortScanDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CPortScanDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CPortScanDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CPortScanDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CPortScanDlg::OnBnClickedButton2)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST2, &CPortScanDlg::OnNMDblclkList2)
	ON_NOTIFY(NM_RCLICK, IDC_LIST2, &CPortScanDlg::OnNMRClickList2)
	ON_COMMAND(ID_OPEN_OPENWITHIEI, &CPortScanDlg::OnMnOpenWithIE)
	ON_COMMAND(ID_OPEN_OPENWITHTELNET, &CPortScanDlg::OnMnOpenWithTelnet)
	ON_COMMAND(ID_OPEN_OPENWITHFTP, &CPortScanDlg::OnMnOpenWithFtp)
	ON_COMMAND(ID_OPEN_OPENWITHMSTSC, &CPortScanDlg::OnMnOpenWithMstsc)
	ON_WM_DESTROY()
	ON_COMMAND(ID_OPEN_SAVE, &CPortScanDlg::OnMnSave)
END_MESSAGE_MAP()


// CPortScanDlg ��Ϣ�������

BOOL CPortScanDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_pDlg = this;
	InitializeCriticalSection(&m_csDataList);

	m_ipAddrFrom.SetAddress(61,219,0,1);
	m_ipAddrTo.SetAddress(61,219,254,254);
	m_ipAddrLocal.SetAddress(0,0,0,0);
	//m_editPortRange.SetWindowText(_T("1-65535"));
	m_editPortRange.SetWindowText(_T("21,23,80,3389,8080,8888"));;
	m_editBurstCount.SetWindowText(_T("50000"));
	m_chkSendData.SetCheck(1);
	m_editSendText.SetWindowText(CA2T(g_szGet).m_psz);


	m_lstResult.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
	m_lstResult.InsertColumn(0, _T("IPAddress:Port"), LVCFMT_LEFT,200);
	m_lstResult.InsertColumn(1, _T("Title"), LVCFMT_LEFT, 300);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CPortScanDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPortScanDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPortScanDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPortScanDlg::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_super.Stop();
	ResetList();
	CDialogEx::OnCancel();
}


void CPortScanDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_super.Stop();
	ResetList();
	CDialogEx::OnOK();
}


BOOL CPortScanDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (GetFocus()->GetDlgCtrlID() == IDOK)//���»س��������ǰ���������Լ������Ŀؼ���
		{
			// ���������£�������»س�ʱ����������Ҫ�Ŀؼ���
			OnBnClickedOk();
		}

		return TRUE;
	}
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CPortScanDlg::OnConnect(SOCKET sSocket, const SOCK_CONTEXT& Context)
{
	CString strResult;
	strResult.Format(_T("%d.%d.%d.%d:%d"), 
		Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b1,
		Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b2,
		Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b3,
		Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b4,
		ntohs(Context.RemoteAddr.sin_port));
	EnterCriticalSection(&g_csResult);
	int nItem = m_pDlg->m_lstResult.InsertItem(m_pDlg->m_lstResult.GetItemCount(), strResult, 0);
	if (nItem!=-1)
	{
		m_pDlg->m_mapAddress[strResult] = nItem;
	}
	LeaveCriticalSection(&g_csResult);

	if (m_pDlg->m_chkSendData.GetCheck())
	{
		std::string strGet = g_szGet;
		strGet += "Host: ";
		strGet += inet_ntoa(Context.RemoteAddr.sin_addr);
		strGet += "\r\n\r\n";
		m_pDlg->m_super.SendTcpData(sSocket, strGet.size(), (char*)strGet.c_str());
	}
	else
	{
		m_pDlg->m_super.KillSocket(sSocket);
	}
}

void CPortScanDlg::OnRecvComplete(SOCKET sSocket, DWORD dwLen, char* pData, const SOCK_CONTEXT& Context)
{
	if (pData)
	{
		CString strResult;
		strResult.Format(_T("%d.%d.%d.%d:%d"),
			Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b1,
			Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b2,
			Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b3,
			Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b4,
			ntohs(Context.RemoteAddr.sin_port));
	
		EnterCriticalSection(&g_csResult);
		auto it = m_pDlg->m_mapAddress.find(strResult);
		if (it!=m_pDlg->m_mapAddress.end())
		{
			char* pBuff = new char[dwLen + 1024];
			if (pBuff)
			{
				sprintf_s(pBuff,dwLen+1024, "data from %d.%d.%d.%d:%d bytes=%u\r\n", 
					Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b1,
					Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b2,
					Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b3,
					Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b4,
					ntohs(Context.RemoteAddr.sin_port),
					dwLen);

				size_t nLen = strlen(pBuff);

				memcpy(pBuff + nLen, pData,dwLen );

				pBuff[nLen + dwLen] = 0x00;

				std::shared_ptr<char> p=nullptr;
				p.reset(pBuff);
				EnterCriticalSection(&m_pDlg->m_csDataList);
				m_pDlg->m_pRecvDataList.push_back(p);
				LeaveCriticalSection(&m_pDlg->m_csDataList);
				m_pDlg->m_lstResult.SetItemData(it->second, (DWORD_PTR)pBuff);

				char* pTilte = StrStrIA(pData, "<title>");
				if (pTilte)
				{
					pTilte += strlen("<title>");
					CStringA strTitle;
					for (int i = 0; i < 256; i++)
					{
						if (*pTilte == '<')
						{
							break;
						}
						else
						{
							strTitle += *pTilte++;
						}
					}

					if (strTitle.GetLength())
					{
						if (StrStrIA(pData, "utf-8"))
						{
							m_pDlg->m_lstResult.SetItemText(it->second, 1, CW2T(CA2W(strTitle, CP_UTF8)).m_psz);
						}
						else if(StrStrIA(pData, "gb2312"))
						{
							m_pDlg->m_lstResult.SetItemText(it->second, 1, CW2T(CA2W(strTitle, 936)).m_psz);
						}
						else if (StrStrIA(pData, "big5"))
						{
							m_pDlg->m_lstResult.SetItemText(it->second, 1, CW2T(CA2W(strTitle, 950)).m_psz);
						}
						else
						{
							m_pDlg->m_lstResult.SetItemText(it->second, 1, CW2T(CA2W(strTitle)).m_psz);
						}
					}
				}
			}	
		}

		LeaveCriticalSection(&g_csResult);
	}

	m_pDlg->m_super.KillSocket(sSocket);
}

void CPortScanDlg::OnSendComplete(SOCKET sSocket, DWORD dwLen, const SOCK_CONTEXT& Context)
{

}

void CPortScanDlg::OnDisconnect(SOCKET sSocket, const SOCK_CONTEXT& Context)
{

}

void CPortScanDlg::ScanComplete(void)
{
	::PostMessage(m_pDlg->GetSafeHwnd(), WM_COMMAND, MAKEWPARAM(m_pDlg->m_btnStop.GetDlgCtrlID(),BN_CLICKED),(LPARAM)m_pDlg->m_btnStop.GetSafeHwnd());
	//m_pDlg->OnBnClickedButton2();
	//m_pDlg->m_btnStart.EnableWindow(TRUE);
}


void CPortScanDlg::OnSockMsg(const char* pszMsg)
{
	EnterCriticalSection(&g_csLog);
	if (pszMsg)
	{		
		m_pDlg->m_strLog += CA2T(pszMsg).m_psz; 
		m_pDlg->m_strLog += _T("\r\n");
		m_pDlg->m_editRecvText.SetWindowText(m_pDlg->m_strLog);
	}
	LeaveCriticalSection(&g_csLog);
}

void CPortScanDlg::OnBnClickedButton1()
{
	m_super.Stop();
	ResetList();
	byte b1, b2, b3, b4;
	CString strFromIP;
	m_ipAddrFrom.GetAddress(b1, b2, b3, b4);
	strFromIP.Format(_T("%d.%d.%d.%d"),b1,b2,b3,b4);

	CString strToIP;
	m_ipAddrTo.GetAddress(b1, b2, b3, b4);
	strToIP.Format(_T("%d.%d.%d.%d"), b1, b2, b3, b4);

	CString strLocalIP;
	m_ipAddrLocal.GetAddress(b1, b2, b3, b4);
	strLocalIP.Format(_T("%d.%d.%d.%d"), b1, b2, b3, b4);

	CString strCount;
	m_editBurstCount.GetWindowText(strCount);
	unsigned long uCount = _ttoi(strCount);

	CString strPorts;
	m_editPortRange.GetWindowText(strPorts);
		

	std::vector<unsigned short> vecPorts;
	
	int nPos = strPorts.Find(_T('-')); //�����˿�
	if (nPos!=-1)
	{
		CString str1 = strPorts.Left(nPos);
		CString str2 = strPorts.Right(strPorts.GetLength() - nPos-1);

		unsigned uPortStart = _ttoi(str1);
		unsigned uPortStop = _ttoi(str2);

		if (uPortStart==0 || uPortStop==0)
		{
			AfxMessageBox(_T("Invalid port range"));
			return;
		}

		if (uPortStart>=uPortStop)
		{
			AfxMessageBox(_T("Invalid port range"));
			return;
		}

		for (unsigned short uPort = uPortStart; uPort < uPortStop;uPort++)
		{
			vecPorts.push_back(uPort);
		}
	}
	else //�������˿�
	{
		CString strGet;
		int i = 0;
		while (AfxExtractSubString(strGet,strPorts,i++,_T(',')))
		{
			vecPorts.push_back(_ttoi(strGet));
		}

	}

	if (m_super.StartScaner(
		std::move(vecPorts),
		CPortScanDlg::OnConnect,
		CPortScanDlg::OnDisconnect,
		CPortScanDlg::OnRecvComplete,
		CPortScanDlg::ScanComplete,
		CPortScanDlg::OnSockMsg,
		uCount,
		CT2A(strFromIP).m_psz,
		CT2A(strToIP).m_psz,
		CT2A(strLocalIP).m_psz
		))
	{
		m_btnStart.EnableWindow(FALSE);
	}
	else
	{
		AfxMessageBox(_T("Error with starting scan"));
	}
}


void CPortScanDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_super.Stop();
	m_btnStart.EnableWindow(TRUE);
}

void CPortScanDlg::ResetList()
{
	m_lstResult.DeleteAllItems();

	decltype(m_mapAddress) tmpMap;
	m_mapAddress.swap(tmpMap);

	//for (auto p:m_pRecvDataList)
	//{
	//	if (p)
	//	{
	//		delete[] p;
	//	}
	//}

	decltype(m_pRecvDataList) tmpVec;
	m_pRecvDataList.swap(tmpVec);	

	//m_pDlg->m_pRecvDataList.reserve(10000);
}


void CPortScanDlg::OnNMDblclkList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	m_editRecvText.SetWindowText(_T("NO DATA"));

	POSITION nPos = m_lstResult.GetFirstSelectedItemPosition();
	if (nPos)
	{
		int nItem = m_lstResult.GetNextSelectedItem(nPos);
		if (nItem!=-1)
		{
			char* p = (char*)m_lstResult.GetItemData(nItem);
			if (p)
			{
				if (StrStrIA(p, "utf-8"))
				{
					m_editRecvText.SetWindowText(CW2T(CA2W(p, CP_UTF8)).m_psz);
				}
				else if (StrStrIA(p, "gb2312"))
				{
					m_editRecvText.SetWindowText(CW2T(CA2W(p, 936)).m_psz);
				}
				else if (StrStrIA(p, "big5"))
				{
					m_editRecvText.SetWindowText(CW2T(CA2W(p, 950)).m_psz);
				}
				else
				{
					m_editRecvText.SetWindowText(CW2T(CA2W(p)).m_psz);
				}				
			}
		}
	}

	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
}




void CPortScanDlg::OnNMRClickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CPoint point;
	GetCursorPos(&point);
	CMenu menu;
	if (menu.LoadMenu(IDR_MENU1))
	{
		CMenu * popup = menu.GetSubMenu(0);///0��ָIDR_MENU1�е�0�в˵����������ȡһ��˵������0~n

		if (m_lstResult.GetSelectedCount())
		{
			popup->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
		}
	}

	*pResult = 0;
}


void CPortScanDlg::OnMnOpenWithIE()
{
	// TODO: �ڴ���������������
	POSITION pos = m_lstResult.GetFirstSelectedItemPosition();
	if (pos)
	{
		int nItem = m_lstResult.GetNextSelectedItem(pos);
		if (nItem != -1)
		{
			CString strText = m_lstResult.GetItemText(nItem, 0);
			CString strUrl;
			strUrl.Format(_T("http://%s"),strText);
			ShellExecute(NULL, NULL, strUrl, NULL, NULL, SW_SHOWNORMAL);
		}
	}	
}


void CPortScanDlg::OnMnOpenWithTelnet()
{
	// TODO: �ڴ���������������
	POSITION pos = m_lstResult.GetFirstSelectedItemPosition();
	if (pos)
	{
		int nItem = m_lstResult.GetNextSelectedItem(pos);
		if (nItem != -1)
		{
			CString strText = m_lstResult.GetItemText(nItem, 0);
			strText.Replace(':', ' ');
			CString strCmdLine;
			strCmdLine.Format(_T("/K %%windir%%\\sysnative\\telnet.exe %s"), strText);
			ShellExecute(NULL, NULL, _T("cmd.exe"), strCmdLine, NULL, SW_SHOWNORMAL);
		}
	}
}


void CPortScanDlg::OnMnOpenWithFtp()
{
	// TODO: �ڴ���������������
	POSITION pos = m_lstResult.GetFirstSelectedItemPosition();
	if (pos)
	{
		int nItem = m_lstResult.GetNextSelectedItem(pos);
		if (nItem != -1)
		{
			CString strText = m_lstResult.GetItemText(nItem, 0);
			CString strUrl;
			strUrl.Format(_T("ftp://%s"), strText);
			ShellExecute(NULL, NULL, strUrl, NULL, NULL, SW_SHOWNORMAL);
		}
	}
}


void CPortScanDlg::OnMnOpenWithMstsc()
{
	// TODO: �ڴ���������������
	POSITION pos = m_lstResult.GetFirstSelectedItemPosition();
	if (pos)
	{
		int nItem = m_lstResult.GetNextSelectedItem(pos);
		if (nItem != -1)
		{
			CString strText = m_lstResult.GetItemText(nItem, 0);
			CString strCmdLine;
			strCmdLine.Format(_T("/v:%s"), strText);
			ShellExecute(NULL, NULL, _T("mstsc.exe"), strCmdLine, NULL, SW_SHOWNORMAL);
		}
	}
}


void CPortScanDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	DeleteCriticalSection(&m_csDataList);
	// TODO: �ڴ˴������Ϣ����������
}


void CPortScanDlg::OnMnSave()
{
	CFileDialog fd(FALSE, _T("txt"), _T("result"), OFN_OVERWRITEPROMPT | OFN_FORCESHOWHIDDEN, _T("�ı��ļ�(*.txt)|*.txt|�����ļ�(*.*)|*.*||"), nullptr, 0);
	if (fd.DoModal() == IDOK)
	{
		HANDLE hFile = CreateFile(fd.GetPathName(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			//write unicode tag 0xFFFE
			unsigned char tag[2] = { 0xFF,0xFE };
			DWORD dwBytesWritten = 0;
			WriteFile(hFile, tag, 2, &dwBytesWritten, 0);

			for (int i = 0; i < m_lstResult.GetItemCount();i++)
			{
				CString strText1, strText2, strText;
				strText1 = m_lstResult.GetItemText(i, 0);
				strText1.Trim();
				strText2 = m_lstResult.GetItemText(i, 1);
				strText2.TrimLeft();
				
				while (strText1.GetLength()<22)
				{
					strText1 += _T(" ");
				}

				strText.Format(_T("%s#%s\r\n"), strText1,strText2);

				if (strText.GetLength())
				{
					CStringW strData = CT2W(strText).m_psz;
					WriteFile(hFile, strData.GetBuffer(), strData.GetLength()*sizeof(wchar_t), &dwBytesWritten, nullptr);
				}
			}

			CloseHandle(hFile);
		}
	}
}
