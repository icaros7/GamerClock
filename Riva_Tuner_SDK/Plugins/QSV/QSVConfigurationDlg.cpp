// QSVConfigurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "QSV.h"
#include "QSVEncoder.h"
#include "QSVConfigurationDlg.h"

#include <shlwapi.h>

extern HINSTANCE g_hModule;

// CQSVConfigurationDlg dialog

IMPLEMENT_DYNAMIC(CQSVConfigurationDlg, CDialog)

CQSVConfigurationDlg::CQSVConfigurationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQSVConfigurationDlg::IDD, pParent)
	, m_strTargetBitrateCaption(_T(""))
{
}

CQSVConfigurationDlg::~CQSVConfigurationDlg()
{
}

void CQSVConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MODE_COMBO, m_d3dModeCombo);
	DDX_Control(pDX, IDC_TARGET_USAGE_SLIDER, m_targetUsageSlider);
	DDX_Control(pDX, IDC_TARGET_BITRATE_SLIDER, m_targetBitrateSlider);
	DDX_Text(pDX, IDC_TARGET_BITRATE_CAPTION, m_strTargetBitrateCaption);
}


BEGIN_MESSAGE_MAP(CQSVConfigurationDlg, CDialog)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CQSVConfigurationDlg message handlers

BOOL CQSVConfigurationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_d3dModeCombo.SetItemData(m_d3dModeCombo.AddString("disabled")		, QSV_ENCODER_MODE_SOFTWARE);

	if (CQSVEncoder::IsD3D9Supported())
		m_d3dModeCombo.SetItemData(m_d3dModeCombo.AddString("Direct3D9")	, QSV_ENCODER_MODE_HARDWARE_D3D9);

	if (CQSVEncoder::IsD3D11Supported())
		m_d3dModeCombo.SetItemData(m_d3dModeCombo.AddString("Direct3D11")	, QSV_ENCODER_MODE_HARDWARE_D3D11);

	m_targetUsageSlider.SetRange(1,7);
	m_targetBitrateSlider.SetRange(1,50);

	int d3dMode			= GetConfigInt("AVC", "D3DMode"			, CQSVEncoder::GetSuggestedMode()	);
	int targetUsage		= GetConfigInt("AVC", "TargetUsage"		, MFX_TARGETUSAGE_BALANCED			);
	int targetBitrate	= GetConfigInt("AVC", "TargetBitrate"	, 5000								);

	m_d3dModeCombo.SetCurSel(0);

	for (int iItem=0; iItem<m_d3dModeCombo.GetCount(); iItem++)
	{
		if (m_d3dModeCombo.GetItemData(iItem) == d3dMode)
		{
			m_d3dModeCombo.SetCurSel(iItem);
			break;
		}

	}

	m_targetUsageSlider.SetPos(8 - targetUsage);
	m_targetBitrateSlider.SetPos(targetBitrate / 1000);

	UpdateBitrateControls();

	return TRUE; 
}

int CQSVConfigurationDlg::GetMode()
{
	int iItem = m_d3dModeCombo.GetCurSel();
	if (iItem != -1)
		return (int)m_d3dModeCombo.GetItemData(iItem);

	return 0;
}

int CQSVConfigurationDlg::GetTargetUsage()
{
	return 8 - m_targetUsageSlider.GetPos();
}

int CQSVConfigurationDlg::GetTargetBitrate()
{
	return m_targetBitrateSlider.GetPos() * 1000;
}

void CQSVConfigurationDlg::OnOK()
{
	UpdateData(TRUE);

	SetConfigInt("AVC", "D3DMode"		, GetMode());
	SetConfigInt("AVC", "TargetUsage"	, GetTargetUsage());
	SetConfigInt("AVC", "TargetBitrate"	, GetTargetBitrate());

	CDialog::OnOK();
}

void CQSVConfigurationDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&m_targetBitrateSlider)
		UpdateBitrateControls();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CQSVConfigurationDlg::UpdateBitrateControls()
{
	m_strTargetBitrateCaption.Format("Target bitrate : %d Mbps", m_targetBitrateSlider.GetPos());

	UpdateData(FALSE);
}
