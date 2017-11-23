#include "StdAfx.h"
#include "VCEAMFEncoder.h"
//////////////////////////////////////////////////////////////////////
#include <io.h>
#include <d3d9.h>
#include <d3d11.h>
//////////////////////////////////////////////////////////////////////
#define ALIGN(X,N) (X+N-1)&~(N-1)
#define MS_TO_100NS 10000
//////////////////////////////////////////////////////////////////////
CVCEAMFEncoder::CVCEAMFEncoder()
{ 
	ZeroMemory(&m_header			, sizeof(m_header));
	m_headerSize					= 0;

	ZeroMemory(&m_pps				, sizeof(m_pps));
	m_ppsSize						= 0;

	ZeroMemory(&m_sps				, sizeof(m_sps));
	m_spsSize						= 0;

	ZeroMemory(&m_szDesc			, sizeof(m_szDesc));

	m_lpContext						= NULL;
	m_lpEncoder						= NULL;

	m_memoryTypeIn					= amf::AMF_MEMORY_DX9;
	m_formatIn						= amf::AMF_SURFACE_NV12;
}
//////////////////////////////////////////////////////////////////////
CVCEAMFEncoder::~CVCEAMFEncoder()
{
	StopEncoding();
	Cleanup();
	DestroyFrames();
}
//////////////////////////////////////////////////////////////////////
void CVCEAMFEncoder::DestroyFrame(LPVCEAMF_ENCODED_FRAME lpFrame)
{
	if (lpFrame) 
	{
		if (lpFrame->lpData)
			delete [] lpFrame->lpData;

		delete lpFrame;
	}
}
//////////////////////////////////////////////////////////////////////
void CVCEAMFEncoder::DestroyFrames()
{
	POSITION pos = GetHeadPosition();

	while (pos)
		DestroyFrame(GetNext(pos));

	RemoveAll();
}
//////////////////////////////////////////////////////////////////////
int CVCEAMFEncoder::StartEncoding(int width, int height, int framerate, int targetBitrate, int device, amf::AMF_MEMORY_TYPE memoryTypeIn, LPCSTR lpConfigFile)
{
	//stop previous encoding session

	StopEncoding();
	Cleanup();

	//reset variables

	AMF_RESULT result;

	m_memoryTypeIn	= memoryTypeIn;
	m_formatIn		= amf::AMF_SURFACE_NV12;

	//init config map from file

	CMapStringToString configMap;

	CStdioFile	file;
	CString		strLine;

	if (file.Open(lpConfigFile, CFile::modeRead|CFile::shareDenyWrite))
	{
		char	szLValue[MAX_PATH];
		char	szRValue[MAX_PATH];

		while (file.ReadString(strLine))
		{
			if (sscanf_s(strLine, "%s %s", szLValue, sizeof(szLValue), szRValue, sizeof(szRValue)) == 2)
			{
				CString strTemp;

				int value;
				if (sscanf(szRValue, "%d", &value) == 1)
					configMap.SetAt(szLValue, szRValue);
			}
		}
	}

    // create context

    result = AMFCreateContext(&m_lpContext);
    if (result != AMF_OK)
        return VCEAMF_ERR_AMFCREATECONTEXT_FAILED;

	// init context

	char szImplMode[MAX_PATH];

	switch (memoryTypeIn)
	{
	case amf::AMF_MEMORY_HOST:
		strcpy_s(szImplMode, sizeof(szImplMode), "host");
		break;
	case amf::AMF_MEMORY_DX9:
        result = m_lpContext->InitDX9(NULL);
	    if (result != AMF_OK)
		    return VCEAMF_ERR_AMFCONTEXT_INITDX9_FAILED;
		strcpy_s(szImplMode, sizeof(szImplMode), "Direct3D9");
		break;
	case amf::AMF_MEMORY_DX11:
	    result = m_lpContext->InitDX11(NULL);
	    if (result != AMF_OK)
		    return VCEAMF_ERR_AMFCONTEXT_INITDX11_FAILED;
		strcpy_s(szImplMode, sizeof(szImplMode), "Direct3D11");
		break;
	default:
	    return VCEAMF_ERR_UNSUPPORTED;
	}

    // create encoder

    result = AMFCreateComponent(m_lpContext, AMFVideoEncoderVCE_AVC, &m_lpEncoder);
    if (result != AMF_OK)
        return VCEAMF_ERR_AMFCREATECOMPONENT_FAILED;

	//configure encoder

	POSITION pos = configMap.GetStartPosition();

    m_lpEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE						, AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);

	while (pos)
	{
		CString strProperty;
		CString strValue;
		configMap.GetNextAssoc(pos, strProperty, strValue);

		WCHAR wszProperty[MAX_PATH];

		size_t result;
		mbstowcs_s(&result, wszProperty	, MAX_PATH, strProperty	, strlen(strProperty));   

		int value;
		if (sscanf_s(strValue, "%d", &value) == 1)
			m_lpEncoder->SetProperty(wszProperty, value);
	}

    m_lpEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE				, targetBitrate * 1000);
    m_lpEncoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE				, targetBitrate * 2000);
    m_lpEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE					, ::AMFConstructSize(width, height));
    m_lpEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE					, ::AMFConstructRate(framerate, 1));
	m_lpEncoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD					, framerate);
	m_lpEncoder->SetProperty(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING		, framerate);

	if (height > 1080)
		m_lpEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, 51);

    result = m_lpEncoder->Init(m_formatIn, width, height);
    if (result != AMF_OK)
        return VCEAMF_ERR_AMFCOMPONENT_INIT_FAILED;

	// init description

	sprintf_s(m_szDesc, "AMD VCE H.264 hardware accelerated via AMD AMF with %s memory interface", szImplMode);

	return VCEAMF_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
int CVCEAMFEncoder::FlushFrames()
{
    AMF_RESULT result;

	amf::AMFDataPtr lpData = NULL;

	do
	{
		result = m_lpEncoder->QueryOutput(&lpData);

		if (result == AMF_REPEAT)
		{
			Sleep(1);
			continue;
		}

		if (result != AMF_OK)
			return VCEAMF_ERR_AMFCOMPONENT_QUERYOUTPUT_FAILED;

		SaveFrame(lpData);
	}
	while (lpData);

	return VCEAMF_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
int CVCEAMFEncoder::Encode(int width, int height, LPBYTE lpImage, DWORDLONG qwTimestamp)
{
    AMF_RESULT result;

    amf::AMFSurfacePtr lpSurface = NULL;

	result = m_lpContext->AllocSurface(m_memoryTypeIn, m_formatIn, width, height, &lpSurface);
	if (result != AMF_OK)
		return VCEAMF_ERR_AMFCONTEXT_ALLOCSURFACE_FAILED;

	LoadFrame(lpSurface, width, height, lpImage, qwTimestamp);

	result = m_lpEncoder->SubmitInput(lpSurface);

	while (result == AMF_INPUT_FULL)
	{
		Sleep(1);

		FlushFrames();

		result = m_lpEncoder->SubmitInput(lpSurface);
	}

	if (result != AMF_OK)
		return VCEAMF_ERR_AMFCOMPONENT_SUBMITINPUT_FAILED;

	FlushFrames();

	return AMF_OK;
}
//////////////////////////////////////////////////////////////////////
int CVCEAMFEncoder::StopEncoding()
{
	//flush encoder 
	if (m_lpEncoder)
	{
		m_lpEncoder->Drain();
		m_lpEncoder->Flush();

		FlushFrames();
	}

	//release encoder

	if (m_lpEncoder)
		m_lpEncoder->Terminate();
	m_lpEncoder					= NULL;

	//release context

	if (m_lpContext)
		m_lpContext->Terminate();
	m_lpContext = NULL;

	//cleanup application allocated memory

	Cleanup();

	return VCEAMF_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
void CVCEAMFEncoder::Cleanup()
{
}
//////////////////////////////////////////////////////////////////////
LPBYTE CVCEAMFEncoder::GetHeader()
{
	return m_header;
}
//////////////////////////////////////////////////////////////////////
DWORD CVCEAMFEncoder::GetHeaderSize()
{
	return m_headerSize;
}
//////////////////////////////////////////////////////////////////////
void CVCEAMFEncoder::AddAnnexB(LPBYTE lpSrc, DWORD dwSize, LPVCEAMF_ENCODED_FRAME lpFrame)
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
void CVCEAMFEncoder::AddNAL(LPBYTE lpSrc, DWORD dwSize, LPVCEAMF_ENCODED_FRAME lpFrame)
{
	BYTE NAL = lpSrc[0] & 0x1F;

	switch (NAL)
	{
	case 0x05:	//IDR Picture
		lpFrame->dwFrameType |= VCEAMF_FRAMETYPE_I;
		break;

	case 0x07:	//SPS NAL
		if (!m_spsSize)
		{
			m_spsSize = dwSize - 1;

			memcpy(m_sps, lpSrc + 1, m_spsSize);
		}

		InitHeader();

		break;
	case 0x08:	//PPS NAL
		if (!m_ppsSize)
		{
			m_ppsSize = dwSize - 1;

			memcpy(m_pps, lpSrc + 1, m_ppsSize);
		}

		InitHeader();

		break;
	}

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
LPVCEAMF_ENCODED_FRAME CVCEAMFEncoder::GetFrame()
{
	POSITION pos = GetHeadPosition();

	if (pos)
	{
		LPVCEAMF_ENCODED_FRAME lpFrame = GetAt(pos);

		RemoveAt(pos);

		return lpFrame;
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////
LPCSTR CVCEAMFEncoder::GetDesc()
{
	return m_szDesc;
}
//////////////////////////////////////////////////////////////////////
int CVCEAMFEncoder::GetConfigMap(CMapStringToString* lpConfigMap, LPCSTR lpName)
{
	CString strValue;

	if (lpConfigMap->Lookup(lpName, strValue))
	{
		int	value;

		if (sscanf_s(strValue, "%d", &value) == 1)
			return value;
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////
void CVCEAMFEncoder::SaveFrame(amf::AMFDataPtr lpData)
{
	LPVCEAMF_ENCODED_FRAME lpFrame	= (LPVCEAMF_ENCODED_FRAME) new BYTE[sizeof(VCEAMF_ENCODED_FRAME)];

	ZeroMemory(lpFrame, sizeof(VCEAMF_ENCODED_FRAME));

	lpFrame->qwTimestamp = lpData->GetPts() / MS_TO_100NS;

    amf::AMFBufferPtr buffer(lpData);

	AddAnnexB((LPBYTE)buffer->GetNative(), buffer->GetSize(), lpFrame);

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
void CVCEAMFEncoder::LoadFrame(LPBYTE lpDstY, LPBYTE lpDstUV, DWORD dwDstPitch, int width, int height, LPBYTE lpSrc)
{
	DWORD dwSrcPitch = width;

	for (int y=0; y<height; y++)
		CopyMemory(lpDstY + y * dwDstPitch, lpSrc + y * dwSrcPitch, width);

	LPBYTE lpSrcUV = lpSrc + dwSrcPitch * height;

	for (int y=0; y<height/2; y++)
		CopyMemory(lpDstUV + y * dwDstPitch, lpSrcUV + y * dwSrcPitch, width);
}
//////////////////////////////////////////////////////////////////////
void CVCEAMFEncoder::LoadFrame(amf::AMFSurfacePtr lpSurface, int width, int height, LPBYTE lpSrc, DWORDLONG qwTimestamp)
{
	lpSurface->SetPts(qwTimestamp * MS_TO_100NS);

	if (lpSurface->GetMemoryType() == amf::AMF_MEMORY_HOST)
	{
		int	horPitch	= lpSurface->GetPlaneAt(0)->GetHPitch();
		int	verPitch	= lpSurface->GetPlaneAt(0)->GetVPitch();
		LPBYTE	lpHost		= (LPBYTE)lpSurface->GetPlaneAt(0)->GetNative();

		LoadFrame(lpHost, lpHost + verPitch * horPitch, horPitch, width, height, lpSrc);
	}
	else
	if (lpSurface->GetMemoryType() == amf::AMF_MEMORY_DX9)
	{
		IDirect3DSurface9* lpSurface9 = (IDirect3DSurface9*)lpSurface->GetPlaneAt(0)->GetNative(); 
			// no reference counting, do not release!

		if (lpSurface9)
		{
			D3DLOCKED_RECT lockedRect;

			if (SUCCEEDED(lpSurface9->LockRect(&lockedRect, NULL, D3DLOCK_DISCARD)))
			{
				LoadFrame((LPBYTE)lockedRect.pBits, (LPBYTE)lockedRect.pBits + height * lockedRect.Pitch, lockedRect.Pitch, width, height, lpSrc);
				lpSurface9->UnlockRect();
			}
		}
	}
	else 
	if (lpSurface->GetMemoryType() == amf::AMF_MEMORY_DX11)
	{
		ID3D11Device*		lpDevice11	= (ID3D11Device*)m_lpContext->GetDX11Device();
			// no reference counting, do not release!
		ID3D11Texture2D*	lpTexture11	= (ID3D11Texture2D*)lpSurface->GetPlaneAt(0)->GetNative(); 
			// no reference counting, do not release!

		if (lpDevice11 && lpTexture11)
		{
			ID3D11DeviceContext* lpContext11 = NULL;

			lpDevice11->GetImmediateContext(&lpContext11);

			if (lpContext11)
			{
				ID3D11Texture2D* lpTextureStage11	= NULL;

				D3D11_TEXTURE2D_DESC desc			= {0};
				desc.Width							= width;
				desc.Height							= height;
				desc.MipLevels						= 1;
				desc.ArraySize						= 1;
				desc.Format							= DXGI_FORMAT_NV12;
				desc.SampleDesc.Count				= 1;
				desc.Usage							= D3D11_USAGE_STAGING;
				desc.CPUAccessFlags					= D3D11_CPU_ACCESS_WRITE;

				if (SUCCEEDED(lpDevice11->CreateTexture2D(&desc, NULL, &lpTextureStage11)))
				{
					D3D11_MAPPED_SUBRESOURCE lockedRect;

					if (SUCCEEDED(lpContext11->Map(lpTextureStage11, 0, D3D11_MAP_WRITE, 0, &lockedRect)))
					{
						LoadFrame((LPBYTE)lockedRect.pData, (LPBYTE)lockedRect.pData + height * lockedRect.RowPitch, lockedRect.RowPitch, width, height, lpSrc);
						lpContext11->Unmap(lpTextureStage11, 0);

				        lpContext11->CopySubresourceRegion(lpTexture11, 0, 0, 0, 0, lpTextureStage11, 0, NULL);
					}

					lpTextureStage11->Release();
				}

				lpContext11->Release();
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////
void CVCEAMFEncoder::InitHeader()
{
	if (!m_headerSize)
	{
		if (m_spsSize &&
			m_ppsSize)
		{
			m_headerSize				= 5 + 1 + 2 + m_spsSize + 1 + 2 + m_ppsSize;

			m_header[0]					= 1; 
			m_header[1]					= m_sps[0];			// profile
			m_header[2]					= m_sps[1];			// profile compat
			m_header[3]					= m_sps[2];			// level
			m_header[4]					= 0xFF;				// nalu size length is four bytes 

			m_header[5]					= 0xE1;				// one sps 
			m_header[6]					= (BYTE)(m_spsSize>>8); 
			m_header[7]					= (BYTE)(m_spsSize); 
			memcpy(m_header + 8, m_sps, m_spsSize); 

			m_header[m_spsSize + 8]		= 1;				// one pps 
			m_header[m_spsSize + 9]		= (BYTE)(m_ppsSize>>8); 
			m_header[m_spsSize + 10]	= (BYTE)(m_ppsSize); 
			memcpy(m_header + m_spsSize + 11, m_pps, m_ppsSize); 
		}
	}
}
//////////////////////////////////////////////////////////////////////
LPCSTR CVCEAMFEncoder::GetStatusStr(int status)
{
	switch (status) 
	{
	case VCEAMF_ERR_NONE:
		return "VCEAMF_ERR_NONE";

	case VCEAMF_ERR_UNKNOWN:
		return "VCEAMF_ERR_UNKNOWN";
	case VCEAMF_ERR_UNSUPPORTED:
		return "VCEAMF_ERR_UNSUPPORTED";
	case VCEAMF_ERR_AMFCREATECONTEXT_FAILED:
		return "VCEAMF_ERR_AMFCREATECONTEXT_FAILED";
	case VCEAMF_ERR_AMFCREATECOMPONENT_FAILED:
		return "VCEAMF_ERR_AMFCREATECOMPONENT_FAILED";
	case VCEAMF_ERR_AMFCONTEXT_INITDX9_FAILED:
		return "VCEAMF_ERR_AMFCONTEXT_INITDX9_FAILED";
	case VCEAMF_ERR_AMFCONTEXT_INITDX11_FAILED:
		return "VCEAMF_ERR_AMFCONTEXT_INITDX11_FAILED";
	case VCEAMF_ERR_AMFCONTEXT_INITOPENCL_FAILED:
		return "VCEAMF_ERR_AMFCONTEXT_INITOPENCL_FAILED";
	case VCEAMF_ERR_AMFCONTEXT_ALLOCSURFACE_FAILED:
		return "VCEAMF_ERR_AMFCONTEXT_ALLOCSURFACE_FAILED";
	case VCEAMF_ERR_AMFCOMPONENT_SETPROPERTY_FAILED:
		return "VCEAMF_ERR_AMFCOMPONENT_SETPROPERTY_FAILED";
	case VCEAMF_ERR_AMFCOMPONENT_INIT_FAILED:
		return "VCEAMF_ERR_AMFCOMPONENT_INIT_FAILED";
	case VCEAMF_ERR_AMFCOMPONENT_SUBMITINPUT_FAILED:
		return "VCEAMF_ERR_AMFCOMPONENT_SUBMITINPUT_FAILED";
	case VCEAMF_ERR_AMFCOMPONENT_QUERYOUTPUT_FAILED:
		return "VCEAMF_ERR_AMFCOMPONENT_QUERYOUTPUT_FAILED";
	}

	return "UNKNOWN";
}
//////////////////////////////////////////////////////////////////////
