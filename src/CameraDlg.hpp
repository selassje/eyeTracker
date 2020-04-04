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

#include "afxcmn.h"
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/videoio/videoio_c.h>
#include "afxwin.h"
#include "resource.h"

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
    CvCapture* m_pCapture {};
    IplImage* m_pCurrentFrame {};
    IplImage* m_pCurrentOrgFrame {};
    CEyeTrackerDlg* m_pEyeTrackerDlg {};
    int m_iWndWidth {};
    int m_iWndHeight {};
    int m_iLeftEyeWndWidth {};
    int m_iLeftEyeWndHeight {};
    int m_iRightEyeWndWidth {};
    int m_iRightEyeWndHeight {};
    BOOL m_bIsMouseControl {};
    BOOL m_bIsTracking {};
    CvPoint m_cCurrentMidPoint;
    CvPoint m_cLastMidPoint;
    CvPoint m_cTemplateMidPoint;
    int m_iTemplateWidth {};
    int m_iTemplateHeight {};
    int m_iSelectedAlg {};
    int m_iAvgFaceFps {};
    int m_iAvgEyeFps {};
    int m_iAccH {};
    int m_iAccV {};
    int m_iVarrianceBlink {};
    int m_iLastFramesNum {};
    double m_dRatioThreshold {};
    double m_dRatioThreshold2 {};
    BOOL m_bSupportClicking {};
    BOOL m_bSupportDoubleClick {};
    DWORD m_iTickCount {};
    bool m_bIsTmpSet;

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
    IplImage* m_pLeftEyeImg {};
    IplImage* m_pRightEyeImg {};

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