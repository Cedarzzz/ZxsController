#pragma once
#include "_IOCPServer.h"
#include "Common.h"

// CDlgRemoteControll 对话框

class CDlgRemoteControll : public CDialog
{
	DECLARE_DYNAMIC(CDlgRemoteControll)

public:
	CDlgRemoteControll(CWnd* pParent = NULL, _CIOCPServer* IOCPServer = NULL, PCONTEXT_OBJECT ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgRemoteControll();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REMOTE_CONTROLL };
#endif
private:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
	LPBITMAPINFO m_BitmapInfo;  //工具
	HBITMAP m_BitmapHandle;  //工具
	PVOID m_BitmapData;  //工具
	HDC m_WindowDCHandle;  //工人
	HDC m_WindowMemoryDCHandle;  //工人的工具箱
	POINT m_CursorPosition;  //存储鼠标位置
	ULONG m_HorizontalScrollPosition;
	ULONG m_VerticalScrollPosition;

	BOOL m_IsTraceCursor = FALSE;  //跟踪光标轨迹
	BOOL m_IsControl = FALSE;  //远程控制
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void CDlgRemoteControll::WindowHandleIO(void);
	VOID CDlgRemoteControll::DrawFirstScreen(void);  //显示第一帧数据
	VOID CDlgRemoteControll::DrawNextScreen(void);
	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	VOID CDlgRemoteControll::OnPrepareSending(MSG* Msg);
	BOOL CDlgRemoteControll::SaveSnapShotData(void);
	VOID CDlgRemoteControll::UpdataClipBoardData(char* BufferData, ULONG BufferLength);
	VOID CDlgRemoteControll::SendClipboardData(void);
};
