// DlgCmdManager.cpp : 实现文件
//

#include "stdafx.h"
#include "XsServer.h"
#include "DlgCmdManager.h"
#include "afxdialogex.h"


// CDlgCmdManager 对话框

IMPLEMENT_DYNAMIC(CDlgCmdManager, CDialog)

CDlgCmdManager::CDlgCmdManager(CWnd* pParent,
	_CIOCPServer* IOCPServer, PCONTEXT_OBJECT ContextObject)
	: CDialog(IDD_DIALOG_CMD_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgCmdManager::~CDlgCmdManager()
{
}

void CDlgCmdManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DIALOG_CMD_MANAGER_MAIN, m_CEdit_Dialog_Cmd_Manager_Main);
}


BEGIN_MESSAGE_MAP(CDlgCmdManager, CDialog)
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CDlgCmdManager 消息处理程序


BOOL CDlgCmdManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_IconHwnd, FALSE);
	//获得客户端IP地址
	CString v1;
	sockaddr_in ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);  //得到连接IP
	v1.Format("\\\\%s - 远程终端管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	BYTE IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(BYTE));


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgCmdManager::OnClose()
{
	if (m_ContextObject != NULL)
	{
		m_ContextObject->DialogID = 0;
	    m_ContextObject->DialogHandle = NULL;
	    CancelIo((HANDLE)m_ContextObject->ClientSocket);
	    closesocket(m_ContextObject->ClientSocket);
	}
	
	CDialog::OnClose();
}
void CDlgCmdManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	ShowCmdData();
	m_ShowDataLength = m_CEdit_Dialog_Cmd_Manager_Main.GetWindowTextLength();
}
VOID CDlgCmdManager::ShowCmdData(void)
{
	//从被控端来的数据加上一个 \0
	m_ContextObject->m_ReceivedDecompressedBufferData.WriteArray((LPBYTE)"", 1);
	//获得所有数据 包括\0
	CString v1 = (char*)m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0);
	//替换掉原来的换行符
	v1.Replace("\n", "\r\n");
	//得到当前窗口字符个数
	int BufferLength =  m_CEdit_Dialog_Cmd_Manager_Main.GetWindowTextLength();
	//将光标定位到该位置并选中指定个数的字符 即末尾 将从被控端传来的数据显示在我们先前内容的后面
	m_CEdit_Dialog_Cmd_Manager_Main.SetSel(BufferLength, BufferLength);

	//用传递过来的数据替换掉该位置的字符   //显示
	m_CEdit_Dialog_Cmd_Manager_Main.ReplaceSel(v1);
	//重新获得字符大小
	m_911 = m_CEdit_Dialog_Cmd_Manager_Main.GetWindowTextLengthA();

}

BOOL CDlgCmdManager::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		//屏蔽VK_ESCAPE和VK_DELETE
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
		{
			return true;
		}
		//如果是可编辑的回车键
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_CEdit_Dialog_Cmd_Manager_Main.m_hWnd)
		{
			//得到窗口数据大小
			int BufferLength = m_CEdit_Dialog_Cmd_Manager_Main.GetWindowTextLength();
			CString BufferData;
			//得到窗口的字符数据
			m_CEdit_Dialog_Cmd_Manager_Main.GetWindowText(BufferData);
			//加入换行符
			BufferData += "\r\n";

			//发送数据
			m_IOCPServer->OnPrepareSending(m_ContextObject, (LPBYTE)BufferData.GetBuffer(0) + m_911,
				BufferData.GetLength() - m_911);
			//重新定位m_911
			m_911 = m_CEdit_Dialog_Cmd_Manager_Main.GetWindowTextLength();

		}
		//限制VK_BACK
		if (pMsg->wParam == VK_BACK && pMsg->hwnd == m_CEdit_Dialog_Cmd_Manager_Main.m_hWnd)
		{
			if (m_CEdit_Dialog_Cmd_Manager_Main.GetWindowTextLength() <= m_ShowDataLength)
			{
				return true;
			}
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH CDlgCmdManager::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((pWnd->GetDlgCtrlID() == IDC_EDIT_DIALOG_CMD_MANAGER_MAIN) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF ColorReference = RGB(255, 255, 255);
		pDC->SetTextColor(ColorReference);  //白色文本
		ColorReference = RGB(0, 0, 0);
		pDC->SetBkColor(ColorReference);    //黑色背景
		return CreateSolidBrush(ColorReference);
	}
	else
	{
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}

	return hbr;
}
