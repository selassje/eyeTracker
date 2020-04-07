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

#include "ImageDlg.hpp"
#include "Constants.hpp"
#include "EyeTrackerDlg.hpp"
#include "ObjectDetection.hpp"
#include "Common.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/highgui/highgui_c.h"

#define EYEDISPLAY_TIMER 2
#define DISPLAY_LEFT_EYE_WINDOW "Left"
#define DISPLAY_RIGHT_EYE_WINDOW "Right"
#define DISPLAY_IMG_WINDOW "Image"

IMPLEMENT_DYNAMIC(CImageDlg, CDialog)

CImageDlg::CImageDlg(CWnd* pParent)
    : CDialog(CImageDlg::IDD, pParent)
{
}

CImageDlg::~CImageDlg()
{
}

void CImageDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CImageDlg, CDialog)
ON_BN_CLICKED(IDC_BUTTON1, &CImageDlg::OnBnClickedButton1)
ON_BN_CLICKED(IDC_NEXT, &CImageDlg::OnBnClickedNext)
ON_BN_CLICKED(IDC_PREV, &CImageDlg::OnBnClickedPrev)
ON_WM_CTLCOLOR()
ON_BN_CLICKED(IDC_CDF, &CImageDlg::OnClickedCdf)
ON_BN_CLICKED(IDC_EDGE, &CImageDlg::OnClickedEdge)
ON_BN_CLICKED(IDC_GPF, &CImageDlg::OnBnClickedGpf)
END_MESSAGE_MAP()


BOOL CImageDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetupWindow(*this,DISPLAY_LEFT_EYE_WINDOW, IDC_LEYEOUT, mLeftEyeWidth, mLeftEyeHeight);
    SetupWindow(*this,DISPLAY_RIGHT_EYE_WINDOW, IDC_REYEOUT, mRightEyeWidth, mRightEyeHeight);
    SetupWindow(*this,DISPLAY_IMG_WINDOW, IDC_IMG, mImgWidth, mImgHeight);

    mCurrentImg = 0;
    mImgCount = 0;

    AnalyzeCurrentImg();

    typedef HRESULT(_stdcall * TSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
    HINSTANCE hDll = ::LoadLibrary(L"UxTheme.dll");
    if (hDll) {
        TSetWindowTheme setWindowTheme = (TSetWindowTheme)::GetProcAddress(hDll, "SetWindowTheme");
        if (setWindowTheme) {
            setWindowTheme(GetDlgItem(IDC_CDF)->m_hWnd, L"", L"");
            setWindowTheme(GetDlgItem(IDC_EDGE)->m_hWnd, L"", L"");
            setWindowTheme(GetDlgItem(IDC_GPF)->m_hWnd, L"", L"");
        }
        ::FreeLibrary(hDll);
    }

    return TRUE;
}

void CImageDlg::OnBnClickedButton1()
{
    CString strBuffer;
    CFileDialog fileDlg(TRUE, NULL, NULL, 4 | 2 | OFN_ALLOWMULTISELECT, L"Eye/Face Image(*.*)|*.*||", this);
    fileDlg.GetOFN().lpstrFile = strBuffer.GetBuffer(MAX_FILES * (_MAX_PATH + 1) + 1);
    fileDlg.GetOFN().nMaxFile = MAX_FILES * (_MAX_PATH + 1) + 1;
    ;
    if (fileDlg.DoModal() == IDOK) {
        mImagePaths.clear();
        mImagePaths.resize(0);
        mCurrentImg = 0;

        POSITION position = fileDlg.GetStartPosition();

        while (position) {
            CString strSelectedPath = fileDlg.GetNextPathName(position);
            CStringA ansiPath(strSelectedPath);
            auto img = cv::imread(static_cast<const char*>(ansiPath));
     
            if (!img.empty()) {
                mImagePaths.push_back(strSelectedPath);
            }
        }
        mImgCount = mImagePaths.size();

        CString strImgCount;
        if (mImgCount == 0) {
            strImgCount.Empty();
            SetDlgItemText(IDC_IMGCURRENT, L"");
        } else {
            strImgCount.Format(L"%Iu", mImgCount);
        }
        SetDlgItemText(IDC_IMGCOUNT, strImgCount);
        AnalyzeCurrentImg();
    }
    strBuffer.ReleaseBuffer(0);
}

void CImageDlg::AnalyzeCurrentImg()
{
    cv::Mat img {};

    if (mImgCount != 0) {
        CString strImgCurrent;
        strImgCurrent.Format(L"%Iu", mCurrentImg + 1);
        SetDlgItemText(IDC_IMGCURRENT, strImgCurrent);
        CString fileName = mImagePaths[mCurrentImg];
        CStringA ansiFileName(fileName);
        img = cv::imread(static_cast<const char*>(ansiFileName));
        SetDlgItemText(IDC_FILENAME, fileName);
    }

    if (!img.empty()) {

        cv::Mat displayImg = { mImgHeight, mImgWidth, img.type(), cv::Scalar {}};
        cv::Mat displayLeftEyeImg = { mLeftEyeHeight, mLeftEyeWidth, img.type(), cv::Scalar {} };
        cv::Mat displayRightEyeImg = { mRightEyeHeight, mRightEyeWidth, img.type(), cv::Scalar {} };

        cv::Mat leftEyeImg {}, rightEyeImg {};
        AnalyzeImage(img, leftEyeImg, rightEyeImg);

        if (!leftEyeImg.empty()) {
            cv::resize(leftEyeImg, displayLeftEyeImg, { mLeftEyeWidth, mLeftEyeHeight }, 1.0, 1.0);
        }

        if (!rightEyeImg.empty()) {
            cv::resize(rightEyeImg, displayRightEyeImg, { mRightEyeWidth, mRightEyeHeight });
        }

        cv::resize(img, displayImg, cv::Size { mImgWidth, mImgHeight });

        cv::imshow(DISPLAY_RIGHT_EYE_WINDOW, displayRightEyeImg);
        cv::imshow(DISPLAY_LEFT_EYE_WINDOW, displayLeftEyeImg);
        cv::imshow(DISPLAY_IMG_WINDOW, displayImg);
    }
}

void CImageDlg::OnBnClickedNext()
{
    mCurrentImg = (mCurrentImg + 1) % mImgCount;
    AnalyzeCurrentImg();
}

void CImageDlg::OnBnClickedPrev()
{
    mCurrentImg = mCurrentImg == 0 ? mImgCount - 1 : mCurrentImg - 1;
    AnalyzeCurrentImg();
}

HBRUSH CImageDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    switch (pWnd->GetDlgCtrlID()) {
    case IDC_CDF:
        pDC->SetTextColor(RGB(255, 0, 0));
        break;
    case IDC_EDGE:
        pDC->SetTextColor(RGB(0, 0, 255));
        break;
    case IDC_GPF:
        pDC->SetTextColor(RGB(0, 255, 0));
        break;
    }

    return hbr;
}
void CImageDlg::AnalyzeImage(const cv::Mat& img, cv::Mat& leftEyeImg, cv::Mat& rightEyeImg)
{
    auto drawPoint = [](const auto& image, const auto& point, const auto& color) {
        const auto Radius = 1;
        const auto Thickness = -1;
        const auto LineType = 4;
        const auto Shift = 0;
        return cv::circle(image, point, Radius, color, Thickness, LineType, Shift);
    };

    auto drawLine = [](const auto& image, const auto& point1, const auto& point2, const auto& color) {
        const auto Thickness = 1;
        const auto LineType = 4;
        const auto Shift = 0;
        return cv::line(image, point1, point2, color, Thickness, LineType, Shift);
    };

    auto face = CObjectDetection::DetectFace(img);
    if (face) {
        auto [leftEye, rightEye] = CObjectDetection::DetectEyes(img, *face);

        if (!leftEye.empty()) {
            leftEyeImg = { cv::Size { leftEye.width, leftEye.height }, img.type() }; 
            img(leftEye).copyTo(leftEyeImg);
        }
        if (!rightEye.empty()) {
            rightEyeImg = { cv::Size { rightEye.width, rightEye.height }, img.type() };
            img(rightEye).copyTo(rightEyeImg);
        }

        if (!leftEyeImg.empty()) {
            cv::Point pupilCDF {},
                pupilEdge {},
                pupilGPF {};

            if (IsDlgButtonChecked(IDC_CDF)) {
                pupilCDF = CObjectDetection::DetectPupilCDF(leftEyeImg);
            }
            if (IsDlgButtonChecked(IDC_EDGE)) {
                pupilEdge = CObjectDetection::DetectPupilEdge(leftEyeImg);
            }
            if (IsDlgButtonChecked(IDC_GPF)) {
                pupilGPF = CObjectDetection::DetectPupilGPF(leftEyeImg);
            }

            if (IsDlgButtonChecked(IDC_CDF)) {
                drawPoint(leftEyeImg, cv::Point(pupilCDF.x, pupilCDF.y), CVCOLORS::RED);
#ifdef DEBUG_
                IplImage* pDrawnPupil = cvCreateImage(cvGetSize(*pLeftEye), 8, 1);
                cvCvtColor(*pLeftEye, pDrawnPupil, CV_BGR2GRAY);
                cvSaveImage("left_eye_cdf.jpg", pDrawnPupil);
                cvReleaseImage(&pDrawnPupil);
#endif
            }
            if (IsDlgButtonChecked(IDC_EDGE)) {
                drawPoint(leftEyeImg, cv::Point { pupilEdge.x, pupilEdge.y }, CVCOLORS::BLUE);
#ifdef DEBUG_
                IplImage* pDrawnPupil = cvCreateImage(cvGetSize(*pLeftEye), 8, 1);
                cvCvtColor(*pLeftEye, pDrawnPupil, CV_BGR2GRAY);
                drawLine(pDrawnPupil, cvPoint(0, cPupilEdge.y), cvPoint((*pLeftEye)->width, cPupilEdge.y), CVCOLORS::RED);
                drawLine(pDrawnPupil, cvPoint(cPupilEdge.x, 0), cvPoint(cPupilEdge.x, (*pLeftEye)->height), CVCOLORS::RED);
                cvSaveImage("right_eye_edge.jpg", pDrawnPupil);
                cvReleaseImage(&pDrawnPupil);
#endif
            }
            if (IsDlgButtonChecked(IDC_GPF)) {
                drawPoint(leftEyeImg, cv::Point { pupilGPF.x, pupilGPF.y }, CVCOLORS::GREEN);
#ifdef DEBUG_
                IplImage* pDrawnPupil = cvCreateImage(cvGetSize(*pLeftEye), 8, 1);
                cvCvtColor(*pLeftEye, pDrawnPupil, CV_BGR2GRAY);
                drawLine(pDrawnPupil, cvPoint(0, cPupilGPF.y), cvPoint((*pLeftEye)->width, cPupilGPF.y), CVCOLORS::RED);
                drawLine(pDrawnPupil, cvPoint(cPupilGPF.x, 0), cvPoint(cPupilGPF.x, (*pLeftEye)->height), CVCOLORS::RED);
                cvSaveImage("left_eye_gpf.jpg", pDrawnPupil);
                cvReleaseImage(&pDrawnPupil);
#endif
            }
        }

        if (!rightEyeImg.empty()) {
            cv::Point pupilCDF {},
                pupilEdge {},
                pupilGPF {};

            if (IsDlgButtonChecked(IDC_CDF)) {
                pupilCDF = CObjectDetection::DetectPupilCDF(rightEyeImg);
            }
            if (IsDlgButtonChecked(IDC_EDGE)) {
                pupilEdge = CObjectDetection::DetectPupilEdge(rightEyeImg);
            }
            if (IsDlgButtonChecked(IDC_GPF)) {
                pupilGPF = CObjectDetection::DetectPupilGPF(rightEyeImg);
            }
            if (IsDlgButtonChecked(IDC_CDF)) {
                drawPoint(rightEyeImg, cv::Point(pupilCDF.x, pupilCDF.y), CVCOLORS::RED);
#ifdef DEBUG_
                IplImage* pDrawnPupil = cvCreateImage(cvGetSize(*pRightEye), 8, 1);
                cvCvtColor(*pRightEye, pDrawnPupil, CV_BGR2GRAY);
                drawLine(pDrawnPupil, cvPoint(0, cPupilCDF.y), cvPoint((*pRightEye)->width, cPupilCDF.y), CVCOLORS::RED);
                drawLine(pDrawnPupil, cvPoint(cPupilCDF.x, 0), cvPoint(cPupilCDF.x, (*pRightEye)->height), CVCOLORS::RED);
                cvSaveImage("right_eye_cdf.jpg", pDrawnPupil);
                cvReleaseImage(&pDrawnPupil);
#endif
            }
            if (IsDlgButtonChecked(IDC_EDGE)) {
                drawPoint(rightEyeImg, cv::Point { pupilEdge.x, pupilEdge.y }, CVCOLORS::BLUE);
            }
            if (IsDlgButtonChecked(IDC_GPF)) {
                drawPoint(rightEyeImg, cv::Point { pupilGPF.x, pupilGPF.y }, CVCOLORS::GREEN);
            }
        }

#if 0
        auto pFaceDrawnImg = cvCreateImage(cvGetSize(pImg), pImg->depth, pImg->nChannels);
        cvCopy(pImg, pFaceDrawnImg);
        cvRectangle(pFaceDrawnImg, cvPoint(pFace->x, pFace->y), cvPoint(pFace->x + pFace->width, pFace->y + pFace->height), CV_RGB(0, 0, 0), 1, 0, 0);
        cvSaveImage("face.jpg", pFaceDrawnImg);
        cvReleaseImage(&pFaceDrawnImg);
#endif
    }
}

void CImageDlg::OnClickedCdf()
{
    AnalyzeCurrentImg();
}

void CImageDlg::OnClickedEdge()
{
    AnalyzeCurrentImg();
}

void CImageDlg::OnBnClickedGpf()
{
    AnalyzeCurrentImg();
}
