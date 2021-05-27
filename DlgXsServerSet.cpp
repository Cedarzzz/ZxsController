// DlgXsServerSet.cpp : 实现文件
//

#include "stdafx.h"
#include "XsServer.h"
#include "DlgXsServerSet.h"
#include "afxdialogex.h"


// CDlgXsServerSet 对话框
extern _CConfigFile __ConfigFile;

IMPLEMENT_DYNAMIC(CDlgXsServerSet, CDialog)

CDlgXsServerSet::CDlgXsServerSet(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_SERVER_SET, pParent)
	, m_CEdit_Dialog_Server_Set_Max_Connection(0)
	, m_CEdit_Dialog_Server_Set_Listen_Port(0)
{

}

CDlgXsServerSet::~CDlgXsServerSet()
{
}

void CDlgXsServerSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DIALOG_SERVER_SET_MAX_CONNECTION, m_CEdit_Dialog_Server_Set_Max_Connection);
	DDX_Text(pDX, IDC_EDIT_DIALOG_SERVER_SET_LISTEN_PORT, m_CEdit_Dialog_Server_Set_Listen_Port);
	DDV_MinMaxLong(pDX, m_CEdit_Dialog_Server_Set_Max_Connection, 1, 10);
	DDV_MinMaxLong(pDX, m_CEdit_Dialog_Server_Set_Listen_Port, 2048, 65536);
	DDX_Control(pDX, IDC_EDIT_DIALOG_SERVER_SET_APPLY, m_CButton_Dialog_Server_Set_Apply);
}


BEGIN_MESSAGE_MAP(CDlgXsServerSet, CDialog)
	ON_BN_CLICKED(IDC_EDIT_DIALOG_SERVER_SET_APPLY, &CDlgXsServerSet::OnBnClickedEditDialogServerSetApply)
	ON_EN_CHANGE(IDC_EDIT_DIALOG_SERVER_SET_MAX_CONNECTION, &CDlgXsServerSet::OnEnChangeEditDialogServerSetMaxConnection)
	ON_EN_CHANGE(IDC_EDIT_DIALOG_SERVER_SET_LISTEN_PORT, &CDlgXsServerSet::OnEnChangeEditDialogServerSetListenPort)
END_MESSAGE_MAP()


// CDlgXsServerSet 消息处理程序


void CDlgXsServerSet::OnBnClickedEditDialogServerSetApply()
{
	UpdateData(TRUE);   //更新修改的数据

	//向ini文件写入值
	__ConfigFile.SetInt("Settings", "ListenPort", m_CEdit_Dialog_Server_Set_Listen_Port);
	__ConfigFile.SetInt("Settings", "MaxConnection", m_CEdit_Dialog_Server_Set_Max_Connection);

	SendMessage(WM_CLOSE);
}
void CDlgXsServerSet::OnEnChangeEditDialogServerSetMaxConnection()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	m_CButton_Dialog_Server_Set_Apply.ShowWindow(SW_NORMAL);
	m_CButton_Dialog_Server_Set_Apply.EnableWindow(TRUE);
}
void CDlgXsServerSet::OnEnChangeEditDialogServerSetListenPort()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	m_CButton_Dialog_Server_Set_Apply.ShowWindow(SW_NORMAL);
	m_CButton_Dialog_Server_Set_Apply.EnableWindow(TRUE);
}


BOOL CDlgXsServerSet::OnInitDialog()
{
	CDialog::OnInitDialog();

	int ListenPort = 0;
	int MaxConnection = 0;

	//将数据设置到Dlg
	ListenPort = __ConfigFile.GetInt("Settings", "ListenPort");
	//读取ini文件中的监听端口
	MaxConnection = __ConfigFile.GetInt("Settings", "MaxConnection");

	m_CEdit_Dialog_Server_Set_Listen_Port = ListenPort;
	m_CEdit_Dialog_Server_Set_Max_Connection = MaxConnection;

	//由于控件是value所以要UpdateData    False           v1      控件(内存)
	//             控制                  CListControl    v1      操作v1就是操作控件
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
