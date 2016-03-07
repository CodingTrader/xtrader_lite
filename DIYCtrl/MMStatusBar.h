#pragma once

const int MM_STATUS_PANE_UPDATE		= 9999;

const DWORD DEFAULT_PANE_COLOR    = 0xFFFFFFFF;

struct MMStatusDrawStruct
{
  int       m_nIndex; //index of pane
  COLORREF  m_crTextColor; //color of text
  COLORREF  m_crBkColor; //color of text background
  CString   m_szText; //pane text
  int       m_cxExtra;
};

class MMStatusCmdUI : public CCmdUI      // class private to this file!
{
public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
	virtual void SetToolTipText(LPCTSTR lpszText);
	virtual void SetWidth(int cxWidth);
	virtual void SetIcon(HICON hIcon);
};

class MMStatusBar : public CStatusBar
{
  friend class MMStatusCmdUI;
private:
  MMStatusDrawStruct* GetPaneStruct(int nIndex);
protected:
	CPtrArray m_oDrawStructArray;
// Construction
public:
	MMStatusBar();

// Attributes
public:

// Operations
  //Used to set the pane text color
  void SetPaneTextColor(int nIndex,COLORREF crTextColor);
  void SetPaneTextBkColor(int nIndex,COLORREF crTextBkColor);
  void UpdateTipText(LPCTSTR lpszTxt,UINT nIDTool);

  void SetDelayTime(UINT uDelayTime=500);
  BOOL SetIndicators(const UINT* lpIDArray,int nIDCount,const UINT* lpIDToolTipArray = NULL,const HICON* lphIconArray = NULL);
  void Redraw();

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MMStatusBar)
	//}}AFX_VIRTUAL

// Implementation
public:
  CToolTipCtrl m_oToolTip;
	virtual ~MMStatusBar();
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual BOOL OnChildNotify(UINT message,WPARAM wParam,LPARAM lParam,LRESULT* pResult);
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct); 
	// Generated message map functions
protected:
	//{{AFX_MSG(MMStatusBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
