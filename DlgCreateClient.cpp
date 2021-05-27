// DlgCreateClient.cpp : 实现文件
//

#include "stdafx.h"
#include "XsServer.h"
#include "DlgCreateClient.h"
#include "afxdialogex.h"


struct _CONNECT_INFORMATION_
{
	DWORD    CheckFlag;
	char     ConnectIP[20];    //服务器IP地址
	USHORT   ConnectPort;      //连接端口
}__ConnectInfo = { 0x87654321,"",0  };

// CDlgCreateClient 对话框

IMPLEMENT_DYNAMIC(CDlgCreateClient, CDialog)

CDlgCreateClient::CDlgCreateClient(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_CREATE_CLIENT, pParent)
	, m_CEdit_Dialog_Create_Client_Connect_IP(_T("请输入一个连接的IP"))
	, m_CEdit_Dialog_Create_Client_Connect_Port(_T("请输入一个连接的Port"))
{

}

CDlgCreateClient::~CDlgCreateClient()
{
}

void CDlgCreateClient::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DIALOG_CREATE_CLIENT_CONNECT_IP, m_CEdit_Dialog_Create_Client_Connect_IP);
	DDX_Text(pDX, IDC_EDIT_DIALOG_CREATE_CLIENT_CONNECT_PORT, m_CEdit_Dialog_Create_Client_Connect_Port);
}


BEGIN_MESSAGE_MAP(CDlgCreateClient, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgCreateClient::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgCreateClient 消息处理程序


void CDlgCreateClient::OnBnClickedOk()
{
	CFile v1;

	UpdateData(TRUE);
	USHORT ConnectPort = atoi(m_CEdit_Dialog_Create_Client_Connect_Port);
	strcpy(__ConnectInfo.ConnectIP, m_CEdit_Dialog_Create_Client_Connect_IP);

	if (ConnectPort < 0 || ConnectPort>65536)
	{
		__ConnectInfo.ConnectPort = 2356;
	}
	else
	{
		__ConnectInfo.ConnectPort = ConnectPort;
	}

	char v2[MAX_PATH];
	ZeroMemory(v2, MAX_PATH);

	LONGLONG BufferLength = 0;
	BYTE* BufferData = NULL;
	CString v3;
	CString FileFullPath;
	try
	{
		//此处得到未处理前文件名
		GetModuleFileName(NULL, v2, MAX_PATH);
		v3 = v2;
		int Position = v3.ReverseFind('\\');   //从右至左查找第一次出现的\\

		v3 = v3.Left(Position);

		FileFullPath = v3 + "\\XsClient.exe";    //得到当前未处理文件名
		//打开文件
		v1.Open(FileFullPath, CFile::modeRead | CFile::typeBinary);

		BufferLength = v1.GetLength();
		BufferData = new BYTE[BufferLength];
		ZeroMemory(BufferData, BufferLength);

		//读取文件内容
		v1.Read(BufferData, BufferLength);
		//关闭文件
		v1.Close();


		//在内存中搜索0x87654321
		int Offset = SeMemoryFind((char*)BufferData, (char*)&__ConnectInfo.CheckFlag,
			BufferLength, sizeof(DWORD));
		//把自己设置的信息拷贝到exe的内存中
		memcpy(BufferData + Offset, &__ConnectInfo, sizeof(__ConnectInfo));
		//保存到文件中
		v1.Open(FileFullPath, CFile::typeBinary | CFile::modeCreate | CFile::modeWrite);
		//更新文件中的数据
		v1.Write(BufferData, BufferLength);
		v1.Close();
		delete[] BufferData;
		MessageBox("生成成功");
	}
	catch (CMemoryException* e)
	{
		MessageBox("内存不足");
	}
	catch (CFileException* e)
	{
		MessageBox("文件操作错误");
	}
	catch (CException* e)
	{
		MessageBox("未知错误");
	}
	CDialog::OnOK();
}
int CDlgCreateClient::SeMemoryFind(const char* BufferData, const char* KeyValue, int BufferLength, int KeyLength)
{
	int i, j;
	if (KeyLength == 0 || BufferLength == 0)
	{
		return -1;
	}
	for (i = 0; i < BufferLength; i++)
	{
		for (j = 0; j < KeyLength; j++)
		{
			if (BufferData[i + j] != KeyValue[j])
			{
				break;
			}
		}
		if (j == KeyLength)
		{
			return i;
		}
	}
	return -1;
}