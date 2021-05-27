#pragma once
#include <windows.h>
#include <winsock2.h>
#include <mstcpip.h>
#include <iostream>
#include "_CriticalSection.h"
#include "_CArray.h"
#include "zconf.h"
#include "zlib.h"
#include "Common.h"

using namespace std;
#pragma comment(lib,"WS2_32.lib")

#define PACKET_FLAG_LENGTH 5
#define PACKET_LENGTH 0x2000      //Alessio
#define PACKET_HEADER_LENGTH 13   //Alessio[数据包总长(4)][原始数据总长(4)]

#define MAX_SEND_BUFFER 0x2000
#define MAX_RECEIVE_BUFFER 0x2000

typedef struct _CONTEXT_OBJECT_
{
	SOCKET ClientSocket;          //Server使用该套接字与目标Client进行通信
	WSABUF ReceiveWsaBuffer;      //接收用户到达的数据 该结构没有内存 与m_BufferData关联
	WSABUF SendWsaBuffer;         //向客户发送数据  
	_CArray m_ReceivedCompressedBufferData;     //接收到的压缩的数据m_BufferData拷贝到该成员中
	_CArray m_ReceivedDecompressedBufferData;   //接收到的压缩的数据进行的解压后数据
	_CArray m_SendCompressedBufferData;
	char BufferData[PACKET_LENGTH];       //用户到达数据的内存  在OnAccept函数中与ReceiveWsaBuffer关联
	int DialogID;
	HANDLE DialogHandle;
	VOID InitMember()
	{
		ClientSocket = INVALID_SOCKET;
		memset(BufferData, 0, sizeof(char)*PACKET_LENGTH);
		memset(&ReceiveWsaBuffer, 0, sizeof(WSABUF));
		memset(&SendWsaBuffer, 0, sizeof(WSABUF));
		DialogID = 0;
		DialogHandle = NULL;
	}
	
}CONTEXT_OBJECT,*PCONTEXT_OBJECT;

enum PACKET_TYPE
{
	IO_INITIALIZE,  //初始化
	IO_RECEIVE,     //接收	
	IO_SEND,        //发送
	IO_IDLE         //空闲
};
class OVERLAPPEDEX
{
public:
	OVERLAPPED m_Overlapped;
	PACKET_TYPE m_PackType;

	OVERLAPPEDEX(PACKET_TYPE PackType)
	{
		ZeroMemory(this, sizeof(OVERLAPPEDEX));
		m_PackType = PackType;
	}
};

typedef void(CALLBACK *LPFN_WINDOWNOTIFYPROCEDURE)(PCONTEXT_OBJECT ContextObject);
//定义一个数据类型
typedef CList<PCONTEXT_OBJECT> CONTEXT_OBJECT_LIST;

class _CIOCPServer
{
public:
	_CIOCPServer();
	~_CIOCPServer();
private:
	SOCKET m_ListenSocket;                //监听套接字
	HANDLE m_ListenThreadHandle;          //监听线程句柄
	HANDLE m_ListenEventHandle;           //监听事件

	CRITICAL_SECTION m_CriticalSection;   //关键代码段
	HANDLE m_CompletionPortHandle;        //完成端口句柄
	HANDLE m_KillEventHandle;             //监听线程退出的循环
	BOOL   m_IsWorking;                   //工作线程的循环标志
	CONTEXT_OBJECT_LIST m_FreeContextObjectList;         //内存池模板
	CONTEXT_OBJECT_LIST m_ConnectionContextObjectList;   //上线用户上下背景文列表
	long  m_KeepAliveTime;                               //保活机制

	ULONG m_ThreadPoolMin;
	ULONG m_ThreadPoolMax;
	ULONG m_WorkThreadCount;
	ULONG m_CurrentThreadCount;
	ULONG m_BusyThreadCount;
	char m_PacketHeaderFlag[PACKET_FLAG_LENGTH];     //数据包匹配
	LPFN_WINDOWNOTIFYPROCEDURE m_WindowNotifyProcedure;  //保存由窗口类中传递过来的函数指针

public:
	BOOL ServerRun(USHORT ListenPort, LPFN_WINDOWNOTIFYPROCEDURE WindowNotifyProcedure);    //启动监听
	static DWORD WINAPI ListenThreadProcedure(LPVOID ParameterData);  //监听线程处理
	void _CIOCPServer::OnAccept();        //监听到后通信套接字匹配
	PCONTEXT_OBJECT _CIOCPServer::AllocateContextObject();
	VOID _CIOCPServer::RemoveContextObject(CONTEXT_OBJECT* ContextObject);   //移除上下背景文数据结构
	VOID _CIOCPServer::MoveContextObjectToFreePool(CONTEXT_OBJECT* ContextObject);  //将该内存结构回收至内存池(与RemoveContextObject相连)
	//投递了请求数据的异步请求
	VOID _CIOCPServer::PostReceive(CONTEXT_OBJECT* ConTextObject);
	BOOL _CIOCPServer::InitializeIOCP(VOID);  //初始化
	static DWORD WINAPI WorkThreadProcedure(LPVOID ParameterData);
	BOOL _CIOCPServer::HandleIO(PACKET_TYPE PacketType, PCONTEXT_OBJECT ContextObject, DWORD BufferLength);
	BOOL _CIOCPServer::OnInitializing(PCONTEXT_OBJECT ContextObject, DWORD BufferLength);  //现为测试代码
	//真正得到了数据端的数据
	BOOL _CIOCPServer::OnReceiving(PCONTEXT_OBJECT ContextObject, DWORD BufferLength);

	//构建数据包
	VOID _CIOCPServer::OnPrepareSending(CONTEXT_OBJECT* ContextObject, PBYTE BufferData, ULONG BufferLength);
	//将数据发送到客户端
	BOOL _CIOCPServer::OnSending(CONTEXT_OBJECT* ContextObject, ULONG BufferLength);
};

