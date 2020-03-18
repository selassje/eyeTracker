// LogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EyeTracker.h"
#include "LogDlg.h"


// CLogDlg dialog

IMPLEMENT_DYNAMIC(CLogDlg, CDialog)

CLogDlg::CLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogDlg::IDD, pParent)
{

}

CLogDlg::~CLogDlg()
{
}

void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_cLogList);
}


BEGIN_MESSAGE_MAP(CLogDlg, CDialog)
END_MESSAGE_MAP()


// CLogDlg message handlers

BOOL CLogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	int iTimeWidth = 120;
	int iMessageWidth = 700;
    m_cLogList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_cLogList.InsertColumn(0,L"Time",LVCFMT_LEFT,iTimeWidth);
	m_cLogList.InsertColumn(1,L"Message",LVCFMT_LEFT,iMessageWidth);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CLogDlg::InsertLog(CString time,CString strMsg)
{
	//int nLogCount = m_cLogList.GetItemCount();
	//m_cLogList.InsertItem(LVIF_TEXT|LVIF_STATE, 0,time,0, LVIS_SELECTED,1, 0);
	//m_cLogList.SetItemText(0, 1,strMsg);
	
}
