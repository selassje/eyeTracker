#pragma once

#include <vector>
#include <map>
#include "graph.h"
#include "afxcmn.h"
#include "stdafx.h"

// CComparerDlg dialog

class CComparerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CComparerDlg)

private :
	int m_iImgCount;
	int m_iFaceCount;
	int m_iEyesCount;
	int m_iPointsNumber;
	double m_dErrorThreshold;
	std::vector<CString> m_lImages;
	std::map<double,double> m_CDFPoints;
	std::map<double,double> m_EdgePoints;
	std::map<double,double> m_GPFPoints;

public:
	CComparerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CComparerDlg();

// Dialog Data
	enum { IDD = IDD_COMPARER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	CGraph m_cGraph;
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
private:
	void Plot(void);
public:
	afx_msg void OnBnClickedCdfg();
	afx_msg void OnBnClickedEdgeg();
	afx_msg void OnBnClickedGpfg();
private:
	void SetupGraph(void);
public:
	CProgressCtrl m_cProgCtrl;
	afx_msg void OnBnClickedExport();
};



class SBioIDEyeCenters : public CObject
{
	DECLARE_SERIAL(SBioIDEyeCenters)
	
	public:
		void Serialize(CArchive& ar);

		double Error(CvPoint cLeftEyeEst,CvPoint cRightEyeEst)
		{
			double dRealDist = Distance(cLeftEye,cRightEye);
			double dMaxEstDist = MAX(Distance(cLeftEye,cLeftEyeEst),Distance(cRightEye,cRightEyeEst));
			return dMaxEstDist/dRealDist;
		}
	private :	
		CvPoint cLeftEye;
		CvPoint cRightEye;
		
		double Distance(CvPoint p1,CvPoint p2)
		{
			return std::sqrt( (double)((p1.x - p2.x)*(p1.x - p2.x)+(p1.y - p2.y)*(p1.y - p2.y)));
		}

};




