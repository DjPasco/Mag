#pragma once


//Dialog to dislplay info about hooked programs.
class CDCAntivirusHookDlg : public CDialog
{
public:
	CDCAntivirusHookDlg();
	virtual ~CDCAntivirusHookDlg();

protected:
	virtual BOOL OnInitDialog();

	void FillProcessTree();

private:
	CTreeCtrl m_treeProc;
};
