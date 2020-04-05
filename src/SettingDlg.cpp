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

#include "SettingDlg.hpp"
#include "opencv2/videoio/videoio.hpp"

using namespace cv;

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

class videoDevice;
class IBaseFilter;
class IAMStreamConfig;
class IGraphBuilder;
class ICaptureGraphBuilder2;

#define VI_MAX_CAMERAS 20
#define VI_NUM_TYPES 20
#define VI_NUM_FORMATS 18

class videoInput {

public:
    videoInput();
    ~videoInput();

    //Functions in rough order they should be used.
    static int listDevices(bool silent = false);

    //needs to be called after listDevices - otherwise returns NULL
    static char* getDeviceName(int deviceID);

    //choose to use callback based capture - or single threaded
    void setUseCallback(bool useCallback);

    //call before setupDevice
    //directshow will try and get the closest possible framerate to what is requested
    void setIdealFramerate(int deviceID, int idealFramerate);

    //some devices will stop delivering frames after a while - this method gives you the option to try and reconnect
    //to a device if videoInput detects that a device has stopped delivering frames.
    //you MUST CALL isFrameNew every app loop for this to have any effect
    void setAutoReconnectOnFreeze(int deviceNumber, bool doReconnect, int numMissedFramesBeforeReconnect);

    //Choose one of these five to setup your device
    bool setupDevice(int deviceID);
    bool setupDevice(int deviceID, int w, int h);
    bool setupDeviceFourcc(int deviceID, int w, int h, int fourcc);

    //These two are only for capture cards
    //USB and Firewire cameras souldn't specify connection
    bool setupDevice(int deviceID, int connection);
    bool setupDevice(int deviceID, int w, int h, int connection);

    bool setFourcc(int deviceNumber, int fourcc);

    //If you need to you can set your NTSC/PAL/SECAM
    //preference here. if it is available it will be used.
    //see #defines above for available formats - eg VI_NTSC_M or VI_PAL_B
    //should be called after setupDevice
    //can be called multiple times
    bool setFormat(int deviceNumber, int format);

    //Tells you when a new frame has arrived - you should call this if you have specified setAutoReconnectOnFreeze to true
    bool isFrameNew(int deviceID);

    bool isDeviceSetup(int deviceID) const;

    //Returns the pixels - flipRedAndBlue toggles RGB/BGR flipping - and you can flip the image too
    unsigned char* getPixels(int deviceID, bool flipRedAndBlue = true, bool flipImage = false);

    //Or pass in a buffer for getPixels to fill returns true if successful.
    bool getPixels(int id, unsigned char* pixels, bool flipRedAndBlue = true, bool flipImage = false);

    //Launches a pop up settings window
    //For some reason in GLUT you have to call it twice each time.
    bool showSettingsWindow(int deviceID);

    //Manual control over settings thanks.....
    //These are experimental for now.
    bool setVideoSettingFilter(int deviceID, long Property, long lValue, long Flags = 0, bool useDefaultValue = false);
    bool setVideoSettingFilterPct(int deviceID, long Property, float pctValue, long Flags = 0);
    bool getVideoSettingFilter(int deviceID, long Property, long& min, long& max, long& SteppingDelta, long& currentValue, long& flags, long& defaultValue);

    bool setVideoSettingCamera(int deviceID, long Property, long lValue, long Flags = 0, bool useDefaultValue = false);
    bool setVideoSettingCameraPct(int deviceID, long Property, float pctValue, long Flags = 0);
    bool getVideoSettingCamera(int deviceID, long Property, long& min, long& max, long& SteppingDelta, long& currentValue, long& flags, long& defaultValue);

    //bool setVideoSettingCam(int deviceID, long Property, long lValue, long Flags = NULL, bool useDefaultValue = false);

    //get width, height and number of pixels
    int getWidth(int deviceID) const;
    int getHeight(int deviceID) const;
    int getSize(int deviceID) const;
    int getFourcc(int deviceID) const;
    double getFPS(int deviceID) const;

    //completely stops and frees a device
    void stopDevice(int deviceID);

    //as above but then sets it up with same settings
    bool restartDevice(int deviceID);

    //number of devices available
    int devicesFound;

    // mapping from OpenCV CV_CAP_PROP to videoinput/dshow properties
    int getVideoPropertyFromCV(int cv_property);
    int getCameraPropertyFromCV(int cv_property);

    bool isDeviceDisconnected(int deviceID);

    int property_window_count(int device_idx);

private:
    void setPhyCon(int deviceID, int conn);
    void setAttemptCaptureSize(int deviceID, int w, int h, GUID mediaType);
    bool setup(int deviceID);
    void processPixels(unsigned char* src, unsigned char* dst, int width, int height, bool bRGB, bool bFlip);
    int start(int deviceID, videoDevice* VD);
    int getDeviceCount();
    void getMediaSubtypeAsString(GUID type, char* typeAsString);
    GUID* getMediaSubtypeFromFourcc(int fourcc);
    int getFourccFromMediaSubtype(GUID type) const;

    void getVideoPropertyAsString(int prop, char* propertyAsString);
    void getCameraPropertyAsString(int prop, char* propertyAsString);

    HRESULT getDevice(IBaseFilter** pSrcFilter, int deviceID, WCHAR* wDeviceName, char* nDeviceName);
    static HRESULT ShowFilterPropertyPages(IBaseFilter* pFilter);
    static HRESULT ShowStreamPropertyPages(IAMStreamConfig* pStream);

    HRESULT SaveGraphFile(IGraphBuilder* pGraph, WCHAR* wszPath);
    HRESULT routeCrossbar(ICaptureGraphBuilder2** ppBuild, IBaseFilter** pVidInFilter, int conType, GUID captureMode);

    //don't touch
    static bool comInit();
    static bool comUnInit();

    int connection;
    int callbackSetCount;
    bool bCallback;

    GUID CAPTURE_MODE;

    //Extra video subtypes
    // GUID MEDIASUBTYPE_Y800;
    // GUID MEDIASUBTYPE_Y8;
    // GUID MEDIASUBTYPE_GREY;

    videoDevice* VDList[VI_MAX_CAMERAS];
    GUID mediaSubtypes[VI_NUM_TYPES];
    long formatTypes[VI_NUM_FORMATS];

    static void __cdecl basicThread(void* objPtr);

    static char deviceNames[VI_MAX_CAMERAS][255];
};

void CSettingDlg::UpdateDeviceList(void)
{
    //videoInput VI;
    cv::VideoCapture v;
    char buf[256];
    int iCameraNum = 0;
    while (v.open(iCameraNum)) {
        ++iCameraNum;
        v.release();
    }
    m_cDeviceCombo.ResetContent();

    for (int i = 0; i < iCameraNum; ++i) {
        if (auto err = _itoa_s(i, buf, 10); err == 0) {
            CStringA strDeviceNameA = buf;
            m_cDeviceCombo.AddString(CString(strDeviceNameA));
        }
    }
}

BOOL CSettingDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_iSelectedAlg = DEF_ALG;

    m_iTmpHeight = DEF_TMP_HEIGHT;
    m_iTmpWidth = DEF_TMP_WIDTH;
    m_iAvgFaceFps = DEF_AVG_FPS_FACE;
    m_iAvgEyeFps = DEF_AVG_FPS_EYE;
    m_iFrameHeight = DEF_FRAME_HEIGHT;
    m_iFrameWidth = DEF_FRAME_WIDTH;

    m_iFrameNumClick = DEF_FRAME_CLICK;
    m_iThresholdClick = DEF_THRESHOLD_CLICK;
    m_fVarrianceRatio = DEF_VARRATIO_CLICK;
    m_fVarrianceRatio2 = DEF_VARRATIO_CLICK2;

    m_iAccH = DEF_ACC_H;
    m_iAccV = DEF_ACC_V;

    SetDlgItemInt(IDC_TMPH, m_iTmpHeight);
    SetDlgItemInt(IDC_TMPW, m_iTmpWidth);

    SetDlgItemInt(IDC_RAWH, m_iFrameHeight);
    SetDlgItemInt(IDC_RAWW, m_iFrameWidth);

    SetDlgItemInt(IDC_AVGF, m_iAvgFaceFps);
    SetDlgItemInt(IDC_AVGE, m_iAvgEyeFps);
    SetDlgItemInt(IDC_ACCH, m_iAccH);
    SetDlgItemInt(IDC_ACCV, m_iAccV);

    SetDlgItemInt(IDC_AVGFRAMENUM, m_iFrameNumClick);
    SetDlgItemInt(IDC_VTHRESHOLD, m_iThresholdClick);
    SetDlgItemDouble(this, IDC_RTHRESHOLD, m_fVarrianceRatio);
    SetDlgItemDouble(this, IDC_RTHRESHOLD2, m_fVarrianceRatio2);

    UpdateDeviceList();

    if (m_cDeviceCombo.GetCount() > 0) {
        m_iSelectedCamera = 0;
        m_cDeviceCombo.SetCurSel(m_iSelectedCamera);
    } else
        m_iSelectedCamera = -1;

    m_cAlgList.AddString(L"CDF Analysis");
    m_cAlgList.AddString(L"Edge Detection");
    m_cAlgList.AddString(L"GPF Detection");

    m_cAlgList.SetCurSel(m_iSelectedAlg);

    m_bSupportClicking = DEF_SUP_CLICK;
    m_bSupportDoubleClick = DEF_SUP_DBLCLICK;

    CheckDlgButton(IDC_CSUPDBLCLICK, m_bSupportDoubleClick);
    CheckDlgButton(IDC_CSUPCLICK, m_bSupportClicking);

    return TRUE;
}

void CSettingDlg::OnCbnDropdownCombodevice()
{
    UpdateDeviceList();
}

void CSettingDlg::Save()
{
    m_iTmpHeight = GetDlgItemInt(IDC_TMPH);
    m_iTmpWidth = GetDlgItemInt(IDC_TMPW);
    m_iFrameHeight = GetDlgItemInt(IDC_RAWH);
    m_iFrameWidth = GetDlgItemInt(IDC_RAWW);

    if (m_iTmpHeight <= 0) {
        m_iTmpHeight = DEF_TMP_HEIGHT;
        SetDlgItemInt(IDC_TMPH, m_iTmpHeight);
    }
    if (m_iTmpWidth <= 0) {
        m_iTmpWidth = DEF_TMP_WIDTH;
        SetDlgItemInt(IDC_TMPW, m_iTmpWidth);
    }

    if (m_iFrameHeight <= 0) {
        m_iFrameHeight = DEF_FRAME_HEIGHT;
        SetDlgItemInt(IDC_RAWH, m_iFrameHeight);
    }
    if (m_iFrameWidth <= 0) {
        m_iFrameWidth = DEF_FRAME_WIDTH;
        SetDlgItemInt(IDC_RAWW, m_iFrameWidth);
    }

    m_iAccH = DEF_ACC_H;
    m_iAccV = DEF_ACC_V;

    m_iAvgFaceFps = GetDlgItemInt(IDC_AVGF);
    m_iAvgEyeFps = GetDlgItemInt(IDC_AVGE);

    if (m_iAvgFaceFps == 0) {
        m_iAvgFaceFps = DEF_AVG_FPS_FACE;
        SetDlgItemInt(IDC_AVGF, m_iAvgFaceFps);
    }
    if (m_iAvgEyeFps == 0) {
        m_iAvgEyeFps = DEF_AVG_FPS_EYE;
        SetDlgItemInt(IDC_AVGE, m_iAvgEyeFps);
    }

    m_iAccH = GetDlgItemInt(IDC_ACCH);
    m_iAccV = GetDlgItemInt(IDC_ACCV);

    if (m_iAccH <= 0) {
        m_iAccH = DEF_ACC_H;
    }
    SetDlgItemInt(IDC_ACCH, m_iAccH);

    if (m_iAccV <= 0) {
        m_iAccV = DEF_ACC_V;
    }
    SetDlgItemInt(IDC_ACCV, m_iAccV);

    m_iFrameNumClick = GetDlgItemInt(IDC_AVGFRAMENUM);
    m_iThresholdClick = GetDlgItemInt(IDC_VTHRESHOLD);

    if (m_iFrameNumClick <= 0) {
        m_iFrameNumClick = DEF_FRAME_CLICK;
        SetDlgItemInt(IDC_AVGFRAMENUM, m_iFrameNumClick);
    }
    if (m_iThresholdClick <= 0 || m_iThresholdClick > 255) {
        m_iThresholdClick = DEF_THRESHOLD_CLICK;
        SetDlgItemInt(IDC_VTHRESHOLD, m_iThresholdClick);
    }

    m_fVarrianceRatio = GetDlgItemDouble(this, IDC_RTHRESHOLD);

    if (m_fVarrianceRatio <= 0 || m_fVarrianceRatio > 1) {
        m_fVarrianceRatio = DEF_VARRATIO_CLICK;
        SetDlgItemDouble(this, IDC_RTHRESHOLD, m_fVarrianceRatio);
    }
    m_fVarrianceRatio2 = GetDlgItemDouble(this, IDC_RTHRESHOLD2);

    if (m_fVarrianceRatio <= 0 || m_fVarrianceRatio > 1) {
        m_fVarrianceRatio2 = DEF_VARRATIO_CLICK2;
        SetDlgItemDouble(this, IDC_RTHRESHOLD2, m_fVarrianceRatio2);
    }

    m_iSelectedCamera = m_cDeviceCombo.GetCurSel();
    if (m_iSelectedCamera == -1 && m_cDeviceCombo.GetCount()) {
        m_iSelectedCamera = 0;
        m_cDeviceCombo.SetCurSel(m_iSelectedCamera);
    }
    m_iSelectedAlg = m_cAlgList.GetCurSel();

    m_bSupportClicking = IsDlgButtonChecked(IDC_CSUPCLICK);
    m_bSupportDoubleClick = IsDlgButtonChecked(IDC_CSUPDBLCLICK);
}
