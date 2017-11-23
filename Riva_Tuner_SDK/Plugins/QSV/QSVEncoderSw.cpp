#include "stdafx.h"
#include "QSVEncoderSw.h"
//////////////////////////////////////////////////////////////////////
#define ALIGN16(X)				(((X + 15) >> 4) << 4)
#define ALIGN32(X)				(((mfxU32)((X)+31)) & ( ~ (mfxU32)31))
//////////////////////////////////////////////////////////////////////
typedef struct 
{
	mfxU32 FourCC;
	mfxU16 width, height;
	mfxU8 *base;
} mid_struct;
//////////////////////////////////////////////////////////////////////
mfxStatus SYS_Alloc(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response) 
{
	if (!(request->Type & MFX_MEMTYPE_SYSTEM_MEMORY))
		return MFX_ERR_UNSUPPORTED;

	if ((request->Info.FourCC != MFX_FOURCC_NV12) &&
		(request->Info.FourCC != MFX_FOURCC_RGB4))
		return MFX_ERR_UNSUPPORTED;

	response->NumFrameActual	= request->NumFrameMin;
	response->mids				= (mfxMemId*)malloc(response->NumFrameActual*sizeof(mfxMemId*));

	for (int i=0;i<request->NumFrameMin;i++) 
	{
		mid_struct *mmid	= (mid_struct*)malloc(sizeof(mid_struct));
		response->mids[i]	= mmid;

		mmid->FourCC		= request->Info.FourCC;
		mmid->width			= ALIGN32(request->Info.Width);
		mmid->height		= ALIGN32(request->Info.Height);

		switch (request->Info.FourCC)
		{
		case MFX_FOURCC_NV12:
			mmid->base			= (mfxU8*)malloc(mmid->width*mmid->height*3/2);
			break;
		case MFX_FOURCC_RGB4:
			mmid->base			= (mfxU8*)malloc(mmid->width*mmid->height*4);
			break;
		}
	}

	return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
mfxStatus SYS_Lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr) 
{
	mid_struct *mmid = (mid_struct*)mid;

	switch (mmid->FourCC)
	{
	case MFX_FOURCC_NV12:
		ptr->Pitch	= mmid->width;
		ptr->Y		= mmid->base;
		ptr->U		= ptr->Y + mmid->width * mmid->height;
		ptr->V		= ptr->U + 1;
		break;
	case MFX_FOURCC_RGB4:
		ptr->Pitch	= mmid->width * 4;
		ptr->B		= mmid->base;
        ptr->G		= ptr->B + 1;
        ptr->R		= ptr->B + 2;
        ptr->A		= ptr->B + 3;
		break;
	}

	return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
mfxStatus SYS_Unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr) 
{
	if (ptr) 
	{
		ptr->Pitch	= 0;
		ptr->Y		= 0;
		ptr->U		= 0;
		ptr->V		= 0;
		ptr->A		= 0;
		ptr->R		= 0;
		ptr->G		= 0;
		ptr->B		= 0;
	}

	return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
mfxStatus SYS_GetHDL(mfxHDL pthis, mfxMemId mid, mfxHDL *handle) 
{
	return MFX_ERR_UNSUPPORTED;
}
//////////////////////////////////////////////////////////////////////
mfxStatus SYS_Free(mfxHDL pthis, mfxFrameAllocResponse *response) 
{
	for (int i=0; i<response->NumFrameActual; i++) 
	{
		mid_struct* mmid = (mid_struct*)response->mids[i];
		free(mmid->base); 
		free(mmid);
	}

	free(response->mids);

	return MFX_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
