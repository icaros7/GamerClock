#pragma once
//////////////////////////////////////////////////////////////////////
#include "mfxvideo++.h"
//////////////////////////////////////////////////////////////////////
class CQSVEncoderTask
{
public:
	void Init(DWORD dwSize);
	void SetSyncPointVPP(mfxSyncPoint syncpVPP);
	void SetSyncPointEnc(mfxSyncPoint syncpEnc);
	void Uninit();

	mfxSyncPoint	GetSyncPointVPP();
	mfxSyncPoint	GetSyncPointEnc();
	mfxBitstream*	GetBitstream();

	CQSVEncoderTask();
	~CQSVEncoderTask();

protected:
	mfxSyncPoint	m_syncpVPP;
	mfxSyncPoint	m_syncpEnc;
	mfxBitstream	m_bitstream;
};
//////////////////////////////////////////////////////////////////////
