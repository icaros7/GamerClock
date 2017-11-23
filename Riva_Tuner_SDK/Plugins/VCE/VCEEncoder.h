#pragma once
//////////////////////////////////////////////////////////////////////
#include <afxtempl.h>
//////////////////////////////////////////////////////////////////////
#include "OpenVideo\OVEncode.h"
#include "OpenVideo\OVEncodeTypes.h"
#include "cl\cl.h"
//////////////////////////////////////////////////////////////////////
#define VCE_FRAMETYPE_I												1
//////////////////////////////////////////////////////////////////////
typedef struct VCE_ENCODED_FRAME
{
	DWORD		dwFrameType;
	LONGLONG	qwTimestamp;

	LPBYTE		lpData;
	DWORD		dwSize;
	DWORD		dwPos;
} VCE_ENCODED_FRAME, *LPVCE_ENCODED_FRAME;
//////////////////////////////////////////////////////////////////////
#define MAX_INPUT_SURFACE										2
#define MAX_INPUT_TIMESTAMP										1024
//////////////////////////////////////////////////////////////////////
typedef struct OVDeviceHandle
{
    ovencode_device_info*	lpDeviceInfo;	// pointer to device info
	unsigned int			numDevices;		// number of devices available
	cl_platform_id			platform;		// platform
}OVDeviceHandle;
//////////////////////////////////////////////////////////////////////
typedef struct OVEncodeHandle
{
	ove_session				session;		// pointer to encoder session
	OPMemHandle				inputSurfaces[MAX_INPUT_SURFACE];	
											// input buffer
	cl_command_queue		clCmdQueue;		// command queue 
}OVEncodeHandle;
//////////////////////////////////////////////////////////////////////
typedef struct OVConfigCtrl
{
    unsigned int							height;
    unsigned int							width;
    OVE_ENCODE_MODE							encodeMode;

	OVE_PROFILE_LEVEL						profileLevel;	// profile level

    OVE_PICTURE_FORMAT						pictFormat;		// profile format
    OVE_ENCODE_TASK_PRIORITY				priority;		// priority settings

    OVE_CONFIG_PICTURE_CONTROL				pictControl;	// picture control
    OVE_CONFIG_RATE_CONTROL					rateControl;	// rate contorl config
    OVE_CONFIG_MOTION_ESTIMATION			meControl;		// motion Estimation settings
    OVE_CONFIG_RDO							rdoControl;		// rate distorsion optimization control
} OvConfigCtrl;
//////////////////////////////////////////////////////////////////////
#define VCE_ERR_NONE												0

#define VCE_ERR_UNKNOWN												-1
#define VCE_ERR_NULL_PTR											-2
#define VCE_ERR_UNSUPPORTED											-3
#define VCE_ERR_CLGETPLATFORMIDS_FAILED								-4
#define VCE_ERR_INVALID_DEVICE										-5
#define VCE_ERR_OVENCODEGETDEVICEINFO_FAILED						-6
#define VCE_ERR_CLCREATECONTEXT_FAILED								-7
#define VCE_ERR_OVENCODEGETDEVICECAP_FAILED							-8
#define VCE_ERR_OVENCODECREATESESSION_FAILED						-9
#define VCE_ERR_OVENCODESENDCONFIG_FAILED							-10
#define VCE_ERR_CLCREATECOMMANDQUEUE_FAILED							-11
#define VCE_ERR_CLCREATEBUFFER_FAILED								-12
#define VCE_ERR_CLENQUEUEMAPBUFFER_FAILED							-13
#define VCE_ERR_OVENCODETASK_FAILED									-14
#define VCE_ERR_CLWAITFOREVENTS_FAILED								-15
#define VCE_ERR_OVENCODEQUERYTASKDESCRIPTION_FAILED					-16
#define VCE_ERR_CLRELEASEMEMOBJECT_FAILED							-17
#define VCE_ERR_CLRELEASECOMMANDQUEUE_FAILED						-18
#define VCE_ERR_OVENCODEDESTROYSESSION_FAILED						-19
#define VCE_ERR_CLRELEASECONTEXT_FAILED								-20
//////////////////////////////////////////////////////////////////////
class CVCEEncoder : public CList<LPVCE_ENCODED_FRAME, LPVCE_ENCODED_FRAME>
{
public:
	//encoding/decoding
	int						StartEncoding(int width, int height, int framerate, int targetBitrate, int device, LPCSTR lpConfigFile);
	int						Encode(int width, int height, LPBYTE lpImage, DWORDLONG qwTimestamp);
	int						StopEncoding();

	//encoder info
	LPCSTR					GetDesc();

	//destruction
	void					Cleanup();

	//header access
	LPBYTE					GetHeader();
	DWORD					GetHeaderSize();

	//encoded frame access
	LPVCE_ENCODED_FRAME		GetFrame();
	void					DestroyFrame(LPVCE_ENCODED_FRAME lpFrame);
	void					DestroyFrames();

	//misc
	static LPCSTR			GetStatusStr(int status);

	CVCEEncoder();
	~CVCEEncoder();

protected:
	void					SaveFrame(OVE_OUTPUT_DESCRIPTION* lpTask);
	void					LoadFrame(LPBYTE lpDst, int width, int height, LPBYTE lpSrc, DWORDLONG qwTimestamp);

	int						GetConfigMap(CMapStringToString* pConfigMap, LPCSTR lpName);

	void					AddAnnexB(LPBYTE lpSrc, DWORD dwSize, LPVCE_ENCODED_FRAME lpDst);
	void					AddNAL(LPBYTE lpSrc, DWORD dwSize, LPVCE_ENCODED_FRAME lpDst);
	void					WaitForEvent(cl_event inMapEvt);

	void					InitHeader();

	OPContextHandle			m_oveContext;
	OVDeviceHandle			m_deviceHandle;
	OVEncodeHandle			m_encodeHandle;
	OvConfigCtrl			m_configCtrl;

	int						m_frameIn;
	int						m_frameOut;

	BYTE					m_header[256];
	DWORD					m_headerSize;

	BYTE					m_pps[256];
	DWORD					m_ppsSize;

	BYTE					m_sps[256];
	DWORD					m_spsSize;

	char					m_szDesc[MAX_PATH];

	DWORDLONG				m_qwTimestamp[MAX_INPUT_TIMESTAMP];
};
//////////////////////////////////////////////////////////////////////
