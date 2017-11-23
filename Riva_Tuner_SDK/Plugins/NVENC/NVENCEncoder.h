#pragma once
//////////////////////////////////////////////////////////////////////
#include <afxtempl.h>
//////////////////////////////////////////////////////////////////////
#include <cuda.h>

#include "NvEncodeAPI.h"
//////////////////////////////////////////////////////////////////////
typedef struct NVENC_ENCODED_FRAME
{
	DWORD							dwFrameType;
	LONGLONG						qwTimestamp;

	LPBYTE							lpData;
	DWORD							dwSize;
	DWORD							dwPos;
} NVENC_ENCODED_FRAME, *LPNVENC_ENCODED_FRAME;
//////////////////////////////////////////////////////////////////////
#define MAX_ENCODERS												16
//////////////////////////////////////////////////////////////////////
typedef struct NVENC_CUDA_ENCODER_DEVICE
{
	char							szName[MAX_PATH];
	unsigned int					dwDevice;
} NVENC_CUDA_ENCODER_DEVICE, *LPNVENC_CUDA_ENCODER_DEVICE;
//////////////////////////////////////////////////////////////////////
typedef struct NVENC_ENCODER_INPUT_SURFACE
{    
	NV_ENC_INPUT_PTR				lpInputBuffer;    
	DWORD							dwWidth;    
	DWORD							dwHeight;    
	BOOL							bLocked;    
} NVENC_ENCODER_INPUT_SURFACE, *LPNVENC_ENCODER_INPUT_SURFACE;
//////////////////////////////////////////////////////////////////////
typedef struct NVENC_ENCODER_OUTPUT_SURFACE
{    
	NV_ENC_OUTPUT_PTR				lpBitstreamBuffer;    
	DWORD							dwSize; 
	BOOL							bLocked;
	HANDLE							hEvent;
} NVENC_ENCODER_OUTPUT_SURFACE, *LPNVENC_ENCODER_OUTPUT_SURFACE;
//////////////////////////////////////////////////////////////////////
#define NVENC_ERR_NONE												0
#define NVENC_ERR_UNKNOWN											-1
#define NVENC_ERR_NULL_PTR											-2
#define NVENC_ERR_UNSUPPORTED										-3
#define NVENC_ERR_INVALID_DEVICE									-4
#define NVENC_ERR_API_LOADLIBRARY_FAILED							-5
#define NVENC_ERR_API_NVENCODEAPICREATEINSTANCE_NOT_FOUND			-6
#define NVENC_ERR_API_NVENCODEAPICREATEINSTANCE_FAILED				-7
#define NVENC_ERR_CUINIT_FAILED										-8
#define NVENC_ERR_CUDEVICEGETCOUNT_FAILED							-9
#define NVENC_ERR_CUDEVICEGET_FAILED								-10
#define NVENC_ERR_CUDEVICECOMPUTECAPABILITY_FAILED					-11
#define NVENC_ERR_CUDEVICEGETNAME_FAILED							-12
#define NVENC_ERR_CUCTXCREATE_FAILED								-13
#define NVENC_ERR_CUCTXPOPCURRENT_FAILED							-14
#define NVENC_ERR_NVENCOPENENCODESESSIONEX_FAILED					-15
#define NVENC_ERR_NVENCGETENCODEPRESETCONFIG_FAILED					-16
#define NVENC_ERR_NVENCINITIALIZEENCODER_FAILED						-17
#define NVENC_ERR_NVENCGETSEQUENCEPARAMS_FAILED						-18
#define NVENC_ERR_INVALID_SEQUENCE_PARAMS							-19
#define NVENC_ERR_NVENCCREATEINPUTBUFFER_FAILED						-20
#define NVENC_ERR_NVENCCREATEBITSTREAMBUFFER_FAILED					-21
#define NVENC_ERR_NVENCREGISTERASYNCEVENT_FAILED					-22		
#define NVENC_ERR_NVENCLOCKINPUTBUFFER_FAILED						-23
#define NVENC_ERR_NVENCUNLOCKINPUTBUFFER_FAILED						-24
#define NVENC_ERR_BUSY												-25
#define NVENC_ERR_NVENCENCODEPICTURE_FAILED							-26
#define NVENC_ERR_NVENCLOCKBITSTREAM_FAILED							-27
#define NVENC_ERR_NVENCUNLOCKBITSTREAM_FAILED						-28
//////////////////////////////////////////////////////////////////////
class CNVENCEncoderTask;
class CNVENCEncoder : public CList<LPNVENC_ENCODED_FRAME, LPNVENC_ENCODED_FRAME>
{
public:
	//encoding/decoding
	int						StartEncoding(int width, int height, int framerate, int targetBitrate, int device, int preset, int profile, int asyncMode, int lowLatency);
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
	LPNVENC_ENCODED_FRAME	GetFrame();
	void					DestroyFrame(LPNVENC_ENCODED_FRAME lpFrame);
	void					DestroyFrames();

	//misc
	static LPCSTR			GetStatusStr(int status);

	CNVENCEncoder();
	~CNVENCEncoder();

protected:
	void							SaveFrame(NV_ENC_LOCK_BITSTREAM* lpLockBitstreamParams);
	void							LoadFrame(LPNVENC_ENCODER_INPUT_SURFACE lpInputSurface, NV_ENC_LOCK_INPUT_BUFFER* lpLockBufferParams, int width, int height, LPBYTE lpSrc);
	CNVENCEncoderTask*				GetQueuedTask(CNVENCEncoderTask* lpTask);
	int								SaveTask(CNVENCEncoderTask* lpTask);


	int								GetFreeInputSurfaceIndex();
	int								GetFreeOutputSurfaceIndex();

	GUID							GetPresetGUID(int preset, int lowLatency);
	GUID							GetProfileGUID(int profile);

	void							AddAnnexB(LPBYTE lpSrc, DWORD dwSize, LPNVENC_ENCODED_FRAME lpDst);
	void							AddNAL(LPBYTE lpSrc, DWORD dwSize, LPNVENC_ENCODED_FRAME lpDst);

	BYTE							m_header[256];
	DWORD							m_headerSize;

	char							m_szDesc[MAX_PATH];

	HINSTANCE						m_hNvEncodeAPI_DLL;
	NV_ENCODE_API_FUNCTION_LIST*	m_lpEncodeAPI;

	CUcontext						m_cuContext;

	LPVOID							m_hEncoder;

	NV_ENC_PRESET_CONFIG			m_presetConfig;
	NV_ENC_CONFIG					m_initEncConfig;
	NV_ENC_INITIALIZE_PARAMS		m_initEncParams;

	DWORD							m_dwSurfaceCount;    
	LPNVENC_ENCODER_INPUT_SURFACE	m_lpInputSurfaces;   
	LPNVENC_ENCODER_OUTPUT_SURFACE	m_lpOutputSurfaces;

	int								m_asyncMode;
	CList<CNVENCEncoderTask*, CNVENCEncoderTask*> m_asyncQueue;
};
//////////////////////////////////////////////////////////////////////
