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

#include "ObjectDetection.hpp"
#include <deque>
#include <iostream>
#include <map>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <queue>

#undef max
#undef min

void CObjectDetection::Init()
{
    mFacesCascade.load("haarcascade_frontalface_alt.xml");
    mEyesCascade.load("haarcascade_eye.xml");
}

std::optional<cv::Rect> CObjectDetection::DetectFace(const cv::Mat& img)
{
    constexpr auto resizeRatio = 2;
    constexpr double scaleFactor = 1.1;
    constexpr auto minNeighbours = 2;
    cv::Mat smallImg;
    cv::resize(img, smallImg, cv::Size {}, 1. / resizeRatio, 1. / resizeRatio);
    std::vector<cv::Rect> faces;
    mFacesCascade.detectMultiScale(smallImg, faces, scaleFactor, minNeighbours, cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));
#ifdef DEBUG
    cv::imwrite("small_face.jpg", smallImg);
#endif
    if (faces.size() == 0) {
        return std::nullopt;
    }

    auto face = faces[0];
    face.x *= resizeRatio;
    face.y *= resizeRatio;
    face.width *= resizeRatio;
    face.height *= resizeRatio;
    return face;
}

std::pair<cv::Rect, cv::Rect> CObjectDetection::DetectEyes(const cv::Mat& img, const cv::Rect& face)
{
    auto searchImageHeight = face.height / 3;
    auto searchImageHeightOffset = face.height / 5;
    auto searchImageWidth = face.width;

    auto ROI = cv::Rect(face.x, face.y + searchImageHeightOffset, searchImageWidth, searchImageHeight);
    if ((ROI.x + ROI.width >= img.cols) || (ROI.y + ROI.height >= img.rows)) {
        return {};
    }

    cv::Mat eyeSearchImg { searchImageHeight, searchImageWidth, CV_8UC3 };
    img(ROI).copyTo(eyeSearchImg);

#ifdef DEBUG
    // auto pSearchDrawnImg = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
    //cvCopy(img, pSearchDrawnImg);
    //cvRectangle(pSearchDrawnImg, cvPoint(face.x, face.y + iSearchImageHeightOffset), cvPoint(face.x + face.width, face.y + iSearchImageHeight + iSearchImageHeightOffset), CV_RGB(0, 0, 0), 1, 0, 0);
    //cv::  cvSaveImage("eye_search.jpg",pSearchDrawnImg);
    //cvReleaseImage(&pSearchDrawnImg);
#endif DEBUG

    constexpr auto scaleFactor = 1.1;
    constexpr auto minNeigbours = 5;

    std::vector<cv::Rect> eyes;
    mEyesCascade.detectMultiScale(eyeSearchImg, eyes, scaleFactor, minNeigbours, cv::CASCADE_SCALE_IMAGE);

    auto distFromLeft = 0;
    auto distFromRight = 0;

    cv::Rect leftEye {};
    cv::Rect rightEye {};

    for (auto eye : eyes) {
        eye.x += ROI.x;
        eye.y += ROI.y;

        int eyeX = eye.x + eye.width / 2;
        distFromLeft = eyeX - face.x;
        distFromRight = face.x + face.width - eyeX;

        if (distFromLeft >= distFromRight) {
            leftEye = eye;
        } else {
            rightEye = eye;
        }
    }

    if (eyes.size() == 1) {
        if (!rightEye.empty()) {
            leftEye = rightEye;
            leftEye.x = face.x + face.width - distFromLeft - rightEye.width / 2;

        } else {
            rightEye = leftEye;
            rightEye.x = face.x + distFromRight - leftEye.width / 2;
        }
    }

    if (rightEye.empty() || leftEye.empty()) {
        return {};
    }
    return { leftEye, rightEye };
#if 0
    auto pEyeDrawnImg = cvCreateImage(img->nSize, img->depth, img->nChannels);
    cvCopy(img, pEyeDrawnImg);
    cvRectangle(pEyeDrawnImg, cvPoint(pLeftEye->x, pLeftEye->y), cvPoint(pLeftEye->x + pLeftEye->width, pLeftEye->y + pLeftEye->height), CV_RGB(0, 0, 0), 1, 0, 0);
    cvRectangle(pEyeDrawnImg, cvPoint(pRightEye->x, pRightEye->y), cvPoint(pRightEye->x + pRightEye->width, pRightEye->y + pRightEye->height), CV_RGB(0, 0, 0), 1, 0, 0);
    cvSaveImage("eye_drawn.jpg", pEyeDrawnImg);
    cvReleaseImage(&pEyeDrawnImg);
#endif
}

cv::Point CObjectDetection::DetectPupilCDF(const cv::Mat& eyeImg)
{
    const auto imageSize = cv::Size { eyeImg.cols, eyeImg.rows };
    cv::Mat grayEyeImg { imageSize, CV_8UC1 };
    cv::Mat grayEyeBinaryImg { imageSize, CV_8UC1 };
    cv::Mat grayEyeBinaryErodedImg { imageSize, CV_8UC1 };
    cv::Mat grayEyeErodedImg { imageSize, CV_8UC1 };

    cv::cvtColor(eyeImg, grayEyeImg, cv::COLOR_BGR2GRAY);
    CFDThreshold(grayEyeImg, grayEyeBinaryImg, 0.05);

    auto erosionKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size { 2, 2 }, cv::Point { 0, 0 });
    cv::erode(grayEyeBinaryImg, grayEyeBinaryErodedImg, erosionKernel, cv::Point(0, 0));
    cv::erode(grayEyeImg, grayEyeErodedImg, erosionKernel, cv::Point(0, 0));

    cv::Point PMI {};
    cv::Point maxLoc {};
    double minVal {};
    double maxVal {};

    cv::minMaxLoc(grayEyeImg, &minVal, &maxVal, &PMI, &maxLoc);

    constexpr auto avgRegionSize = 10;
    constexpr auto regionIrisSize = static_cast<decltype(avgRegionSize)>(1.5 * avgRegionSize);
    const auto leftX = std::max(0, PMI.x - avgRegionSize / 2);
    const auto rightX = std::min(grayEyeImg.cols - 1, PMI.x + avgRegionSize / 2);
    const auto topY = std::max(0, PMI.y - avgRegionSize / 2);
    const auto bottomY = std::min(grayEyeImg.rows - 1, PMI.y + avgRegionSize / 2);
    const auto avgRegionWidth = rightX - leftX + 1;
    const auto avgRegionHeight = bottomY - topY + 1;
    const auto ROI = grayEyeImg(cv::Rect { leftX, topY, avgRegionWidth, avgRegionHeight });
    const int avgThreshold = static_cast<int>(cv::mean(ROI)[0]);

#if 0

    cvNamedWindow("eroded", 1);
    cvNamedWindow("eroded_bin", 1);
    cvNamedWindow("gray", 1);
    cvNamedWindow("binary", 1);
    auto eroded = cvCreateImage(cvSize(177, 122), 8, 1);
    auto gray = cvCreateImage(cvSize(177, 122), 8, 1);
    auto binary = cvCreateImage(cvSize(177, 122), 8, 1);
    auto eroded_bin = cvCreateImage(cvSize(177, 122), 8, 1);

    cvResize(pGrayEyeErodedImg, eroded);
    cvResize(pGrayEyeImg, gray);
    cvResize(pGrayEyeBinaryErodedImg, eroded_bin);
    cvResize(pGrayEyeBinaryImg, binary);

    //cv::imwrite("input_edge.jpg"

    cv::imwrite("input.jpg", cv::cvarrToMat(gray));
    cv::imwrite("cdf.jpg", cv::cvarrToMat(binary));
    cv::imwrite("cdf_eroded.jpg", cv::cvarrToMat(eroded_bin));
    cv::imwrite("eroded.jpg", cv::cvarrToMat(eroded));

    cvShowImage("eroded", eroded);
    cvShowImage("gray", gray);
    cvShowImage("binary", binary);
    cvShowImage("eroded_bin", eroded_bin);
    cvReleaseImage(&gray);
    cvReleaseImage(&eroded_bin);
    cvReleaseImage(&eroded);
    cvReleaseImage(&binary);

#endif
    const auto irisLeftX = std::max(0, PMI.x - regionIrisSize / 2);
    const auto irisRightX = std::min(grayEyeImg.cols - 1, PMI.x + regionIrisSize / 2);
    const auto irisTopY = std::max(0, PMI.y - regionIrisSize / 2);
    const auto irisBottomY = std::min(grayEyeImg.rows - 1, PMI.y + regionIrisSize / 2);

    const auto irisWidth = irisRightX - irisLeftX + 1;
    const auto irisHeight = irisBottomY - irisTopY + 1;

    auto total = 0;
    auto centerX = 0;
    auto centerY = 0;

    for (auto x = irisLeftX; x <= irisRightX; ++x)
        for (auto y = irisTopY; y <= irisBottomY; ++y) {
            int intensity = grayEyeErodedImg.at<unsigned char>(y, x);
            if (intensity < avgThreshold) {
                centerX += x;
                centerY += y;
                ++total;
            }
        }

    if (total != 0) {
        centerX = cvRound(centerX / total);
        centerY = cvRound(centerY / total);
    }
    return { centerX, centerY };
}

double CObjectDetection::CFDThreshold(const cv::Mat& eyeImg, cv::Mat& eyeImgOut, const double threshold)
{
    const int bins = 256;
    int histSize[] = { bins };
    float ranges[] = { 0, static_cast<float>(bins - 1) };
    const float* histRanges[] = { ranges };
    int channels[] = { 0 };

    cv::Mat histogram {};
    cv::calcHist(&eyeImg, 1, channels, {}, histogram, 1, histSize, histRanges);

    double maxValue = 0;
    double minValue = 0;
    cv::minMaxLoc(histogram, &minValue, &maxValue);

    const auto totalPixelCount = eyeImg.total();
    auto passedPixelCount = 0;
    for (auto y = 0; y < eyeImg.rows; ++y) {
        for (auto x = 0; x < eyeImg.cols; ++x) {
            const int value = eyeImg.at<unsigned char>(y, x);
            double CDF = 0;
            for (auto i = 0; i <= value; ++i) {
                double fHistValue = histogram.at<float>(i) / totalPixelCount;
                CDF += fHistValue;
            }
            unsigned char outPixel = 0;
            if (CDF <= threshold) {
                outPixel = 255;
                ++passedPixelCount;
            }
            eyeImgOut.at<unsigned char>(y, x) = outPixel;
        }
    }
    return static_cast<double>(passedPixelCount) / passedPixelCount;
}

template <class T>
struct greater_second {
    inline bool operator()(const T& lhs, const T& rhs)
    {
        return lhs.second < rhs.second;
    }
};

typedef std::pair<int, double> t_data;
typedef std::priority_queue<t_data, std::deque<t_data>, greater_second<t_data>> t_queue;


cv::Point CObjectDetection::DetectPupilEdge(const cv::Mat& eyeImg)
{
    const int thresholdX = static_cast<int>(0.25 * eyeImg.cols);
    const int thresholdY = static_cast<int>(0.25 * eyeImg.rows);
    int iLimitY = thresholdY;

    const auto imageSize = cv::Size { eyeImg.cols, eyeImg.rows };

    cv::Mat grayImg { imageSize, CV_8UC1 };
    cv::Mat normalizedImg { imageSize, CV_8UC1 };

    cv::cvtColor(eyeImg, grayImg, cv::COLOR_BGR2GRAY);
    cv::normalize(grayImg, normalizedImg, 0.0, 255.0, cv::NORM_MINMAX);

    cv::Mat cannyImg { imageSize, CV_8UC1 };
    const auto avgIntensity = cv::mean(normalizedImg)[0];
    const auto lowThreshold = avgIntensity * 1.5;
    const auto highThreshold = avgIntensity * 2.0;

    cv::Canny(normalizedImg, cannyImg, lowThreshold, highThreshold);

    std::map<int, int> hIntersections;
    std::map<int, int> vIntersections;

    for (auto x = 0; x < cannyImg.cols; ++x)
        for (auto y = 0; y < cannyImg.rows; ++y) {
            if (y > iLimitY)
                if (cannyImg.at<unsigned char>(y, x) != 0) {
                    if (vIntersections.find(x) == vIntersections.end()) {
                        vIntersections.emplace(x, 1);
                    } else
                        vIntersections[x] = vIntersections[x] + 1;
                }
        }
    for (auto y = 0; y < cannyImg.rows; ++y)
        for (auto x = 0; x < cannyImg.cols; ++x) {
            if (y > iLimitY)
                if (cannyImg.at<unsigned char>(y, x) != 0) {
                    if (hIntersections.find(y) == hIntersections.end()) {
                        hIntersections.emplace(y, 1);
                    } else
                        hIntersections[y] = hIntersections[y] + 1;
                }
        }

    t_queue qVertical(vIntersections.begin(), vIntersections.end());
    t_queue qHorizontal(hIntersections.begin(), hIntersections.end());

    int xBorder1 = 0, xBorder2 = 0;
    int yBorder1 = 0, yBorder2 = 0;

    if (!qVertical.empty()) {
        xBorder1 = qVertical.top().first;
        qVertical.pop();
    }

    while (!qVertical.empty()) {

        xBorder2 = qVertical.top().first;
        if (abs(xBorder2 - xBorder1) > thresholdX)
            break;
        qVertical.pop();
    }

    if (!qHorizontal.empty()) {
        yBorder1 = qHorizontal.top().first;
        qHorizontal.pop();
    }

    while (!qHorizontal.empty()) {
        yBorder2 = qHorizontal.top().first;

        if (abs(yBorder2 - yBorder1) > thresholdY)
            break;
        qHorizontal.pop();
    }
    cv::Point pupil {};
    pupil.x = (xBorder1 + xBorder2) / 2;
    pupil.y = (yBorder1 + yBorder2) / 2;

#ifdef DEBUG_

    cv::imwrite("canny.jpg", cv::cvarrToMat(pCannyEyeImg));
    cvDrawLine(pCannyEyeImg, cvPoint(xBorder1, 0), cvPoint(xBorder1, pEyeImg->height), CV_RGB(127, 127, 127), 1, 8, 0);
    cvDrawLine(pCannyEyeImg, cvPoint(xBorder2, 0), cvPoint(xBorder2, pEyeImg->height), CV_RGB(127, 127, 127), 1, 8, 0);
    cvDrawLine(pCannyEyeImg, cvPoint(0, yBorder1), cvPoint(pEyeImg->width, yBorder1), CV_RGB(127, 127, 127), 1, 8, 0);
    cvDrawLine(pCannyEyeImg, cvPoint(0, yBorder2), cvPoint(pEyeImg->width, yBorder2), CV_RGB(127, 127, 127), 1, 8, 0);
    cv::imwrite("canny_inter.jpg", cv::cvarrToMat(pCannyEyeImg));
    cvDrawLine(pNormalized, cvPoint(xBorder2, 0), cvPoint(xBorder2, pEyeImg->height), CV_RGB(0, 0, 0), 1, 8, 0);
    cvDrawLine(pNormalized, cvPoint(xBorder1, 0), cvPoint(xBorder1, pEyeImg->height), CV_RGB(0, 0, 0), 1, 8, 0);
    cvDrawLine(pNormalized, cvPoint(0, yBorder1), cvPoint(pEyeImg->width, yBorder1), CV_RGB(0, 0, 0), 1, 8, 0);
    cvDrawLine(pNormalized, cvPoint(0, yBorder2), cvPoint(pEyeImg->width, yBorder2), CV_RGB(0, 0, 0), 1, 8, 0);

    auto normdIsp = cvCreateImage(cvSize(119, 104), IPL_DEPTH_8U, 1);
    cvResize(pNormalized, normdIsp);
    cvNamedWindow("gray");
    cvShowImage("gray", normdIsp);
    cvNamedWindow("canny");
    IplImage* pCannyDispl = cvCreateImage(cvSize(119, 104), IPL_DEPTH_8U, 1);

    cvResize(pCannyEyeImg, pCannyDispl);
    cvReleaseImage(&normdIsp);
    cvShowImage("canny", pCannyDispl);

    cv::imwrite("input_edge.jpg", cv::cvarrToMat(pGrayEyeImg));
    cv::imwrite("edge_gray.jpg", cv::cvarrToMat(pNormalized));
    cvReleaseImage(&pCannyDispl);
#endif

    return pupil;
}

cv::Point CObjectDetection::DetectPupilGPF(const cv::Mat& eyeImg)
{
    constexpr double alfa = 0;
    const auto imageSize = cv::Size { eyeImg.cols, eyeImg.rows };

    int distLimitW = static_cast<int>(imageSize.width * 0.15);
    int distLimitH = static_cast<int>(imageSize.width * 0.15);

    t_queue qGPFH;
    t_queue qGPFV;
#ifdef DEBUG_
    IplImage* pDisplay = cvCreateImage(cvSize(250, 150), 8, 1);
    IplImage* pDisplay2 = cvCreateImage(cvSize(250, 150), 8, 1);
    IplImage* pDisplay3 = cvCreateImage(cvSize(250, 150), 8, 1);
#endif

    cv::Mat grayImg { imageSize, CV_8UC1 };
    cv::cvtColor(eyeImg, grayImg, cv::COLOR_BGR2GRAY);

    cv::Mat grayImgH = grayImg.clone();
    cv::Mat grayImgV = grayImg.clone();

    struct {
        cv::Point lastPoint {};
        cv::Point nextPoint {};
        cv::Point lastPointDer {};
        cv::Point nextPointDer {};
    } H, V;

    double currentGPFH = 0;
    double derGPFH = 0;
    double prevDerGPFH = 0;

    const int yLimit = static_cast<int>(imageSize.height * 0.25);
    const int yLimit2 = static_cast<int>(imageSize.height * 0.8);

    for (int y = 0; y < imageSize.height; ++y) {
        const double nextGPFH = GPFH(grayImg, y, 0, imageSize.height - 1, alfa);
        if (y > 0) {
            derGPFH = nextGPFH - currentGPFH;
            const double absDerH = abs(derGPFH);
            H.nextPointDer.x = static_cast<int>(absDerH * 0.4);
            H.nextPointDer.y = y;

            if (y > 1)
                cv::line(grayImgH, H.lastPointDer, H.nextPointDer, { 255 }, 1, 8);

            prevDerGPFH = derGPFH;

            if (y > yLimit && y < yLimit2)
                qGPFH.push(t_data(y, absDerH));

            H.lastPointDer = H.nextPointDer;
        }

        H.nextPoint.x = imageSize.width - 1 - static_cast<int>(nextGPFH * 0.05);
        H.nextPoint.y = y;

        currentGPFH = nextGPFH;

        if (y > 0) {
            cv::line(grayImgH, H.nextPoint, H.lastPoint, { 255 }, 1, 8);
        }
        H.lastPoint = H.nextPoint;
    }

    auto eyeLid1 = qGPFH.top().first;
    auto eyeLid2 = 0;
    qGPFH.pop();
    while (!qGPFH.empty()) {
        eyeLid2 = qGPFH.top().first;
        qGPFH.pop();
        if (abs(eyeLid1 - eyeLid2) > distLimitH) {
            break;
        }
    }

    if (eyeLid1 > eyeLid2) {
        std::swap(eyeLid1, eyeLid2);
    }

    double currentGPFV = 0;
    int xLimit = static_cast<int>(imageSize.width * 0.0);
    int xLimit2 = static_cast<int>(imageSize.width * 1.0);
    for (int x = 0; x < imageSize.width; ++x) {
        double nextGPFV = GPFV(grayImg, x, eyeLid1, eyeLid2, alfa);

        if (x > 0) {
            double derGPFV = nextGPFV - currentGPFV;
            double absDerV = abs(derGPFV);
            V.nextPointDer.x = x;
            V.nextPointDer.y = imageSize.width - 1 - (int)(absDerV * 0.5);

            if (x > 1)
                cv::line(grayImgV, V.lastPointDer, V.nextPointDer, cv::Scalar(255), 1, 8, 0);

            if (x > xLimit && x < xLimit2)
                qGPFV.push(t_data(x, absDerV));

            V.lastPointDer = V.nextPointDer;
        }

        V.nextPoint.x = x;
        V.nextPoint.y = imageSize.height - 1 - static_cast<int>(nextGPFV * 0.1);

        currentGPFV = nextGPFV;
        if (x > 0) {
            cv::line(grayImgV, V.lastPoint, V.nextPoint, cv::Scalar { 255 }, 1, 8, 0);
        }

        V.lastPoint = V.nextPoint;
    }

    auto eyeCor1 = qGPFV.top().first;
    int eyeCor2 {};
    qGPFV.pop();

    while (!qGPFV.empty()) {
        eyeCor2 = qGPFV.top().first;
        qGPFV.pop();

        if (abs(eyeCor1 - eyeCor2) > distLimitH) {
            break;
        }
    }

#ifdef DEBUG_
    cvDrawLine(pGrayEyeImg, cvPoint(iEyeCor1, 0), cvPoint(iEyeCor1, pEyeImg->height), CV_RGB(255, 0, 0), 1, 8, 0);
    cvDrawLine(pGrayEyeImg, cvPoint(iEyeCor2, 0), cvPoint(iEyeCor2, pEyeImg->height), CV_RGB(255, 0, 0), 1, 8, 0);
    cvDrawLine(pGrayEyeImg, cvPoint(0, iEyeLid1), cvPoint(pEyeImg->width, iEyeLid1), CV_RGB(0, 255, 0), 1, 8, 0);
    cvDrawLine(pGrayEyeImg, cvPoint(0, iEyeLid2), cvPoint(pEyeImg->width, iEyeLid2), CV_RGB(0, 255, 0), 1, 8, 0);
#endif
    cv::Point pupil {};
    pupil.y = (eyeLid1 + eyeLid2) / 2;
    pupil.x = (eyeCor1 + eyeCor2) / 2;

#ifdef DEBUG_
    cv::imwrite("gph.jpg", cv::cvarrToMat(pGrayEyeImgH));
    cv::imwrite("gpv.jpg", cv::cvarrToMat(pGrayEyeImgV));
    cv::imwrite("gpf_lines.jpg", cv::cvarrToMat(pGrayEyeImg));
    cvResize(pGrayEyeImgH, pDisplay);
    cvResize(pGrayEyeImgV, pDisplay2);
    cvResize(pGrayEyeImg, pDisplay3);
    cvNamedWindow("GPFH");
    cvNamedWindow("GPFV");
    cvNamedWindow("LINES");
    cvShowImage("GPFH", pDisplay);
    cvShowImage("GPFV", pDisplay2);
    cvShowImage("LINES", pDisplay3);
    cvReleaseImage(&pDisplay);
    cvReleaseImage(&pDisplay2);
    cvReleaseImage(&pDisplay3);
#endif
    return pupil;
}

double CObjectDetection::IPFH(const cv::Mat& eyeImg, const int y, const int x1, const int x2)
{
    double result = 0.0;
    for (int x = x1; x <= x2; ++x) {
        result += eyeImg.at<unsigned char>(y, x);
    }
    result /= (x2 - x1);
    return result;
}
double CObjectDetection::VPFH(const cv::Mat& eyeImg, const double IPF, const int y, const int x1, const int x2)
{
    double result = 0.0;
    for (auto x = x1; x <= x2; ++x) {
        result += pow(IPF - eyeImg.at<unsigned char>(y, x), 2.0);
    }
    result /= (x2 - x1);
    return result;
}
double CObjectDetection::GPFH(const cv::Mat& eyeImg, const int y, const int x1, const int x2, const double alfa)
{
    const double IPF = IPFH(eyeImg, y, x1, x2);
    const double VPF = VPFH(eyeImg, IPF, y, x1, x2);
    double result = (1 - alfa) * IPF + alfa * VPF;
    return result;
}
double CObjectDetection::IPFV(const cv::Mat& eyeImg, const int x, const int y1, const int y2)
{
    double result = 0;
    for (auto y = y1; y <= y2; ++y) {
        result += eyeImg.at<unsigned char>(y, x);
    }
    result /= (y2 - y1);
    return result;
}
double CObjectDetection::VPFV(const cv::Mat& eyeImg, const double IPF, const int x, const int y1, const int y2)
{
    double result = 0;
    for (auto y = y1; y <= y2; ++y) {
        result += pow(IPF - eyeImg.at<unsigned char>(y, x), 2);
    }
    result /= (y2 - y1);
    return result;
}
double CObjectDetection::GPFV(const cv::Mat& eyeImg, const int x, const int y1, const int y2, const double alfa)
{
    const double IPF = IPFV(eyeImg, x, y1, y2);
    const double VPF = VPFV(eyeImg, IPF, x, y1, y2);
    double result = (1 - alfa) * IPF + alfa * VPF;
    return result;
}

bool CObjectDetection::DetectLeftBlink(const cv::Mat& eyeImg, const size_t lastFramesNumber, const int varrianceThreshold, const double ratioThreshold, bool reset)
{
    static cv::Mat meanMap {};
    static cv::Mat varrianceMap {};
    static std::deque<cv::Mat> lastFramesQueue;
    return DetectBlink(eyeImg, lastFramesNumber, varrianceThreshold, ratioThreshold, meanMap, varrianceMap, lastFramesQueue, reset);
}

bool CObjectDetection::DetectRightBlink(const cv::Mat& eyeImg, const size_t lastFramesNumber, const int varrianceThreshold, const double ratioThreshold, bool reset)
{
    static cv::Mat meanMap {};
    static cv::Mat varrianceMap {};
    static std::deque<cv::Mat> lastFramesQueue;
    return DetectBlink(eyeImg, lastFramesNumber, varrianceThreshold, ratioThreshold, meanMap, varrianceMap, lastFramesQueue, reset);
}

bool CObjectDetection::DetectBlink(const cv::Mat& eyeImg,
    const size_t lastFramesNumber,
    int varrianceThreshold,
    const double ratioThreshold,
    cv::Mat& meanMap,
    cv::Mat& varrianceMap,
    std::deque<cv::Mat>& framesQueue, bool reset)
{
    bool result = false;
    if (reset) {
        framesQueue.clear();
        return result;
    }

    const auto imageSize = cv::Size { eyeImg.cols, eyeImg.rows };
    cv::Mat grayEyeImg { imageSize, CV_8UC1 };
    cv::cvtColor(eyeImg, grayEyeImg, cv::COLOR_BGR2GRAY);
    
    const auto resizedImageSize = cv::Size { meanMap.cols, meanMap.rows };
    auto queueSize = framesQueue.size();

    if (queueSize < lastFramesNumber) {
        if (queueSize == 0) {
            framesQueue.push_front(grayEyeImg);
            meanMap = { imageSize, CV_8UC1, cv::Scalar { 0 } };
            varrianceMap = { imageSize, CV_8UC1, cv::Scalar { 0 } };
        } else {
            cv::Mat resizedGrayEyeImg { resizedImageSize, CV_8UC1 };
            cv::resize(grayEyeImg, resizedGrayEyeImg, resizedImageSize);
            framesQueue.push_front(resizedGrayEyeImg);
        }
    } else {
        cv::Mat resizedGrayEyeImg { resizedImageSize, CV_8UC1 };
        cv::resize(grayEyeImg, resizedGrayEyeImg, resizedImageSize);
        framesQueue.push_front(resizedGrayEyeImg);
        framesQueue.pop_back();
    }

    queueSize = framesQueue.size();

    if (queueSize == lastFramesNumber) {

        auto tresholdedPixels = 0;

        auto xStart = 0;
        for (auto x = xStart; x < resizedImageSize.width; ++x)
            for (int y = 0; y < resizedImageSize.height; ++y) {
                size_t sum {};   
                for (const auto& frame : framesQueue) {
                    sum += frame.at<unsigned char>(y, x);
                }
                auto mean = static_cast<size_t>(sum / lastFramesNumber);
                sum = 0;

                for (const auto& frame : framesQueue) {
                    size_t intensity = frame.at<unsigned char>(y, x);
                    sum += (intensity - mean) * (intensity - mean);
                }
                auto var = static_cast<size_t>(sum / lastFramesNumber);

                if (var > static_cast<size_t>(varrianceThreshold)) {
                    ++tresholdedPixels;

                }
                meanMap.at<unsigned char>(y, x) = static_cast<unsigned char>(mean);
                varrianceMap.at<unsigned char>(y, x) = static_cast<unsigned char>(var);
            }

#ifdef DEBUG_
        IplImage* pDisplay = cvCreateImage(cvSize(150, 120), 8, 1);
        cvResize(pMeanMap, pDisplay);
        IplImage* pDisplay2 = cvCreateImage(cvSize(150, 120), 8, 1);
        cvResize(pVarrianceMap, pDisplay2);
        IplImage* pDisplay3 = cvCreateImage(cvSize(150, 120), 8, 1);
        cvResize(pGrayEyeImg, pDisplay3);
        IplImage* pDisplay4 = cvCreateImage(cvSize(150, 120), 8, 1);
        cvResize(pVarrianceThesholded, pDisplay4);
        cvNamedWindow("MEAN2");
        cvShowImage("MEAN2", pDisplay);
        cvNamedWindow("VAR2");
        cvShowImage("VAR2", pDisplay2);
        cvNamedWindow("THRESH");
        cvShowImage("THRESH", pDisplay4);
        cvSaveImage("blink_input.jpg", pDisplay3);
        cvSaveImage("blink_varriance.jpg", pDisplay2);
        cvSaveImage("blink_varriance_threshold.jpg", pDisplay4);
        cvReleaseImage(&pDisplay);
        cvReleaseImage(&pDisplay2);
        cvReleaseImage(&pDisplay3);
        cvReleaseImage(&pDisplay4);
        cvReleaseImage(&pVarrianceThesholded);
#endif
        double varRatio = (tresholdedPixels) / (static_cast<double>(resizedImageSize.height) * (resizedImageSize.width - xStart));
        if (varRatio > ratioThreshold) {
            framesQueue.clear();
            result = true;
        }
    }
    return result;
}
