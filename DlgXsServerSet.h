#pragma once
#include "_ConfigFile.h"
#include "afxwin.h"

// CDlgXsServerSet 对话框

class CDlgXsServerSet : public CDialog
{
	DECLARE_DYNAMIC(CDlgXsServerSet)

public:
	CDlgXsServerSet(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgXsServerSet();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SERVER_SET };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	long m_CEdit_Dialog_Server_Set_Max_Connection;
	long m_CEdit_Dialog_Server_Set_Listen_Port;
	afx_msg void OnBnClickedEditDialogServerSetApply();
	afx_msg void OnEnChangeEditDialogServerSetMaxConnection();
	afx_msg void OnEnChangeEditDialogServerSetListenPort();
	virtual BOOL OnInitDialog();
	CButton m_CButton_Dialog_Server_Set_Apply;
};
