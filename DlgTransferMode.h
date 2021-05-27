#pragma once


// CDlgTransferMode 对话框

class CDlgTransferMode : public CDialog
{
	DECLARE_DYNAMIC(CDlgTransferMode)

public:
	CDlgTransferMode(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgTransferMode();
	CString m_FileName;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_TRANSFER_MODE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg void OnEndDialog(UINT id);
	DECLARE_MESSAGE_MAP()
public:
	CString m_CString_Dialog_Transfer_Mode_Tips;
	virtual BOOL OnInitDialog();
};
