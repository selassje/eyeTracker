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
#include "CameraDlg.hpp"
#include "EyeTracker.hpp"

#include "Constants.hpp"
#include "EyeTrackerDlg.hpp"
#include "ObjectDetection.hpp"
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/videoio/videoio_c.h>

#define DISPLAY_TIMER 1
#define MOUSE_SENSIVITY 20

#define DBLCLK_FRAME_LIMIT 15

IMPLEMENT_DYNAMIC(CCameraDlg, CDialog)

using namespace cv;

CCameraDlg::CCameraDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CCameraDlg::IDD, pParent)
    , mIsTmpSet(false)
{
}

CCameraDlg::~CCameraDlg()
{
}

void CCameraDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCameraDlg, CDialog)
ON_WM_TIMER()
ON_BN_CLICKED(IDC_BTRACK, &CCameraDlg::OnClickedBtrack)
ON_BN_CLICKED(IDC_BMOUSECTRL, &CCameraDlg::OnClickedBmousectrl)
ON_BN_CLICKED(IDC_BTMP, &CCameraDlg::OnBnClickedBtmp)
END_MESSAGE_MAP()

BOOL CCameraDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    mCapture = NULL;
    mEyeTrackerDlg = (CEyeTrackerDlg*)GetParent();

    {
        cvNamedWindow(DISPLAY_WINDOW, CV_WINDOW_AUTOSIZE);
        HWND hWnd = (HWND)cvGetWindowHandle(DISPLAY_WINDOW);
        HWND hParent = ::GetParent(hWnd);
        CWnd* pCameraWndParent = GetDlgItem(IDC_STATICCAMERA);
        ::SetParent(hWnd, pCameraWndParent->m_hWnd);
        ::ShowWindow(hParent, SW_HIDE);
        CRect cCameraRect;
        pCameraWndParent->GetClientRect(&cCameraRect);
        mWndWidth = cCameraRect.Width();
        mWndHeight = cCameraRect.Height();
    }

    {
        cvNamedWindow(EYEL_WINDOW, CV_WINDOW_AUTOSIZE);
        HWND hWnd = (HWND)cvGetWindowHandle(EYEL_WINDOW);
        HWND hParent = ::GetParent(hWnd);
        CWnd* pCameraWndParent = GetDlgItem(IDC_LEYEDISPL);
        ::SetParent(hWnd, pCameraWndParent->m_hWnd);
        ::ShowWindow(hParent, SW_HIDE);
        CRect cCameraRect;
        pCameraWndParent->GetClientRect(&cCameraRect);
        mLeftEyeWndWidth = cCameraRect.Width();
        mLeftEyeWndHeight = cCameraRect.Height();
    }

    {
        cvNamedWindow(EYER_WINDOW, CV_WINDOW_AUTOSIZE);
        HWND hWnd = (HWND)cvGetWindowHandle(EYER_WINDOW);
        HWND hParent = ::GetParent(hWnd);
        CWnd* pCameraWndParent = GetDlgItem(IDC_REYEDISPL);
        ::SetParent(hWnd, pCameraWndParent->m_hWnd);
        ::ShowWindow(hParent, SW_HIDE);
        CRect cCameraRect;
        pCameraWndParent->GetClientRect(&cCameraRect);
        mRightEyeWndWidth = cCameraRect.Width();
        mRightEyeWndHeight = cCameraRect.Height();
    }

    mIsMouseControl = FALSE;
    mIsTracking = FALSE;

    mRightEyeImg = NULL;
    mLeftEyeImg = NULL;

    mIsTmpSet = FALSE;

    CheckDlgButton(IDC_CHSHOWFACE, 1);
    CheckDlgButton(IDC_CSHOWEYES, 1);
    CheckDlgButton(IDC_CSHOWPUPIL, 1);
    CheckDlgButton(IDC_CSHOWTMP, 1);
    CheckDlgButton(IDC_CSHOWMID, 1);

    mLastMidPoint.x = -1;
    mLastMidPoint.y = -1;

    return TRUE;
}

void CCameraDlg::PostNcDestroy()
{
    cvReleaseCapture(&mCapture);
    cvDestroyAllWindows();
    CDialog::PostNcDestroy();
}

void CCameraDlg::OnTimer(UINT_PTR nIDEvent)
{
    static int iLefEyeFrameCount = 0;
    static int iRightEyeFrameCount = 0;

    static unsigned int iFaceFrameCount = 0;
    static int iAvgFaceX = 0;
    static int iAvgFaceY = 0;
    static int iAvgFaceWidth = 0;
    static int iAvgFaceHeight = 0;

    static int iAvgLeftEyeX = 0;
    static int iAvgLeftEyeY = 0;
    static int iAvgLeftEyeWidth = 0;
    static int iAvgLeftEyeHeight = 0;

    static int iAvgRightEyeX = 0;
    static int iAvgRightEyeY = 0;
    static int iAvgRightEyeWidth = 0;
    static int iAvgRightEyeHeight = 0;

    static bool isMove = FALSE;

    static bool bLeftEyeClosed = FALSE;
    static bool bRightEyeClosed = FALSE;

    CvPoint cPupilLeft;
    CvPoint cPupilRight;

    cPupilLeft.x = -1;
    cPupilLeft.y = -1;

    cPupilRight.x = -1;
    cPupilRight.y = -1;

    if (mCapture) {
        mCurrentFrame = cvCloneImage(cvQueryFrame(mCapture));
        BOOL bLeftEyeBlink = FALSE;
        BOOL bRightEyeBlink = FALSE;
        if (mCurrentFrame) {
            auto pFace = CObjectDetection::DetectFace(cv::cvarrToMat(mCurrentFrame));
            double dWidthRatio = ((double)mWndWidth) / mCurrentFrame->width;
            double dHeightRatio = ((double)mWndHeight) / mCurrentFrame->height;
            auto pDisplay = cvCreateImage(cvSize(mWndWidth, mWndHeight),
                mCurrentFrame->depth, mCurrentFrame->nChannels);
            cvResize(mCurrentFrame, pDisplay);

            isMove = FALSE;

            if (pFace) {

                ++iFaceFrameCount;

                iAvgFaceX += pFace->x;
                iAvgFaceY += pFace->y;
                iAvgFaceWidth += pFace->width;
                iAvgFaceHeight += pFace->height;

                if (iFaceFrameCount % mAvgFaceFps) {
                    cvReleaseImage(&mCurrentFrame);
                    cvReleaseImage(&pDisplay);
                    return;
                }

                pFace->x = iAvgFaceX / mAvgFaceFps;
                pFace->y = iAvgFaceY / mAvgFaceFps;
                pFace->height = iAvgFaceHeight / mAvgFaceFps;
                pFace->width = iAvgFaceWidth / mAvgFaceFps;

                iAvgFaceX = 0;
                iAvgFaceY = 0;
                iAvgFaceWidth = 0;
                iAvgFaceHeight = 0;
                iFaceFrameCount = 0;

                CString strLog;
                strLog.Format(L"Detected Face (x=%d y=%d width=%d height=%d)", pFace->x, pFace->y, pFace->width, pFace->height);
                Log(strLog);

                std::pair<cv::Rect, cv::Rect> eyes;

                auto& cEyeLeft = eyes.first;
                auto& cEyeRight = eyes.second;
  
                bool bAvgLeft = TRUE;
                bool bAvgRight = TRUE;

                for (int i = 0; i < mAvgEyeFps; ++i) {
                    if (mCapture) {

                        IplImage* pCurrentFrame = cvCloneImage(cvQueryFrame(mCapture));

                        eyes = CObjectDetection::DetectEyes(cv::cvarrToMat(pCurrentFrame), *pFace);

                        cvReleaseImage(&pCurrentFrame);

                        if (cEyeLeft.width == 0 && cEyeLeft.height == 0)
                            bAvgLeft = FALSE;
                        if (cEyeRight.width == 0 && cEyeRight.height == 0)
                            bAvgRight = FALSE;

                        iAvgLeftEyeX += cEyeLeft.x;
                        iAvgLeftEyeY += cEyeLeft.y;
                        iAvgLeftEyeWidth += cEyeLeft.width;
                        iAvgLeftEyeHeight += cEyeLeft.height;

                        iAvgRightEyeX += cEyeRight.x;
                        iAvgRightEyeY += cEyeRight.y;
                        iAvgRightEyeWidth += cEyeRight.width;
                        iAvgRightEyeHeight += cEyeRight.height;
                    }
                }

                cEyeLeft.x = iAvgLeftEyeX / mAvgEyeFps;
                cEyeLeft.y = iAvgLeftEyeY / mAvgEyeFps;
                cEyeLeft.width = iAvgLeftEyeWidth / mAvgEyeFps;
                cEyeLeft.height = iAvgLeftEyeHeight / mAvgEyeFps;

                cEyeRight.x = iAvgRightEyeX / mAvgEyeFps;
                cEyeRight.y = iAvgRightEyeY / mAvgEyeFps;
                cEyeRight.width = iAvgRightEyeWidth / mAvgEyeFps;
                cEyeRight.height = iAvgRightEyeHeight / mAvgEyeFps;

                iAvgLeftEyeX = 0;
                iAvgLeftEyeY = 0;
                iAvgLeftEyeWidth = 0;
                iAvgLeftEyeHeight = 0;

                iAvgRightEyeX = 0;
                iAvgRightEyeY = 0;
                iAvgRightEyeWidth = 0;
                iAvgRightEyeHeight = 0;

                auto drawPupils = [](const auto& mainDisplay,
                                      const auto& pointOnMainDisplay,
                                      const auto& eyeDisplay,
                                      const auto& pointOnEyeDisplay) {
                    const auto MainRadius = 3;
                    const auto EyeDisplayRadius = 4;
                    const auto Thickness = -1;
                    const auto LineType = 4;
                    const auto Shift = 0;
                    const auto color = CV_RGB(250, 250, 210);
                    cvDrawCircle(mainDisplay, pointOnMainDisplay, MainRadius, color, Thickness, LineType, Shift);
                    cvDrawCircle(eyeDisplay, pointOnEyeDisplay, EyeDisplayRadius, color, Thickness, LineType, Shift);
                };

                if (bAvgLeft) {

                    cvReleaseImage(&mLeftEyeImg);
                    mLeftEyeImg = cvCreateImage(cvSize(cEyeLeft.width, cEyeLeft.height), mCurrentFrame->depth, mCurrentFrame->nChannels);
                    cvSetImageROI(mCurrentFrame,
                        cvRect(cEyeLeft.x, cEyeLeft.y, cEyeLeft.width, cEyeLeft.height));
                    cvCopy(mCurrentFrame, mLeftEyeImg, NULL);
                    cvResetImageROI(mCurrentFrame);

                    bLeftEyeBlink = CObjectDetection::DetectLeftBlink(cv::cvarrToMat(mLeftEyeImg), static_cast<size_t>(mLastFramesNum), mVarrianceBlink, mRatioThreshold);

                    CvPoint cPupilCenter;

                    switch (mSelectedAlg) {
                    case 0:
                        cPupilCenter = CObjectDetection::DetectPupilCDF(cv::cvarrToMat(mLeftEyeImg));
                        break;
                    case 1:
                        cPupilCenter = CObjectDetection::DetectPupilEdge(cv::cvarrToMat(mLeftEyeImg));
                        break;
                    case 2:
                        cPupilCenter = CObjectDetection::DetectPupilGPF(cv::cvarrToMat(mLeftEyeImg));
                        break;
                    }

                    cPupilLeft.x = cPupilCenter.x + cEyeLeft.x;
                    cPupilLeft.y = cPupilCenter.y + cEyeLeft.y;

                    double dEyeWidthRatio = ((double)mLeftEyeWndWidth) / mLeftEyeImg->width;
                    double dEyeHeightRatio = ((double)mLeftEyeWndHeight) / mLeftEyeImg->height;
                    IplImage* pLeftEyeDisplay = cvCreateImage(cvSize(mLeftEyeWndWidth, mLeftEyeWndHeight), mCurrentFrame->depth, mCurrentFrame->nChannels);
                    cvResize(mLeftEyeImg, pLeftEyeDisplay);
                    if (IsDlgButtonChecked(IDC_CSHOWPUPIL)) {

                        drawPupils(pDisplay,
                            cvPoint((int)(cPupilLeft.x * dWidthRatio), (int)(cPupilLeft.y * dHeightRatio)),
                            pLeftEyeDisplay,
                            cvPoint((int)(cPupilCenter.x * dEyeWidthRatio), (int)(cPupilCenter.y * dEyeHeightRatio)));

                        CString strLog;
                        strLog.Format(L"Detected Left Pupil (x=%d y=%d)", cPupilLeft.x, cPupilLeft.y);
                        Log(strLog);
                    }

                    cvShowImage(EYEL_WINDOW, pLeftEyeDisplay);

                    CString strLog;
                    strLog.Format(L"Detected Left Eye (x=%d y=%d width=%d height=%d)", cEyeLeft.x, cEyeLeft.y, cEyeLeft.width, cEyeLeft.height);
                    Log(strLog);

                    cvReleaseImage(&pLeftEyeDisplay);
                }

                if (bAvgRight) {
                    cvReleaseImage(&mRightEyeImg);
                    mRightEyeImg = cvCreateImage(cvSize(cEyeRight.width, cEyeRight.height), mCurrentFrame->depth, mCurrentFrame->nChannels);

                    cvSetImageROI(mCurrentFrame,
                        cvRect(cEyeRight.x, cEyeRight.y, cEyeRight.width, cEyeRight.height));
                    cvCopy(mCurrentFrame, mRightEyeImg, NULL);
                    cvResetImageROI(mCurrentFrame);

                    CString strLog;
                    strLog.Format(L"Detected Right Eye (x=%d y=%d width=%d height=%d)", cEyeRight.x, cEyeRight.y, cEyeRight.width, cEyeRight.height);
                    Log(strLog);

                    bRightEyeBlink = CObjectDetection::DetectRightBlink(cv::cvarrToMat(mRightEyeImg), static_cast<size_t>(mLastFramesNum), mVarrianceBlink, mRatioThreshold2);

                    CvPoint cPupilCenter;

                    switch (mSelectedAlg) {
                    case 0:
                        cPupilCenter = CObjectDetection::DetectPupilCDF(cv::cvarrToMat(mRightEyeImg));
                        break;
                    case 1:
                        cPupilCenter = CObjectDetection::DetectPupilEdge(cv::cvarrToMat(mRightEyeImg));
                        break;
                    case 2:
                        cPupilCenter = CObjectDetection::DetectPupilGPF(cv::cvarrToMat(mRightEyeImg));
                        break;
                    }
                    cPupilRight.x = cPupilCenter.x + cEyeRight.x;
                    cPupilRight.y = cPupilCenter.y + cEyeRight.y;

                    IplImage* pRightEyeDisplay = cvCreateImage(cvSize(mRightEyeWndWidth, mRightEyeWndHeight), mCurrentFrame->depth, mCurrentFrame->nChannels);
                    cvResize(mRightEyeImg, pRightEyeDisplay);

                    double dEyeWidthRatio = ((double)mRightEyeWndWidth) / mRightEyeImg->width;
                    double dEyeHeightRatio = ((double)mRightEyeWndHeight) / mRightEyeImg->height;

                    if (IsDlgButtonChecked(IDC_CSHOWPUPIL)) {
                        drawPupils(pDisplay,
                            cvPoint((int)(cPupilRight.x * dWidthRatio), (int)(cPupilRight.y * dHeightRatio)),
                            pRightEyeDisplay,
                            cvPoint((int)(cPupilCenter.x * dEyeWidthRatio), (int)(cPupilCenter.y * dEyeHeightRatio)));

                        CString strLog;
                        strLog.Format(L"Detected Right Pupil (x=%d y=%d)", cPupilRight.x, cPupilRight.y);
                        Log(strLog);
                    }

                    cvShowImage(EYER_WINDOW, pRightEyeDisplay);
                    cvReleaseImage(&pRightEyeDisplay);

                    if (IsDlgButtonChecked(IDC_CSHOWEYES)) {
                        cEyeRight.x = (int)(dWidthRatio * cEyeRight.x);
                        cEyeRight.y = (int)(dHeightRatio * cEyeRight.y);
                        cEyeRight.width = (int)(cEyeRight.width * dWidthRatio);
                        cEyeRight.height = (int)(cEyeRight.height * dHeightRatio);

                        cvRectangle(pDisplay, cvPoint(cEyeRight.x, cEyeRight.y), cvPoint(cEyeRight.x + cEyeRight.width, cEyeRight.y + cEyeRight.height),
                            CV_RGB(0, 255, 0), 1, 0, 0);
                    }
                }

                if (bAvgLeft)
                    if (IsDlgButtonChecked(IDC_CSHOWEYES)) {
                        cEyeLeft.x = (int)(dWidthRatio * cEyeLeft.x);
                        cEyeLeft.y = (int)(dHeightRatio * cEyeLeft.y);
                        cEyeLeft.width = (int)(cEyeLeft.width * dWidthRatio);
                        cEyeLeft.height = (int)(cEyeLeft.height * dHeightRatio);
                        cvRectangle(pDisplay, cvPoint(cEyeLeft.x, cEyeLeft.y), cvPoint(cEyeLeft.x + cEyeLeft.width, cEyeLeft.y + cEyeLeft.height),
                            cvScalar(0, 255, 0, 0), 1, 0, 0);
                    }

                if (mIsTmpSet && IsDlgButtonChecked(IDC_CSHOWTMP))
                    cvRectangle(pDisplay, cvPoint((int)((mTemplateMidPoint.x - mTemplateWidth / 2) * dWidthRatio), (int)((mTemplateMidPoint.y - mTemplateHeight / 2) * dHeightRatio)),
                        cvPoint((int)((mTemplateMidPoint.x + mTemplateWidth / 2) * dWidthRatio), (int)((mTemplateMidPoint.y + mTemplateHeight / 2) * dHeightRatio)), CV_RGB(0, 0, 255), 2, 0, 0);

                if (cPupilRight.x != -1 && cPupilLeft.x != -1) {
                    mCurrentMidPoint.x = (cPupilRight.x + cPupilLeft.x) / 2;
                    mCurrentMidPoint.y = (cPupilRight.y + cPupilLeft.y) / 2;

                    if (IsDlgButtonChecked(IDC_CSHOWMID)) {

                        CvPoint cDisplayMid;
                        cDisplayMid.x = (int)(mCurrentMidPoint.x * dWidthRatio);
                        cDisplayMid.y = (int)(mCurrentMidPoint.y * dHeightRatio);

                        CvPoint cDisplayLeft;
                        cDisplayLeft.x = (int)(cPupilLeft.x * dWidthRatio);
                        cDisplayLeft.y = (int)(cPupilLeft.y * dHeightRatio);
                        CvPoint cDisplayRight;
                        cDisplayRight.x = (int)(cPupilRight.x * dWidthRatio);
                        cDisplayRight.y = (int)(cPupilRight.y * dHeightRatio);

                        constexpr auto lineThickness = 1;
                        constexpr auto lineType = 8;
                        constexpr auto circleThickness = -1;
                        constexpr auto circleLineType = 4;

                        cvDrawLine(pDisplay, cDisplayRight, cDisplayLeft, CVCOLORS::GREEN, lineThickness, lineType);
                        cvDrawCircle(pDisplay, cDisplayMid, 3, CVCOLORS::WHITE, circleThickness, circleLineType);
                    }
                    if (!mIsTmpSet && IsDlgButtonChecked(IDC_CSHOWTMP))
                        cvRectangle(pDisplay, cvPoint((int)((mCurrentMidPoint.x - mTemplateWidth / 2) * dWidthRatio), (int)((mCurrentMidPoint.y - mTemplateHeight / 2) * dHeightRatio)),
                            cvPoint((int)((mCurrentMidPoint.x + mTemplateWidth / 2) * dWidthRatio), (int)((mCurrentMidPoint.y + mTemplateHeight / 2) * dHeightRatio)), CV_RGB(0, 0, 255), 2, 0, 0);

                    if (mIsMouseControl) {

                        if (mIsTmpSet) {

                            if (mCurrentMidPoint.x - mTemplateMidPoint.x > mTemplateWidth / 2) {
                                int iMove = (int)-sqrt(double(mCurrentMidPoint.x - mTemplateMidPoint.x)) * mAccH;
                                MoveCursor(iMove, 0);
                                isMove = TRUE;
                                CString strLog;
                                strLog.Format(L"Detected Left Movement");
                                Log(strLog);
                            }
                            if (mTemplateMidPoint.x - mCurrentMidPoint.x > mTemplateWidth / 2) {
                                int iMove = (int)sqrt(double(mTemplateMidPoint.x - mCurrentMidPoint.x)) * mAccH;
                                MoveCursor(iMove, 0);
                                isMove = TRUE;
                                CString strLog;
                                strLog.Format(L"Detected Right Movement");
                                Log(strLog);
                            }

                            if (mTemplateMidPoint.y - mCurrentMidPoint.y > mTemplateHeight / 2) {
                                int iMove = -(int)sqrt(double(mTemplateMidPoint.y - mCurrentMidPoint.y)) * mAccV;
                                MoveCursor(0, iMove);
                                isMove = TRUE;
                                CString strLog;
                                strLog.Format(L"Detected Up Movement");
                                Log(strLog);
                            }

                            if (mCurrentMidPoint.y - mTemplateMidPoint.y > mTemplateHeight / 2) {
                                int iMove = (int)sqrt(double(mCurrentMidPoint.y - mTemplateMidPoint.y)) * mAccV;
                                MoveCursor(0, iMove);
                                isMove = TRUE;
                                CString strLog;
                                strLog.Format(L"Detected Down Movement");
                                Log(strLog);
                            }
                        }

                        if (isMove) {
                            ResetEyeBlinks();
                        }
                    }
                }

                auto pressAndReleaseLeftButton = [this]() {
                    PressLeftButton();
                    ReleaseLeftButton();
                };

                if (!isMove && mIsMouseControl) {
                    if (bLeftEyeBlink && !bRightEyeBlink && mSupportClicking) {
                        pressAndReleaseLeftButton();
                        Log(L"Detected Left Eye blink");
                        isMove = TRUE;
                    }
                    if (bRightEyeBlink && !bLeftEyeBlink && mSupportDoubleClick) {
                        pressAndReleaseLeftButton();
                        pressAndReleaseLeftButton();
                        Log(L"Detected Right Eye Blink");
                        isMove = TRUE;
                    }
                }

                if (IsDlgButtonChecked(IDC_CHSHOWFACE)) {
                    pFace->x = (int)(dWidthRatio * pFace->x);
                    pFace->y = (int)(dHeightRatio * pFace->y);
                    pFace->width = (int)(pFace->width * dWidthRatio);
                    pFace->height = (int)(pFace->height * dHeightRatio);

                    cvRectangle(pDisplay, cvPoint(pFace->x, pFace->y), cvPoint(pFace->x + pFace->width, pFace->y + pFace->height),
                        cvScalar(0, 0, 255, 0), 1, 0, 0);
                }
            }

            cvShowImage(DISPLAY_WINDOW, pDisplay);
            cvReleaseImage(&pDisplay);
            int iNewTick = GetTickCount();
            if (mTickCount) {
                int iTickDiff = iNewTick - mTickCount;
                int iFPS = (int)(1. / (iTickDiff / 1000.));
                SetDlgItemInt(IDC_FPS, iFPS);
            }
            mTickCount = iNewTick;
        }

        cvReleaseImage(&mCurrentFrame);
    }
    CDialog::OnTimer(nIDEvent);
}

void CCameraDlg::MoveCursor(int iXOffset, int iYOffset)
{
    POINT point;
    GetCursorPos(&point);
    SetCursorPos(point.x + iXOffset, point.y + iYOffset);
}
void CCameraDlg::PressLeftButton()
{
    POINT point;
    GetCursorPos(&point);

    INPUT cInput;
    MOUSEINPUT cMouseInput;
    cMouseInput.dx = point.x;
    cMouseInput.dy = point.y;
    cMouseInput.time = 0;
    cMouseInput.mouseData = 0;
    cMouseInput.dwFlags = MOUSEEVENTF_LEFTDOWN;
    cMouseInput.dwExtraInfo = 0;

    cInput.type = INPUT_MOUSE;
    cInput.mi = cMouseInput;

    ::SendInput(1, &cInput, sizeof(cInput));
}
void CCameraDlg::PressRightButton()
{
    POINT point;
    GetCursorPos(&point);

    INPUT cInput;
    MOUSEINPUT cMouseInput;
    cMouseInput.dx = point.x;
    cMouseInput.dy = point.y;
    cMouseInput.time = 0;
    cMouseInput.mouseData = 0;
    cMouseInput.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    cMouseInput.dwExtraInfo = 0;

    cInput.type = INPUT_MOUSE;
    cInput.mi = cMouseInput;

    ::SendInput(1, &cInput, sizeof(cInput));
}
void CCameraDlg::ReleaseLeftButton()
{
    POINT point;
    GetCursorPos(&point);

    INPUT cInput;
    MOUSEINPUT cMouseInput;
    cMouseInput.dx = point.x;
    cMouseInput.dy = point.y;
    cMouseInput.time = 0;
    cMouseInput.mouseData = 0;
    cMouseInput.dwFlags = MOUSEEVENTF_LEFTUP;
    cMouseInput.dwExtraInfo = 0;

    cInput.type = INPUT_MOUSE;
    cInput.mi = cMouseInput;

    ::SendInput(1, &cInput, sizeof(cInput));
}

void CCameraDlg::ReleaseRightButton()
{
    POINT point;
    GetCursorPos(&point);

    INPUT cInput;
    MOUSEINPUT cMouseInput;
    cMouseInput.dx = point.x;
    cMouseInput.dy = point.y;
    cMouseInput.time = 0;
    cMouseInput.mouseData = 0;
    cMouseInput.dwFlags = MOUSEEVENTF_RIGHTUP;
    cMouseInput.dwExtraInfo = 0;

    cInput.type = INPUT_MOUSE;
    cInput.mi = cMouseInput;

    ::SendInput(1, &cInput, sizeof(cInput));
}

void CCameraDlg::OnClickedBtrack()
{
    if (!mIsTracking) {
        int iCameraIndex = mEyeTrackerDlg->m_pSettingDlg->mSelectedCamera;
        mCapture = cvCaptureFromCAM(iCameraIndex);

        if (mCapture) {
            cvSetCaptureProperty(mCapture, CV_CAP_PROP_FRAME_WIDTH, mEyeTrackerDlg->m_pSettingDlg->mFrameWidth);
            cvSetCaptureProperty(mCapture, CV_CAP_PROP_FRAME_HEIGHT, mEyeTrackerDlg->m_pSettingDlg->mFrameHeight);
            mIsTracking = TRUE;
            ::EnableWindow(GetDlgItem(IDC_BTMP)->m_hWnd, TRUE);
            mSelectedAlg = mEyeTrackerDlg->m_pSettingDlg->mSelectedAlg;
            mTemplateHeight = mEyeTrackerDlg->m_pSettingDlg->mTmpHeight;
            mTemplateWidth = mEyeTrackerDlg->m_pSettingDlg->mTmpWidth;
            mAvgFaceFps = mEyeTrackerDlg->m_pSettingDlg->mAvgFaceFps;
            mAvgEyeFps = mEyeTrackerDlg->m_pSettingDlg->mAvgEyeFps;
            mAccH = mEyeTrackerDlg->m_pSettingDlg->mAccH;
            mAccV = mEyeTrackerDlg->m_pSettingDlg->mAccV;
            mSupportClicking = mEyeTrackerDlg->m_pSettingDlg->mSupportClicking;
            mSupportDoubleClick = mEyeTrackerDlg->m_pSettingDlg->mSupportDoubleClick;
            mVarrianceBlink = mEyeTrackerDlg->m_pSettingDlg->mThresholdClick;
            mLastFramesNum = mEyeTrackerDlg->m_pSettingDlg->mFrameNumClick;
            mRatioThreshold = mEyeTrackerDlg->m_pSettingDlg->mVarrianceRatio;
            mRatioThreshold2 = mEyeTrackerDlg->m_pSettingDlg->mVarrianceRatio2;
            mTickCount = 0;
            SetDlgItemText(IDC_BTRACK, L"Stop Tracking");
            SetTimer(DISPLAY_TIMER, 0, NULL);
        } else {
            MessageBox(L"Could not capture camera.", L"Error",
                MB_ICONERROR | MB_OK);
            return;
        }
    }

    else {
        KillTimer(DISPLAY_TIMER);
        cvReleaseCapture(&mCapture);
        mIsTracking = FALSE;
        ResetEyeBlinks();
        ::EnableWindow(GetDlgItem(IDC_BTMP)->m_hWnd, FALSE);
        SetDlgItemText(IDC_BTRACK, L"Start Tracking");
    }
}

void CCameraDlg::OnClickedBmousectrl()
{
    mIsMouseControl = !mIsMouseControl;

    if (mIsMouseControl)
        SetDlgItemText(IDC_BMOUSECTRL, L"Switch Mouse Control Off");
    else
        SetDlgItemText(IDC_BMOUSECTRL, L"Switch Mouse Control On");
}

void CCameraDlg::OnBnClickedBtmp()
{

    if (!mIsTmpSet) {
        mTemplateMidPoint = mCurrentMidPoint;
        mIsTmpSet = TRUE;
        SetDlgItemText(IDC_BTMP, L"Reset Template");
    }

    else {
        mIsTmpSet = FALSE;
        SetDlgItemText(IDC_BTMP, L"Set Template");
    }
}

BOOL CCameraDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN) {
        if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
            pMsg->wParam = NULL;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void CCameraDlg::ResetEyeBlinks()
{
    CObjectDetection::DetectLeftBlink(cv::cvarrToMat(mLeftEyeImg), static_cast<size_t>(mLastFramesNum), mVarrianceBlink, mRatioThreshold, true);
    CObjectDetection::DetectRightBlink(cv::cvarrToMat(mRightEyeImg), static_cast<size_t>(mLastFramesNum), mVarrianceBlink, mRatioThreshold, true);
}
