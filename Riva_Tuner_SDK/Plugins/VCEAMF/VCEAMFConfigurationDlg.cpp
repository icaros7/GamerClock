// VCEAMFConfigurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VCEAMF.h"
#include "VCEAMFEncoder.h"
#include "VCEAMFConfigurationDlg.h"

#include <shlwapi.h>

extern HINSTANCE g_hModule;

// CVCEAMFConfigurationDlg dialog

IMPLEMENT_DYNAMIC(CVCEAMFConfigurationDlg, CDialog)

CVCEAMFConfigurationDlg::CVCEAMFConfigurationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVCEAMFConfigurationDlg::IDD, pParent)
	, m_strTargetBitrateCaption(_T(""))
	, m_nMemoryType(0)
{
}

CVCEAMFConfigurationDlg::~CVCEAMFConfigurationDlg()
{
}

void CVCEAMFConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRESET_SLIDER, m_presetSlider);
	DDX_Control(pDX, IDC_TARGET_BITRATE_SLIDER, m_targetBitrateSlider);
	DDX_Text(pDX, IDC_TARGET_BITRATE_CAPTION, m_strTargetBitrateCaption);
	DDX_CBIndex(pDX, IDC_MEMORY_TYPE_COMBO, m_nMemoryType);
}


BEGIN_MESSAGE_MAP(CVCEAMFConfigurationDlg, CDialog)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CVCEAMFConfigurationDlg message handlers

BOOL CVCEAMFConfigurationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_presetSlider.SetRange(0,2);
	m_targetBitrateSlider.SetRange(1,50);

	int preset			= GetConfigInt("AVC", "Preset"			, 0									);
	int targetBitrate	= GetConfigInt("AVC", "TargetBitrate"	, 5000								);
	m_nMemoryType		= GetConfigInt("AVC", "MemoryType"		, amf::AMF_MEMORY_DX9				) - 1;

	m_presetSlider.SetPos(preset);
	m_targetBitrateSlider.SetPos(targetBitrate / 1000);

	UpdateBitrateControls();

	return TRUE; 
}


int CVCEAMFConfigurationDlg::GetPreset()
{
	return m_presetSlider.GetPos();
}

int CVCEAMFConfigurationDlg::GetTargetBitrate()
{
	return m_targetBitrateSlider.GetPos() * 1000;
}

void CVCEAMFConfigurationDlg::OnOK()
{
	UpdateData(TRUE);

	SetConfigInt("AVC", "Preset"		, GetPreset());
	SetConfigInt("AVC", "TargetBitrate"	, GetTargetBitrate());
	SetConfigInt("AVC", "MemoryType"	, m_nMemoryType + 1);

	CDialog::OnOK();
}

void CVCEAMFConfigurationDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&m_targetBitrateSlider)
		UpdateBitrateControls();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CVCEAMFConfigurationDlg::UpdateBitrateControls()
{
	m_strTargetBitrateCaption.Format("Target bitrate : %d Mbps", m_targetBitrateSlider.GetPos());

	UpdateData(FALSE);
}
