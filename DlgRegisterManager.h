#pragma once
#include "_IOCPServer.h"
#include "Common.h"
#include "afxcmn.h"

// CDlgRegisterManager 对话框

class CDlgRegisterManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgRegisterManager)

public:
	CDlgRegisterManager(CWnd* pParent = NULL, _CIOCPServer* IOCPServer = NULL, PCONTEXT_OBJECT ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgRegisterManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REGISTER_MANAGER };
#endif
private:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	
	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_CTreeCtrl_Dialog_Register_Manager_Show;
	CListCtrl m_CListCtrl_Dialog_Register_Manager_Show;
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void  CDlgRegisterManager::WindowHandleIO(void);
public:
	CImageList m_ImageListTree;  //树控件上的图标
	CImageList m_ImageListList;  //List控件上的图标
	HTREEITEM  m_hRoot;
	HTREEITEM  HKCU;
	HTREEITEM  HKLM;
	HTREEITEM  HKUS;
	HTREEITEM  HKCC;
	HTREEITEM  HKCR;
	HTREEITEM  m_SelectedTreeItem;
	BOOL       m_IsEnable;   //解决频繁向被控端请求
	afx_msg void OnTvnSelchangedTreeDialogRegisterManagerShow(NMHDR *pNMHDR, LRESULT *pResult);
	char CDlgRegisterManager::GetFatherPath(CString& FullPath);
	CString CDlgRegisterManager::GetFullPath(HTREEITEM Current);
	void CDlgRegisterManager::AddPath(char* BufferData);
	void CDlgRegisterManager::AddKey(char* BufferData);
};
