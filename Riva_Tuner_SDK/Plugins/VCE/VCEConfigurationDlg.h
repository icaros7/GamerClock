#pragma once

#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"

class CVCEConfigurationDlg : public CDialog
{
	DECLARE_DYNAMIC(CVCEConfigurationDlg)

public:
	CVCEConfigurationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVCEConfigurationDlg();

// Dialog Data
	enum { IDD = IDD_VCE_CONFIGURATION_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CSliderCtrl m_presetSlider;
	CSliderCtrl m_targetBitrateSlider;
	virtual BOOL OnInitDialog();
	void UpdateBitrateControls();
	int GetPreset();
	int GetTargetBitrate();

protected:
	virtual void OnOK();
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CString m_strTargetBitrateCaption;
};
