#pragma once

#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"

class CVCEAMFConfigurationDlg : public CDialog
{
	DECLARE_DYNAMIC(CVCEAMFConfigurationDlg)

public:
	CVCEAMFConfigurationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVCEAMFConfigurationDlg();

// Dialog Data
	enum { IDD = IDD_VCEAMF_CONFIGURATION_DLG };

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
	int m_nMemoryType;
};
