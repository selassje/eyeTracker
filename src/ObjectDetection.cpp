#include "StdAfx.h"
#include "ObjectDetection.h"
#include "CameraDlg.h"
#include "EyeTracker.h"
#include <iostream>
#include <deque>
#include <map>
#include <utility>

#include "opencv2/highgui/highgui_c.h"

CvHaarClassifierCascade* CObjectDetection::m_pFacesCascade = (CvHaarClassifierCascade*)cvLoad("haarcascade_frontalface_alt.xml") ;
CvHaarClassifierCascade* CObjectDetection::m_pEyesCascade = (CvHaarClassifierCascade*)cvLoad("haarcascade_eye.xml");

CvMemStorage* CObjectDetection::m_pStorage = cvCreateMemStorage( 0 );


CvRect* CObjectDetection::DetectFace(IplImage *img)
{
	int iResizeRatio =2;
	IplImage *pSmallImg = cvCreateImage( cvSize(img->width/iResizeRatio,img->height/iResizeRatio), IPL_DEPTH_8U, 3 );
	cvResize(img, pSmallImg);
	CvSeq *pFaces = cvHaarDetectObjects(
            pSmallImg,
			m_pFacesCascade,
            m_pStorage,
            1.2,
            1,
            CV_HAAR_DO_CANNY_PRUNING);
#ifdef DEBUG
	cvSaveImage("small_face.jpg",pSmallImg);
#endif
	cvReleaseImage(&pSmallImg);
	if(pFaces->total == 0)
	{	
		return NULL;
	}

	CvRect *pFaceRect = (CvRect*)cvGetSeqElem(pFaces,0);
	
	if(pFaceRect)
	{
		pFaceRect->x *=  iResizeRatio;
		pFaceRect->y *=  iResizeRatio;

		pFaceRect->width*= iResizeRatio;
		pFaceRect->height*=iResizeRatio;
	}

	return pFaceRect;
}


void CObjectDetection::DetectEyes(IplImage *img,CvRect* pFace,CvRect* pLeftEye,CvRect* pRightEye)
{
	pLeftEye->height = 0;
	pLeftEye->width = 0;
	pRightEye->height = 0;
	pRightEye->width = 0;
	
	pLeftEye->x = -1;
	pLeftEye->y = -1;
	pRightEye->x = -1;
	pRightEye->y = -1;


	IplImage *pEyeSearchImg;
	int iSearchImageHeight =0;
	int iSearchImageWidth =0;
	int iSearchImageHeightOffset = 0;
	if(pFace != NULL)
	{
	
		iSearchImageHeight = pFace->height/3;
		iSearchImageHeightOffset =pFace->height/5;
		iSearchImageWidth =  pFace->width;
		
		CvRect cROI = cvRect(pFace->x,pFace->y + iSearchImageHeightOffset ,iSearchImageWidth,iSearchImageHeight);
		if((cROI.x + cROI.width >= img->width) || (cROI.y + cROI.height >= img->height))
		{
			pFace->width = 0;
			pFace->height = 0;
			return;
		}
		
		pEyeSearchImg = cvCreateImage( cvSize(iSearchImageWidth,iSearchImageHeight), IPL_DEPTH_8U, 3 );
		cvSetImageROI(img,cROI);
		cvCopy( img, pEyeSearchImg, NULL );	
		cvResetImageROI(img);
		#ifdef DEBUG
				auto pSearchDrawnImg = cvCreateImage(cvGetSize(img), img->depth, img->nChannels );
				cvCopy(img,pSearchDrawnImg);
				cvRectangle(pSearchDrawnImg,cvPoint(pFace->x,pFace->y + iSearchImageHeightOffset ),cvPoint(pFace->x + pFace->width,pFace->y + iSearchImageHeight+ iSearchImageHeightOffset),CV_RGB(0,0,0),1,0,0);		
				cvSaveImage("eye_search.jpg",pSearchDrawnImg);
				cvReleaseImage(&pSearchDrawnImg);
		#endif DEBUG
	}
	else
	{
		return ;
	}
	
	double dFactor = 1.2;
	int iMinNeigbours =5;
	
	CvSeq *pEyes = cvHaarDetectObjects(
            pEyeSearchImg,
			m_pEyesCascade,
            m_pStorage,
            dFactor,
            iMinNeigbours,
            CV_HAAR_DO_CANNY_PRUNING
			);
   
	for(int i  = 0;i<pEyes->total;++i)
	{

		CvRect *pEyeRect = (CvRect*)cvGetSeqElem(pEyes,i);

		if(pFace)
		{
			pEyeRect->x +=  pFace->x;
			pEyeRect->y =  pFace->y + pEyeRect->y +iSearchImageHeightOffset;
		}

		int iEyeX = pEyeRect->x + pEyeRect->width/2;
		int iDistFromLeft = iEyeX - pFace->x;
		int iDistFromRight = pFace->x + pFace->width - iEyeX;
					
		if(iDistFromLeft >=  iDistFromRight)
		{
			pLeftEye->height = pEyeRect->height;
			pLeftEye->width =  pEyeRect->width;
			pLeftEye->x =  pEyeRect->x;
			pLeftEye->y =  pEyeRect->y;
		}
		else
		{
			pRightEye->height = pEyeRect->height;
			pRightEye->width =  pEyeRect->width;
			pRightEye->x =  pEyeRect->x;
			pRightEye->y =  pEyeRect->y;
		}
		if(pEyes->total == 1)
		{
			if(pRightEye->height)
			{
				pLeftEye->width =  pRightEye->width;
				pLeftEye->height =  pRightEye->height;
				pLeftEye->x =  pFace->x + pFace->width - iDistFromLeft - (int)(pRightEye->width/2);
				pLeftEye->y =  pRightEye->y;
			}
			else
			{
				pRightEye->width =  pLeftEye->width;
				pRightEye->height =  pLeftEye->height;
				pRightEye->x =  pFace->x +  iDistFromRight- (int)(pLeftEye->width/2);
				pRightEye->y =  pLeftEye->y;
			}

			break;
		}
	}
	
	

	  #ifdef DEBUG
				auto pEyeDrawnImg = cvCreateImage(cvGetSize(img), img->depth, img->nChannels );
				cvCopy(img,pEyeDrawnImg);
				cvRectangle(pEyeDrawnImg,cvPoint(pLeftEye->x,pLeftEye->y ),cvPoint(pLeftEye->x + pLeftEye->width,pLeftEye->y + pLeftEye->height ),CV_RGB(0,0,0),1,0,0);		
				cvRectangle(pEyeDrawnImg,cvPoint(pRightEye->x,pRightEye->y ),cvPoint(pRightEye->x + pRightEye->width,pRightEye->y + pRightEye->height ),CV_RGB(0,0,0),1,0,0);		
				cvSaveImage("eye_drawn.jpg",pEyeDrawnImg);
				cvReleaseImage(&pEyeDrawnImg);
		#endif DEBUG


	cvReleaseImage(&pEyeSearchImg);
}



void  CObjectDetection::Clear()
{
	cvReleaseMemStorage(&m_pStorage);
	m_pStorage = cvCreateMemStorage(0);
}


CvPoint CObjectDetection::DetectPupilCDF(IplImage *pEyeImg)
{
	CvPoint cPupil;	
	IplImage* pGrayEyeImg = cvCreateImage( cvSize(pEyeImg->width,pEyeImg->height),8,1);
	IplImage* pGrayEyeBinaryImg = cvCreateImage(cvGetSize(pGrayEyeImg),IPL_DEPTH_8U,1);
	IplImage* pGrayEyeBinaryErodedImg = cvCreateImage(cvGetSize(pGrayEyeImg),IPL_DEPTH_8U,1);

	cvCvtColor(pEyeImg, pGrayEyeImg, CV_BGR2GRAY );

	CFDThreshold(pGrayEyeImg,pGrayEyeBinaryImg,0.05);

	IplConvKernel* pErosionKernel =  cvCreateStructuringElementEx(2,2,0,0,CV_SHAPE_RECT);
	cvErode(pGrayEyeBinaryImg,pGrayEyeBinaryErodedImg,pErosionKernel,1);
	
	IplImage* pGrayEyeErodedImg = cvCreateImage(cvGetSize(pGrayEyeImg),IPL_DEPTH_8U,1);
	cvErode(pGrayEyeImg,pGrayEyeErodedImg,pErosionKernel,1);
	
	CvPoint cPMI, cMaxLoc;
	double  dMinVal, dMaxVal;
	cvMinMaxLoc(pGrayEyeImg, &dMinVal, &dMaxVal, &cPMI, &cMaxLoc,pGrayEyeBinaryImg);

	int iAvgRegionSize = 10;
	int iRegionIrisSize = (int) (1.5 *iAvgRegionSize);
	int iLeftX = cPMI.x - iAvgRegionSize/2;
	if(iLeftX < 0)
			iLeftX = 0;	
	int iRightX = cPMI.x + iAvgRegionSize/2;
	if(iRightX  > pGrayEyeImg->width - 1)
			iRightX = pGrayEyeImg->width - 1;
	int iTopY = cPMI.y - iAvgRegionSize/2;
	if(iTopY < 0)
			iTopY = 0;	
	int iBottomY = cPMI.y + iAvgRegionSize/2;
	if(iBottomY  > pGrayEyeImg->height - 1)
			iBottomY = pGrayEyeImg->height - 1;

	int iAvgRegionWidth = iRightX - iLeftX + 1;
	int iAvgRegionHeight = iBottomY - iTopY + 1;

	cvSetImageROI(pGrayEyeImg,cvRect(iLeftX,iTopY,iAvgRegionWidth,iAvgRegionHeight));
	int iAvgTheshold = (int) cvAvg(pGrayEyeImg).val[0];
	cvResetImageROI(pGrayEyeImg);

#ifdef DEBUG
	
	cvNamedWindow("eroded",1);
	cvNamedWindow("eroded_bin",1);
	cvNamedWindow("gray",1);
	cvNamedWindow("binary",1);
	auto eroded = cvCreateImage( cvSize(177,122),8,1);
	auto gray = cvCreateImage( cvSize(177,122),8,1);
	auto binary = cvCreateImage( cvSize(177,122),8,1);
	auto eroded_bin = cvCreateImage( cvSize(177,122),8,1);

	cvResize(pGrayEyeErodedImg,eroded);
	cvResize(pGrayEyeImg,gray);
	cvResize(pGrayEyeBinaryErodedImg,eroded_bin);
	cvResize(pGrayEyeBinaryImg,binary);
	
	cvSaveImage("input.jpg",gray);
	cvSaveImage("cdf.jpg",binary);
	cvSaveImage("cdf_eroded.jpg",eroded_bin);
	cvSaveImage("eroded.jpg",eroded);


	cvShowImage("eroded",eroded);
	cvShowImage("gray",gray);
	cvShowImage("binary",binary);
	cvShowImage("eroded_bin",eroded_bin);
	cvReleaseImage(&gray);
	cvReleaseImage(&eroded_bin);
	cvReleaseImage(&eroded);
	cvReleaseImage(&binary);
	
#endif

	int iIrisLeftX = cPMI.x - iRegionIrisSize/2;
	if(iIrisLeftX < 0)
			iIrisLeftX = 0;	
	int iIrisRightX = cPMI.x + iRegionIrisSize/2;
	if(iIrisRightX  > pGrayEyeImg->width - 1)
			iIrisRightX = pGrayEyeImg->width - 1;
	int iIrisTopY = cPMI.y - iRegionIrisSize/2;
	if(iIrisTopY < 0)
			iIrisTopY = 0;	
	int iIrisBottomY = cPMI.y + iRegionIrisSize/2;
	if(iIrisBottomY  > pGrayEyeImg->height - 1)
			iIrisBottomY = pGrayEyeImg->height - 1;
	
	int iIrisWidth = iIrisRightX - iIrisLeftX + 1;
	int iIrisHeight = iIrisBottomY -iIrisTopY + 1;
	
	int iTotal=0;
	int iCenterX=0;
	int iCenterY=0;

	for( int x = iIrisLeftX ; x<=iIrisRightX;++x)
		for( int y = iIrisTopY ; y <=iIrisBottomY;++y)
	{
		int iIntensity = (int) cvGet2D(pGrayEyeErodedImg,y,x).val[0];
		if(iIntensity < iAvgTheshold)
		{
			iIntensity = 1;
			iCenterX+= iIntensity*x;
			iCenterY+= iIntensity*y;
			iTotal+=iIntensity;
		}
	}
	
	if(iTotal)
	{
		iCenterX=cvRound(iCenterX/iTotal);
		iCenterY=cvRound(iCenterY/iTotal);
	}
	cPupil.x = iCenterX;
	cPupil.y = iCenterY;

	cvReleaseStructuringElement(&pErosionKernel);
	cvReleaseImage(&pGrayEyeImg);
	cvReleaseImage(&pGrayEyeBinaryImg);
	cvReleaseImage(&pGrayEyeBinaryErodedImg);
	cvReleaseImage(&pGrayEyeErodedImg);

	return cPupil;
}

 double CObjectDetection::CFDThreshold(IplImage *pEyeImg,IplImage *pEyeImgOut,double fTreshold)
{
	int high_count = 0;	
	int passedPixels = 0;
	
	int iPixelNumber =pEyeImg->width* pEyeImg->height;

	int iBins = 256;
	int pHsize[] = {iBins};
	float fMaxValue = 0;
	float fMinValue = 0;

	float fRanges[] = {0,(float) iBins-1};
	float* pRanges[] = { fRanges };
	
	IplImage* pImagPlanes[] = {pEyeImg};

	CvHistogram* pHist = cvCreateHist( 1, pHsize,CV_HIST_ARRAY,pRanges,1);
	cvCalcHist(pImagPlanes, pHist, 0, NULL);
	cvGetMinMaxHistValue( pHist, &fMinValue, &fMaxValue);

	for(int iX = 0;iX<pEyeImg->height;++iX)
	{
		for(int iY = 0;iY<pEyeImg->width;++iY)
		{		
			int iValue = (int) cvGet2D(pEyeImg,iX,iY).val[0];
			double fCDF = 0;

			for(int i = 0;i<=iValue;++i)
			{
				double fHistValue  = cvGetReal1D(pHist,i)/iPixelNumber;
				fCDF += fHistValue;
			}
			CvScalar s;
			if(fCDF <= fTreshold)
			{
				s.val[0] = 255;
				++passedPixels;
			}
			
			else
				s.val[0] = 0;
			
			cvSet2D(pEyeImgOut,iX,iY,s);
		}
	}

	cvReleaseHist(&pHist);	

	return (double) passedPixels/ (double)iPixelNumber;
}


template<class T>struct greater_second: std::binary_function<T,T,bool>
{   
	inline bool operator()(const T& lhs, const T& rhs)   
	{      
		return lhs.second < rhs.second;   
	}
};

typedef std::pair<int,double> t_data;
typedef std::priority_queue<t_data,std::deque<t_data>,greater_second<t_data>> t_queue;

template <typename T>
void ClearQueue(T queue)
{
	while(queue.size())
	{
		queue.pop();
	}
}


CvPoint CObjectDetection::DetectPupilEdge(IplImage *pEyeImg)
{	
	CvPoint cPupil;	
	cPupil.x=-1;
	cPupil.y=-1;

	int iLimitY = (int) ( 0.25 * pEyeImg->height);
	int iThersholdX =(int) ( 0.25 * pEyeImg->width);
	int iThersholdY = (int) ( 0.25 * pEyeImg->height);

	IplImage* pNormalized = cvCreateImage( cvGetSize(pEyeImg),8,1);
	IplImage* pGrayEyeImg = cvCreateImage( cvGetSize(pEyeImg),8,1);
	IplImage* pCannyEyeImg = cvCreateImage(cvGetSize(pGrayEyeImg),IPL_DEPTH_8U,1);

	cvCvtColor(pEyeImg, pGrayEyeImg, CV_BGR2GRAY );
	cvNormalize(pGrayEyeImg,pNormalized,0,255,CV_MINMAX);
	
	int iAverageIntenisity = (int) cvAvg(pNormalized).val[0];

	int iLowThreshold = (int)(iAverageIntenisity*1.5);
	int iHighThreshold =iAverageIntenisity*2;
	
	cvCanny(pNormalized,pCannyEyeImg,iLowThreshold,iHighThreshold);

	std::map<int,int> mHIntersections;
	std::map<int,int> mVIntersections;

	for(int x = 0;x<pCannyEyeImg->width;++x)
		for(int y = 0;y<pCannyEyeImg->height;++y)
	{
		if(y > iLimitY)
		
		if(cvGet2D(pCannyEyeImg,y,x).val[0]!=0)
		{
			if(mVIntersections.find(x) == mVIntersections.end())
			{
				mVIntersections.insert(std::make_pair(x,1));
			}
			else
				mVIntersections[x] = mVIntersections[x] + 1;
		}
	}
	for(int y = 0;y<pCannyEyeImg->height;++y)
		for(int x = 0;x<pCannyEyeImg->width;++x)
		{
			if(y > iLimitY)
			if(cvGet2D(pCannyEyeImg,y,x).val[0]!=0)
			{
				if(mHIntersections.find(y) == mHIntersections.end())
				{
					mHIntersections.insert(std::make_pair(y,1));
				}
				else
					mHIntersections[y] = mHIntersections[y] + 1;
			}
		}
	
		
	t_queue qHorizontal(mHIntersections.begin(),mHIntersections.end());
	t_queue qVertical(mVIntersections.begin(),mVIntersections.end());


	int xBorder1 = 0,xBorder2 = 0;
	int yBorder1 = 0,yBorder2 = 0;


	if(!qVertical.empty()) {
		xBorder1=  qVertical.top().first;
		qVertical.pop();
	}
	
	while(!qVertical.empty()) {
	
		xBorder2 =  qVertical.top().first;
		if(abs(xBorder2 - xBorder1) > iThersholdX)
			break;
		qVertical.pop();
		
	}
	
	
	if(!qHorizontal.empty()) {
		yBorder1=  qHorizontal.top().first;
		qHorizontal.pop();

	}

	while(!qHorizontal.empty()) {
		yBorder2 =  qHorizontal.top().first;

		if(abs(yBorder2 - yBorder1) > iThersholdY)
			break;
		qHorizontal.pop();

	}


	cPupil.x = (xBorder1 + xBorder2)/2;
	cPupil.y = (yBorder1 + yBorder2)/2;

#ifdef DEBUG
	cvSaveImage("canny.jpg",pCannyEyeImg);
	cvDrawLine(pCannyEyeImg,cvPoint(xBorder1,0),cvPoint(xBorder1,pEyeImg->height),CV_RGB(127,127,127),1,8,0);
	cvDrawLine(pCannyEyeImg,cvPoint(xBorder2,0),cvPoint(xBorder2,pEyeImg->height),CV_RGB(127,127,127),1,8,0);
	cvDrawLine(pCannyEyeImg,cvPoint(0,yBorder1),cvPoint(pEyeImg->width,yBorder1),CV_RGB(127,127,127),1,8,0);
	cvDrawLine(pCannyEyeImg,cvPoint(0,yBorder2),cvPoint(pEyeImg->width,yBorder2),CV_RGB(127,127,127),1,8,0);
	cvSaveImage("canny_inter.jpg",pCannyEyeImg);
	cvSaveImage("canny_inter.jpg",pCannyEyeImg);
	cvDrawLine(pNormalized,cvPoint(xBorder2,0),cvPoint(xBorder2,pEyeImg->height),CV_RGB(0,0,0),1,8,0);
	cvDrawLine(pNormalized,cvPoint(xBorder1,0),cvPoint(xBorder1,pEyeImg->height),CV_RGB(0,0,0),1,8,0);
	cvDrawLine(pNormalized,cvPoint(0,yBorder1),cvPoint(pEyeImg->width,yBorder1),CV_RGB(0,0,0),1,8,0);
	cvDrawLine(pNormalized,cvPoint(0,yBorder2),cvPoint(pEyeImg->width,yBorder2),CV_RGB(0,0,0),1,8,0);

	auto normdIsp = cvCreateImage(cvSize(119,104),IPL_DEPTH_8U,1);
	cvResize(pNormalized,normdIsp);
	cvNamedWindow("gray");
	cvShowImage("gray",normdIsp);
	cvNamedWindow("canny");
	IplImage* pCannyDispl = cvCreateImage(cvSize(119,104),IPL_DEPTH_8U,1);
	
	cvResize(pCannyEyeImg,pCannyDispl);
	cvReleaseImage(&normdIsp);
	cvShowImage("canny",pCannyDispl);

	cvSaveImage("input_edge.jpg",pGrayEyeImg);
	cvSaveImage("edge_gray.jpg",pNormalized);
	cvReleaseImage(&pCannyDispl);
#endif

	cvReleaseImage(&pGrayEyeImg);
	cvReleaseImage(&pCannyEyeImg);
	cvReleaseImage(&pNormalized);
	return cPupil;
	
}

CvPoint CObjectDetection::DetectPupilGPF(IplImage *pEyeImg)
{	
	
	double alfa = 0;
	
	CvPoint cPupil;	
	cPupil.x=-1;
	cPupil.y=-1;

	int iWidth = pEyeImg->width;
	int iHeight = pEyeImg->height;

	int iDistLimitW = (int)( iWidth*0.15);
	int iDistLimitH = (int)( iWidth*0.15);

	t_queue qGPFH;
	t_queue qGPFV;
#ifdef DEBUG
	IplImage* pDisplay = cvCreateImage(cvSize(250,150),8,1);
	IplImage* pDisplay2 = cvCreateImage(cvSize(250,150),8,1);
	IplImage* pDisplay3 = cvCreateImage(cvSize(250,150),8,1);
#endif
	IplImage* pGrayEyeImg = cvCreateImage( cvGetSize(pEyeImg),8,1);
	IplImage* pGrayEyeImgH = cvCreateImage( cvGetSize(pEyeImg),8,1);
	IplImage* pGrayEyeImgV = cvCreateImage( cvGetSize(pEyeImg),8,1);
	cvCvtColor(pEyeImg, pGrayEyeImg, CV_BGR2GRAY );
	cvCvtColor(pEyeImg, pGrayEyeImgH, CV_BGR2GRAY );
	cvCvtColor(pEyeImg, pGrayEyeImgV, CV_BGR2GRAY );

	CvPoint cLastPoint;
	CvPoint cNextPoint;
	CvPoint cLastPointDer;
	CvPoint cNextPointDer;

	double dGPFH = 0;
	double dDerGPFH = 0;
	double dPrevDerGPFH = 0;
	
	int iYLimit = (int)( iHeight*0.25);
	int iYLimit2 = (int)( iHeight*0.8);
	
	for(int y = 0;y<iHeight;++y)
	{
		double dNextGPFH = GPFH(pGrayEyeImg,y,0,iWidth-1,alfa);
		if(y > 0)
		{	
			dDerGPFH = dNextGPFH - dGPFH;
			double absDerH =  abs(dDerGPFH);
			cNextPointDer.x = (int) (absDerH*0.4);
			cNextPointDer.y = y;
			
			if(y>1)
				cvDrawLine(pGrayEyeImgH,cLastPointDer,cNextPointDer,cvScalar(255),1,8,0);
			
			dPrevDerGPFH = dDerGPFH;
			
			if(y> iYLimit && y < iYLimit2)
				qGPFH.push(t_data(y,absDerH));	

			cLastPointDer = cNextPointDer;
		}
		
		cNextPoint.x = iWidth - 1 - (int) ( dNextGPFH*0.05);
		cNextPoint.y = y;

		dGPFH =dNextGPFH;
			
		if(y > 0)
		{
			cvDrawLine(pGrayEyeImgH,cNextPoint,cLastPoint,cvScalar(0),1,8,0);
		}
		cLastPoint = cNextPoint;

	}
	
	int iEyeLid1 = qGPFH.top().first;
	int iEyeLid2;
	qGPFH.pop();
	while(!qGPFH.empty())
	{
		iEyeLid2 = qGPFH.top().first;
		qGPFH.pop();
		if(abs(iEyeLid1 - iEyeLid2) > iDistLimitH)
		{
			break;
		}
	}
	
	if(iEyeLid1 > iEyeLid2)
	{
		int tmp = iEyeLid2;
		iEyeLid2 = iEyeLid1;
		iEyeLid1 = tmp;
	}
	
	CvPoint cLastPointV;
	CvPoint cNextPointV;
	CvPoint cLastPointDerV;
	CvPoint cNextPointDerV;
	
	
	double dGPFV = 0;
	int iXLimit = (int)( iWidth*0.0);
	int iXLimit2 = (int)( iWidth*1.0);
	for(int x = 0;x<iWidth;++x)
	{
		double dNextGPFV = GPFV(pGrayEyeImg,x,iEyeLid1,iEyeLid2,alfa);
		
		if(x > 0)
		{
			double dDerGPFV = dNextGPFV - dGPFV;
			double absDerV =  abs(dDerGPFV);
			cNextPointDerV.x = x;
			cNextPointDerV.y = iWidth - 1 - (int) (absDerV*0.5);
			
			if(x>1)
				cvDrawLine(pGrayEyeImgV,cLastPointDerV,cNextPointDerV,cvScalar(255),1,8,0);
			
			if(x> iXLimit && x < iXLimit2)
				qGPFV.push(t_data(x,absDerV));

			cLastPointDerV = cNextPointDerV;
		}
		
		cNextPointV.x = x;
		cNextPointV.y = iHeight - 1 - (int) ( dNextGPFV*0.1);
		
		dGPFV =dNextGPFV;
		if(x > 0)
		{
			cvDrawLine(pGrayEyeImgV,cLastPointV,cNextPointV,cvScalar(0),1,8,0);
		}

		cLastPointV = cNextPointV;
	}

	int iEyeCor1 = qGPFV.top().first;
	int iEyeCor2;
	qGPFV.pop();
	

	while(!qGPFV.empty())
	{
		iEyeCor2 = qGPFV.top().first;
		qGPFV.pop();
		
		if(abs(iEyeCor1 - iEyeCor2) > iDistLimitH)
		{
			break;
		}		
	}

#ifdef DEBUG
	cvDrawLine(pGrayEyeImg,cvPoint(iEyeCor1,0),cvPoint(iEyeCor1,pEyeImg->height),CV_RGB(255,0,0),1,8,0);
	cvDrawLine(pGrayEyeImg,cvPoint(iEyeCor2,0),cvPoint(iEyeCor2,pEyeImg->height),CV_RGB(255,0,0),1,8,0);
	cvDrawLine(pGrayEyeImg,cvPoint(0,iEyeLid1),cvPoint(pEyeImg->width,iEyeLid1),CV_RGB(0,255,0),1,8,0);
	cvDrawLine(pGrayEyeImg,cvPoint(0,iEyeLid2),cvPoint(pEyeImg->width,iEyeLid2),CV_RGB(0,255,0),1,8,0);
#endif
		
	cPupil.y = (iEyeLid1 + iEyeLid2)/2;	
	cPupil.x = (iEyeCor1 + iEyeCor2)/2;
	
#ifdef DEBUG
	cvSaveImage("gph.jpg",pGrayEyeImgH);
	cvSaveImage("gpv.jpg",pGrayEyeImgV);
	cvSaveImage("gpf_lines.jpg",pGrayEyeImg);
	cvResize(pGrayEyeImgH,pDisplay);
	cvResize(pGrayEyeImgV,pDisplay2);
	cvResize(pGrayEyeImg,pDisplay3);
	cvNamedWindow("GPFH");
	cvNamedWindow("GPFV");
	cvNamedWindow("LINES");
	cvShowImage("GPFH",pDisplay);
	cvShowImage("GPFV",pDisplay2);
	cvShowImage("LINES",pDisplay3);
	cvReleaseImage(&pDisplay);
	cvReleaseImage(&pDisplay2);
	cvReleaseImage(&pDisplay3);
#endif
	cvReleaseImage(&pGrayEyeImg);
	cvReleaseImage(&pGrayEyeImgH);
	cvReleaseImage(&pGrayEyeImgV);
	ClearQueue(qGPFV);
	ClearQueue(qGPFH);
	return cPupil;

}

double CObjectDetection::IPFH(IplImage *pImg,int iY,int iX1,int iX2)
{	
	double dResult = 0;
	for(int iX =iX1;iX<=iX2;++iX)
	{
		dResult+=(int) cvGet2D(pImg,iY,iX).val[0];
	}
	
	dResult/= (iX2 - iX1);
	return dResult;
}
double CObjectDetection::VPFH(IplImage *pImg,double dIPF,int iY,int iX1,int iX2)
{	
	double dResult = 0;

	for(int iX =iX1;iX<=iX2;++iX)
	{
		dResult+= pow(dIPF - cvGet2D(pImg,iY,iX).val[0],2);	
	}
	
	dResult/= (iX2 - iX1);
	return dResult;
}
double CObjectDetection::GPFH(IplImage *pImg,int iY,int iX1,int iX2,double dAlfa)
{	
	double dResult = 0;
	
	double dIPF =IPFH(pImg,iY,iX1,iX2);
	double dVPF = VPFH(pImg,dIPF,iY,iX1,iX2);

	dResult= (1-dAlfa)*dIPF + dAlfa*dVPF;
	return dResult;
}
double CObjectDetection::IPFV(IplImage *pImg,int iX,int iY1,int iY2)
{	
	double dResult = 0;
	for(int iY =iY1;iY<=iY2;++iY)
	{
		dResult+=(int) cvGet2D(pImg,iY,iX).val[0];
	}
	
	dResult/= (iY2 - iY1);
	return dResult;
}
double CObjectDetection::VPFV(IplImage *pImg,double dIPF,int iX,int iY1,int iY2)
{	
	double dResult = 0;
	for(int iY =iY1;iY<=iY2;++iY)
	{
		dResult+= pow(dIPF - cvGet2D(pImg,iY,iX).val[0],2);	
	}
	
	dResult/= (iY2 - iY1);
	return dResult;
}
double CObjectDetection::GPFV(IplImage *pImg,int iX,int iY1,int iY2,double dAlfa)
{	
	double dResult = 0;
	
	double dIPF =IPFV(pImg,iX,iY1,iY2);
	double dVPF = VPFV(pImg,dIPF,iX,iY1,iY2);

	dResult= (1-dAlfa)*dIPF + dAlfa*dVPF;
	return dResult;
}


BOOL CObjectDetection::DetectLeftBlink(IplImage *pEyeImg,int iLastFramesNumber,int iVarrianceThreshold,double dRatioThreshold,BOOL bReset )
{	
	static CvMat* pMeanMap =NULL;
	static CvMat* pVarrianceMap = NULL;
	static std::deque<IplImage*> qLastFrames;	
	return DetectBlink(pEyeImg,iLastFramesNumber,iVarrianceThreshold,dRatioThreshold,pMeanMap,pVarrianceMap,qLastFrames,bReset);

}

BOOL CObjectDetection::DetectRightBlink(IplImage *pEyeImg,int iLastFramesNumber,int iVarrianceThreshold,double dRatioThreshold,BOOL bReset )
{	
	static CvMat* pMeanMap =NULL;
	static CvMat* pVarrianceMap = NULL;
	static std::deque<IplImage*> qLastFrames;	
	return DetectBlink(pEyeImg,iLastFramesNumber,iVarrianceThreshold,dRatioThreshold,pMeanMap,pVarrianceMap,qLastFrames,bReset);
}

BOOL CObjectDetection::DetectBlink(IplImage *pEyeImg,
		int iLastFramesNumber,
		int iVarrianceThreshold,
		double dRatioThreshold,
		CvMat*& pMeanMap,
		CvMat*& pVarrianceMap,
		std::deque<IplImage*>& qLastFrames,
		BOOL bReset)
{

	int iWidth = 0;
	int iHeight = 0;
	BOOL bResult = false;
	
	if(bReset)
	{
		while(qLastFrames.size())
		{
			cvReleaseImage(&qLastFrames.back());
			qLastFrames.pop_back();

		}
		return FALSE;
	}

	IplImage *pGrayEyeImg = cvCreateImage(cvGetSize(pEyeImg),8,1);
	cvCvtColor(pEyeImg, pGrayEyeImg, CV_BGR2GRAY );

	if(pMeanMap)
	{
		iWidth =  pMeanMap->width;
		iHeight = pMeanMap->height;
	}

	int qSize = qLastFrames.size();
	
	if(qSize< iLastFramesNumber)
	{
		if(qSize == 0)
		{
			qLastFrames.push_front(pGrayEyeImg);
			
			if(pMeanMap) {
				cvReleaseMat(&pMeanMap);
				cvReleaseMat(&pVarrianceMap);
			}
			pMeanMap = cvCreateMat(pGrayEyeImg->height, pGrayEyeImg->width,CV_8UC1);
			pVarrianceMap = cvCreateMat(pGrayEyeImg->height, pGrayEyeImg->width,CV_8UC1);
			cvZero(pMeanMap);
			cvZero(pVarrianceMap);
		}
		else
		{
			IplImage *pEyeImgResized = cvCreateImage(cvSize(iWidth,iHeight),8,1);
			cvResize(pGrayEyeImg,pEyeImgResized);
			qLastFrames.push_front(pEyeImgResized);
		}
	}
	else
	{
		IplImage *pEyeImgResized = cvCreateImage(cvSize(iWidth,iHeight),8,1);
		cvResize(pGrayEyeImg,pEyeImgResized);
		qLastFrames.push_front(pEyeImgResized);
		cvReleaseImage(&qLastFrames.back());
		qLastFrames.pop_back();	
	}
	
	qSize = qLastFrames.size();

	if(qSize==iLastFramesNumber)
	{
	
		int iTresholdedPixels = 0;
#ifdef DEBUG
		auto  pVarrianceThesholded = cvCreateImage(cvGetSize(pVarrianceMap),8,1);
		cvZero(pVarrianceThesholded);
#endif
		int iXStart = 0;
		for(int x = iXStart; x<iWidth;++x)
		for(int y = 0; y<iHeight;++y)
		{
			
			int iSum = 0;
			for(auto it = qLastFrames.begin();it!=qLastFrames.end();++it)
			{
				iSum += (int) cvGet2D(*it,y,x).val[0];
			}

			int mean = iSum/iLastFramesNumber;

			iSum = 0;
			for(auto it = qLastFrames.begin();it!=qLastFrames.end();++it)
			{
				int iIntensity = (int) cvGet2D(*it,y,x).val[0];
				iSum+= (iIntensity - mean)*(iIntensity - mean);		
			}

			int var = iSum/iLastFramesNumber;
			
			if(var>iVarrianceThreshold)
			{
				++iTresholdedPixels;
#ifdef DEBUG
				cvSet2D(pVarrianceThesholded,y,x,cvScalar(255));
#endif	
			}
			cvSet2D(pMeanMap,y,x,cvScalar(mean));
			cvSet2D(pVarrianceMap,y,x,cvScalar(var));
		
		}

#ifdef DEBUG
			IplImage* pDisplay = cvCreateImage(cvSize(150,120),8,1);
			cvResize(pMeanMap,pDisplay);
			IplImage* pDisplay2 = cvCreateImage(cvSize(150,120),8,1);
			cvResize(pVarrianceMap,pDisplay2);
			IplImage* pDisplay3 = cvCreateImage(cvSize(150,120),8,1);
			cvResize(pGrayEyeImg,pDisplay3);
			IplImage* pDisplay4 = cvCreateImage(cvSize(150,120),8,1);
			cvResize(pVarrianceThesholded,pDisplay4);
			cvNamedWindow("MEAN2");
			cvShowImage("MEAN2",pDisplay);
			cvNamedWindow("VAR2");
			cvShowImage("VAR2",pDisplay2);
			cvNamedWindow("THRESH");
			cvShowImage("THRESH",pDisplay4);
			cvSaveImage("blink_input.jpg",pDisplay3);	
			cvSaveImage("blink_varriance.jpg",pDisplay2);
			cvSaveImage("blink_varriance_threshold.jpg",pDisplay4);	
			cvReleaseImage(&pDisplay);
			cvReleaseImage(&pDisplay2);
			cvReleaseImage(&pDisplay3);
			cvReleaseImage(&pDisplay4);
			cvReleaseImage(&pVarrianceThesholded);
#endif
		   double varRatio = 	((double) iTresholdedPixels)/(iHeight*(iWidth-iXStart));
		   if(varRatio > dRatioThreshold)
		   {
			    while(qLastFrames.size())
				{
				   cvReleaseImage(&qLastFrames.back());
				   qLastFrames.pop_back();
				}
				bResult = true;
		   }
	}	
	return	bResult;
}