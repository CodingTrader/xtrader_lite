#include "stdafx.h"
#include "xTrader.h"
#include "DlgNetSel.h"
#include "LoginDlg.h"

IMPLEMENT_DYNAMIC(DlgNetSel, CDialog)

DlgNetSel::DlgNetSel(CWnd* pParent /*=NULL*/)
	: CDialog(DlgNetSel::IDD, pParent)
{
	m_bUseProxy=FALSE;
	m_szSkAddr =_T("");
	m_iSkPort = 8000;
	m_szSkUser =_T("");
	m_szSkPwd = _T("");
	m_szGrpName =_T("");
	m_iPxyType = T_SOCKS4;
	m_pTdTest = NULL;
	m_pRun = NULL;
	m_pLogin = NULL;
	m_IsModified = false;
}

DlgNetSel::~DlgNetSel()
{
	m_pLogin = NULL;
}

void DlgNetSel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHK_SOCK, m_bUseProxy);
	DDX_Control(pDX, IDC_CBNETTYPE, m_CombSockLst);
	DDX_Text(pDX, IDC_SOCKSERV, m_szSkAddr);
	DDX_Text(pDX, IDC_SOCK_PORT, m_iSkPort);
	DDX_Text(pDX, IDC_SK_USER, m_szSkUser);
	DDX_Text(pDX, IDC_SKPWD, m_szSkPwd);
	DDX_Control(pDX, IDC_LST_TRADE, m_LstTds);
	DDX_Control(pDX, IDC_LST_MD, m_LstMds);
}

BEGIN_MESSAGE_MAP(DlgNetSel, CDialog)
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LST_TRADE, OnBeginEditTdLst)
	ON_NOTIFY(LVN_DOLABELEDIT, IDC_LST_TRADE, OnDoEditTdLst)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LST_TRADE, OnEndEditTdLst)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LST_MD, OnBeginEditMdLst)
	ON_NOTIFY(LVN_DOLABELEDIT, IDC_LST_MD, OnDoEditMdLst)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LST_MD, OnEndEditMdLst)
	ON_BN_CLICKED(IDC_CHK_SOCK, OnBnClkChkSock)
	ON_EN_CHANGE(IDC_SOCKSERV, OnEnChgPxyServ)
	ON_EN_CHANGE(IDC_SOCK_PORT, OnEnChgPxyPort)
	ON_EN_CHANGE(IDC_SK_USER, OnEnChgPxyUser)
	ON_EN_CHANGE(IDC_SKPWD, OnEnChgPxyPass)
	ON_BN_CLICKED(ID_TESTSPEED, OnTestSpeed)
END_MESSAGE_MAP()

BOOL DlgNetSel::OnInitDialog()
{
	InitCombo();
	InitLists();

	UpdateData(FALSE);
	
    ChkProxy(m_bUseProxy);

	OnTestSpeed();

	return TRUE;
}

void DlgNetSel::ChkProxy(BOOL bUse)
{
	m_CombSockLst.EnableWindow(bUse);
	GetDlgItem(IDC_SOCKSERV)->EnableWindow(bUse);
	GetDlgItem(IDC_SOCK_PORT)->EnableWindow(bUse);
	GetDlgItem(IDC_SK_USER)->EnableWindow(bUse);
	GetDlgItem(IDC_SKPWD)->EnableWindow(bUse);
}

void DlgNetSel::OnTestSpeed()
{
	if (!m_pRun)
	{  m_pRun = AfxBeginThread((AFX_THREADPROC)DlgNetSel::Mthread, this); }
}

void DlgNetSel::GetPxyCfg()
{
	CAppCfgs& s = g_s;
	LoginDlg* pLogin = (LoginDlg*)m_pLogin;
	if (m_bUseProxy)
	{   //#define PROXY_ADDR "%s://%s:%d/%s:%s@%s:%d"
		CString stSock=_T("");

		m_iPxyType = m_CombSockLst.GetCurSel();
		switch (m_iPxyType)
		{
			case T_SOCKS4:
				stSock = SOCK4_PRE;
				break;
			case T_SOCKS4A:
				stSock = SOCK4A_PRE;
				break;
			case T_SOCKS5:
				stSock = SOCK5_PRE;
				break;
		}

		s.m_szProxy.Format(_T("%s%s:%s@%s:%d"),stSock,m_szSkUser,m_szSkPwd,m_szSkAddr,m_iSkPort);
	}
	s.m_bProxy = !!m_bUseProxy;
	if(m_IsModified) 
	{ 
		pLogin->m_szArTs.Copy(m_szArTs);
		pLogin->m_szArMd.Copy(m_szArMd);

		s.RwSvrsbyAr(m_szArTs,m_szArMd,m_szXml,m_szGrpName,SAVE); 
	}
}

void DlgNetSel::OnCancel()
{
	CDialog::OnCancel();
	DestroyWindow();
}

void DlgNetSel::OnOK()
{
	CDialog::OnCancel();
	GetPxyCfg();

	if (m_pRun != NULL)
	{
		TermThread(m_pRun->m_hThread);
		m_pRun= NULL;
	}

	if (m_pTdTest != NULL)
	{
		TermThread(m_pTdTest->m_hThread);
		m_pTdTest= NULL;
	}
	DestroyWindow();
}

void DlgNetSel::OnDestroy()
{
    CDialog::OnDestroy();
	LoginDlg* pLogin = (LoginDlg*)m_pLogin;
	pLogin->GetDlgItem(IDC_NETSET)->EnableWindow(TRUE);
	
	delete this;
}

void DlgNetSel::InitCombo()
{
	m_CombSockLst.SubclassDlgItem(IDC_CBNETTYPE,this);
	const TCHAR* strServs[3] ={_T("socks4"),_T("socks4a"),_T("socks5")};
	for (int i=0;i<3;i++){ m_CombSockLst.AddString(strServs[i]); }
	m_CombSockLst.SetCurSel(m_iPxyType);
}

UINT DlgNetSel::TestSvr(LPVOID pParam)
{
	TESTSVRPARAM*  pTest = static_cast<TESTSVRPARAM*>(pParam);
	DlgNetSel* pDlg = static_cast<DlgNetSel*>(pTest->pDlg);

	GetDelayMs((LPCTSTR)pTest->szSvr,pTest->iPort,pTest->iDelayMs);
	pDlg->m_pTdTest = NULL;

	DEL(pTest);
	return 0;
}

void DlgNetSel::InitLists()
{
	m_LstTds.SubclassDlgItem(IDC_LST_TRADE,this);
	m_LstMds.SubclassDlgItem(IDC_LST_MD,this);
	
	LPCTSTR lpHdrs[2] = {_T("交易服务器地址"),_T("网络延时")};
	LPCTSTR lpHdrs2[2] = {_T("行情服务器地址"),_T("网络延时")};
	int i=0,row=0,iWidths[2] = {260,80};
	for (i = 0;i<2 ; i++)
	{
		m_LstTds.InsertColumn(i, lpHdrs[i], LVCFMT_RIGHT,iWidths[i]);
		m_LstMds.InsertColumn(i, lpHdrs2[i], LVCFMT_RIGHT,iWidths[i]);
	}

	int iSize = static_cast<int>(m_szArTs.GetSize());
	for (i=0;i<iSize;i++)
	{
		row = m_LstTds.InsertItem(m_LstTds.GetItemCount(), m_szArTs[i], 0);
		m_LstTds.SetItemData(row, (DWORD_PTR)i);
	}

	iSize = static_cast<int>(m_szArMd.GetSize());
	for (i=0;i<iSize;i++)
	{
		row = m_LstMds.InsertItem(m_LstMds.GetItemCount(), m_szArMd[i], 0);
		m_LstMds.SetItemData(row, (DWORD_PTR)i);
	}
}

UINT DlgNetSel::Mthread(LPVOID pParam)
{
	DlgNetSel*  pDlg = static_cast<DlgNetSel*>(pParam);
	
	pDlg->GetDelay();

	pDlg->m_pRun = NULL;
	
	return 0;
}

void DlgNetSel::GetDelay()
{
	int iSizeTd = static_cast<int>(m_szArTs.GetSize());
	int iSubItem=0;
	
	CString szDelay = _T("");
	CString sTdSvr,sMdSvr;
	int iTdPort,iMdPort;
	for (int i = 0; i <iSizeTd; i++)
	{
		TESTSVRPARAM* testpara1 = new TESTSVRPARAM;
		testpara1->pDlg = this;
		testpara1->iDelayMs = -1;
		SplitFrtUrl(m_szArTs[i],sTdSvr,iTdPort);
		testpara1->szSvr = m_bUseProxy?m_szSkAddr:sTdSvr;
		testpara1->iPort = m_bUseProxy?m_iSkPort:iTdPort;
		if (!m_pTdTest)
		{
			m_pTdTest=AfxBeginThread((AFX_THREADPROC)DlgNetSel::TestSvr, (LPVOID)testpara1);
			WaitForSingleObject(m_pTdTest->m_hThread,500); 
		}
			
		if (testpara1->iDelayMs <=0)
		{ m_LstTds.SetItemText(i, 1, LoadStr(IDS_CONN_ERR)); }
		else
		{
			szDelay.Format(_T("%ldms"),testpara1->iDelayMs);
			m_LstTds.SetItemText(i, 1, szDelay);
		}
			
		TESTSVRPARAM* testpara2 = new TESTSVRPARAM;
		testpara2->pDlg = this;
		testpara2->iDelayMs = -1;
		SplitFrtUrl(m_szArMd[i],sMdSvr,iMdPort);
		testpara2->szSvr = m_bUseProxy?m_szSkAddr:sMdSvr;
		testpara2->iPort = m_bUseProxy?m_iSkPort:iMdPort;
		if (!m_pTdTest)
		{
			m_pTdTest=AfxBeginThread((AFX_THREADPROC)DlgNetSel::TestSvr, (LPVOID)testpara2);
			WaitForSingleObject(m_pTdTest->m_hThread,500); 
		}
			
		if (testpara2->iDelayMs <=0)
		{ m_LstMds.SetItemText(i, 1, LoadStr(IDS_CONN_ERR)); }
		else
		{
			szDelay.Format(_T("%ldms"),testpara2->iDelayMs);
			m_LstMds.SetItemText(i, 1, szDelay); 
		}	
	}
}

void DlgNetSel::OnBnClkChkSock()
{
	m_bUseProxy = !m_bUseProxy;
	
	ChkProxy(m_bUseProxy);
}

void DlgNetSel::OnBeginEditTdLst(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;
	
	*pResult = FALSE;
	
	if (pItem->iItem < 0) { return; }
	if (pItem->iSubItem == 0 ) { *pResult = TRUE; }
}

void DlgNetSel::OnDoEditTdLst(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;
	
	if (pItem->iItem < 0) {
		*pResult = FALSE;
		return;
	}
	
	*pResult = TRUE;
	switch (pItem->iSubItem) {
	case 0: 
		{
			m_LstTds.ShowInPlaceEdit(pItem->iItem, pItem->iSubItem);
			break;
		}
	default:
		*pResult = FALSE;
		break;
	}
}

void DlgNetSel::OnEndEditTdLst(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;
	
	*pResult = FALSE;
	
	if (!m_LstTds.m_fInPlaceDirty) { return; }
	if (pItem->iItem < 0) { return; }
	
	int iPos = static_cast<int>(m_LstTds.GetItemData(pItem->iItem));
	switch (pItem->iSubItem) {
	case 0: 
		{
			if (CheckInpSvr(pItem->pszText))
			{
				m_szArTs[iPos] = pItem->pszText;
				m_LstTds.SetItemText(pItem->iItem, pItem->iSubItem, pItem->pszText);
				*pResult = TRUE;
			}
		}
		break;
	}
	
	m_IsModified = !!(*pResult);
}

void DlgNetSel::OnBeginEditMdLst(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;
	
	*pResult = FALSE;
	
	if (pItem->iItem < 0) { return; }
	if (pItem->iSubItem == 0) { *pResult = TRUE; }
}

void DlgNetSel::OnDoEditMdLst(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;
	
	if (pItem->iItem < 0) {
		*pResult = FALSE;
		return;
	}
	
	*pResult = TRUE;
	switch (pItem->iSubItem) {
	case 0: 
		{
			m_LstMds.ShowInPlaceEdit(pItem->iItem, pItem->iSubItem);
			break;
		}
	default:
		*pResult = FALSE;
		break;
	}
}

void DlgNetSel::OnEndEditMdLst(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;
	
	*pResult = FALSE;
	
	if (!m_LstMds.m_fInPlaceDirty) { return; }
	if (pItem->iItem < 0) { return; }
	
	int iPos = static_cast<int>(m_LstTds.GetItemData(pItem->iItem));
	switch (pItem->iSubItem) {
	case 0: 
		{
			if (CheckInpSvr(pItem->pszText))
			{
				m_szArMd[iPos] = pItem->pszText;
				m_LstMds.SetItemText(pItem->iItem, pItem->iSubItem, pItem->pszText);
				*pResult = TRUE;
			}
		}
		break;
	}
	
	m_IsModified = !!(*pResult);
}

BOOL DlgNetSel::CheckInpSvr(LPCTSTR szInp)
{
	CString str(szInp),szPort;
	int iPos2,iPort,i=0;
	int iPos = str.ReverseFind(':');
	szPort = str.Mid(iPos+1);

	LPCTSTR lp = szPort.GetBuffer(0);
	int iLen = szPort.GetLength();
	
	for(i=0;*(lp+i) != '\0';i++)
	{
		if (!isdigit(*(lp+i))){ goto ERROR_RTN; }
	}

	iPos2 = str.Find(_T("//"));	
	iPort = _ttoi(szPort);

	if(!_tcsnicmp(szInp,TCP_PRE,6) && iPos!=-1 && 
        iPos2!=-1 &&iPort>0 && iPort<0xffff)
	{ 
		GetDlgItem(IDC_STACHKMSG)->SetWindowText(_T("地址格式正确!"));
		return TRUE; 
	}

ERROR_RTN:
	GetDlgItem(IDC_STACHKMSG)->SetWindowText(LoadStr(IDS_SVRURLERR));
	return FALSE;
}

void DlgNetSel::OnEnChgPxyServ()
{
	UpdateData();
}

void DlgNetSel::OnEnChgPxyPort()
{
	UpdateData();
}

void DlgNetSel::OnEnChgPxyUser()
{
	UpdateData();
}

void DlgNetSel::OnEnChgPxyPass()
{
	UpdateData();
}
