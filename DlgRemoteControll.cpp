// DlgRemoteControll.cpp : 实现文件
//

#include "stdafx.h"
#include "XsServer.h"
#include "DlgRemoteControll.h"
#include "afxdialogex.h"

enum
{
	ID_MENU_DIALOG_REMOTE_CONTROLL_CONTROL = 0x1010,
    ID_MENU_DIALOG_REMOTE_CONTROLL_SEND_CTRL_ALT_DEL,
	ID_MENU_DIALOG_REMOTE_CONTROLL_TRACE_CURSOR,   //跟踪显示远程鼠标
	ID_MENU_DIALOG_REMOTE_CONTROLL_BLOCK_INPUT,    //锁定远程计算机输入
	ID_MENU_DIALOG_REMOTE_CONTROLL_SAVE_DIB,       //保存图片
	ID_MENU_DIALOG_REMOTE_CONTROLL_GET_CLIPBOARD,  //获取剪贴板
	ID_MENU_DIALOG_REMOTE_CONTROLL_SET_CLIPBOARD,  //设置剪贴板

};



extern "C" VOID CopyScreenData(PVOID SourceData, PVOID DestinationData, ULONG BufferLength);
// CDlgRemoteControll 对话框

IMPLEMENT_DYNAMIC(CDlgRemoteControll, CDialog)

CDlgRemoteControll::CDlgRemoteControll(CWnd* pParent,
	_CIOCPServer* IOCPServer, PCONTEXT_OBJECT ContextObject)
	: CDialog(IDD_DIALOG_REMOTE_CONTROLL, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	
	m_HorizontalScrollPosition = 0;
	m_VerticalScrollPosition = 0;

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	if (m_ContextObject == NULL)
	{
		return;
	}
	ULONG BufferLength = m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1;
	m_BitmapInfo = (BITMAPINFO*) new BYTE[BufferLength];
	if (m_BitmapInfo == NULL)
	{
		return;
	}
	//拷贝位图信息
	memcpy(m_BitmapInfo, m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1), BufferLength);

}

CDlgRemoteControll::~CDlgRemoteControll()
{
	if (m_BitmapInfo != NULL)
	{
		delete m_BitmapInfo;
		m_BitmapInfo = NULL;
	}
	
	::ReleaseDC(m_hWnd, m_WindowDCHandle);
	
	//回收工具箱
	if (m_WindowMemoryDCHandle != NULL)
	{
		DeleteDC(m_WindowMemoryDCHandle);
		//销毁工具
		DeleteObject(m_BitmapHandle);
		if (m_BitmapData != NULL)
		{
			m_BitmapData = NULL;
		}
		m_WindowMemoryDCHandle = NULL;
	}

}

void CDlgRemoteControll::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgRemoteControll, CDialog)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// CDlgRemoteControll 消息处理程序


BOOL CDlgRemoteControll::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_IconHwnd, FALSE);
	//获得客户端IP地址
	CString v1;
	sockaddr_in ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);
	v1.Format("\\\\%s - 远程控制", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	m_WindowDCHandle = ::GetDC(m_hWnd);
	m_WindowMemoryDCHandle = CreateCompatibleDC(m_WindowDCHandle);

	m_BitmapHandle = CreateDIBSection(m_WindowDCHandle, m_BitmapInfo,
		DIB_RGB_COLORS, &m_BitmapData, NULL, NULL);
	//初始化工具箱
	SelectObject(m_WindowMemoryDCHandle, m_BitmapHandle);

	SetWindowPos(NULL, 0, 0, m_BitmapInfo->bmiHeader.biWidth/2, m_BitmapInfo->bmiHeader.biHeight/2, 0);

	SetScrollRange(SB_HORZ, 0, m_BitmapInfo->bmiHeader.biWidth);  //指定滚动条范围的最大最小值
	SetScrollRange(SB_VERT, 0, m_BitmapInfo->bmiHeader.biHeight);

	//获得系统菜单
	CMenu* Menu = GetSystemMenu(FALSE);
	if (Menu != NULL)
	{
		Menu->AppendMenu(MF_SEPARATOR);
		Menu->AppendMenu(MF_STRING, ID_MENU_DIALOG_REMOTE_CONTROLL_CONTROL, "控制屏幕(&Y)");
		Menu->AppendMenu(MF_STRING, ID_MENU_DIALOG_REMOTE_CONTROLL_TRACE_CURSOR, "跟踪被控端鼠标(&T)");
		Menu->AppendMenu(MF_STRING, ID_MENU_DIALOG_REMOTE_CONTROLL_BLOCK_INPUT, "锁定被控端鼠标和键盘(&L)");
		Menu->AppendMenu(MF_STRING, ID_MENU_DIALOG_REMOTE_CONTROLL_SAVE_DIB, "保存快照(&S)");
		Menu->AppendMenu(MF_SEPARATOR);
		Menu->AppendMenu(MF_STRING, ID_MENU_DIALOG_REMOTE_CONTROLL_GET_CLIPBOARD, "获取剪贴板(&R)");
		Menu->AppendMenu(MF_STRING, ID_MENU_DIALOG_REMOTE_CONTROLL_SET_CLIPBOARD, "更改剪贴板(&L)");
		Menu->AppendMenu(MF_SEPARATOR);
	}

	//客户端中的鼠标位置
	m_CursorPosition.x = 0;
	m_CursorPosition.y = 0;

	//回传数据
	BYTE IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
void CDlgRemoteControll::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray()[0])
	{
	case CLIENT_REMOTE_CONTROLL_FIRST_SCREEN:
	{
		DrawFirstScreen();  //显示第一帧数据
		break;
	}
	case CLIENT_REMOTE_CONTROLL_NEXT_SCREEN:
	{
#define ALOGORITHM_DIFF 1

		if (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[1] == ALOGORITHM_DIFF)
		{
			DrawNextScreen();
		}
		break;
	}
	case CLIENT_REMOTE_CONTROLL_GET_CLIPBOARD_REPLY:
	{
		//将接收到的客户端数据放入到主控端的剪切板中
		UpdataClipBoardData((char*)m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1),
			m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1);
		break;
	}
	
	}
}
void CDlgRemoteControll::OnClose()
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
VOID CDlgRemoteControll::DrawFirstScreen(void)
{
	//得到被控端发来的数据 并将数据拷贝到工具箱中的工具中
	memcpy(m_BitmapData, m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1), m_BitmapInfo->bmiHeader.biSizeImage);
	PostMessage(WM_PAINT);
}
VOID CDlgRemoteControll::DrawNextScreen(void)
{
	BOOL IsChanged = FALSE;
	ULONG v1 = 1 + 1 + sizeof(POINT) + sizeof(BYTE);

	//前一帧数据
	LPVOID PreviousScreenData = m_BitmapData;
	//获得当前帧长度
	LPVOID CurrentScreenData = m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(v1);

	ULONG CurrentScreenLength = m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - v1;

	POINT PreviousCursorPosition;

	memcpy(&PreviousCursorPosition, &m_CursorPosition, sizeof(POINT));
	
	//更新光标位置
	memcpy(&m_CursorPosition, m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(2), sizeof(POINT));

	//鼠标移动了
	if (memcmp(&PreviousCursorPosition, &m_CursorPosition, sizeof(POINT)) != 0)
	{
		IsChanged = TRUE;
	}
	//屏幕是否变化
	if (CurrentScreenLength > 0)
	{
		IsChanged = TRUE;
	}

	CopyScreenData(PreviousScreenData, CurrentScreenData, CurrentScreenLength);

	if (IsChanged)
	{
		//如果有变化响应PAINT消息
		PostMessage(WM_PAINT);
	}

}
void CDlgRemoteControll::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialog::OnPaint()

	//将工具箱中的工具给工人
	BitBlt(m_WindowDCHandle, 0, 0,
		m_BitmapInfo->bmiHeader.biWidth,
		m_BitmapInfo->bmiHeader.biHeight,
		m_WindowMemoryDCHandle,
		m_HorizontalScrollPosition,
		m_VerticalScrollPosition,
		SRCCOPY);

	if (m_IsTraceCursor)
	{
		DrawIconEx(m_WindowDCHandle,
			m_CursorPosition.x - m_HorizontalScrollPosition,
			m_CursorPosition.y - m_VerticalScrollPosition,
			m_IconHwnd,
			0, 0,
			0,
			NULL,
			DI_NORMAL | DI_COMPAT
			);
	}

}


void CDlgRemoteControll::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO ScrollInfo;
	int i;
	ScrollInfo.cbSize = sizeof(SCROLLINFO);
	ScrollInfo.fMask = SIF_ALL;
	GetScrollInfo(SB_VERT, &ScrollInfo);

	switch (nSBCode)
	{
	case SB_LINEUP:
	{
		i = nPos - 1;
		break;
	}
	case SB_LINEDOWN:
	{
		i = nPos + 1;
		break;
	}
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
	{
		i = ScrollInfo.nTrackPos;
		break;
	}
	default:
		return;
	}

	i = max(i, ScrollInfo.nMin);
	i = min(i, (int)(ScrollInfo.nMax - ScrollInfo.nPage + 1));

	RECT Rect;
	GetClientRect(&Rect);

	if ((Rect.bottom + i) > m_BitmapInfo->bmiHeader.biHeight)
	{
		i = m_BitmapInfo->bmiHeader.biHeight - Rect.bottom;
	}

	InterlockedExchange((PLONG)&m_VerticalScrollPosition, i);

	SetScrollPos(SB_VERT, i);
	OnPaint();

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CDlgRemoteControll::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO ScrollInfo;
	int i;
	ScrollInfo.cbSize = sizeof(SCROLLINFO);
	ScrollInfo.fMask = SIF_ALL;
	GetScrollInfo(SB_HORZ, &ScrollInfo);

	switch (nSBCode)
	{
	case SB_LINEUP:
	{
		i = nPos - 1;
		break;
	}
	case SB_LINEDOWN:
	{
		i = nPos + 1;
		break;
	}
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
	{
		i = ScrollInfo.nTrackPos;
		break;
	}
	default:
		return;
	}

	i = max(i, ScrollInfo.nMin);
	i = min(i, (int)(ScrollInfo.nMax - ScrollInfo.nPage + 1));

	RECT Rect;
	GetClientRect(&Rect);

	if ((Rect.right + i) > m_BitmapInfo->bmiHeader.biWidth)
	{
		i = m_BitmapInfo->bmiHeader.biWidth - Rect.right;
	}

	InterlockedExchange((PLONG)&m_HorizontalScrollPosition, i);

	SetScrollPos(SB_HORZ, i);
	OnPaint();


	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
//该消息接送包括系统菜单等待窗口处理消息
void CDlgRemoteControll::OnSysCommand(UINT nID, LPARAM lParam)
{
	CMenu* Menu = GetSystemMenu(FALSE);
	switch (nID)
	{
	case ID_MENU_DIALOG_REMOTE_CONTROLL_CONTROL:  //远程控制
	{
		m_IsControl = !m_IsControl;
		//设置菜单样式
		Menu->CheckMenuItem(ID_MENU_DIALOG_REMOTE_CONTROLL_CONTROL, m_IsControl ? MF_CHECKED : MF_UNCHECKED);
		break;
	}
	case ID_MENU_DIALOG_REMOTE_CONTROLL_TRACE_CURSOR:  //跟踪被控端鼠标
	{
		m_IsTraceCursor = !m_IsTraceCursor;
		Menu->CheckMenuItem(ID_MENU_DIALOG_REMOTE_CONTROLL_TRACE_CURSOR, m_IsTraceCursor ? MF_CHECKED : MF_UNCHECKED);

		break;
	}
	case ID_MENU_DIALOG_REMOTE_CONTROLL_BLOCK_INPUT:   //锁定客户端鼠标和键盘
	{
		BOOL IsChecked = Menu->GetMenuState(ID_MENU_DIALOG_REMOTE_CONTROLL_BLOCK_INPUT, MF_BYCOMMAND) & MF_CHECKED;
		Menu->CheckMenuItem(ID_MENU_DIALOG_REMOTE_CONTROLL_BLOCK_INPUT, IsChecked ? MF_UNCHECKED : MF_CHECKED);

		BYTE IsToken[2];
		IsToken[0] = CLIENT_REMOTE_CONTROLL_BLOCK_INPUT_REQUEST;
		IsToken[1] = !IsChecked;
		m_IOCPServer->OnPrepareSending(m_ContextObject, IsToken, sizeof(IsToken));

		break;
	}
	case ID_MENU_DIALOG_REMOTE_CONTROLL_SAVE_DIB:  //保存截图
	{
		SaveSnapShotData();
		break;
	}
	case ID_MENU_DIALOG_REMOTE_CONTROLL_GET_CLIPBOARD:  //获取客户端剪贴板内容
	{
		BYTE IsToken = CLIENT_REMOTE_CONTROLL_GET_CLIPBOARD_REQUEST;
		m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(IsToken));
		break;
	}
	case ID_MENU_DIALOG_REMOTE_CONTROLL_SET_CLIPBOARD:  //设置客户端剪贴板内容
	{
		//从当前服务器中的剪贴板中获取数据后发送到客户端
		SendClipboardData();
		break;
	}
	}

	CDialog::OnSysCommand(nID, lParam);
}
BOOL CDlgRemoteControll::PreTranslateMessage(MSG* pMsg)
{
	//过滤键盘鼠标消息
#define MAKEDWORD(h,l)   (((unsigned long)h<<16) | 1)
	
	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	{
		MSG Msg;
		memcpy(&Msg, pMsg, sizeof(MSG));
		Msg.lParam = MAKEDWORD(HIWORD(pMsg->lParam) + m_VerticalScrollPosition, LOWORD(pMsg->lParam) + m_HorizontalScrollPosition);
		Msg.pt.x += m_HorizontalScrollPosition;
		Msg.pt.y += m_VerticalScrollPosition;
		OnPrepareSending(&Msg);
		break;
	}
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	{
		if (pMsg->wParam != VK_LWIN&&pMsg->wParam != VK_RWIN)
		{
			MSG Msg;
		    memcpy(&Msg, pMsg, sizeof(MSG));
		    Msg.lParam = MAKEDWORD(HIWORD(pMsg->lParam) + m_VerticalScrollPosition, LOWORD(pMsg->lParam) + m_HorizontalScrollPosition);
		    Msg.pt.x += m_HorizontalScrollPosition;
		    Msg.pt.y += m_VerticalScrollPosition;
		    OnPrepareSending(&Msg);
		}
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return true;

		break;
	}
	}

	return CDialog::PreTranslateMessage(pMsg);
}
VOID CDlgRemoteControll::OnPrepareSending(MSG* Msg)
{
	if (!m_IsControl)
	{
		return;
	}
	LPBYTE BufferData = new BYTE[sizeof(MSG) + 1];
	BufferData[0] = CLIENT_REMOTE_CONTROLL_CONTROL_REQUEST;
	memcpy(BufferData + 1, Msg, sizeof(MSG));
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, sizeof(MSG) + 1);
	delete[] BufferData;
}
BOOL CDlgRemoteControll::SaveSnapShotData(void)
{
	//位图保存
	CString FileFullPath = CTime::GetCurrentTime().Format("%Y-%m-%d_%H-%M-%S.bmp");
	CFileDialog Dialog(FALSE, "bmp", FileFullPath, OFN_OVERWRITEPROMPT, "位图文件(*.bmp)|*.bmp|", this);
	if (Dialog.DoModal() != IDOK)
		return FALSE;

	BITMAPFILEHEADER BitmapFileHeader;
	LPBITMAPINFO BitmapInfo = m_BitmapInfo;
	CFile FileHandle;
	//生成一个文件
	if (!FileHandle.Open(Dialog.GetPathName(), CFile::modeWrite| CFile::modeCreate))
	{
		return FALSE;
	}
	//BITMAPINFO大小
	int BitmapInfoLength = sizeof(BITMAPINFO);
	//协议  TCP 校验值
	BitmapFileHeader.bfType      = ((WORD)('M' << 8) | 'B');
	BitmapFileHeader.bfSize      = BitmapInfo->bmiHeader.biSizeImage + sizeof(BITMAPFILEHEADER);
	BitmapFileHeader.bfReserved1 = 0;
	BitmapFileHeader.bfReserved2 = 0;
	BitmapFileHeader.bfOffBits   = sizeof(BITMAPFILEHEADER) + BitmapInfoLength;

	FileHandle.Write(&BitmapFileHeader, sizeof(BITMAPFILEHEADER));
	FileHandle.Write(BitmapInfo, BitmapInfoLength);

	FileHandle.Write(m_BitmapData, BitmapInfo->bmiHeader.biSizeImage);
	FileHandle.Close();
	return true;
}
VOID CDlgRemoteControll::UpdataClipBoardData(char* BufferData, ULONG BufferLength)
{
	if (!::OpenClipboard(NULL))
	{
		return;
	}
	//清空剪贴板
	::EmptyClipboard();
	//申请内存
	HGLOBAL GlobalHandle = GlobalAlloc(GPTR, BufferLength);
	if (GlobalHandle != NULL)
	{
		char* v5 = (LPTSTR)GlobalLock(GlobalHandle);  //锁定物理页面
		memcpy(v5, BufferData, BufferLength);
		GlobalUnlock(GlobalHandle);
		SetClipboardData(CF_TEXT, GlobalHandle);
		GlobalFree(GlobalHandle);
	}
	CloseClipboard();
}
VOID CDlgRemoteControll::SendClipboardData(void)
{
	//打开剪贴板设备
	if (!::OpenClipboard(NULL))
	{
		return;
	}
	HGLOBAL GlobalHandle = GetClipboardData(CF_TEXT);  //代表着一个内存
	if (GlobalHandle == NULL)
	{
		CloseClipboard();
		return;
	}
	//通过剪贴板句柄锁定数据内存
	int BufferLength = GlobalSize(GlobalHandle) + 1;
	char* v5 = (LPSTR)GlobalLock(GlobalHandle);
	LPBYTE BufferData = new BYTE[BufferLength];
	
	BufferData[0] = CLIENT_REMOTE_CONTROLL_SET_CLIPBOARD_REPLY;
	memcpy(BufferData + 1, v5, BufferLength - 1);
	::GlobalUnlock(GlobalHandle);
	::CloseClipboard();
	m_IOCPServer->OnPrepareSending(m_ContextObject, (PBYTE)BufferData, BufferLength);
	delete[] BufferData;
}