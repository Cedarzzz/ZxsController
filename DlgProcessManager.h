#pragma once
#include "afxcmn.h"
#include "_IOCPServer.h"
#include "Common.h"

// CDlgProcessManager �Ի���

class CDlgProcessManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgProcessManager)

public:
	CDlgProcessManager(CWnd* pParent = NULL, _CIOCPServer* IOCPServer = NULL, PCONTEXT_OBJECT ContextObject = NULL);   // ��׼���캯��
	virtual ~CDlgProcessManager();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROCESS_MANAGER };
#endif
private:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_CListCtrl_Dialog_Process_Manager_Show;
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void CDlgProcessManager::ShowProcessList(void);   //������Ϣ��ʾ
	afx_msg void OnNMCustomdrawListDialogProcessManagerShow(NMHDR *pNMHDR, LRESULT *pResult);   //������ɫ
	afx_msg void OnNMRClickListDialogProcessManagerShow(NMHDR *pNMHDR, LRESULT *pResult);   //�Ҽ��˵���ʾ
	afx_msg void OnMenuDialogProcessManagerListShowRefresh();   //ˢ�½����б���Ϣ����
	void CDlgProcessManager::WindowHandleIO(void);  //ˢ�½����б�
};