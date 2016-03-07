#include "stdafx.h"
#include "xTrader.h"
#include "LoginDlg.h"
#include "NoticeDlg.h"
#include "DlgNetSel.h"
#include "DlgRecent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HANDLE g_hEvent;

IMPLEMENT_DYNAMIC(LoginDlg, CDialog)
LoginDlg::LoginDlg(CWnd* pParent)
	: CDialog(LoginDlg::IDD, pParent)
{
    CAppCfgs& s = g_s;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bModeLess = FALSE;
	m_szUid = _T("00000028");
	m_szPass = _T("123456");
	m_sDymPwd.Empty();
	m_pLogin = NULL;
	m_iSelBkr = 0;
	m_iSelSvr = 0;
	m_szArTs.RemoveAll();
	m_szArMd.RemoveAll();
	m_szBrkName = _T("");
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_szUid,s.m_sUid);
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_szPass,s.m_sPwd);
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_sDymPwd,s.m_sDymPwd);
}

LoginDlg::~LoginDlg()
{
	
}

void LoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LoginDlg)
	DDX_Control(pDX, IDC_ISPLIST, m_ComboIsp);
	DDX_Control(pDX, IDC_BKR_LIST, m_ComboBkr);
	DDX_Text(pDX, IDC_USERNAME, m_szUid);
	DDV_MaxChars(pDX, m_szUid, 13);
	DDX_Text(pDX, IDC_PASSWORD, m_szPass);
	DDV_MaxChars(pDX, m_szPass, 41);
	DDX_Text(pDX, IDC_DYMPWD, m_sDymPwd);
	DDV_MaxChars(pDX, m_sDymPwd, 41);
	DDX_Check(pDX, IDC_SAVE, m_bSavePwd);
	DDX_Control(pDX, IDC_PROGRESS, m_prgs);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(LoginDlg, CDialog)
	//{{AFX_MSG_MAP(LoginDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_LOGIN, OnLogin)
	ON_BN_CLICKED(IDC_LOGOUT, OnQuit)
	ON_BN_CLICKED(IDC_NETSET, OnNetset)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_EN_CHANGE(IDC_USERNAME, OnChangeUser)
	ON_EN_CHANGE(IDC_PASSWORD, OnChangePass)
	ON_EN_CHANGE(IDC_DYMPWD, OnChangeDym)
	ON_CBN_SELCHANGE(IDC_ISPLIST, OnSelSvr)
	ON_CBN_SELCHANGE(IDC_BKR_LIST, OnSelBkr)
	ON_CBN_EDITCHANGE(IDC_BKR_LIST, OnEditChgBkr)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_EXTRA, OnBnClkExtra)
END_MESSAGE_MAP()

void LoginDlg::LoadConfig()
{
	xml_document doc;
	xml_node proot;
	xml_parse_result result;
	CAppCfgs& s = g_s;
	vector<LOGINPARA>& pAcc = s.m_pInfVec;
	vector<BKRPARA>& pBkr = s.m_BkrParaVec;

	m_bSavePwd = s.m_bSavePwd;
	size_t iSize = pAcc.size();
	if (iSize >= 1)
	{
		strcpy(s.m_sUid, pAcc[iSize-1].szUid);
		ansi2uni(CP_UTF8, s.m_sUid, m_szUid.GetBuffer(MAX_PATH));
		m_szUid.ReleaseBuffer();

		Base64Decode(s.m_sPwd, (const char*)pAcc[iSize-1].szPass, 0);
		ansi2uni(CP_UTF8, s.m_sPwd, m_szPass.GetBuffer(MAX_PATH));
		m_szPass.ReleaseBuffer();

		m_iSelBkr = pAcc[iSize-1].iBkrGroup;
		m_iSelSvr = pAcc[iSize-1].iSvrGroup;
	}			

	int iBkrSize = static_cast<int>(pBkr.size());
	//index��ҪԽ��
	if (iBkrSize>0 && iBkrSize>=m_iSelBkr)
	{
		/////////////////////////////////////////
		m_ComboBkr.AddString(pBkr[m_iSelBkr].BkrName);
		m_ComboBkr.SetCurSel(0);
		/////////////////////////////////////////////////////////////	
		result = doc.load_file(pBkr[m_iSelBkr].XmlPath);
		if (result.status == status_ok) 
		{
			proot = doc.child(_NDRT).child(_NDBKR);
			if (!proot) return;
			
			strcpy(s.m_sBkrId,pBkr[m_iSelBkr].BkrId);
			m_szBrkName = pBkr[m_iSelBkr].BkrName;
			strcpy(s.m_sAtCode,pBkr[m_iSelBkr].AuthCode);
			///////////����������//////////////////		
			TCHAR tTd[MAX_PATH],tMd[MAX_PATH];
			CString tName,tTrading,tMData;
			CStringArray szAr;
			szAr.RemoveAll();
			
			xpath_node_set sVrs = doc.select_nodes(_NDPATHSVR);
			if (sVrs.empty()) return;
			xpath_node_set::const_iterator it = sVrs.begin();
			for (it; it !=  sVrs.end(); ++it)
			{
				xml_node ndRt = it->node();
				ansi2uni(CP_ACP,(char*)ndRt.child_value(_NDNAME),tName.GetBuffer(MAX_PATH));
				tName.ReleaseBuffer();
				szAr.Add(tName);
				//���ѭ����ָ����,������Ϣ,����������������
				if (szAr.GetSize()==(m_iSelSvr+1))
				{
					xml_node ndIts,ndTd = ndRt.child(_NDTRADE),ndMd = ndRt.child(_NDMKTD);
					
					for (ndIts = ndTd.first_child(); ndIts; ndIts = ndIts.next_sibling())
					{
						ansi2uni(CP_ACP,(char*)ndIts.child_value(),tTd);
						if (!_tcsnicmp(tTd,SSL_PRE,6)){ tTrading.Format(_T("%s"),tTd); }
                        else
                        { tTrading.Format(TCP_PFS,tTd); }
						
						m_szArTs.Add(tTrading);
					}
					
					for (ndIts = ndMd.first_child(); ndIts; ndIts = ndIts.next_sibling())
					{
						ansi2uni(CP_ACP,(char*)ndIts.child_value(),tMd);
						
						if (!_tcsnicmp(tMd,SSL_PRE,6)){ tMData.Format(_T("%s"),tMd); }
						else
						{
							//UDPǰ׺ʱһ����TCP����createapi�������һ��
							if (!_tcsnicmp(tMd,UDP_PRE,6))
							{
								tMData.Format(_T("%s"),tMd);
								tMData.Replace(UDP_PRE,TCP_PRE);
							}
							else
							{ tMData.Format(TCP_PFS,tMd); }
						}
						m_szArMd.Add(tMData);
					}
				}
			}
			////////////////////////////////////////////////////////////////////////////
			for (int i=0;i<szAr.GetSize();i++){ m_ComboIsp.AddString(szAr[i]); }
			
			m_ComboIsp.SetCurSel(m_iSelSvr);
			SendMessage(WM_COMMAND,MAKEWPARAM(IDC_ISPLIST,CBN_SELCHANGE),(LPARAM)m_ComboIsp.GetSafeHwnd());
		}	
	}
	else
	{
		GetDlgItem(IDC_NETSET)->EnableWindow(FALSE);
		GetDlgItem(IDC_LOGIN)->EnableWindow(FALSE);
		ProgressUpdate(_T("����brokersĿ¼!"), 0);
	}	
}

void LoginDlg::SaveConfig()
{
	CAppCfgs& s = g_s;
	vector<LOGINPARA>& pAcc = s.m_pInfVec;
	UpdateData();
		
    s.m_bSavePwd = !!m_bSavePwd;
	TThostFtdcInvestorIDType szUid;
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_szUid,szUid);
	char szEncPass[64];
	//����������붼Ҫ�򵥼���
	if (m_bSavePwd)
	{ Base64Encode(szEncPass, s.m_sPwd, 0); }
	else
	{ ZeroMemory(szEncPass, sizeof(szEncPass)); }
		
	//�ѵ�¼������¼�ɽṹ����
	LOGINPARA pInf;
	strcpy(pInf.szUid, szUid);
	strcpy(pInf.szPass, szEncPass);
	pInf.iBkrGroup = m_iSelBkr;
	pInf.iSvrGroup = m_iSelSvr;

	if (s.m_bSaveHis)
	{
		size_t iSize = pAcc.size();
		if (iSize>=1)
		{	///// ����ظ���¼���û��浵
			for (VIT_lp vlp = pAcc.begin(); vlp != pAcc.end();)
			{
				if ((!strcmp(vlp->szUid, szUid)) && (vlp->iBkrGroup == m_iSelBkr))
				{
					vlp = pAcc.erase(vlp);
				}
				else
					++vlp;
			}
		}
		pAcc.push_back(pInf);
	}
	else
	{ ClearVec(pAcc); }

	if (!g_Dlg){ s.ProcConfigs(SAVE); }
}

BOOL LoginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_prgs.SetRange(0, 100);
	ProgressUpdate(_T("δ��¼!"), 0);

	LoadConfig();
	UpdateData(FALSE);
	
	//CXTraderDlg::StartLog();
	//OnBnClkExtra();
	return 0;
}

void LoginDlg::OnLogin() 
{
	//LOGI("��ʼ��¼(" << this << ") ");
	UpdateData();
	if (m_szUid.IsEmpty() || m_szPass.IsEmpty())
	{ ShowErroTips(IDS_NOUIDPWD,IDS_MY_TIPS); }
	else
	{
		GetDlgItem(IDC_NETSET)->EnableWindow(FALSE);
		GetDlgItem(IDC_LOGIN)->EnableWindow(FALSE);	
		if (!m_pLogin) { m_pLogin = AfxBeginThread((AFX_THREADPROC)LoginThread, this); }
	}
}

void LoginDlg::OnQuit() 
{
	EndDialog(IDCANCEL);
	if (m_bModeLess){ DestroyWindow(); }
}

void LoginDlg::OnOK() 
{
	CDialog::OnOK();

	DestroyWindow();  
}

void LoginDlg::OnCancel() 
{
	CDialog::OnCancel();

	DestroyWindow();  
}

void LoginDlg::OnDestroy()
{
	SaveConfig();
	CDialog::OnDestroy();

    if (m_pLogin) 
    {
        TermThread(m_pLogin->m_hThread);
		m_pLogin = NULL;
    }
	//��ģֻ̬�ڽ����������л��˻�ʱ
	if (m_bModeLess)
	{ 
		g_Dlg->m_eLogMod = NORMAL_LOG;
		delete this; 
	}	
}

void LoginDlg::OnNetset() 
{
    CAppCfgs& s = g_s;
	GetDlgItem(IDC_NETSET)->EnableWindow(FALSE);
	
	CString str;
	m_ComboIsp.GetWindowText(str);

	DlgNetSel* tdlg = new DlgNetSel;
	tdlg->m_pLogin = this;
	tdlg->m_szGrpName = str;
	tdlg->m_szXml = s.m_BkrParaVec[m_iSelBkr].XmlPath;
	tdlg->m_szArTs.Copy(m_szArTs);
	tdlg->m_szArMd.Copy(m_szArMd);
	tdlg->m_bUseProxy = s.m_bProxy;
	SplitPxyUrl(s.m_szProxy,tdlg->m_iPxyType,tdlg->m_szSkAddr,
		tdlg->m_iSkPort,tdlg->m_szSkUser,tdlg->m_szSkPwd);

	BOOL res=tdlg->Create(IDD_DLG_NETSET,NULL);
	VERIFY( res==TRUE );
	tdlg->CenterWindow();
	tdlg->ShowWindow(SW_SHOW);
}

void LoginDlg::SplitPxyUrl(CString szUrl,int& iType,CString& szSkAddr,int& iSkPort,CString& szSkUser,CString& szSkPwd)
{
	CString szSkType,szUnP,szSnP;
	int iPos1 = szUrl.Find(_T("://"));
	int iPos2 = szUrl.Find(_T("@"));

	szSkType = szUrl.Mid(0,iPos1);
	if (!szSkType.CompareNoCase(_T("socks4"))){ iType = T_SOCKS4;}
	if (!szSkType.CompareNoCase(_T("socks4a"))){ iType = T_SOCKS4A;}
	if (!szSkType.CompareNoCase(_T("socks5"))){ iType = T_SOCKS5;}

	szUnP = szUrl.Mid(iPos1+3,iPos2-iPos1-3);
	szSnP = szUrl.Mid(iPos2+1);

	iPos1 = szUnP.Find(':');
	szSkUser = szUnP.Mid(0,iPos1);
	szSkPwd = szUnP.Mid(iPos1+1);
	
	iPos1 = szSnP.Find(':');
	szSkAddr = szSnP.Mid(0,iPos1);
	iSkPort = _ttoi(szSnP.Mid(iPos1+1));
	if (iSkPort<1){ iSkPort = 80 ;}
}

void LoginDlg::OnSave() 
{
	m_bSavePwd = !m_bSavePwd;
}

////////ѡ��һ���������Ӧ�¼�///////
void LoginDlg::OnSelSvr() 
{
	if(m_ComboIsp.GetCurSel() >= 0)
	{
        CAppCfgs& s = g_s;
		m_iSelSvr = m_ComboIsp.GetCurSel();

		CString szGrp;
		m_ComboIsp.GetWindowText(szGrp);
		LPCTSTR lpXml = s.m_BkrParaVec[m_iSelBkr].XmlPath;

		//��ָ����ķ�������Ϣ��ȡ���
		s.RwSvrsbyAr(m_szArTs,m_szArMd,lpXml,szGrp);
	}
}

void LoginDlg::OnEditChgBkr()
{
	CString strTxt,szHzpy;
	int iIdx = -1;
	m_ComboBkr.GetWindowText(strTxt);
	
	CAppCfgs& s = g_s;
	vector<BKRPARA>& pBkr = s.m_BkrParaVec;
	size_t iSize = pBkr.size(),i=0;

	if (_istalpha(strTxt.GetAt(0)))
	{
		m_ComboBkr.ResetContent();
		for (i=0;i<iSize;i++)
		{
			GetFirstLetter(pBkr[i].BkrName,szHzpy);
			if (!_tcsnicmp((LPCTSTR)szHzpy,(LPCTSTR)strTxt,_tcslen(strTxt)))
			{
				iIdx = m_ComboBkr.AddString(pBkr[i].BkrName);
				m_ComboBkr.SetItemData(iIdx,i);
			}
		}
	}

	if (iIdx==0)
	{
		m_ComboBkr.SetCurSel(0);
		SendMessage(WM_COMMAND,MAKEWPARAM(IDC_BKR_LIST,CBN_SELCHANGE),(LPARAM)m_ComboBkr.GetSafeHwnd());
	}
	else if (iIdx>0)
	{ 
		m_ComboBkr.ShowDropDown(); 
		SetCursor(LoadCursor(NULL,IDC_ARROW));
	}
}

////////ѡ��һ���ڻ���˾��Ӧ�¼�///////
void LoginDlg::OnSelBkr() 
{
	if(m_ComboBkr.GetCurSel() >= 0)
	{
        CAppCfgs& s = g_s;
        vector<BKRPARA>& pBkr = s.m_BkrParaVec;

		size_t iIdx = m_ComboBkr.GetItemData(m_ComboBkr.GetCurSel());
		m_iSelBkr = static_cast<int>(iIdx);

		VITBkr bit = pBkr.begin()+iIdx;
		
		xml_document doc;
		xml_node proot;
		xml_parse_result result = doc.load_file(bit->XmlPath);
		if (result.status == status_ok) 
		{
			proot = doc.child(_NDRT).child(_NDBKR);
			if (!proot) return;
			
			strcpy(s.m_sBkrId,bit->BkrId);
			m_szBrkName = bit->BkrName;
			strcpy(s.m_sAtCode,bit->AuthCode);
			
			///////////���������������б�//////////////////
			char strName[64];
			CString tName,tTrading,tMData;
			CStringArray szAr;
			szAr.RemoveAll();
			
			xpath_node_set sVrs = doc.select_nodes(_NDPATHSVR);
			if (sVrs.empty()) return;
			xpath_node nd;
			for (xpath_node_set::const_iterator it = sVrs.begin(); it !=  sVrs.end(); ++it)
			{
				nd = *it;
				strcpy(strName,nd.node().child_value(_NDNAME));
				ansi2uni(CP_ACP,strName,tName.GetBuffer(MAX_PATH));
				tName.ReleaseBuffer();
				szAr.Add(tName);	
			}
			/////��ʼ��շ������������///
			m_ComboIsp.ResetContent();
			////////////////////////////////////////////////////////////////////////////
			int i=0;
			for (i=0;i<szAr.GetSize();i++){ m_ComboIsp.AddString(szAr[i]); }
			/////////ѡ�����鲢������Ӧ/////////////
			m_ComboIsp.SetCurSel(0);
			SendMessage(WM_COMMAND,MAKEWPARAM(IDC_ISPLIST,CBN_SELCHANGE),(LPARAM)m_ComboIsp.GetSafeHwnd());
			///////////////////////////////////////////
		}
	}
}

void LoginDlg::OnChangeUser() 
{
	UpdateData(true);
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_szUid,g_s.m_sUid);
}

void LoginDlg::OnChangePass() 
{
	UpdateData(true);
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_szPass,g_s.m_sPwd);
}

void LoginDlg::OnChangeDym() 
{
    UpdateData(true);
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_sDymPwd,g_s.m_sDymPwd);
}

void LoginDlg::ProgressUpdate(LPCTSTR szMsg, const int nPercent)
{
    ASSERT (AfxIsValidString(szMsg));
    ASSERT ( nPercent >= 0  &&  nPercent <= 100 );
	
    SetDlgItemText(IDC_LOGINFO,szMsg);
    m_prgs.SetPos(nPercent);
}

UINT LoginDlg::LoginThread(LPVOID pParam)
{
	LoginDlg* pDlg = static_cast<LoginDlg*>(pParam);
	CXTraderApp* app = g_App;
	CAppCfgs& s = app->m_cfg;

	s.m_szTitle.Format(_T("%s@%s-%s"),(LPCTSTR)pDlg->m_szUid,(LPCTSTR)pDlg->m_szBrkName,(LPCTSTR)LoadStr(IDS_TITLE));

	pDlg->ProgressUpdate(_T("��ʼ��,ע��ǰ��!"), 2);
	app->ReleaseApi();
	app->CreateApi();
	CtpTdSpi* td = app->m_cT;
	td->ClrAllVecs();
	td->InitMgrFee();

	#ifdef _NEED_LOGIN_
	int iSize = static_cast<int>(pDlg->m_szArTs.GetSize());
	char szTd[MAX_PATH],szMd[MAX_PATH];
	CString szTdt,szMdt,szTemp;
	int iPos = s.m_szProxy.Find(_T("//"));
	CString szPxyPre = s.m_bProxy ? s.m_szProxy.Mid(0,iPos+2):_T("");
	CString szPxyUrl = s.m_bProxy ? s.m_szProxy.Mid(iPos+2):_T("");
	int i = 0;

	for (i=0;i<iSize;i++)
	{
		szTemp = pDlg->m_szArTs.GetAt(i);
		if (s.m_bProxy)
		{
			iPos = szTemp.Find(_T("//"));
			szTdt = szTemp.Mid(iPos+2);

			szMdt.Format(_T("%s%s/%s"),szPxyPre,szTdt,szPxyUrl);
			uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)szMdt,szTd);	
		}
		else
		{ uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)szTemp,szTd); }
		app->m_TApi->RegisterFront(szTd);
	}

	pDlg->ProgressUpdate(_T("����ǰ��>>>"), 4);
	app->m_TApi->Init();	
	
	if (strlen(s.m_sAtCode)) //����֤
	{ pDlg->ProgressUpdate(_T("��֤�ͻ���>>>"), 6); }
	else
	{ pDlg->ProgressUpdate(_T("��¼����>>>"), 6); }
	
	DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��֤�û�ͨ��!"), 8);
		ResetEvent(g_hEvent);
	}
	else
	{
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);
		pDlg->GetDlgItem(IDC_NETSET)->EnableWindow(TRUE);
		pDlg->m_pLogin = NULL;
		if (strlen(s.m_sAtCode) && strlen(td->m_RspMsg.ErrorMsg))
		{ pDlg->ProgressUpdate((LPCTSTR)GetCTPErr(td->m_RspMsg.ErrorMsg),0); }
		else
		{ pDlg->ProgressUpdate(_T("������ϻ�CTP������!"),0); }

		return 0;
	}
	
	if (td->IsErrorRspInfo(&td->m_RspMsg))
	{
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);
		pDlg->GetDlgItem(IDC_NETSET)->EnableWindow(TRUE);

		TCHAR szMsg[MAX_PATH];
		ansi2uni(CP_ACP,td->m_RspMsg.ErrorMsg,szMsg);
		pDlg->ProgressUpdate(szMsg,0);
		pDlg->m_pLogin = NULL;
	
		return 0;
	}
	///////////��¼�������//////////////////////////
	iSize = static_cast<int>(pDlg->m_szArMd.GetSize());
	for (i=0;i<iSize;i++)
	{
		szTemp = pDlg->m_szArMd[i];
		if (s.m_bProxy)
		{
			iPos = szTemp.Find(_T("//"));
			szMdt = szTemp.Mid(iPos+2);
			
			szTdt.Format(_T("%s%s/%s"),szPxyPre,szMdt,szPxyUrl);
			uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)szTdt,szMd);	
		}
		else
		{ uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)szTemp,szMd); }
		app->m_MApi->RegisterFront(szMd);
	}
	pDlg->ProgressUpdate(_T("��¼����>>>"), 10);
	app->m_MApi->Init();
	
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��¼����ɹ�!"), 13);
		ResetEvent(g_hEvent);
	}

	pDlg->ProgressUpdate(_T("ȷ�Ͻ��㵥>>>"), 22);
	td->ReqSetInfConfirm();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("ȷ�Ͻ��㵥�ɹ�!"), 25);
		ResetEvent(g_hEvent);
	}
	else
	{
		pDlg->m_pLogin = NULL;
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);

		pDlg->ProgressUpdate(_T("ȷ�Ͻ��㳬ʱ!"),0);
		return 0;
	}
	/////////////////////////////////////////////////
	pDlg->ProgressUpdate(_T("��ѯ������Ϣ>>"), 28);

	td->ReqQryDepthMD(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ������Ϣ�ɹ�!"), 33);
		ResetEvent(g_hEvent);	
	}
	else
	{
		pDlg->m_pLogin = NULL;
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);
		
		pDlg->ProgressUpdate(_T("��ѯ������Ϣ��ʱ!"),0);
		return 0;
	}
	/////////////////////////////////////////////////
	pDlg->ProgressUpdate(_T("��ѯ��Լ�б�>>>"), 38);
	td->ReqQryInst(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ��Լ�б�ɹ�!"), 43);
		ResetEvent(g_hEvent);
	}
	else
	{
		pDlg->m_pLogin = NULL;
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);

		pDlg->ProgressUpdate(_T("��ѯ��Լ��ʱ!"),0);
		return 0;
	}
	
	pDlg->ProgressUpdate(_T("��ѯ���ճɽ���Ϣ>>"), 48);
	td->ReqQryTd(NULL,NULL,NULL,NULL,NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ�ɽ���Ϣ�ɹ�!"), 53);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);
		
		pDlg->ProgressUpdate(_T("��ѯ�ɽ���Ϣ��ʱ!"),0);
		return 0;
	}
	
	pDlg->ProgressUpdate(_T("��ѯ����ί����Ϣ>>"), 58);
	td->ReqQryOrd(NULL,NULL,NULL,NULL,NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡί����Ϣ�ɹ�!"), 63);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);
		
		pDlg->ProgressUpdate(_T("��ѯί����Ϣ��ʱ!"),0);
		return 0;
	}

	pDlg->ProgressUpdate(_T("��ѯ�ֲ���Ϣ>>"),68);
	td->ReqQryInvPos(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ�ֲ���Ϣ�ɹ�!"), 73);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);

		pDlg->ProgressUpdate(_T("��ֲ���Ϣ��ʱ!"),0);
		return 0;
	}
	
	pDlg->ProgressUpdate(_T("��ѯ�ʽ��˻�>>"), 78);
	
	td->ReqQryTdAcc();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ�ʽ���Ϣ�ɹ�!"), 83);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);

		pDlg->ProgressUpdate(_T("���˻���ʱ!"),0);
		return 0;
	}
	//////////////////////////////////////////
//#ifdef _REAL_CTP_
	pDlg->ProgressUpdate(_T("��ѯ����ǩԼ>>"), 85);
	
	td->ReqQryAccreg();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ������Ϣ�ɹ�!"), 88);
		ResetEvent(g_hEvent);		
	}
	else
	{
		pDlg->m_pLogin = NULL;
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);

		pDlg->ProgressUpdate(_T("��ѯ������Ϣ��ʱ!"),0);
		return 0;
	}
	
	pDlg->ProgressUpdate(_T("��ѯ���ױ���>>"), 90);
		
	td->ReqQryTdCode();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ���ױ���ɹ�!"), 93);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);

		pDlg->ProgressUpdate(_T("��ѯ���ױ��볬ʱ!"),0);
		return 0;
	}
//#endif
	
	pDlg->ProgressUpdate(_T("��ѯ���Ͳ���>>"), 96);

	td->ReqQryBkrTdParams();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡ���Ͳ����ɹ�!"), 97);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);
		
		pDlg->ProgressUpdate(_T("��ѯ���Ͳ�����ʱ!"),0);
		return 0;
	}

	pDlg->ProgressUpdate(_T("��ѯԤ��>>"), 98);
	
	td->ReqQryParkedOrd(NULL,NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("��ȡԤ�񵥳ɹ�!"), 99);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		pDlg->GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);
		
		pDlg->ProgressUpdate(_T("��ѯԤ�񵥳�ʱ!"),0);
		return 0;
	}
	////////////////////////////////////////////////////
	//pDlg->ProgressUpdate(_T("��ѯ�����㷨>>"), 98);
	//
	//td->ReqQryBkrTdAlgos(NULL,NULL);
	//dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	//if (dwRet==WAIT_OBJECT_0)
	//{
	//	pDlg->ProgressUpdate(_T("��ȡ�����㷨�ɹ�!"), 99);
	//	ResetEvent(g_hEvent);
	//}	
	//else
	//{
	//	pDlg->m_pLogin = NULL;
	//	GetDlgItem(IDC_LOGIN)->EnableWindow(TRUE);
	//	
	//	pDlg->ProgressUpdate(_T("��ѯ�����㷨��ʱ!"),0);
	//	return 0;
	//}
	////////////////////////////////////////////////
	#endif

	pDlg->m_pLogin = NULL;

	if (pDlg->m_bModeLess)
	{
		pDlg->PostMessage(WM_CLOSE); 
		::PostMessage(g_Dlg->m_hWnd,WM_RELOGINOK,0,0);
	}
	else
	{ pDlg->EndDialog(IDOK); }
	
	return 0;
}

void LoginDlg::OnBnClkExtra()
{
	GetDlgItem(IDC_EXTRA)->EnableWindow(FALSE);

	DlgRecent* Dlg = new DlgRecent;

	CRect rc0,rc1;
	GetWindowRect(rc0);
	int xPos = rc0.right;
	Dlg->Create(IDD_DLG_RECENT,this);
	Dlg->GetWindowRect(rc1);
	//������Ļ�����൯��
	if ((rc0.right+rc1.Width())>GetSystemMetrics(SM_CXSCREEN)) 
	{ xPos = rc0.left - rc1.Width(); }
	Dlg->SetWindowPos(this,xPos,rc0.top,0,0,SWP_NOSIZE);
	Dlg->ShowWindow(SW_SHOW);
}
