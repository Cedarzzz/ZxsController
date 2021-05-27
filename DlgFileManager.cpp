// DlgFileManager.cpp : 实现文件
//

#include "stdafx.h"
#include "XsServer.h"
#include "DlgFileManager.h"
#include "afxdialogex.h"
#include "DlgNewFolder.h"
#include "DlgTransferMode.h"


// CDlgFileManager 对话框

static UINT __Indicators[] = 
{ 
	ID_SEPARATOR,
	ID_SEPARATOR,
	IDR_STATUSBAR_FILE_MANAGER_PROCESS
};

IMPLEMENT_DYNAMIC(CDlgFileManager, CDialog)

CDlgFileManager::CDlgFileManager(CWnd* pParent,
	_CIOCPServer* IOCPServer, PCONTEXT_OBJECT ContextObject)
	: CDialog(IDD_DIALOG_FILE_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	memset(m_ClientData, 0, sizeof(m_ClientData));
	memcpy(m_ClientData, ContextObject->m_ReceivedDecompressedBufferData.GetArray(1),
		ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1);

	SHFILEINFO v1;
	//卷图标
	HIMAGELIST ImageListHwnd;  //SDK
	//加载系统图标
	ImageListHwnd = (HIMAGELIST)SHGetFileInfo(
		NULL,
		0,
		&v1,
		sizeof(SHFILEINFO),
		SHGFI_LARGEICON | SHGFI_SYSICONINDEX);

	m_CImageList_Large = CImageList::FromHandle(ImageListHwnd);

	ImageListHwnd = (HIMAGELIST)SHGetFileInfo(NULL, 0, &v1,
		sizeof(SHFILEINFO),SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
	m_CImageList_Small = CImageList::FromHandle(ImageListHwnd);

	m_IsDragging = FALSE;
    m_IsStop = FALSE;   


}

CDlgFileManager::~CDlgFileManager()
{
}

void CDlgFileManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DIALOG_FILE_MANAGER_SERVER_FILE, m_CListCtrl_Dialog_File_Manager_Server_File);
	DDX_Control(pDX, IDC_LIST_DIALOG_FILE_MANAGER_CLIENT_FILE, m_CListCtrl_Dialog_File_Manager_Client_File);
	DDX_Control(pDX, IDC_STATIC_DIALOG_FILE_MANAGER_SERVER_POSITION, m_CStatic_Dialog_File_Manager_Server_Position);
	DDX_Control(pDX, IDC_STATIC_DIALOG_FILE_MANAGER_CLIENT_POSITION, m_CStatic_Dialog_File_Manager_Client_Position);
	DDX_Control(pDX, IDC_COMBO_DIALOG_FILE_MANAGER_SERVER_FILE, m_CComboBox_Dialog_File_Manager_Server_File);
	DDX_Control(pDX, IDC_COMBO_DIALOG_FILE_MANAGER_CLIENT_FILE, m_CComboBox_Dialog_File_Manager_Client_File);
}


BEGIN_MESSAGE_MAP(CDlgFileManager, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DIALOG_FILE_MANAGER_SERVER_FILE, &CDlgFileManager::OnNMDblclkListDialogFileManagerServerFile)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DIALOG_FILE_MANAGER_CLIENT_FILE, &CDlgFileManager::OnNMDblclkListDialogFileManagerClientFile)
	ON_COMMAND(IDT_TOOLBAR_DIALOG_FILE_MANAGER_SERVER_FILE_PREVIOUS, &CDlgFileManager::OnFileManagerServerFilePrevious)
	ON_COMMAND(IDT_TOOLBAR_DIALOG_FILE_MANAGER_CLIENT_FILE_PREVIOUS, &CDlgFileManager::OnFileManagerClientFilePrevious)
	ON_COMMAND(IDT_TOOLBAR_DIALOG_FILE_MANAGER_SERVER_FILE_DELETE, &CDlgFileManager::OnFileManagerServerFileDelete)
	ON_COMMAND(IDT_TOOLBAR_DIALOG_FILE_MANAGER_SERVER_FILE_NEW_FOLDER, &CDlgFileManager::OnFileManagerServerFileNewFolder)
	ON_WM_MOUSEMOVE()
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_DIALOG_FILE_MANAGER_SERVER_FILE, &CDlgFileManager::OnLvnBegindragListDialogFileManagerServerFile)
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CDlgFileManager 消息处理程序


BOOL CDlgFileManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_IconHwnd, FALSE);
	//获得客户端IP地址
	CString v1;
	sockaddr_in ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);  //得到连接IP
	v1.Format("\\\\%s - 远程文件管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	//界面设置
	if (!m_ToolBar_Dialog_File_Manager_Server_File.Create(this, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY,
		IDR_TOOLBAR_DIALOG_FILE_MANAGER_SERVER_FILE) ||
		!m_ToolBar_Dialog_File_Manager_Server_File.LoadToolBar(IDR_TOOLBAR_DIALOG_FILE_MANAGER_SERVER_FILE))
	{
		return -1;
	}
	m_ToolBar_Dialog_File_Manager_Server_File.LoadTrueColorToolBar(
			24,
			IDB_BITMAP_FILE_MANAGER,
			IDB_BITMAP_FILE_MANAGER,
			IDB_BITMAP_FILE_MANAGER);

	m_ToolBar_Dialog_File_Manager_Server_File.AddDropDownButton(this, IDT_TOOLBAR_DIALOG_FILE_MANAGER_SERVER_FILE_VIEW,
		IDT_TOOLBAR_DIALOG_FILE_MANAGER_SERVER_FILE_VIEW);
	m_ToolBar_Dialog_File_Manager_Server_File.SetButtonText(0, "返回");
	m_ToolBar_Dialog_File_Manager_Server_File.SetButtonText(1, "查看");
	m_ToolBar_Dialog_File_Manager_Server_File.SetButtonText(2, "删除");
	m_ToolBar_Dialog_File_Manager_Server_File.SetButtonText(3, "新建");
	m_ToolBar_Dialog_File_Manager_Server_File.SetButtonText(4, "查找");
	m_ToolBar_Dialog_File_Manager_Server_File.SetButtonText(5, "停止");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	m_CListCtrl_Dialog_File_Manager_Server_File.SetImageList(m_CImageList_Large, LVSIL_NORMAL);
	m_CListCtrl_Dialog_File_Manager_Server_File.SetImageList(m_CImageList_Small, LVSIL_SMALL);
	
	//界面设置  客户端TOOLBAR
	if (!m_ToolBar_Dialog_File_Manager_Client_File.Create(this, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY,
		IDR_TOOLBAR_DIALOG_FILE_MANAGER_CLIENT_FILE) ||
		!m_ToolBar_Dialog_File_Manager_Client_File.LoadToolBar(IDR_TOOLBAR_DIALOG_FILE_MANAGER_CLIENT_FILE))
	{
		return -1;
	}
	m_ToolBar_Dialog_File_Manager_Client_File.LoadTrueColorToolBar(
			24,
			IDB_BITMAP_FILE_MANAGER,
			IDB_BITMAP_FILE_MANAGER,
			IDB_BITMAP_FILE_MANAGER);
	
	m_ToolBar_Dialog_File_Manager_Client_File.AddDropDownButton(this, IDT_TOOLBAR_DIALOG_FILE_MANAGER_CLIENT_FILE_VIEW,
		IDT_TOOLBAR_DIALOG_FILE_MANAGER_CLIENT_FILE_VIEW);
	m_ToolBar_Dialog_File_Manager_Client_File.SetButtonText(0, "返回");
	m_ToolBar_Dialog_File_Manager_Client_File.SetButtonText(1, "查看");
	m_ToolBar_Dialog_File_Manager_Client_File.SetButtonText(2, "删除");
	m_ToolBar_Dialog_File_Manager_Client_File.SetButtonText(3, "新建");
	m_ToolBar_Dialog_File_Manager_Client_File.SetButtonText(4, "查找");
	m_ToolBar_Dialog_File_Manager_Client_File.SetButtonText(5, "停止");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	
	m_CListCtrl_Dialog_File_Manager_Client_File.SetImageList(m_CImageList_Large, LVSIL_NORMAL);
	m_CListCtrl_Dialog_File_Manager_Client_File.SetImageList(m_CImageList_Small, LVSIL_SMALL);

	//获得整个窗口大小
	RECT Rect;
	GetClientRect(&Rect);

	CRect v3;
	v3.top = Rect.bottom - 25;
	v3.left = 0;
	v3.right = Rect.right;
	v3.bottom = Rect.bottom;
	if (!m_StatusBar.Create(this) ||
		!m_StatusBar.SetIndicators(__Indicators,
			sizeof(__Indicators) / sizeof(UINT)))
	{
		return -1;
	}
	m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_StatusBar.SetPaneInfo(1, m_StatusBar.GetItemID(1), SBPS_NORMAL, 120);
	m_StatusBar.SetPaneInfo(2, m_StatusBar.GetItemID(2), SBPS_NORMAL, 50);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);   //显示状态栏

	m_StatusBar.MoveWindow(v3);
	m_StatusBar.GetItemRect(1, &Rect);

	Rect.bottom -= 1;

	m_ProgressCtrl = new CProgressCtrl;
	m_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, Rect, &m_StatusBar, 1);
	m_ProgressCtrl->SetRange(0, 100);   //设置进度条范围
	m_ProgressCtrl->SetPos(0);


	//----------------------------------------------Server真彩
	RECT ServerRect;
	m_CStatic_Dialog_File_Manager_Server_Position.GetWindowRect(&ServerRect);
	m_CStatic_Dialog_File_Manager_Server_Position.ShowWindow(SW_HIDE);
	//显示工具栏
	m_ToolBar_Dialog_File_Manager_Server_File.MoveWindow(&ServerRect);

	//----------------------------------------------Client真彩

	RECT  ClientRect;
	//静态文本框
	m_CStatic_Dialog_File_Manager_Client_Position.GetWindowRect(&ClientRect);
	m_CStatic_Dialog_File_Manager_Client_Position.ShowWindow(SW_HIDE);
	//显示工具栏
	m_ToolBar_Dialog_File_Manager_Client_File.MoveWindow(&ClientRect);

	FixedServerVolumeData();
	FixedClientVolumeData();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CDlgFileManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case CLIENT_FILE_MANAGER_FILE_DATA_REPLY:
	{
		FixedClientFileData(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(),
			m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1);
		break;
	}
	case CLIENT_FILE_MANAGER_TRANSFER_MODE_REQUEST:
	{
		//在客户端中发现有重名文件
		SendTransferMode();
		break;
	}
	case CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE:
	{
		//发送数据
		SendServerFileDataToClient();
		break;
	}
	default:
		break;
	}

}
VOID CDlgFileManager::SendTransferMode()
{
	CDlgTransferMode Dialog(this);
	Dialog.m_FileName = m_DestFileFullPath;
	switch (Dialog.DoModal())
	{
	case IDC_BUTTON_DIALOG_TRANSFER_MODE_COVER:
	{
		m_TransferMode = TRANSFER_MODE_COVER;
		break;
	}
	case IDC_BUTTON_DIALOG_TRANSFER_MODE_COVER_ALL:
	{
		m_TransferMode = TRANSFER_MODE_COVER_ALL;
		break;
	}
	case IDC_BUTTON_DIALOG_TRANSFER_MODE_JUMP:
	{
		m_TransferMode = TRANSFER_MODE_JUMP;
		break;
	}
	case IDC_BUTTON_DIALOG_TRANSFER_MODE_JUMP_ALL:
	{
		m_TransferMode = TRANSFER_MODE_JUMP_ALL;
		break;
	}
	case IDCANCEL:
	{
		m_TransferMode = TRANSFER_MODE_CANCEL;
		break;
	}
	}
	if (m_TransferMode == TRANSFER_MODE_CANCEL)
	{
		EndCopyServerFileToClient();   //结束拷贝
		return;
	}
	BYTE IsToken[5];
	IsToken[0] = CLIENT_FILE_MANAGER_SET_TRANSFER_MODE;
	memcpy(IsToken + 1, &m_TransferMode, sizeof(m_TransferMode));
	m_IOCPServer->OnPrepareSending(m_ContextObject, (unsigned char*)&IsToken, sizeof(IsToken));
}
void CDlgFileManager::OnClose()
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
//获得磁盘卷信息
VOID CDlgFileManager::FixedServerVolumeData()
{
	char VolumeData[0x500] = { 0 };
	CHAR* Travel = NULL;
	m_CListCtrl_Dialog_File_Manager_Server_File.DeleteAllItems();

	while (m_CListCtrl_Dialog_File_Manager_Server_File.DeleteColumn(0) != 0);

	//初始化列表信息
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(0, "名称", LVCFMT_CENTER, 50);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(1, "类型", LVCFMT_CENTER, 80);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(2, "文件系统", LVCFMT_CENTER, 70);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(3, "总大小", LVCFMT_CENTER, 80);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(4, "可用空间", LVCFMT_CENTER, 80);

	m_CListCtrl_Dialog_File_Manager_Server_File.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	GetLogicalDriveStrings(sizeof(VolumeData), (LPSTR)VolumeData);
	Travel = VolumeData;

	CHAR FileSystemType[MAX_PATH];
	unsigned __int64 HardDiskAmount = 0;
	unsigned __int64 HardDiskFreeSpace = 0;
	unsigned long    HardDiskAmountMB = 0;     //总大小
	unsigned long    HardDiskFreeSpaceMB = 0;  //剩余空间

	for (int i = 1; *Travel != '\0';i++, Travel += lstrlen(Travel) + 1)
	{
		memset(FileSystemType, 0, sizeof(FileSystemType));  //文件系统  NTFS
		//得到文件系统信息及大小
		GetVolumeInformation(Travel, NULL, 0, NULL, NULL, NULL, FileSystemType, MAX_PATH);

		ULONG FileSystemTypeLength = lstrlen(FileSystemType) + 1;
		if (GetDiskFreeSpaceEx(Travel, (PULARGE_INTEGER)&HardDiskFreeSpace,
			(PULARGE_INTEGER)&HardDiskAmount, NULL))
		{
			HardDiskAmountMB = HardDiskAmount / 1024 / 1024;   //这里获得为字节 要转换成G
			HardDiskFreeSpaceMB = HardDiskFreeSpace / 1024 / 1024;
		}
		else
		{
			HardDiskAmountMB = 0;
			HardDiskFreeSpaceMB = 0;
		}

		int Item = m_CListCtrl_Dialog_File_Manager_Server_File.InsertItem(i, Travel,
			GetIconIndex(Travel, GetFileAttributes(Travel)));   //获得系统图标

		m_CListCtrl_Dialog_File_Manager_Server_File.SetItemData(Item, 1);  //这里将代表目录的隐藏数据插入到改项
		m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 2, FileSystemType);

		SHFILEINFO v1;
		SHGetFileInfo(Travel, FILE_ATTRIBUTE_NORMAL, &v1,
			sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);

		m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 1, v1.szTypeName);

		CString v2;
		v2.Format("%10.lf GB", (float)HardDiskAmountMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 3, v2);
		v2.Format("%10.lf GB", (float)HardDiskFreeSpaceMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 4, v2);
	}
}
VOID CDlgFileManager::FixedClientVolumeData()
{
	m_CListCtrl_Dialog_File_Manager_Client_File.DeleteAllItems();
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(0, "名称", LVCFMT_CENTER, 50);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(1, "类型", LVCFMT_CENTER, 80);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(2, "文件系统", LVCFMT_CENTER, 70);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(3, "总大小", LVCFMT_CENTER, 80);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(4, "可用空间", LVCFMT_CENTER, 80);

	m_CListCtrl_Dialog_File_Manager_Client_File.SetExtendedStyle(LVS_EX_FULLROWSELECT);


	char* Travel = NULL;
	Travel = (char*)m_ClientData;   //去掉了消息头的一个字节了

	int i = 0;
	ULONG v1 = 0;
	for (i = 0; Travel[i] != '\0';)
	{
		//由驱动器名判断图标的索引
		if (Travel[i] == 'A' || Travel[i] == 'B')
		{
			v1 = 6;
		}
		else
		{
			switch (Travel[i + 1])   //这里是判断驱动器的类型 查看被控端
			{
			case DRIVE_REMOVABLE:
				v1 = 2 + 5;
				break;
			case DRIVE_FIXED:
				v1 = 3 + 5;
				break;
			case DRIVE_REMOTE:
				v1 = 4 + 5;
				break;
			case DRIVE_CDROM:
				v1 = 9;
				break;
			default:
				v1 = 0;
				break;

			}
		}
		CString v2;
		//格式化盘符
		v2.Format("%c:\\", Travel[i]);
		int Item = m_CListCtrl_Dialog_File_Manager_Client_File.InsertItem(i, v2, v1);
		m_CListCtrl_Dialog_File_Manager_Client_File.SetItemData(Item, 1);  //该数据不显示 隐藏在当前项中    1代表目录 0代表文件

		unsigned long    HardDiskAmountMB = 0;     //总大小
	    unsigned long    HardDiskFreeSpaceMB = 0;  //剩余空间
		memcpy(&HardDiskAmountMB, Travel + i + 2, 4);
		memcpy(&HardDiskFreeSpaceMB, Travel + i + 6, 4);

		v2.Format("%10.lf GB", (float)HardDiskAmountMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 3, v2);
		v2.Format("%10.lf GB", (float)HardDiskFreeSpaceMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 4, v2);

		i += 10;

		CString v7;
		v7 = Travel + i;
		m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 1, v7);

		i += strlen(Travel + i) + 1;

		CString FileSystemType;
		FileSystemType = Travel + i;
		m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 2, FileSystemType);
		i += strlen(Travel + i) + 1;

	}
}
//获得服务端文件数据
VOID CDlgFileManager::FixedServerFileData(CString Directory)
{
	if (Directory.GetLength() == 0)
	{
		//取出ControlList中的项
		int Item = m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectionMark();

		//如果有选中
		if (Item != -1)
		{
			//获得该项中的隐藏数据
			if (m_CListCtrl_Dialog_File_Manager_Server_File.GetItemData(Item) == 1)
			{
				//是目录
				Directory = m_CListCtrl_Dialog_File_Manager_Server_File.GetItemText(Item, 0);
			}
		}
		//从组合框里得到路径
		else
		{
			//
		}
	}
	if (Directory == "..")
	{
		//返回上一层目录
		m_ServerFileFullPath = GetParentDirectory(m_ServerFileFullPath);   //笔记

	}
	//刷新当前用
	else if (Directory != ".")   //在系统中的每个目录中都会存在一个.或..目录
	{
		m_ServerFileFullPath += Directory;
		if (m_ServerFileFullPath.Right(1) != "\\")
		{
			//如果该字符串的右边不存在'\\'
			m_ServerFileFullPath += "\\";
		}

	}
	if (m_ServerFileFullPath.GetLength() == 0)
	{
		FixedServerVolumeData();
		return;
	}

	//将最终的文件路径放入到控件中
	m_CComboBox_Dialog_File_Manager_Server_File.InsertString(0, m_ServerFileFullPath);
	m_CComboBox_Dialog_File_Manager_Server_File.SetCurSel(0);

	//删除ControlList上的项
	m_CListCtrl_Dialog_File_Manager_Server_File.DeleteAllItems();
	while (m_CListCtrl_Dialog_File_Manager_Server_File.DeleteColumn(0) != 0);

	//初始化列表信息
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(0, "名称", LVCFMT_LEFT, 150);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(1, "大小", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(2, "类型", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(3, "修改日期", LVCFMT_LEFT, 115);


	int v10 = 0;

	//自己在ControlList控件上写入一个..目录（一旦双击就返回上一层）
	m_CListCtrl_Dialog_File_Manager_Server_File.SetItemData(m_CListCtrl_Dialog_File_Manager_Server_File.InsertItem(v10++, "..",
		GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)), 1);

	//循环两次代表两种类型(目录与文件)
	for (int i = 0; i < 2; i++)
	{
		CFileFind FileFind;
		BOOL IsLoop;
		IsLoop = FileFind.FindFile(m_ServerFileFullPath + "*.*");
		while (IsLoop)
		{
			IsLoop = FileFind.FindNextFile();
			if (FileFind.IsDots())
			{
				continue;
			}
			BOOL IsInsert = !FileFind.IsDirectory() == i;
			if (!IsInsert)
			{
				continue;
			}
			int Item = m_CListCtrl_Dialog_File_Manager_Server_File.InsertItem(v10++, FileFind.GetFileName(),
				GetIconIndex(FileFind.GetFileName(), GetFileAttributes(FileFind.GetFilePath())));
			//如果是目录设置隐藏数据为1
			m_CListCtrl_Dialog_File_Manager_Server_File.SetItemData(Item, FileFind.IsDirectory());
			SHFILEINFO v1;
			SHGetFileInfo(FileFind.GetFileName(), FILE_ATTRIBUTE_NORMAL, &v1, sizeof(SHFILEINFO),
				SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);

			if (FileFind.IsDirectory())
			{
				m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 2, "文件夹");
			}
			else
			{
				m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 2, v1.szTypeName);
			}
			CString v2;
			v2.Format("%10d KB", FileFind.GetLength() / 1024 + (FileFind.GetLength() % 1024 ? 1 : 0));
			m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 1, v2);
			CTime Time;
			FileFind.GetLastWriteTime(Time);
			m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 3, Time.Format("%Y-%m-%d %H:%M"));

		}

	}
}
//获得客户端文件数据
VOID CDlgFileManager::FixedClientFileData(BYTE* BufferData, ULONG BufferLength)
{
	//SHFILEINFO v1;
	//重建标题
	m_CListCtrl_Dialog_File_Manager_Client_File.DeleteAllItems();
	while (m_CListCtrl_Dialog_File_Manager_Client_File.DeleteColumn(0) != 0);

	//初始化列表信息
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(0, "名称", LVCFMT_LEFT, 150);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(1, "大小", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(2, "类型", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(3, "修改日期", LVCFMT_LEFT, 115);
	int v10 = 0;

	//自己在ControlList控件上写入一个..目录（一旦双击就返回上一层）
	m_CListCtrl_Dialog_File_Manager_Client_File.SetItemData(m_CListCtrl_Dialog_File_Manager_Client_File.InsertItem(v10++, "..",
		GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)), 1);
	if (BufferLength != 0)
	{
		for (int i = 0; i < 2; i++)
		{
			//跳过Token
			char* Travel = (char*)(BufferData + 1);
			for (char* v1 = Travel; Travel - v1 < BufferLength - 1;)
			{
				char* FileName = NULL;
				DWORD FileSizeHigh = 0;       //文件高字节大小
				DWORD FileSizeLow = 0;        //文件低字节大小
				int Item = 0;
				bool IsInsert = false;
				FILETIME FileTime;

				int v3 = *Travel ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
				//i为0时列目录i为1时列文件
				IsInsert = !(v3 == FILE_ATTRIBUTE_DIRECTORY) == i;

				FileName = ++Travel;
				if (IsInsert)
				{
					Item = m_CListCtrl_Dialog_File_Manager_Client_File.InsertItem(v10++, FileName, GetIconIndex(FileName, v3));
					m_CListCtrl_Dialog_File_Manager_Client_File.SetItemData(Item, v3 == FILE_ATTRIBUTE_DIRECTORY);  //隐藏属性
					SHFILEINFO v2;
					SHGetFileInfo(FileName, FILE_ATTRIBUTE_NORMAL | v3, &v2, sizeof(SHFILEINFO),
						SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
					m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 2, v2.szTypeName);

				}
				//得到文件大小
				Travel += strlen(FileName) + 1;
				if (IsInsert)
				{
					memcpy(&FileSizeHigh, Travel, 4);
					memcpy(&FileSizeLow, Travel+4, 4);
					CString v7;
					v7.Format("%10d KB", (FileSizeHigh*(MAXDWORD + 1)) / 1024 + FileSizeLow / 1024 + (FileSizeLow % 1024  ? 1 : 0));
					m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 1, v7);
					memcpy(&FileTime, Travel + 8, sizeof(FILETIME));
					CTime Time(FileTime);
					m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 3, Time.Format("%Y-%m-%D %H:%M"));
				}
				Travel += 16;
			}
		}
	}

	//恢复窗口
	m_CListCtrl_Dialog_File_Manager_Client_File.EnableWindow(TRUE);

}
//获得光标缩影
int CDlgFileManager::GetIconIndex(LPCTSTR VolumnName, DWORD FileAttributes)
{
	SHFILEINFO v1;
	if (FileAttributes == INVALID_FILE_ATTRIBUTES)
		FileAttributes = FILE_ATTRIBUTE_NORMAL;
	else
		FileAttributes |= FILE_ATTRIBUTE_NORMAL;
	SHGetFileInfo(
		VolumnName,
		FileAttributes,
		&v1,
		sizeof(SHFILEINFO),
		SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
	return v1.iIcon;
}
//返回上一层目录
CString CDlgFileManager::GetParentDirectory(CString FileFullPath)
{
	CString v1 = FileFullPath;
	int Index = v1.ReverseFind('\\');
	if (Index == -1)
	{
		return v1;
	}
	CString v2 = v1.Left(Index);
	Index = v2.ReverseFind('\\');
	if (Index == -1)
	{
		v1 = "";
		return v1;
	}
	v1 = v2.Left(Index);
	if (v1.Right(1) != "\\")
	{
		v1 += "\\";
	}
	return v1;
}
//双击项ControlList中的项
void CDlgFileManager::OnNMDblclkListDialogFileManagerServerFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	if (m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectedCount() == 0 ||
		m_CListCtrl_Dialog_File_Manager_Server_File.GetItemData(m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectionMark()) != 1)
	{
		//点击的不是目录
		return;
	}
	FixedServerFileData();
	*pResult = 0;
}
void CDlgFileManager::OnNMDblclkListDialogFileManagerClientFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	if (m_CListCtrl_Dialog_File_Manager_Client_File.GetSelectedCount() == 0 ||
		m_CListCtrl_Dialog_File_Manager_Client_File.GetItemData(m_CListCtrl_Dialog_File_Manager_Client_File.GetSelectionMark()) != 1)
	{
		return;
	}
	GetClientFileData();  //向客户端发信息

	*pResult = 0;
}
VOID CDlgFileManager::GetClientFileData(CString Directory)
{
	if (Directory.GetLength() == 0)
	{
		//取出ControlList中的项
		int Item = m_CListCtrl_Dialog_File_Manager_Client_File.GetSelectionMark();

		//如果有选中
		if (Item != -1)
		{
			//获得该项中的隐藏数据
			if (m_CListCtrl_Dialog_File_Manager_Client_File.GetItemData(Item) == 1)
			{
				//是目录
				Directory = m_CListCtrl_Dialog_File_Manager_Client_File.GetItemText(Item, 0);
			}
		}
		//从组合框里得到路径
		else
		{
			//
		}
	}
	if (Directory == "..")
	{
		//返回上一层目录
		m_ClientFileFullPath = GetParentDirectory(m_ClientFileFullPath);   //笔记

	}
	//刷新当前用
	else if (Directory != ".")   //在系统中的每个目录中都会存在一个.或..目录
	{
		m_ClientFileFullPath += Directory;
		if (m_ClientFileFullPath.Right(1) != "\\")
		{
			//如果该字符串的右边不存在'\\'
			m_ClientFileFullPath += "\\";
		}

	}
	if (m_ClientFileFullPath.GetLength() == 0)
	{
		//到达根目录 实际就是要刷新卷
		FixedClientVolumeData();
		return;
	}

	ULONG BufferLength = m_ClientFileFullPath.GetLength() + 2;
	BYTE* BufferData = (BYTE*)new BYTE[BufferLength];

	BufferData[0] = CLIENT_FILE_MANAGER_FILE_DATA_REQUEST;
	memcpy(BufferData + 1, m_ClientFileFullPath.GetBuffer(0), BufferLength - 1);
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);
	delete[] BufferData;
	BufferData = NULL;

	//得到数据前禁用窗口
	m_CListCtrl_Dialog_File_Manager_Client_File.EnableWindow(FALSE);  //不能点击
	m_ProgressCtrl->SetPos(0);  //初始化进度条

}
void CDlgFileManager::OnFileManagerServerFilePrevious()
{
	FixedServerFileData("..");
}
void CDlgFileManager::OnFileManagerServerFileDelete()
{
	CString v1;
	if (m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectedCount() > 1)
	{
		v1.Format("确定要将这%d项删除吗?", m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectedCount());
	}
	else
	{
		int Item = m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectionMark();
		if (Item == -1)
		{
			return;
		}
		CString FileName = m_CListCtrl_Dialog_File_Manager_Server_File.GetItemText(m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectionMark(), 0);

		//删除的是不是目录
		if (m_CListCtrl_Dialog_File_Manager_Server_File.GetItemData(Item) == 1)
		{
			v1.Format("确实要删除文件夹“%s”并将其所有内容删除吗?", FileName);
		}
		else
		{
			v1.Format("确实要把“%s”删除吗?", FileName);
		}

	}
	if (::MessageBox(m_hWnd, v1, "确认删除", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}
	//不能让用户乱点
	EnableControl(FALSE);

	POSITION Position = m_CListCtrl_Dialog_File_Manager_Server_File.GetFirstSelectedItemPosition();
	while (Position)
	{
		int Item = m_CListCtrl_Dialog_File_Manager_Server_File.GetNextSelectedItem(Position);
		CString FileFullPath = m_ServerFileFullPath + m_CListCtrl_Dialog_File_Manager_Server_File.GetItemText(Item, 0);
		//如果是目录
		if (m_CListCtrl_Dialog_File_Manager_Server_File.GetItemData(Item))
		{
			FileFullPath += '\\';
			DeleteDirectory(FileFullPath);
		}
		else
		{
			DeleteFile(FileFullPath);
		}
	}
	//禁用文件管理窗口
	EnableControl(TRUE);
	//刷新数据
	FixedServerFileData(".");
}
void CDlgFileManager::OnFileManagerServerFileNewFolder()
{
	if (m_ServerFileFullPath == "")
	{
		return;
	}

	CDlgNewFolder Dialog(this);
	if (Dialog.DoModal() == IDOK&&Dialog.m_CEdit_Dialog_New_Folder_Main.GetLength())
	{
		//创建多层目录
		CString v1;
		v1 = m_ServerFileFullPath + Dialog.m_CEdit_Dialog_New_Folder_Main + "\\";
		MakeSureDirectoryPathExists(v1.GetBuffer());
		FixedServerFileData(".");
	}
}
void CDlgFileManager::OnFileManagerServerFileStop()
{

}
void CDlgFileManager::OnFileManagerServerFileViewSmall()
{

}
void CDlgFileManager::OnFileManagerServerFileViewList()
{

}
void CDlgFileManager::OnFileManagerServerFileViewDetail()
{

}
void CDlgFileManager::OnFileManagerClientFilePrevious()
{
	GetClientFileData("..");
}
void CDlgFileManager::OnFileManagerClientFileDelete()
{

}
void CDlgFileManager::OnFileManagerClientFileNewFolder()
{

}
void CDlgFileManager::OnFileManagerClientFileStop()
{

}
void CDlgFileManager::OnFileManagerClientFileViewSmall()
{

}
void CDlgFileManager::OnFileManagerClientFileViewList()
{

}
void CDlgFileManager::OnFileManagerClientFileViewDetail()
{

}
void CDlgFileManager::EnableControl(BOOL IsEnable)
{
	m_CListCtrl_Dialog_File_Manager_Client_File.EnableWindow(IsEnable);
	m_CListCtrl_Dialog_File_Manager_Server_File.EnableWindow(IsEnable);
	m_CComboBox_Dialog_File_Manager_Client_File.EnableWindow(IsEnable);
	m_CComboBox_Dialog_File_Manager_Server_File.EnableWindow(IsEnable);
}
BOOL CDlgFileManager::DeleteDirectory(LPCTSTR DirectoryFullPath)
{
	WIN32_FIND_DATA v1;
	char BufferData[MAX_PATH] = { 0 };

	wsprintf(BufferData, "%s\\*.*", DirectoryFullPath);

	HANDLE FileHandle = FindFirstFile(BufferData, &v1);
	if (FileHandle == INVALID_HANDLE_VALUE)  //如果没有找到或者查找失败
	{
		return FALSE;
	}

	do 
	{
		if (v1.cFileName[0] == '.'&&strlen(v1.cFileName)<=2)
		{
			continue;
		}
		else
		{
			if (v1.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				char v2[MAX_PATH];
				wsprintf(v2, "%s\\%s", DirectoryFullPath, v1.cFileName);
				DeleteDirectory(v2);
			}
			else
			{
				char v2[MAX_PATH];
				wsprintf(v2, "%s\\%s", DirectoryFullPath, v1.cFileName);
				DeleteFile(v2);
			}
		}
	} while (FindNextFile(FileHandle, &v1));

	FindClose(FileHandle);   //关闭查找句柄

	if (!RemoveDirectory(DirectoryFullPath))
	{
		return FALSE;
	}
	return TRUE;

}
//深层目录
BOOL CDlgFileManager::MakeSureDirectoryPathExists(char* DirectoryFullPath)
{
	char* Travel = NULL;
	char* BufferData = NULL;
	DWORD DirectoryAttributes = 0;
	__try
	{
		BufferData = (char*)malloc(sizeof(char)*(strlen(DirectoryFullPath) + 1));
		if (BufferData == NULL)
		{
			return FALSE;
		}
		strcpy(BufferData, DirectoryFullPath);
		Travel = BufferData;

		if (*(Travel + 1) == ':')
		{
			Travel++;
			Travel++;
			if (*Travel && (*Travel == '\\'))
			{
				Travel++;
			}
		}
		//深层目录
		while (*Travel)
		{
			if (*Travel == '\\')
			{
				*Travel = '\0';
				//查看是否为目录 目录是否存在
				DirectoryAttributes = GetFileAttributes(BufferData);
				if (DirectoryAttributes == 0xffffffff)
				{
					if (!CreateDirectory(BufferData, NULL))
					{
						if (GetLastError() != ERROR_ALREADY_EXISTS)
						{
							free(BufferData);
							return FALSE;
						}

					}

				}
				else
				{
					if ((DirectoryAttributes&FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
					{
						free(BufferData);
						BufferData = NULL;
						return FALSE;
					}
				}
				*Travel = '\\';
			}
			Travel = CharNext(Travel);
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (BufferData != NULL)
		{
			free(BufferData);
			BufferData = NULL;
		}
		return FALSE;
	}
	if (BufferData != NULL)
	{
		free(BufferData);
		BufferData = NULL;
	}
	return TRUE;
}
//从服务端到客户端的文件传输 
void CDlgFileManager::OnLvnBegindragListDialogFileManagerServerFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	if (m_ServerFileFullPath.IsEmpty() || m_ClientFileFullPath.IsEmpty())
	{
		return;
	}
	if (m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectedCount() > 1)
	{
		//如果选择多项
		m_CursorHwnd = AfxGetApp()->LoadCursor(IDC_CURSOR_MULTI_DRAG);
	}
	else
	{
		//如果选择单项
		m_CursorHwnd = AfxGetApp()->LoadCursor(IDC_CURSOR_SINGLE_DRAG);
	}
	m_IsDragging = TRUE;
	m_DragListControl = &m_CListCtrl_Dialog_File_Manager_Server_File;
	m_DropListControl = &m_CListCtrl_Dialog_File_Manager_Client_File;

	SetCapture();

	*pResult = 0;
}
void CDlgFileManager::OnMouseMove(UINT nFlags, CPoint point)
{
	//判断是否拖拽文件
	if (m_IsDragging)
	{
		CPoint Point(point);  //获得鼠标位置
		ClientToScreen(&Point);  //转换光标坐标成为相对于屏幕的
		//根据鼠标获得窗口句柄
		CWnd* v1 = WindowFromPoint(Point);

		//属于我们的窗口范围内
		if (v1->IsKindOf(RUNTIME_CLASS(CListCtrl)))
		{
			//改变鼠标样式
			SetCursor(m_CursorHwnd);
			return;
		}
		else
		{
			SetCursor(LoadCursor(NULL, IDC_NO));   //超出窗口换为鼠标样式
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}
void CDlgFileManager::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_IsDragging)
	{
		ReleaseCapture();  //释放鼠标的捕获
		m_IsDragging = FALSE;

		CPoint Point(point);   //获得当前鼠标位置 相对于整个屏幕的
		ClientToScreen(&Point);  //转换成相对于当前用户的窗口的位置

		CWnd* v1 = WindowFromPoint(Point);   //判断当前鼠标下方有无控件

		if (v1->IsKindOf(RUNTIME_CLASS(CListCtrl)))  //如果是一个ListControl
		{
			m_DropListControl = (CListCtrl*)v1;  //保存当前窗口句柄
			DropDataOnList();   //处理传输
		}
	}

	CDialog::OnLButtonUp(nFlags, point);
}
//判断数据从哪里来 分别调用函数
VOID CDlgFileManager::DropDataOnList()
{
	//判断是从哪里拷贝数据
	if (m_DragListControl == m_DropListControl)
	{
		return;
	}
	if ((CWnd*)m_DropListControl == &m_CListCtrl_Dialog_File_Manager_Server_File)
	{
		//客户端向主控端
	}
	else if ((CWnd*)m_DropListControl == &m_CListCtrl_Dialog_File_Manager_Client_File)
	{
		//主控端向客户端
		CopyServerDataToClient();
	}
	else
	{
		return;
	}
}
//从主控端到客户端
VOID CDlgFileManager::CopyServerDataToClient()
{
	//定义一个模板
	m_ServerDataToClientJob.RemoveAll();
	POSITION Position = m_CListCtrl_Dialog_File_Manager_Server_File.GetFirstSelectedItemPosition();
	while (Position)
	{
		int Item = m_CListCtrl_Dialog_File_Manager_Server_File.GetNextSelectedItem(Position);
		CString FileFullPath = NULL;
		
		FileFullPath = m_ServerFileFullPath + m_CListCtrl_Dialog_File_Manager_Server_File.GetItemText(Item, 0);
		
		//如果是目录
		if (m_CListCtrl_Dialog_File_Manager_Server_File.GetItemData(Item))
		{
			FileFullPath += '\\';
			FixedServerDataToClient(FileFullPath.GetBuffer(0));
		}
		else
		{
			//打开文件判断是否合法
			HANDLE FileHandle = CreateFile(FileFullPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			if (FileHandle == INVALID_HANDLE_VALUE)
			{
				continue;
			}
			m_ServerDataToClientJob.AddTail(FileFullPath);

			CloseHandle(FileHandle);
		}

	}
	if (m_ServerDataToClientJob.IsEmpty())
	{
		//如果选择有空目录 不处理
		::MessageBox(m_hWnd, "文件夹为空", "警告", MB_OK | MB_ICONWARNING);
		return;
	}
	EnableControl(TRUE);
	SendServerDataToClient();   //发送第一个任务

}
//遍历目录中的数据  笔记
BOOL CDlgFileManager::FixedServerDataToClient(LPCTSTR DirectoryFullPath)
{
	CHAR BufferData[MAX_PATH];
	CHAR *Slash = NULL;
	memset(BufferData, 0, sizeof(BufferData));

	if (DirectoryFullPath[strlen(DirectoryFullPath) - 1] != '\\')
	{
		Slash = "\\";
	}
	else
	{
		Slash = "";
	}
	sprintf(BufferData, "%s%s*.*", DirectoryFullPath, Slash);

	WIN32_FIND_DATA WinFindData;
	HANDLE FileHandle = FindFirstFile(BufferData, &WinFindData);
	if (FileHandle == INVALID_HANDLE_VALUE)  //若没有查找到或查找失败
	{
		return FALSE;
	}
	do
	{
		//过滤这两个目录 '.'和'..'
		if (WinFindData.cFileName[0] == '.')
		{
			continue;
		}
		if (WinFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			CHAR v1[MAX_PATH];
			sprintf(v1, "%s%s%s", DirectoryFullPath, Slash, WinFindData.cFileName);
			FixedServerDataToClient(v1);  //如果是目录，则进入此目录进行递归
		}
		else
		{
			CString FileFullPath;
			FileFullPath.Format("%s%s%s", DirectoryFullPath, Slash, WinFindData.cFileName);
			m_ServerDataToClientJob.AddTail(FileFullPath);
			//对文件进行操作
		}
	} while (FindNextFile(FileHandle, &WinFindData));
	FindClose(FileHandle);  //关闭查找句柄
	return true;
}
//发送数据到被控端
BOOL CDlgFileManager::SendServerDataToClient()
{
	if (m_ServerDataToClientJob.IsEmpty())
	{
		return FALSE;
	}

	CString DestinationDirectory = m_ClientFileFullPath;  //确认目标路径
	m_SourFileFullPath = m_ServerDataToClientJob.GetHead();  //获得第一个任务的名称

	DWORD FileSizeHigh = 0;
	DWORD FileSizeLow = 0;
	HANDLE FileHandle;
	CString v1 = m_SourFileFullPath;  //远程文件

	//得到要保存的远程文件路径
	v1.Replace(m_ServerFileFullPath, m_ClientFileFullPath);
	m_DestFileFullPath = v1;  //修正好的名字

	//打开本地文件
	FileHandle = CreateFile(m_SourFileFullPath.GetBuffer(0),
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	//获取文件大小
	FileSizeLow = GetFileSize(FileHandle, &FileSizeHigh);
	//构建数据包 发送文件长度
	m_TransferFileLength = (FileSizeHigh * (MAXDWORD + 1)) + FileSizeLow;
	CloseHandle(FileHandle);

	ULONG BufferLength = v1.GetLength() + 10;
	BYTE* BufferData = (BYTE*)LocalAlloc(LPTR, BufferLength);
	memset(BufferData, 0, BufferLength);

	BufferData[0] = CLIENT_FILE_MANAGER_SEND_FILE_INFORMATION;

	memcpy(BufferData + 1, &FileSizeHigh, sizeof(DWORD));     
	memcpy(BufferData + 5, &FileSizeLow, sizeof(DWORD));

	memcpy(BufferData + 9, v1.GetBuffer(0), v1.GetLength() + 1);

	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);

	LocalFree(BufferData);
	//从下载列表中删除自己
	m_ServerDataToClientJob.RemoveHead();
	return TRUE;
}
VOID CDlgFileManager::EndCopyServerFileToClient()
{
	m_Counter = 0;
	m_TransferFileLength = 0;

	//进度条
	ShowProgress();
	if (m_ServerDataToClientJob.IsEmpty() || m_IsStop)
	{
		m_ServerDataToClientJob.RemoveAll();
		m_IsStop = FALSE;
		EnableControl(TRUE);     //用户可以点击控件
		m_TransferMode = TRANSFER_MODE_NORMAL;
		GetClientFileData(".");  //刷新列表
	}
	else
	{
		Sleep(5);
		SendServerDataToClient();
	}
	return;
}
void CDlgFileManager::ShowProgress()
{
	if ((int)m_Counter == -1)
	{
		m_Counter = m_TransferFileLength;
	}

	int Progress = (float)(m_Counter * 100) / m_TransferFileLength;
	m_ProgressCtrl->SetPos(Progress);

	if (m_Counter == m_TransferFileLength)
	{
		m_Counter = m_TransferFileLength = 0;
	}
}
VOID CDlgFileManager::SendServerFileDataToClient()
{
	FILE_SIZE* v1 = (FILE_SIZE*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));
	LONG FileSizeHigh = v1->FileSizeHigh;
	LONG FileSizeLow  = v1->FileSizeLow;

	m_Counter = MAKEINT64(v1->FileSizeLow, v1->FileSizeHigh);

	ShowProgress();  //显示进度条

	if (m_Counter == m_TransferFileLength || m_IsStop || v1->FileSizeLow == -1)
	{
		//v1->FileSizeLow == -1 是对方选择了跳过   m_Counter == m_TransferFileLength 完成了当前传输
		EndCopyServerFileToClient();  //进行下一个传送 若存在的话
		return;
	}
	HANDLE FileHandle;
	FileHandle = CreateFile(m_SourFileFullPath.GetBuffer(0), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}
	SetFilePointer(FileHandle, FileSizeLow, &FileSizeHigh, FILE_BEGIN);  //设置文件的大小

	int v3 = 9;  //数据包头部大小

	DWORD NumberofBytesToRead = MAX_SEND_BUFFER - v3;
	DWORD NumberofBytesRead = 0;
	BYTE* BufferData = (BYTE*)LocalAlloc(LPTR, MAX_SEND_BUFFER);

	if (BufferData == NULL)
	{
		CloseHandle(FileHandle);
		return;
	}
	BufferData[0] = CLIENT_FILE_MANAGER_FILE_DATA;
	memcpy(BufferData + 1, &FileSizeHigh, sizeof(FileSizeHigh));
	memcpy(BufferData + 5, &FileSizeLow, sizeof(FileSizeLow));
	//从文件中读取数据
	ReadFile(FileHandle, BufferData + v3, NumberofBytesToRead, &NumberofBytesRead, NULL);
	CloseHandle(FileHandle);

	if (NumberofBytesRead > 0)
	{
		ULONG BufferLength = NumberofBytesRead + v3;
		m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);
	}
	LocalFree(BufferData);
	
}