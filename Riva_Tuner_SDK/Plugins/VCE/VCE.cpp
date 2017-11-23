// VCE.cpp : Defines the initialization routines for the DLL.
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <float.h>
#include <shlwapi.h>
#include <afxdllx.h>
#include <winbase.h>
#include <io.h>
 
#include "VCE.h"
#include "VCEEncoder.h"
#include "VCEConfigurationDlg.h"
#include "EncoderPluginTypes.h"
/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
static AFX_EXTENSION_MODULE VCEDLL = { NULL, NULL };
/////////////////////////////////////////////////////////////////////////////
HINSTANCE					g_hModule				= 0;
CVCEEncoder*				g_lpEncoder				= NULL;
char						g_szCfgPath[MAX_PATH]	= { 0 };
char						g_szDllPath[MAX_PATH]	= { 0 };
int							g_lastError				= 0;
/////////////////////////////////////////////////////////////////////////////
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (!AfxInitExtensionModule(VCEDLL, hInstance))
			return 0;

		new CDynLinkLibrary(VCEDLL);

		g_hModule = hInstance;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(VCEDLL);
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////
VCE_API HRESULT Init(LPCSTR lpCfgPath)
{
	char czPath[MAX_PATH];
	GetSystemDirectory(czPath, MAX_PATH);

#ifdef _WIN64
	strcat_s(czPath, sizeof(czPath), "\\OpenVideo64.dll");
#else
	strcat_s(czPath, sizeof(czPath), "\\OpenVideo.dll");
#endif

	if (_taccess(czPath, 0))
		return E_NOINTERFACE;

	if (lpCfgPath)
		strcpy_s(g_szCfgPath, sizeof(g_szCfgPath), lpCfgPath);
	else
	{
		GetModuleFileName(g_hModule, g_szCfgPath, sizeof(g_szCfgPath));

		PathRenameExtension(g_szCfgPath, ".cfg");
	}

	GetModuleFileName(g_hModule, g_szDllPath, sizeof(g_szDllPath));

	PathRemoveFileSpec(g_szDllPath);

	if (g_lpEncoder)
		return S_FALSE;

	g_lpEncoder = new CVCEEncoder;

	return S_OK;
}
//////////////////////////////////////////////////////////////////////
VCE_API HRESULT Uninit()
{
	if (g_lpEncoder)
	{
		delete g_lpEncoder;

		g_lpEncoder = NULL;

		return S_OK;
	}

	return S_FALSE;
}
//////////////////////////////////////////////////////////////////////
VCE_API DWORD GetEncodersNum()
{
	return 1;
}
//////////////////////////////////////////////////////////////////////
VCE_API HRESULT GetEncoderCaps(DWORD dwEncoder, LPENCODER_CAPS lpCaps)
{
	if (!lpCaps)
		return E_POINTER;

	switch (dwEncoder)
	{
	case 0:
		lpCaps->dwFlags				= ENCODER_CAPS_FLAGS_CONFIGURE_SUPPORTED | ENCODER_CAPS_FLAGS_MULTITHREADING_SAFE;

		lpCaps->dwInputFormatsNum	= 1;
		lpCaps->dwInputFormats[0]	= ENCODER_INPUT_FORMAT_NV12;
		lpCaps->dwOutputFormat		= ' CVA';
		strcpy_s(lpCaps->szDesc, sizeof(lpCaps->szDesc), "AMD VCE H.264");
		return S_OK;
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////
VCE_API HRESULT GetEncoderStat(DWORD dwEncoder, LPENCODER_STAT lpStat)
{
	if (!g_lpEncoder)
		return E_UNEXPECTED;
		
	if (!lpStat || !lpStat->lpData)
		return E_POINTER;

	switch (dwEncoder)
	{
	case 0:
		switch (lpStat->dwType)
		{
		case ENCODER_STAT_TYPE_DESC:
			strcpy_s((LPSTR)lpStat->lpData, lpStat->dwSize, g_lpEncoder->GetDesc());
			return S_OK;
		case ENCODER_STAT_TYPE_LAST_ERROR:
			strcpy_s((LPSTR)lpStat->lpData, lpStat->dwSize, g_lpEncoder->GetStatusStr(g_lastError));
			return S_OK;
		}
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////
VCE_API HRESULT Configure(DWORD dwEncoder, HWND hParent)
{
	switch (dwEncoder)
	{
	case 0:
		CVCEConfigurationDlg dlg;
		dlg.DoModal();

		return S_OK;
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////
int GetConfigInt(LPCSTR lpSection, LPCSTR lpName, int nDefault)
{
	return GetPrivateProfileInt(lpSection, lpName, nDefault, g_szCfgPath);
}
//////////////////////////////////////////////////////////////////////
void SetConfigInt(LPCSTR lpSection, LPCSTR lpName, int nValue)
{
	char szValue[MAX_PATH];
	sprintf_s(szValue, sizeof(szValue), "%d", nValue);

	WritePrivateProfileString(lpSection, lpName, szValue, g_szCfgPath);
}
//////////////////////////////////////////////////////////////////////
VCE_API HRESULT StartEncoding(DWORD dwEncoder, LPENCODER_INPUT lpInput)
{
	if (!g_lpEncoder)
		return E_UNEXPECTED;
		
	if (!lpInput)
		return E_POINTER;

	switch (dwEncoder)
	{
	case 0:

		//get encoder config
		int device			= GetConfigInt("AVC", "Device"			, 0									);
		int preset			= GetConfigInt("AVC", "Preset"			, 0									);
		int targetBitrate	= GetConfigInt("AVC", "TargetBitrate"	, 5000								);

		char szCfgPath[MAX_PATH];
		sprintf_s(szCfgPath, sizeof(szCfgPath), "%s\\VCEPreset%d.cfg", g_szDllPath, preset);

		g_lastError			= g_lpEncoder->StartEncoding(lpInput->dwInputWidth, lpInput->dwInputHeight, lpInput->dwFramerate, targetBitrate, device, szCfgPath);

		if (g_lastError != VCE_ERR_NONE)
			return E_FAIL;

		return S_OK;
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////
VCE_API HRESULT Encode(DWORD dwEncoder, LPENCODER_INPUT lpInput)
{
	if (!g_lpEncoder)
		return E_UNEXPECTED;
		
	if (!lpInput)
		return E_POINTER;

	switch (dwEncoder)
	{
	case 0:
		int depth = 0;

		if (lpInput->dwInputFormat != ENCODER_INPUT_FORMAT_NV12)
			return E_INVALIDARG;

		g_lastError = g_lpEncoder->Encode(lpInput->dwInputWidth, lpInput->dwInputHeight, lpInput->lpInputData, lpInput->qwTimestamp);

		if (g_lastError != VCE_ERR_NONE)
			return E_FAIL;

		return S_OK;
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////
VCE_API BOOL StopEncoding(DWORD dwEncoder)
{
	if (!g_lpEncoder)
		return E_UNEXPECTED;
		
	switch (dwEncoder)
	{
	case 0:
		g_lastError = g_lpEncoder->StopEncoding();

		if (g_lastError != VCE_ERR_NONE)
			return E_FAIL;

		return S_OK;
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////
VCE_API LPBYTE GetHeader(DWORD dwEncoder)
{
	if (!g_lpEncoder)
		return NULL;
		
	switch (dwEncoder)
	{
	case 0:
		return g_lpEncoder->GetHeader();
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////
VCE_API DWORD GetHeaderSize(DWORD dwEncoder)
{
	if (!g_lpEncoder)
		return 0;

	switch (dwEncoder)
	{
	case 0:
		return g_lpEncoder->GetHeaderSize();
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////
VCE_API HRESULT GetFrame(DWORD dwEncoder, LPENCODER_OUTPUT lpOutput)
{
	if (!g_lpEncoder)
		return E_UNEXPECTED;

	if (!lpOutput)
		return E_POINTER;

	switch (dwEncoder)
	{
	case 0:
		LPVCE_ENCODED_FRAME lpFrame = g_lpEncoder->GetFrame();

		if (lpFrame)
		{
			lpOutput->dwFlags		= 0;

			if (lpFrame->dwFrameType & VCE_FRAMETYPE_I)
				lpOutput->dwFlags |= ENCODER_OUTPUT_FLAG_KEYFRAME;

			lpOutput->qwTimestamp	= lpFrame->qwTimestamp;
			lpOutput->lpOutputData	= lpFrame->lpData;
			lpOutput->dwOutputSize	= lpFrame->dwSize;
			lpOutput->lpContext		= lpFrame;

			return S_OK;
		}

		lpOutput->dwFlags		= 0;
		lpOutput->qwTimestamp	= 0;
		lpOutput->lpOutputData	= NULL;
		lpOutput->dwOutputSize	= 0;
		lpOutput->lpContext		= NULL;

		return S_FALSE;
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////
VCE_API HRESULT ReleaseFrame(DWORD dwEncoder, LPENCODER_OUTPUT lpOutput)
{
	if (!g_lpEncoder)
		return E_UNEXPECTED;

	if (!lpOutput || !lpOutput->lpContext)
		return E_POINTER;

	switch (dwEncoder)
	{
	case 0:
		g_lpEncoder->DestroyFrame((LPVCE_ENCODED_FRAME)lpOutput->lpContext);

		lpOutput->lpOutputData	= NULL;
		lpOutput->dwOutputSize	= 0;
		lpOutput->lpContext		= NULL;

		return S_OK;
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////


