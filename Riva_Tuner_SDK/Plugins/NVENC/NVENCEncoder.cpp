#include "StdAfx.h"
#include "NVENCEncoder.h"
//////////////////////////////////////////////////////////////////////
#include <io.h>
//////////////////////////////////////////////////////////////////////
#define ALIGN(X,N) (X+N-1)&~(N-1)
//////////////////////////////////////////////////////////////////////
#ifdef _WIN64
#define NVENCODEAPI_DLL	"nvEncodeAPI64.dll"
#else
#define NVENCODEAPI_DLL	"nvEncodeAPI.dll"
#endif
//////////////////////////////////////////////////////////////////////
static const GUID NV_CLIENT_KEY_TEST = { 0x0, 0x0, 0x0, { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } };
//////////////////////////////////////////////////////////////////////
typedef NVENCSTATUS(__stdcall * NVENCODEAPICREATEINSTANCE)(NV_ENCODE_API_FUNCTION_LIST *);
//////////////////////////////////////////////////////////////////////
#include "NVENCEncoderTask.h"
//////////////////////////////////////////////////////////////////////
CNVENCEncoder::CNVENCEncoder()
{
	ZeroMemory(&m_header			, sizeof(m_header));
	m_headerSize					= 0;

	m_hNvEncodeAPI_DLL				= NULL;
	m_lpEncodeAPI					= NULL;

	m_cuContext						= NULL;

	m_hEncoder						= NULL;

	m_lpInputSurfaces				= NULL;
	m_lpOutputSurfaces				= NULL;
	m_dwSurfaceCount				= 0;

	m_asyncMode						= 0;
}
//////////////////////////////////////////////////////////////////////
CNVENCEncoder::~CNVENCEncoder()
{
	StopEncoding();
	Cleanup();
	DestroyFrames();
}
//////////////////////////////////////////////////////////////////////
void CNVENCEncoder::DestroyFrame(LPNVENC_ENCODED_FRAME lpFrame)
{
	if (lpFrame) 
	{
		if (lpFrame->lpData)
			delete [] lpFrame->lpData;

		delete lpFrame;
	}
}
//////////////////////////////////////////////////////////////////////
void CNVENCEncoder::DestroyFrames()
{
	POSITION pos = GetHeadPosition();

	while (pos)
		DestroyFrame(GetNext(pos));

	RemoveAll();
}
//////////////////////////////////////////////////////////////////////
int CNVENCEncoder::StartEncoding(int width, int height, int framerate, int targetBitrate, int device, int preset, int profile, int asyncMode, int lowLatency)
{
	//stop previous encoding session and cleanup previous CUDA objects

	StopEncoding();
	Cleanup();

	//save asynchronous mode flag

	m_asyncMode = asyncMode;

	//////////////////////////////////////////////////////////////////////
	//STEP 1	: init encode API
	//////////////////////////////////////////////////////////////////////

	m_hNvEncodeAPI_DLL = LoadLibrary(NVENCODEAPI_DLL);

	if (!m_hNvEncodeAPI_DLL)
		return NVENC_ERR_API_LOADLIBRARY_FAILED;

	NVENCODEAPICREATEINSTANCE nvEncodeAPICreateInstance = (NVENCODEAPICREATEINSTANCE)GetProcAddress(m_hNvEncodeAPI_DLL, "NvEncodeAPICreateInstance");

	if (!nvEncodeAPICreateInstance)
		return NVENC_ERR_API_NVENCODEAPICREATEINSTANCE_NOT_FOUND;

	m_lpEncodeAPI = new NV_ENCODE_API_FUNCTION_LIST;
	ZeroMemory(m_lpEncodeAPI, sizeof(NV_ENCODE_API_FUNCTION_LIST));

	m_lpEncodeAPI->version = NV_ENCODE_API_FUNCTION_LIST_VER;

	NVENCSTATUS nvStatus = nvEncodeAPICreateInstance(m_lpEncodeAPI);

	if (nvStatus != NV_ENC_SUCCESS)
		return NVENC_ERR_API_NVENCODEAPICREATEINSTANCE_FAILED;

	//////////////////////////////////////////////////////////////////////
	//STEP 2	: enumerate CUDA devices
	//////////////////////////////////////////////////////////////////////

	NVENC_CUDA_ENCODER_DEVICE cudaEncoders[MAX_ENCODERS];
	ZeroMemory(&cudaEncoders, sizeof(cudaEncoders));
	
	int cudaEncodersCount = 0;

	//init CUDA

	CUresult cuResult = cuInit(0);

	if (cuResult != CUDA_SUCCESS)
		return NVENC_ERR_CUINIT_FAILED;

	//enmerate CUDA devices and search for Kepler or newer devices with NVENC support
	
    int	deviceCount = 0;

	cuResult = cuDeviceGetCount(&deviceCount);

	if (cuResult != CUDA_SUCCESS)
		return NVENC_ERR_CUDEVICEGETCOUNT_FAILED;

	if (!deviceCount)
		return NVENC_ERR_UNSUPPORTED;

    for (int iDevice=0; iDevice<deviceCount; iDevice++)
    {
		CUdevice cuDevice;

		cuResult = cuDeviceGet(&cuDevice, iDevice);

		if (cuResult != CUDA_SUCCESS)
			return NVENC_ERR_CUDEVICEGET_FAILED;

		int smMajor;
		int smMinor;

		cuResult = cuDeviceComputeCapability(&smMajor, &smMinor, cuDevice);

		if (cuResult != CUDA_SUCCESS)
			return NVENC_ERR_CUDEVICECOMPUTECAPABILITY_FAILED;

		if (((smMajor << 4) + smMinor) >= 0x30)
			//Kepler or newer devices must have at least SM 3.0
		{
			cuResult = cuDeviceGetName(cudaEncoders[cudaEncodersCount].szName, sizeof(cudaEncoders[cudaEncodersCount].szName), cuDevice);

			if (cuResult != CUDA_SUCCESS)
				return NVENC_ERR_CUDEVICEGETNAME_FAILED;

			cudaEncoders[cudaEncodersCount].dwDevice = iDevice;

			cudaEncodersCount++;
		}
    }

	if (!cudaEncodersCount)
		return NVENC_ERR_UNSUPPORTED;

	//////////////////////////////////////////////////////////////////////
	//STEP 3	: create CUDA context on specified device
	//////////////////////////////////////////////////////////////////////

	//validate specified device index

	if (device >= cudaEncodersCount)
		return NVENC_ERR_INVALID_DEVICE;

	//created context and pop the current one

	CUdevice cuDevice;
	cuResult = cuDeviceGet(&cuDevice, cudaEncoders[device].dwDevice);

	if (cuResult != CUDA_SUCCESS)
		return NVENC_ERR_CUDEVICEGET_FAILED;

    cuResult = cuCtxCreate(&m_cuContext, 0, cuDevice);

	if (cuResult != CUDA_SUCCESS)
		return NVENC_ERR_CUCTXCREATE_FAILED;

    CUcontext cuContextCurr;
    cuResult = cuCtxPopCurrent(&cuContextCurr);

	if (cuResult != CUDA_SUCCESS)
		return NVENC_ERR_CUCTXPOPCURRENT_FAILED;

	//////////////////////////////////////////////////////////////////////
	//STEP 4	: open encode session
	//////////////////////////////////////////////////////////////////////

	//init encode session parameters

    NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS encodeSessionParams;
	ZeroMemory(&encodeSessionParams, sizeof(encodeSessionParams));

    GUID clientKey						= NV_CLIENT_KEY_TEST;

	encodeSessionParams.version			= NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
    encodeSessionParams.apiVersion		= NVENCAPI_VERSION;
    encodeSessionParams.clientKeyPtr	= &clientKey;
	encodeSessionParams.device			= (LPVOID)m_cuContext;
	encodeSessionParams.deviceType		= NV_ENC_DEVICE_TYPE_CUDA;

    nvStatus = m_lpEncodeAPI->nvEncOpenEncodeSessionEx(&encodeSessionParams, &m_hEncoder);

	if (nvStatus != NV_ENC_SUCCESS)
		return NVENC_ERR_NVENCOPENENCODESESSIONEX_FAILED;

	//////////////////////////////////////////////////////////////////////
	//STEP 5	: initialize encoder
	//////////////////////////////////////////////////////////////////////

	//get preset

	ZeroMemory(&m_presetConfig, sizeof(m_presetConfig));

	m_presetConfig.version									= NV_ENC_PRESET_CONFIG_VER;
	m_presetConfig.presetCfg.version						= NV_ENC_CONFIG_VER;
		//required for compatibility with 340.xx and higher drivers

	nvStatus = m_lpEncodeAPI->nvEncGetEncodePresetConfig(m_hEncoder, NV_ENC_CODEC_H264_GUID, GetPresetGUID(preset, lowLatency), &m_presetConfig);

	if (nvStatus != NV_ENC_SUCCESS)
		return NVENC_ERR_NVENCGETENCODEPRESETCONFIG_FAILED;

	//prepare init config

	CopyMemory(&m_initEncConfig, &m_presetConfig.presetCfg, sizeof(m_initEncConfig));

	m_initEncConfig.version									= NV_ENC_PRESET_CONFIG_VER;
		//required for compatibility with 337.xx and higher drivers

	m_initEncConfig.gopLength								= framerate;
	m_initEncConfig.encodeCodecConfig.h264Config.idrPeriod	= framerate;
	m_initEncConfig.rcParams.averageBitRate					= targetBitrate * 1000;        
	m_initEncConfig.rcParams.maxBitRate						= targetBitrate * 1000;    
	m_initEncConfig.rcParams.vbvBufferSize					= targetBitrate * 1000;
	m_initEncConfig.encodeCodecConfig.h264Config.enableVFR	= 1;
	m_initEncConfig.frameFieldMode							= NV_ENC_PARAMS_FRAME_FIELD_MODE_FRAME;
	if (profile)
		m_initEncConfig.profileGUID							= GetProfileGUID(profile);

	//prepare init params

	ZeroMemory(&m_initEncParams, sizeof(m_initEncParams));

	m_initEncParams.version									= NV_ENC_INITIALIZE_PARAMS_VER;
	m_initEncParams.encodeGUID								= NV_ENC_CODEC_H264_GUID;
	m_initEncParams.presetGUID								= GetPresetGUID(preset, lowLatency);
	m_initEncParams.encodeWidth								= width;
	m_initEncParams.encodeHeight							= height;
	m_initEncParams.darWidth								= width;
	m_initEncParams.darHeight								= height;
	m_initEncParams.frameRateNum							= framerate;
	m_initEncParams.frameRateDen							= 1;
	m_initEncParams.enableEncodeAsync						= m_asyncMode ? 1 : 0;
	m_initEncParams.enablePTD								= 1;
	m_initEncParams.encodeConfig							= &m_initEncConfig;
	m_initEncParams.maxEncodeWidth							= width;
	m_initEncParams.maxEncodeHeight							= height;

	nvStatus = m_lpEncodeAPI->nvEncInitializeEncoder(m_hEncoder, &m_initEncParams);

	if (nvStatus != NV_ENC_SUCCESS)
		return NVENC_ERR_NVENCINITIALIZEENCODER_FAILED;

	//////////////////////////////////////////////////////////////////////
	//STEP 6	: allocate surfaces
	//////////////////////////////////////////////////////////////////////

	//get surface count depending on maximum macroblocks number

	m_dwSurfaceCount	= ((ALIGN(width, 16) / 16) * (ALIGN(height, 16) / 16) >= 8160) ? 16 : 32;

	//allocate input and output surface descriptors

	m_lpInputSurfaces	= new NVENC_ENCODER_INPUT_SURFACE[m_dwSurfaceCount];    
	m_lpOutputSurfaces	= new NVENC_ENCODER_OUTPUT_SURFACE[m_dwSurfaceCount];

	ZeroMemory(m_lpInputSurfaces, sizeof(NVENC_ENCODER_INPUT_SURFACE) * m_dwSurfaceCount);
	ZeroMemory(m_lpOutputSurfaces, sizeof(NVENC_ENCODER_OUTPUT_SURFACE) * m_dwSurfaceCount);

	//allocate surfaces

    for (DWORD dwSurface=0; dwSurface<m_dwSurfaceCount; dwSurface++)    
	{        
		//input surfaces

		NV_ENC_CREATE_INPUT_BUFFER createInputBuffer;
		ZeroMemory(&createInputBuffer, sizeof(createInputBuffer));

		createInputBuffer.version						= NV_ENC_CREATE_INPUT_BUFFER_VER;        
		createInputBuffer.width							= ALIGN(width	, 32);        
		createInputBuffer.height						= ALIGN(height	, 32);        
		createInputBuffer.memoryHeap					= NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;        
		createInputBuffer.bufferFmt						= NV_ENC_BUFFER_FORMAT_NV12_PL;      

		nvStatus = m_lpEncodeAPI->nvEncCreateInputBuffer(m_hEncoder, &createInputBuffer);        

		if (nvStatus != NV_ENC_SUCCESS)       
			return NVENC_ERR_NVENCCREATEINPUTBUFFER_FAILED;

		m_lpInputSurfaces[dwSurface].lpInputBuffer		= createInputBuffer.inputBuffer;        
		m_lpInputSurfaces[dwSurface].dwWidth			= createInputBuffer.width;        
		m_lpInputSurfaces[dwSurface].dwHeight			= createInputBuffer.height;        
		m_lpInputSurfaces[dwSurface].bLocked			= FALSE;        

		//output surfaces

		NV_ENC_CREATE_BITSTREAM_BUFFER createBitstreamBuffer;        
		ZeroMemory(&createBitstreamBuffer, sizeof(createBitstreamBuffer));

		createBitstreamBuffer.version					= NV_ENC_CREATE_BITSTREAM_BUFFER_VER;        
		createBitstreamBuffer.size						= 1024 * 1024;        
		createBitstreamBuffer.memoryHeap				= NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;        

		nvStatus = m_lpEncodeAPI->nvEncCreateBitstreamBuffer(m_hEncoder, &createBitstreamBuffer);        

		if (nvStatus != NV_ENC_SUCCESS)        
			return NVENC_ERR_NVENCCREATEBITSTREAMBUFFER_FAILED;
		
		m_lpOutputSurfaces[dwSurface].lpBitstreamBuffer	= createBitstreamBuffer.bitstreamBuffer;        
		m_lpOutputSurfaces[dwSurface].dwSize			= createBitstreamBuffer.size;        
		m_lpOutputSurfaces[dwSurface].bLocked			= FALSE;    

		//asynchronous mode events

		if (m_asyncMode)
		{
			m_lpOutputSurfaces[dwSurface].hEvent		= CreateEvent(NULL, FALSE, FALSE, NULL);

	        NV_ENC_EVENT_PARAMS eventParams;
			ZeroMemory(&eventParams, sizeof(eventParams));

			eventParams.version							= NV_ENC_EVENT_PARAMS_VER;
			eventParams.completionEvent					= m_lpOutputSurfaces[dwSurface].hEvent;

	        nvStatus = m_lpEncodeAPI->nvEncRegisterAsyncEvent(m_hEncoder, &eventParams);

			if (nvStatus != NV_ENC_SUCCESS)        
				return NVENC_ERR_NVENCREGISTERASYNCEVENT_FAILED;
		}
	}

	//retreive SPS/PPS headers and init header

	BYTE			buffer[200];    
	unsigned int	bufferSize;

	NV_ENC_SEQUENCE_PARAM_PAYLOAD payload;    
	ZeroMemory(&payload, sizeof(payload));

	payload.version										= NV_ENC_SEQUENCE_PARAM_PAYLOAD_VER;    
	payload.spsppsBuffer								= buffer;    
	payload.inBufferSize								= sizeof(buffer);    
	payload.outSPSPPSPayloadSize						= &bufferSize;    

	nvStatus = m_lpEncodeAPI->nvEncGetSequenceParams(m_hEncoder, &payload);

	if (nvStatus != NV_ENC_SUCCESS)
		return NVENC_ERR_NVENCGETSEQUENCEPARAMS_FAILED;

	LPBYTE SPS = buffer;
	LPBYTE PPS = buffer + 4;

	while (*(LPDWORD)PPS != 0x01000000)
	{
		PPS++;

		if (PPS >= buffer + bufferSize - 4)
			return NVENC_ERR_INVALID_SEQUENCE_PARAMS;
	}

	DWORD SPSLen = (DWORD)(PPS - SPS - 4);
	DWORD PPSLen = (DWORD)(buffer + bufferSize - PPS - 4);

	m_headerSize			= 5 + 1 + 2 + SPSLen + 1 + 2 + PPSLen;

	m_header[0]				= 1; 
	m_header[1]				= SPS[4 + 1];		// profile
	m_header[2]				= SPS[4 + 2];		// profile compat
	m_header[3]				= SPS[4 + 3];		// level
	m_header[4]				= 0xFF;				// nalu size length is four bytes 

	m_header[5]				= 0xE1;				// one sps 
	m_header[6]				= (BYTE)(SPSLen>>8); 
	m_header[7]				= (BYTE)(SPSLen); 
	memcpy(m_header + 8, SPS + 4, SPSLen); 

	m_header[SPSLen + 8]	= 1;				// one pps 
	m_header[SPSLen + 9]	= (BYTE)(PPSLen>>8); 
	m_header[SPSLen + 10]	= (BYTE)(PPSLen); 
	memcpy(m_header + SPSLen + 11, PPS + 4, PPSLen); 

	//init description

	sprintf_s(m_szDesc, "NVIDIA NVENC H.264 accelerated via CUDA on device %d (%s), %s mode", cudaEncoders[device].dwDevice, cudaEncoders[device].szName, m_asyncMode ? "async" : "sync");

	return NVENC_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
int CNVENCEncoder::GetFreeInputSurfaceIndex()
{
	for (int nSurface=0; nSurface<(int)m_dwSurfaceCount; nSurface++)
	{
		if (!m_lpInputSurfaces[nSurface].bLocked)
			return nSurface;
	}

	return -1;
}
//////////////////////////////////////////////////////////////////////
int CNVENCEncoder::GetFreeOutputSurfaceIndex()
{
	for (int nSurface=0; nSurface<(int)m_dwSurfaceCount; nSurface++)
	{
		if (!m_lpOutputSurfaces[nSurface].bLocked)
			return nSurface;
	}

	return -1;
}
//////////////////////////////////////////////////////////////////////
int CNVENCEncoder::Encode(int width, int height, LPBYTE lpImage, DWORDLONG qwTimestamp)
{
	NVENCSTATUS nvStatus;

	//find free input surface

	int nInputSurface = GetFreeInputSurfaceIndex();

	if (nInputSurface < 0)
		//if there is no free input surface then we'll wait for it during 25ms before throwing NVENC_ERR_BUSY
	{
		for (int iRetry=0; iRetry<25; iRetry++)
		{
			CNVENCEncoderTask* lpQueuedTask = GetQueuedTask(NULL);
				//try to flush the task from the queue

			if (lpQueuedTask)
			{
				SaveTask(lpQueuedTask);
					//save task bitstream
				lpQueuedTask->UnlockSurfaces();
					//unlock the surfaces associated with it
				delete lpQueuedTask;
					//delete task

				nInputSurface = GetFreeInputSurfaceIndex();
					//we've just freed input and output surfaces so we can try to find free input surface again

				break;
			}

			Sleep(1);
		}
	}

	if (nInputSurface < 0)
		return NVENC_ERR_BUSY;

	//find free output surface

	int nOutputSurface = GetFreeOutputSurfaceIndex();

	if (nOutputSurface < 0)
		return NVENC_ERR_BUSY;

	//lock input surface

	NV_ENC_LOCK_INPUT_BUFFER lockBufferParams;        
	ZeroMemory(&lockBufferParams, sizeof(lockBufferParams));

	lockBufferParams.version				= NV_ENC_LOCK_INPUT_BUFFER_VER;        
	lockBufferParams.inputBuffer			= m_lpInputSurfaces[nInputSurface].lpInputBuffer;        
	
	nvStatus = m_lpEncodeAPI->nvEncLockInputBuffer(m_hEncoder, &lockBufferParams);       
	
	if (nvStatus != NV_ENC_SUCCESS)        
		return NVENC_ERR_NVENCLOCKINPUTBUFFER_FAILED;

	//load frame into locked input surface

	LoadFrame(&m_lpInputSurfaces[nInputSurface], &lockBufferParams, width, height, lpImage);

	//unlock input surface

	nvStatus = m_lpEncodeAPI->nvEncUnlockInputBuffer(m_hEncoder, lockBufferParams.inputBuffer);

	if (nvStatus != NV_ENC_SUCCESS)        
		return NVENC_ERR_NVENCUNLOCKINPUTBUFFER_FAILED;

	//prepare picture parameters for encoder

	NV_ENC_PIC_PARAMS picParams;
	ZeroMemory(&picParams, sizeof(picParams));
		
	picParams.version										= NV_ENC_PIC_PARAMS_VER;
	picParams.inputBuffer									= m_lpInputSurfaces[nInputSurface].lpInputBuffer;        
	picParams.bufferFmt										= NV_ENC_BUFFER_FORMAT_NV12_PL;        
	picParams.inputWidth									= width;        
	picParams.inputHeight									= height;        
	picParams.outputBitstream								= m_lpOutputSurfaces[nOutputSurface].lpBitstreamBuffer;        
	picParams.completionEvent								= m_asyncMode ? m_lpOutputSurfaces[nOutputSurface].hEvent : 0;        
	picParams.pictureStruct									= NV_ENC_PIC_STRUCT_FRAME;
	picParams.encodePicFlags								= 0;
	picParams.inputTimeStamp								= qwTimestamp; 
	picParams.inputDuration									= 0;  

	nvStatus = m_lpEncodeAPI->nvEncEncodePicture(m_hEncoder, &picParams);

	if ((nvStatus != NV_ENC_SUCCESS				) && 
		(nvStatus == NV_ENC_ERR_NEED_MORE_INPUT	))        
		return NVENC_ERR_NVENCENCODEPICTURE_FAILED;

	if (nvStatus == NV_ENC_ERR_NEED_MORE_INPUT)
		return NVENC_ERR_NONE;

	// allocate new task
	CNVENCEncoderTask* lpTask = new CNVENCEncoderTask(&m_lpInputSurfaces[nInputSurface], &m_lpOutputSurfaces[nOutputSurface]);

	//lock the surfaces associated with task
	lpTask->LockSurfaces();

	//add task to the queue and get queued task
	CNVENCEncoderTask* lpQueuedTask = GetQueuedTask(lpTask);
	if (!lpQueuedTask)
		return NVENC_ERR_NONE;

	//save bitstream for retreived queued task
	int nResult = SaveTask(lpQueuedTask);

	//unlock surfaces associated with retreived queued task
	lpQueuedTask->UnlockSurfaces();

	//delete retreived queued task
	delete lpQueuedTask;

	return nResult;
}
//////////////////////////////////////////////////////////////////////
int CNVENCEncoder::SaveTask(CNVENCEncoderTask* lpTask)
{
	NVENCSTATUS nvStatus;

	NV_ENC_LOCK_BITSTREAM lockBitstreamParams;   
	ZeroMemory(&lockBitstreamParams, sizeof(lockBitstreamParams));

	lockBitstreamParams.version								= NV_ENC_LOCK_BITSTREAM_VER;    
	lockBitstreamParams.doNotWait							= m_asyncMode ? 1 : 0;    
	lockBitstreamParams.outputBitstream						= lpTask->GetOutputSurface()->lpBitstreamBuffer;    
	
	nvStatus = m_lpEncodeAPI->nvEncLockBitstream(m_hEncoder, &lockBitstreamParams);    
	
	if (nvStatus != NV_ENC_SUCCESS)
		return NVENC_ERR_NVENCLOCKBITSTREAM_FAILED;

	SaveFrame(&lockBitstreamParams);

	nvStatus = m_lpEncodeAPI->nvEncUnlockBitstream(m_hEncoder, lpTask->GetOutputSurface()->lpBitstreamBuffer);

	if (nvStatus != NV_ENC_SUCCESS)
		return NVENC_ERR_NVENCUNLOCKBITSTREAM_FAILED;

	return NVENC_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
int CNVENCEncoder::StopEncoding()
{
	//destroy encode session

	if (m_lpEncodeAPI)
	{
		//flush the encoder

		if (m_hEncoder)
		{
			NV_ENC_PIC_PARAMS picParams;
			ZeroMemory(&picParams, sizeof(picParams));
		
			picParams.version			= NV_ENC_PIC_PARAMS_VER;
			picParams.encodePicFlags	= NV_ENC_PIC_FLAG_EOS;

			if (m_lpInputSurfaces && m_lpOutputSurfaces)
				//flush the encoder only if it is initialized properly and the surfaces are allocated, otherwise
				//it will crash
				m_lpEncodeAPI->nvEncEncodePicture(m_hEncoder, &picParams);
		}

		//flush async queue

		CNVENCEncoderTask* lpQueuedTask = GetQueuedTask(NULL);

		while (lpQueuedTask)
		{
			SaveTask(lpQueuedTask);

			lpQueuedTask->UnlockSurfaces();

			delete lpQueuedTask;

			lpQueuedTask = GetQueuedTask(NULL);
		}

		//destroy surfaces

		for (DWORD dwSurface=0; dwSurface<m_dwSurfaceCount; dwSurface++)
		{            
			//destroy input surfaces

			if (m_lpInputSurfaces)
			{
				if (m_lpInputSurfaces[dwSurface].lpInputBuffer)
				{
					m_lpEncodeAPI->nvEncDestroyInputBuffer(m_hEncoder, m_lpInputSurfaces[dwSurface].lpInputBuffer);            

					m_lpInputSurfaces[dwSurface].lpInputBuffer = NULL;
				}
			}

			if (m_lpOutputSurfaces)
			{
				//destroy output surfaces

				if (m_lpOutputSurfaces[dwSurface].lpBitstreamBuffer)
				{
					m_lpEncodeAPI->nvEncDestroyBitstreamBuffer(m_hEncoder, m_lpOutputSurfaces[dwSurface].lpBitstreamBuffer);        

					m_lpOutputSurfaces[dwSurface].lpBitstreamBuffer = NULL;
				}

				//destroy events

				if (m_lpOutputSurfaces[dwSurface].hEvent)
				{
					NV_ENC_EVENT_PARAMS eventParams;
					ZeroMemory(&eventParams, sizeof(eventParams));

					eventParams.version			= NV_ENC_EVENT_PARAMS_VER;
					eventParams.completionEvent	= m_lpOutputSurfaces[dwSurface].hEvent;

					m_lpEncodeAPI->nvEncUnregisterAsyncEvent(m_hEncoder, &eventParams);
		
					CloseHandle(m_lpOutputSurfaces[dwSurface].hEvent);

					m_lpOutputSurfaces[dwSurface].hEvent = NULL;
				}
			}
		}

		m_dwSurfaceCount = 0;

		if (m_hEncoder)
			m_lpEncodeAPI->nvEncDestroyEncoder(m_hEncoder);

		m_hEncoder = NULL;
	}

	//delete input surface descriptors

	if (m_lpInputSurfaces)
		delete [] m_lpInputSurfaces;
	m_lpInputSurfaces = NULL;

	//delete output surface descriptors

	if (m_lpOutputSurfaces)
		delete [] m_lpOutputSurfaces;
	m_lpOutputSurfaces = NULL;

	//destroy CUDA context

	if (m_cuContext)
	{
		CUresult cuResult = cuCtxDestroy(m_cuContext);

		m_cuContext = NULL;
	}

	//destroy encode API

	if (m_lpEncodeAPI)
		delete m_lpEncodeAPI;
	m_lpEncodeAPI = NULL;

	if (m_hNvEncodeAPI_DLL)
		FreeLibrary(m_hNvEncodeAPI_DLL);
	m_hNvEncodeAPI_DLL = NULL;

	Cleanup();

	return NVENC_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
void CNVENCEncoder::Cleanup()
{
}
//////////////////////////////////////////////////////////////////////
LPBYTE CNVENCEncoder::GetHeader()
{
	return m_header;
}
//////////////////////////////////////////////////////////////////////
DWORD CNVENCEncoder::GetHeaderSize()
{
	return m_headerSize;
}
//////////////////////////////////////////////////////////////////////
void CNVENCEncoder::AddAnnexB(LPBYTE lpSrc, DWORD dwSize, LPNVENC_ENCODED_FRAME lpFrame)
{
	if (dwSize >= 4)
	{
		lpFrame->lpData		= new BYTE[dwSize + 32];
		lpFrame->dwSize		= dwSize + 32;

		DWORD	dwSrcPos	= 0;
		DWORD	dwNALPos	= 0;
		DWORD	dwDstPos	= 0;

		while (dwSrcPos <= dwSize - 4)
		{	
			DWORD dwPeek = *(LPDWORD)(lpSrc + dwSrcPos);

			if ((dwPeek	== 0x01000000) || ((dwPeek & 0xFFFFFF)	== 0x00010000))
			{
				if (dwPeek == 0x01000000)
					dwSrcPos++;

				if (dwNALPos)
					AddNAL(lpSrc + dwNALPos, dwSrcPos - dwNALPos, lpFrame);

				dwSrcPos += 3;	
				dwNALPos = dwSrcPos;
			}
			else
				dwSrcPos++;
		}

		if (dwNALPos)
			AddNAL(lpSrc + dwNALPos, dwSize - dwNALPos, lpFrame);
	}
}
//////////////////////////////////////////////////////////////////////
void CNVENCEncoder::AddNAL(LPBYTE lpSrc, DWORD dwSize, LPNVENC_ENCODED_FRAME lpFrame)
{
	BYTE NAL = lpSrc[0] & 0x1F;

	if (lpFrame->dwPos + dwSize + 4 > lpFrame->dwSize)
	{
		LPBYTE lpData = new BYTE[lpFrame->dwPos + dwSize + 4];

		if (lpFrame->lpData)
		{
			if (lpFrame->dwSize)
				CopyMemory(lpData, lpFrame->lpData, lpFrame->dwSize);

			delete [] lpFrame->lpData;
		}

		lpFrame->lpData = lpData;
		lpFrame->dwSize = lpFrame->dwPos + dwSize;
	}

	lpFrame->lpData[lpFrame->dwPos    ] = (BYTE)(dwSize>>24);
	lpFrame->lpData[lpFrame->dwPos + 1] = (BYTE)(dwSize>>16);
	lpFrame->lpData[lpFrame->dwPos + 2] = (BYTE)(dwSize>>8);
	lpFrame->lpData[lpFrame->dwPos + 3] = (BYTE)(dwSize);
	lpFrame->dwPos += 4;

	CopyMemory(lpFrame->lpData + lpFrame->dwPos, lpSrc, dwSize);	
	lpFrame->dwPos += dwSize;
}
//////////////////////////////////////////////////////////////////////
LPNVENC_ENCODED_FRAME CNVENCEncoder::GetFrame()
{
	POSITION pos = GetHeadPosition();

	if (pos)
	{
		LPNVENC_ENCODED_FRAME lpFrame = GetAt(pos);

		RemoveAt(pos);

		return lpFrame;
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////
LPCSTR CNVENCEncoder::GetDesc()
{
	return m_szDesc;
}
//////////////////////////////////////////////////////////////////////
void CNVENCEncoder::SaveFrame(NV_ENC_LOCK_BITSTREAM* lpLockBitstreamParams)
{
	LPNVENC_ENCODED_FRAME lpFrame	= (LPNVENC_ENCODED_FRAME) new BYTE[sizeof(NVENC_ENCODED_FRAME)];

	ZeroMemory(lpFrame, sizeof(NVENC_ENCODED_FRAME));

	lpFrame->dwFrameType		= lpLockBitstreamParams->pictureType;
	lpFrame->qwTimestamp		= lpLockBitstreamParams->outputTimeStamp;

	AddAnnexB((LPBYTE)lpLockBitstreamParams->bitstreamBufferPtr, lpLockBitstreamParams->bitstreamSizeInBytes, lpFrame);

	if (lpFrame->dwPos)
	{
		lpFrame->dwSize = lpFrame->dwPos;
		lpFrame->dwPos	= 0;

		AddTail(lpFrame);
	}
	else
		DestroyFrame(lpFrame);
}
//////////////////////////////////////////////////////////////////////
void CNVENCEncoder::LoadFrame(LPNVENC_ENCODER_INPUT_SURFACE lpInputSurface, NV_ENC_LOCK_INPUT_BUFFER* lpLockBufferParams, int width, int height, LPBYTE lpSrc)
{
	LPBYTE lpDst	= (LPBYTE)lpLockBufferParams->bufferDataPtr;
	LPBYTE lpDstUV	= (LPBYTE)lpLockBufferParams->bufferDataPtr + lpLockBufferParams->pitch * lpInputSurface->dwHeight;

	DWORD dwSrcPitch = width;
	DWORD dwDstPitch = lpLockBufferParams->pitch;

	for (int y=0; y<height; y++)
		CopyMemory(lpDst + y * dwDstPitch, lpSrc + y * dwSrcPitch, width);

	LPBYTE lpSrcUV = lpSrc + dwSrcPitch * height;

	for (int y=0; y<height/2; y++)
		CopyMemory(lpDstUV + y * dwDstPitch, lpSrcUV + y * dwSrcPitch, width);
}
//////////////////////////////////////////////////////////////////////
LPCSTR CNVENCEncoder::GetStatusStr(int status)
{
	switch (status) 
	{
	case NVENC_ERR_NONE:
		return "NVENC_ERR_NONE";
	case NVENC_ERR_UNKNOWN:
		return "NVENC_ERR_UNKNOWN";
	case NVENC_ERR_NULL_PTR:
		return "NVENC_ERR_NULL_PTR";
	case NVENC_ERR_UNSUPPORTED:
		return "NVENC_ERR_UNSUPPORTED";
	case NVENC_ERR_INVALID_DEVICE:
		return "NVENC_ERR_INVALID_DEVICE";
	case NVENC_ERR_API_LOADLIBRARY_FAILED:
		return "NVENC_ERR_API_LOADLIBRARY_FAILED";
	case NVENC_ERR_API_NVENCODEAPICREATEINSTANCE_NOT_FOUND:
		return "NVENC_ERR_API_NVENCODEAPICREATEINSTANCE_NOT_FOUND";
	case NVENC_ERR_API_NVENCODEAPICREATEINSTANCE_FAILED:
		return "NVENC_ERR_API_NVENCODEAPICREATEINSTANCE_FAILED";
	case NVENC_ERR_CUINIT_FAILED:
		return "NVENC_ERR_CUINIT_FAILED";
	case NVENC_ERR_CUDEVICEGETCOUNT_FAILED:
		return "NVENC_ERR_CUDEVICEGETCOUNT_FAILED";
	case NVENC_ERR_CUDEVICEGET_FAILED:
		return "NVENC_ERR_CUDEVICEGET_FAILED";
	case NVENC_ERR_CUDEVICECOMPUTECAPABILITY_FAILED:
		return "NVENC_ERR_CUDEVICECOMPUTECAPABILITY_FAILED";
	case NVENC_ERR_CUDEVICEGETNAME_FAILED:
		return "NVENC_ERR_CUDEVICEGETNAME_FAILED";
	case NVENC_ERR_CUCTXCREATE_FAILED:
		return "NVENC_ERR_CUCTXCREATE_FAILED";
	case NVENC_ERR_CUCTXPOPCURRENT_FAILED:
		return "NVENC_ERR_CUCTXPOPCURRENT_FAILED";
	case NVENC_ERR_NVENCOPENENCODESESSIONEX_FAILED:
		return "NVENC_ERR_NVENCOPENENCODESESSIONEX_FAILED";
	case NVENC_ERR_NVENCGETENCODEPRESETCONFIG_FAILED:
		return "NVENC_ERR_NVENCGETENCODEPRESETCONFIG_FAILED";
	case NVENC_ERR_NVENCINITIALIZEENCODER_FAILED:
		return "NVENC_ERR_NVENCINITIALIZEENCODER_FAILED";
	case NVENC_ERR_NVENCGETSEQUENCEPARAMS_FAILED:
		return "NVENC_ERR_NVENCGETSEQUENCEPARAMS_FAILED";
	case NVENC_ERR_INVALID_SEQUENCE_PARAMS:
		return "NVENC_ERR_INVALID_SEQUENCE_PARAMS";
	case NVENC_ERR_NVENCCREATEINPUTBUFFER_FAILED:
		return "NVENC_ERR_NVENCCREATEINPUTBUFFER_FAILED";
	case NVENC_ERR_NVENCCREATEBITSTREAMBUFFER_FAILED:
		return "NVENC_ERR_NVENCCREATEBITSTREAMBUFFER_FAILED";
	case NVENC_ERR_NVENCREGISTERASYNCEVENT_FAILED:
		return "NVENC_ERR_NVENCREGISTERASYNCEVENT_FAILED";
	case NVENC_ERR_NVENCLOCKINPUTBUFFER_FAILED:
		return "NVENC_ERR_NVENCLOCKINPUTBUFFER_FAILED";
	case NVENC_ERR_NVENCUNLOCKINPUTBUFFER_FAILED:
		return "NVENC_ERR_NVENCUNLOCKINPUTBUFFER_FAILED";
	case NVENC_ERR_BUSY:
		return "NVENC_ERR_BUSY";
	case NVENC_ERR_NVENCENCODEPICTURE_FAILED:
		return "NVENC_ERR_NVENCENCODEPICTURE_FAILED";
	case NVENC_ERR_NVENCLOCKBITSTREAM_FAILED:
		return "NVENC_ERR_NVENCLOCKBITSTREAM_FAILED";
	case NVENC_ERR_NVENCUNLOCKBITSTREAM_FAILED:
		return "NVENC_ERR_NVENCUNLOCKBITSTREAM_FAILED";
	}

	return "UNKNOWN";
}
//////////////////////////////////////////////////////////////////////
GUID CNVENCEncoder::GetPresetGUID(int preset, int lowLatency)
{
	if (lowLatency)
	{
		switch (preset)
		{
		case 0:
			return NV_ENC_PRESET_LOW_LATENCY_HP_GUID;
		case 1:
			return NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID;
		case 2:
			return NV_ENC_PRESET_LOW_LATENCY_HQ_GUID;
		}
	}
	else
	{
		switch (preset)
		{
		case 0:
			return NV_ENC_PRESET_HP_GUID;
		case 1:
			return NV_ENC_PRESET_DEFAULT_GUID;
		case 2:
			return NV_ENC_PRESET_HQ_GUID;
		}
	}

    return NV_ENC_PRESET_DEFAULT_GUID;
}
//////////////////////////////////////////////////////////////////////
GUID CNVENCEncoder::GetProfileGUID(int profile)
{
	switch (profile)
	{
	case 0:
		return NV_ENC_CODEC_PROFILE_AUTOSELECT_GUID;
	case 66:
		return NV_ENC_H264_PROFILE_BASELINE_GUID;
	case 77:
		return NV_ENC_H264_PROFILE_MAIN_GUID;
	case 100:
		return NV_ENC_H264_PROFILE_HIGH_GUID;
	}

	return NV_ENC_CODEC_PROFILE_AUTOSELECT_GUID;
}
//////////////////////////////////////////////////////////////////////
CNVENCEncoderTask* CNVENCEncoder::GetQueuedTask(CNVENCEncoderTask* lpTask)
{
	if (m_asyncMode)
	{
		if (lpTask)
			m_asyncQueue.AddTail(lpTask);

		if (m_asyncQueue.GetCount())
		{
			if (m_asyncQueue.GetHead()->IsReady())
				return m_asyncQueue.RemoveHead();
		}

		return NULL;
	}

	return lpTask;
}
//////////////////////////////////////////////////////////////////////

