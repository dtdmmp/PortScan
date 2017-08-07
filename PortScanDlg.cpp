
// PortScanDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PortScan.h"
#include "PortScanDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CPortScanDlg 对话框

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
END_MESSAGE_MAP()


// CPortScanDlg 消息处理程序

BOOL CPortScanDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_pDlg = this;

	m_ipAddrFrom.SetAddress(192,168,0,1);
	m_ipAddrTo.SetAddress(192,168,0,254);
	m_ipAddrLocal.SetAddress(0,0,0,0);
	m_editPortRange.SetWindowText(_T("1-65535"));;
	m_editBurstCount.SetWindowText(_T("30000"));
	m_chkSendData.SetCheck(0);
	m_editSendText.SetWindowText(CA2T(g_szGet).m_psz);

	m_lstResult.InsertColumn(0,_T("IPAddress:Port"), LVCFMT_LEFT,300);


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPortScanDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPortScanDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPortScanDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	m_super.Stop();
	ResetList();
	CDialogEx::OnCancel();
}


void CPortScanDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	m_super.Stop();
	ResetList();
	CDialogEx::OnOK();
}


BOOL CPortScanDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (GetFocus()->GetDlgCtrlID() == IDOK)//按下回车，如果当前焦点是在自己期望的控件上
		{
			// 你想做的事，如果按下回车时焦点在你想要的控件上
			OnBnClickedOk();
		}

		return TRUE;
	}
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;
	return CDialogEx::PreTranslateMessage(pMsg);

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
	int nItem = m_pDlg->m_lstResult.InsertItem(m_pDlg->m_lstResult.GetItemCount(), strResult, 0);
	if (nItem!=-1)
	{
		m_pDlg->m_mapAddress[strResult] = nItem;
	}

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
	CString strResult;
	strResult.Format(_T("%d.%d.%d.%d:%d"),
		Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b1,
		Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b2,
		Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b3,
		Context.RemoteAddr.sin_addr.S_un.S_un_b.s_b4,
		ntohs(Context.RemoteAddr.sin_port));

	auto it = m_pDlg->m_mapAddress.find(strResult);
	if (it!=m_pDlg->m_mapAddress.end())
	{
		std::shared_ptr<char> p (new char[dwLen+1]);
		if (p)
		{
			memcpy(p.get(), pData, dwLen);
			(p.get())[dwLen] = 0x00;
			m_pDlg->m_pRecvDataList.push_back(p);
			m_pDlg->m_lstResult.SetItemData(it->second, (DWORD_PTR)(p.get()));
		}		
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
	m_pDlg->OnBnClickedButton2();
}


void CPortScanDlg::OnBnClickedButton1()
{
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

	unsigned short uPortStart = 80;
	unsigned short uPortStop = 81;
	int nPos = strPorts.Find(_T('-'));
	if (nPos!=-1)
	{
		CString str1 = strPorts.Left(nPos);
		CString str2 = strPorts.Right(strPorts.GetLength() - nPos-1);

		uPortStart = _ttoi(str1);
		uPortStop = _ttoi(str2);

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
	}

	if (m_super.StartScaner(
		CPortScanDlg::OnConnect,
		CPortScanDlg::OnDisconnect,
		CPortScanDlg::OnRecvComplete,
		CPortScanDlg::ScanComplete,
		uCount,
		CT2A(strFromIP).m_psz,
		CT2A(strToIP).m_psz,
		uPortStart,
		uPortStop,
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
	// TODO: 在此添加控件通知处理程序代码
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
				m_editRecvText.SetWindowText(CW2T(CA2W(p,CP_UTF8)).m_psz);
			}
		}
	}

	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}
