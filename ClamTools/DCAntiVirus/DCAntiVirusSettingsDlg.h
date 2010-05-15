#pragma once

class CSettingsInfo
{
public:
	CSettingsInfo():
	  m_bScan(FALSE),
	  m_bDeny(FALSE),
	  m_bDeep(FALSE),
	  m_bOffice(FALSE),
	  m_bArchives(FALSE),
	  m_bPDF(FALSE),
	  m_bHTML(FALSE),
	  m_bIdle(FALSE),
	  m_nIdleTime(0),
	  m_nCPULoad(0){};

public:
	BOOL m_bScan;
	BOOL m_bDeny;
	BOOL m_bDeep;
	BOOL m_bOffice;
	BOOL m_bArchives;
	BOOL m_bPDF;
	BOOL m_bHTML;

	BOOL m_bIdle;
	int m_nIdleTime;
	int m_nCPULoad;
};

class CScheduledTask;
class CDCAntiVirusSettingsDlg : public CDialog
{
public:
	CDCAntiVirusSettingsDlg(CWnd* pParent = NULL);
	virtual ~CDCAntiVirusSettingsDlg();

protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	void UpdateOnAccessControls();
	void UpdateIdleScanControls();
	void UpdateAllControls();

private:
	void LoadRegistryData();
	void SaveRegistryData();

	void GetData(CSettingsInfo &info);
	void GetScanData(CSettingsInfo &info);
	void GetIdleData(CSettingsInfo &info);
	bool ControlChecked(UINT ID);

	void SetData(const CSettingsInfo &info);
	void SetScanData(const CSettingsInfo &info);
	void SetIdleData(const CSettingsInfo &info);
	void CheckControl(UINT ID, BOOL bSheck);

	void SetTaskInfo(CScheduledTask *pTask, UINT ID);

public:
	void OnCheckEnableScan();
	void OnCheckEnableIdleScan();
	void OnBnClickedOk();
	void OnChangeSchedScan();
	afx_msg void OnDeleteSchedScan();
	afx_msg void OnChangeUpd();
	afx_msg void OnDeleteUpd();
};
