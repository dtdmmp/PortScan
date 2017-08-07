
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

	m_ipAddrFrom.SetAddress(192,168,0,1);
	m_ipAddrTo.SetAddress(192,168,0,254);
	m_ipAddrLocal.SetAddress(0,0,0,0);
	m_editPortRange.SetWindowText(_T("1-65535"));;
	m_editBurstCount.SetWindowText(_T("30000"));
	m_chkSendData.SetCheck(0);
	m_editSendText.SetWindowText(CA2T(g_szGet).m_psz);

	m_lstResult.InsertColumn(0,_T("IPAddress:Port"), LVCFMT_LEFT,300);


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

	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
}
