// VCEConfigurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VCE.h"
#include "VCEEncoder.h"
#include "VCEConfigurationDlg.h"

#include <shlwapi.h>

extern HINSTANCE g_hModule;

// CVCEConfigurationDlg dialog

IMPLEMENT_DYNAMIC(CVCEConfigurationDlg, CDialog)

CVCEConfigurationDlg::CVCEConfigurationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVCEConfigurationDlg::IDD, pParent)
	, m_strTargetBitrateCaption(_T(""))
{
}

CVCEConfigurationDlg::~CVCEConfigurationDlg()
{
}

void CVCEConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRESET_SLIDER, m_presetSlider);
	DDX_Control(pDX, IDC_TARGET_BITRATE_SLIDER, m_targetBitrateSlider);
	DDX_Text(pDX, IDC_TARGET_BITRATE_CAPTION, m_strTargetBitrateCaption);
}


BEGIN_MESSAGE_MAP(CVCEConfigurationDlg, CDialog)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CVCEConfigurationDlg message handlers

BOOL CVCEConfigurationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_presetSlider.SetRange(0,2);
	m_targetBitrateSlider.SetRange(1,50);

	int preset			= GetConfigInt("AVC", "Preset"			, 0									);
	int targetBitrate	= GetConfigInt("AVC", "TargetBitrate"	, 5000								);

	m_presetSlider.SetPos(preset);
	m_targetBitrateSlider.SetPos(targetBitrate / 1000);

	UpdateBitrateControls();

	return TRUE; 
}


int CVCEConfigurationDlg::GetPreset()
{
	return m_presetSlider.GetPos();
}

int CVCEConfigurationDlg::GetTargetBitrate()
{
	return m_targetBitrateSlider.GetPos() * 1000;
}

void CVCEConfigurationDlg::OnOK()
{
	UpdateData(TRUE);

	SetConfigInt("AVC", "Preset"		, GetPreset());
	SetConfigInt("AVC", "TargetBitrate"	, GetTargetBitrate());

	CDialog::OnOK();
}

void CVCEConfigurationDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&m_targetBitrateSlider)
		UpdateBitrateControls();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CVCEConfigurationDlg::UpdateBitrateControls()
{
	m_strTargetBitrateCaption.Format("Target bitrate : %d Mbps", m_targetBitrateSlider.GetPos());

	UpdateData(FALSE);
}
