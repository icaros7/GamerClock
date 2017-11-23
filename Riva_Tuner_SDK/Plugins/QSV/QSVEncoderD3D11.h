//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2013 Intel Corporation. All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////
#include <d3d11.h>
#include <dxgi1_2.h>
#include <atlbase.h>

#include "mfxvideo++.h"
//////////////////////////////////////////////////////////////////////
#define WILL_READ  0x1000
#define WILL_WRITE 0x2000
//////////////////////////////////////////////////////////////////////
// DirectX functionality required to manage D3D surfaces
//////////////////////////////////////////////////////////////////////
// Create DirectX 11 device context
// - Required when using D3D surfaces.
// - D3D Device created and handed to Intel Media SDK
// - Intel graphics device adapter will be determined automatically (does not have to be primary),
//   but with the following caveats:
//     - Device must be active (but monitor does NOT have to be attached)
//     - Device must be enabled in BIOS. Required for the case when used together with a discrete graphics card
//     - For switchable graphics solutions (mobile) make sure that Intel device is the active device
//////////////////////////////////////////////////////////////////////
mfxStatus						D3D11_CreateDevice(mfxSession session, mfxHDL* deviceHandle);
void							D3D11_CleanupDevice();
void							D3D11_SetDeviceContext(ID3D11DeviceContext* devCtx);
ID3D11DeviceContext*			D3D11_GetDeviceContext();
//////////////////////////////////////////////////////////////////////
// Intel Media SDK memory allocator entrypoints
//////////////////////////////////////////////////////////////////////
mfxStatus D3D11_Alloc(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);
mfxStatus D3D11_Lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus D3D11_Unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus D3D11_GetHDL(mfxHDL pthis, mfxMemId mid, mfxHDL *handle);
mfxStatus D3D11_Free(mfxHDL pthis, mfxFrameAllocResponse *response);
