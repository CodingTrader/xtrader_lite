#pragma once

#include "DIYCtrl/HtmlCtrl.h"
////////////////////////////

class BkrNtDlg : public CDialog
{
	DECLARE_DYNAMIC(BkrNtDlg)

public:
	BkrNtDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~BkrNtDlg();

	CMyHtmlCtrl	m_Page;
	CString		m_szUrl,m_szTitle;
	BOOL		m_bMemory;
	CString		m_sContent;

// �Ի�������
	enum { IDD = IDD_DLG_BKRNT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	DECLARE_MESSAGE_MAP()
};
