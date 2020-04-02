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
#include "CustomTabCtrl.hpp"
#include "CameraDlg.hpp"
#include "ComparerDlg.hpp"
#include "EyeTrackerDlg.hpp"
#include "ImageDlg.hpp"
#include "LogDlg.hpp"
#include "SettingDlg.hpp"


IMPLEMENT_DYNAMIC(CCustomTabCtrl, CTabCtrl)

CCustomTabCtrl::CCustomTabCtrl()
{

    m_DialogID[0] = IDD_CAMERADLG;
    m_DialogID[1] = IDD_SETTINGDLG;
    m_DialogID[2] = IDD_LOGDLG;
    m_DialogID[3] = IDD_IMAGEDLG;
    m_DialogID[4] = IDD_COMPARER;

    m_Dialog[0] = new CCameraDlg();
    m_Dialog[1] = new CSettingDlg();
    m_Dialog[2] = new CLogDlg();
    m_Dialog[3] = new CImageDlg();
    m_Dialog[4] = new CComparerDlg();

    m_nPageCount = 5;
}

CCustomTabCtrl::~CCustomTabCtrl()
{
}

void CCustomTabCtrl::InitDialogs()
{
    m_Dialog[0]->Create(m_DialogID[0], GetParent());
    m_Dialog[1]->Create(m_DialogID[1], GetParent());
    m_Dialog[2]->Create(m_DialogID[2], GetParent());
    m_Dialog[3]->Create(m_DialogID[3], GetParent());
    m_Dialog[4]->Create(m_DialogID[4], GetParent());

    CEyeTrackerDlg* pEyeTrackerDlg = (CEyeTrackerDlg*)GetParent();

    pEyeTrackerDlg->m_pCameraDlg = (CCameraDlg*)m_Dialog[0];
    pEyeTrackerDlg->m_pSettingDlg = (CSettingDlg*)m_Dialog[1];
    pEyeTrackerDlg->m_pLogDlg = (CLogDlg*)m_Dialog[2];
    pEyeTrackerDlg->m_pImageDlg = (CImageDlg*)m_Dialog[3];
}

void CCustomTabCtrl::ActivateTabDialogs()
{
    m_iCurrentPage = GetCurSel();
    if (m_Dialog[m_iCurrentPage]->m_hWnd)
        m_Dialog[m_iCurrentPage]->ShowWindow(SW_HIDE);

    CRect l_rectClient;
    CRect l_rectWnd;

    GetClientRect(l_rectClient);
    AdjustRect(FALSE, l_rectClient);
    GetWindowRect(l_rectWnd);
    GetParent()->ScreenToClient(l_rectWnd);
    l_rectClient.OffsetRect(l_rectWnd.left, l_rectWnd.top);
    for (int nCount = 0; nCount < m_nPageCount; nCount++) {
        m_Dialog[nCount]->SetWindowPos(&wndTop, l_rectClient.left, l_rectClient.top, l_rectClient.Width(), l_rectClient.Height(), SWP_HIDEWINDOW);
    }
    m_Dialog[m_iCurrentPage]->SetWindowPos(&wndTop, l_rectClient.left, l_rectClient.top, l_rectClient.Width(), l_rectClient.Height(), SWP_SHOWWINDOW);

    m_Dialog[m_iCurrentPage]->ShowWindow(SW_SHOW);
}

BEGIN_MESSAGE_MAP(CCustomTabCtrl, CTabCtrl)
ON_NOTIFY_REFLECT(TCN_SELCHANGE, &CCustomTabCtrl::OnTcnSelchange)
END_MESSAGE_MAP()

void CCustomTabCtrl::OnTcnSelchange(NMHDR* pNMHDR, LRESULT* pResult)
{

    if (m_iCurrentPage == 1) {
        ((CSettingDlg*)m_Dialog[1])->Save();
    }
    ActivateTabDialogs();
    pResult = 0;
}
