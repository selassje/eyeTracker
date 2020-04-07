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
#include "ComparerDlg.hpp"
#include "EyeTracker.hpp"
#include "ObjectDetection.hpp"
#include "opencv2/imgcodecs.hpp"


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
    DDX_Control(pDX, IDC_GRAPH, mGraph);
    DDX_Control(pDX, IDC_PROGR, mProgCtrl);
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
        mImagePaths.clear();
        mGraph.ClearGraph();
        mCDFPoints.clear();
        mEdgePoints.clear();
        mGPFPoints.clear();

        POSITION position = fileDlg.GetStartPosition();

        while (position) {

            CString strSelectedPath = fileDlg.GetNextPathName(position);
            CStringA ansiPath(strSelectedPath);
            auto img = cv::imread(static_cast<const char*>(ansiPath));

            if (!img.empty()) {
                mImagePaths.push_back(strSelectedPath);
            }
        }
        auto iImgCount = mImagePaths.size();

        CString strImgCount;
        if (iImgCount == 0) {
            strImgCount.Empty();
        } else {
            strImgCount.Format(L"%Iu", iImgCount);
        }

        SetDlgItemText(IDC_IMGCOUNT2, strImgCount);
        strBuffer.ReleaseBuffer(0);
    }
}

void CComparerDlg::OnBnClickedButton2()
{
    mCDFPoints.clear();
    mEdgePoints.clear();
    mGPFPoints.clear();
    mImgCount = 0;
    mFaceCount = 0;
    mEyesCount = 0;

    mErrorThreshold = GetDlgItemDouble(this, IDC_THRESH);
    if (mErrorThreshold > 1 || mErrorThreshold <= 0) {
        mErrorThreshold = DEF_MAXTHRESHOLD;
        SetDlgItemDouble(this, IDC_THRESH, mErrorThreshold);
        UpdateWindow();
    }

    mPointsNumber = GetDlgItemInt(IDC_EPOINTNUM);
    if (mPointsNumber <= 0) {
        mPointsNumber = DEF_POINTNUMBER;
        SetDlgItemInt(IDC_EPOINTNUM, mPointsNumber);
        UpdateWindow();
    }

    double dInterval = mErrorThreshold / mPointsNumber;

    auto iImageCount = mImagePaths.size();

    mProgCtrl.SetRange(0, static_cast<short>(iImageCount) - 1);
    CWaitCursor cWait;
    for (unsigned i = 0; i < static_cast<unsigned>(mImagePaths.size()); ++i) {
        CString sImgPath = mImagePaths[static_cast<size_t>(i)];

        int iExtPoint = sImgPath.ReverseFind('.') + 1;
        CString sEyePosPath = sImgPath.Mid(0, iExtPoint) + L"pts";

        CFile cEyePosFile;

        if (cEyePosFile.Open(sEyePosPath, CFile::modeRead)) {
            SBioIDEyeCenters cBioIDPos;
            CArchive ar(&cEyePosFile, CArchive::load);
            cBioIDPos.Serialize(ar);
            ar.Close();
            cEyePosFile.Close();

            auto img = cv::imread(static_cast<const char*>(CStringA{sImgPath}));

            if (!img.empty()) {
                ++mImgCount;

                auto face = CObjectDetection::DetectFace(img);
                if (face) {
                    ++mFaceCount;

                    cv::Point leftPupilCDF {};
                    cv::Point rightPupilCDF {};
                    cv::Point leftPupilEdge;
                    cv::Point rightPupilEdge;
                    cv::Point leftPupilGPF;
                    cv::Point rightPupilGPF;

                    auto [leftEye, rightEye] = CObjectDetection::DetectEyes(img, *face);

                    if (!leftEye.empty()) {

                        cv::Mat leftEyeImg{ leftEye.height, leftEye.width, img.type() };
                        img(leftEye).copyTo(leftEyeImg);

                        leftPupilCDF = CObjectDetection::DetectPupilCDF(leftEyeImg);
                        leftPupilEdge = CObjectDetection::DetectPupilEdge(leftEyeImg);
                        leftPupilGPF = CObjectDetection::DetectPupilGPF(leftEyeImg);

                        leftPupilCDF.x += leftEye.x;
                        leftPupilCDF.y += leftEye.y;

                        leftPupilEdge.x += leftEye.x;
                        leftPupilEdge.y += leftEye.y;

                        leftPupilGPF.x += leftEye.x;
                        leftPupilGPF.y += leftEye.y;
                    }

                    if (!rightEye.empty()) {
                        cv::Mat rightEyeImg { rightEye.height, rightEye.width, img.type() };
                        img(rightEye).copyTo(rightEyeImg);

                        rightPupilCDF = CObjectDetection::DetectPupilCDF(rightEyeImg);
                        rightPupilEdge = CObjectDetection::DetectPupilEdge(rightEyeImg);
                        rightPupilGPF = CObjectDetection::DetectPupilGPF(rightEyeImg);

                        rightPupilCDF.x += rightEye.x;
                        rightPupilCDF.y += rightEye.y;

                        rightPupilEdge.x += rightEye.x;
                        rightPupilEdge.y += rightEye.y;

                        rightPupilGPF.x += rightEye.x;
                        rightPupilGPF.y += rightEye.y;
                    }

                    if (!leftEye.empty() && !rightEye.empty()) {
                        ++mEyesCount;
                        double errorCDF = cBioIDPos.Error(leftPupilCDF, rightPupilCDF);
                        double errorEdge = cBioIDPos.Error(leftPupilEdge, rightPupilEdge);
                        double errorGPF = cBioIDPos.Error(leftPupilGPF, rightPupilGPF);
                        for (double maxError = 0; maxError <= mErrorThreshold; maxError += dInterval) { //-V1034

                            if (errorCDF < maxError) {
                                if (mCDFPoints.find(maxError) == mCDFPoints.end())
                                    mCDFPoints.emplace(maxError, 1.);
                                else
                                    mCDFPoints[maxError] += 1;
                            }

                            else if (mCDFPoints.find(maxError) == mCDFPoints.end())
                                mCDFPoints.emplace(maxError, 0);

                            if (errorEdge < maxError) {
                                if (mEdgePoints.find(maxError) == mEdgePoints.end())
                                    mEdgePoints.emplace(maxError, 1.);
                                else
                                    mEdgePoints[maxError] += 1;
                            } else if (mEdgePoints.find(maxError) == mEdgePoints.end())
                                mEdgePoints.emplace(maxError, 0);

                            if (errorGPF < maxError) {
                                if (mGPFPoints.find(maxError) == mGPFPoints.end())
                                    mGPFPoints.emplace(maxError, 1.);
                                else
                                    mGPFPoints[maxError] += 1;
                            } else if (mGPFPoints.find(maxError) == mGPFPoints.end())
                                mGPFPoints.emplace(maxError, 0);

                            if (fabs(maxError - dInterval) > mErrorThreshold && fabs(maxError - mErrorThreshold) > 0) {
                                maxError = mErrorThreshold - dInterval;
                            }
                        }
                    }
                }
            }
        }

        mProgCtrl.SetPos(static_cast<int>(i));
        UpdateWindow();
    }
    cWait.Restore();
    for (auto it = mCDFPoints.begin(); it != mCDFPoints.end(); ++it) {
        it->second *= 100. / mEyesCount;
    }
    for (auto it = mEdgePoints.begin(); it != mEdgePoints.end(); ++it) {
        it->second *= 100. / mEyesCount;
    }
    for (auto it = mGPFPoints.begin(); it != mGPFPoints.end(); ++it) {
        it->second *= 100. / mEyesCount;
    }

    SetDlgItemInt(IDC_IMGD, static_cast<int>(mImgCount));
    SetDlgItemInt(IDC_FACED, static_cast<int>(mFaceCount));
    SetDlgItemInt(IDC_EYESD, static_cast<int>(mEyesCount));

    Plot();
}

IMPLEMENT_SERIAL(SBioIDEyeCenters, CObject, 1)

void SBioIDEyeCenters::Serialize(CArchive& ar)
{
    CObject::Serialize(ar);

    if (ar.IsLoading()) {
        CString lineU;
        ar.ReadString(lineU);
        char* pBuffer = reinterpret_cast<char*>(lineU.GetBuffer(15));

        CStringA line(pBuffer);
        auto lineStart = line.Find('\n');
        lineStart = line.Find('\n', lineStart + 1);
        lineStart = line.Find('\n', lineStart + 1);
        auto lineEnd = line.Find('\n', lineStart + 1);
        auto space = line.Find(' ', lineStart + 1);

        rightEye.x = atoi(line.Mid(lineStart, space - lineStart));
        rightEye.y = atoi(line.Mid(space, lineEnd - lineStart));

        lineStart = lineEnd;
        lineEnd = line.Find('\n', lineStart + 1);

        space = line.Find(' ', lineStart + 1);

        leftEye.x = atoi(line.Mid(lineStart, space - lineStart));
        leftEye.y = atoi(line.Mid(space, lineEnd - lineStart));

        lineU.ReleaseBuffer();
    }
}

BOOL CComparerDlg::OnInitDialog()
{

    CDialogEx::OnInitDialog();

    mErrorThreshold = DEF_MAXTHRESHOLD;
    mPointsNumber = DEF_POINTNUMBER;

    SetDlgItemDouble(this, IDC_THRESH, mErrorThreshold);
    SetDlgItemInt(IDC_EPOINTNUM, static_cast<int>(mPointsNumber));

    mImgCount = 0;
    mFaceCount = 0;
    mEyesCount = 0;

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

    mGraph.SetXLabel(L"ErrorThreshold");
    mGraph.SetYLabel(L"Detection Rate [%]");

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
    mGraph.ClearGraph();
    SetupGraph();
    if (IsDlgButtonChecked(IDC_CDFG))
        for (auto it = mCDFPoints.begin(); it != mCDFPoints.end(); ++it) {
            mGraph.PlotXY(it->first, it->second, 0);
        }
    if (IsDlgButtonChecked(IDC_EDGEG))
        for (auto it = mEdgePoints.begin(); it != mEdgePoints.end(); ++it) {
            mGraph.PlotXY(it->first, it->second, 1);
        }
    if (IsDlgButtonChecked(IDC_GPFG))
        for (auto it = mGPFPoints.begin(); it != mGPFPoints.end(); ++it) {
            mGraph.PlotXY(it->first, it->second, 2);
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

    mGraph.SetRange(0., mErrorThreshold, -10, 100);

    mGraph.SetElementLinetype(0);
    mGraph.SetElementWidth(2);
    mGraph.SetElementLineColor(RGB(255, 0, 0));

    mGraph.AddElement();

    mGraph.SetElementLineColor(RGB(0, 0, 255));
    mGraph.SetElementWidth(2);
    mGraph.SetElementLinetype(0);

    mGraph.AddElement();

    mGraph.SetElementLineColor(RGB(0, 255, 0));
    mGraph.SetElementWidth(2);
    mGraph.SetElementLinetype(0);
}

void CComparerDlg::OnBnClickedExport()
{
    auto CDFNum = mCDFPoints.size();
    auto GPFNum = mGPFPoints.size();
    auto EDGENum = mEdgePoints.size();

    auto points = MAX(CDFNum, MAX(GPFNum, EDGENum));
    CFileDialog fileDlg(FALSE, NULL, NULL, 4 | 2, L"Text Files(*.txt)|*.txt|All(*.*)|*.*||", this);
    if (fileDlg.DoModal() == IDOK) {
        CStdioFile exportFile;
        exportFile.Open(fileDlg.GetPathName(), CFile::modeWrite | CFile::modeCreate);
        if (points) {
            auto itCdf = mCDFPoints.begin();
            auto itGpf = mGPFPoints.begin();
            auto itEdge = mEdgePoints.begin();

            for (size_t i = 0; i < points; ++i) {
                double t = itCdf->first;
                CString line;
                line.Format(L"%f", t);

                if (IsDlgButtonChecked(IDC_CDFG)) {
                    double val = itCdf->second;
                    CString strVal;
                    strVal.Format(L"\t%f", val);
                    line += strVal;
                }
                if (IsDlgButtonChecked(IDC_GPFG)) {
                    double val = itGpf->second;
                    CString strVal;
                    strVal.Format(L"\t%f", val);
                    line += strVal;
                }
                if (IsDlgButtonChecked(IDC_EDGEG)) {
                    double val = itEdge->second;
                    CString strVal;
                    strVal.Format(L"\t%f", val);
                    line += strVal;
                }
                line += "\n";
                exportFile.WriteString(line.GetBuffer(100));
                itCdf++;
                itGpf++;
                itEdge++;
                line.ReleaseBuffer();
            }
        }
        exportFile.Close();
    }
}
