// alibrationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EyeTracker.h"
#include "EyeTrackerDlg.h"
#include "ImageDlg.h"
#include "ObjectDetection.h"

#include "opencv2/highgui/highgui.hpp"

#define EYEDISPLAY_TIMER 2
#define DISPLAY_LEFT_EYE_WINDOW "Left"
#define DISPLAY_RIGHT_EYE_WINDOW "Right"
#define DISPLAY_IMG_WINDOW "Image"

// CalibrationDlg dialog

IMPLEMENT_DYNAMIC(CImageDlg, CDialog)

CImageDlg::CImageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImageDlg::IDD, pParent)
{

}

CImageDlg::~CImageDlg()
{
}

void CImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_COMBOMOV, m_cEyeMovCombo);
}


BEGIN_MESSAGE_MAP(CImageDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CImageDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_NEXT, &CImageDlg::OnBnClickedNext)
	ON_BN_CLICKED(IDC_PREV, &CImageDlg::OnBnClickedPrev)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CDF, &CImageDlg::OnClickedCdf)
	ON_BN_CLICKED(IDC_EDGE, &CImageDlg::OnClickedEdge)
	ON_BN_CLICKED(IDC_GPF, &CImageDlg::OnBnClickedGpf)
END_MESSAGE_MAP()


IplImage* CImageDlg::GetRightEyeImg(void)
{
	CEyeTrackerDlg * pEyeDlg = (CEyeTrackerDlg*) GetParent();
	return pEyeDlg->m_pCameraDlg->m_pRightEyeImg;
}

IplImage* CImageDlg::GetLeftEyeImg(void)
{
	CEyeTrackerDlg * pEyeDlg = (CEyeTrackerDlg*) GetParent();
	return pEyeDlg->m_pCameraDlg->m_pLeftEyeImg;
}

BOOL CImageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	{
	cvNamedWindow(DISPLAY_LEFT_EYE_WINDOW, CV_WINDOW_AUTOSIZE );
	HWND hWnd = (HWND) cvGetWindowHandle(DISPLAY_LEFT_EYE_WINDOW);
	HWND hParent = ::GetParent(hWnd);
	CWnd* pCameraWndParent = GetDlgItem(IDC_LEYEOUT);
	RECT cLeftEye;
	pCameraWndParent->GetClientRect(&cLeftEye);
	m_iLeftEyeWidth = cLeftEye.right;
	m_iLeftEyeHeight = cLeftEye.bottom;
	::SetParent(hWnd,pCameraWndParent->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);
	}

	{
	cvNamedWindow(DISPLAY_RIGHT_EYE_WINDOW, CV_WINDOW_AUTOSIZE );
	HWND hWnd = (HWND) cvGetWindowHandle(DISPLAY_RIGHT_EYE_WINDOW);
	HWND hParent = ::GetParent(hWnd);
	CWnd* pCameraWndParent = GetDlgItem(IDC_REYEOUT);
	RECT cRightEye;
	pCameraWndParent->GetClientRect(&cRightEye);
	m_iImgWidth = cRightEye.right;
	m_iImgHeight = cRightEye.bottom;
	::SetParent(hWnd,pCameraWndParent->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);
	}

	{
	cvNamedWindow(DISPLAY_IMG_WINDOW, CV_WINDOW_AUTOSIZE );
	HWND hWnd = (HWND) cvGetWindowHandle(DISPLAY_IMG_WINDOW);
	HWND hParent = ::GetParent(hWnd);
	CWnd* pCameraWndParent = GetDlgItem(IDC_IMG);
	RECT cRightEye;
	pCameraWndParent->GetClientRect(&cRightEye);
	m_iRightEyeWidth = cRightEye.right;
	m_iRightEyeHeight = cRightEye.bottom;
	::SetParent(hWnd,pCameraWndParent->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);
	}
	
	
	m_iCurrentImg = 0;
	m_iImgCount = 0;

	AnalyzeCurrentImg();
	
	typedef HRESULT (_stdcall *TSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
	HINSTANCE hDll = ::LoadLibrary(L"UxTheme.dll");
	if (hDll)
	{
		TSetWindowTheme setWindowTheme =
			(TSetWindowTheme) ::GetProcAddress(hDll, "SetWindowTheme");
		if (setWindowTheme) 
		{
			setWindowTheme(GetDlgItem(IDC_CDF)->m_hWnd, L"", L"");
			setWindowTheme(GetDlgItem(IDC_EDGE)->m_hWnd, L"", L"");
			setWindowTheme(GetDlgItem(IDC_GPF)->m_hWnd, L"", L"");
		}
			
			
		::FreeLibrary(hDll);
	}
	
	return TRUE;  
}





void CImageDlg::OnBnClickedButton1()
{
	CString strBuffer;
	CFileDialog fileDlg(TRUE,NULL,NULL,4|2|OFN_ALLOWMULTISELECT, L"Eye/Face Image(*.*)|*.*||",this);
	fileDlg.GetOFN().lpstrFile = strBuffer.GetBuffer(MAX_FILES*(_MAX_PATH + 1) + 1);
	fileDlg.GetOFN().nMaxFile = MAX_FILES*(_MAX_PATH + 1) + 1;;
	if(fileDlg.DoModal() == IDOK)
	{	
		m_lImages.clear();
		m_lImages.resize(0);
		m_iCurrentImg = 0;

		POSITION position = fileDlg.GetStartPosition();
		
		while(position)
		{
			CString strSelectedPath = fileDlg.GetNextPathName(position);
			CStringA ansiPath(strSelectedPath);
			
			IplImage* pImg = cvLoadImage(ansiPath);

			if(pImg)
			{
				m_lImages.push_back(strSelectedPath);
				cvReleaseImage(&pImg);
			}
		}
		m_iImgCount = m_lImages.size();

		CString strImgCount;
		if(m_iImgCount == 0)
		{					
			strImgCount = L"";
			SetDlgItemText(IDC_IMGCURRENT,L"");
		}
		else
			strImgCount.Format(L"%d",m_iImgCount);
		
		SetDlgItemText(IDC_IMGCOUNT,strImgCount);

		//strBuffer.ReleaseBuffer(0);
		AnalyzeCurrentImg();		
	}
	strBuffer.ReleaseBuffer(0);
}


void CImageDlg::AnalyzeCurrentImg()
{
		IplImage* pImg = NULL;
		IplImage* pLeftEye = NULL;
		IplImage* pRightEye = NULL;
	
		static int iDepth = 8;
		static int iChannels = 1;

		if(m_iImgCount != 0)
		{					
			CString strImgCurrent;
			strImgCurrent.Format(L"%d",m_iCurrentImg + 1);
			SetDlgItemText(IDC_IMGCURRENT,strImgCurrent);
			
			CString fileName = m_lImages[m_iCurrentImg];
			CStringA ansiFileName(fileName);
			pImg = cvLoadImage(ansiFileName);
			SetDlgItemText(IDC_FILENAME,fileName);
		}
		IplImage* pDisplayImg = NULL;
		IplImage*  pDisplayLeftEye = NULL;
		IplImage*  pDisplayRightEye = NULL;

		if(pImg)
		{
			iDepth = pImg->depth;
			iChannels = pImg->nChannels;
			
			pDisplayImg = cvCreateImage(cvSize(m_iImgWidth,m_iImgHeight),iDepth,iChannels);
			
			AnalyzeImage(pImg,&pLeftEye,&pRightEye);

			pDisplayLeftEye = cvCreateImage(cvSize(m_iLeftEyeWidth,m_iLeftEyeHeight),iDepth,iChannels);
			cvZero(pDisplayLeftEye);
			if(pLeftEye)
			{				
				cvResize(pLeftEye,pDisplayLeftEye);
				cvReleaseImage(&pLeftEye);
			}
		
			pDisplayRightEye = cvCreateImage(cvSize(m_iRightEyeWidth,m_iRightEyeHeight),iDepth,iChannels);
			cvZero(pDisplayRightEye);
			if(pRightEye)
			{	
			   
				cvResize(pRightEye,pDisplayRightEye);
				cvReleaseImage(&pRightEye);
			}
			
					
			cvResize(pImg,pDisplayImg);
			cvReleaseImage(&pImg);
		}
		
		
		
		cvShowImage(DISPLAY_RIGHT_EYE_WINDOW,pDisplayRightEye);
		cvReleaseImage(&pDisplayRightEye);

		cvShowImage(DISPLAY_LEFT_EYE_WINDOW,pDisplayLeftEye);
		cvReleaseImage(&pDisplayLeftEye);
		
		cvShowImage(DISPLAY_IMG_WINDOW,pDisplayImg);
		cvReleaseImage(&pDisplayImg);

	
		
}

void CImageDlg::OnBnClickedNext()
{
	++m_iCurrentImg %= m_iImgCount;
	AnalyzeCurrentImg();
	
}


void CImageDlg::OnBnClickedPrev()
{
	m_iCurrentImg = --m_iCurrentImg < 0 ? m_iImgCount - 1:m_iCurrentImg;
	
	AnalyzeCurrentImg();
	// TODO: Add your control notification handler code here
}



HBRUSH CImageDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_CDF:
			pDC->SetTextColor(RGB(255,0,0));
			break;
		case IDC_EDGE:
			pDC->SetTextColor(RGB(0,0,255));
			break;
		case IDC_GPF:
			pDC->SetTextColor(RGB(0,255,0));
			break;
	}

	return hbr;
}
void CImageDlg::AnalyzeImage(IplImage* pImg,IplImage** pLeftEye,IplImage** pRightEye)
 {
	 CvRect* pFace = CObjectDetection::DetectFace(pImg);
	 if(pFace)
	 {
		  CvRect cLeftEye;
		  cLeftEye.x = -1;
		  cLeftEye.y = -1;

		  CvRect cRightEye;
		  cRightEye.x = -1;
		  cRightEye.y = -1;


		  CObjectDetection::DetectEyes(pImg,pFace,&cLeftEye,&cRightEye);		 

		  if(cLeftEye.x != -1)
		  {
			  *pLeftEye = cvCreateImage(cvSize(cLeftEye.width,cLeftEye.height),pImg->depth,pImg->nChannels);

			cvSetImageROI(pImg,
						cvRect(cLeftEye.x,cLeftEye.y,cLeftEye.width,cLeftEye.height));
			cvCopy( pImg, *pLeftEye, NULL );	
			cvResetImageROI(pImg);
		  }
		  if(cRightEye.x != -1)
		  {
			 *pRightEye = cvCreateImage(cvSize(cRightEye.width,cRightEye.height),pImg->depth,pImg->nChannels);

			 cvSetImageROI(pImg,
						cvRect(cRightEye.x,cRightEye.y,cRightEye.width,cRightEye.height));
			 cvCopy( pImg, *pRightEye, NULL );	
			cvResetImageROI(pImg);
		  }
		

		  if(*pLeftEye)
		  {	
			  CvPoint cPupilCDF;
			  CvPoint cPupilEdge;
			  CvPoint cPupilGPF;
			  cPupilGPF.x = -1;
			  cPupilGPF.y = -1;

			  
			  if(IsDlgButtonChecked(IDC_CDF))
			  {
				  cPupilCDF = CObjectDetection::DetectPupilCDF(*pLeftEye); 
					  
			  }	
			   if(IsDlgButtonChecked(IDC_EDGE))
			  {	 
				  cPupilEdge = CObjectDetection::DetectPupilEdge(*pLeftEye);
			  }	
			   if(IsDlgButtonChecked(IDC_GPF))
			  {
				  cPupilGPF = CObjectDetection::DetectPupilGPF(*pLeftEye);
			  }

			  if(IsDlgButtonChecked(IDC_CDF))
			  {
				  cvDrawCircle(*pLeftEye,cvPoint(cPupilCDF.x,cPupilCDF.y),1,CV_RGB(255,0,0),-1,4,0);
				  #ifdef DEBUG
					IplImage* pDrawnPupil = cvCreateImage(cvGetSize(*pLeftEye), 8, 1);	
					cvCvtColor(*pLeftEye, pDrawnPupil, CV_BGR2GRAY );
					cvSaveImage("left_eye_cdf.jpg",pDrawnPupil);
					cvReleaseImage(&pDrawnPupil);
				 #endif DEBUG
			  
			  
			  }	
			  if(IsDlgButtonChecked(IDC_EDGE))
			  {
				  cvDrawCircle(*pLeftEye,cvPoint(cPupilEdge.x,cPupilEdge.y),1,CV_RGB(0,0,255),-1,4,0);		 
				  #ifdef DEBUG		
					IplImage* pDrawnPupil = cvCreateImage(cvGetSize(*pLeftEye), 8, 1);	
					cvCvtColor(*pLeftEye, pDrawnPupil, CV_BGR2GRAY );
					cvDrawLine(pDrawnPupil,cvPoint(0,cPupilEdge.y),cvPoint((*pLeftEye)->width,cPupilEdge.y),CV_RGB(255,0,0),1,4,0);
					cvDrawLine(pDrawnPupil,cvPoint(cPupilEdge.x,0),cvPoint(cPupilEdge.x,(*pLeftEye)->height),CV_RGB(255,0,0),1,4,0);
					cvSaveImage("right_eye_edge.jpg",pDrawnPupil);
					cvReleaseImage(&pDrawnPupil);
				  #endif DEBUG		
			  }	
			  if(IsDlgButtonChecked(IDC_GPF))
			  {
			      cvDrawCircle(*pLeftEye,cvPoint(cPupilGPF.x,cPupilGPF.y),1,CV_RGB(0,255,0),-1,4,0);
				  #ifdef DEBUG		
					IplImage* pDrawnPupil = cvCreateImage(cvGetSize(*pLeftEye), 8, 1);	
					cvCvtColor(*pLeftEye, pDrawnPupil, CV_BGR2GRAY );
					cvDrawLine(pDrawnPupil,cvPoint(0,cPupilGPF.y),cvPoint((*pLeftEye)->width,cPupilGPF.y),CV_RGB(255,0,0),1,4,0);
					cvDrawLine(pDrawnPupil,cvPoint(cPupilGPF.x,0),cvPoint(cPupilGPF.x,(*pLeftEye)->height),CV_RGB(255,0,0),1,4,0);
					cvSaveImage("left_eye_gpf.jpg",pDrawnPupil);
					cvReleaseImage(&pDrawnPupil);
				 #endif DEBUG	
			  }	

		  }	
		   

		  if(*pRightEye)
		  {	
			  CvPoint cPupilCDF;
			  CvPoint cPupilEdge;
			  CvPoint cPupilGPF;
			  
			  cPupilGPF.x = -1;
			  cPupilGPF.y = -1;

			  cPupilEdge.x = -1;
			  cPupilEdge.y = -1;

			  if(IsDlgButtonChecked(IDC_CDF))
			  {
				  cPupilCDF = CObjectDetection::DetectPupilCDF(*pRightEye);
			  }	
			  if(IsDlgButtonChecked(IDC_EDGE))
			  {
				  cPupilEdge = CObjectDetection::DetectPupilEdge(*pRightEye);
			  }
			  if(IsDlgButtonChecked(IDC_GPF))
			  {
				  cPupilGPF = CObjectDetection::DetectPupilGPF(*pRightEye);
			  }
			  if(IsDlgButtonChecked(IDC_CDF))
			  {
				  cvDrawCircle(*pRightEye,cvPoint(cPupilCDF.x,cPupilCDF.y),1,CV_RGB(255,0,0),-1,4,0);
				  #ifdef DEBUG		
					IplImage* pDrawnPupil = cvCreateImage(cvGetSize(*pRightEye), 8, 1);	
					cvCvtColor(*pRightEye, pDrawnPupil, CV_BGR2GRAY );
					cvDrawLine(pDrawnPupil,cvPoint(0,cPupilCDF.y),cvPoint((*pRightEye)->width,cPupilCDF.y),CV_RGB(255,0,0),1,4,0);
					cvDrawLine(pDrawnPupil,cvPoint(cPupilCDF.x,0),cvPoint(cPupilCDF.x,(*pRightEye)->height),CV_RGB(255,0,0),1,4,0);
					cvSaveImage("right_eye_cdf.jpg",pDrawnPupil);
					cvReleaseImage(&pDrawnPupil);
				 #endif DEBUG	
			  
			  }	
			  if(IsDlgButtonChecked(IDC_EDGE))
			  {
			     cvDrawCircle(*pRightEye,cvPoint(cPupilEdge.x,cPupilEdge.y),1,CV_RGB(0,0,255),-1,4,0);
			  }	
			  if(IsDlgButtonChecked(IDC_GPF))
			  {
			     cvDrawCircle(*pRightEye,cvPoint(cPupilGPF.x,cPupilGPF.y),1,CV_RGB(0,255,0),-1,4,0);
			  }	
		  }	

			#ifdef DEBUG
				auto pFaceDrawnImg = cvCreateImage(cvGetSize(pImg), pImg->depth, pImg->nChannels );
				cvCopy(pImg,pFaceDrawnImg);
				cvRectangle(pFaceDrawnImg,cvPoint(pFace->x,pFace->y),cvPoint(pFace->x + pFace->width,pFace->y + pFace->height),CV_RGB(0,0,0),1,0,0);
				cvSaveImage("face.jpg",pFaceDrawnImg);
				cvReleaseImage(&pFaceDrawnImg);
			#endif DEBUG
	 }
	 else
	 {
		  *pLeftEye= cvCreateImage(cvGetSize(pImg),pImg->depth,pImg->nChannels);
		  cvSetImageROI(pImg,cvRect(0,0,pImg->width,pImg->height));
		  cvCopy(pImg, *pLeftEye,NULL);
	      cvResetImageROI(pImg);

		  CvPoint cPupilCDF;
		  CvPoint cPupilEdge;
		  CvPoint cPupilGPF;
	  
		  if(IsDlgButtonChecked(IDC_CDF))
		  {
				  cPupilCDF = CObjectDetection::DetectPupilCDF(*pLeftEye);
		  }	
		  if(IsDlgButtonChecked(IDC_EDGE))
		  {
				  cPupilEdge = CObjectDetection::DetectPupilEdge(*pLeftEye);
		  }
			  
		  if(IsDlgButtonChecked(IDC_GPF))
		  {
				  cPupilGPF = CObjectDetection::DetectPupilGPF(*pLeftEye);
		  }
		  
		  if(IsDlgButtonChecked(IDC_CDF))
		  {
				  cvDrawCircle(*pLeftEye,cvPoint(cPupilCDF.x,cPupilCDF.y),1,CV_RGB(255,0,0),-1,4,0);
		  }	
		
		  if(IsDlgButtonChecked(IDC_EDGE))
		  {
			      cvDrawCircle(*pLeftEye,cvPoint(cPupilEdge.x,cPupilEdge.y),1,CV_RGB(0,0,255),-1,4,0);
		  }	
			  if(IsDlgButtonChecked(IDC_GPF))
		  {
			     cvDrawCircle(*pLeftEye,cvPoint(cPupilGPF.x,cPupilGPF.y),1,CV_RGB(0,255,0),-1,4,0);
		  }	
	 }
 }


void CImageDlg::OnClickedCdf()
{
	AnalyzeCurrentImg();
}


void CImageDlg::OnClickedEdge()
{
	AnalyzeCurrentImg();
}


void CImageDlg::OnBnClickedGpf()
{
	AnalyzeCurrentImg();
}
