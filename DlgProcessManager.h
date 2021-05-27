#pragma once
#include "afxcmn.h"
#include "_IOCPServer.h"
#include "Common.h"

// CDlgProcessManager 对话框

class CDlgProcessManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgProcessManager)

public:
	CDlgProcessManager(CWnd* pParent = NULL, _CIOCPServer* IOCPServer = NULL, PCONTEXT_OBJECT ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgProcessManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROCESS_MANAGER };
#endif
private:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_CListCtrl_Dialog_Process_Manager_Show;
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void CDlgProcessManager::ShowProcessList(void);   //进程信息显示
	afx_msg void OnNMCustomdrawListDialogProcessManagerShow(NMHDR *pNMHDR, LRESULT *pResult);   //更换颜色
	afx_msg void OnNMRClickListDialogProcessManagerShow(NMHDR *pNMHDR, LRESULT *pResult);   //右键菜单显示
	afx_msg void OnMenuDialogProcessManagerListShowRefresh();   //刷新进程列表消息发送
	void CDlgProcessManager::WindowHandleIO(void);  //刷新进程列表
};
