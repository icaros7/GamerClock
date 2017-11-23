#include "StdAfx.h"
#include "VCEEncoder.h"
//////////////////////////////////////////////////////////////////////
#include <io.h>
//////////////////////////////////////////////////////////////////////
#define ALIGN(X,N) (X+N-1)&~(N-1)
//////////////////////////////////////////////////////////////////////
CVCEEncoder::CVCEEncoder()
{
	ZeroMemory(&m_header			, sizeof(m_header));
	m_headerSize					= 0;

	ZeroMemory(&m_pps				, sizeof(m_pps));
	m_ppsSize						= 0;

	ZeroMemory(&m_sps				, sizeof(m_sps));
	m_spsSize						= 0;

	ZeroMemory(&m_szDesc			, sizeof(m_szDesc));

	ZeroMemory(&m_deviceHandle		, sizeof(m_deviceHandle));
	ZeroMemory(&m_encodeHandle		, sizeof(m_encodeHandle));
	ZeroMemory(&m_configCtrl		, sizeof(m_configCtrl));

	m_oveContext					= NULL;

	m_frameIn						= 0;
	m_frameOut						= 0;
}
//////////////////////////////////////////////////////////////////////
CVCEEncoder::~CVCEEncoder()
{
	StopEncoding();
	Cleanup();
	DestroyFrames();
}
//////////////////////////////////////////////////////////////////////
void CVCEEncoder::DestroyFrame(LPVCE_ENCODED_FRAME lpFrame)
{
	if (lpFrame) 
	{
		if (lpFrame->lpData)
			delete [] lpFrame->lpData;

		delete lpFrame;
	}
}
//////////////////////////////////////////////////////////////////////
void CVCEEncoder::DestroyFrames()
{
	POSITION pos = GetHeadPosition();

	while (pos)
		DestroyFrame(GetNext(pos));

	RemoveAll();
}
//////////////////////////////////////////////////////////////////////
int CVCEEncoder::StartEncoding(int width, int height, int framerate, int targetBitrate, int device, LPCSTR lpConfigFile)
{
	//stop previous encoding session and cleanup previous OpenCL objects

	StopEncoding();
	Cleanup();

	cl_int err;

	//reset variables

	m_frameIn		= 0;
	m_frameOut		= 0;

	ZeroMemory(&m_deviceHandle		, sizeof(m_deviceHandle));
	ZeroMemory(&m_encodeHandle		, sizeof(m_encodeHandle));
	ZeroMemory(&m_configCtrl		, sizeof(m_configCtrl));

	//////////////////////////////////////////////////////////////////////
	//STEP 1	: init encoder config
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//STEP 1.1	: prepare default config map
	//////////////////////////////////////////////////////////////////////

	CMapStringToString configMap;

	// EncodeSpecifications
	configMap.SetAt("pictureHeight"							, "1080"); 
	configMap.SetAt("pictureWidth"							, "1920"); 
	configMap.SetAt("EncodeMode"							, "1"); 
	configMap.SetAt("level"									, "40"); 
	configMap.SetAt("profile"								, "77"); 
	configMap.SetAt("pictureFormat"							, "1"); 
	configMap.SetAt("requestedPriority"						, "2"); 

	// ConfigPicCtl
	configMap.SetAt("useConstrainedIntraPred"				, "0"); 
	configMap.SetAt("CABACEnable"							, "0"); 
	configMap.SetAt("CABACIDC"								, "0"); 
	configMap.SetAt("loopFilterDisable"						, "0"); 
	configMap.SetAt("encLFBetaOffset"						, "0"); 
	configMap.SetAt("encLFAlphaC0Offset"					, "0"); 
	configMap.SetAt("encIDRPeriod"							, "0"); 
	configMap.SetAt("encIPicPeriod"							, "0"); 
	configMap.SetAt("encHeaderInsertionSpacing"				, "0"); 
	configMap.SetAt("encCropLeftOffset"						, "0"); 
	configMap.SetAt("encCropRightOffset"					, "0"); 
	configMap.SetAt("encCropTopOffset"						, "0"); 
	configMap.SetAt("encCropBottomOffset"					, "0"); 
	configMap.SetAt("encNumMBsPerSlice"						, "8160"); 
	configMap.SetAt("encNumSlicesPerFrame"					, "1"); 
	configMap.SetAt("encForceIntraRefresh"					, "0"); 
	configMap.SetAt("encForceIMBPeriod"						, "0"); 
	configMap.SetAt("encInsertVUIParam"						, "0"); 
	configMap.SetAt("encInsertSEIMsg"						, "0"); 

	// ConfigRateCtl 
	configMap.SetAt("encRateControlMethod"					, "4"); 
	configMap.SetAt("encRateControlTargetBitRate"			, "5000000"); 
	configMap.SetAt("encRateControlPeakBitRate"				, "0"); 
	configMap.SetAt("encRateControlFrameRateNumerator"		, "30"); 
	configMap.SetAt("encGOPSize"							, "0"); 
	configMap.SetAt("encRCOptions"							, "0"); 
	configMap.SetAt("encQP_I"								, "22"); 
	configMap.SetAt("encQP_P"								, "22"); 
	configMap.SetAt("encQP_B"								, "0"); 
	configMap.SetAt("encVBVBufferSize"						, "5000000"); 
	configMap.SetAt("encRateControlFrameRateDenominator"	, "1"); 

	// ConfigMotionEstimation				
	configMap.SetAt("IMEDecimationSearch"					, "1"); 
	configMap.SetAt("motionEstHalfPixel"					, "1"); 
	configMap.SetAt("motionEstQuarterPixel"					, "1"); 
	configMap.SetAt("disableFavorPMVPoint"					, "0"); 
	configMap.SetAt("forceZeroPointCenter"					, "1"); 
	configMap.SetAt("LSMVert"								, "0"); 
	configMap.SetAt("encSearchRangeX"						, "16"); 
	configMap.SetAt("encSearchRangeY"						, "16"); 
	configMap.SetAt("encSearch1RangeX"						, "0"); 
	configMap.SetAt("encSearch1RangeY"						, "0"); 
	configMap.SetAt("disable16x16Frame1"					, "0"); 
	configMap.SetAt("disableSATD"							, "0"); 
	configMap.SetAt("enableAMD"								, "0"); 
	configMap.SetAt("encDisableSubMode"						, "0"); 
	configMap.SetAt("encIMESkipX"							, "0"); 
	configMap.SetAt("encIMESkipY"							, "0"); 
	configMap.SetAt("encEnImeOverwDisSubm"					, "0"); 
	configMap.SetAt("encImeOverwDisSubmNo"					, "0"); 
	configMap.SetAt("encIME2SearchRangeX"					, "4"); 
	configMap.SetAt("encIME2SearchRangeY"					, "4"); 

	// ConfigRDO
	configMap.SetAt("encDisableTbePredIFrame"				, "0"); 
	configMap.SetAt("encDisableTbePredPFrame"				, "0"); 
	configMap.SetAt("useFmeInterpolY"						, "0"); 
	configMap.SetAt("useFmeInterpolUV"						, "0"); 
	configMap.SetAt("enc16x16CostAdj"						, "0"); 
	configMap.SetAt("encSkipCostAdj"						, "0"); 
	configMap.SetAt("encForce16x16skip"						, "0"); 

	//////////////////////////////////////////////////////////////////////
	//STEP 1.2	: override config map from config file
	//////////////////////////////////////////////////////////////////////

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

				if (configMap.Lookup(szLValue, strTemp))
					configMap.SetAt(szLValue, szRValue);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////	
	//STEP 1.3	: fill config using the map
	//////////////////////////////////////////////////////////////////////

	// fill general configuration structures    
	m_configCtrl.height												= GetConfigMap(&configMap, "pictureHeight");
	m_configCtrl.width												= GetConfigMap(&configMap, "pictureWidth");
	m_configCtrl.encodeMode											= (OVE_ENCODE_MODE)GetConfigMap(&configMap, "EncodeMode");

	// fill profile and level
	m_configCtrl.profileLevel.level									= GetConfigMap(&configMap, "level");
	m_configCtrl.profileLevel.profile								= GetConfigMap(&configMap, "profile");
	m_configCtrl.pictFormat											= (OVE_PICTURE_FORMAT		)GetConfigMap(&configMap, "pictureFormat");
	m_configCtrl.priority											= (OVE_ENCODE_TASK_PRIORITY	)GetConfigMap(&configMap, "requestedPriority");

	// fill picture control structures
	m_configCtrl.pictControl.size									= sizeof(OVE_CONFIG_PICTURE_CONTROL);
	m_configCtrl.pictControl.useConstrainedIntraPred				= GetConfigMap(&configMap, "useConstrainedIntraPred");
	m_configCtrl.pictControl.cabacEnable							= GetConfigMap(&configMap, "CABACEnable");
	m_configCtrl.pictControl.cabacIDC								= GetConfigMap(&configMap, "CABACIDC");
	m_configCtrl.pictControl.loopFilterDisable						= GetConfigMap(&configMap, "loopFilterDisable");
	m_configCtrl.pictControl.encLFBetaOffset						= GetConfigMap(&configMap, "encLFBetaOffset");
	m_configCtrl.pictControl.encLFAlphaC0Offset						= GetConfigMap(&configMap, "encLFAlphaC0Offset");
	m_configCtrl.pictControl.encIDRPeriod							= GetConfigMap(&configMap, "encIDRPeriod");
	m_configCtrl.pictControl.encIPicPeriod							= GetConfigMap(&configMap, "encIPicPeriod");
	m_configCtrl.pictControl.encHeaderInsertionSpacing				= GetConfigMap(&configMap, "encHeaderInsertionSpacing");
	m_configCtrl.pictControl.encCropLeftOffset						= GetConfigMap(&configMap, "encCropLeftOffset");
	m_configCtrl.pictControl.encCropRightOffset						= GetConfigMap(&configMap, "encCropRightOffset");
	m_configCtrl.pictControl.encCropTopOffset						= GetConfigMap(&configMap, "encCropTopOffset");
	m_configCtrl.pictControl.encCropBottomOffset					= GetConfigMap(&configMap, "encCropBottomOffset");
	m_configCtrl.pictControl.encNumMBsPerSlice						= GetConfigMap(&configMap, "encNumMBsPerSlice");
	m_configCtrl.pictControl.encNumSlicesPerFrame					= GetConfigMap(&configMap, "encNumSlicesPerFrame");
	m_configCtrl.pictControl.encForceIntraRefresh					= GetConfigMap(&configMap, "encForceIntraRefresh");
	m_configCtrl.pictControl.encForceIMBPeriod						= GetConfigMap(&configMap, "encForceIMBPeriod");
	m_configCtrl.pictControl.encInsertVUIParam						= GetConfigMap(&configMap, "encInsertVUIParam");
	m_configCtrl.pictControl.encInsertSEIMsg						= GetConfigMap(&configMap, "encInsertSEIMsg");

	// fill rate control structures
	m_configCtrl.rateControl.size									= sizeof(OVE_CONFIG_RATE_CONTROL);
	m_configCtrl.rateControl.encRateControlMethod					= GetConfigMap(&configMap, "encRateControlMethod");
	m_configCtrl.rateControl.encRateControlTargetBitRate			= GetConfigMap(&configMap, "encRateControlTargetBitRate");
	m_configCtrl.rateControl.encRateControlPeakBitRate				= GetConfigMap(&configMap, "encRateControlPeakBitRate");
	m_configCtrl.rateControl.encRateControlFrameRateNumerator		= GetConfigMap(&configMap, "encRateControlFrameRateNumerator");
	m_configCtrl.rateControl.encGOPSize								= GetConfigMap(&configMap, "encGOPSize");
	m_configCtrl.rateControl.encRCOptions							= GetConfigMap(&configMap, "encRCOptions");
	m_configCtrl.rateControl.encQP_I								= GetConfigMap(&configMap, "encQP_I");
	m_configCtrl.rateControl.encQP_P								= GetConfigMap(&configMap, "encQP_P");
	m_configCtrl.rateControl.encQP_B								= GetConfigMap(&configMap, "encQP_B");
	m_configCtrl.rateControl.encVBVBufferSize						= GetConfigMap(&configMap, "encVBVBufferSize");
	m_configCtrl.rateControl.encRateControlFrameRateDenominator		= GetConfigMap(&configMap, "encRateControlFrameRateDenominator");

	// fill motion estimation control structures
	m_configCtrl.meControl.size										= sizeof(OVE_CONFIG_MOTION_ESTIMATION);
	m_configCtrl.meControl.imeDecimationSearch						= GetConfigMap(&configMap, "IMEDecimationSearch");
	m_configCtrl.meControl.motionEstHalfPixel						= GetConfigMap(&configMap, "motionEstHalfPixel");
	m_configCtrl.meControl.motionEstQuarterPixel					= GetConfigMap(&configMap, "motionEstQuarterPixel");
	m_configCtrl.meControl.disableFavorPMVPoint						= GetConfigMap(&configMap, "disableFavorPMVPoint");
	m_configCtrl.meControl.forceZeroPointCenter						= GetConfigMap(&configMap, "forceZeroPointCenter");
	m_configCtrl.meControl.lsmVert									= GetConfigMap(&configMap, "LSMVert");
	m_configCtrl.meControl.encSearchRangeX							= GetConfigMap(&configMap, "encSearchRangeX");
	m_configCtrl.meControl.encSearchRangeY							= GetConfigMap(&configMap, "encSearchRangeY");
	m_configCtrl.meControl.encSearch1RangeX							= GetConfigMap(&configMap, "encSearch1RangeX");
	m_configCtrl.meControl.encSearch1RangeY							= GetConfigMap(&configMap, "encSearch1RangeY");
	m_configCtrl.meControl.disable16x16Frame1						= GetConfigMap(&configMap, "disable16x16Frame1");
	m_configCtrl.meControl.disableSATD								= GetConfigMap(&configMap, "disableSATD");
	m_configCtrl.meControl.enableAMD								= GetConfigMap(&configMap, "enableAMD");
	m_configCtrl.meControl.encDisableSubMode						= GetConfigMap(&configMap, "encDisableSubMode");
	m_configCtrl.meControl.encIMESkipX								= GetConfigMap(&configMap, "encIMESkipX");
	m_configCtrl.meControl.encIMESkipY								= GetConfigMap(&configMap, "encIMESkipY");
	m_configCtrl.meControl.encEnImeOverwDisSubm						= GetConfigMap(&configMap, "encEnImeOverwDisSubm");
	m_configCtrl.meControl.encImeOverwDisSubmNo						= GetConfigMap(&configMap, "encImeOverwDisSubmNo");
	m_configCtrl.meControl.encIME2SearchRangeX						= GetConfigMap(&configMap, "encIME2SearchRangeX");
	m_configCtrl.meControl.encIME2SearchRangeY						= GetConfigMap(&configMap, "encIME2SearchRangeY");

	// fill RDO control structures
	m_configCtrl.rdoControl.size									= sizeof(OVE_CONFIG_RDO);
	m_configCtrl.rdoControl.encDisableTbePredIFrame					= GetConfigMap(&configMap, "encDisableTbePredIFrame");
	m_configCtrl.rdoControl.encDisableTbePredPFrame					= GetConfigMap(&configMap, "encDisableTbePredPFrame");
	m_configCtrl.rdoControl.useFmeInterpolY							= GetConfigMap(&configMap, "useFmeInterpolY");
	m_configCtrl.rdoControl.useFmeInterpolUV						= GetConfigMap(&configMap, "useFmeInterpolUV");
	m_configCtrl.rdoControl.enc16x16CostAdj							= GetConfigMap(&configMap, "enc16x16CostAdj");
	m_configCtrl.rdoControl.encSkipCostAdj							= GetConfigMap(&configMap, "encSkipCostAdj");
	m_configCtrl.rdoControl.encForce16x16skip						= GetConfigMap(&configMap, "encForce16x16skip");

	//////////////////////////////////////////////////////////////////////
	//STEP 1.4	: override specified config parameters
	//////////////////////////////////////////////////////////////////////

	m_configCtrl.width												= width;
	m_configCtrl.height												= height;

	if ((framerate == 24) ||
		(framerate == 25) ||
		(framerate == 30) ||
		(framerate == 50) ||
		(framerate == 60))
		//VCE supports limited set of framerates
		m_configCtrl.rateControl.encRateControlFrameRateNumerator	= framerate;
	else
		m_configCtrl.rateControl.encRateControlFrameRateNumerator	= 30;

	m_configCtrl.rateControl.encRateControlTargetBitRate			= targetBitrate * 1000;
	m_configCtrl.rateControl.encVBVBufferSize						= targetBitrate * 2000;
	m_configCtrl.pictControl.encIDRPeriod							= framerate;
	m_configCtrl.pictControl.encHeaderInsertionSpacing				= framerate;
	m_configCtrl.pictControl.encNumMBsPerSlice						= (ALIGN(width, 16) / 16) * (ALIGN(height, 16) / 16);

	//////////////////////////////////////////////////////////////////////
	//STEP 2	: enumerate OpenVideo encoder devices
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//STEP 2.1	: select AMD OpenCL platform
	//////////////////////////////////////////////////////////////////////

	//enumerate all OpenCL platforms avaiable in the system

	cl_uint numPlatforms;

	err = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (err != CL_SUCCESS)
		return VCE_ERR_UNSUPPORTED;

	// if there are multiple platforms available, search for an AMD system

	if (numPlatforms) 
	{
		cl_platform_id* lpPlatforms = new cl_platform_id[numPlatforms];

		err = clGetPlatformIDs(numPlatforms, lpPlatforms, NULL);

		if (err != CL_SUCCESS) 
		{
			delete [] lpPlatforms;

			return VCE_ERR_CLGETPLATFORMIDS_FAILED;
		}

		// loop through the platforms looking for the first AMD system

		for (unsigned int iPlatform=0; iPlatform<numPlatforms; ++iPlatform) 
		{
			char szPlatformVendor[MAX_PATH];

			err = clGetPlatformInfo(lpPlatforms[iPlatform], CL_PLATFORM_VENDOR, sizeof(szPlatformVendor), szPlatformVendor, NULL);

			// stop at the first platform that is an AMD system

			if (!strcmp(szPlatformVendor, "Advanced Micro Devices, Inc.")) 
			{
				m_deviceHandle.platform = lpPlatforms[iPlatform];
				break;
			}
		}

		delete [] lpPlatforms;
	}

	if (!m_deviceHandle.platform) 
		return VCE_ERR_UNSUPPORTED;

	//////////////////////////////////////////////////////////////////////
	//STEP 2.2	: ensure that selected AMD OpenCL platform is GPU accelerated
	//////////////////////////////////////////////////////////////////////

	cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)m_deviceHandle.platform, 0};

	cl_context context = clCreateContextFromType(cps, CL_DEVICE_TYPE_GPU, NULL, NULL, &err);

	if (err == CL_DEVICE_NOT_FOUND)
		return VCE_ERR_UNSUPPORTED;

	if (context)
		clReleaseContext(context);

	//////////////////////////////////////////////////////////////////////
	//STEP 2.3	: enumerate OpenVideo encoder devices for selected AMD OpenCL platform
	//////////////////////////////////////////////////////////////////////

	if (!OVEncodeGetDeviceInfo(&m_deviceHandle.numDevices, 0))
		return VCE_ERR_OVENCODEGETDEVICEINFO_FAILED;

	if (!m_deviceHandle.numDevices)
		return VCE_ERR_UNSUPPORTED;

	//allocate memory for device info array

	m_deviceHandle.lpDeviceInfo = new ovencode_device_info[m_deviceHandle.numDevices];

	ZeroMemory(m_deviceHandle.lpDeviceInfo, sizeof(ovencode_device_info) * m_deviceHandle.numDevices);

	if (!OVEncodeGetDeviceInfo(&m_deviceHandle.numDevices, m_deviceHandle.lpDeviceInfo))
		return VCE_ERR_OVENCODEGETDEVICEINFO_FAILED;

	//validate specified device index and device ID
	
	if (device >= (int)m_deviceHandle.numDevices)
		return VCE_ERR_INVALID_DEVICE;

	unsigned int deviceId = m_deviceHandle.lpDeviceInfo[device].device_id;

	if (!deviceId)
		return VCE_ERR_INVALID_DEVICE;

	//////////////////////////////////////////////////////////////////////
	//STEP 2	: create the encoder context on the device specified by device ID   
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//STEP 2.1	: create OpenCL context   
	//////////////////////////////////////////////////////////////////////

	intptr_t properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)m_deviceHandle.platform, 0};

	//get OpenCL device ID from OpenVideo device ID

	cl_device_id clDeviceID = reinterpret_cast<cl_device_id>(deviceId);

	//create OpenCL context from device ID

	m_oveContext = clCreateContext(properties, 1, &clDeviceID, 0, 0, &err);

	if (m_oveContext == (cl_context)0) 
		return VCE_ERR_CLCREATECONTEXT_FAILED;

	if (err != CL_SUCCESS) 
		return VCE_ERR_CLCREATECONTEXT_FAILED;

	//////////////////////////////////////////////////////////////////////
	//STEP 2.2	: get OpenVideo encoder capabilities   
	//////////////////////////////////////////////////////////////////////

	OVE_ENCODE_CAPS			encodeCaps;
	OVE_ENCODE_CAPS_H264	encode_cap_full;

	unsigned int			numCaps = 1;

	// initialize the encode capabilities variable    

	encodeCaps.EncodeModes										= OVE_AVC_FULL;
	encodeCaps.encode_cap_size									= sizeof(OVE_ENCODE_CAPS);
	encodeCaps.caps.encode_cap_full								= (OVE_ENCODE_CAPS_H264*)&encode_cap_full;
	encodeCaps.caps.encode_cap_full->max_picture_size_in_MB    = 0;
	encodeCaps.caps.encode_cap_full->min_picture_size_in_MB    = 0;
	encodeCaps.caps.encode_cap_full->num_picture_formats       = 0;
	encodeCaps.caps.encode_cap_full->num_Profile_level         = 0;
	encodeCaps.caps.encode_cap_full->max_bit_rate              = 0;
	encodeCaps.caps.encode_cap_full->min_bit_rate              = 0;
	encodeCaps.caps.encode_cap_full->supported_task_priority   = OVE_ENCODE_TASK_PRIORITY_NONE;

	for(int j=0; j<OVE_MAX_NUM_PICTURE_FORMATS_H264; j++)
		encodeCaps.caps.encode_cap_full->supported_picture_formats[j] = OVE_PICTURE_FORMAT_NONE;

	for(int j=0; j<OVE_MAX_NUM_PROFILE_LEVELS_H264; j++)
	{
		encodeCaps.caps.encode_cap_full->supported_profile_level[j].profile = 0;
		encodeCaps.caps.encode_cap_full->supported_profile_level[j].level   = 0;
	}

	// get the device capabilities

	if (!OVEncodeGetDeviceCap(m_oveContext, deviceId, encodeCaps.encode_cap_size, &numCaps, &encodeCaps))
		return VCE_ERR_OVENCODEGETDEVICECAP_FAILED;

	//////////////////////////////////////////////////////////////////////
	//STEP 2.3	: validate specified encoder configuration 
	//////////////////////////////////////////////////////////////////////

	//TODO

	//////////////////////////////////////////////////////////////////////
	// STEP 3	: initialize the encoder session with specified configuration 
	//////////////////////////////////////////////////////////////////////

	OVresult  res = 0;

	//create OpenVideo Encode session

	m_encodeHandle.session = OVEncodeCreateSession(m_oveContext,	// platform context
									deviceId,						// device id 
									m_configCtrl.encodeMode,		// encode mode
									m_configCtrl.profileLevel,		// encode profile
									m_configCtrl.pictFormat,		// encode format
									m_configCtrl.width,				// width
									m_configCtrl.height,			// height 
									m_configCtrl.priority);			// encode task priority, ie. FOR POSSIBLY LOW LATENCY OVE_ENCODE_TASK_PRIORITY_LEVEL2 

	if (!m_encodeHandle.session) 
		return VCE_ERR_OVENCODECREATESESSION_FAILED;

	//configure the encoding engine

	unsigned int	numOfConfigBuffers = 4;
	OVE_CONFIG		configBuffers[4];

	//send configuration values for this session

	configBuffers[0].config.pPictureControl     = &(m_configCtrl.pictControl);
	configBuffers[0].configType                 = OVE_CONFIG_TYPE_PICTURE_CONTROL;
	configBuffers[1].config.pRateControl        = &(m_configCtrl.rateControl);
	configBuffers[1].configType                 = OVE_CONFIG_TYPE_RATE_CONTROL;
	configBuffers[2].config.pMotionEstimation   = &(m_configCtrl.meControl);
	configBuffers[2].configType                 = OVE_CONFIG_TYPE_MOTION_ESTIMATION;
	configBuffers[3].config.pRDO                = &(m_configCtrl.rdoControl);
	configBuffers[3].configType                 = OVE_CONFIG_TYPE_RDO;

	if (!OVEncodeSendConfig (m_encodeHandle.session, numOfConfigBuffers, configBuffers))
		return VCE_ERR_OVENCODESENDCONFIG_FAILED;

	//create a command queue

	m_encodeHandle.clCmdQueue = clCreateCommandQueue((cl_context)m_oveContext, clDeviceID, 0, &err);

	if (err != CL_SUCCESS)
		return VCE_ERR_CLCREATECOMMANDQUEUE_FAILED;

	// calculate surface size

	unsigned int	alignedSurfaceWidth		= ALIGN(m_configCtrl.width, 256);
    unsigned int	alignedSurfaceHeight	= ALIGN(m_configCtrl.height, 32);
	int				alignedSurfaceSize		= alignedSurfaceHeight * alignedSurfaceWidth * 3 / 2; 

	for(int iSurface=0; iSurface<MAX_INPUT_SURFACE; iSurface++)
	{
		m_encodeHandle.inputSurfaces[iSurface] = clCreateBuffer((cl_context)m_oveContext, CL_MEM_READ_WRITE, alignedSurfaceSize, NULL, &err);

		if (err != CL_SUCCESS) 
			return VCE_ERR_CLCREATEBUFFER_FAILED;
	}

	//init header, PPS and SPS buffers

	ZeroMemory(&m_header			, sizeof(m_header));
	m_headerSize					= 0;

	ZeroMemory(&m_pps				, sizeof(m_pps));
	m_ppsSize						= 0;

	ZeroMemory(&m_sps				, sizeof(m_sps));
	m_spsSize						= 0;

	//get OpenCL platform name

	char szPlatformName[MAX_PATH]	= { 0 };
	err = clGetPlatformInfo(m_deviceHandle.platform, CL_PLATFORM_NAME, sizeof(szPlatformName), szPlatformName, NULL);

	//get OpenCL device name

	size_t size;
	char szDeviceName[MAX_PATH]		= { 0 };
	err = clGetDeviceInfo(clDeviceID, CL_DEVICE_NAME, sizeof(szDeviceName), szDeviceName, &size);

	sprintf_s(m_szDesc, "AMD VCE H.264 hardware accelerated via %s on device %d (%s)", szPlatformName, device, szDeviceName);

	return VCE_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
int CVCEEncoder::Encode(int width, int height, LPBYTE lpImage, DWORDLONG qwTimestamp)
{
	cl_int			err;

	//get input surface handle

	OPMemHandle		inputSurface			= m_encodeHandle.inputSurfaces[m_frameIn % MAX_INPUT_SURFACE];

	//calculate surface size

	unsigned int	alignedSurfaceWidth		= ALIGN(width, 256);
    unsigned int	alignedSurfaceHeight	= ALIGN(height, 32);
	int				alignedSurfaceSize		= alignedSurfaceHeight * alignedSurfaceWidth * 3 / 2; 

	//map input surface

	cl_event	mapEvent;
	cl_int		status;

	LPVOID		lpMappedBuffer = clEnqueueMapBuffer(m_encodeHandle.clCmdQueue, (cl_mem)inputSurface, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, alignedSurfaceSize, 0, NULL, &mapEvent, &status);

	if (!lpMappedBuffer)
		return VCE_ERR_CLENQUEUEMAPBUFFER_FAILED;

	//wait for map operation to complete

    clFlush(m_encodeHandle.clCmdQueue);
	WaitForEvent(mapEvent);
	clReleaseEvent(mapEvent);
		
	//read frame into mapped input surface

	LoadFrame((LPBYTE)lpMappedBuffer, width, height, lpImage, qwTimestamp);

	//unmap input surface
	
	cl_event unmapEvent;

	clEnqueueUnmapMemObject(m_encodeHandle.clCmdQueue, (cl_mem)inputSurface, lpMappedBuffer, 0, NULL, &unmapEvent);

	//wait for unmap operation to complete

	clFlush(m_encodeHandle.clCmdQueue);
	WaitForEvent(unmapEvent);
	clReleaseEvent(unmapEvent);
	
	//use the input surface buffer as our picture
		
	OVE_INPUT_DESCRIPTION encodeTaskInputBuffer;
	encodeTaskInputBuffer.bufferType			= OVE_BUFFER_TYPE_PICTURE;
	encodeTaskInputBuffer.buffer.pPicture		= (OVE_SURFACE_HANDLE)inputSurface;

    //setup the picture parameters

    OVE_ENCODE_PARAMETERS_H264	pictureParameter;
	ZeroMemory(&pictureParameter, sizeof(pictureParameter));

	pictureParameter.size						= sizeof(OVE_ENCODE_PARAMETERS_H264);
	pictureParameter.flags.value				= 0;
	pictureParameter.flags.flags.reserved		= 0;
	pictureParameter.insertSPS					= m_frameIn ? false : true;
	pictureParameter.pictureStructure			= OVE_PICTURE_STRUCTURE_H264_FRAME;
	pictureParameter.forceRefreshMap			= true;
	pictureParameter.forceIMBPeriod				= 0;
	pictureParameter.forcePicType				= OVE_PICTURE_TYPE_H264_NONE;

	//encode a single picture

    unsigned int	taskID						= 0;
    OPEventHandle	eventRunVideoProgram;

    if (!OVEncodeTask(m_encodeHandle.session, 1, &encodeTaskInputBuffer, &pictureParameter, &taskID, 0, NULL, &eventRunVideoProgram)) 
		return VCE_ERR_OVENCODETASK_FAILED;
		 
	//wait for encode session to complete

	err = clWaitForEvents(1, (cl_event*)&eventRunVideoProgram);
	if (err != CL_SUCCESS) 
		return VCE_ERR_CLWAITFOREVENTS_FAILED;

	//query output tasks

    unsigned int numTaskDescriptionsReturned	= 0;

    OVE_OUTPUT_DESCRIPTION	taskDescription;
	ZeroMemory(&taskDescription, sizeof(taskDescription));

    taskDescription.size = sizeof(OVE_OUTPUT_DESCRIPTION);

    do
    {
		if (!OVEncodeQueryTaskDescription(m_encodeHandle.session, 1, &numTaskDescriptionsReturned, &taskDescription))
			return VCE_ERR_OVENCODEQUERYTASKDESCRIPTION_FAILED;
	} 
	while (numTaskDescriptionsReturned && (taskDescription.status == OVE_TASK_STATUS_NONE));

    // save compressed frames

    if (numTaskDescriptionsReturned)
    {
		 if ((taskDescription.status == OVE_TASK_STATUS_COMPLETE) &&  taskDescription.size_of_bitstream_data)
		{
			SaveFrame(&taskDescription);

			m_frameOut++;
            
			OVEncodeReleaseTask(m_encodeHandle.session, taskDescription.taskID);
        }
    }
	
	if (eventRunVideoProgram)
		clReleaseEvent((cl_event)eventRunVideoProgram);

	m_frameIn++;

	return VCE_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
int CVCEEncoder::StopEncoding()
{
	//release input surfaces

	cl_int err;

	for (int iSurface=0; iSurface<MAX_INPUT_SURFACE ;iSurface++)
	{
		if (m_encodeHandle.inputSurfaces[iSurface])
		{
			err = clReleaseMemObject((cl_mem)m_encodeHandle.inputSurfaces[iSurface]);

			if (err != CL_SUCCESS)
				return VCE_ERR_CLRELEASEMEMOBJECT_FAILED;

			m_encodeHandle.inputSurfaces[iSurface] = NULL;
		}
	}

	//release command queue

	if (m_encodeHandle.clCmdQueue)
	{
		err = clReleaseCommandQueue(m_encodeHandle.clCmdQueue);

		if (err != CL_SUCCESS)
			return VCE_ERR_CLRELEASECOMMANDQUEUE_FAILED;

		m_encodeHandle.clCmdQueue = NULL;
	}

	//destroy OpenVideo Encode session

	bool bResult = true;

	if (m_encodeHandle.session)
	{
		if (!OVEncodeDestroySession(m_encodeHandle.session))
			return VCE_ERR_OVENCODEDESTROYSESSION_FAILED;

		m_encodeHandle.session = NULL;
	}

	//release OpenCL context

	if ((cl_context)m_oveContext) 
	{
		err = clReleaseContext((cl_context)m_oveContext);

		if (err != CL_SUCCESS) 
			return VCE_ERR_CLRELEASECONTEXT_FAILED;

		m_oveContext = NULL;
	}

	//cleanup application allocated memory

	Cleanup();

	return VCE_ERR_NONE;
}
//////////////////////////////////////////////////////////////////////
void CVCEEncoder::Cleanup()
{
	if (m_deviceHandle.lpDeviceInfo)
		delete [] m_deviceHandle.lpDeviceInfo;
	m_deviceHandle.lpDeviceInfo = NULL;
}
//////////////////////////////////////////////////////////////////////
LPBYTE CVCEEncoder::GetHeader()
{
	return m_header;
}
//////////////////////////////////////////////////////////////////////
DWORD CVCEEncoder::GetHeaderSize()
{
	return m_headerSize;
}
//////////////////////////////////////////////////////////////////////
void CVCEEncoder::AddAnnexB(LPBYTE lpSrc, DWORD dwSize, LPVCE_ENCODED_FRAME lpFrame)
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
void CVCEEncoder::AddNAL(LPBYTE lpSrc, DWORD dwSize, LPVCE_ENCODED_FRAME lpFrame)
{
	BYTE NAL = lpSrc[0] & 0x1F;

	switch (NAL)
	{
	case 0x05:	//IDR Picture
		lpFrame->dwFrameType |= VCE_FRAMETYPE_I;
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
LPVCE_ENCODED_FRAME CVCEEncoder::GetFrame()
{
	POSITION pos = GetHeadPosition();

	if (pos)
	{
		LPVCE_ENCODED_FRAME lpFrame = GetAt(pos);

		RemoveAt(pos);

		return lpFrame;
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////
LPCSTR CVCEEncoder::GetDesc()
{
	return m_szDesc;
}
//////////////////////////////////////////////////////////////////////
int CVCEEncoder::GetConfigMap(CMapStringToString* lpConfigMap, LPCSTR lpName)
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
void CVCEEncoder::WaitForEvent(cl_event clEvent)
{
	cl_int	eventStatus = CL_QUEUED;
	cl_int  status;

	while (eventStatus != CL_COMPLETE)
	{
		status = clGetEventInfo(clEvent, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &eventStatus, NULL);
	}
}
//////////////////////////////////////////////////////////////////////
void CVCEEncoder::SaveFrame(OVE_OUTPUT_DESCRIPTION* lpTask)
{
	LPVCE_ENCODED_FRAME lpFrame	= (LPVCE_ENCODED_FRAME) new BYTE[sizeof(VCE_ENCODED_FRAME)];

	ZeroMemory(lpFrame, sizeof(VCE_ENCODED_FRAME));

	lpFrame->qwTimestamp		= m_qwTimestamp[m_frameOut % MAX_INPUT_TIMESTAMP];

	AddAnnexB((LPBYTE)lpTask->bitstream_data, lpTask->size_of_bitstream_data, lpFrame);

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
void CVCEEncoder::LoadFrame(LPBYTE lpDst, int width, int height, LPBYTE lpSrc, DWORDLONG qwTimestamp)
{
	DWORD dwSrcPitch = width;
	DWORD dwDstPitch = ALIGN(width, 256);

	for (int y=0; y<height; y++)
		CopyMemory(lpDst + y * dwDstPitch, lpSrc + y * dwSrcPitch, width);

	LPBYTE lpSrcUV = lpSrc + dwSrcPitch * height;
	LPBYTE lpDstUV = lpDst + dwDstPitch * height;

	for (int y=0; y<height/2; y++)
		CopyMemory(lpDstUV + y * dwDstPitch, lpSrcUV + y * dwSrcPitch, width);

	m_qwTimestamp[m_frameIn % MAX_INPUT_TIMESTAMP] = qwTimestamp;
}
//////////////////////////////////////////////////////////////////////
void CVCEEncoder::InitHeader()
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
LPCSTR CVCEEncoder::GetStatusStr(int status)
{
	switch (status) 
	{
	case VCE_ERR_NONE:
		return "VCE_ERR_NONE";
	case VCE_ERR_UNKNOWN:
		return "VCE_ERR_UNKNOWN";
	case VCE_ERR_NULL_PTR:
		return "VCE_ERR_NULL_PTR";
	case VCE_ERR_UNSUPPORTED:
		return "VCE_ERR_UNSUPPORTED";
	case VCE_ERR_CLGETPLATFORMIDS_FAILED:
		return "VCE_ERR_CLGETPLATFORMIDS_FAILED";
	case VCE_ERR_INVALID_DEVICE:
		return "VCE_ERR_INVALID_DEVICE";
	case VCE_ERR_OVENCODEGETDEVICEINFO_FAILED:
		return "VCE_ERR_OVENCODEGETDEVICEINFO_FAILED";
	case VCE_ERR_CLCREATECONTEXT_FAILED:
		return "VCE_ERR_CLCREATECONTEXT_FAILED";
	case VCE_ERR_OVENCODEGETDEVICECAP_FAILED:
		return "VCE_ERR_OVENCODEGETDEVICECAP_FAILED";
	case VCE_ERR_OVENCODECREATESESSION_FAILED:
		return "VCE_ERR_OVENCODECREATESESSION_FAILED";
	case VCE_ERR_OVENCODESENDCONFIG_FAILED:
		return "VCE_ERR_OVENCODESENDCONFIG_FAILED";
	case VCE_ERR_CLCREATECOMMANDQUEUE_FAILED:
		return "VCE_ERR_CLCREATECOMMANDQUEUE_FAILED";
	case VCE_ERR_CLCREATEBUFFER_FAILED:
		return "VCE_ERR_CLCREATEBUFFER_FAILED";
	case VCE_ERR_CLENQUEUEMAPBUFFER_FAILED:
		return "VCE_ERR_CLENQUEUEMAPBUFFER_FAILED";
	case VCE_ERR_OVENCODETASK_FAILED:
		return "VCE_ERR_OVENCODETASK_FAILED";
	case VCE_ERR_CLWAITFOREVENTS_FAILED:
		return "VCE_ERR_CLWAITFOREVENTS_FAILED";
	case VCE_ERR_OVENCODEQUERYTASKDESCRIPTION_FAILED:
		return "VCE_ERR_OVENCODEQUERYTASKDESCRIPTION_FAILED";
	case VCE_ERR_CLRELEASEMEMOBJECT_FAILED:
		return "VCE_ERR_CLRELEASEMEMOBJECT_FAILED";
	case VCE_ERR_CLRELEASECOMMANDQUEUE_FAILED:
		return "VCE_ERR_CLRELEASECOMMANDQUEUE_FAILED";
	case VCE_ERR_OVENCODEDESTROYSESSION_FAILED:
		return "VCE_ERR_OVENCODEDESTROYSESSION_FAILED";
	case VCE_ERR_CLRELEASECONTEXT_FAILED:
		return "VCE_ERR_CLRELEASECONTEXT_FAILED";
	}

	return "UNKNOWN";
}
//////////////////////////////////////////////////////////////////////
