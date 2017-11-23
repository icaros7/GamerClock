#pragma once
//////////////////////////////////////////////////////////////////////
#include "NVENCEncoder.h"
//////////////////////////////////////////////////////////////////////
class CNVENCEncoderTask
{
public:
	void LockSurfaces();
	void UnlockSurfaces();

	BOOL IsReady();

	LPNVENC_ENCODER_INPUT_SURFACE	GetInputSurface();
	LPNVENC_ENCODER_OUTPUT_SURFACE	GetOutputSurface();

	CNVENCEncoderTask(LPNVENC_ENCODER_INPUT_SURFACE lpInputSurface, LPNVENC_ENCODER_OUTPUT_SURFACE lpOutputSurface);
	~CNVENCEncoderTask();

protected:
	LPNVENC_ENCODER_INPUT_SURFACE	m_lpInputSurface;
	LPNVENC_ENCODER_OUTPUT_SURFACE	m_lpOutputSurface;
};
//////////////////////////////////////////////////////////////////////
