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
#include "opencv2/highgui.hpp"


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
    cv::destroyAllWindows();
    CDialog::PostNcDestroy();
}

void CCameraDlg::OnTimer(UINT_PTR nIDEvent)
{
    static int lefEyeFrameCount = 0;
    static int rightEyeFrameCount = 0;

    static unsigned int faceFrameCount = 0;
    static int avgFaceX = 0;
    static int avgFaceY = 0;
    static int avgFaceWidth = 0;
    static int avgFaceHeight = 0;

    static int avgLeftEyeX = 0;
    static int avgLeftEyeY = 0;
    static int avgLeftEyeWidth = 0;
    static int avgLeftEyeHeight = 0;

    static int avgRightEyeX = 0;
    static int avgRightEyeY = 0;
    static int avgRightEyeWidth = 0;
    static int avgRightEyeHeight = 0;

    static bool isMove = false;

    static bool leftEyeClosed = false;
    static bool rightEyeClosed = false;

    cv::Point pupilLeft { -1, -1 };
    cv::Point pupilRight { -1, -1 };

    if (mCapture.isOpened()) {
        mCapture >> mCurrentFrame;
        bool leftEyeBlink = false;
        bool rightEyeBlink = false;
        if (!mCurrentFrame.empty()) {
            auto face = CObjectDetection::DetectFace(mCurrentFrame);
            double widthRatio =  mWndWidth  /  static_cast<double>(mCurrentFrame.cols);
            double heightRatio = mWndHeight /  static_cast<double>(mCurrentFrame.rows);
            cv::Mat display{ mWndHeight, mWndWidth, mCurrentFrame.type() };
            cv::resize(mCurrentFrame, display, cv::Size { mWndWidth, mWndHeight });
            
            isMove = false;

            if (face) {
                ++faceFrameCount;

                avgFaceX += face->x;
                avgFaceY += face->y;
                avgFaceWidth += face->width;
                avgFaceHeight += face->height;

                if (faceFrameCount % mAvgFaceFps) {
                    return;
                }

                face->x = avgFaceX / mAvgFaceFps;
                face->y = avgFaceY / mAvgFaceFps;
                face->height = avgFaceHeight / mAvgFaceFps;
                face->width = avgFaceWidth / mAvgFaceFps;

                avgFaceX = 0;
                avgFaceY = 0;
                avgFaceWidth = 0;
                avgFaceHeight = 0;
                faceFrameCount = 0;

                CString strLog;
                strLog.Format(L"Detected Face (x=%d y=%d width=%d height=%d)", face->x, face->y, face->width, face->height);
                Log(strLog);

                std::pair<cv::Rect, cv::Rect> eyes;

                auto& eyeLeft = eyes.first;
                auto& eyeRight = eyes.second;
  
                bool avgLeft = false;
                bool avgRight = false;

                for (int i = 0; i < mAvgEyeFps; ++i) {
                    if (mCapture.isOpened()) {

                        cv::Mat currentFrame;
                        mCapture >> currentFrame;

                        eyes = CObjectDetection::DetectEyes(currentFrame, *face);

                        if (eyeLeft.empty())
                            avgLeft = false;
                        if (eyeRight.empty())
                            avgRight = FALSE;

                        avgLeftEyeX += eyeLeft.x;
                        avgLeftEyeY += eyeLeft.y;
                        avgLeftEyeWidth += eyeLeft.width;
                        avgLeftEyeHeight += eyeLeft.height;

                        avgRightEyeX += eyeRight.x;
                        avgRightEyeY += eyeRight.y;
                        avgRightEyeWidth += eyeRight.width;
                        avgRightEyeHeight += eyeRight.height;
                    }
                }

                eyeLeft.x = avgLeftEyeX / mAvgEyeFps;
                eyeLeft.y = avgLeftEyeY / mAvgEyeFps;
                eyeLeft.width = avgLeftEyeWidth / mAvgEyeFps;
                eyeLeft.height = avgLeftEyeHeight / mAvgEyeFps;

                eyeRight.x = avgRightEyeX / mAvgEyeFps;
                eyeRight.y = avgRightEyeY / mAvgEyeFps;
                eyeRight.width = avgRightEyeWidth / mAvgEyeFps;
                eyeRight.height = avgRightEyeHeight / mAvgEyeFps;

                avgLeftEyeX = 0;
                avgLeftEyeY = 0;
                avgLeftEyeWidth = 0;
                avgLeftEyeHeight = 0;

                avgRightEyeX = 0;
                avgRightEyeY = 0;
                avgRightEyeWidth = 0;
                avgRightEyeHeight = 0;

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
                    cv::circle(mainDisplay, pointOnMainDisplay, MainRadius, color, Thickness, LineType, Shift);
                    cv::circle(eyeDisplay, pointOnEyeDisplay, EyeDisplayRadius, color, Thickness, LineType, Shift);
                };

                if (avgLeft) {

                    mLeftEyeImg = cv::Mat { eyeLeft.height, eyeLeft.width, mCurrentFrame.type() };
                    mCurrentFrame(eyeLeft).copyTo(mLeftEyeImg);

                    leftEyeBlink = CObjectDetection::DetectLeftBlink(mLeftEyeImg, static_cast<size_t>(mLastFramesNum), mVarrianceBlink, mRatioThreshold);

                    cv::Point pupilCenter {};

                    switch (mSelectedAlg) {
                    case 0:
                        pupilCenter = CObjectDetection::DetectPupilCDF(mLeftEyeImg);
                        break;
                    case 1:
                        pupilCenter = CObjectDetection::DetectPupilEdge(mLeftEyeImg);
                        break;
                    case 2:
                        pupilCenter = CObjectDetection::DetectPupilGPF(mLeftEyeImg);
                        break;
                    }

                    pupilLeft.x = pupilCenter.x + eyeLeft.x;
                    pupilLeft.y = pupilCenter.y + eyeLeft.y;

                    double eyeWidthRatio = static_cast<double>(mLeftEyeWndWidth) / mLeftEyeImg.cols;
                    double eyeHeightRatio = static_cast<double>(mLeftEyeWndHeight) / mLeftEyeImg.rows;
                    cv::Mat leftEyeDisplay { mLeftEyeWndHeight, mLeftEyeWndWidth, mCurrentFrame.type() };
                    cv::resize(mLeftEyeImg, leftEyeDisplay, cv::Size { mLeftEyeWndWidth, mLeftEyeWndHeight });
                    if (IsDlgButtonChecked(IDC_CSHOWPUPIL)) {

                        drawPupils(display,
                            cv::Point { static_cast<int>(pupilLeft.x * widthRatio), static_cast<int>(pupilLeft.y * heightRatio) },
                            leftEyeDisplay,
                            cv::Point { static_cast<int>(pupilCenter.x * eyeWidthRatio), static_cast<int>(pupilCenter.y * eyeHeightRatio)});

                        CString strLog;
                        strLog.Format(L"Detected Left Pupil (x=%d y=%d)", pupilLeft.x, pupilLeft.y);
                        Log(strLog);
                    }

                    cv::imshow(EYEL_WINDOW, leftEyeDisplay);

                    CString strLog;
                    strLog.Format(L"Detected Left Eye (x=%d y=%d width=%d height=%d)", eyeLeft.x, eyeLeft.y, eyeLeft.width, eyeLeft.height);
                    Log(strLog);
                }

                if (avgRight) {
                    mRightEyeImg = cv::Mat { eyeRight.height, eyeRight.width, mCurrentFrame.type() };
                    mCurrentFrame(eyeRight).copyTo(mRightEyeImg);


                    CString strLog;
                    strLog.Format(L"Detected Right Eye (x=%d y=%d width=%d height=%d)", eyeRight.x, eyeRight.y, eyeRight.width, eyeRight.height);
                    Log(strLog);

                    rightEyeBlink = CObjectDetection::DetectRightBlink(mRightEyeImg, static_cast<size_t>(mLastFramesNum), mVarrianceBlink, mRatioThreshold2);

                    cv::Point pupilCenter;

                    switch (mSelectedAlg) {
                    case 0:
                        pupilCenter = CObjectDetection::DetectPupilCDF(mRightEyeImg);
                        break;
                    case 1:
                        pupilCenter = CObjectDetection::DetectPupilEdge(mRightEyeImg);
                        break;
                    case 2:
                        pupilCenter = CObjectDetection::DetectPupilGPF(mRightEyeImg);
                        break;
                    }
                    pupilRight.x = pupilCenter.x + eyeRight.x;
                    pupilRight.y = pupilCenter.y + eyeRight.y;

                    cv::Mat rightEyeDisplay { mRightEyeWndHeight, mRightEyeWndWidth, mCurrentFrame.type() };
                    cv::resize(mRightEyeImg, rightEyeDisplay, cv::Size { mRightEyeWndWidth, mRightEyeWndHeight });

                    double eyeWidthRatio = mRightEyeWndWidth / static_cast<double>(mRightEyeImg.cols);
                    double eyeHeightRatio = mRightEyeWndHeight / static_cast<double>(mRightEyeImg.rows);

                    if (IsDlgButtonChecked(IDC_CSHOWPUPIL)) {
                        drawPupils(display,
                            cv::Point(static_cast<int>(pupilRight.x * widthRatio), static_cast<int>(pupilRight.y * heightRatio)),
                            rightEyeDisplay,
                            cv::Point(static_cast<int>(pupilCenter.x * eyeWidthRatio), static_cast<int>(pupilCenter.y * eyeHeightRatio)));

                        CString strLog;
                        strLog.Format(L"Detected Right Pupil (x=%d y=%d)", pupilRight.x, pupilRight.y);
                        Log(strLog);
                    }

                    cv::imshow(EYER_WINDOW, rightEyeDisplay);
                    
                    if (IsDlgButtonChecked(IDC_CSHOWEYES)) {
                        eyeRight.x = static_cast<int>(widthRatio * eyeRight.x);
                        eyeRight.y = static_cast<int>(heightRatio * eyeRight.y);
                        eyeRight.width = static_cast<int>(eyeRight.width * widthRatio);
                        eyeRight.height = static_cast<int>(eyeRight.height * heightRatio);

                        cv::rectangle(display, cv::Point { eyeRight.x, eyeRight.y }, 
                                               cv::Point { eyeRight.x + eyeRight.width, eyeRight.y + eyeRight.height },
                                               CVCOLORS::GREEN, 1, 0, 0);
                    }
                }

                if (avgLeft)
                    if (IsDlgButtonChecked(IDC_CSHOWEYES)) {
                        eyeLeft.x = static_cast<int>(widthRatio * eyeLeft.x);
                        eyeLeft.y = static_cast<int>(heightRatio * eyeLeft.y);
                        eyeLeft.width = static_cast<int>(eyeLeft.width * widthRatio);
                        eyeLeft.height = static_cast<int>(eyeLeft.height * heightRatio);
                        cv::rectangle(display, 
                            cv::Point { eyeLeft.x, eyeLeft.y }, 
                            cv::Point { eyeLeft.x + eyeLeft.width, eyeLeft.y + eyeLeft.height },
                            CVCOLORS::GREEN, 1, 0, 0);
                    }

                if (mIsTmpSet && IsDlgButtonChecked(IDC_CSHOWTMP))
                    cv::rectangle(display, 
                        cv::Point { static_cast<int>((mTemplateMidPoint.x - mTemplateWidth / 2) * widthRatio), (int)((mTemplateMidPoint.y - mTemplateHeight / 2) * heightRatio) },
                        cv::Point { static_cast<int>((mTemplateMidPoint.x + mTemplateWidth / 2) * widthRatio), (int)((mTemplateMidPoint.y + mTemplateHeight / 2) * heightRatio) }, 
                        CV_RGB(0, 0, 255), 2, 0, 0);

                if (pupilRight.x != -1 && pupilLeft.x != -1) {
                    mCurrentMidPoint.x = (pupilRight.x + pupilLeft.x) / 2;
                    mCurrentMidPoint.y = (pupilRight.y + pupilLeft.y) / 2;

                    if (IsDlgButtonChecked(IDC_CSHOWMID)) {

                        cv::Point displayMid;
                        displayMid.x = static_cast<int>(mCurrentMidPoint.x * widthRatio);
                        displayMid.y = static_cast<int>(mCurrentMidPoint.y * heightRatio);

                        cv::Point displayLeft;
                        displayLeft.x = static_cast<int>(pupilLeft.x * widthRatio);
                        displayLeft.y = static_cast<int>(pupilLeft.y * heightRatio);
                        cv::Point displayRight;
                        displayRight.x = static_cast<int>(pupilRight.x * widthRatio);
                        displayRight.y = static_cast<int>(pupilRight.y * heightRatio);

                        constexpr auto lineThickness = 1;
                        constexpr auto lineType = 8;
                        constexpr auto circleThickness = -1;
                        constexpr auto circleLineType = 4;

                        cv::line(display, displayRight, displayLeft, CVCOLORS::GREEN, lineThickness, lineType);
                        cv::circle(display, displayMid, 3, CVCOLORS::WHITE, circleThickness, circleLineType);
                    }
                    if (!mIsTmpSet && IsDlgButtonChecked(IDC_CSHOWTMP))
                       cv::rectangle(display, 
                            cv::Point { static_cast<int>((mCurrentMidPoint.x - mTemplateWidth / 2) * widthRatio), static_cast<int>((mCurrentMidPoint.y - mTemplateHeight / 2) * heightRatio) },
                            cv::Point { static_cast<int>((mCurrentMidPoint.x + mTemplateWidth / 2) * widthRatio), static_cast<int>((mCurrentMidPoint.y + mTemplateHeight / 2) * heightRatio) }, 
                            CVCOLORS::BLUE, 2, 0, 0);

                    if (mIsMouseControl) {

                        if (mIsTmpSet) {

                            if (mCurrentMidPoint.x - mTemplateMidPoint.x > mTemplateWidth / 2) {
                                int move = static_cast<int>(-sqrt(static_cast<double>(mCurrentMidPoint.x - mTemplateMidPoint.x)) * mAccH);
                                MoveCursor(move, 0);
                                isMove = TRUE;
                                CString strLog;
                                strLog.Format(L"Detected Left Movement");
                                Log(strLog);
                            }
                            if (mTemplateMidPoint.x - mCurrentMidPoint.x > mTemplateWidth / 2) {
                                int move = (int)sqrt(double(mTemplateMidPoint.x - mCurrentMidPoint.x)) * mAccH;
                                MoveCursor(move, 0);
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

               auto pressAndReleaseRightButton = [this]() {
                    PressRightButton();
                    ReleaseRightButton();
                };

                if (!isMove && mIsMouseControl) {
                    if (leftEyeBlink && !rightEyeBlink && mSupportClicking) {
                        pressAndReleaseLeftButton();
                        pressAndReleaseLeftButton();
                        Log(L"Detected Left Eye blink");
                        isMove = TRUE;
                    }
                    if (rightEyeBlink && !leftEyeBlink && mSupportDoubleClick) {
                        pressAndReleaseRightButton();
                        pressAndReleaseRightButton();
                        Log(L"Detected Right Eye Blink");
                        isMove = TRUE;
                    }
                }

                if (IsDlgButtonChecked(IDC_CHSHOWFACE)) {
                    face->x = static_cast<int>(widthRatio * face->x);
                    face->y = static_cast<int>(heightRatio * face->y);
                    face->width = static_cast<int>(face->width * widthRatio);
                    face->height = static_cast<int>(face->height * heightRatio);

                    cv::rectangle(display, cv::Point { face->x, face->y }, 
                        cv::Point(face->x + face->width, face->y + face->height),
                        CVCOLORS::BLUE, 1, 0, 0);
                }
            }

            cv::imshow(DISPLAY_WINDOW, display);
            auto newTick = GetTickCount();
            if (mTickCount) {
                auto tickDiff = newTick - mTickCount;
                int FPS = static_cast<int>(1. / (tickDiff / 1000.));
                SetDlgItemInt(IDC_FPS, FPS);
            }
            mTickCount = newTick;
        }
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
        auto cameraIndex = mEyeTrackerDlg->m_pSettingDlg->mSelectedCamera;
        mCapture.open(cameraIndex);
        if (mCapture.isOpened()) {
            mCapture.set(cv::CAP_PROP_FRAME_WIDTH, mEyeTrackerDlg->m_pSettingDlg->mFrameWidth);
            mCapture.set(cv::CAP_PROP_FRAME_WIDTH, mEyeTrackerDlg->m_pSettingDlg->mFrameHeight);
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
    CObjectDetection::DetectLeftBlink(mLeftEyeImg, static_cast<size_t>(mLastFramesNum), mVarrianceBlink, mRatioThreshold, true);
    CObjectDetection::DetectRightBlink(mRightEyeImg, static_cast<size_t>(mLastFramesNum), mVarrianceBlink, mRatioThreshold, true);
}
