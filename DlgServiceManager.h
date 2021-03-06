#pragma once
#include "_IOCPServer.h"
#include "Common.h"
#include "afxcmn.h"

// CDlgServiceManager 对话框

class CDlgServiceManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgServiceManager)

public:
	CDlgServiceManager(CWnd* pParent = NULL, _CIOCPServer* IOCPServer = NULL, PCONTEXT_OBJECT ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgServiceManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SERVICE_MANAGER };
#endif
private:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	CListCtrl m_CListCtrl_Dialog_Service_Manager_Show;

	int CDlgServiceManager::ShowClientServiceList(void);
	afx_msg void OnNMRClickListDialogServiceManagerShow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuDialogServiceManagerListShowRefresh();
	void  CDlgServiceManager::WindowHandleIO(void);
	afx_msg void OnMenuDialogServiceManagerListShowStrat();
	afx_msg void OnMenuDialogServiceManagerListShowStop();
	void CDlgServiceManager::ConfigClientService(BYTE IsMethod);  //服务开启关闭 核心代码
	afx_msg void OnMenuDialogServiceManagerListShowAutoRun();
	afx_msg void OnMenuDialogServiceManagerListShowManualRun();
};
