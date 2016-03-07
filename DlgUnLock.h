#pragma once

class DlgUnLock : public CDialog
{
	DECLARE_DYNAMIC(DlgUnLock)

public:
	DlgUnLock(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~DlgUnLock();

	enum { IDD = IDD_UNLOCK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CString m_szPwd;
	afx_msg void OnEnChgPwd();
	afx_msg void OnUnLockOK();
	afx_msg void OnDestroy();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnClkQuit();
};
