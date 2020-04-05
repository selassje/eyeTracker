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

#ifndef SETTINGDLG_HPP
#define SETTINGDLG_HPP

#include "EyeTracker.hpp"
#include "stdafx.h"

class CSettingDlg : public CDialog {
    DECLARE_DYNAMIC(CSettingDlg)

public:
    CSettingDlg(CWnd* pParent = NULL);
    virtual ~CSettingDlg();

    enum { IDD = IDD_SETTINGDLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
private:
    CComboBox m_cDeviceCombo;
    void UpdateDeviceList(void);

public:
    virtual BOOL OnInitDialog();
    int m_iSelectedCamera;
    int m_iSelectedAlg;
    int m_iTmpWidth;
    int m_iTmpHeight;
    int m_iFrameWidth;
    int m_iFrameHeight;
    int m_iAvgFaceFps;
    int m_iAvgEyeFps;
    int m_iAccH;
    int m_iAccV;
    BOOL m_bSupportClicking;
    BOOL m_bSupportDoubleClick;
    double m_fVarrianceRatio;
    double m_fVarrianceRatio2;
    int m_iThresholdClick;
    int m_iFrameNumClick;

public:
    afx_msg void OnCbnDropdownCombodevice();
    void Save();

private:
    CComboBox m_cAlgList;
};

#endif