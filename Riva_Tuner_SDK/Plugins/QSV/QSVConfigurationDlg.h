#pragma once

#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"

class CQSVConfigurationDlg : public CDialog
{
	DECLARE_DYNAMIC(CQSVConfigurationDlg)

public:
	CQSVConfigurationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CQSVConfigurationDlg();

// Dialog Data
	enum { IDD = IDD_QSV_CONFIGURATION_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_d3dModeCombo;
	CSliderCtrl m_targetUsageSlider;
	CSliderCtrl m_targetBitrateSlider;
	virtual BOOL OnInitDialog();
	void UpdateBitrateControls();
	int GetMode();
	int GetTargetUsage();
	int GetTargetBitrate();

protected:
	virtual void OnOK();
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CString m_strTargetBitrateCaption;
};
