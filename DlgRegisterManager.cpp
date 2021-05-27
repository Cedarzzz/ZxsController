// DlgRegisterManager.cpp : 实现文件
//

#include "stdafx.h"
#include "XsServer.h"
#include "DlgRegisterManager.h"
#include "afxdialogex.h"


// CDlgRegisterManager 对话框

enum MYKEY
{
	MHKEY_CLASSES_ROOT,
	MHKEY_CURRENT_USER,
	MHKEY_LOCAL_MACHINE,
	MHKEY_USERS,
	MHKEY_CURRENT_CONFIG
};

enum KEYVALUE
{
	MREG_SZ,
	MREG_DWORD,
	MREG_BINARY,
	MREG_EXPAND_SZ
};

struct REGMSG
{
	int Count;      //名字个数
	DWORD Size;     //名字大小
	DWORD valsize;  //值大小
};

IMPLEMENT_DYNAMIC(CDlgRegisterManager, CDialog)

CDlgRegisterManager::CDlgRegisterManager(CWnd* pParent,
	_CIOCPServer* IOCPServer, PCONTEXT_OBJECT ContextObject)
	: CDialog(IDD_DIALOG_REGISTER_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgRegisterManager::~CDlgRegisterManager()
{
}

void CDlgRegisterManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_DIALOG_REGISTER_MANAGER_SHOW, m_CTreeCtrl_Dialog_Register_Manager_Show);
	DDX_Control(pDX, IDC_LIST_DIALOG_REGISTER_MANAGER_SHOW, m_CListCtrl_Dialog_Register_Manager_Show);
}


BEGIN_MESSAGE_MAP(CDlgRegisterManager, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_DIALOG_REGISTER_MANAGER_SHOW, &CDlgRegisterManager::OnTvnSelchangedTreeDialogRegisterManagerShow)
END_MESSAGE_MAP()


// CDlgRegisterManager 消息处理程序
void  CDlgRegisterManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case  CLIENT_REGISTER_MANAGER_PATH_DATA_REPLY:
	{
		AddPath((char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1)));
		break;
	}
	case CLIENT_REGISTER_MANAGER_KEY_DATA_REPLY:
	{
		AddKey((char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1)));
		break;
	}
	default:
		break;
	}
}

BOOL CDlgRegisterManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_IconHwnd, FALSE);
	//获得客户端IP地址
	CString v1;
	sockaddr_in ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);  //得到连接IP
	v1.Format("\\\\%s - 远程注册表管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	//制作树控件上的图标
	m_ImageListTree.Create(18, 18, ILC_COLOR16, 10, 0);
	m_IconHwnd = (HICON)::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FATHER), IMAGE_ICON, 18, 18, 0);
	m_ImageListTree.Add(m_IconHwnd);
	m_IconHwnd = (HICON)::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FILE), IMAGE_ICON, 18, 18, 0);
	m_ImageListTree.Add(m_IconHwnd);

	m_CTreeCtrl_Dialog_Register_Manager_Show.SetImageList(&m_ImageListTree, TVSIL_NORMAL);

	m_hRoot = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("注册表管理", 0, 0, 0, 0);
	HKCU = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_CURRENT_USER", 1, 1, m_hRoot, 0);
	HKLM = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_LOCAL_MACHINE", 1, 1, m_hRoot, 0);
	HKUS = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_USERS", 1, 1, m_hRoot, 0);
	HKCC = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_CURRENT_CONFIG", 1, 1, m_hRoot, 0);
	HKCR = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_CLASSES_ROOT", 1, 1, m_hRoot, 0);

	m_CTreeCtrl_Dialog_Register_Manager_Show.Expand(m_hRoot, TVE_EXPAND);


	//List控件上的图标设置
	m_CListCtrl_Dialog_Register_Manager_Show.InsertColumn(0, "名称", LVCFMT_LEFT, 150, -1);
	m_CListCtrl_Dialog_Register_Manager_Show.InsertColumn(1, "类型", LVCFMT_LEFT, 60, -1);
	m_CListCtrl_Dialog_Register_Manager_Show.InsertColumn(2, "数据", LVCFMT_LEFT, 300, -1);
	m_CListCtrl_Dialog_Register_Manager_Show.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	//添加图标
	m_ImageListList.Create(16, 16, TRUE, 2, 2);
	m_ImageListList.Add(AfxGetApp()->LoadIcon(IDI_ICON_STRING));
	m_ImageListList.Add(AfxGetApp()->LoadIcon(IDI_ICON_DWORD));

	m_CListCtrl_Dialog_Register_Manager_Show.SetImageList(&m_ImageListList, LVSIL_SMALL);

	m_IsEnable = TRUE;   //解决频繁向被控端请求

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgRegisterManager::OnClose()
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


void CDlgRegisterManager::OnTvnSelchangedTreeDialogRegisterManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	
	if (!m_IsEnable)
	{
		return;
	}

	m_IsEnable = FALSE;

	//获得你点击的Item
	TVITEM Item = pNMTreeView->itemNew;

	if (Item.hItem == m_hRoot)
	{
		m_IsEnable = TRUE;
		return;
	}

	m_SelectedTreeItem = Item.hItem;  //保存用户打开的子树节点句柄
	m_CListCtrl_Dialog_Register_Manager_Show.DeleteAllItems();

	CString BufferData = GetFullPath(m_SelectedTreeItem);  //获得键值句柄

	char IsToken = GetFatherPath(BufferData);

	while (m_CTreeCtrl_Dialog_Register_Manager_Show.GetChildItem(Item.hItem) != NULL)
	{
		m_CTreeCtrl_Dialog_Register_Manager_Show.DeleteItem(m_CTreeCtrl_Dialog_Register_Manager_Show.GetChildItem(Item.hItem));
	}

	//预先加一个键
	int iItem = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, "(默认)", 0);
	m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(iItem, 1, "REG_SZ");
	m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(iItem, 1, "(数据未设置值)");

	BufferData.Insert(0, IsToken);  //插入 根键
	IsToken = CLIENT_REGISTER_MANAGER_DATA_CONTINUE;
	BufferData.Insert(0, IsToken);  //插入查询命令
	m_IOCPServer->OnPrepareSending(m_ContextObject, (LPBYTE)(BufferData.GetBuffer(0)),
		BufferData.GetLength() + 1);

	m_IsEnable = TRUE;
	*pResult = 0;
}
CString CDlgRegisterManager::GetFullPath(HTREEITEM Current)
{
	CString strTemp;
	CString strReturn = "";
	while (1)
	{
		if (Current == m_hRoot)
		{
			return strReturn;
		}
		strTemp = m_CTreeCtrl_Dialog_Register_Manager_Show.GetItemText(Current);
		if (strTemp.Right(1) != "\\")
		{
			strTemp += "\\";
		}
		strReturn = strTemp + strReturn;
		Current = m_CTreeCtrl_Dialog_Register_Manager_Show.GetParentItem(Current);  //得到父的
	}
	return strReturn;
}
char CDlgRegisterManager::GetFatherPath(CString& FullPath)
{
	char IsToken;
	if (!FullPath.Find("HKEY_CLASSES_ROOT"))  //判断主键
	{
		IsToken = MHKEY_CLASSES_ROOT;
		FullPath.Delete(0, sizeof("HKEY_CLASSES_ROOT"));
	}
	else if (!FullPath.Find("HKEY_CURRENT_USER"))
	{
		IsToken = MHKEY_CURRENT_USER;
		FullPath.Delete(0, sizeof("HKEY_CURRENT_USER"));
	}
	else if (!FullPath.Find("HKEY_LOCAL_MACHINE"))
	{
		IsToken = MHKEY_LOCAL_MACHINE;
		FullPath.Delete(0, sizeof("HKEY_LOCAL_MACHINE"));
	}
	else if (!FullPath.Find("HKEY_USERS"))
	{
		IsToken = MHKEY_USERS;
		FullPath.Delete(0, sizeof("HKEY_USERS"));
	}
	else if (!FullPath.Find("HKEY_CURRENT_CONFIG"))
	{
		IsToken = MHKEY_CURRENT_CONFIG;
		FullPath.Delete(0, sizeof("HKEY_CURRENT_CONFIG"));
	}
	return IsToken;
}
void CDlgRegisterManager::AddPath(char* BufferData)
{
	
	if (BufferData == NULL)
	{
		return;
	}
	int msgsize = sizeof(REGMSG);
	REGMSG msg;
	memcpy((void*)&msg, BufferData, msgsize);
	DWORD Size = msg.Size;
	int Count = msg.Count;

	if (Size > 0 && Count > 0)   //保护措施
	{
		for (int i = 0; i < Count; i++)
		{
			char* szKeyName = BufferData + Size*i + msgsize;
			m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem(szKeyName, 1, 1, m_SelectedTreeItem, 0);  //插入子健名称
			m_CTreeCtrl_Dialog_Register_Manager_Show.Expand(m_SelectedTreeItem, TVE_EXPAND);
		}
	}
}
void CDlgRegisterManager::AddKey(char* BufferData)
{
	m_CListCtrl_Dialog_Register_Manager_Show.DeleteAllItems();
	int Item = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, "(DATA)", 0);
	m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 1, "REG_SZ");
	m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 2, "(NULL)");

	if (BufferData == NULL)
	{
		return;
	}
	REGMSG msg;
	memcpy((void*)&msg, BufferData, sizeof(msg));
	char* szTemp = BufferData + sizeof(msg);
	for (int i = 0; i < msg.Count; i++)
	{
		BYTE Type = szTemp[0];  //类型
		szTemp += sizeof(BYTE);
		char* ValueName = szTemp;  //取出名字
		szTemp += msg.Size;
		BYTE* szValueData = (BYTE*)szTemp;  //取出值
		szTemp += msg.valsize;
		if (Type == MREG_SZ)
		{
			int iItem = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, ValueName, 0);
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(iItem, 1, "REG_SZ");
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(iItem, 2, (char*)szValueData);
		}
		if (Type == MREG_DWORD)
		{
			char ValueData[256];
			DWORD d = (DWORD)szValueData;
			memcpy((void*)&d, szValueData, sizeof(DWORD));
			CString strValue;
			strValue.Format("0x%x", d);
			
			int iItem = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, ValueName, 1);
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(iItem, 1, "REG_DWORD");
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(iItem, 2, strValue);
		}
		if (Type == MREG_BINARY)
		{
			char ValueData[256];
			sprintf(ValueData, "  (%d)", szValueData);
			int iItem = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, ValueName, 1);
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(iItem, 1, "REG_BINARY");
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(iItem, 2, ValueData);
		}
		if (Type == MREG_EXPAND_SZ)
		{
			int iItem = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, ValueName, 0);
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(iItem, 1, "REG_EXPAND_SZ");
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(iItem, 2, (char*)szValueData);
		}
	}
}