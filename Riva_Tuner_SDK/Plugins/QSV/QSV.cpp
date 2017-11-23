// QSV.cpp : Defines the initialization routines for the DLL.
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <float.h>
#include <shlwapi.h>
#include <afxdllx.h>
#include <winbase.h>
 
#include "QSV.h"
#include "QSVEncoder.h"
#include "QSVConfigurationDlg.h"
#include "EncoderPluginTypes.h"
/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
static AFX_EXTENSION_MODULE QSVDLL = { NULL, NULL };
/////////////////////////////////////////////////////////////////////////////
HINSTANCE					g_hModule				= 0;
CQSVEncoder*				g_lpEncoder				= NULL;
char						g_szCfgPath[MAX_PATH]	= { 0 };
mfxStatus					g_lastError				= MFX_ERR_NONE;
/////////////////////////////////////////////////////////////////////////////
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (!AfxInitExtensionModule(QSVDLL, hInstance))
			return 0;

		new CDynLinkLibrary(QSVDLL);

		g_hModule = hInstance;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(QSVDLL);
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////
QSV_API HRESULT Init(LPCSTR lpCfgPath)
{
	if (lpCfgPath)
		strcpy_s(g_szCfgPath, sizeof(g_szCfgPath), lpCfgPath);
	else
	{
		GetModuleFileName(g_hModule, g_szCfgPath, sizeof(g_szCfgPath));

		PathRenameExtension(g_szCfgPath, ".cfg");
	}

	if (g_lpEncoder)
		return S_FALSE;

	g_lpEncoder = new CQSVEncoder;

	return S_OK;
}
//////////////////////////////////////////////////////////////////////
QSV_API HRESULT Uninit()
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
QSV_API DWORD GetEncodersNum()
{
	return 1;
}
//////////////////////////////////////////////////////////////////////
BOOL IsLoadedByEncoderServer()
{
	char szModuleName[MAX_PATH];
	GetModuleFileName(NULL, szModuleName, MAX_PATH);
	PathStripPath(szModuleName);

	return !_stricmp(szModuleName, "EncoderServer.exe");
}
//////////////////////////////////////////////////////////////////////
QSV_API HRESULT GetEncoderCaps(DWORD dwEncoder, LPENCODER_CAPS lpCaps)
{
	if (!lpCaps)
		return E_POINTER;

	int d3dMode				= GetConfigInt("AVC", "D3DMode"				, CQSVEncoder::GetSuggestedMode()	);

	switch (dwEncoder)
	{
	case 0:
		lpCaps->dwFlags				= ENCODER_CAPS_FLAGS_RESIZE_SUPPORTED | ENCODER_CAPS_FLAGS_CONFIGURE_SUPPORTED;

		if ((d3dMode != QSV_ENCODER_MODE_HARDWARE_D3D9) || !GetModuleHandle("d3d9.dll") || IsLoadedByEncoderServer())
			lpCaps->dwFlags			|= ENCODER_CAPS_FLAGS_MULTITHREADING_SAFE;

		lpCaps->dwInputFormatsNum	= 2;
		lpCaps->dwInputFormats[0]	= ENCODER_INPUT_FORMAT_RGB3;
		lpCaps->dwInputFormats[1]	= ENCODER_INPUT_FORMAT_RGB4;
		lpCaps->dwOutputFormat		= ' CVA';
		strcpy_s(lpCaps->szDesc, sizeof(lpCaps->szDesc), "Intel QuickSync H.264");
		return S_OK;
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////
QSV_API HRESULT GetEncoderStat(DWORD dwEncoder, LPENCODER_STAT lpStat)
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
QSV_API HRESULT Configure(DWORD dwEncoder, HWND hParent)
{
	switch (dwEncoder)
	{
	case 0:
		CQSVConfigurationDlg dlg;
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
QSV_API HRESULT StartEncoding(DWORD dwEncoder, LPENCODER_INPUT lpInput)
{
	if (!g_lpEncoder)
		return E_UNEXPECTED;
		
	if (!lpInput)
		return E_POINTER;

	switch (dwEncoder)
	{
	case 0:

		//get encoder config
		int d3dMode			= GetConfigInt("AVC", "D3DMode"			, CQSVEncoder::GetSuggestedMode()	);
		int targetUsage		= GetConfigInt("AVC", "TargetUsage"		, MFX_TARGETUSAGE_BALANCED			);
		int targetBitrate	= GetConfigInt("AVC", "TargetBitrate"	, 5000								);
		int asyncDepth		= GetConfigInt("AVC", "AsyncDepth"		, 4									);

		g_lastError			= g_lpEncoder->StartEncoding(lpInput->dwInputWidth, lpInput->dwInputHeight, lpInput->dwOutputWidth, lpInput->dwOutputHeight, lpInput->dwFramerate, d3dMode, targetUsage, targetBitrate, MFX_PROFILE_UNKNOWN, MFX_LEVEL_UNKNOWN, asyncDepth);

		if (g_lastError != MFX_ERR_NONE)
			return E_FAIL;

		return S_OK;
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////
QSV_API HRESULT Encode(DWORD dwEncoder, LPENCODER_INPUT lpInput)
{
	if (!g_lpEncoder)
		return E_UNEXPECTED;
		
	if (!lpInput)
		return E_POINTER;

	switch (dwEncoder)
	{
	case 0:
		int depth = 0;

		switch (lpInput->dwInputFormat)
		{
		case ENCODER_INPUT_FORMAT_RGB3:
			depth = 24;
			break;
		case ENCODER_INPUT_FORMAT_RGB4:
			depth = 32;
			break;
		default:
			return E_INVALIDARG;
		}

		g_lastError = g_lpEncoder->Encode(lpInput->dwInputWidth, lpInput->dwInputHeight, depth, lpInput->lpInputData, lpInput->qwTimestamp);

		if (g_lastError != MFX_ERR_NONE)
			return E_FAIL;

		return S_OK;
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////
QSV_API BOOL StopEncoding(DWORD dwEncoder)
{
	if (!g_lpEncoder)
		return E_UNEXPECTED;
		
	switch (dwEncoder)
	{
	case 0:
		g_lastError = g_lpEncoder->StopEncoding();

		if (g_lastError != MFX_ERR_NONE)
			return E_FAIL;

		return S_OK;
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////
QSV_API LPBYTE GetHeader(DWORD dwEncoder)
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
QSV_API DWORD GetHeaderSize(DWORD dwEncoder)
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
QSV_API HRESULT GetFrame(DWORD dwEncoder, LPENCODER_OUTPUT lpOutput)
{
	if (!g_lpEncoder)
		return E_UNEXPECTED;

	if (!lpOutput)
		return E_POINTER;

	switch (dwEncoder)
	{
	case 0:
		LPQSV_ENCODED_FRAME lpFrame = g_lpEncoder->GetFrame();

		if (lpFrame)
		{
			lpOutput->dwFlags		= 0;

			if (lpFrame->dwFrameType & MFX_FRAMETYPE_I)
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
QSV_API HRESULT ReleaseFrame(DWORD dwEncoder, LPENCODER_OUTPUT lpOutput)
{
	if (!g_lpEncoder)
		return E_UNEXPECTED;

	if (!lpOutput || !lpOutput->lpContext)
		return E_POINTER;

	switch (dwEncoder)
	{
	case 0:
		g_lpEncoder->DestroyFrame((LPQSV_ENCODED_FRAME)lpOutput->lpContext);

		lpOutput->lpOutputData	= NULL;
		lpOutput->dwOutputSize	= 0;
		lpOutput->lpContext		= NULL;

		return S_OK;
	}

	return E_INVALIDARG;
}
//////////////////////////////////////////////////////////////////////


