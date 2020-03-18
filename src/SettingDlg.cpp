
#include "stdafx.h"
#include "EyeTracker.h"
#include "SettingDlg.h"
#include "videoinput.h"



#define DEF_ALG 2
#define DEF_TMP_WIDTH 20
#define DEF_TMP_HEIGHT 20
#define DEF_AVG_FPS_FACE 1
#define DEF_AVG_FPS_EYE 1
#define DEF_ACC_H 2
#define DEF_ACC_V 2
#define DEF_SUP_CLICK 1
#define DEF_SUP_DBLCLICK 0
#define DEF_FRAME_WIDTH 320
#define DEF_FRAME_HEIGHT 240
#define DEF_FRAME_CLICK 10
#define DEF_THRESHOLD_CLICK 100
#define DEF_VARRATIO_CLICK 0.2
#define DEF_VARRATIO_CLICK2 0.4

IMPLEMENT_DYNAMIC(CSettingDlg, CDialog)

CSettingDlg::CSettingDlg(CWnd* pParent)
	: CDialog(CSettingDlg::IDD, pParent)
{

}

CSettingDlg::~CSettingDlg()
{
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBODEVICE, m_cDeviceCombo);
	DDX_Control(pDX, IDC_ALGCOMBO, m_cAlgList);
}


BEGIN_MESSAGE_MAP(CSettingDlg, CDialog)
	ON_CBN_DROPDOWN(IDC_COMBODEVICE, &CSettingDlg::OnCbnDropdownCombodevice)
END_MESSAGE_MAP()



void CSettingDlg::UpdateDeviceList(void)
{
	videoInput VI;
	m_cDeviceCombo.ResetContent();
	int iCameraNum = VI.listDevices(); 
	for(int i =0;i<iCameraNum;++i)
	{
		CStringA strDeviceNameA =  VI.getDeviceName(i);
		CString strDeviceName(strDeviceNameA);
		m_cDeviceCombo.AddString(strDeviceName);
	}
}

BOOL CSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_iSelectedAlg = DEF_ALG;
	
	m_iTmpHeight=DEF_TMP_HEIGHT;
	m_iTmpWidth = DEF_TMP_WIDTH;
	m_iAvgFaceFps = DEF_AVG_FPS_FACE;
	m_iAvgEyeFps = DEF_AVG_FPS_EYE;
	m_iFrameHeight=DEF_FRAME_HEIGHT;
	m_iFrameWidth = DEF_FRAME_WIDTH;
	
	m_iFrameNumClick=DEF_FRAME_CLICK;
	m_iThresholdClick=DEF_THRESHOLD_CLICK;
	m_fVarrianceRatio=DEF_VARRATIO_CLICK;
	m_fVarrianceRatio2=DEF_VARRATIO_CLICK2;

    m_iAccH = DEF_ACC_H;
	m_iAccV = DEF_ACC_V;

	SetDlgItemInt(IDC_TMPH,m_iTmpHeight);
	SetDlgItemInt(IDC_TMPW,m_iTmpWidth);

	SetDlgItemInt(IDC_RAWH,m_iFrameHeight);
	SetDlgItemInt(IDC_RAWW,m_iFrameWidth);

	SetDlgItemInt(IDC_AVGF,m_iAvgFaceFps);
	SetDlgItemInt(IDC_AVGE,m_iAvgEyeFps);
	SetDlgItemInt(IDC_ACCH,m_iAccH);
	SetDlgItemInt(IDC_ACCV,m_iAccV);


	SetDlgItemInt(IDC_AVGFRAMENUM,m_iFrameNumClick);
	SetDlgItemInt(IDC_VTHRESHOLD,m_iThresholdClick);
	SetDlgItemDouble(this,IDC_RTHRESHOLD,m_fVarrianceRatio);
	SetDlgItemDouble(this,IDC_RTHRESHOLD2,m_fVarrianceRatio2);

	UpdateDeviceList();

	if(m_cDeviceCombo.GetCount() > 0 )
	{
		m_iSelectedCamera = 0;
		m_cDeviceCombo.SetCurSel(m_iSelectedCamera);
	}
	else
		m_iSelectedCamera = -1;

	m_cAlgList.AddString(L"CDF Analysis");
	m_cAlgList.AddString(L"Edge Detection");
	m_cAlgList.AddString(L"GPF Detection");

	m_cAlgList.SetCurSel(m_iSelectedAlg);

	m_bSupportClicking = DEF_SUP_CLICK;
	m_bSupportDoubleClick = DEF_SUP_DBLCLICK;

	CheckDlgButton(IDC_CSUPDBLCLICK,m_bSupportDoubleClick);
	CheckDlgButton(IDC_CSUPCLICK,m_bSupportClicking);


	return TRUE;  
}

void CSettingDlg::OnCbnDropdownCombodevice()
{
	UpdateDeviceList();
}



void CSettingDlg::Save()
{
	m_iTmpHeight = GetDlgItemInt(IDC_TMPH);
	m_iTmpWidth =  GetDlgItemInt(IDC_TMPW);
	m_iFrameHeight = GetDlgItemInt(IDC_RAWH);
	m_iFrameWidth =  GetDlgItemInt(IDC_RAWW);

	if(m_iTmpHeight <= 0)
	{
		m_iTmpHeight = DEF_TMP_HEIGHT;
		SetDlgItemInt(IDC_TMPH,m_iTmpHeight);
	}
	if(m_iTmpWidth <= 0)
	{
		m_iTmpWidth = DEF_TMP_WIDTH;
		SetDlgItemInt(IDC_TMPW,m_iTmpWidth);
	}

	if(m_iFrameHeight <= 0)
	{
		m_iFrameHeight = DEF_FRAME_HEIGHT;
		SetDlgItemInt(IDC_RAWH,m_iFrameHeight);
	}
	if(m_iFrameWidth <= 0)
	{
		m_iFrameWidth = DEF_FRAME_WIDTH;
		SetDlgItemInt(IDC_RAWW,m_iFrameWidth);
	}


	m_iAccH = DEF_ACC_H;
	m_iAccV = DEF_ACC_V;


	m_iAvgFaceFps = GetDlgItemInt(IDC_AVGF);
	m_iAvgEyeFps =  GetDlgItemInt(IDC_AVGE);

	if(m_iAvgFaceFps == 0)
	{
		m_iAvgFaceFps = DEF_AVG_FPS_FACE;
		SetDlgItemInt(IDC_AVGF,m_iAvgFaceFps);
	}
	if(m_iAvgEyeFps == 0)
	{
		m_iAvgEyeFps = DEF_AVG_FPS_EYE;
		SetDlgItemInt(IDC_AVGE,m_iAvgEyeFps);
	}

	m_iAccH = GetDlgItemInt(IDC_ACCH);
	m_iAccV = GetDlgItemInt(IDC_ACCV);

	if(m_iAccH <= 0)
	{
		m_iAccH = DEF_ACC_H;
	}
	SetDlgItemInt(IDC_ACCH,m_iAccH);

	if(m_iAccV <= 0)
	{
		m_iAccV = DEF_ACC_V;
	}
	SetDlgItemInt(IDC_ACCV,m_iAccV);
	

	m_iFrameNumClick = GetDlgItemInt(IDC_AVGFRAMENUM);
	m_iThresholdClick =  GetDlgItemInt(IDC_VTHRESHOLD);

	if(m_iFrameNumClick <= 0)
	{
		m_iFrameNumClick = DEF_FRAME_CLICK;
		SetDlgItemInt(IDC_AVGFRAMENUM,m_iFrameNumClick);
	}
	if(m_iThresholdClick <= 0 || m_iThresholdClick> 255)
	{
		m_iThresholdClick = DEF_THRESHOLD_CLICK;
		SetDlgItemInt(IDC_VTHRESHOLD,m_iThresholdClick);
	}

	m_fVarrianceRatio = GetDlgItemDouble(this,IDC_RTHRESHOLD);

	if(m_fVarrianceRatio <= 0 || m_fVarrianceRatio> 1)
	{
		m_fVarrianceRatio = DEF_VARRATIO_CLICK;
		SetDlgItemDouble(this,IDC_RTHRESHOLD,m_fVarrianceRatio);
	}
	m_fVarrianceRatio2 = GetDlgItemDouble(this,IDC_RTHRESHOLD2);

	if(m_fVarrianceRatio <= 0 || m_fVarrianceRatio> 1)
	{
		m_fVarrianceRatio2 = DEF_VARRATIO_CLICK2;
		SetDlgItemDouble(this,IDC_RTHRESHOLD2,m_fVarrianceRatio2);
	}


	m_iSelectedCamera = m_cDeviceCombo.GetCurSel();
	if(m_iSelectedCamera== -1 && m_cDeviceCombo.GetCount())
	{
		m_iSelectedCamera = 0;
		m_cDeviceCombo.SetCurSel(m_iSelectedCamera);
	}
	m_iSelectedAlg = m_cAlgList.GetCurSel();
	
	m_bSupportClicking =  IsDlgButtonChecked(IDC_CSUPCLICK);
	m_bSupportDoubleClick =  IsDlgButtonChecked(IDC_CSUPDBLCLICK);

}
