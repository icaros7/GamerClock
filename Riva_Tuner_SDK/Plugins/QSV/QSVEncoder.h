#pragma once
//////////////////////////////////////////////////////////////////////
#include <afxtempl.h>
#include "mfxvideo++.h"
#include "QSVEncoderTask.h"
//////////////////////////////////////////////////////////////////////
typedef struct QSV_ENCODED_FRAME
{
	DWORD		dwFrameType;
	LONGLONG	qwTimestamp;
	LONGLONG	qwDecodeTimestamp;

	LPBYTE		lpData;
	DWORD		dwSize;
	DWORD		dwPos;
} QSV_ENCODED_FRAME, *LPQSV_ENCODED_FRAME;
//////////////////////////////////////////////////////////////////////
#define QSV_ENCODER_MODE_SOFTWARE							0x00000000
#define QSV_ENCODER_MODE_HARDWARE_D3D9						0x00000001
#define QSV_ENCODER_MODE_HARDWARE_D3D11						0x00000002
//////////////////////////////////////////////////////////////////////
class CQSVEncoder : public CList<LPQSV_ENCODED_FRAME, LPQSV_ENCODED_FRAME>
{
public:
	//encoding/decoding
	mfxStatus				StartEncoding(int inputWidth, int inputHeight, int outputWidth, int outputHeight, int framerate, int mode, int targetUsage, int targetBitrate, int profile, int level, int asyncDepth);
	mfxStatus				Encode(int inputWidth, int inputHeight, int inputDepth, LPBYTE lpImage, DWORDLONG qwTimestamp);
	mfxStatus				StopEncoding();

	//encoder info
	LPCSTR					GetDesc();

	//destruction
	void					Cleanup();

	//header access
	LPBYTE					GetHeader();
	DWORD					GetHeaderSize();

	//encoded frame access
	LPQSV_ENCODED_FRAME		GetFrame();
	void					DestroyFrame(LPQSV_ENCODED_FRAME lpFrame);
	void					DestroyFrames();

	//misc
	int						GetMode();
	static BOOL				IsD3D9Supported();
	static BOOL				IsD3D11Supported();
	static DWORD			GetSuggestedMode();
	static LPCSTR			GetStatusStr(mfxStatus status);

	CQSVEncoder();
	~CQSVEncoder();

protected:
	mfxStatus				LoadFrame(mfxFrameSurface1* lpSurface, int inputWidth, int inputHeight, int inputDepth, LPBYTE lpData, DWORDLONG qwTimestamp);
	mfxStatus				SaveFrame(mfxBitstream* lpBistream);
	CQSVEncoderTask*		GetQueuedTask(CQSVEncoderTask* lpTask);

	int						GetFreeSurfaceIndex(mfxFrameSurface1** pSurfacesPool, mfxU16 nPoolSize);

	void					AddAnnexB(LPBYTE lpSrc, DWORD dwSize, LPQSV_ENCODED_FRAME lpDst);
	void					AddNAL(LPBYTE lpSrc, DWORD dwSize, LPQSV_ENCODED_FRAME lpDst);

	int						m_mode;
	int						m_asyncDepth;
	CList<CQSVEncoderTask*, CQSVEncoderTask*> m_asyncQueue;

	MFXVideoSession			m_session;
    MFXVideoENCODE*			m_lpEnc; 
    MFXVideoVPP*			m_lpVPP;
	mfxFrameAllocator		m_allocator;
	mfxExtVPPDoNotUse		m_extVPPDoNotUse;
    mfxFrameAllocResponse	m_responseVPPIn;
    mfxFrameAllocResponse	m_responseVPPOutEnc;
	mfxU16					m_nSurfacesVPPIn;
	mfxU16					m_nSurfacesVPPOutEnc;
	mfxFrameSurface1**		m_lpSurfacesVPPIn;
	mfxFrameSurface1**		m_lpSurfacesVPPOutEnc;
	mfxExtCodingOption		m_extCodingOption;

	BYTE					m_header[256];
	DWORD					m_headerSize;
	DWORD					m_bitstreamSize;

	char					m_szDesc[MAX_PATH];
};
//////////////////////////////////////////////////////////////////////
