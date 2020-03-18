#pragma once

//#include "cv.h"
//#include "highgui.h"
#include <queue>
#include "opencv2/objdetect.hpp"

class CObjectDetection
{
public:
	static CvRect* DetectFace(IplImage *img );
	static void DetectEyes(IplImage *img,CvRect* pFace,CvRect* pLeftEye,CvRect* pRightEye);
	static CvPoint DetectPupilCDF(IplImage *pEyeImg);
	static CvPoint DetectPupilGPF(IplImage *pEyeImg);
	static CvPoint DetectPupilEdge(IplImage *pEyeImg);
	static BOOL DetectLeftBlink(IplImage *pEyeImg,int iLastFramesNumber,int iVarrianceThreshold,double dRatioThreshold,BOOL bReset=FALSE);
	static BOOL DetectRightBlink(IplImage *pEyeImg,int iLastFramesNumber,int iVarrianceThreshold,double dRatioThreshold,BOOL bReset=FALSE);
	static void Clear();
private:
	static double CFDThreshold(IplImage *pEyeImg,IplImage *pEyeImgOut,double fTreshold);
	static CvHaarClassifierCascade* m_pFacesCascade;
	static CvHaarClassifierCascade* m_pEyesCascade;
	static CvHaarClassifierCascade* m_pEyesSmallCascade;
	static CvMemStorage *m_pStorage;

	static BOOL DetectBlink(IplImage *pEyeImg,int iLastFramesNumber,
		int iVarrianceThreshold,double dRatioThreshold,CvMat*& pMeanMap,
		CvMat*& pVarrianceMap,std::deque<IplImage*>& qFrames,BOOL bReset = FALSE);
	static double IPFH(IplImage *pImg,int iY,int iX1,int iX2);
	static double VPFH(IplImage *pImg,double dIPF,int iY,int iX1,int iX2);
	static double GPFH(IplImage *pImg,int iY,int iX1,int iX2,double dAlfa);

	static double IPFV(IplImage *pImg,int iX,int iY1,int iY2);
	static double VPFV(IplImage *pImg,double dIPF,int iX,int iY1,int iY2);
	static double GPFV(IplImage *pImg,int iX,int iY1,int iY2,double dAlfa);
		


};

