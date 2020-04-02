/*
MIT License

Copyright (c) 2020 Przemyslaw Koziol

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "stdafx.h"
#include "EyeTracker.hpp"
#include "EyeTrackerDlg.hpp"

class CAboutDlg : public CDialog {
public:
    CAboutDlg();

    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

 
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg()
    : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()



CEyeTrackerDlg* EyeTrackerDlg;

CEyeTrackerDlg::CEyeTrackerDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CEyeTrackerDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEyeTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB, m_cTabCtrl);
}

BEGIN_MESSAGE_MAP(CEyeTrackerDlg, CDialog)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()

ON_COMMAND(ID_MENU_EXIT, &CEyeTrackerDlg::OnMenuExit)
ON_COMMAND(ID_MENU_ABOUT, &CEyeTrackerDlg::OnMenuAbout)
ON_MESSAGE(WM_NOTIFYICON, &CEyeTrackerDlg::OnTrayNotify)
ON_WM_DESTROY()

END_MESSAGE_MAP()


BOOL CEyeTrackerDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL) {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty()) {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    SetIcon(m_hIcon, TRUE); // Set big icon
    SetIcon(m_hIcon, FALSE); // Set small icon

    m_cTabCtrl.InitDialogs();
    TCITEM tcItem;
    tcItem.mask = TCIF_TEXT;

    tcItem.pszText = _T("Camera");
    m_cTabCtrl.InsertItem(0, &tcItem);

    tcItem.pszText = _T("Settings");
    m_cTabCtrl.InsertItem(1, &tcItem);

    tcItem.pszText = _T("Logs");
    m_cTabCtrl.InsertItem(2, &tcItem);

    tcItem.pszText = _T("Image");
    m_cTabCtrl.InsertItem(3, &tcItem);

    tcItem.pszText = _T("Comparer");
    m_cTabCtrl.InsertItem(4, &tcItem);

    m_cTabCtrl.ActivateTabDialogs();

    CString sTip(_T("Eye Tracker"));
    m_cTnd.cbSize = sizeof(NOTIFYICONDATA);
    m_cTnd.hWnd = this->GetSafeHwnd();
    ;
    m_cTnd.uID = IDR_MAINFRAME;

    m_cTnd.uCallbackMessage = WM_NOTIFYICON;
    m_cTnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_cTnd.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

    lstrcpyn(m_cTnd.szTip, (LPCTSTR)sTip, sizeof(m_cTnd.szTip));
    m_cTrayMenu.LoadMenu(IDR_MENU);

    EyeTrackerDlg = this;

    Log(L"Starting EyeTracker");

    return TRUE;
}

void CEyeTrackerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    CDialog::OnSysCommand(nID, lParam);

    if (nID == SC_MINIMIZE) {
        DWORD dwMessage = NIM_ADD;
        ShowWindow(SW_HIDE);
        Shell_NotifyIcon(dwMessage, &m_cTnd);
    }
}

void CEyeTrackerDlg::OnPaint()
{
    if (IsIconic()) {
        CPaintDC dc(this);

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

     
        dc.DrawIcon(x, y, m_hIcon);
    } else {
        CDialog::OnPaint();
    }
}

HCURSOR CEyeTrackerDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CEyeTrackerDlg::Log(CString strMessage)
{
    SYSTEMTIME myTime;

    GetSystemTime(&myTime);

    CString strTime;
    strTime.Format(L"%02d-%02d-%d %02d:%02d:%02d", myTime.wDay, myTime.wMonth, myTime.wYear, myTime.wHour, myTime.wMinute, myTime.wSecond);

    if (m_pLogDlg)
        m_pLogDlg->InsertLog(strTime, strMessage);
}

void CEyeTrackerDlg::OnMenuExit()
{
    SendMessage(WM_CLOSE);
}

void CEyeTrackerDlg::OnMenuAbout()
{
    CAboutDlg cAboutDlg;
    cAboutDlg.DoModal();
}
LRESULT CEyeTrackerDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam)
{
    UINT uMsg = (UINT)lParam;
    switch (uMsg) {
    case WM_LBUTTONDBLCLK:
        this->ShowWindow(SW_SHOW);
        SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
        Shell_NotifyIcon(NIM_DELETE, &m_cTnd);
        break;
    case WM_RBUTTONUP:
        CPoint pt;
        GetCursorPos(&pt);
        SetForegroundWindow();
        m_cTrayMenu.GetSubMenu(0)->TrackPopupMenu(TPM_RIGHTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this);
        break;
    }
    return TRUE;
}
void CEyeTrackerDlg::OnDestroy()
{
    CDialog::OnDestroy();
}
