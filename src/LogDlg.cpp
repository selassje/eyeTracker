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
    DDX_Control(pDX, IDC_LIST2, mLogList);
}

BEGIN_MESSAGE_MAP(CLogDlg, CDialog)
END_MESSAGE_MAP()


BOOL CLogDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    constexpr auto timeWidth = 120;
    constexpr auto messageWidth = 700;
    mLogList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
    mLogList.InsertColumn(0, L"Time", LVCFMT_LEFT, timeWidth);
    mLogList.InsertColumn(1, L"Message", LVCFMT_LEFT, messageWidth);

    return TRUE; 
}

void CLogDlg::InsertLog(CString time, CString strMsg)
{
   int nLogCount = mLogList.GetItemCount();
   mLogList.InsertItem(LVIF_TEXT|LVIF_STATE, 0,time,0, LVIS_SELECTED,1, 0);
   mLogList.SetItemText(0, 1,strMsg);
}
