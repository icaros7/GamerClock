#include "StdAfx.h"
#include "QSVEncoder.h"
//////////////////////////////////////////////////////////////////////
#include <io.h>

#include "QSVEncoderSw.h"
#include "QSVEncoderD3D9.h"
#include "QSVEncoderD3D11.h"
//////////////////////////////////////////////////////////////////////
#define ALIGN16(X)				(((X + 15) >> 4) << 4)
#define ALIGN32(X)				(((mfxU32)((X)+31)) & ( ~ (mfxU32)31))
//////////////////////////////////////////////////////////////////////
CQSVEncoder::CQSVEncoder()
{
	m_mode							= QSV_ENCODER_MODE_SOFTWARE;
	m_asyncDepth					= 0;

	m_lpEnc							= NULL;
	m_lpVPP							= NULL;
	m_nSurfacesVPPIn				= 0;
	m_nSurfacesVPPOutEnc			= 0;
	m_lpSurfacesVPPIn				= NULL;
	m_lpSurfacesVPPOutEnc			= NULL;

	ZeroMemory(&m_allocator			, sizeof(m_allocator));
	ZeroMemory(&m_responseVPPIn		, sizeof(m_responseVPPIn));
	ZeroMemory(&m_responseVPPOutEnc	, sizeof(m_responseVPPOutEnc));
	ZeroMemory(&m_extVPPDoNotUse	, sizeof(m_extVPPDoNotUse));
	ZeroMemory(&m_extCodingOption	, sizeof(m_extCodingOption));

	ZeroMemory(&m_header			, sizeof(m_header));
	m_headerSize					= 0;
	m_bitstreamSize					= 0;

	ZeroMemory(&m_szDesc			, sizeof(m_szDesc));
}
//////////////////////////////////////////////////////////////////////
CQSVEncoder::~CQSVEncoder()
{
	StopEncoding();
	Cleanup();
	DestroyFrames();
}
//////////////////////////////////////////////////////////////////////
void CQSVEncoder::DestroyFrame(LPQSV_ENCODED_FRAME lpFrame)
{
	if (lpFrame) 
	{
		if (lpFrame->lpData)
			delete [] lpFrame->lpData;

		delete lpFrame;
	}
}
//////////////////////////////////////////////////////////////////////
void CQSVEncoder::DestroyFrames()
{
	POSITION pos = GetHeadPosition();

	while (pos)
		DestroyFrame(GetNext(pos));

	RemoveAll();
}
//////////////////////////////////////////////////////////////////////
int CQSVEncoder::GetFreeSurfaceIndex(mfxFrameSurface1** pSurfacesPool, mfxU16 nPoolSize)
{
    if (pSurfacesPool)
	{
		for (mfxU16 i=0; i<nPoolSize; i++)
		{
			if (0 == pSurfacesPool[i]->Data.Locked)
				return i;
		}
	}

    return -1;
}
//////////////////////////////////////////////////////////////////////
mfxStatus CQSVEncoder::StartEncoding(int inputWidth, int inputHeight, int outputWidth, int outputHeight, int framerate, int mode, int targetUsage, int targetBitrate, int profile, int level, int asyncDepth)
{
	//stop previous encoding session and cleanup Direct3D objects
	StopEncoding();
	Cleanup();

	// initialize mode
	m_mode				= mode;

	// intialize async depth
	m_asyncDepth		= asyncDepth;

	// initialize error code
    mfxStatus status	= MFX_ERR_NONE;

    // initialize session
	mfxIMPL	impl;

	switch (m_mode) 
	{
	case QSV_ENCODER_MODE_SOFTWARE:
		impl = MFX_IMPL_SOFTWARE;
		break;
	case QSV_ENCODER_MODE_HARDWARE_D3D9:
		impl = MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D9;
		break;
	case QSV_ENCODER_MODE_HARDWARE_D3D11:
		impl = MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D11;
		break;
	default:
		return MFX_ERR_UNSUPPORTED;
	}

    mfxVersion	ver	= {0, 1};
    status = m_session.Init(impl, &ver);
	if (status < MFX_ERR_NONE)
		return status;

    // create hardware device
    mfxHDL deviceHandle;

	switch (m_mode)
	{
	case QSV_ENCODER_MODE_HARDWARE_D3D9:
		status = D3D9_CreateDevice(m_session, &deviceHandle, NULL);
		break;
	case QSV_ENCODER_MODE_HARDWARE_D3D11:
		status = D3D11_CreateDevice(m_session, &deviceHandle);
		break;
	}
	if (status < MFX_ERR_NONE)
		return status;

    // provide device to session
	switch (m_mode)
	{
	case QSV_ENCODER_MODE_HARDWARE_D3D9:
		status = m_session.SetHandle(MFX_HANDLE_DIRECT3D_DEVICE_MANAGER9, deviceHandle);
		break;
	case QSV_ENCODER_MODE_HARDWARE_D3D11:
		status = m_session.SetHandle(MFX_HANDLE_D3D11_DEVICE, deviceHandle);
		break;
	}
	if (status < MFX_ERR_NONE)
		return status;

	//initialize allocator
	switch (m_mode)
	{
	case QSV_ENCODER_MODE_SOFTWARE:
		m_allocator.Alloc	= SYS_Alloc;
		m_allocator.Free	= SYS_Free;
		m_allocator.Lock	= SYS_Lock;
		m_allocator.Unlock	= SYS_Unlock;
		m_allocator.GetHDL	= SYS_GetHDL;
		break;
	case QSV_ENCODER_MODE_HARDWARE_D3D9:
		m_allocator.Alloc	= D3D9_Alloc;
		m_allocator.Free	= D3D9_Free;
		m_allocator.Lock	= D3D9_Lock;
		m_allocator.Unlock	= D3D9_Unlock;
		m_allocator.GetHDL	= D3D9_GetHDL;
		break;
	case QSV_ENCODER_MODE_HARDWARE_D3D11:
		m_allocator.Alloc	= D3D11_Alloc;
		m_allocator.Free	= D3D11_Free;
		m_allocator.Lock	= D3D11_Lock;
		m_allocator.Unlock	= D3D11_Unlock;
		m_allocator.GetHDL	= D3D11_GetHDL;
		break;
	}

    // provide allocator to session
    status = m_session.SetFrameAllocator(&m_allocator);
	if (status < MFX_ERR_NONE)
		return status;

    // initialize encoder parameters
    mfxVideoParam EncParams;
    memset(&EncParams, 0, sizeof(EncParams));
    EncParams.mfx.CodecId					    = MFX_CODEC_AVC;
    EncParams.mfx.TargetUsage					= targetUsage;
	EncParams.mfx.CodecProfile					= profile;
	EncParams.mfx.CodecLevel					= level;
    EncParams.mfx.TargetKbps					= targetBitrate;
    EncParams.mfx.RateControlMethod				= MFX_RATECONTROL_VBR; 
    EncParams.mfx.FrameInfo.FrameRateExtN		= framerate;
    EncParams.mfx.FrameInfo.FrameRateExtD		= 1;
    EncParams.mfx.FrameInfo.FourCC				= MFX_FOURCC_NV12;
    EncParams.mfx.FrameInfo.ChromaFormat		= MFX_CHROMAFORMAT_YUV420;
    EncParams.mfx.FrameInfo.PicStruct			= MFX_PICSTRUCT_PROGRESSIVE;
    EncParams.mfx.FrameInfo.CropX				= 0; 
    EncParams.mfx.FrameInfo.CropY				= 0;
    EncParams.mfx.FrameInfo.CropW				= outputWidth;
    EncParams.mfx.FrameInfo.CropH				= outputHeight;
	EncParams.mfx.GopPicSize					= framerate;
    // width must be a multiple of 16 
    // height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
    EncParams.mfx.FrameInfo.Width				= ALIGN16(outputWidth);
    EncParams.mfx.FrameInfo.Height				= (MFX_PICSTRUCT_PROGRESSIVE == EncParams.mfx.FrameInfo.PicStruct) ? ALIGN16(outputHeight) : ALIGN32(outputHeight);
	EncParams.IOPattern							= (m_mode == QSV_ENCODER_MODE_SOFTWARE) ? MFX_IOPATTERN_IN_SYSTEM_MEMORY : MFX_IOPATTERN_IN_VIDEO_MEMORY;
	EncParams.AsyncDepth						= m_asyncDepth;

	mfxExtBuffer* extBuffersEnc[1];
	extBuffersEnc[0]							= (mfxExtBuffer*)&m_extCodingOption;
	
	if (m_asyncDepth == 1)
	{
		EncParams.mfx.GopRefDist				= 1;
		EncParams.mfx.NumRefFrame				= 1;


		ZeroMemory(&m_extCodingOption			, sizeof(m_extCodingOption));
		m_extCodingOption.Header.BufferSz		= sizeof(m_extCodingOption);
		m_extCodingOption.Header.BufferId		= MFX_EXTBUFF_CODING_OPTION;
		m_extCodingOption.MaxDecFrameBuffering	= 1;

		//don't be stupid and don't declare extBuffersEnc here, it will be used further!!!

		EncParams.ExtParam						= extBuffersEnc;
		EncParams.NumExtParam					= 1;
	}

    // initialize VPP parameters
    mfxVideoParam VPPParams;
    memset(&VPPParams, 0, sizeof(VPPParams));

    // VPP input data
    VPPParams.vpp.In.FourCC						= MFX_FOURCC_RGB4;
    VPPParams.vpp.In.ChromaFormat				= MFX_CHROMAFORMAT_YUV420;  
    VPPParams.vpp.In.CropX						= 0;
    VPPParams.vpp.In.CropY						= 0; 
    VPPParams.vpp.In.CropW						= inputWidth;
    VPPParams.vpp.In.CropH						= inputHeight;
    VPPParams.vpp.In.PicStruct					= MFX_PICSTRUCT_PROGRESSIVE;
    VPPParams.vpp.In.FrameRateExtN				= framerate;
    VPPParams.vpp.In.FrameRateExtD				= 1;
    // width must be a multiple of 16 
    // height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture  
    VPPParams.vpp.In.Width						= ALIGN16(inputWidth);
    VPPParams.vpp.In.Height						= (MFX_PICSTRUCT_PROGRESSIVE == VPPParams.vpp.In.PicStruct) ? ALIGN16(inputHeight) : ALIGN32(inputHeight);

    // VPP output data
    VPPParams.vpp.Out.FourCC					= MFX_FOURCC_NV12;     
    VPPParams.vpp.Out.ChromaFormat				= MFX_CHROMAFORMAT_YUV420;             
    VPPParams.vpp.Out.CropX						= 0;
    VPPParams.vpp.Out.CropY						= 0; 
    VPPParams.vpp.Out.CropW						= outputWidth;
    VPPParams.vpp.Out.CropH						= outputHeight;
    VPPParams.vpp.Out.PicStruct					= MFX_PICSTRUCT_PROGRESSIVE;
    VPPParams.vpp.Out.FrameRateExtN				= framerate;
    VPPParams.vpp.Out.FrameRateExtD				= 1;
    // width must be a multiple of 16 
    // height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture  
    VPPParams.vpp.Out.Width						= ALIGN16(VPPParams.vpp.Out.CropW); 
    VPPParams.vpp.Out.Height					= (MFX_PICSTRUCT_PROGRESSIVE == VPPParams.vpp.Out.PicStruct) ? ALIGN16(VPPParams.vpp.Out.CropH) : ALIGN32(VPPParams.vpp.Out.CropH);
	VPPParams.IOPattern							= (m_mode == QSV_ENCODER_MODE_SOFTWARE) ? (MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY) : (MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY);
	VPPParams.AsyncDepth						= m_asyncDepth;

    // create encoder
	m_lpEnc	= new MFXVideoENCODE(m_session); 
    // create VPP
    m_lpVPP	= new MFXVideoVPP(m_session);

    // query number of required surfaces for encoder
    mfxFrameAllocRequest EncRequest;
    memset(&EncRequest, 0, sizeof(EncRequest));
    status = m_lpEnc->QueryIOSurf(&EncParams, &EncRequest);
	if (status < MFX_ERR_NONE)
		return status;

    // query number of required surfaces for VPP
	// [0] - in, [1] - out
    mfxFrameAllocRequest VPPRequest[2];
    memset(&VPPRequest, 0, sizeof(mfxFrameAllocRequest)*2);
	status = m_lpVPP->QueryIOSurf(&VPPParams, VPPRequest);
	if (status < MFX_ERR_NONE)
		return status;

	if (m_mode == QSV_ENCODER_MODE_HARDWARE_D3D11)
		// hint to DX11 memory handler that application will write data to VPP input surfaces
	    VPPRequest[0].Type |= WILL_WRITE; 

    // surfaces are shared between VPP output and encode input
	EncRequest.Type |= MFX_MEMTYPE_FROM_VPPOUT; 

    // determine the required number of surfaces for VPP input and for VPP output (encoder input)
    m_nSurfacesVPPIn				= VPPRequest[0].NumFrameSuggested;
    m_nSurfacesVPPOutEnc			= VPPRequest[1].NumFrameSuggested + EncRequest.NumFrameSuggested;

	VPPRequest[0].NumFrameMin		= m_nSurfacesVPPIn;
	VPPRequest[0].NumFrameSuggested	= m_nSurfacesVPPIn;

    EncRequest.NumFrameMin			= m_nSurfacesVPPOutEnc;
    EncRequest.NumFrameSuggested	= m_nSurfacesVPPOutEnc;
    
    // allocate required surfaces
    status = m_allocator.Alloc(m_allocator.pthis, &VPPRequest[0], &m_responseVPPIn);
	if (status < MFX_ERR_NONE)
		return status;
    status = m_allocator.Alloc(m_allocator.pthis, &EncRequest	, &m_responseVPPOutEnc);
	if (status < MFX_ERR_NONE)
		return status;

    // allocate surface headers (mfxFrameSurface1) for VPPIn
    m_lpSurfacesVPPIn = new mfxFrameSurface1*[m_nSurfacesVPPIn];
    for (int i = 0; i < m_nSurfacesVPPIn; i++)
    {
        m_lpSurfacesVPPIn[i] = new mfxFrameSurface1;
        memset(m_lpSurfacesVPPIn[i], 0, sizeof(mfxFrameSurface1));
        memcpy(&(m_lpSurfacesVPPIn[i]->Info), &(VPPParams.vpp.In), sizeof(mfxFrameInfo));
        m_lpSurfacesVPPIn[i]->Data.MemId = m_responseVPPIn.mids[i]; 
    }  

    // allocate surface headers (mfxFrameSurface1) for VPPOut
    m_lpSurfacesVPPOutEnc = new mfxFrameSurface1*[m_nSurfacesVPPOutEnc];
    for (int i = 0; i < m_nSurfacesVPPOutEnc; i++)
    {       
        m_lpSurfacesVPPOutEnc[i] = new mfxFrameSurface1;
        memset(m_lpSurfacesVPPOutEnc[i], 0, sizeof(mfxFrameSurface1));
        memcpy(&(m_lpSurfacesVPPOutEnc[i]->Info), &(VPPParams.vpp.Out), sizeof(mfxFrameInfo));
        m_lpSurfacesVPPOutEnc[i]->Data.MemId = m_responseVPPOutEnc.mids[i];
    }  

    // disable default VPP operations
    memset(&m_extVPPDoNotUse, 0, sizeof(mfxExtVPPDoNotUse));
    m_extVPPDoNotUse.Header.BufferId	= MFX_EXTBUFF_VPP_DONOTUSE;
    m_extVPPDoNotUse.Header.BufferSz	= sizeof(mfxExtVPPDoNotUse);
    m_extVPPDoNotUse.NumAlg				= 4;
    m_extVPPDoNotUse.AlgList			= new mfxU32 [m_extVPPDoNotUse.NumAlg];    
    m_extVPPDoNotUse.AlgList[0]			= MFX_EXTBUFF_VPP_DENOISE;			// turn off denoising (on by default)
    m_extVPPDoNotUse.AlgList[1]			= MFX_EXTBUFF_VPP_SCENE_ANALYSIS;	// turn off scene analysis (on by default)
    m_extVPPDoNotUse.AlgList[2]			= MFX_EXTBUFF_VPP_DETAIL;			// turn off detail enhancement (on by default)
    m_extVPPDoNotUse.AlgList[3]			= MFX_EXTBUFF_VPP_PROCAMP;			// turn off processing amplified (on by default)

    // add extended VPP buffers
    mfxExtBuffer* extBuffersVPP[1];
    extBuffersVPP[0]					= (mfxExtBuffer*)&m_extVPPDoNotUse;
    VPPParams.ExtParam					= extBuffersVPP;

    VPPParams.NumExtParam				= 1;

    // initialize encoder
    status = m_lpEnc->Init(&EncParams);
	if ((status != MFX_ERR_NONE					) && 
		(status != MFX_WRN_PARTIAL_ACCELERATION	))
		return status;

    // initialize VPP
    status = m_lpVPP->Init(&VPPParams);
	if ((status != MFX_ERR_NONE					) && 
		(status != MFX_WRN_PARTIAL_ACCELERATION	))
		return status;

    // retrieve video parameters selected by encoder.
    // BufferSizeInKB parameter is required to set bit stream buffer size
    mfxVideoParam par;
    memset(&par, 0, sizeof(par));

	//retrieve SPS/PPS headers 

	mfxU8 SPS[100];
	mfxU8 PPS[100]; 
	mfxExtCodingOptionSPSPPS headers;
	memset(&headers, 0, sizeof(headers));
	headers.Header.BufferId = MFX_EXTBUFF_CODING_OPTION_SPSPPS;
	headers.Header.BufferSz = sizeof(mfxExtCodingOptionSPSPPS);   
	headers.SPSBuffer		= SPS;            
	headers.SPSBufSize		= sizeof(SPS);
	headers.PPSBuffer		= PPS; 
	headers.PPSBufSize		= sizeof(PPS);

	mfxExtBuffer* extBuffersHeaders[1];
	extBuffersHeaders[0]	= (mfxExtBuffer*)&headers;

	par.ExtParam			= extBuffersHeaders;
	par.NumExtParam			= 1;

    status = m_lpEnc->GetVideoParam(&par);
	if (status < MFX_ERR_NONE)
		return status;

	DWORD	SPSLen			= headers.SPSBufSize - 4;
	DWORD	PPSLen			= headers.PPSBufSize - 4;

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

    // init bitstream size
	m_bitstreamSize			= par.mfx.BufferSizeInKB * 1000;
	
	// read back actual implementation
    m_session.QueryIMPL(&impl);

	// init description
	char szImplMode[MAX_PATH];

	switch (impl & 0xFF00)
	{
	case MFX_IMPL_VIA_D3D11:
		strcpy_s(szImplMode, sizeof(szImplMode), "Direct3D11");
		break;
	default:
		strcpy_s(szImplMode, sizeof(szImplMode), "Direct3D9");
		break;
	}

	char szImpl[MAX_PATH];

	switch (impl & 0xFF)
	{
	case MFX_IMPL_SOFTWARE:
		strcpy_s(szImpl, sizeof(szImpl), "software emulation");
		break;
	case MFX_IMPL_HARDWARE:
		sprintf_s(szImpl, sizeof(szImpl), "hardware accelerated via %s on adapter 1", szImplMode);
		break;
	case MFX_IMPL_HARDWARE2:
		sprintf_s(szImpl, sizeof(szImpl), "hardware accelerated via %s on adapter 2", szImplMode);
		break;
	case MFX_IMPL_HARDWARE3:
		sprintf_s(szImpl, sizeof(szImpl), "hardware accelerated via %s on adapter 3", szImplMode);
		break;
	case MFX_IMPL_HARDWARE4:
		sprintf_s(szImpl, sizeof(szImpl), "hardware accelerated via %s on adapter 4", szImplMode);
		break;
	default:
		sprintf_s(szImpl, sizeof(szImpl), "unknown implementation %08Xh", impl);
		break;
	}

	sprintf_s(m_szDesc, "Intel QuickSync H.264 %s, async depth %d", szImpl, m_asyncDepth);

	return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
mfxStatus CQSVEncoder::Encode(int inputWidth, int inputHeight, int inputDepth, LPBYTE lpImage, DWORDLONG qwTimestamp)
{
    mfxStatus status = MFX_ERR_NONE;
    mfxSyncPoint syncpVPP, syncpEnc;

	// find free input frame surface
	int nVPPSurfIdx = GetFreeSurfaceIndex(m_lpSurfacesVPPIn, m_nSurfacesVPPIn); 

	if (nVPPSurfIdx < 0)
		//if there is no free input surface then we'll wait for it during 25ms before throwing MFX_ERR_MEMORY_ALLOC
	{
		for (int iRetry=0; iRetry<25; iRetry++)
		{
			nVPPSurfIdx = GetFreeSurfaceIndex(m_lpSurfacesVPPIn, m_nSurfacesVPPIn); 

			if (nVPPSurfIdx >= 0)
				break;

			Sleep(1);
		}
	}

	if (nVPPSurfIdx < 0)
		return MFX_ERR_MEMORY_ALLOC;

	// surface locking required when read/write D3D surfaces
	status = m_allocator.Lock(m_allocator.pthis, m_lpSurfacesVPPIn[nVPPSurfIdx]->Data.MemId, &(m_lpSurfacesVPPIn[nVPPSurfIdx]->Data));
	if (status < MFX_ERR_NONE)
		return status;

	// load frame into surface
    status = LoadFrame(m_lpSurfacesVPPIn[nVPPSurfIdx], inputWidth, inputHeight, inputDepth, lpImage, qwTimestamp); 
	if (status < MFX_ERR_NONE)
		return status;
       
	status = m_allocator.Unlock(m_allocator.pthis, m_lpSurfacesVPPIn[nVPPSurfIdx]->Data.MemId, &(m_lpSurfacesVPPIn[nVPPSurfIdx]->Data));
	if (status < MFX_ERR_NONE)
		return status;

	// find free output frame surface
	int nEncSurfIdx = GetFreeSurfaceIndex(m_lpSurfacesVPPOutEnc, m_nSurfacesVPPOutEnc); 
	if (nEncSurfIdx < 0)
		return MFX_ERR_MEMORY_ALLOC;

	for (;;)
	{  
		// process a frame asychronously (returns immediately)
		status = m_lpVPP->RunFrameVPPAsync(m_lpSurfacesVPPIn[nVPPSurfIdx], m_lpSurfacesVPPOutEnc[nEncSurfIdx], NULL, &syncpVPP);
		if (status == MFX_WRN_DEVICE_BUSY)                
			// wait if device is busy, then repeat the same call          
			Sleep(1); 
		else 
			break;
	}

	if (status == MFX_ERR_MORE_DATA)
		return status;

	// MFX_ERR_MORE_SURFACE means output is ready but need more surface (example: Frame Rate Conversion 30->60)
	// * Not handled in this example!

	if (status < MFX_ERR_NONE)
		return status;

	//wait for VPP output before encoding if async depth is disabled

	if (m_asyncDepth == 1)
	{
		if (MFX_ERR_NONE == status)
		{
			// synchronize and wait until encoded frame is ready
			status = m_session.SyncOperation(syncpVPP, 60000); 
			if (status < MFX_ERR_NONE)
				return status;
		}
	}

	// allocate new task
	CQSVEncoderTask* lpTask = new CQSVEncoderTask;

	// allocate bitstream for new task
	lpTask->Init(m_bitstreamSize);

	for (;;)
	{    
		// encode a frame asychronously (returns immediately)
		status = m_lpEnc->EncodeFrameAsync(NULL, m_lpSurfacesVPPOutEnc[nEncSurfIdx], lpTask->GetBitstream(), &syncpEnc); 
       
		if ((status > MFX_ERR_NONE) && !syncpEnc) 
			// repeat the call if warning and no output
		{
			if (status == MFX_WRN_DEVICE_BUSY)                
				// wait if device is busy, then repeat the same call            
				Sleep(1); 
		}
		else if ((status > MFX_ERR_NONE) && syncpEnc)                 
			// ignore warnings if output is available  
		{
			status = MFX_ERR_NONE; 
			break;
		}
		else if (status == MFX_ERR_NOT_ENOUGH_BUFFER)
			// allocate more bitstream buffer memory here if needed...
		{
			m_bitstreamSize = m_bitstreamSize * 2;

			lpTask->Init(m_bitstreamSize);
		}
		else
			break;
	}  

	if (MFX_ERR_NONE == status)
	{
		// pass sync points to the task
		lpTask->SetSyncPointVPP(syncpVPP);
		lpTask->SetSyncPointEnc(syncpEnc);

		//add task to the queue and get the first queued task if the queue is full
		CQSVEncoderTask* lpQueuedTask = GetQueuedTask(lpTask);
		if (!lpQueuedTask)
			return MFX_ERR_NONE;

		// synchronize and wait until encoded frame is ready
		status = m_session.SyncOperation(lpQueuedTask->GetSyncPointEnc(), 60000); 
		if (status < MFX_ERR_NONE)
		{
			delete lpQueuedTask;
			return status;
		}

		status = SaveFrame(lpQueuedTask->GetBitstream());
		if (status < MFX_ERR_NONE)
		{
			delete lpQueuedTask;
			return status;
		}

		delete lpQueuedTask;
	}
	else
	{
		delete lpTask;
	}

    // MFX_ERR_MORE_DATA means that the input file has ended, need to go to buffering loop, exit in case of other errors
	if (status == MFX_ERR_MORE_DATA)
		status = MFX_ERR_NONE; 

	if (status < MFX_ERR_NONE)
		return status;

	return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
mfxStatus CQSVEncoder::StopEncoding()
{
    mfxStatus status = MFX_ERR_NONE;
    mfxSyncPoint syncpVPP, syncpEnc;

    // retrieve buffered VPP frames
    while (MFX_ERR_NONE <= status)
    {       
		// find free output frame surface
        int nEncSurfIdx = GetFreeSurfaceIndex(m_lpSurfacesVPPOutEnc, m_nSurfacesVPPOutEnc); 
		if (nEncSurfIdx < 0)
			return MFX_ERR_MEMORY_ALLOC;

        for (;;)
        {  
            // process a frame asychronously (returns immediately)
            status = m_lpVPP->RunFrameVPPAsync(NULL, m_lpSurfacesVPPOutEnc[nEncSurfIdx], NULL, &syncpVPP);
            if (MFX_WRN_DEVICE_BUSY == status)                
				// wait if device is busy, then repeat the same call          
                Sleep(1); 
            else 
                break;
        }

		if (status < MFX_ERR_NONE)
			break;

		//wait for VPP output before encoding if async depth is disabled

		if (m_asyncDepth == 1)
		{
			if (MFX_ERR_NONE == status)
			{
				// synchronize and wait until encoded frame is ready
				status = m_session.SyncOperation(syncpVPP, 60000); 
				if (status < MFX_ERR_NONE)
					return status;
			}
		}

		// allocate new task
		CQSVEncoderTask* lpTask = new CQSVEncoderTask;

		// allocate bitstream for new task
		lpTask->Init(m_bitstreamSize);

        for (;;)
        {    
            // encode a frame asychronously (returns immediately)
			status = m_lpEnc->EncodeFrameAsync(NULL, m_lpSurfacesVPPOutEnc[nEncSurfIdx], lpTask->GetBitstream(), &syncpEnc); 
           
            if (MFX_ERR_NONE < status && !syncpEnc) 
				// repeat the call if warning and no output
            {
                if (MFX_WRN_DEVICE_BUSY == status)                
					// wait if device is busy, then repeat the same call            
                    Sleep(1); 
            }
            else if (MFX_ERR_NONE < status && syncpEnc)                 
            {
				// ignore warnings if output is available  
                status = MFX_ERR_NONE; 
                break;
            }
            else if (MFX_ERR_NOT_ENOUGH_BUFFER == status)
            {
				m_bitstreamSize = m_bitstreamSize * 2;

				lpTask->Init(m_bitstreamSize);
            }
            else
                break;
        }  

		if (MFX_ERR_NONE == status)
		{
			// pass sync points to the task
			lpTask->SetSyncPointVPP(syncpVPP);
			lpTask->SetSyncPointEnc(syncpEnc);

			//add task to the queue and get the first queued task if the queue is full
			CQSVEncoderTask* lpQueuedTask = GetQueuedTask(lpTask);

			if (lpQueuedTask)
			{
				// synchronize and wait until encoded frame is ready
				status = m_session.SyncOperation(lpQueuedTask->GetSyncPointEnc(), 60000); 
				if (status < MFX_ERR_NONE)
				{
					delete lpQueuedTask;
					return status;
				}

				status = SaveFrame(lpQueuedTask->GetBitstream());
				if (status < MFX_ERR_NONE)
				{
					delete lpQueuedTask;
					return status;
				}

				delete lpQueuedTask;
			}
		}
		else
		{
			delete lpTask;
		}
    }

    // MFX_ERR_MORE_DATA indicates that there are no more buffered frames, exit in case of other errors
	if (status == MFX_ERR_MORE_DATA)
		status = MFX_ERR_NONE;
	if (status < MFX_ERR_NONE)
		return status;

    // Retrieve the buffered encoder frames

    while (MFX_ERR_NONE <= status)
    {       
		// allocate new task
		CQSVEncoderTask* lpTask = new CQSVEncoderTask;

		// allocate bitstream for new task
		lpTask->Init(m_bitstreamSize);

        for (;;)
        {                
            // Encode a frame asychronously (returns immediately)
			status = m_lpEnc->EncodeFrameAsync(NULL, NULL, lpTask->GetBitstream(), &syncpEnc);  

            if (MFX_ERR_NONE < status && !syncpEnc) // Repeat the call if warning and no output
            {
                if (MFX_WRN_DEVICE_BUSY == status)                
                    Sleep(1); // Wait if device is busy, then repeat the same call                 
            }
            else if (MFX_ERR_NONE < status && syncpEnc)                 
            {
                status = MFX_ERR_NONE; // Ignore warnings if output is available 
                break;
            }
            else
                break;
        }            

		if (MFX_ERR_NONE == status)
		{
			// pass sync points to the task
			lpTask->SetSyncPointEnc(syncpEnc);

			//add task to the queue and get the first queued task if the queue is full
			CQSVEncoderTask* lpQueuedTask = GetQueuedTask(lpTask);
			if (lpQueuedTask)
			{
				// synchronize and wait until encoded frame is ready
				status = m_session.SyncOperation(lpQueuedTask->GetSyncPointEnc(), 60000); 
				if (status < MFX_ERR_NONE)
				{
					delete lpQueuedTask;
					return status;
				}

				status = SaveFrame(lpQueuedTask->GetBitstream());
				if (status < MFX_ERR_NONE)
				{
					delete lpQueuedTask;
					return status;
				}

				delete lpQueuedTask;
			}
		}
		else
		{
			delete lpTask;
		}
    }    

    // MFX_ERR_MORE_DATA indicates that there are no more buffered frames, exit in case of other errors
	if (status == MFX_ERR_MORE_DATA)
		status = MFX_ERR_NONE;
	if (status < MFX_ERR_NONE)
		return status;

	// retreive queued tasks
	CQSVEncoderTask* lpQueuedTask = GetQueuedTask(NULL);

	while (lpQueuedTask)
	{
		// synchronize and wait until encoded frame is ready
		status = m_session.SyncOperation(lpQueuedTask->GetSyncPointEnc(), 60000); 
		if (status < MFX_ERR_NONE)
		{
			delete lpQueuedTask;
			return status;
		}

		status = SaveFrame(lpQueuedTask->GetBitstream());
		if (status < MFX_ERR_NONE)
		{
			delete lpQueuedTask;
			return status;
		}

		delete lpQueuedTask;
		lpQueuedTask = GetQueuedTask(NULL);
	}

	Cleanup();

	return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
void CQSVEncoder::Cleanup()
{
   // clean up resources
   // It is recommended to close Media SDK components first, before releasing allocated surfaces, since
   // some surfaces may still be locked by internal Media SDK resources
    
	if (m_lpEnc)
		m_lpEnc->Close();
	if (m_lpVPP)
		m_lpVPP->Close();

	if (m_lpEnc)
		delete m_lpEnc;
	m_lpEnc = NULL;

	if (m_lpVPP)
		delete m_lpVPP;
	m_lpVPP = NULL;

    // mfxSession closed automatically on destruction
	m_session.Close();

	if (m_lpSurfacesVPPIn)
	{
		for (int i = 0; i < m_nSurfacesVPPIn; i++)
			delete m_lpSurfacesVPPIn[i];

		delete [] m_lpSurfacesVPPIn;

		m_lpSurfacesVPPIn		= NULL;
		m_nSurfacesVPPIn		= 0;
	}

	if (m_lpSurfacesVPPOutEnc)
	{
		for (int i = 0; i < m_nSurfacesVPPOutEnc; i++)
			delete m_lpSurfacesVPPOutEnc[i];

		delete [] m_lpSurfacesVPPOutEnc;

		m_lpSurfacesVPPOutEnc	= NULL;
		m_nSurfacesVPPOutEnc	= 0;
	}

	if (m_extVPPDoNotUse.AlgList)
	{
		delete [] m_extVPPDoNotUse.AlgList;

		m_extVPPDoNotUse.AlgList = NULL;
	}

	if (m_allocator.Free)
	{
		m_allocator.Free(m_allocator.pthis, &m_responseVPPIn	);
		m_allocator.Free(m_allocator.pthis, &m_responseVPPOutEnc);

		ZeroMemory(&m_responseVPPIn		, sizeof(m_responseVPPIn	));
		ZeroMemory(&m_responseVPPOutEnc	, sizeof(m_responseVPPOutEnc));
	}

	switch (m_mode)
	{
	case QSV_ENCODER_MODE_HARDWARE_D3D9:
		D3D9_CleanupDevice();
		break;
	case QSV_ENCODER_MODE_HARDWARE_D3D11:
		D3D11_CleanupDevice();
		break;
	}
}
//////////////////////////////////////////////////////////////////////
mfxStatus CQSVEncoder::LoadFrame(mfxFrameSurface1* lpSurface, int inputWidth, int inputHeight, int inputDepth, LPBYTE lpData, DWORDLONG qwTimestamp)
{
	DWORD dwPitch = inputWidth * (inputDepth>>3);

	if (dwPitch & 3)
		dwPitch = 4 + dwPitch & 0xfffffffc; 

	switch (inputDepth)
	{
	case 24:
		for (int y=0; y<inputHeight; y++)
		{
			DWORD dwSrcOffset = (inputHeight - 1 - y) * dwPitch;
			DWORD dwDstOffset = y * lpSurface->Data.Pitch;

			for (int x=0; x<inputWidth; x++)
			{
				BYTE	r = lpData[dwSrcOffset];
				BYTE	g = lpData[dwSrcOffset + 1];
				BYTE	b = lpData[dwSrcOffset + 2];

				*(LPDWORD)(lpSurface->Data.B + dwDstOffset) = r | (g<<8) | (b<<16);

				dwSrcOffset += 3;
				dwDstOffset += 4;
			}
		}
		break;
	case 32:
		for (int y=0; y<inputHeight; y++)
			memcpy(lpSurface->Data.B + y * lpSurface->Data.Pitch, lpData + (inputHeight - 1 - y) * dwPitch, dwPitch);
		break;
	default:
		return MFX_ERR_UNSUPPORTED;
	}

	lpSurface->Data.TimeStamp = qwTimestamp * 90000 / 1000000;

	return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
mfxStatus CQSVEncoder::SaveFrame(mfxBitstream* lpBitstream)
{
	if (lpBitstream->DataLength)
	{
		LPQSV_ENCODED_FRAME lpFrame	= (LPQSV_ENCODED_FRAME) new BYTE[sizeof(QSV_ENCODED_FRAME)];

		ZeroMemory(lpFrame, sizeof(QSV_ENCODED_FRAME));

		lpFrame->dwFrameType		= lpBitstream->FrameType;
		lpFrame->qwTimestamp		= lpBitstream->TimeStamp * 1000000 / 90000;
		lpFrame->qwDecodeTimestamp	= lpBitstream->DecodeTimeStamp * 1000000 / 90000;

		AddAnnexB(lpBitstream->Data + lpBitstream->DataOffset, lpBitstream->DataLength, lpFrame);

		if (lpFrame->dwPos)
		{
			lpFrame->dwSize = lpFrame->dwPos;
			lpFrame->dwPos	= 0;

			AddTail(lpFrame);
		}
		else
			DestroyFrame(lpFrame);

		lpBitstream->DataLength = 0;
	}

	return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
LPBYTE CQSVEncoder::GetHeader()
{
	return m_header;
}
//////////////////////////////////////////////////////////////////////
DWORD CQSVEncoder::GetHeaderSize()
{
	return m_headerSize;
}
//////////////////////////////////////////////////////////////////////
void CQSVEncoder::AddAnnexB(LPBYTE lpSrc, DWORD dwSize, LPQSV_ENCODED_FRAME lpFrame)
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
void CQSVEncoder::AddNAL(LPBYTE lpSrc, DWORD dwSize, LPQSV_ENCODED_FRAME lpFrame)
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
LPQSV_ENCODED_FRAME CQSVEncoder::GetFrame()
{
	POSITION pos = GetHeadPosition();

	if (pos)
	{
		LPQSV_ENCODED_FRAME lpFrame = GetAt(pos);

		RemoveAt(pos);

		return lpFrame;
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////
LPCSTR CQSVEncoder::GetDesc()
{
	return m_szDesc;
}
//////////////////////////////////////////////////////////////////////
CQSVEncoderTask* CQSVEncoder::GetQueuedTask(CQSVEncoderTask* lpTask)
{
	if (m_asyncDepth > 1)
	{
		if (lpTask)
			m_asyncQueue.AddTail(lpTask);

		int depth = (int)m_asyncQueue.GetCount();

		if (lpTask)
		{
			if (depth >= m_asyncDepth)
				return m_asyncQueue.RemoveHead();
		}
		else
		{
			if (depth)
				return m_asyncQueue.RemoveHead();
		}

		return NULL;
	}

	return lpTask;
}
//////////////////////////////////////////////////////////////////////
BOOL CQSVEncoder::IsD3D9Supported()
{
	OSVERSIONINFO osVersionInfo;
	osVersionInfo.dwOSVersionInfoSize	= sizeof(OSVERSIONINFO);
	GetVersionEx(&osVersionInfo);

	if	((osVersionInfo.dwPlatformId	== VER_PLATFORM_WIN32_NT) &&
		 (osVersionInfo.dwMajorVersion	>= 6					))
		 return TRUE;

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
BOOL CQSVEncoder::IsD3D11Supported()
{
	OSVERSIONINFO osVersionInfo;
	osVersionInfo.dwOSVersionInfoSize	= sizeof(OSVERSIONINFO);
	GetVersionEx(&osVersionInfo);

	if	((osVersionInfo.dwPlatformId	== VER_PLATFORM_WIN32_NT) &&
		 (osVersionInfo.dwMajorVersion	>  6					))
		 return TRUE;

	if	((osVersionInfo.dwPlatformId	== VER_PLATFORM_WIN32_NT) &&
		 (osVersionInfo.dwMajorVersion	== 6					) &&
		 (osVersionInfo.dwMinorVersion	>= 2					))
		 return TRUE;

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
DWORD CQSVEncoder::GetSuggestedMode()
{
	if (IsD3D11Supported())
		return QSV_ENCODER_MODE_HARDWARE_D3D11;

	if (IsD3D9Supported())
		return QSV_ENCODER_MODE_HARDWARE_D3D9;

	return QSV_ENCODER_MODE_SOFTWARE;
}
//////////////////////////////////////////////////////////////////////
int CQSVEncoder::GetMode()
{
	return m_mode;
}
//////////////////////////////////////////////////////////////////////
LPCSTR CQSVEncoder::GetStatusStr(mfxStatus status)
{
	switch (status) 
	{
	case MFX_ERR_NONE:
		return "MFX_ERR_NONE";
	case MFX_ERR_UNKNOWN:
		return "MFX_ERR_UNKNOWN";
	case MFX_ERR_NULL_PTR:
		return "MFX_ERR_NULL_PTR";
	case MFX_ERR_UNSUPPORTED:
		return "MFX_ERR_UNSUPPORTED";
	case MFX_ERR_MEMORY_ALLOC:
		return "MFX_ERR_MEMORY_ALLOC";
	case MFX_ERR_NOT_ENOUGH_BUFFER:
		return "MFX_ERR_NOT_ENOUGH_BUFFER";
	case MFX_ERR_INVALID_HANDLE:
		return "MFX_ERR_INVALID_HANDLE";
	case MFX_ERR_LOCK_MEMORY:
		return "MFX_ERR_LOCK_MEMORY";
	case MFX_ERR_NOT_INITIALIZED:
		return "MFX_ERR_NOT_INITIALIZED";
	case MFX_ERR_NOT_FOUND:
		return "MFX_ERR_NOT_FOUND";
	case MFX_ERR_MORE_DATA:
		return "MFX_ERR_MORE_DATA";
	case MFX_ERR_MORE_SURFACE:
		return "MFX_ERR_MORE_SURFACE";
	case MFX_ERR_ABORTED:
		return "MFX_ERR_ABORTED";
	case MFX_ERR_DEVICE_LOST:
		return "MFX_ERR_DEVICE_LOST";
	case MFX_ERR_INCOMPATIBLE_VIDEO_PARAM:
		return "MFX_ERR_INCOMPATIBLE_VIDEO_PARAM";
	case MFX_ERR_INVALID_VIDEO_PARAM:
		return "MFX_ERR_INVALID_VIDEO_PARAM";
	case MFX_ERR_UNDEFINED_BEHAVIOR:
		return "MFX_ERR_UNDEFINED_BEHAVIOR";
	case MFX_ERR_DEVICE_FAILED:
		return "MFX_ERR_DEVICE_FAILED";
	case MFX_ERR_MORE_BITSTREAM:
		return "MFX_ERR_MORE_BITSTREAM";
	case MFX_WRN_IN_EXECUTION:
		return "MFX_WRN_IN_EXECUTION";
	case MFX_WRN_DEVICE_BUSY:
		return "MFX_WRN_DEVICE_BUSY";
	case MFX_WRN_VIDEO_PARAM_CHANGED:
		return "MFX_WRN_VIDEO_PARAM_CHANGED";
	case MFX_WRN_PARTIAL_ACCELERATION:
		return "MFX_WRN_PARTIAL_ACCELERATION";
	case MFX_WRN_INCOMPATIBLE_VIDEO_PARAM:
		return "MFX_WRN_INCOMPATIBLE_VIDEO_PARAM";
	case MFX_WRN_VALUE_NOT_CHANGED:
		return "MFX_WRN_VALUE_NOT_CHANGED";
	case MFX_WRN_OUT_OF_RANGE:
		return "MFX_WRN_OUT_OF_RANGE";
	case MFX_WRN_FILTER_SKIPPED:
		return "MFX_WRN_FILTER_SKIPPED";
	}

	return "UNKNOWN";
}
//////////////////////////////////////////////////////////////////////

