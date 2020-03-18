#pragma once


// CCustomTabCtrl

class CCustomTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CCustomTabCtrl)

private:
	int m_DialogID[5];
	int m_nPageCount;
	CDialog *m_Dialog[5];
	int m_iCurrentPage;

public:
	CCustomTabCtrl();
	virtual ~CCustomTabCtrl();
	void InitDialogs();
	void ActivateTabDialogs();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult);
};


