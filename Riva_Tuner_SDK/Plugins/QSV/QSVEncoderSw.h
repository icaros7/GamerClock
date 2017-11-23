#pragma once
//////////////////////////////////////////////////////////////////////
#include "mfxvideo++.h"
//////////////////////////////////////////////////////////////////////
// Intel Media SDK memory allocator entrypoints
//////////////////////////////////////////////////////////////////////
mfxStatus SYS_Alloc(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);
mfxStatus SYS_Lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus SYS_Unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
mfxStatus SYS_GetHDL(mfxHDL pthis, mfxMemId mid, mfxHDL *handle);
mfxStatus SYS_Free(mfxHDL pthis, mfxFrameAllocResponse *response);
