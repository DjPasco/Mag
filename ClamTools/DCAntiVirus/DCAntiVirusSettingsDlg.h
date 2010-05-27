#pragma once

class CScheduledTask;
class CSettingsInfo;
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
	void GetMemoryData(CSettingsInfo &info);
	bool ControlChecked(UINT ID);

	void SetData(const CSettingsInfo &info);
	void SetScanData(const CSettingsInfo &info);
	void SetIdleData(const CSettingsInfo &info);
	void SetMemoryData(const CSettingsInfo &info);
	void CheckControl(UINT ID, BOOL bSheck);

	void SetTaskInfo(CScheduledTask *pTask, UINT ID, LPCSTR sEntry = 0);

	void LoadShedInfo();

	void CheckTasks();

public:
	void OnCheckEnableScan();
	void OnCheckEnableIdleScan();
	void OnBnClickedOk();
	void OnChangeSchedScan();
	void OnDeleteSchedScan();
	void OnChangeUpd();
	void OnDeleteUpd();

private:
	CListCtrl m_list;
	CComboBox m_priority;

public:
	void OnAdd();
	void OnRemove();
	void OnAboutSpecScan();
	void OnQuarDir();
};
