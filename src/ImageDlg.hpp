﻿/*MIT License

Copyright (c) 2020 Przemysław Kozioł

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
SOFTWARE.*/


#ifndef IMAGDLG_HPP
#define IMAGDLG_HPP

#include "stdafx.h"
#include "resource.h"
#include <vector>

class CImageDlg : public CDialog {
    DECLARE_DYNAMIC(CImageDlg)

public:
    CImageDlg(CWnd* pParent = NULL); 
    virtual ~CImageDlg();

    enum { IDD = IDD_IMAGEDLG };

    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedNext();
    afx_msg void OnBnClickedPrev();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnClickedCdf();
    afx_msg void OnClickedEdge();
    afx_msg void OnBnClickedGpf();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    DECLARE_MESSAGE_MAP()

private:
    int mLeftEyeWidth {};
    int mLeftEyeHeight {};
    int mRightEyeWidth {};
    int mRightEyeHeight {};
    int mImgWidth {};
    int mImgHeight {};
    size_t mCurrentImg {};
    size_t mImgCount {};
    CComboBox mEyeMovCombo;
    std::vector<CString> mImagePaths;
    void AnalyzeImage(const cv::Mat &img, cv::Mat& leftEyeImg, cv::Mat& rightEyeImg);
    void AnalyzeCurrentImg();
};

#endif