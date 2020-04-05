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

#ifndef EYETRACKERDLG_HPP
#define EYETRACKERDLG_HPP
#endif

#include "CameraDlg.hpp"
#include "CustomTabCtrl.hpp"
#include "ImageDlg.hpp"
#include "LogDlg.hpp"
#include "SettingDlg.hpp"

#define WM_NOTIFYICON (WM_USER + 1)

class CEyeTrackerDlg : public CDialog {

public:
    CEyeTrackerDlg(CWnd* pParent = NULL);

    enum { IDD = IDD_EYETRACKER_DIALOG };

    afx_msg void OnDestroy();

    CCustomTabCtrl m_cTabCtrl {};
    CLogDlg* m_pLogDlg {};
    CSettingDlg* m_pSettingDlg {};
    CCameraDlg* m_pCameraDlg {};
    CImageDlg* m_pImageDlg {};

    void Log(CString strMessage);
    afx_msg void OnMenuExit();
    afx_msg void OnMenuAbout();
    afx_msg LRESULT OnTrayNotify(WPARAM, LPARAM);

protected:
    HICON m_hIcon;

    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    virtual void DoDataExchange(CDataExchange* pDX);
    DECLARE_MESSAGE_MAP()

private:
    CMenu m_cTrayMenu {};
    NOTIFYICONDATA m_cTnd {};

    enum { CAMERA_TAB = 0,
        SETTINGS_TAB,
        LOGS_TAB,
        IMAGE_TAB,
        COMPARER_TAB };
};
