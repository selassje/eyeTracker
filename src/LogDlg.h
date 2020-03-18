#pragma once
#include "afxcmn.h"


// CLogDlg dialog

class CLogDlg : public CDialog
{
	DECLARE_DYNAMIC(CLogDlg)

public:
	CLogDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLogDlg();

// Dialog Data
	enum { IDD = IDD_LOGDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_cLogList;
	virtual BOOL OnInitDialog();
	void InsertLog(CString time,CString strMsg);
};
