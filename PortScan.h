
// PortScan.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPortScanApp: 
// �йش����ʵ�֣������ PortScan.cpp
//

class CPortScanApp : public CWinApp
{
public:
	CPortScanApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CRITICAL_SECTION g_csResult,g_csLog;

extern CPortScanApp theApp;