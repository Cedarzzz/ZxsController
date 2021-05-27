// DlgRemoteMessage.cpp : 实现文件
//

#include "stdafx.h"
#include "XsServer.h"
#include "DlgRemoteMessage.h"
#include "afxdialogex.h"


// CDlgRemoteMessage 对话框

IMPLEMENT_DYNAMIC(CDlgRemoteMessage, CDialog)

CDlgRemoteMessage::CDlgRemoteMessage(CWnd* pParent, _CIOCPServer* IOCPServer,
	PCONTEXT_OBJECT ContextObject)
	: CDialog(IDD_DIALOG_REMOTE_MESSAGE, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgRemoteMessage::~CDlgRemoteMessage()
{
}

void CDlgRemoteMessage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DIALOG_REMOTE_MESSAGE_MAIN, m_CEdit_Dialog_Remote_Message_Main);
}


BEGIN_MESSAGE_MAP(CDlgRemoteMessage, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDlgRemoteMessage 消息处理程序


BOOL CDlgRemoteMessage::OnInitDialog()
{
	CDialog::OnInitDialog();
	//设置图标
	SetIcon(m_IconHwnd, FALSE);

	BYTE IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgRemoteMessage::OnClose()
{
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);

	CDialog::OnClose();
}


BOOL CDlgRemoteMessage::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		//点击Esc键退出窗口
		if (pMsg->wParam == VK_ESCAPE)
		{
			PostMessage(WM_CLOSE);
			return true;
		}

		//判断是否在m_CEdit_Dialog_Remote_Message_Main控件上点击了回车键
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_CEdit_Dialog_Remote_Message_Main.m_hWnd)
		{
			OnSending();
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}
void CDlgRemoteMessage::OnSending()
{
	//获得m_CEdit_Dialog_Remote_Message_Main控件上的数据长度
	int BufferLength = m_CEdit_Dialog_Remote_Message_Main.GetWindowTextLength();
	if (!BufferLength)
	{
		return;
	}
	CString v1;
	//获得m_CEdit_Dialog_Remote_Message_Main控件上的数据内容
	m_CEdit_Dialog_Remote_Message_Main.GetWindowText(v1);
	//IO通信套接字只支持char型数据
	char* BufferData = (char*)VirtualAlloc(NULL, BufferLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	memset(BufferData, 0, sizeof(char)*BufferLength);
	sprintf(BufferData, "%s", v1.GetBuffer(0));

	//清空m_CEdit_Dialog_Remote_Message_Main控件上的数据
	m_CEdit_Dialog_Remote_Message_Main.SetWindowText(NULL);

	//调用通信引擎类中的发送数据函数
	m_IOCPServer->OnPrepareSending(m_ContextObject,
		(LPBYTE)BufferData, strlen(BufferData));

	if (BufferData != NULL)
	{
		VirtualFree(BufferData, BufferLength, MEM_RELEASE);
		BufferData = NULL;
	}
}
