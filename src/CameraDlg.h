#pragma once
#include "afxcmn.h"


#include <opencv2/highgui/highgui_c.h>
#include <opencv2/videoio/videoio_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc.hpp>

#include "resource.h"
#include "afxwin.h"


#define DISPLAY_WINDOW "Camera"
#define EYE_DEBUG_WINDOW "Eye"
#define EYER_DEBUG_WINDOW "LeftEye"
#define EYEL_DEBUG_WINDOW "RightEye"
#define IRIS_DEBUG_WINDOW "Iris"
#define EYEL_WINDOW "Left2"
#define EYER_WINDOW "Right2"

class CEyeTrackerDlg;

// CCameraDlg dialog

class CCameraDlg : public CDialog
{
	DECLARE_DYNAMIC(CCameraDlg)

private:
  CvCapture	*m_pCapture;
  IplImage	*m_pCurrentFrame;
  IplImage	*m_pCurrentOrgFrame;
  CEyeTrackerDlg *m_pEyeTrackerDlg;
  int m_iWndWidth;
  int m_iWndHeight;
  int m_iLeftEyeWndWidth;
  int m_iLeftEyeWndHeight;
  int m_iRightEyeWndWidth;
  int m_iRightEyeWndHeight;
  BOOL		m_bIsMouseControl;
  BOOL		m_bIsTracking;
  CvPoint m_cCurrentMidPoint;
  CvPoint m_cLastMidPoint;
  CvPoint m_cTemplateMidPoint;
  int m_iTemplateWidth;
  int m_iTemplateHeight;
  int m_iSelectedAlg;
  int m_iAvgFaceFps;
  int m_iAvgEyeFps;
  int m_iAccH;
  int m_iAccV;
  int m_iVarrianceBlink;
  int m_iLastFramesNum;
  double m_dRatioThreshold;
  double m_dRatioThreshold2;
  BOOL m_bSupportClicking;
  BOOL m_bSupportDoubleClick;
  DWORD m_iTickCount;


public:
	CCameraDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCameraDlg();
	IplImage	*m_pLeftEyeImg;
	IplImage	*m_pRightEyeImg;
// Dialog Data
	enum { IDD = IDD_CAMERADLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void PostNcDestroy();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButton1();
private:
	void MoveCursor(int iXOffset, int iYOffset);
	void PressLeftButton();
	void ReleaseLeftButton();
	void PressRightButton();
	void ReleaseRightButton();
	void ResetEyeBlinks();
public:
	afx_msg void OnClickedBtrack();
	afx_msg void OnClickedBmousectrl();
private:
	bool m_bIsTmpSet;
public:
	afx_msg void OnBnClickedBtmp();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
