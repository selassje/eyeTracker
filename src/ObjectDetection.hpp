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
    static std::optional<cv::Rect> DetectFace(const cv::Mat& img);
    static void DetectEyes(const cv::Mat& img, const cv::Rect& face, cv::Rect& leftEye, cv::Rect& rightEye);
    static cv::Point DetectPupilCDF(const cv::Mat& eyeImg);
    static CvPoint DetectPupilGPF(const cv::Mat& eyeImg);
    static CvPoint DetectPupilEdge(const cv::Mat& eyeImg);
    static BOOL DetectLeftBlink(const cv::Mat& eyeImg, const size_t lastFramesNumber, const int varrianceThreshold, const double ratioThreshold, bool reset = false);
    static BOOL DetectRightBlink(const cv::Mat& eyeImg, const size_t lastFramesNumber, const int varrianceThreshold, const double ratioThreshold, bool reset = false);
    static void Init();

private:
    static double CFDThreshold(const cv::Mat& eyeImg, cv::Mat& eyeImgOut, const double threshold);
    inline static cv::CascadeClassifier m_pFacesCascade {};
    inline static cv::CascadeClassifier m_pEyesCascade {};

    static bool DetectBlink(const cv::Mat& eyeImg,
        const size_t lastFramesNumber,
        int varrianceThreshold,
        const double ratioThreshold,
        cv::Mat& meanMap,
        cv::Mat& varrianceMap,
        std::deque<cv::Mat>& framesQueue,
        bool reset = false);

    static double IPFH(const cv::Mat& img, int y, int x1, int x2);
    static double VPFH(const cv::Mat& img, double IPF, int y, int x1, int x2);
    static double GPFH(const cv::Mat& img, int y, int x1, int x2, double alfa);

    static double IPFV(const cv::Mat& img, int x, int y1, int y2);
    static double VPFV(const cv::Mat& img, double dIPF, int x, int y1, int y2);
    static double GPFV(const cv::Mat& img, int x, int y, int y2, double alfa);
};

#endif
