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
#include "ComparerDlg.hpp"
#include "ObjectDetection.hpp"
#include "afxdialogex.h"
#include "opencv2/imgcodecs/imgcodecs_c.h"

#define DEF_MAXTHRESHOLD 0.25
#define DEF_POINTNUMBER 100

IMPLEMENT_DYNAMIC(CComparerDlg, CDialogEx)

CComparerDlg::CComparerDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CComparerDlg::IDD, pParent)
{
}

CComparerDlg::~CComparerDlg()
{
}

void CComparerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_GRAPH, m_cGraph);
    DDX_Control(pDX, IDC_PROGR, m_cProgCtrl);
}

BEGIN_MESSAGE_MAP(CComparerDlg, CDialogEx)
ON_BN_CLICKED(IDC_BUTTON1, &CComparerDlg::OnBnClickedButton1)
ON_BN_CLICKED(IDC_BUTTON2, &CComparerDlg::OnBnClickedButton2)
ON_WM_CTLCOLOR()
ON_BN_CLICKED(IDC_CDFG, &CComparerDlg::OnBnClickedCdfg)
ON_BN_CLICKED(IDC_EDGEG, &CComparerDlg::OnBnClickedEdgeg)
ON_BN_CLICKED(IDC_GPFG, &CComparerDlg::OnBnClickedGpfg)
ON_BN_CLICKED(IDC_EXPORT, &CComparerDlg::OnBnClickedExport)
END_MESSAGE_MAP()

void CComparerDlg::OnBnClickedButton1()
{
    CString strBuffer;
    CFileDialog fileDlg(TRUE, NULL, NULL, 4 | 2 | OFN_ALLOWMULTISELECT, L"BioID Images(*.pgm)|*.pgm||", this);
    fileDlg.GetOFN().lpstrFile = strBuffer.GetBuffer(MAX_FILES * (_MAX_PATH + 1) + 1);
    fileDlg.GetOFN().nMaxFile = MAX_FILES * (_MAX_PATH + 1) + 1;
    if (fileDlg.DoModal() == IDOK) {
        m_lImages.clear();
        m_cGraph.ClearGraph();
        m_CDFPoints.clear();
        m_EdgePoints.clear();
        m_GPFPoints.clear();

        POSITION position = fileDlg.GetStartPosition();

        while (position) {
            CString strSelectedPath = fileDlg.GetNextPathName(position);
            CStringA ansiPath(strSelectedPath);

            IplImage* pImg = cvLoadImage(ansiPath);

            if (pImg) {
                m_lImages.push_back(strSelectedPath);
                cvReleaseImage(&pImg);
            }
        }
        auto iImgCount = m_lImages.size();

        CString strImgCount;
        if (iImgCount == 0) {
            strImgCount = L"";
        } else {
            strImgCount.Format(L"%d", iImgCount);
        }

        SetDlgItemText(IDC_IMGCOUNT2, strImgCount);
        strBuffer.ReleaseBuffer(0);
    }
}

void CComparerDlg::OnBnClickedButton2()
{
    m_CDFPoints.clear();
    m_EdgePoints.clear();
    m_GPFPoints.clear();
    m_iImgCount = 0;
    m_iFaceCount = 0;
    m_iEyesCount = 0;

    m_dErrorThreshold = GetDlgItemDouble(this, IDC_THRESH);
    if (m_dErrorThreshold > 1 || m_dErrorThreshold <= 0) {
        m_dErrorThreshold = DEF_MAXTHRESHOLD;
        SetDlgItemDouble(this, IDC_THRESH, m_dErrorThreshold);
        UpdateWindow();
    }

    m_iPointsNumber = GetDlgItemInt(IDC_EPOINTNUM);
    if (m_iPointsNumber <= 0) {
        m_iPointsNumber = DEF_POINTNUMBER;
        SetDlgItemInt(IDC_EPOINTNUM, m_iPointsNumber);
        UpdateWindow();
    }

    double dInterval = m_dErrorThreshold / m_iPointsNumber;

    auto iImageCount = m_lImages.size();

    m_cProgCtrl.SetRange(0, static_cast<short>(iImageCount) - 1);
    CWaitCursor cWait;
    for (unsigned int i = 0; i < m_lImages.size(); ++i) {
        CString sImgPath = m_lImages[i];

        int iExtPoint = sImgPath.ReverseFind('.') + 1;
        CString sEyePosPath = sImgPath.Mid(0, iExtPoint) + L"pts";

        CFile cEyePosFile;

        if (cEyePosFile.Open(sEyePosPath, CFile::modeRead)) {
            SBioIDEyeCenters cBioIDPos;
            CArchive ar(&cEyePosFile, CArchive::load);
            cBioIDPos.Serialize(ar);
            ar.Close();
            cEyePosFile.Close();

            IplImage* pImage = cvLoadImage(CStringA(sImgPath));

            if (pImage) {
                ++m_iImgCount;

                CvRect* pFace = CObjectDetection::DetectFace(pImage);
                if (pFace) {
                    ++m_iFaceCount;

                    CvRect cLeftEye;
                    CvRect cRightEye;

                    CvPoint cLeftPupilCDF;
                    CvPoint cRightPupilCDF;

                    cLeftPupilCDF.x = -1;
                    cLeftPupilCDF.y = -1;
                    cRightPupilCDF.x = -1;
                    cRightPupilCDF.y = -1;

                    CvPoint cLeftPupilEdge;
                    CvPoint cRightPupilEdge;

                    cLeftPupilEdge.x = -1;
                    cLeftPupilEdge.y = -1;
                    cRightPupilEdge.x = -1;
                    cRightPupilEdge.y = -1;

                    CvPoint cLeftPupilGPF;
                    CvPoint cRightPupilGPF;

                    cLeftPupilGPF.x = -1;
                    cLeftPupilGPF.y = -1;
                    cRightPupilGPF.x = -1;
                    cRightPupilGPF.y = -1;

                    CObjectDetection::DetectEyes(pImage, pFace, &cLeftEye, &cRightEye);

                    if (cLeftEye.x != -1) {
                        IplImage* pLeftEyeImg = cvCreateImage(cvSize(cLeftEye.width, cLeftEye.height), pImage->depth, pImage->nChannels);
                        cvSetImageROI(pImage,
                            cvRect(cLeftEye.x, cLeftEye.y, cLeftEye.width, cLeftEye.height));
                        cvCopy(pImage, pLeftEyeImg, NULL);
                        cvResetImageROI(pImage);

                        cLeftPupilCDF = CObjectDetection::DetectPupilCDF(pLeftEyeImg);
                        cLeftPupilEdge = CObjectDetection::DetectPupilEdge(pLeftEyeImg);
                        cLeftPupilGPF = CObjectDetection::DetectPupilGPF(pLeftEyeImg);

                        cLeftPupilCDF.x += cLeftEye.x;
                        cLeftPupilCDF.y += cLeftEye.y;

                        cLeftPupilEdge.x += cLeftEye.x;
                        cLeftPupilEdge.y += cLeftEye.y;

                        cLeftPupilGPF.x += cLeftEye.x;
                        cLeftPupilGPF.y += cLeftEye.y;

                        cvReleaseImage(&pLeftEyeImg);
                    }

                    if (cRightEye.x != -1) {
                        IplImage* pRightEyeImg = cvCreateImage(cvSize(cRightEye.width, cRightEye.height), pImage->depth, pImage->nChannels);
                        cvSetImageROI(pImage,
                            cvRect(cRightEye.x, cRightEye.y, cRightEye.width, cRightEye.height));
                        cvCopy(pImage, pRightEyeImg, NULL);
                        cvResetImageROI(pImage);

                        cRightPupilCDF = CObjectDetection::DetectPupilCDF(pRightEyeImg);
                        cRightPupilEdge = CObjectDetection::DetectPupilEdge(pRightEyeImg);
                        cRightPupilGPF = CObjectDetection::DetectPupilGPF(pRightEyeImg);

                        cRightPupilCDF.x += cRightEye.x;
                        cRightPupilCDF.y += cRightEye.y;

                        cRightPupilEdge.x += cRightEye.x;
                        cRightPupilEdge.y += cRightEye.y;

                        cRightPupilGPF.x += cRightEye.x;
                        cRightPupilGPF.y += cRightEye.y;

                        cvReleaseImage(&pRightEyeImg);
                    }

                    if (cRightEye.x != -1 && cLeftEye.x != -1) {
                        ++m_iEyesCount;
                        double dErrorCDF = cBioIDPos.Error(cLeftPupilCDF, cRightPupilCDF);
                        double dErrorEdge = cBioIDPos.Error(cLeftPupilEdge, cRightPupilEdge);
                        double dErrorGPF = cBioIDPos.Error(cLeftPupilGPF, cRightPupilGPF);
                        for (double dMaxError = 0; dMaxError <= m_dErrorThreshold; dMaxError += dInterval) {

                            if (dErrorCDF < dMaxError) {
                                if (m_CDFPoints.find(dMaxError) == m_CDFPoints.end())
                                    m_CDFPoints.insert(std::make_pair(dMaxError, 1.));
                                else
                                    m_CDFPoints[dMaxError] += 1;
                            }

                            else if (m_CDFPoints.find(dMaxError) == m_CDFPoints.end())
                                m_CDFPoints.insert(std::make_pair(dMaxError, 0));

                            if (dErrorEdge < dMaxError) {
                                if (m_EdgePoints.find(dMaxError) == m_EdgePoints.end())
                                    m_EdgePoints.insert(std::make_pair(dMaxError, 1.));
                                else
                                    m_EdgePoints[dMaxError] += 1;
                            } else if (m_EdgePoints.find(dMaxError) == m_EdgePoints.end())
                                m_EdgePoints.insert(std::make_pair(dMaxError, 0));

                            if (dErrorGPF < dMaxError) {
                                if (m_GPFPoints.find(dMaxError) == m_GPFPoints.end())
                                    m_GPFPoints.insert(std::make_pair(dMaxError, 1.));
                                else
                                    m_GPFPoints[dMaxError] += 1;
                            } else if (m_GPFPoints.find(dMaxError) == m_GPFPoints.end())
                                m_GPFPoints.insert(std::make_pair(dMaxError, 0));

                            double nextError = dMaxError + dInterval;
                            if (dMaxError + dInterval > m_dErrorThreshold && dMaxError != m_dErrorThreshold) {
                                dMaxError = m_dErrorThreshold - dInterval;
                            }
                        }
                    }
                }

                cvReleaseImage(&pImage);
                CObjectDetection::Clear();
            }
        }

        m_cProgCtrl.SetPos(i);
        UpdateWindow();
    }
    cWait.Restore();
    for (auto it = m_CDFPoints.begin(); it != m_CDFPoints.end(); ++it) {
        it->second *= 100. / m_iEyesCount;
    }
    for (auto it = m_EdgePoints.begin(); it != m_EdgePoints.end(); ++it) {
        it->second *= 100. / m_iEyesCount;
    }
    for (auto it = m_GPFPoints.begin(); it != m_GPFPoints.end(); ++it) {
        it->second *= 100. / m_iEyesCount;
    }

    SetDlgItemInt(IDC_IMGD, m_iImgCount);
    SetDlgItemInt(IDC_FACED, m_iFaceCount);
    SetDlgItemInt(IDC_EYESD, m_iEyesCount);

    Plot();
}

IMPLEMENT_SERIAL(SBioIDEyeCenters, CObject, 1)

void SBioIDEyeCenters::Serialize(CArchive& ar)
{
    CObject::Serialize(ar);

    if (ar.IsLoading()) {
        CString sLineU;
        ar.ReadString(sLineU);
        char* pBuffer = reinterpret_cast<char*>(sLineU.GetBuffer(15));

        CStringA sLine(pBuffer);
        int iLineStart = sLine.Find('\n');
        iLineStart = sLine.Find('\n', iLineStart + 1);
        iLineStart = sLine.Find('\n', iLineStart + 1);
        int iLineEnd = sLine.Find('\n', iLineStart + 1);
        int iSpace = sLine.Find(' ', iLineStart + 1);

        CStringA test = sLine.Mid(iLineStart, iSpace - iLineStart);
        cRightEye.x = atoi(sLine.Mid(iLineStart, iSpace - iLineStart));
        cRightEye.y = atoi(sLine.Mid(iSpace, iLineEnd - iLineStart));

        iLineStart = iLineEnd;
        iLineEnd = sLine.Find('\n', iLineStart + 1);

        iSpace = sLine.Find(' ', iLineStart + 1);

        cLeftEye.x = atoi(sLine.Mid(iLineStart, iSpace - iLineStart));
        cLeftEye.y = atoi(sLine.Mid(iSpace, iLineEnd - iLineStart));

        sLineU.ReleaseBuffer();
    }
}

BOOL CComparerDlg::OnInitDialog()
{

    CDialogEx::OnInitDialog();

    m_dErrorThreshold = DEF_MAXTHRESHOLD;
    m_iPointsNumber = DEF_POINTNUMBER;

    SetDlgItemDouble(this, IDC_THRESH, m_dErrorThreshold);
    SetDlgItemInt(IDC_EPOINTNUM, m_iPointsNumber);

    m_iImgCount = 0;
    m_iFaceCount = 0;
    m_iEyesCount = 0;

    SetDlgItemText(IDC_IMGD, L"");
    SetDlgItemText(IDC_FACED, L"");
    SetDlgItemText(IDC_EYESD, L"");

    SetupGraph();

    typedef HRESULT(_stdcall * TSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
    HINSTANCE hDll = ::LoadLibrary(L"UxTheme.dll");
    if (hDll) {
        TSetWindowTheme setWindowTheme = (TSetWindowTheme)::GetProcAddress(hDll, "SetWindowTheme");
        if (setWindowTheme) {
            setWindowTheme(GetDlgItem(IDC_CDFG)->m_hWnd, L"", L"");
            setWindowTheme(GetDlgItem(IDC_EDGEG)->m_hWnd, L"", L"");
            setWindowTheme(GetDlgItem(IDC_GPFG)->m_hWnd, L"", L"");
        }

        ::FreeLibrary(hDll);
    }

    m_cGraph.SetXLabel(L"ErrorThreshold");
    m_cGraph.SetYLabel(L"Detection Rate [%]");

    CheckDlgButton(IDC_CDFG, TRUE);
    CheckDlgButton(IDC_EDGEG, TRUE);
    CheckDlgButton(IDC_GPFG, TRUE);

    return TRUE;
}

HBRUSH CComparerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    switch (pWnd->GetDlgCtrlID()) {
    case IDC_CDFG:
        pDC->SetTextColor(RGB(255, 0, 0));
        break;
    case IDC_EDGEG:
        pDC->SetTextColor(RGB(0, 0, 255));
        break;
    case IDC_GPFG:
        pDC->SetTextColor(RGB(0, 255, 0));
        break;
    }

    return hbr;
}

void CComparerDlg::Plot(void)
{
    m_cGraph.ClearGraph();
    SetupGraph();
    if (IsDlgButtonChecked(IDC_CDFG))
        for (auto it = m_CDFPoints.begin(); it != m_CDFPoints.end(); ++it) {
            m_cGraph.PlotXY(it->first, it->second, 0);
        }
    if (IsDlgButtonChecked(IDC_EDGEG))
        for (auto it = m_EdgePoints.begin(); it != m_EdgePoints.end(); ++it) {
            m_cGraph.PlotXY(it->first, it->second, 1);
        }
    if (IsDlgButtonChecked(IDC_GPFG))
        for (auto it = m_GPFPoints.begin(); it != m_GPFPoints.end(); ++it) {
            m_cGraph.PlotXY(it->first, it->second, 2);
        }
}

void CComparerDlg::OnBnClickedCdfg()
{
    Plot();
}

void CComparerDlg::OnBnClickedEdgeg()
{
    Plot();
}

void CComparerDlg::OnBnClickedGpfg()
{
    Plot();
}

void CComparerDlg::SetupGraph(void)
{

    m_cGraph.SetRange(0., m_dErrorThreshold, -10, 100);

    m_cGraph.SetElementLinetype(0);
    m_cGraph.SetElementWidth(2);
    m_cGraph.SetElementLineColor(RGB(255, 0, 0));

    m_cGraph.AddElement();

    m_cGraph.SetElementLineColor(RGB(0, 0, 255));
    m_cGraph.SetElementWidth(2);
    m_cGraph.SetElementLinetype(0);

    m_cGraph.AddElement();

    m_cGraph.SetElementLineColor(RGB(0, 255, 0));
    m_cGraph.SetElementWidth(2);
    m_cGraph.SetElementLinetype(0);
}

void CComparerDlg::OnBnClickedExport()
{
    auto iCDFNum = m_CDFPoints.size();
    auto iGPFNum = m_GPFPoints.size();
    auto iEDGENum = m_EdgePoints.size();

    auto iPoints = MAX(iCDFNum, MAX(iGPFNum, iEDGENum));
    CFileDialog fileDlg(FALSE, NULL, NULL, 4 | 2, L"Text Files(*.txt)|*.txt|All(*.*)|*.*||", this);
    if (fileDlg.DoModal() == IDOK) {
        CStdioFile exportFile;
        CString strPath = fileDlg.GetPathName();
        exportFile.Open(fileDlg.GetPathName(), CFile::modeWrite | CFile::modeCreate);
        if (iPoints) {
            auto it_cdf = m_CDFPoints.begin();
            auto it_gpf = m_GPFPoints.begin();
            auto it_edge = m_EdgePoints.begin();

            for (size_t i = 0; i < iPoints; ++i) {
                double t = it_cdf->first;
                CString strLine;
                strLine.Format(L"%f", t);

                if (IsDlgButtonChecked(IDC_CDFG)) {
                    double val = it_cdf->second;
                    CString strVal;
                    strVal.Format(L"\t%f", val);
                    strLine += strVal;
                }
                if (IsDlgButtonChecked(IDC_GPFG)) {
                    double val = it_gpf->second;
                    CString strVal;
                    strVal.Format(L"\t%f", val);
                    strLine += strVal;
                }
                if (IsDlgButtonChecked(IDC_EDGEG)) {
                    double val = it_edge->second;
                    CString strVal;
                    strVal.Format(L"\t%f", val);
                    strLine += strVal;
                }
                strLine += "\n";
                exportFile.WriteString(strLine.GetBuffer(100));
                it_cdf++;
                it_gpf++;
                it_edge++;
                strLine.ReleaseBuffer();
            }
        }
        exportFile.Close();
    }
}
