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

#include "Graph.hpp"
#include "afxcmn.h"
#include "stdafx.h"
#include <map>
#include <vector>

class CComparerDlg : public CDialogEx {
    DECLARE_DYNAMIC(CComparerDlg)

private:
    int m_iImgCount {};
    int m_iFaceCount {};
    int m_iEyesCount {};
    int m_iPointsNumber {};
    double m_dErrorThreshold {};
    std::vector<CString> m_lImages;
    std::map<double, double> m_CDFPoints;
    std::map<double, double> m_EdgePoints;
    std::map<double, double> m_GPFPoints;

    void Plot(void);
    void SetupGraph(void);

public:
    CComparerDlg(CWnd* pParent = NULL);
    virtual ~CComparerDlg();
    enum { IDD = IDD_COMPARER }; 
    CGraph m_cGraph;
    CProgressCtrl m_cProgCtrl;

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

    double Error(CvPoint cLeftEyeEst, CvPoint cRightEyeEst)
    {
        double dRealDist = Distance(cLeftEye, cRightEye);
        double dMaxEstDist = MAX(Distance(cLeftEye, cLeftEyeEst), Distance(cRightEye, cRightEyeEst));
        return dMaxEstDist / dRealDist;
    }

private:
    CvPoint cLeftEye;
    CvPoint cRightEye;

    double Distance(CvPoint p1, CvPoint p2)
    {
        return std::sqrt((double)((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)));
    }
};

#endif