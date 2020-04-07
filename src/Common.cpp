#include "Common.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/highgui/highgui_c.h"

void SetupWindow(const CWnd& cwnd, const char* windowName, int resourceId, int& width, int& height)
{
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
    HWND wnd = (HWND)cvGetWindowHandle(windowName);
    HWND parent = ::GetParent(wnd);
    CWnd* resourceWnd = cwnd.GetDlgItem(resourceId);
    RECT rect;
    resourceWnd->GetClientRect(&rect);
    width = rect.right;
    height = rect.bottom;
    ::SetParent(wnd, resourceWnd->m_hWnd);
    ::ShowWindow(parent, SW_HIDE);
}