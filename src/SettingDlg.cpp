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

#include "SettingDlg.hpp"
#include "opencv2/videoio/videoio.hpp"

using namespace cv;

#define DEF_ALG 2
#define DEF_TMP_WIDTH 20
#define DEF_TMP_HEIGHT 20
#define DEF_AVG_FPS_FACE 1
#define DEF_AVG_FPS_EYE 1
#define DEF_ACC_H 2
#define DEF_ACC_V 2
#define DEF_SUP_CLICK 1
#define DEF_SUP_DBLCLICK 0
#define DEF_FRAME_WIDTH 320
#define DEF_FRAME_HEIGHT 240
#define DEF_FRAME_CLICK 10
#define DEF_THRESHOLD_CLICK 100
#define DEF_VARRATIO_CLICK 0.2
#define DEF_VARRATIO_CLICK2 0.4

IMPLEMENT_DYNAMIC(CSettingDlg, CDialog)

CSettingDlg::CSettingDlg(CWnd* pParent)
    : CDialog(CSettingDlg::IDD, pParent)
{
}

CSettingDlg::~CSettingDlg()
{
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBODEVICE, mDeviceCombo);
    DDX_Control(pDX, IDC_ALGCOMBO, mAlgList);
}

BEGIN_MESSAGE_MAP(CSettingDlg, CDialog)
ON_CBN_DROPDOWN(IDC_COMBODEVICE, &CSettingDlg::OnCbnDropdownCombodevice)
END_MESSAGE_MAP()

void CSettingDlg::UpdateDeviceList(void)
{
    cv::VideoCapture v;
    char buf[256];
    int iCameraNum = 0;
    while (v.open(iCameraNum)) {
        ++iCameraNum;
        v.release();
    }
    mDeviceCombo.ResetContent();

    for (int i = 0; i < iCameraNum; ++i) {
        if (auto err = _itoa_s(i, buf, 10); err == 0) {
            CStringA strDeviceNameA = buf;
            mDeviceCombo.AddString(CString(strDeviceNameA));
        }
    }
}

BOOL CSettingDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    mSelectedAlg = DEF_ALG;

    mTmpHeight = DEF_TMP_HEIGHT;
    mTmpWidth = DEF_TMP_WIDTH;
    mAvgFaceFps = DEF_AVG_FPS_FACE;
    mAvgEyeFps = DEF_AVG_FPS_EYE;
    mFrameHeight = DEF_FRAME_HEIGHT;
    mFrameWidth = DEF_FRAME_WIDTH;

    mFrameNumClick = DEF_FRAME_CLICK;
    mThresholdClick = DEF_THRESHOLD_CLICK;
    mVarrianceRatio = DEF_VARRATIO_CLICK;
    mVarrianceRatio2 = DEF_VARRATIO_CLICK2;

    mAccH = DEF_ACC_H;
    mAccV = DEF_ACC_V;

    SetDlgItemInt(IDC_TMPH, mTmpHeight);
    SetDlgItemInt(IDC_TMPW, mTmpWidth);

    SetDlgItemInt(IDC_RAWH, mFrameHeight);
    SetDlgItemInt(IDC_RAWW, mFrameWidth);

    SetDlgItemInt(IDC_AVGF, mAvgFaceFps);
    SetDlgItemInt(IDC_AVGE, mAvgEyeFps);
    SetDlgItemInt(IDC_ACCH, mAccH);
    SetDlgItemInt(IDC_ACCV, mAccV);

    SetDlgItemInt(IDC_AVGFRAMENUM, mFrameNumClick);
    SetDlgItemInt(IDC_VTHRESHOLD, mThresholdClick);
    SetDlgItemDouble(this, IDC_RTHRESHOLD, mVarrianceRatio);
    SetDlgItemDouble(this, IDC_RTHRESHOLD2, mVarrianceRatio2);

    UpdateDeviceList();

    if (mDeviceCombo.GetCount() > 0) {
        mSelectedCamera = 0;
        mDeviceCombo.SetCurSel(mSelectedCamera);
    } else
        mSelectedCamera = -1;

    mAlgList.AddString(L"CDF Analysis");
    mAlgList.AddString(L"Edge Detection");
    mAlgList.AddString(L"GPF Detection");

    mAlgList.SetCurSel(mSelectedAlg);

    mSupportClicking = DEF_SUP_CLICK;
    mSupportDoubleClick = DEF_SUP_DBLCLICK;

    CheckDlgButton(IDC_CSUPDBLCLICK, mSupportDoubleClick);
    CheckDlgButton(IDC_CSUPCLICK, mSupportClicking);

    return TRUE;
}

void CSettingDlg::OnCbnDropdownCombodevice()
{
    UpdateDeviceList();
}

void CSettingDlg::Save()
{
    mTmpHeight = GetDlgItemInt(IDC_TMPH);
    mTmpWidth = GetDlgItemInt(IDC_TMPW);
    mFrameHeight = GetDlgItemInt(IDC_RAWH);
    mFrameWidth = GetDlgItemInt(IDC_RAWW);

    if (mTmpHeight <= 0) {
        mTmpHeight = DEF_TMP_HEIGHT;
        SetDlgItemInt(IDC_TMPH, mTmpHeight);
    }
    if (mTmpWidth <= 0) {
        mTmpWidth = DEF_TMP_WIDTH;
        SetDlgItemInt(IDC_TMPW, mTmpWidth);
    }

    if (mFrameHeight <= 0) {
        mFrameHeight = DEF_FRAME_HEIGHT;
        SetDlgItemInt(IDC_RAWH, mFrameHeight);
    }
    if (mFrameWidth <= 0) {
        mFrameWidth = DEF_FRAME_WIDTH;
        SetDlgItemInt(IDC_RAWW, mFrameWidth);
    }

    mAccH = DEF_ACC_H;
    mAccV = DEF_ACC_V;

    mAvgFaceFps = GetDlgItemInt(IDC_AVGF);
    mAvgEyeFps = GetDlgItemInt(IDC_AVGE);

    if (mAvgFaceFps == 0) {
        mAvgFaceFps = DEF_AVG_FPS_FACE;
        SetDlgItemInt(IDC_AVGF, mAvgFaceFps);
    }
    if (mAvgEyeFps == 0) {
        mAvgEyeFps = DEF_AVG_FPS_EYE;
        SetDlgItemInt(IDC_AVGE, mAvgEyeFps);
    }

    mAccH = GetDlgItemInt(IDC_ACCH);
    mAccV = GetDlgItemInt(IDC_ACCV);

    if (mAccH <= 0) {
        mAccH = DEF_ACC_H;
    }
    SetDlgItemInt(IDC_ACCH, mAccH);

    if (mAccV <= 0) {
        mAccV = DEF_ACC_V;
    }
    SetDlgItemInt(IDC_ACCV, mAccV);

    mFrameNumClick = GetDlgItemInt(IDC_AVGFRAMENUM);
    mThresholdClick = GetDlgItemInt(IDC_VTHRESHOLD);

    if (mFrameNumClick <= 0) {
        mFrameNumClick = DEF_FRAME_CLICK;
        SetDlgItemInt(IDC_AVGFRAMENUM, mFrameNumClick);
    }
    if (mThresholdClick <= 0 || mThresholdClick > 255) {
        mThresholdClick = DEF_THRESHOLD_CLICK;
        SetDlgItemInt(IDC_VTHRESHOLD, mThresholdClick);
    }

    mVarrianceRatio = GetDlgItemDouble(this, IDC_RTHRESHOLD);

    if (mVarrianceRatio <= 0 || mVarrianceRatio > 1) {
        mVarrianceRatio = DEF_VARRATIO_CLICK;
        SetDlgItemDouble(this, IDC_RTHRESHOLD, mVarrianceRatio);
    }
    mVarrianceRatio2 = GetDlgItemDouble(this, IDC_RTHRESHOLD2);

    if (mVarrianceRatio <= 0 || mVarrianceRatio > 1) {
        mVarrianceRatio2 = DEF_VARRATIO_CLICK2;
        SetDlgItemDouble(this, IDC_RTHRESHOLD2, mVarrianceRatio2);
    }

    mSelectedCamera = mDeviceCombo.GetCurSel();
    if (mSelectedCamera == -1 && mDeviceCombo.GetCount()) {
        mSelectedCamera = 0;
        mDeviceCombo.SetCurSel(mSelectedCamera);
    }
    mSelectedAlg = mAlgList.GetCurSel();

    mSupportClicking = IsDlgButtonChecked(IDC_CSUPCLICK);
    mSupportDoubleClick = IsDlgButtonChecked(IDC_CSUPDBLCLICK);
}
