#pragma once

#include <vector>

class CInfectedItem
{
public:
	CString m_sFile;
	CString m_sVirus;
};

class CInfItems : public std::vector<CInfectedItem>{};

class CVirusFilesList : public CListCtrl
{
	//
};

class CDCAntivirusLogDlg : public CDialog
{
public:
	CDCAntivirusLogDlg(CWnd *pParent = NULL);
	virtual ~CDCAntivirusLogDlg();

public:
	void SetFilesCount(int nCount) { m_nFilesCount = nCount; };
	void SetInfectedItems(CInfItems items) { m_items = items; };
	void SetTime(CTimeSpan time) { m_time = time; };

protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	void FillList();
	void FillLog();

	void RemoveFromList(std::vector<int> items);


public:
	void OnDel(int nItem);
	void OnQuarantine(int nItem);

private:
	int m_nFilesCount;
	CInfItems m_items;
	CListCtrl m_listLog;
	CTimeSpan m_time; 
public:
	afx_msg void OnRClickListInfected(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuDeletefile();
	afx_msg void OnMenuQuarantinefile();
};
