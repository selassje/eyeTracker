#pragma once
#include "afxwin.h"



// CSettingDlg dialog

class CSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CSettingDlg)

public:
	CSettingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingDlg();

// Dialog Data
	enum { IDD = IDD_SETTINGDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CComboBox m_cDeviceCombo;
	void UpdateDeviceList(void);
public:
	virtual BOOL OnInitDialog();
	int m_iSelectedCamera;
	int m_iSelectedAlg;
	int m_iTmpWidth;
	int m_iTmpHeight;
	int m_iFrameWidth;
	int m_iFrameHeight;
	int m_iAvgFaceFps;
	int m_iAvgEyeFps;
	int m_iAccH;
	int m_iAccV;
	BOOL m_bSupportClicking;
	BOOL m_bSupportDoubleClick;
	double m_fVarrianceRatio;
	double m_fVarrianceRatio2;
	int m_iThresholdClick;
	int m_iFrameNumClick;
public:
	afx_msg void OnCbnDropdownCombodevice();
	void Save();
private:
	CComboBox m_cAlgList;
};
