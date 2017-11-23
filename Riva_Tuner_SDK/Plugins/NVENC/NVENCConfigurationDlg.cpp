// NVENCConfigurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NVENC.h"
#include "NVENCEncoder.h"
#include "NVENCConfigurationDlg.h"

#include <shlwapi.h>

extern HINSTANCE g_hModule;

// CNVENCConfigurationDlg dialog

IMPLEMENT_DYNAMIC(CNVENCConfigurationDlg, CDialog)

CNVENCConfigurationDlg::CNVENCConfigurationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNVENCConfigurationDlg::IDD, pParent)
	, m_strTargetBitrateCaption(_T(""))
	, m_bLowLatency(FALSE)
{
}

CNVENCConfigurationDlg::~CNVENCConfigurationDlg()
{
}

void CNVENCConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRESET_SLIDER, m_presetSlider);
	DDX_Control(pDX, IDC_TARGET_BITRATE_SLIDER, m_targetBitrateSlider);
	DDX_Text(pDX, IDC_TARGET_BITRATE_CAPTION, m_strTargetBitrateCaption);
	DDX_Check(pDX, IDC_LOW_LATENCY_CHECK, m_bLowLatency);
}


BEGIN_MESSAGE_MAP(CNVENCConfigurationDlg, CDialog)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CNVENCConfigurationDlg message handlers

BOOL CNVENCConfigurationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_presetSlider.SetRange(0,2);
	m_targetBitrateSlider.SetRange(1,130);

	int preset			= GetConfigInt("AVC", "Preset"			, 1									);
	int targetBitrate	= GetConfigInt("AVC", "TargetBitrate"	, 5000								);
	m_bLowLatency		= GetConfigInt("AVC", "LowLatency"		, 1									);

	m_presetSlider.SetPos(preset);
	m_targetBitrateSlider.SetPos(targetBitrate / 1000);

	UpdateBitrateControls();

	return TRUE; 
}


int CNVENCConfigurationDlg::GetPreset()
{
	return m_presetSlider.GetPos();
}

int CNVENCConfigurationDlg::GetTargetBitrate()
{
	return m_targetBitrateSlider.GetPos() * 1000;
}

void CNVENCConfigurationDlg::OnOK()
{
	UpdateData(TRUE);

	SetConfigInt("AVC", "Preset"		, GetPreset());
	SetConfigInt("AVC", "TargetBitrate"	, GetTargetBitrate());
	SetConfigInt("AVC", "LowLatency"	, m_bLowLatency);

	CDialog::OnOK();
}

void CNVENCConfigurationDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&m_targetBitrateSlider)
		UpdateBitrateControls();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CNVENCConfigurationDlg::UpdateBitrateControls()
{
	m_strTargetBitrateCaption.Format("Target bitrate : %d Mbps", m_targetBitrateSlider.GetPos());

	UpdateData(FALSE);
}
