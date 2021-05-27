#pragma once


// CDlgNewFolder 对话框

class CDlgNewFolder : public CDialog
{
	DECLARE_DYNAMIC(CDlgNewFolder)

public:
	CDlgNewFolder(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgNewFolder();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_NEW_FOLDER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_CEdit_Dialog_New_Folder_Main;
};
