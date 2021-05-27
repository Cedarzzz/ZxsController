// DlgServiceManager.cpp : 实现文件
//

#include "stdafx.h"
#include "XsServer.h"
#include "DlgServiceManager.h"
#include "afxdialogex.h"


// CDlgServiceManager 对话框

IMPLEMENT_DYNAMIC(CDlgServiceManager, CDialog)

CDlgServiceManager::CDlgServiceManager(CWnd* pParent,
	_CIOCPServer* IOCPServer, PCONTEXT_OBJECT ContextObject)
	: CDialog(IDD_DIALOG_SERVICE_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgServiceManager::~CDlgServiceManager()
{

}

void CDlgServiceManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DIALOG_SERVICE_MANAGER_SHOW, m_CListCtrl_Dialog_Service_Manager_Show);
}


BEGIN_MESSAGE_MAP(CDlgServiceManager, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_DIALOG_SERVICE_MANAGER_SHOW, &CDlgServiceManager::OnNMRClickListDialogServiceManagerShow)
	ON_COMMAND(ID_MENU_DIALOG_SERVICE_MANAGER_LIST_SHOW_REFRESH, &CDlgServiceManager::OnMenuDialogServiceManagerListShowRefresh)
	ON_COMMAND(ID_MENU_DIALOG_SERVICE_MANAGER_LIST_SHOW_STRAT, &CDlgServiceManager::OnMenuDialogServiceManagerListShowStrat)
	ON_COMMAND(ID_MENU_DIALOG_SERVICE_MANAGER_LIST_SHOW_STOP, &CDlgServiceManager::OnMenuDialogServiceManagerListShowStop)
	ON_COMMAND(ID_MENU_DIALOG_SERVICE_MANAGER_LIST_SHOW_AUTO_RUN, &CDlgServiceManager::OnMenuDialogServiceManagerListShowAutoRun)
	ON_COMMAND(ID_MENU_DIALOG_SERVICE_MANAGER_LIST_SHOW_MANUAL_RUN, &CDlgServiceManager::OnMenuDialogServiceManagerListShowManualRun)
END_MESSAGE_MAP()


// CDlgServiceManager 消息处理程序


void CDlgServiceManager::OnClose()
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


BOOL CDlgServiceManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_IconHwnd, FALSE);
	//获得客户端IP地址
	CString v1;
	sockaddr_in ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);  //得到连接IP
	v1.Format("\\\\%s - 远程服务管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	m_CListCtrl_Dialog_Service_Manager_Show.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(0, "真实名称", LVCFMT_LEFT, 150);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(1, "显示名称", LVCFMT_LEFT, 260);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(2, "启动类型", LVCFMT_LEFT, 80);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(3, "运行状态", LVCFMT_LEFT, 80);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(4, "可执行文件路径", LVCFMT_LEFT, 380);

	ShowClientServiceList();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
//获得数据
int CDlgServiceManager::ShowClientServiceList()
{
	char* BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));
	char* DisplayName;
	char* ServiceName;
	char* RunWay;
	char* AutoRun;
	char* FileFullPath;
	DWORD Offset = 0;
	m_CListCtrl_Dialog_Service_Manager_Show.DeleteAllItems();

	int i = 0;
	for (i = 0; Offset < m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1; i++)
	{
		DisplayName = BufferData + Offset;
		ServiceName = DisplayName + lstrlen(DisplayName) + 1;
		FileFullPath = ServiceName + lstrlen(ServiceName) + 1;
		RunWay = FileFullPath + lstrlen(FileFullPath) + 1;
		AutoRun = RunWay + lstrlen(RunWay) + 1;

		m_CListCtrl_Dialog_Service_Manager_Show.InsertItem(i, ServiceName);
		m_CListCtrl_Dialog_Service_Manager_Show.SetItemText(i, 1, DisplayName);
		m_CListCtrl_Dialog_Service_Manager_Show.SetItemText(i, 2, AutoRun);
		m_CListCtrl_Dialog_Service_Manager_Show.SetItemText(i, 3, RunWay);
		m_CListCtrl_Dialog_Service_Manager_Show.SetItemText(i, 4, FileFullPath);

		Offset += lstrlen(DisplayName) + lstrlen(ServiceName) + lstrlen(FileFullPath) + lstrlen(RunWay)
			+ lstrlen(AutoRun) + 5;
	}
	CString  v1;
	v1.Format("服务个数:%d", i);
	LVCOLUMN  v3;
	v3.mask = LVCF_TEXT;
	v3.pszText = v1.GetBuffer(0);
	v3.cchTextMax = v1.GetLength();
	m_CListCtrl_Dialog_Service_Manager_Show.SetColumn(4, &v3);

	return 0;
}

void CDlgServiceManager::OnNMRClickListDialogServiceManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	CMenu Menu;
	Menu.LoadMenu(IDR_MENU_DIALOG_SERVICE_MANAGER_LIST_SHOW_MAIN);
	CMenu* SubMenu = Menu.GetSubMenu(0);
	CPoint Point;
	GetCursorPos(&Point);	
	SubMenu->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
		Point.x, Point.y, this);

	*pResult = 0;
}
void  CDlgServiceManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case  CLIENT_SERVICE_MANAGER_REPLY:
	{
		ShowClientServiceList();
		break;
	}
	default:
		break;
	}
}

void CDlgServiceManager::OnMenuDialogServiceManagerListShowRefresh()
{
	BYTE IsToken = CLIENT_SERVICE_MANAGER_REQUEST;   //刷新
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, 1);
}


void CDlgServiceManager::OnMenuDialogServiceManagerListShowStrat()
{
	ConfigClientService(1);
}
void CDlgServiceManager::OnMenuDialogServiceManagerListShowStop()
{
	ConfigClientService(2);
}
void CDlgServiceManager::OnMenuDialogServiceManagerListShowAutoRun()
{
	ConfigClientService(3);
}
void CDlgServiceManager::OnMenuDialogServiceManagerListShowManualRun()
{
	ConfigClientService(4);
}
void CDlgServiceManager::ConfigClientService(BYTE IsMethod)
{
	DWORD Offset = 2;
	POSITION Position = m_CListCtrl_Dialog_Service_Manager_Show.GetFirstSelectedItemPosition();
	int Item = m_CListCtrl_Dialog_Service_Manager_Show.GetNextSelectedItem(Position);
	CString v1 = m_CListCtrl_Dialog_Service_Manager_Show.GetItemText(Item, 0);

	char* ServiceName = v1.GetBuffer(0);
	//构建数据包
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 3 + lstrlen(ServiceName));
	//配置服务请求
	BufferData[0] = CLIENT_SERVICE_MANAGER_CONFIG_REQUEST;
	//配置形式 1 2 3 4
	BufferData[1] = IsMethod;
	memcpy(BufferData + Offset, ServiceName, lstrlen(ServiceName) + 1);
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}

