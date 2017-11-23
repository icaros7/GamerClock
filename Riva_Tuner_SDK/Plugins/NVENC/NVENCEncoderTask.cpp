#include "stdafx.h"
#include "NVENCEncoderTask.h"
//////////////////////////////////////////////////////////////////////
CNVENCEncoderTask::CNVENCEncoderTask(LPNVENC_ENCODER_INPUT_SURFACE lpInputSurface, LPNVENC_ENCODER_OUTPUT_SURFACE lpOutputSurface)
{
	m_lpInputSurface	= lpInputSurface;
	m_lpOutputSurface	= lpOutputSurface;
}
//////////////////////////////////////////////////////////////////////
CNVENCEncoderTask::~CNVENCEncoderTask()
{
}
//////////////////////////////////////////////////////////////////////
LPNVENC_ENCODER_INPUT_SURFACE CNVENCEncoderTask::GetInputSurface()
{
	return m_lpInputSurface;
}
//////////////////////////////////////////////////////////////////////
LPNVENC_ENCODER_OUTPUT_SURFACE CNVENCEncoderTask::GetOutputSurface()
{
	return m_lpOutputSurface;
}
//////////////////////////////////////////////////////////////////////
BOOL CNVENCEncoderTask::IsReady()
{
	if (m_lpOutputSurface)
	{
		if (m_lpOutputSurface->hEvent)
			return (WaitForSingleObject(m_lpOutputSurface->hEvent, 0) == WAIT_OBJECT_0);

		return TRUE;
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
void CNVENCEncoderTask::LockSurfaces()
{
	if (m_lpInputSurface)
		m_lpInputSurface->bLocked = TRUE;

	if (m_lpOutputSurface)
		m_lpOutputSurface->bLocked = TRUE;
}
//////////////////////////////////////////////////////////////////////
void CNVENCEncoderTask::UnlockSurfaces()
{
	if (m_lpInputSurface)
		m_lpInputSurface->bLocked = FALSE;

	if (m_lpOutputSurface)
		m_lpOutputSurface->bLocked = FALSE;
}
//////////////////////////////////////////////////////////////////////
