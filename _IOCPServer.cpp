#include "stdafx.h"
#include "_IOCPServer.h"


_CIOCPServer::_CIOCPServer()
{
    //初始化套接字类库
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		return;
	}

	memcpy(m_PacketHeaderFlag, "zengz", PACKET_FLAG_LENGTH);   //Shine-->Alessio
	m_KillEventHandle = NULL;
	m_ListenSocket = INVALID_SOCKET;
	m_ListenThreadHandle = NULL;
	m_ListenEventHandle = WSA_INVALID_EVENT;  //监听事件

	InitializeCriticalSection(&m_CriticalSection);
	m_CompletionPortHandle = INVALID_HANDLE_VALUE;

	m_ThreadPoolMin = 0;
	m_ThreadPoolMax = 0;
	m_WorkThreadCount = 0;
	m_CurrentThreadCount = 0;
	m_BusyThreadCount = 0;

	m_IsWorking = FALSE;


}


//服务器的主对话框发生关闭
_CIOCPServer::~_CIOCPServer()
{
	Sleep(1);
	//触发事件使其正常退出监听线程的循环
	SetEvent(m_KillEventHandle);

	//等待监听线程退出
	WaitForSingleObject(m_ListenThreadHandle, INFINITE);


	//回收各种资源
	if (m_ListenSocket != INVALID_SOCKET)
	{
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
	}
	if (m_ListenEventHandle != WSA_INVALID_EVENT)
	{
		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
	}
	if (m_CompletionPortHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_CompletionPortHandle);
		m_CompletionPortHandle = INVALID_HANDLE_VALUE;
	}

	m_ThreadPoolMax = 0;
	m_ThreadPoolMin = 0;
	m_WorkThreadCount = 0;
	m_CurrentThreadCount = 0;
	m_BusyThreadCount = 0;
	m_IsWorking = FALSE;

	//销毁关键代码段
	DeleteCriticalSection(&m_CriticalSection);
	WSACleanup();
}
BOOL _CIOCPServer::ServerRun(USHORT ListenPort, LPFN_WINDOWNOTIFYPROCEDURE WindowNotifyProcedure)
{

	m_WindowNotifyProcedure = WindowNotifyProcedure;
	//创建退出监听线程的事件
	m_KillEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_KillEventHandle == NULL)
	{
		return FALSE;
	}

	//创建一个监听套接字
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_ListenSocket == INVALID_SOCKET)
	{
		return FALSE;
	}

	//创建一个监听事件
	//事件选择模型
	m_ListenEventHandle = WSACreateEvent();
	if (m_ListenEventHandle == WSA_INVALID_EVENT)     //注意 我把"!="改成了"=="   已查
	{
		goto Error;
	}
	//将监听套接字与事件进行关联并授予FD_ACCEPT与FD_CLOSE的属性
	BOOL IsOk = WSAEventSelect(m_ListenSocket,  
		m_ListenEventHandle,
		FD_ACCEPT | FD_CLOSE);

	if (IsOk == SOCKET_ERROR)
	{
		goto Error;
	}

	//初始化Server端网卡
	SOCKADDR_IN ServerAddress;
	ServerAddress.sin_port = htons(ListenPort);
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = INADDR_ANY;

	//绑定套接字
	IsOk = bind(m_ListenSocket,
		(sockaddr*)&ServerAddress,
		sizeof(ServerAddress));
	if (IsOk == SOCKET_ERROR)
	{
		goto Error;
	}

	//监听
	IsOk = listen(m_ListenSocket, SOMAXCONN);
	if (IsOk == SOCKET_ERROR)
	{
		goto Error;
	}

	//创建监听线程
	m_ListenThreadHandle = 
		(HANDLE)CreateThread(NULL,
		0,
		(LPTHREAD_START_ROUTINE)ListenThreadProcedure,
		(void*)this,          //向线程回调传入This 方便我们的线程回调访问类中的成员
		0,
		NULL);
	if (m_ListenThreadHandle == INVALID_HANDLE_VALUE)
	{
		goto Error;
	}

	//初始化IOCP(当异步请求完成的时候)
	//创建完成端口
	//启动工作线程(守候在完成端口上等待异步请求的完成)
	InitializeIOCP();

	return TRUE;

Error:
	WSACloseEvent(m_ListenEventHandle);
	m_ListenEventHandle = WSA_INVALID_EVENT;
	closesocket(m_ListenSocket);
	m_ListenSocket = INVALID_SOCKET;
	return FALSE;
}
DWORD _CIOCPServer::ListenThreadProcedure(LPVOID ParameterData)
{
	_CIOCPServer* This = (_CIOCPServer*)ParameterData;

	int EventIndex = 0;
	WSANETWORKEVENTS NetWorkEvents;
	while (1)
	{
		EventIndex = WaitForSingleObject(This->m_KillEventHandle, 100);
		EventIndex = EventIndex - WAIT_OBJECT_0;
		if (EventIndex == 0)
		{
			//由于析构函数触发m_KillEventHandle事件
			break;
		}

		DWORD v1;
		//等待监听事件授信（监听套接字授信）
		v1 = WSAWaitForMultipleEvents(1,
			&This->m_ListenEventHandle,
			FALSE,
			100,
			FALSE);

		if (v1 == WSA_WAIT_TIMEOUT)
		{
			//该事件没有授信
			continue;
		}

		//发生了FD_ACCEPT或者FD_CLOSE事件才会进行到这里
		v1 = WSAEnumNetworkEvents(This->m_ListenSocket,
			//如果事件授信 我们就将该事件转换成一个网络事件进行判断
			This->m_ListenEventHandle,
			&NetWorkEvents);

		if (v1 == SOCKET_ERROR)
		{
			break;
		}
		if (NetWorkEvents.lNetworkEvents == FD_ACCEPT)    //监听套接字授信——>
		{                    //!!!!此处有BUG  是&还是==?????
			if (NetWorkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				//处理客户端上线请求
				This->OnAccept();

			}
			else
			{
				break;
			}
		}
		else
		{
			//当删除一个用户是代码会执行到这里不要在这里退出循环
			//break;   //？？？？？？bug
		}
	}
	return 0;
}
void _CIOCPServer::OnAccept()
{
	int Result = 0;


	//保存上线用户IP地址
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKADDR_IN ClientAddress = { 0 };
	int ClientAddressLength = sizeof(SOCKADDR_IN);

	//通过我们的监听套接字来生成一个与之对信号通信的套接字
	ClientSocket = accept(m_ListenSocket,
		(sockaddr*)&ClientAddress,
		&ClientAddressLength);

	if (ClientSocket == SOCKET_ERROR)
	{
		return;
	}

	//我们在这里为每一个到达的信号维护另一个与之关联的数据结构这里简称为用户的上下背景文
	PCONTEXT_OBJECT ContextObject = AllocateContextObject();

	if (ContextObject == NULL)
	{
		closesocket(ClientSocket);
		ClientSocket = INVALID_SOCKET;
		return;
	}

	//成员赋值
	ContextObject->ClientSocket = ClientSocket;

	//关联内存
	ContextObject->ReceiveWsaBuffer.buf = (char*)ContextObject->BufferData;
	ContextObject->ReceiveWsaBuffer.len = sizeof(ContextObject->BufferData);

	//将生成的通信套接字与完成端口句柄相关联
	HANDLE Handle = CreateIoCompletionPort((HANDLE)ClientSocket,
		m_CompletionPortHandle, (ULONG_PTR)ContextObject, 0);

	if (Handle != m_CompletionPortHandle)
	{
		delete ContextObject;
		ContextObject = NULL;
		if (ClientSocket != INVALID_SOCKET)
		{
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
		}
		return;
	}




	//保持链接检测对方主机是否崩溃 如果两小时内在此套接口的任意一方向都没有
	//数据交换，TCP就自动给对方发一个保持存活


	//方法: 设置套接字的选项卡 Set KeepAlive 开启保活机制 SO_KEEPALIVE
	m_KeepAliveTime = 3;
	BOOL IsOk = TRUE;
	
	if (setsockopt(ContextObject->ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&IsOk, sizeof(IsOk)) != 0)
	{

	}
	//设置超时详细信息
	tcp_keepalive v1;
	v1.onoff = 1;                          //启用保活
	v1.keepalivetime = m_KeepAliveTime;    //超过三分钟没有数据，发送探测包
	v1.keepaliveinterval = 1000 * 10;      //重试间隔为10秒
	WSAIoctl
		(
			ContextObject->ClientSocket,
			SIO_KEEPALIVE_VALS,
			&v1,
			sizeof(v1),
			NULL,
			0,
			(unsigned long *)&IsOk,
			0,
			NULL
		);

	_CCriticalSection CriticalSection(m_CriticalSection);
	m_ConnectionContextObjectList.AddTail(ContextObject);   //插入到我们的内存列表中

	OVERLAPPEDEX *OverlappedEx = new OVERLAPPEDEX(IO_INITIALIZE);

	IsOk = FALSE;
	//向完成端口中投递一个请求
	//工作线程会等待完成端口的完成状态
	IsOk = PostQueuedCompletionStatus(m_CompletionPortHandle,
		0, (ULONG_PTR)ContextObject, &OverlappedEx->m_Overlapped);

	if ((!IsOk && GetLastError() != ERROR_IO_PENDING))
	{
		//如果投递失败,将上下背景文回收到内存池 移除
		RemoveContextObject(ContextObject);
		return;
	}

	//该上线用户已完成了上线的请求
	//服务器向该用户投递PostRecv请求
	PostReceive(ContextObject);
}
VOID _CIOCPServer::PostReceive(CONTEXT_OBJECT* ContextObject)
{
	//向我们刚上线的用户投递一个接受数据的请求
	//如果该请求得到完成(用户发送数据)
	//工作线程(守候在完成端口)会响应并调用ProcessIOMessage函数
	OVERLAPPEDEX* OverLappedEx = new OVERLAPPEDEX(IO_RECEIVE);

	DWORD ReturnLength;
	ULONG Flags = MSG_PARTIAL;
	int IsOk = WSARecv(ContextObject->ClientSocket,
		&ContextObject->ReceiveWsaBuffer,    //接收数据的内存
		1,
		&ReturnLength,                       //TransferBufferLength
		&Flags,
		&OverLappedEx->m_Overlapped,
		NULL);

	if (IsOk == SOCKET_ERROR&&WSAGetLastError() != WSA_IO_PENDING)
	{
		//请求发送错误
		RemoveContextObject(ContextObject);
	}
}
PCONTEXT_OBJECT _CIOCPServer::AllocateContextObject()
{
	int a = GetLastError();
	PCONTEXT_OBJECT ContextObject = NULL;
	//进入临界区
	_CCriticalSection CriticalSection(m_CriticalSection);
	//判断内存池是否为空
	if (m_FreeContextObjectList.IsEmpty() == FALSE)
	{
		//内存池取内存
		ContextObject = m_FreeContextObjectList.RemoveHead();
	}
	else
	{
		ContextObject = new CONTEXT_OBJECT;
	}
	if (ContextObject != NULL)
	{
		ContextObject->InitMember();  //初始化成员变量
	}
	return ContextObject;
}
VOID _CIOCPServer::RemoveContextObject(CONTEXT_OBJECT* ContextObject)
{
	_CCriticalSection CriticalSection(m_CriticalSection);
	//在内存中查找该用户的上下背景文数据结构
	if (m_ConnectionContextObjectList.Find(ContextObject))
	{
		//取消在当前套接字的异步IO以前的未完成的异步请求全部立即取消
		CancelIo((HANDLE)ContextObject->ClientSocket);
		//关闭套接字
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = INVALID_SOCKET;
		//判断还有没有异步请求在当前的套接字上
		while (!HasOverlappedIoCompleted((LPOVERLAPPED)ContextObject))
		{
			Sleep(1);
		}
		//将该内存结构回收至内存池
		MoveContextObjectToFreePool(ContextObject);
	}
}
VOID _CIOCPServer::MoveContextObjectToFreePool(CONTEXT_OBJECT* ContextObject)
{
	_CCriticalSection CriticalSection(m_CriticalSection);

	POSITION Position = m_ConnectionContextObjectList.Find(ContextObject);
	if (Position)
	{
		ContextObject->m_ReceivedCompressedBufferData.ClearArray();
		ContextObject->m_ReceivedDecompressedBufferData.ClearArray();
		ContextObject->m_SendCompressedBufferData.ClearArray();
		memset(ContextObject->BufferData, 0, PACKET_LENGTH);
		m_FreeContextObjectList.AddTail(ContextObject);      //回收至内存池
		m_ConnectionContextObjectList.RemoveAt(Position);    //从内存结构中移除
	}

}
BOOL _CIOCPServer::InitializeIOCP(VOID)
{
	m_CompletionPortHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (m_CompletionPortHandle == NULL)   //创建完成端口
	{
		return FALSE;
	}
	//线程池
	SYSTEM_INFO SystemInfo;
	//获得pc中有几核
	GetSystemInfo(&SystemInfo);
	m_ThreadPoolMin = 1;
	m_ThreadPoolMax = SystemInfo.dwNumberOfProcessors * 2;

	ULONG  WorkThreadCount = 2;
	HANDLE WorkThreadHandle = NULL;
	for (int i = 0; i < WorkThreadCount; i++)
	{
		WorkThreadHandle = CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)WorkThreadProcedure,
			(void*)this,
			0,
			NULL);
		if (WorkThreadHandle == NULL)
		{
			CloseHandle(m_CompletionPortHandle);
			return FALSE;
		}
		m_WorkThreadCount++;
		CloseHandle(WorkThreadHandle);
	}
}
DWORD _CIOCPServer::WorkThreadProcedure(LPVOID ParameterData)
{
	_CIOCPServer* This = (_CIOCPServer*)ParameterData;

	HANDLE CompletionPortHandle = This->m_CompletionPortHandle;
	DWORD ReturnLength = 0;    //有多少数据得到完成
	PCONTEXT_OBJECT ContextObject = NULL;
	LPOVERLAPPED    Overlapped = NULL;
	OVERLAPPEDEX*   OverlappedEx = NULL;

	ULONG v3 = 0;       //正在工作的线程数目计数器
	BOOL v1 = FALSE;


	InterlockedIncrement(&This->m_CurrentThreadCount);  //原子锁
	InterlockedIncrement(&This->m_BusyThreadCount);     //工作的数目

	while (This->m_IsWorking == FALSE)
	{
		InterlockedDecrement(&This->m_BusyThreadCount);
		//该函数是阻塞函数如果该函数返回说明有请求得到了完成
		BOOL IsOk = GetQueuedCompletionStatus(
			CompletionPortHandle,
			&ReturnLength,                 //完成多少数据
			(PULONG_PTR)&ContextObject,    //完成key
			&Overlapped, 60000);

		/*OVERLAPPEDEX *OverlappedEx = new OVERLAPPEDEX(IO_INITIALIZE);
		IsOk = FALSE;
		IsOk = PostQueuedCompletionStatus(m_CompletionPortHandle,
			0, (ULONG_PTR)ContextObject, &OverlappedEx->m_Overlapped);
			*/


		DWORD LastError = GetLastError();
		OverlappedEx = CONTAINING_RECORD(Overlapped, OVERLAPPEDEX, m_Overlapped);

		v3 = InterlockedIncrement(&This->m_BusyThreadCount);

		if (!IsOk && LastError != WAIT_TIMEOUT)
		{
			if (ContextObject && This->m_IsWorking == FALSE&& ReturnLength == 0)
			{
				//当对方套接字发生关闭
				This->RemoveContextObject(ContextObject);
			}
			continue;
		}
		if (!v1)
		{
			if (v3 == This->m_CurrentThreadCount)
			{
				if (v3 < This->m_ThreadPoolMax)
				{
					if (ContextObject != NULL)
					{
						HANDLE ThreadHandle = (HANDLE)CreateThread(NULL,
							0,
							(LPTHREAD_START_ROUTINE)WorkThreadProcedure,
							(void*)This,
							0,
							NULL);
						InterlockedIncrement(&This->m_WorkThreadCount);

						CloseHandle(ThreadHandle);
					}
				}
			}
			//从线程池销毁一个线程
			if (!IsOk && LastError == WAIT_TIMEOUT)
			{
				if (ContextObject == NULL)
				{
					{
						if (This->m_CurrentThreadCount > This->m_ThreadPoolMin)
						{
							break;
						}
					}
					v1 = TRUE;
				}
			}
		}
		if (!v1)
		{
			//请求得到完成(两种请求 IO_INITIALIZE  IO_RECEIVE)
			
			if (IsOk && OverlappedEx != NULL && ContextObject != NULL)
			{
				try
				{
					//请求得到完成的处理函数
					This->HandleIO(OverlappedEx->m_PackType, ContextObject, ReturnLength);

					//没有释放内存
					ContextObject = NULL;
				}
				catch(...){}
			}
		}
		if (OverlappedEx)
		{
			delete OverlappedEx;
			OverlappedEx = NULL;
		}
	}
	InterlockedDecrement(&This->m_WorkThreadCount);
	InterlockedDecrement(&This->m_CurrentThreadCount);
	InterlockedDecrement(&This->m_BusyThreadCount);
	return 0;
}
BOOL _CIOCPServer::HandleIO(PACKET_TYPE PacketType, PCONTEXT_OBJECT ContextObject, DWORD BufferLength)
{
	BOOL v1 = FALSE;
	if (IO_INITIALIZE == PacketType)
	{
		//函数的投递请求
		v1 = OnInitializing(ContextObject, BufferLength);
	}

	if (IO_RECEIVE == PacketType)
	{
		//客户端发送数据
		v1 = OnReceiving(ContextObject, BufferLength);
	}

	if (IO_SEND == PacketType)
	{
		//发送数据到客户端
		v1 = OnSending(ContextObject, BufferLength);
	}
	return v1;
}
BOOL _CIOCPServer::OnInitializing(PCONTEXT_OBJECT ContextObject, DWORD BufferLength)
{

	//MessageBox(NULL, "HelloInit", "HelloInit", 0);

	return TRUE;
}
BOOL _CIOCPServer::OnReceiving(PCONTEXT_OBJECT ContextObject, DWORD BufferLength)
{
	_CCriticalSection CriticalSection(m_CriticalSection);
	try
	{
		if (BufferLength == 0)
		{
			//对方关闭了套接字
			MessageBox(NULL, "关闭套接字", "关闭套接字", 0);
			RemoveContextObject(ContextObject);
			return FALSE;
		}

		//将接到的数据拷贝到m_ReceivedCompressedBufferData
		ContextObject->m_ReceivedCompressedBufferData.WriteArray(
			(PBYTE)ContextObject->BufferData, BufferLength);

		//读取数据包的头部(数据包的头部是不参与压缩的)
		while (ContextObject->m_ReceivedCompressedBufferData.GetArrayLength() > PACKET_HEADER_LENGTH)  //?????是m_ReceivedCompressedBufferData还是m_ReceivedDecompressedBufferData
		{
			//储存数据包头部标志
			char v1[PACKET_FLAG_LENGTH] = { 0 };

			//拷贝数据包头部标志
			CopyMemory(v1, ContextObject->m_ReceivedCompressedBufferData.GetArray(), PACKET_FLAG_LENGTH);

			//校验数据包头部标志
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGTH) != 0)
			{
				throw "Bad Buffer";
			}

			//获取数据包总大小
			ULONG PackTotalLength = 0;
			CopyMemory(&PackTotalLength,
				ContextObject->m_ReceivedCompressedBufferData.GetArray(PACKET_FLAG_LENGTH),
				sizeof(ULONG));

			//取出数据包总长
			if (PackTotalLength &&
				(ContextObject->m_ReceivedCompressedBufferData.GetArrayLength()) >= PackTotalLength)
			{
				//[Alessio][压缩的长度+13][没有压缩的长度][HelloWorld(真实数据)]

				ULONG DecompressedLength = 0;

				//取[Alessio]
				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)v1, PACKET_FLAG_LENGTH);
				//取[压缩的长度+13]
				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)&PackTotalLength, sizeof(ULONG));
				//取[没有压缩的长度]
				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)&DecompressedLength, sizeof(ULONG));

				//只剩[HelloWorld(真实数据)]

				ULONG CompressedLength = PackTotalLength - PACKET_HEADER_LENGTH;

				//用来存放压缩数据
				PBYTE CompressedData = new BYTE[CompressedLength];

				//用来存放解压缩数据
				PBYTE DecompressedData = new BYTE[DecompressedLength];

				if (CompressedData == NULL || DecompressedData == NULL)
				{
					throw "Bad Allocate";
				}

				//从数据包中获取压缩后的数据 即[HelloWorld(真实数据)]
				ContextObject->m_ReceivedCompressedBufferData.ReadArray(CompressedData, CompressedLength);


				//解压缩数据
				int IsOk = uncompress(DecompressedData,
					&DecompressedLength, CompressedData, CompressedLength);

				if (IsOk == Z_OK)
				{
					ContextObject->m_ReceivedDecompressedBufferData.ClearArray();
					ContextObject->m_ReceivedCompressedBufferData.ClearArray();

					//拷贝真实数据
					ContextObject->m_ReceivedDecompressedBufferData.WriteArray(DecompressedData,
						DecompressedLength);

					delete[] CompressedData;
					delete[] DecompressedData;

					m_WindowNotifyProcedure(ContextObject);   //通知窗口
				}
				else
				{
					delete[] CompressedData;
					delete[] DecompressedData;
					throw "Bad Buffer";
				}

			}
			else
			{
				break;
			}
		}

		//上一次的异步请求已经得到完成 重新投递新的异步请求
		PostReceive(ContextObject);
	}
	catch (...)
	{
		ContextObject->m_ReceivedDecompressedBufferData.ClearArray();
		ContextObject->m_ReceivedCompressedBufferData.ClearArray();

		PostReceive(ContextObject);
	}

	return TRUE;
}
//准备发送数据
VOID _CIOCPServer::OnPrepareSending(CONTEXT_OBJECT* ContextObject, PBYTE BufferData, ULONG BufferLength)
{
	if (ContextObject == NULL)
	{
		return;
	}
	try
	{
		if (BufferLength > 0)
		{
			unsigned long CompressedLength = (double)BufferLength*1.001 + 12;
			LPBYTE CompressedData = new BYTE[CompressedLength];
			int IsOk = compress(CompressedData, &CompressedLength, (LPBYTE)BufferData, BufferLength);

			if (IsOk != Z_OK)
			{
				delete[] CompressedData;
				return;
			}
			ULONG PackTotalLength = CompressedLength + PACKET_HEADER_LENGTH;
			//构建数据包头部
			ContextObject->m_SendCompressedBufferData.WriteArray((LPBYTE)m_PacketHeaderFlag,PACKET_FLAG_LENGTH);
			//[Shine]
			ContextObject->m_SendCompressedBufferData.WriteArray((PBYTE)&PackTotalLength,sizeof(ULONG));
			//[Shine][PackTotalLength]
			ContextObject->m_SendCompressedBufferData.WriteArray((PBYTE)&BufferLength, sizeof(ULONG));
			//[Shine][PackTotalLength][BufferLength]
			ContextObject->m_SendCompressedBufferData.WriteArray(CompressedData,CompressedLength);
			//[Shine][PackTotalLength][BufferLength][真实数据]
			delete[] CompressedData;
		}
		OVERLAPPEDEX* OverlappedEx = new OVERLAPPEDEX(IO_SEND);

		//将该请求投递到完成端口
		PostQueuedCompletionStatus(m_CompletionPortHandle, 0, (DWORD)ContextObject, &OverlappedEx->m_Overlapped);
	}
	catch (...) {}
}
BOOL _CIOCPServer::OnSending(CONTEXT_OBJECT* ContextObject, ULONG BufferLength)
{
	try
	{
		DWORD Flags = MSG_PARTIAL;
		//将完成的数据从数据结构中移除
		ContextObject->m_SendCompressedBufferData.RemoveArray(BufferLength);
		//判断还有多少数据需要发送
		if (ContextObject->m_SendCompressedBufferData.GetArrayLength() == 0)
		{
			//为0 即数据已经发送完毕
			ContextObject->m_SendCompressedBufferData.ClearArray();
			return true;
		}
		else
		{
			//真正发送数据
			OVERLAPPEDEX* OverlappedEx = new OVERLAPPEDEX(IO_SEND);
			ContextObject->SendWsaBuffer.buf = (char*)ContextObject->m_SendCompressedBufferData.GetArray();
			ContextObject->SendWsaBuffer.len = ContextObject->m_SendCompressedBufferData.GetArrayLength();
			int IsOk = WSASend(ContextObject->ClientSocket,
				&ContextObject->SendWsaBuffer,
				1,
				&ContextObject->SendWsaBuffer.len,
				Flags,
				&OverlappedEx->m_Overlapped,
				NULL);
			if (IsOk == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				RemoveContextObject(ContextObject);
			}
		}
	}
	catch(...){}
	return FALSE;
}