/*MIT License

Copyright (c) 2020 Przemyslaw Kozioł

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
SOFTWARE.*/

#include "stdafx.h"
#include "EyeTracker.hpp"

#include "LogDlg.hpp"


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


BOOL CLogDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    int iTimeWidth = 120;
    int iMessageWidth = 700;
    m_cLogList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
    m_cLogList.InsertColumn(0, L"Time", LVCFMT_LEFT, iTimeWidth);
    m_cLogList.InsertColumn(1, L"Message", LVCFMT_LEFT, iMessageWidth);

    return TRUE; 
}

void CLogDlg::InsertLog(CString time, CString strMsg)
{
    //int nLogCount = m_cLogList.GetItemCount();
    //m_cLogList.InsertItem(LVIF_TEXT|LVIF_STATE, 0,time,0, LVIS_SELECTED,1, 0);
    //m_cLogList.SetItemText(0, 1,strMsg);
}
