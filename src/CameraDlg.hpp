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

#ifndef CAMERADLG_HPP
#define CAMERADLG_HPP

#include "resource.h"
#include "stdafx.h"
#include <opencv2/videoio/videoio_c.h>

#define DISPLAY_WINDOW "Camera"
#define EYE_DEBUG_WINDOW "Eye"
#define EYER_DEBUG_WINDOW "LeftEye"
#define EYEL_DEBUG_WINDOW "RightEye"
#define IRIS_DEBUG_WINDOW "Iris"
#define EYEL_WINDOW "Left2"
#define EYER_WINDOW "Right2"

class CEyeTrackerDlg;

class CCameraDlg : public CDialog {
    DECLARE_DYNAMIC(CCameraDlg)

private:
    CvCapture* mCapture {};
    IplImage* mCurrentFrame {};
    IplImage* mCurrentOrgFrame {};
    CEyeTrackerDlg* mEyeTrackerDlg {};
    int mWndWidth {};
    int mWndHeight {};
    int mLeftEyeWndWidth {};
    int mLeftEyeWndHeight {};
    int mRightEyeWndWidth {};
    int mRightEyeWndHeight {};
    BOOL mIsMouseControl {};
    BOOL mIsTracking {};
    CvPoint mCurrentMidPoint;
    CvPoint mLastMidPoint;
    CvPoint mTemplateMidPoint;
    int mTemplateWidth {};
    int mTemplateHeight {};
    int mSelectedAlg {};
    int mAvgFaceFps {};
    int mAvgEyeFps {};
    int mAccH {};
    int mAccV {};
    int mVarrianceBlink {};
    int mLastFramesNum {};
    double mRatioThreshold {};
    double mRatioThreshold2 {};
    BOOL mSupportClicking {};
    BOOL mSupportDoubleClick {};
    DWORD mTickCount {};
    bool mIsTmpSet;

    afx_msg void OnTimer(UINT_PTR nIDEvent);

    void MoveCursor(int iXOffset, int iYOffset);
    void PressLeftButton();
    void ReleaseLeftButton();
    void PressRightButton();
    void ReleaseRightButton();
    void ResetEyeBlinks();

public:
    CCameraDlg(CWnd* pParent = NULL);
    virtual ~CCameraDlg();
    IplImage* mLeftEyeImg {};
    IplImage* mRightEyeImg {};

    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBtmp();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnClickedBtrack();
    afx_msg void OnClickedBmousectrl();

    enum { IDD = IDD_CAMERADLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual void PostNcDestroy();

    DECLARE_MESSAGE_MAP()
};

#endif
