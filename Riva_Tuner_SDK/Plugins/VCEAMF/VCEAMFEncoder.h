#pragma once
//////////////////////////////////////////////////////////////////////
#include <afxtempl.h>
//////////////////////////////////////////////////////////////////////
#include "OpenVideo\OVEncode.h"
#include "OpenVideo\OVEncodeTypes.h"
#include "cl\cl.h"

#include "amf\core\Context.h"
#include "amf\components\Component.h"
#include "amf\components\VideoEncoderVCE.h"
//////////////////////////////////////////////////////////////////////
#define VCEAMF_FRAMETYPE_I												1
//////////////////////////////////////////////////////////////////////
typedef struct VCEAMF_ENCODED_FRAME
{
	DWORD		dwFrameType;
	LONGLONG	qwTimestamp;

	LPBYTE		lpData;
	DWORD		dwSize;
	DWORD		dwPos;
} VCEAMF_ENCODED_FRAME, *LPVCEAMF_ENCODED_FRAME;
//////////////////////////////////////////////////////////////////////
#define VCEAMF_ERR_NONE												0

#define VCEAMF_ERR_UNKNOWN											-1
#define VCEAMF_ERR_UNSUPPORTED										-2
#define VCEAMF_ERR_AMFCREATECONTEXT_FAILED							-3
#define VCEAMF_ERR_AMFCREATECOMPONENT_FAILED						-4
#define VCEAMF_ERR_AMFCONTEXT_INITDX9_FAILED						-5
#define VCEAMF_ERR_AMFCONTEXT_INITDX11_FAILED						-6
#define VCEAMF_ERR_AMFCONTEXT_INITOPENCL_FAILED						-7
#define VCEAMF_ERR_AMFCONTEXT_ALLOCSURFACE_FAILED					-8
#define VCEAMF_ERR_AMFCOMPONENT_SETPROPERTY_FAILED					-9
#define VCEAMF_ERR_AMFCOMPONENT_INIT_FAILED							-10
#define VCEAMF_ERR_AMFCOMPONENT_SUBMITINPUT_FAILED					-11
#define VCEAMF_ERR_AMFCOMPONENT_QUERYOUTPUT_FAILED					-12
//////////////////////////////////////////////////////////////////////
class CVCEAMFEncoder : public CList<LPVCEAMF_ENCODED_FRAME, LPVCEAMF_ENCODED_FRAME>
{
public:
	//encoding/decoding
	int						StartEncoding(int width, int height, int framerate, int targetBitrate, int device, amf::AMF_MEMORY_TYPE memoryTypeIn, LPCSTR lpConfigFile);
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
	LPVCEAMF_ENCODED_FRAME	GetFrame();
	void					DestroyFrame(LPVCEAMF_ENCODED_FRAME lpFrame);
	void					DestroyFrames();

	//misc
	static LPCSTR			GetStatusStr(int status);

	CVCEAMFEncoder();
	~CVCEAMFEncoder();

protected:
	void					SaveFrame(amf::AMFDataPtr lpData);
	void					LoadFrame(amf::AMFSurfacePtr lpSurface, int width, int height, LPBYTE lpSrc, DWORDLONG qwTimestamp);
	void					LoadFrame(LPBYTE lpDstY, LPBYTE lpDstUV, DWORD dwDstPitch, int width, int height, LPBYTE lpSrc);
	int						FlushFrames();

	int						GetConfigMap(CMapStringToString* pConfigMap, LPCSTR lpName);

	void					AddAnnexB(LPBYTE lpSrc, DWORD dwSize, LPVCEAMF_ENCODED_FRAME lpDst);
	void					AddNAL(LPBYTE lpSrc, DWORD dwSize, LPVCEAMF_ENCODED_FRAME lpDst);

	void					InitHeader();

    amf::AMFContextPtr		m_lpContext;
	amf::AMFComponentPtr	m_lpEncoder;

	amf::AMF_MEMORY_TYPE	m_memoryTypeIn;
	amf::AMF_SURFACE_FORMAT m_formatIn;

	BYTE					m_header[256];
	DWORD					m_headerSize;

	BYTE					m_pps[256];
	DWORD					m_ppsSize;

	BYTE					m_sps[256];
	DWORD					m_spsSize;

	char					m_szDesc[MAX_PATH];
};
//////////////////////////////////////////////////////////////////////
