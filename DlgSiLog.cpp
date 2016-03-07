#include "stdafx.h"
#include "xTrader.h"
#include "DlgSiLog.h"

extern HANDLE g_hEvent;
IMPLEMENT_DYNAMIC(DlgSiLog, CDialog)
DlgSiLog::DlgSiLog(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SILOGIN, pParent)
{

}

DlgSiLog::~DlgSiLog()
{
}

void DlgSiLog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRGSILOGIN, m_prg);
}

BEGIN_MESSAGE_MAP(DlgSiLog, CDialog)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void DlgSiLog::ProgressUpdate(LPCTSTR szMsg, const int nPercent)
{
    ASSERT (AfxIsValidString(szMsg));
    ASSERT ( nPercent >= 0  &&  nPercent <= 100 );
	
    SetDlgItemText(IDC_SLISTAMSG,szMsg);
    m_prg.SetPos(nPercent);
}

BOOL DlgSiLog::OnInitDialog()
{
	CDialog::OnInitDialog();

	AfxBeginThread((AFX_THREADPROC)LoginThread, this);

	return TRUE; 
}

void DlgSiLog::OnOK()
{
	CDialog::OnOK();
	DestroyWindow();
}

void DlgSiLog::OnCancel()
{
	CDialog::OnCancel();
	DestroyWindow();
}

void DlgSiLog::OnDestroy()
{
	CDialog::OnDestroy();
    delete this;
}

UINT DlgSiLog::LoginThread(LPVOID pParam)
{
	CtpTdSpi* td = gc_Td;
	DlgSiLog* pDlg = static_cast<DlgSiLog*>(pParam);

    td->ClrAllVecs();
	td->InitMgrFee();

	#ifdef _NEED_LOGIN_
	pDlg->ProgressUpdate(_T("ȷ�Ͻ��㵥>>>"), 2);
	td->ReqSetInfConfirm();
	DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("ȷ�Ͻ��㵥�ɹ�!"), 5);
		ResetEvent(g_hEvent);
	}
	else
	{
		pDlg->ProgressUpdate(_T("ȷ�Ͻ��㳬ʱ!"),0);
		goto THREAD_END;
	}
	/////////////////////////////////////////////////
	pDlg->ProgressUpdate(_T("��ѯ������Ϣ>>"), 10);

	td->ReqQryDepthMD(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ������Ϣ�ɹ�!"), 15);
		ResetEvent(g_hEvent);	
	}
	else
	{
		pDlg->ProgressUpdate(_T("��ѯ������Ϣ��ʱ!"),0);
		goto THREAD_END;
	}
	/////////////////////////////////////////////////
	pDlg->ProgressUpdate(_T("��ѯ��Լ�б�>>>"), 18);
	td->ReqQryInst(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ��Լ�б�ɹ�!"), 23);
		ResetEvent(g_hEvent);
	}
	else
	{
		pDlg->ProgressUpdate(_T("��ѯ��Լ��ʱ!"),0);
		goto THREAD_END;
	}
	
	pDlg->ProgressUpdate(_T("��ѯ���ճɽ���Ϣ>>"), 28);
	td->ReqQryTd(NULL,NULL,NULL,NULL,NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ�ɽ���Ϣ�ɹ�!"), 33);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->ProgressUpdate(_T("��ѯ�ɽ���Ϣ��ʱ!"),0);
		goto THREAD_END;
	}
	
	pDlg->ProgressUpdate(_T("��ѯ����ί����Ϣ>>"), 38);
	td->ReqQryOrd(NULL,NULL,NULL,NULL,NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡί����Ϣ�ɹ�!"), 43);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->ProgressUpdate(_T("��ѯί����Ϣ��ʱ!"),0);
		goto THREAD_END;
	}

	pDlg->ProgressUpdate(_T("��ѯ�ֲ���Ϣ>>"),48);
	td->ReqQryInvPos(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ�ֲ���Ϣ�ɹ�!"), 53);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->ProgressUpdate(_T("��ֲ���Ϣ��ʱ!"),0);
		goto THREAD_END;
	}
	
	pDlg->ProgressUpdate(_T("��ѯ�ʽ��˻�>>"), 58);
	
	td->ReqQryTdAcc();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ�ʽ���Ϣ�ɹ�!"), 63);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->ProgressUpdate(_T("���˻���ʱ!"),0);
		goto THREAD_END;
	}
	//////////////////////////////////////////
	pDlg->ProgressUpdate(_T("��ѯ����ǩԼ>>"), 65);
	
	td->ReqQryAccreg();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ������Ϣ�ɹ�!"), 70);
		ResetEvent(g_hEvent);		
	}
	else
	{
		pDlg->ProgressUpdate(_T("��ѯ������Ϣ��ʱ!"),0);
		goto THREAD_END;
	}
	
	pDlg->ProgressUpdate(_T("��ѯ���ױ���>>"), 73);
		
	td->ReqQryTdCode();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ���ױ���ɹ�!"), 78);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->ProgressUpdate(_T("��ѯ���ױ��볬ʱ!"),0);
		goto THREAD_END;
	}
	
	pDlg->ProgressUpdate(_T("��ѯ���Ͳ���>>"), 80);

	td->ReqQryBkrTdParams();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ���Ͳ����ɹ�!"), 85);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->ProgressUpdate(_T("��ѯ���Ͳ�����ʱ!"),0);
		goto THREAD_END;
	}

	pDlg->ProgressUpdate(_T("��ѯԤ��>>"), 90);
	
	td->ReqQryParkedOrd(NULL,NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡԤ�񵥳ɹ�!"), 99);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->ProgressUpdate(_T("��ѯԤ�񵥳�ʱ!"),0);
		goto THREAD_END;
	}
	#endif

	g_Dlg->PostMessage(WM_RELOGINOK);
	
THREAD_END:
	pDlg->PostMessage(WM_CLOSE); 
	return 0;
}