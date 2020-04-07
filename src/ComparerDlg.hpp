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

#ifndef COMPRARERDLG_HPP
#define COMPRARERDLG_HPP

#include "resource.h"
#include "stdafx.h"
#include "Graph.hpp"
#include <map>
#include <vector>

class CComparerDlg : public CDialogEx {
    DECLARE_DYNAMIC(CComparerDlg)

private:
    int mImgCount {};
    int mFaceCount {};
    int mEyesCount {};
    int mPointsNumber {};
    double mErrorThreshold {};
    std::vector<CString> mImagePaths;
    std::map<double, double> mCDFPoints;
    std::map<double, double> mEdgePoints;
    std::map<double, double> mGPFPoints;

    void Plot(void);
    void SetupGraph(void);

public:
    CComparerDlg(CWnd* pParent = NULL);
    virtual ~CComparerDlg();
    enum { IDD = IDD_COMPARER };
    CGraph mGraph;
    CProgressCtrl mProgCtrl;

    virtual BOOL OnInitDialog();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();
    afx_msg void OnBnClickedCdfg();
    afx_msg void OnBnClickedEdgeg();
    afx_msg void OnBnClickedGpfg();
    afx_msg void OnBnClickedExport();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
};

class SBioIDEyeCenters : public CObject {
    DECLARE_SERIAL(SBioIDEyeCenters)

public:
    void Serialize(CArchive& ar);

    double Error(cv::Point leftEyeEst, cv::Point rightEyeEst)
    {
        double realDist = Distance(leftEye, rightEye);
        double maxEstDist = MAX(Distance(leftEye, leftEyeEst), Distance(rightEye, rightEyeEst));
        return maxEstDist / realDist;
    }

private:
    cv::Point leftEye;
    cv::Point rightEye;

    double Distance(cv::Point p1, cv::Point p2)
    {
        return std::sqrt(static_cast<double>((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)));
    }
};

#endif
