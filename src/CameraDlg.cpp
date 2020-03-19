// CameraDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EyeTracker.h"
//
#include "CameraDlg.h"
#include "EyeTrackerDlg.h"
#include "ObjectDetection.h"


#define DISPLAY_TIMER 1
#define MOUSE_SENSIVITY 20

#define DBLCLK_FRAME_LIMIT 15

IMPLEMENT_DYNAMIC(CCameraDlg, CDialog)

using namespace cv;

CCameraDlg::CCameraDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCameraDlg::IDD, pParent)
	, m_bIsTmpSet(false)
{

}

CCameraDlg::~CCameraDlg()
{
}

void CCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCameraDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTRACK, &CCameraDlg::OnClickedBtrack)
	ON_BN_CLICKED(IDC_BMOUSECTRL, &CCameraDlg::OnClickedBmousectrl)
	ON_BN_CLICKED(IDC_BTMP, &CCameraDlg::OnBnClickedBtmp)
END_MESSAGE_MAP()





BOOL CCameraDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_pCapture = NULL;
	m_pEyeTrackerDlg = (CEyeTrackerDlg*) GetParent();
	
	{
	cvNamedWindow(DISPLAY_WINDOW, CV_WINDOW_AUTOSIZE );
	HWND hWnd = (HWND) cvGetWindowHandle(DISPLAY_WINDOW);
	HWND hParent = ::GetParent(hWnd);
	CWnd* pCameraWndParent = GetDlgItem(IDC_STATICCAMERA);
	::SetParent(hWnd,pCameraWndParent->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);
	CRect cCameraRect;
	pCameraWndParent->GetClientRect(&cCameraRect);
	m_iWndWidth = cCameraRect.Width();
	m_iWndHeight = cCameraRect.Height();
	}

	
	{
	cvNamedWindow(EYEL_WINDOW, CV_WINDOW_AUTOSIZE );
	HWND hWnd = (HWND) cvGetWindowHandle(EYEL_WINDOW);
	HWND hParent = ::GetParent(hWnd);
	CWnd* pCameraWndParent = GetDlgItem(IDC_LEYEDISPL);
	::SetParent(hWnd,pCameraWndParent->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);
	CRect cCameraRect;
	pCameraWndParent->GetClientRect(&cCameraRect);
	m_iLeftEyeWndWidth= cCameraRect.Width();
	m_iLeftEyeWndHeight = cCameraRect.Height();
	}
	
	{
	cvNamedWindow(EYER_WINDOW, CV_WINDOW_AUTOSIZE );
	HWND hWnd = (HWND) cvGetWindowHandle(EYER_WINDOW);
	HWND hParent = ::GetParent(hWnd);
	CWnd* pCameraWndParent = GetDlgItem(IDC_REYEDISPL);
	::SetParent(hWnd,pCameraWndParent->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);
	CRect cCameraRect;
	pCameraWndParent->GetClientRect(&cCameraRect);
	m_iRightEyeWndWidth= cCameraRect.Width();
	m_iRightEyeWndHeight = cCameraRect.Height();
	}
	
	m_bIsMouseControl = FALSE;
	m_bIsTracking = FALSE;
	
	m_pRightEyeImg = NULL;
	m_pLeftEyeImg = NULL;

	m_bIsTmpSet = FALSE;

	CheckDlgButton(IDC_CHSHOWFACE,1);
	CheckDlgButton(IDC_CSHOWEYES,1);
	CheckDlgButton(IDC_CSHOWPUPIL,1);
	CheckDlgButton(IDC_CSHOWTMP,1);
	CheckDlgButton(IDC_CSHOWMID,1);
	
	m_cLastMidPoint.x = -1;
	m_cLastMidPoint.y = -1;


	return TRUE;

}

void CCameraDlg::PostNcDestroy()
{
	cvReleaseCapture(&m_pCapture);
	cvDestroyAllWindows();
	CDialog::PostNcDestroy();
}

void CCameraDlg::OnTimer(UINT_PTR nIDEvent)
{	
	static int iLefEyeFrameCount = 0;
	static int iRightEyeFrameCount = 0;

	static unsigned int iFaceFrameCount = 0;
	static int iAvgFaceX = 0;
	static int iAvgFaceY = 0;
	static int iAvgFaceWidth = 0;
	static int iAvgFaceHeight = 0;

	static int iAvgLeftEyeX = 0;
	static int iAvgLeftEyeY = 0;
	static int iAvgLeftEyeWidth = 0;
	static int iAvgLeftEyeHeight = 0;

	static int iAvgRightEyeX = 0;
	static int iAvgRightEyeY = 0;
	static int iAvgRightEyeWidth = 0;
	static int iAvgRightEyeHeight = 0;

	static bool isMove = FALSE;

	static bool bLeftEyeClosed = FALSE;
	static bool bRightEyeClosed = FALSE;

	CvPoint cPupilLeft;
	CvPoint cPupilRight;

	cPupilLeft.x = -1;
	cPupilLeft.y = -1;

	cPupilRight.x = -1;
	cPupilRight.y = -1;

	

	if(m_pCapture) 
	{
		m_pCurrentFrame = cvCloneImage(cvQueryFrame(m_pCapture));
		BOOL bLeftEyeBlink = FALSE;
		BOOL bRightEyeBlink = FALSE;
		if(m_pCurrentFrame)
		{	
			CvRect* pFace = CObjectDetection::DetectFace(m_pCurrentFrame);
			double dWidthRatio =  ((double) m_iWndWidth)/m_pCurrentFrame->width;
			double dHeightRatio =  ((double) m_iWndHeight)/m_pCurrentFrame->height;
			auto pDisplay = cvCreateImage( cvSize( m_iWndWidth, m_iWndHeight ),
							m_pCurrentFrame->depth, m_pCurrentFrame->nChannels );
			cvResize(m_pCurrentFrame,pDisplay);
			
			isMove = FALSE;


			if(pFace)
			{		
			
				++iFaceFrameCount;

				iAvgFaceX += pFace->x;
				iAvgFaceY += pFace->y;
				iAvgFaceWidth += pFace->width;
				iAvgFaceHeight += pFace->height;
				
				if(iFaceFrameCount%m_iAvgFaceFps)
				{
					cvReleaseImage(&m_pCurrentFrame);
					cvReleaseImage(&pDisplay);
					CObjectDetection::Clear();	
					return;
				}
				
				pFace->x = iAvgFaceX/m_iAvgFaceFps;
				pFace->y = iAvgFaceY/m_iAvgFaceFps;
				pFace->height =iAvgFaceHeight/m_iAvgFaceFps;
				pFace->width =iAvgFaceWidth/m_iAvgFaceFps;


				iAvgFaceX = 0;
				iAvgFaceY = 0;
				iAvgFaceWidth = 0;
				iAvgFaceHeight = 0;
				iFaceFrameCount = 0;
	
				CString strLog;
				strLog.Format(L"Detected Face (x=%d y=%d width=%d height=%d)",pFace->x,pFace->y,pFace->width,pFace->height);
				Log(strLog);
				
				iFaceFrameCount = 0;

				CvRect cEyeLeft = cvRect(0,0,0,0);
				CvRect cEyeRight = cvRect(0,0,0,0);
				
				bool bAvgLeft = TRUE;
				bool bAvgRight = TRUE;
				
				for(int i = 0;i<m_iAvgEyeFps;++i)
				{
					if(m_pCapture)
					{
						
				
						IplImage*	pCurrentFrame = cvCloneImage(cvQueryFrame(m_pCapture));
					
						CObjectDetection::DetectEyes(pCurrentFrame,pFace,&cEyeLeft,&cEyeRight);
	
						cvReleaseImage(&pCurrentFrame);
					
						if( cEyeLeft.width == 0 &&  cEyeLeft.height == 0)
									bAvgLeft = FALSE;
						if( cEyeRight.width == 0 &&  cEyeRight.height == 0)
									bAvgRight = FALSE;

						iAvgLeftEyeX += cEyeLeft.x;
						iAvgLeftEyeY += cEyeLeft.y;
						iAvgLeftEyeWidth += cEyeLeft.width;
						iAvgLeftEyeHeight += cEyeLeft.height;

						iAvgRightEyeX += cEyeRight.x;
						iAvgRightEyeY += cEyeRight.y;
						iAvgRightEyeWidth += cEyeRight.width;
						iAvgRightEyeHeight += cEyeRight.height;


					}
				}
				
				cEyeLeft.x=iAvgLeftEyeX/m_iAvgEyeFps;
				cEyeLeft.y=iAvgLeftEyeY/m_iAvgEyeFps;
				cEyeLeft.width = iAvgLeftEyeWidth/m_iAvgEyeFps ;
				cEyeLeft.height=iAvgLeftEyeHeight/m_iAvgEyeFps;

				cEyeRight.x=iAvgRightEyeX/m_iAvgEyeFps;
				cEyeRight.y=iAvgRightEyeY/m_iAvgEyeFps;
				cEyeRight.width = iAvgRightEyeWidth/m_iAvgEyeFps ;
				cEyeRight.height=iAvgRightEyeHeight/m_iAvgEyeFps;


				iAvgLeftEyeX = 0;
				iAvgLeftEyeY = 0;
				iAvgLeftEyeWidth = 0;
				iAvgLeftEyeHeight = 0;

				iAvgRightEyeX = 0;
				iAvgRightEyeY = 0;
				iAvgRightEyeWidth = 0;
				iAvgRightEyeHeight = 0;

						
				if(bAvgLeft)
				 {

					cvReleaseImage(&m_pLeftEyeImg);
					m_pLeftEyeImg = cvCreateImage( cvSize( cEyeLeft.width, cEyeLeft.height),m_pCurrentFrame->depth,m_pCurrentFrame->nChannels);
					cvSetImageROI(m_pCurrentFrame,
						cvRect(cEyeLeft.x,cEyeLeft.y,cEyeLeft.width,cEyeLeft.height));
					cvCopy( m_pCurrentFrame, m_pLeftEyeImg, NULL );	
					cvResetImageROI(m_pCurrentFrame);
					
					bLeftEyeBlink = CObjectDetection::DetectLeftBlink(m_pLeftEyeImg,m_iLastFramesNum,m_iVarrianceBlink,m_dRatioThreshold);
				
					CvPoint cPupilCenter;
					
					switch(m_iSelectedAlg)
					{
						case 0:
							cPupilCenter =  CObjectDetection::DetectPupilCDF(m_pLeftEyeImg);
							break;
						case 1:
							cPupilCenter =  CObjectDetection::DetectPupilEdge(m_pLeftEyeImg);
							break;
						case 2:
							cPupilCenter =  CObjectDetection::DetectPupilGPF(m_pLeftEyeImg);
							break;
					}
					
					cPupilLeft.x = cPupilCenter.x + cEyeLeft.x;
					cPupilLeft.y = cPupilCenter.y + cEyeLeft.y;


					double dEyeWidthRatio = ((double)m_iLeftEyeWndWidth)/m_pLeftEyeImg->width;
					double dEyeHeightRatio = ((double)m_iLeftEyeWndHeight)/m_pLeftEyeImg->height;
					IplImage* pLeftEyeDisplay = cvCreateImage(cvSize(m_iLeftEyeWndWidth,m_iLeftEyeWndHeight),m_pCurrentFrame->depth,m_pCurrentFrame->nChannels);
					cvResize(m_pLeftEyeImg,pLeftEyeDisplay);
					if(IsDlgButtonChecked(IDC_CSHOWPUPIL))	{
						cvDrawCircle(pLeftEyeDisplay,cvPoint((int)(cPupilCenter.x*dEyeWidthRatio),(int)(cPupilCenter.y*dEyeHeightRatio)),4,CV_RGB(250,250,210),-1,4,0);
						cvDrawCircle(pDisplay,cvPoint((int)(cPupilLeft.x*dWidthRatio),(int) (cPupilLeft.y*dHeightRatio)),3,CV_RGB(250,250,210),-1,4,0);
						CString strLog;
						strLog.Format(L"Detected Left Pupil (x=%d y=%d)",cPupilLeft.x,cPupilLeft.y);
						Log(strLog);
					}
					
					cvShowImage(EYEL_WINDOW,pLeftEyeDisplay);
						
					CString strLog;
					strLog.Format(L"Detected Left Eye (x=%d y=%d width=%d height=%d)",cEyeLeft.x,cEyeLeft.y,cEyeLeft.width,cEyeLeft.height);
					Log(strLog);			

					
					cvReleaseImage(&pLeftEyeDisplay);

				 }
				

				if(bAvgRight)
				{
					cvReleaseImage(&m_pRightEyeImg);
					m_pRightEyeImg = cvCreateImage( cvSize( cEyeRight.width, cEyeRight.height),m_pCurrentFrame->depth,m_pCurrentFrame->nChannels);

					cvSetImageROI(m_pCurrentFrame,
						cvRect(cEyeRight.x,cEyeRight.y,cEyeRight.width,cEyeRight.height));
					cvCopy( m_pCurrentFrame, m_pRightEyeImg, NULL );	
					cvResetImageROI(m_pCurrentFrame);
					
					CString strLog;
					strLog.Format(L"Detected Right Eye (x=%d y=%d width=%d height=%d)",cEyeRight.x,cEyeRight.y,cEyeRight.width,cEyeRight.height);
					Log(strLog);
				

					bRightEyeBlink =  CObjectDetection::DetectRightBlink(m_pRightEyeImg,m_iLastFramesNum,m_iVarrianceBlink,m_dRatioThreshold2);		
					
					CvPoint cPupilCenter;
					
					switch(m_iSelectedAlg)
					{
					case 0:
							cPupilCenter =  CObjectDetection::DetectPupilCDF(m_pRightEyeImg);
							break;
					case 1:
							cPupilCenter =  CObjectDetection::DetectPupilEdge(m_pRightEyeImg);
							break;
					case 2:
							cPupilCenter =  CObjectDetection::DetectPupilGPF(m_pRightEyeImg);
							break;
					}
					cPupilRight.x = cPupilCenter.x + cEyeRight.x;
					cPupilRight.y = cPupilCenter.y + cEyeRight.y;

					IplImage* pRightEyeDisplay = cvCreateImage(cvSize(m_iRightEyeWndWidth,m_iRightEyeWndHeight),m_pCurrentFrame->depth,m_pCurrentFrame->nChannels);
					cvResize(m_pRightEyeImg,pRightEyeDisplay);

					double dEyeWidthRatio = ((double)m_iRightEyeWndWidth)/m_pRightEyeImg->width;
					double dEyeHeightRatio = ((double)m_iRightEyeWndHeight)/m_pRightEyeImg->height;

					if(IsDlgButtonChecked(IDC_CSHOWPUPIL))
					{
							cvDrawCircle(pRightEyeDisplay,cvPoint((int)( cPupilCenter.x*dEyeWidthRatio),(int) (cPupilCenter.y*dEyeHeightRatio)),4,CV_RGB(250,250,210),-1,4,0);
							cvDrawCircle(pDisplay,cvPoint((int)(cPupilRight.x*dWidthRatio),(int) (cPupilRight.y*dHeightRatio)),3,CV_RGB(250,250,210),-1,4,0);
							CString strLog;
							strLog.Format(L"Detected Right Pupil (x=%d y=%d)",cPupilRight.x,cPupilRight.y);
							Log(strLog);	
					}
					
					cvShowImage(EYER_WINDOW,pRightEyeDisplay);
					cvReleaseImage(&pRightEyeDisplay);
				
			
					if(IsDlgButtonChecked(IDC_CSHOWEYES))
					{
						cEyeRight.x=(int) (dWidthRatio*cEyeRight.x);
						cEyeRight.y=(int)( dHeightRatio*cEyeRight.y);
						cEyeRight.width=(int)(cEyeRight.width*dWidthRatio);   
						cEyeRight.height=(int)(cEyeRight.height*dHeightRatio);  
						
						cvRectangle( pDisplay,cvPoint(cEyeRight.x,cEyeRight.y),cvPoint(cEyeRight.x + cEyeRight.width,
						cEyeRight.y + cEyeRight.height ),
						 CV_RGB(0,255,0), 1, 0, 0 ); 
					}
				}
				
				if(bAvgLeft)
				 if(IsDlgButtonChecked(IDC_CSHOWEYES))
				 {
					cEyeLeft.x=(int) (dWidthRatio*cEyeLeft.x);
					cEyeLeft.y=(int)( dHeightRatio*cEyeLeft.y);
					cEyeLeft.width=(int)(cEyeLeft.width*dWidthRatio);   
					cEyeLeft.height=(int)(cEyeLeft.height*dHeightRatio);
					 cvRectangle( pDisplay,cvPoint(cEyeLeft.x,cEyeLeft.y),cvPoint(cEyeLeft.x + cEyeLeft.width,
						cEyeLeft.y + cEyeLeft.height ),
						cvScalar( 0,255,0, 0 ), 1, 0, 0 );
				 }


				if(m_bIsTmpSet && IsDlgButtonChecked(IDC_CSHOWTMP))
						cvRectangle( pDisplay,cvPoint((int)((m_cTemplateMidPoint.x - m_iTemplateWidth/2)*dWidthRatio),(int)((m_cTemplateMidPoint.y - m_iTemplateHeight/2)*dHeightRatio)),
						cvPoint((int)((m_cTemplateMidPoint.x + m_iTemplateWidth/2)*dWidthRatio),(int) ((m_cTemplateMidPoint.y + m_iTemplateHeight/2)*dHeightRatio)),CV_RGB(0,0,255),2,0,0);			

				if(cPupilRight.x != -1 && cPupilLeft.x != -1)
				{
					m_cCurrentMidPoint.x = (cPupilRight.x + cPupilLeft.x)/2;
					m_cCurrentMidPoint.y = (cPupilRight.y + cPupilLeft.y)/2;

					if(IsDlgButtonChecked(IDC_CSHOWMID))
					{
						
						CvPoint cDisplayMid;
						cDisplayMid.x =(int)( m_cCurrentMidPoint.x*dWidthRatio);
						cDisplayMid.y =(int)( m_cCurrentMidPoint.y*dHeightRatio);
						
						CvPoint cDisplayLeft;
						cDisplayLeft.x =(int)( cPupilLeft.x*dWidthRatio);
						cDisplayLeft.y =(int)( cPupilLeft.y*dHeightRatio);
						CvPoint cDisplayRight;
						cDisplayRight.x =(int)( cPupilRight.x*dWidthRatio);
						cDisplayRight.y =(int)( cPupilRight.y*dHeightRatio);

						
						cvDrawLine(pDisplay,cDisplayRight,cDisplayLeft,CV_RGB(0,255,0),1,8,0);
						cvDrawCircle(pDisplay,cDisplayMid,3,CV_RGB(255,255,255),-1,4,0);
					}
					if(!m_bIsTmpSet && IsDlgButtonChecked(IDC_CSHOWTMP))
						cvRectangle( pDisplay,cvPoint((int)((m_cCurrentMidPoint.x - m_iTemplateWidth/2)*dWidthRatio),(int)((m_cCurrentMidPoint.y - m_iTemplateHeight/2)*dHeightRatio)),
						cvPoint((int)((m_cCurrentMidPoint.x + m_iTemplateWidth/2)*dWidthRatio),(int) ((m_cCurrentMidPoint.y + m_iTemplateHeight/2)*dHeightRatio)),CV_RGB(0,0,255),2,0,0);			
				
				
				if(m_bIsMouseControl)
				{
					
					if(m_bIsTmpSet)
					{
					
						if( m_cCurrentMidPoint.x -m_cTemplateMidPoint.x > m_iTemplateWidth/2)
						{
								int iMove = (int) -sqrt(double (m_cCurrentMidPoint.x -m_cTemplateMidPoint.x))*m_iAccH;
								MoveCursor(iMove,0);
								isMove = TRUE;
								CString strLog;
								strLog.Format(L"Detected Left Movement");
								Log(strLog);
						}
					    if(m_cTemplateMidPoint.x - m_cCurrentMidPoint.x > m_iTemplateWidth/2)
						{
								int iMove = (int)  sqrt(double (m_cTemplateMidPoint.x -m_cCurrentMidPoint.x))*m_iAccH;
								MoveCursor(iMove,0);
								isMove = TRUE;
								CString strLog;
								strLog.Format(L"Detected Right Movement");
								Log(strLog);
						}

						if(m_cTemplateMidPoint.y - m_cCurrentMidPoint.y > m_iTemplateHeight/2)
						{
								int iMove =  - (int) sqrt(double (m_cTemplateMidPoint.y -m_cCurrentMidPoint.y))*m_iAccV;
								MoveCursor(0,iMove);
								isMove = TRUE;
								CString strLog;
								strLog.Format(L"Detected Up Movement");
								Log(strLog);
						}

						if( m_cCurrentMidPoint.y -m_cTemplateMidPoint.y > m_iTemplateHeight/2)
						{
								int iMove =  (int) sqrt(double (m_cCurrentMidPoint.y -m_cTemplateMidPoint.y))*m_iAccV;
								MoveCursor(0,iMove);
								isMove = TRUE;
								CString strLog;
								strLog.Format(L"Detected Down Movement");
								Log(strLog);
						}

					}
					
					if(isMove)
					{
						ResetEyeBlinks();
					}
				}
				
				}
				if(!isMove && m_bIsMouseControl)
					{
							if( bLeftEyeBlink && !bRightEyeBlink && m_bSupportClicking)
							{
									PressLeftButton();
									ReleaseLeftButton();
									Log(L"Detected Left Eye blink");
									isMove = TRUE;
							}
							if(bRightEyeBlink && !bLeftEyeBlink && m_bSupportDoubleClick)
							{
									PressLeftButton();
									ReleaseLeftButton();
									PressLeftButton();
									ReleaseLeftButton();
									Log(L"Detected Right Eye Blink");
									isMove = TRUE;
							}
					}

				if(IsDlgButtonChecked(IDC_CHSHOWFACE))
				{
					pFace->x=(int) (dWidthRatio*pFace->x);
					pFace->y=(int)( dHeightRatio*pFace->y);
					pFace->width=(int)(pFace->width*dWidthRatio);   
					pFace->height=(int)(pFace->height*dHeightRatio);   

					cvRectangle( pDisplay,cvPoint(pFace->x,pFace->y),cvPoint(pFace->x + pFace->width,
					 pFace->y + pFace->height ),
                    cvScalar( 0, 0,255, 0 ), 1, 0, 0 );	
				}
			}
			
		
			cvShowImage(DISPLAY_WINDOW,pDisplay);
			cvReleaseImage(&pDisplay);
			CObjectDetection::Clear();	
			int iNewTick = GetTickCount();
			if(m_iTickCount)
			{
				int iTickDiff = iNewTick-m_iTickCount; 
				int iFPS =(int)(   1./(iTickDiff/1000.));
				SetDlgItemInt(IDC_FPS,iFPS);
				
			}
			m_iTickCount = iNewTick;
		}
	
		cvReleaseImage(&m_pCurrentFrame);
	}
	CDialog::OnTimer(nIDEvent);
}


void CCameraDlg::MoveCursor(int iXOffset, int iYOffset)
{
	POINT point;
	GetCursorPos(&point);
	SetCursorPos(point.x + iXOffset,point.y + iYOffset);
}
void CCameraDlg::PressLeftButton()
{
	POINT point;
	GetCursorPos(&point);

	INPUT cInput;
	MOUSEINPUT cMouseInput;
	cMouseInput.dx = point.x;
	cMouseInput.dy = point.y;
	cMouseInput.time = 0;
	cMouseInput.mouseData = 0;
	cMouseInput.dwFlags = MOUSEEVENTF_LEFTDOWN;
	cMouseInput.dwExtraInfo = 0;

	cInput.type = INPUT_MOUSE;
	cInput.mi = cMouseInput;

	::SendInput(1,&cInput,sizeof(cInput));

}
void CCameraDlg::PressRightButton()
{
	POINT point;
	GetCursorPos(&point);

	INPUT cInput;
	MOUSEINPUT cMouseInput;
	cMouseInput.dx = point.x;
	cMouseInput.dy = point.y;
	cMouseInput.time = 0;
	cMouseInput.mouseData = 0;
	cMouseInput.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	cMouseInput.dwExtraInfo = 0;

	cInput.type = INPUT_MOUSE;
	cInput.mi = cMouseInput;

	::SendInput(1,&cInput,sizeof(cInput));

}
void CCameraDlg::ReleaseLeftButton()
{
	POINT point;
	GetCursorPos(&point);

	INPUT cInput;
	MOUSEINPUT cMouseInput;
	cMouseInput.dx = point.x;
	cMouseInput.dy = point.y;
	cMouseInput.time = 0;
	cMouseInput.mouseData = 0;
	cMouseInput.dwFlags = MOUSEEVENTF_LEFTUP;
	cMouseInput.dwExtraInfo = 0;

	cInput.type = INPUT_MOUSE;
	cInput.mi = cMouseInput;

	::SendInput(1,&cInput,sizeof(cInput));

}

void CCameraDlg::ReleaseRightButton()
{
	POINT point;
	GetCursorPos(&point);

	INPUT cInput;
	MOUSEINPUT cMouseInput;
	cMouseInput.dx = point.x;
	cMouseInput.dy = point.y;
	cMouseInput.time = 0;
	cMouseInput.mouseData = 0;
	cMouseInput.dwFlags = MOUSEEVENTF_RIGHTUP;
	cMouseInput.dwExtraInfo = 0;

	cInput.type = INPUT_MOUSE;
	cInput.mi = cMouseInput;

	::SendInput(1,&cInput,sizeof(cInput));

}


void CCameraDlg::OnClickedBtrack()
{
		if(!m_bIsTracking)
		{
			int iCameraIndex = m_pEyeTrackerDlg->m_pSettingDlg->m_iSelectedCamera;
			m_pCapture = cvCaptureFromCAM(iCameraIndex);	
	
			if(m_pCapture) 
			{
				cvSetCaptureProperty(m_pCapture, CV_CAP_PROP_FRAME_WIDTH,  m_pEyeTrackerDlg->m_pSettingDlg->m_iFrameWidth);
				cvSetCaptureProperty(m_pCapture, CV_CAP_PROP_FRAME_HEIGHT,m_pEyeTrackerDlg->m_pSettingDlg->m_iFrameHeight);
				m_bIsTracking = TRUE;
				::EnableWindow(GetDlgItem(IDC_BTMP)->m_hWnd,TRUE);
				m_iSelectedAlg =  m_pEyeTrackerDlg->m_pSettingDlg->m_iSelectedAlg;
				m_iTemplateHeight = m_pEyeTrackerDlg->m_pSettingDlg->m_iTmpHeight;
				m_iTemplateWidth = m_pEyeTrackerDlg->m_pSettingDlg->m_iTmpWidth;
				m_iAvgFaceFps = m_pEyeTrackerDlg->m_pSettingDlg->m_iAvgFaceFps;
				m_iAvgEyeFps=m_pEyeTrackerDlg->m_pSettingDlg->m_iAvgEyeFps;
				m_iAccH = m_pEyeTrackerDlg->m_pSettingDlg->m_iAccH;
				m_iAccV = m_pEyeTrackerDlg->m_pSettingDlg->m_iAccV;
				m_bSupportClicking =m_pEyeTrackerDlg->m_pSettingDlg->m_bSupportClicking;
				m_bSupportDoubleClick = m_pEyeTrackerDlg->m_pSettingDlg->m_bSupportDoubleClick;
				m_iVarrianceBlink =  m_pEyeTrackerDlg->m_pSettingDlg->m_iThresholdClick;
				m_iLastFramesNum =  m_pEyeTrackerDlg->m_pSettingDlg->m_iFrameNumClick;
				m_dRatioThreshold =  m_pEyeTrackerDlg->m_pSettingDlg->m_fVarrianceRatio;
				m_dRatioThreshold2 =  m_pEyeTrackerDlg->m_pSettingDlg->m_fVarrianceRatio2;
				m_iTickCount = 0;
				SetDlgItemText(IDC_BTRACK,L"Stop Tracking");
				SetTimer(DISPLAY_TIMER,0,NULL);
			}
			else
			{
				MessageBox(L"Could not capture camera.", L"Error", 
				MB_ICONERROR | MB_OK);
				return;
			}
		}
		
		else
		{
			KillTimer(DISPLAY_TIMER);
			cvReleaseCapture(&m_pCapture);
			m_bIsTracking = FALSE;
			ResetEyeBlinks();
			::EnableWindow(GetDlgItem(IDC_BTMP)->m_hWnd,FALSE);
			SetDlgItemText(IDC_BTRACK,L"Start Tracking");
		}
}


void CCameraDlg::OnClickedBmousectrl()
{
	m_bIsMouseControl = !m_bIsMouseControl;
		
	if(m_bIsMouseControl)
		SetDlgItemText(IDC_BMOUSECTRL,L"Switch Mouse Control Off");
	else
		SetDlgItemText(IDC_BMOUSECTRL,L"Switch Mouse Control On");
}


void CCameraDlg::OnBnClickedBtmp()
{
		
	if(!m_bIsTmpSet)
	{
		m_cTemplateMidPoint = m_cCurrentMidPoint;
		m_bIsTmpSet = TRUE;
		SetDlgItemText(IDC_BTMP,L"Reset Template");
	}
	
	else
	{
		m_bIsTmpSet = FALSE;
		SetDlgItemText(IDC_BTMP,L"Set Template");
	}
}

BOOL CCameraDlg::PreTranslateMessage(MSG* pMsg)
{
 	if(pMsg->message==WM_KEYDOWN)
          {			  
			  if(pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)
                  pMsg->wParam=NULL ;
        
	} 
	
return CDialog::PreTranslateMessage(pMsg);
}


void CCameraDlg::ResetEyeBlinks()
{
	CObjectDetection::DetectLeftBlink(m_pLeftEyeImg,m_iLastFramesNum,m_iVarrianceBlink,m_dRatioThreshold,TRUE);
	CObjectDetection::DetectRightBlink(m_pLeftEyeImg,m_iLastFramesNum,m_iVarrianceBlink,m_dRatioThreshold,TRUE);
}