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

#ifndef OBJDETECTION_HPP
#define OBJDETECTION_HPP

#include "opencv2/objdetect.hpp"
#include "stdafx.h"
#include <optional>

class CObjectDetection {
public:
    static std::optional<cv::Rect> DetectFace(IplImage* img);
    static void DetectEyes(IplImage* img, const cv::Rect& face, CvRect* pLeftEye, CvRect* pRightEye);
    static CvPoint DetectPupilCDF(IplImage* pEyeImg);
    static CvPoint DetectPupilGPF(IplImage* pEyeImg);
    static CvPoint DetectPupilEdge(IplImage* pEyeImg);
    static BOOL DetectLeftBlink(IplImage* pEyeImg, size_t iLastFramesNumber, int iVarrianceThreshold, double dRatioThreshold, BOOL bReset = FALSE);
    static BOOL DetectRightBlink(IplImage* pEyeImg, size_t iLastFramesNumber, int iVarrianceThreshold, double dRatioThreshold, BOOL bReset = FALSE);
    static void Clear();
    static void Init();

private:
    static double CFDThreshold(IplImage* pEyeImg, IplImage* pEyeImgOut, double fTreshold);
    inline static cv::CascadeClassifier m_pFacesCascade {};
    inline static cv::CascadeClassifier m_pEyesCascade {};
    static CvMemStorage* m_pStorage;

    static BOOL DetectBlink(IplImage* pEyeImg, size_t iLastFramesNumber,
        int iVarrianceThreshold, double dRatioThreshold, CvMat*& pMeanMap,
        CvMat*& pVarrianceMap, std::deque<IplImage*>& qFrames, BOOL bReset = FALSE);
    static double IPFH(IplImage* pImg, int iY, int iX1, int iX2);
    static double VPFH(IplImage* pImg, double dIPF, int iY, int iX1, int iX2);
    static double GPFH(IplImage* pImg, int iY, int iX1, int iX2, double dAlfa);

    static double IPFV(IplImage* pImg, int iX, int iY1, int iY2);
    static double VPFV(IplImage* pImg, double dIPF, int iX, int iY1, int iY2);
    static double GPFV(IplImage* pImg, int iX, int iY1, int iY2, double dAlfa);
};

#endif
