#pragma once

#include "afxwin.h"
#include <vector>

// CalibrationDlg dialog

class CImageDlg : public CDialog
{
	DECLARE_DYNAMIC(CImageDlg)

public:
	CImageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CImageDlg();
	

// Dialog Data
	enum { IDD = IDD_IMAGEDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
private:
	IplImage* GetRightEyeImg(void);
	IplImage* GetLeftEyeImg(void);
	int m_iLeftEyeWidth;
	int m_iLeftEyeHeight;
	int m_iRightEyeWidth;
	int m_iRightEyeHeight;
	int m_iImgWidth;
	int m_iImgHeight;
	int m_iCurrentImg;
	int m_iImgCount;
	
	void AnalyzeCurrentImg();

public:
	virtual BOOL OnInitDialog();
private:
	CComboBox m_cEyeMovCombo;
	std::vector<CString> m_lImages;
	void AnalyzeImage(IplImage *pImg,IplImage** pLeftEye,IplImage** pRightEy);
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedPrev();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClickedCdf();
	afx_msg void OnClickedEdge();
	afx_msg void OnBnClickedGpf();
};
