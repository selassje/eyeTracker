// EyeTracker.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CEyeTrackerApp:
// See EyeTracker.cpp for the implementation of this class
//

class CEyeTrackerApp : public CWinApp
{
public:
	CEyeTrackerApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CEyeTrackerApp theApp;

void Log(CString strMessage);

void SetDlgItemDouble(CWnd* pWnd,int iIdc,double dValue);

double GetDlgItemDouble(CWnd* pWnd,int iIdc);