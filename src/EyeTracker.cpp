/*
MIT License

Copyright (c) 2020 Przemyslaw Koziol

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "stdafx.h"
#include "EyeTracker.hpp"
#include "EyeTrackerDlg.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CEyeTrackerApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CEyeTrackerApp::CEyeTrackerApp()
{
   
}


CEyeTrackerApp theApp;

BOOL CEyeTrackerApp::InitInstance()
{
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
  
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    AfxEnableControlContainer();

    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    CEyeTrackerDlg dlg;
    m_pMainWnd = &dlg;
    typedef HRESULT(STDAPICALLTYPE* FuncDllRegisterServer)();

    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
      
    }
    else if (nResponse == IDCANCEL)
    {
       
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