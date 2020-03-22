// EyeTracker.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "EyeTracker.h"
#include "EyeTrackerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEyeTrackerApp

BEGIN_MESSAGE_MAP(CEyeTrackerApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CEyeTrackerApp construction

CEyeTrackerApp::CEyeTrackerApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}


// The one and only CEyeTrackerApp object

CEyeTrackerApp theApp;


// CEyeTrackerApp initialization

BOOL CEyeTrackerApp::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    AfxEnableControlContainer();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    CEyeTrackerDlg dlg;
    m_pMainWnd = &dlg;
    typedef HRESULT(STDAPICALLTYPE* FuncDllRegisterServer)();

    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with OK
    }
    else if (nResponse == IDCANCEL)
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with Cancel
    }
    return FALSE;
}


extern CEyeTrackerDlg* EyeTrackerDlg;

void Log(CString strMessage)
{
    if (EyeTrackerDlg)
        EyeTrackerDlg->Log(strMessage);
}

void SetDlgItemDouble(CWnd* pWnd, int iIdc, double dValue)
{
    CString strString;
    strString.Format(L"%.2f", dValue);
    SetDlgItemText(pWnd->m_hWnd, iIdc, strString);
}

double GetDlgItemDouble(CWnd* pWnd, int iIdc)
{

    CString strString;
    GetDlgItemText(pWnd->m_hWnd, iIdc, (LPWSTR)strString.GetBuffer(100), 100);
    double dValue = wcstod(strString, NULL);
    strString.ReleaseBuffer();

    return dValue;
}