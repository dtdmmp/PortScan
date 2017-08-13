
// PortScanDlg.h : 头文件
//
#include <list>

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "SuperTcp.h"

const char g_szGet[] =
"GET / HTTP/1.1 \r\n"
"Accept: text / html, application / xhtml + xml, image / jxr, */*\r\n"
"Accept-Encoding: gzip, deflate \r\n"
"Accept-Charset: utf-8 \r\n"
"Accept-Language: zh-CN \r\n"
"Connection: Keep-Alive \r\n"
"User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko \r\n";


// CPortScanDlg 对话框
class CPortScanDlg : public CDialogEx
{
// 构造
public:
	CPortScanDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PORTSCAN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	CIPAddressCtrl m_ipAddrFrom;
	CIPAddressCtrl m_ipAddrTo;
	CIPAddressCtrl m_ipAddrLocal;
	CEdit m_editPortRange;
	CEdit m_editBurstCount;
	CButton m_chkSendData;
	CEdit m_editSendText;
	CEdit m_editRecvText;
	CListCtrl m_lstResult;
	CButton m_btnStart;
	CButton m_btnStop;
	CButton m_btnExit;
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	std::map<CString, int> m_mapAddress;
	std::list<std::shared_ptr<char>> m_pRecvDataList;
	CRITICAL_SECTION m_csDataList;

	CStringArray m_arLogs;

private:
	CSuperTcp m_super;

	static CPortScanDlg* m_pDlg;

	static void OnConnect(SOCKET sSocket, const SOCK_CONTEXT&  Context);
	static void OnRecvComplete(SOCKET sSocket, DWORD dwLen, char* pData, const SOCK_CONTEXT& Context);
	static void OnSendComplete(SOCKET sSocket, DWORD dwLen, const SOCK_CONTEXT& Context);
	static void OnDisconnect(SOCKET sSocket, const SOCK_CONTEXT& Context);
	static void ScanComplete(void);
	static void OnSockMsg(const char* pszMsg);
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();

	void ResetList();
	afx_msg void OnNMDblclkList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMnOpenWithIE();
	afx_msg void OnMnOpenWithTelnet();
	afx_msg void OnMnOpenWithFtp();
	afx_msg void OnMnOpenWithMstsc();
	afx_msg void OnDestroy();
	afx_msg void OnMnSave();
	CTabCtrl m_tabLogAndData;
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	CEdit m_editLogText;
};
