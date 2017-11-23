//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2013 Intel Corporation. All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "QSVEncoderD3D9.h"
//////////////////////////////////////////////////////////////////////
#define D3DFMT_NV12						(D3DFORMAT)MAKEFOURCC('N','V','1','2')
#define D3DFMT_YV12                     (D3DFORMAT)MAKEFOURCC('Y','V','1','2')
//////////////////////////////////////////////////////////////////////
IDirect3DDeviceManager9*				g_pDeviceManager9		= NULL;
IDirect3DDevice9Ex*						g_pD3DD9				= NULL;
IDirect3D9Ex*							g_pD3D9					= NULL;
HANDLE									g_hDeviceHandle			= NULL;
IDirectXVideoAccelerationService*		g_pDXVAServiceDec		= NULL;
IDirectXVideoAccelerationService*		g_pDXVAServiceVPP		= NULL;
mfxFrameAllocResponse					g_savedAllocResponse9	= {};
bool									g_bCreateSharedHandles	= false;
//////////////////////////////////////////////////////////////////////
const struct {
    mfxIMPL impl;		// actual implementation
    mfxU32	adapterID;	// device adapter number
} implTypes[] = {
    {MFX_IMPL_HARDWARE, 0},
    {MFX_IMPL_HARDWARE2, 1},
    {MFX_IMPL_HARDWARE3, 2},
    {MFX_IMPL_HARDWARE4, 3}
};
//////////////////////////////////////////////////////////////////////
// DirectX functionality required to manage D3D surfaces
//////////////////////////////////////////////////////////////////////
mfxU32 GetIntelDeviceAdapterNum(mfxSession session)
{
    mfxU32	adapterNum = 0;
    mfxIMPL impl;

    MFXQueryIMPL(session, &impl);

    mfxIMPL baseImpl = MFX_IMPL_BASETYPE(impl); // Extract Media SDK base implementation type

    // get corresponding adapter number
    for (mfxU8 i = 0; i < sizeof(implTypes)/sizeof(implTypes[0]); i++)
    {
        if (implTypes[i].impl == baseImpl)
        {
            adapterNum = implTypes[i].adapterID;
            break;
        }
    }

    return adapterNum;
}
//////////////////////////////////////////////////////////////////////
// Window procedure stub
//////////////////////////////////////////////////////////////////////
mfxStatus D3D9_CreateDevice(mfxSession session, mfxHDL* deviceHandle, HWND hWnd, bool bCreateSharedHandles)
{
	HRESULT errCode = S_OK;

    if	(hWnd == NULL)
    {
        POINT point = {0, 0};
        hWnd = WindowFromPoint(point);
    }

    g_bCreateSharedHandles = bCreateSharedHandles;

    errCode = Direct3DCreate9Ex(D3D_SDK_VERSION, &g_pD3D9);
    if (FAILED(errCode)) 
		return MFX_ERR_DEVICE_FAILED;
	if (!g_pD3D9)
		return MFX_ERR_DEVICE_FAILED;

	RECT rc;
    GetClientRect(hWnd, &rc);

    D3DPRESENT_PARAMETERS d3dpp;
    memset(&d3dpp, 0, sizeof(d3dpp));

    d3dpp.Windowed					 = true;
    d3dpp.hDeviceWindow				 = hWnd;
    d3dpp.Flags                      = D3DPRESENTFLAG_VIDEO;
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_ONE;
    d3dpp.BackBufferCount            = 1;
    d3dpp.BackBufferFormat           = D3DFMT_A8R8G8B8;
    d3dpp.BackBufferWidth			 = rc.right - rc.left;
    d3dpp.BackBufferHeight			 = rc.bottom - rc.top;
    d3dpp.Flags						|= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    d3dpp.SwapEffect				 = D3DSWAPEFFECT_DISCARD;

    errCode = g_pD3D9->CreateDeviceEx(	GetIntelDeviceAdapterNum(session),
                                D3DDEVTYPE_HAL,
                                hWnd,
                                D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
                                &d3dpp,
                                NULL,
                                &g_pD3DD9);
    if (FAILED(errCode)) 
		return MFX_ERR_NULL_PTR;

    errCode = g_pD3DD9->ResetEx(&d3dpp, NULL);
    if (FAILED(errCode)) 
		return MFX_ERR_UNDEFINED_BEHAVIOR;    

    errCode = g_pD3DD9->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    if (FAILED(errCode)) 
		return MFX_ERR_UNDEFINED_BEHAVIOR;    

    UINT resetToken = 0;
	errCode = DXVA2CreateDirect3DDeviceManager9(&resetToken, &g_pDeviceManager9);
    if (FAILED(errCode)) 
		return MFX_ERR_NULL_PTR;

    errCode = g_pDeviceManager9->ResetDevice(g_pD3DD9, resetToken);
    if (FAILED(errCode)) 
		return MFX_ERR_UNDEFINED_BEHAVIOR;

    *deviceHandle = (mfxHDL)g_pDeviceManager9;
    return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
// Free HW device context
//////////////////////////////////////////////////////////////////////
void D3D9_CleanupDevice()
{
	if (g_pDeviceManager9)
		g_pDeviceManager9->CloseDeviceHandle(g_hDeviceHandle);
	g_hDeviceHandle = NULL;

	if (g_pDXVAServiceDec)
		g_pDXVAServiceDec->Release();
	g_pDXVAServiceDec = NULL;

	if (g_pDXVAServiceVPP)
		g_pDXVAServiceVPP->Release();
	g_pDXVAServiceVPP = NULL;

	if (g_pDeviceManager9)
		g_pDeviceManager9->Release();
	g_pDeviceManager9 = NULL;

	if (g_pD3DD9)
		g_pD3DD9->Release();
	g_pD3DD9 = NULL;

	if (g_pD3D9)
		g_pD3D9->Release();
	g_pD3D9 = NULL;
}
//////////////////////////////////////////////////////////////////////
// Media SDK memory allocator entrypoints
//////////////////////////////////////////////////////////////////////
mfxStatus D3D9_AllocHelper(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
    HRESULT errCode = S_OK;

    DWORD	dwDXVAType;

    // Determine surface format (current simple implementation only supports NV12 and RGB4(32))
    D3DFORMAT format;
    if(MFX_FOURCC_NV12 == request->Info.FourCC)
        format = D3DFMT_NV12;
    else if(MFX_FOURCC_RGB4 == request->Info.FourCC)
        format = D3DFMT_A8R8G8B8;
    else if(MFX_FOURCC_YUY2 == request->Info.FourCC)
        format = D3DFMT_YUY2;
    else if(MFX_FOURCC_YV12 == request->Info.FourCC)
        format = D3DFMT_YV12;
    else
        format = (D3DFORMAT)request->Info.FourCC;
    
    // Determine render target
    if (request->Type & MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET )
        dwDXVAType = DXVA2_VideoProcessorRenderTarget;
    else
        dwDXVAType = DXVA2_VideoDecoderRenderTarget;

    // Force use of video processor if color conversion is required (with this simple set of samples we only illustrate RGB4 color converison via VPP)
    if (format == D3DFMT_A8R8G8B8)  // must use processor service
        dwDXVAType = DXVA2_VideoProcessorRenderTarget;

    mfxMemId* mids = NULL;
    if (!g_bCreateSharedHandles)
    {
        mids = new mfxMemId[request->NumFrameSuggested];
        if (!mids) 
			return MFX_ERR_MEMORY_ALLOC;
    }
    else
    {
        mids = new mfxMemId[request->NumFrameSuggested*2];
        if (!mids) 
			return MFX_ERR_MEMORY_ALLOC;

        memset(mids, 0, sizeof(mfxMemId)*request->NumFrameSuggested*2);
    }
        
    if (!g_hDeviceHandle)
    {
        errCode = g_pDeviceManager9->OpenDeviceHandle(&g_hDeviceHandle);
        if (FAILED(errCode)) 
			return MFX_ERR_MEMORY_ALLOC;
    }

    IDirectXVideoAccelerationService* pDXVAServiceTmp = NULL;

    if (dwDXVAType == DXVA2_VideoDecoderRenderTarget) // for both decode and encode
    {
        if (!g_pDXVAServiceDec)
            errCode = g_pDeviceManager9->GetVideoService(g_hDeviceHandle, IID_IDirectXVideoDecoderService, (void**)&g_pDXVAServiceDec);

        pDXVAServiceTmp = g_pDXVAServiceDec;
    }
    else // DXVA2_VideoProcessorRenderTarget ; for VPP
    {
        if (!g_pDXVAServiceVPP)
            errCode = g_pDeviceManager9->GetVideoService(g_hDeviceHandle, IID_IDirectXVideoProcessorService, (void**)&g_pDXVAServiceVPP);

        pDXVAServiceTmp = g_pDXVAServiceVPP;
    }
    if (FAILED(errCode)) 
		return MFX_ERR_MEMORY_ALLOC;

    if	(g_bCreateSharedHandles && !(request->Type & MFX_MEMTYPE_INTERNAL_FRAME))
    {
        // Allocate surfaces with shared handles. Commonly used for OpenCL interoperability
        for(int i=0; i<request->NumFrameSuggested; ++i)
        {
            mfxMemId* tmpptr = mids + i + request->NumFrameSuggested;

            errCode = pDXVAServiceTmp->CreateSurface(request->Info.Width,
                                                request->Info.Height,
                                                0,
                                                format,
                                                D3DPOOL_DEFAULT,
                                                0,
                                                dwDXVAType,
                                                (IDirect3DSurface9 **)mids+i,
                                                (HANDLE*)(tmpptr));
        }
    }
    else
    {
        // Allocate surfaces 
        errCode = pDXVAServiceTmp->CreateSurface(request->Info.Width,
                                            request->Info.Height,
                                            request->NumFrameSuggested - 1,
                                            format,
                                            D3DPOOL_DEFAULT,
                                            0,
                                            dwDXVAType,
                                            (IDirect3DSurface9**)mids,
                                            NULL);
    }
    if (FAILED(errCode)) 
		return MFX_ERR_MEMORY_ALLOC;

    response->mids				= mids;
    response->NumFrameActual	= request->NumFrameSuggested;

    return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
mfxStatus D3D9_Alloc(mfxHDL /*pthis*/, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
    mfxStatus sts = MFX_ERR_NONE;

    if (request->Type & MFX_MEMTYPE_SYSTEM_MEMORY)
        return MFX_ERR_UNSUPPORTED;

    if (request->NumFrameSuggested <= g_savedAllocResponse9.NumFrameActual &&
        MFX_MEMTYPE_EXTERNAL_FRAME & request->Type &&
        MFX_MEMTYPE_FROM_DECODE & request->Type &&
        g_savedAllocResponse9.NumFrameActual != 0)
    {
        // Memory for this request was already allocated during manual allocation stage. Return saved response
        // When decode acceleration device (DXVA) is created it requires a list of D3D surfaces to be passed.
        // Therefore Media SDK will ask for the surface info/mids again at Init() stage, thus requiring us to return the saved response
        // (No such restriction applies to Encode or VPP)

        *response = g_savedAllocResponse9;
    }
    else
    {
        sts = D3D9_AllocHelper(request, response);

        if (MFX_MEMTYPE_EXTERNAL_FRAME & request->Type &&
            MFX_MEMTYPE_FROM_DECODE & request->Type)
        {
            g_savedAllocResponse9 = *response;
        }
    }

    return sts;
}
//////////////////////////////////////////////////////////////////////
mfxStatus D3D9_Lock(mfxHDL /*pthis*/, mfxMemId mid, mfxFrameData *ptr)
{
    IDirect3DSurface9 *pSurface = (IDirect3DSurface9 *)mid;

    if (pSurface == 0) 
		return MFX_ERR_INVALID_HANDLE;
    if (ptr == 0) 
		return MFX_ERR_LOCK_MEMORY;
    
    D3DSURFACE_DESC desc;
    HRESULT hr = pSurface->GetDesc(&desc);
    if (FAILED(hr)) 
		return MFX_ERR_LOCK_MEMORY;

    D3DLOCKED_RECT locked;
    hr = pSurface->LockRect(&locked, 0, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr)) 
		return MFX_ERR_LOCK_MEMORY;

    // In these simple set of samples we only illustrate usage of NV12 and RGB4(32)
    if(D3DFMT_NV12 == desc.Format)
    {
        ptr->Pitch	= (mfxU16)locked.Pitch;
        ptr->Y		= (mfxU8 *)locked.pBits;
        ptr->U		= (mfxU8 *)locked.pBits + desc.Height * locked.Pitch;
        ptr->V		= ptr->U + 1;
    }
    else if(D3DFMT_A8R8G8B8 == desc.Format)
    {
        ptr->Pitch = (mfxU16)locked.Pitch;
        ptr->B = (mfxU8 *)locked.pBits;
        ptr->G = ptr->B + 1;
        ptr->R = ptr->B + 2;
        ptr->A = ptr->B + 3;
    }
    else if(D3DFMT_YUY2 == desc.Format)
    {
        ptr->Pitch = (mfxU16)locked.Pitch;
        ptr->Y = (mfxU8 *)locked.pBits;
        ptr->U = ptr->Y + 1;
        ptr->V = ptr->Y + 3;
    }
    else if(D3DFMT_YV12 == desc.Format)
    {
        ptr->Pitch = (mfxU16)locked.Pitch;
        ptr->Y = (mfxU8 *)locked.pBits;
        ptr->V = ptr->Y + desc.Height * locked.Pitch;
        ptr->U = ptr->V + (desc.Height * locked.Pitch) / 4;
    }
    else if(D3DFMT_P8 == desc.Format)
    {
        ptr->Pitch = (mfxU16)locked.Pitch;
        ptr->Y = (mfxU8 *)locked.pBits;
        ptr->U = 0;
        ptr->V = 0;
    }

    return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
mfxStatus D3D9_Unlock(mfxHDL /*pthis*/, mfxMemId mid, mfxFrameData *ptr)
{
    IDirect3DSurface9 *pSurface = (IDirect3DSurface9 *)mid;

    if (pSurface == 0) 
		return MFX_ERR_INVALID_HANDLE;

    pSurface->UnlockRect();
    
    if (NULL != ptr)
    {
        ptr->Pitch = 0;
        ptr->R     = 0;
        ptr->G     = 0;
        ptr->B     = 0;
        ptr->A     = 0;
    }

    return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
mfxStatus D3D9_GetHDL(mfxHDL /*pthis*/, mfxMemId mid, mfxHDL *handle)
{
    if (handle == 0) 
		return MFX_ERR_INVALID_HANDLE;

    *handle = mid;
    return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
mfxStatus D3D9_Free(mfxHDL /*pthis*/, mfxFrameAllocResponse *response)
{
    if (!response) 
		return MFX_ERR_NULL_PTR;

    if (response->mids)
    {
        for (mfxU32 i = 0; i < response->NumFrameActual; i++)
        {
            if (response->mids[i])
            {
                IDirect3DSurface9* handle = (IDirect3DSurface9*)response->mids[i];
                handle->Release();
            }
        }        
    }

    delete [] response->mids;
    response->mids = 0;
    
    return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
