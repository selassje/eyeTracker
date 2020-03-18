// EyeTrackerDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "CustomTabCtrl.h"
#include "LogDlg.h"
#include "SettingDlg.h"
#include "CameraDlg.h"
#include "ImageDlg.h"


#define WM_NOTIFYICON (WM_USER+1)

// CEyeTrackerDlg dialog
class CEyeTrackerDlg : public CDialog
{
// Construction
public:
	CEyeTrackerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EYETRACKER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CCustomTabCtrl m_cTabCtrl;
	CLogDlg* m_pLogDlg;
	CSettingDlg* m_pSettingDlg;
	CCameraDlg* m_pCameraDlg;
	CImageDlg* m_pImageDlg;
//	CAboutDlg m_cAbout;

//	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	void Log(CString strMessage);
	afx_msg void OnMenuExit();
	afx_msg void OnMenuAbout();
	afx_msg LRESULT OnTrayNotify(WPARAM, LPARAM);

private:	
	CMenu m_cTrayMenu;
    NOTIFYICONDATA m_cTnd;
  //Icon Data
  
  
public:
	afx_msg void OnDestroy();
//	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


