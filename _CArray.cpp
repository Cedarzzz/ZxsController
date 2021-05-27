#include "stdafx.h"
#include "_CArray.h"
#define U_PAGE_ALIGNMENT   3
#define F_PAGE_ALIGNMENT 3.0


_CArray::_CArray()
{
	m_MaximumLength = 0;
	m_BufferData = m_CheckPosition = NULL;
	InitializeCriticalSection(&m_CriticalSection);
}


_CArray::~_CArray()
{
	if (m_BufferData)
	{
		VirtualFree(m_BufferData, 0, MEM_RELEASE);
		m_BufferData = NULL;
	}

	DeleteCriticalSection(&m_CriticalSection);
	m_BufferData = m_CheckPosition = NULL;
	m_MaximumLength = 0;
}

BOOL _CArray::WriteArray(PUINT8 BufferData, ULONG_PTR BufferLength)
{
	EnterCriticalSection(&m_CriticalSection);

	if (ReallocateArray(BufferLength + GetArrayLength()) == (ULONG_PTR)-1)
	{
		LeaveCriticalSection(&m_CriticalSection);
		return FALSE;
	}

	CopyMemory(m_CheckPosition, BufferData, BufferLength);

	m_CheckPosition += BufferLength;
	LeaveCriticalSection(&m_CriticalSection);
	return TRUE;
}

ULONG_PTR _CArray::ReallocateArray(ULONG_PTR BufferLength)
{
	if (BufferLength < GetArrayMaximumLength())
	{
		return 0;
	}

	//向上取边界    11/3.0
	ULONG_PTR v7 = (ULONG_PTR)ceil(BufferLength / F_PAGE_ALIGNMENT)*U_PAGE_ALIGNMENT;
	PUINT8 v5 = (PUINT8)VirtualAlloc(NULL, v7, MEM_COMMIT, PAGE_READWRITE);
	if (v5 == NULL)
	{
		return -1;
	}
	//原先的数据长度
	ULONG_PTR v3 = GetArrayLength(); 
	//拷贝原先数据到新的申请的内存中
	CopyMemory(v5, m_BufferData, v3);

	//释放原先内存
	if (m_BufferData)
	{
		VirtualFree(m_BufferData, 0, MEM_RELEASE);
	}
	m_BufferData = v5;
	m_CheckPosition = m_BufferData + v3;  //1024
	m_MaximumLength = v7;   //2048

	return m_MaximumLength;
}

ULONG_PTR _CArray::GetArrayMaximumLength()
{

	return m_MaximumLength;
}

ULONG_PTR _CArray::GetArrayLength()
{
	if (m_BufferData == NULL)
	{
		return 0;
	}
	return (ULONG_PTR)m_CheckPosition - (ULONG_PTR)m_BufferData;
}

PUINT8 _CArray::GetArray(ULONG_PTR Position)
{
	if (m_BufferData == NULL)
	{
		return NULL;
	}
	if (Position >= GetArrayLength())
	{
		return NULL;
	}

	return m_BufferData + Position;
}

VOID _CArray::ClearArray()
{
	EnterCriticalSection(&m_CriticalSection);
	m_CheckPosition = m_BufferData;
	//只保留1024内存
	DeallocateArray(1024);
	LeaveCriticalSection(&m_CriticalSection);
}

ULONG_PTR _CArray::DeallocateArray(ULONG_PTR BufferLength)
{
	if (BufferLength < GetArrayLength())
	{
		return 0;
	}
	ULONG_PTR v7 = (ULONG_PTR)ceil(BufferLength / F_PAGE_ALIGNMENT) * U_PAGE_ALIGNMENT;
	if (GetArrayMaximumLength() <= v7)
	{
		return 0;
	}
	PUINT8 v5 = (PUINT8)VirtualAlloc(NULL, v7, MEM_COMMIT, PAGE_READWRITE);

	ULONG_PTR v3 = GetArrayLength();    //算原来内存的有效长度
	CopyMemory(v5, m_BufferData, v3);

	VirtualFree(m_BufferData, 0, MEM_RELEASE);

	m_BufferData = v5;
	m_CheckPosition = m_BufferData + v3;
	m_MaximumLength = v7;
	return m_MaximumLength;
}

ULONG_PTR _CArray::ReadArray(PUINT8 BufferData, ULONG_PTR BufferLength)
{
	EnterCriticalSection(&m_CriticalSection);
	if (BufferLength > GetArrayMaximumLength())
	{
		LeaveCriticalSection(&m_CriticalSection);
		return 0;
	}
	if (BufferLength > GetArrayLength())
	{
		BufferLength = GetArrayLength();
	}

	if (BufferLength)
	{
		CopyMemory(BufferData, m_BufferData, BufferLength);
		MoveMemory(m_BufferData, m_BufferData + BufferLength, GetArrayMaximumLength() - BufferLength);
		m_CheckPosition -= BufferLength;
	}
	DeallocateArray(GetArrayLength());
	LeaveCriticalSection(&m_CriticalSection);
	return ULONG_PTR();
}

ULONG_PTR _CArray::RemoveArray(ULONG_PTR BufferLength)
{
	EnterCriticalSection(&m_CriticalSection);
	if (BufferLength > GetArrayMaximumLength())    //如果传进的内存比内存长度还大
	{
		LeaveCriticalSection(&m_CriticalSection);
		return 0;
	}
	if (BufferLength > GetArrayLength())           //如果传进的长度 比有效数据的的长度还大
	{
		BufferLength = GetArrayLength();
	}
	if (BufferLength)
	{
		MoveMemory(m_BufferData, m_BufferData + BufferLength, GetArrayMaximumLength() - BufferLength);
		m_CheckPosition -= BufferLength;
	}
	DeallocateArray(GetArrayLength());
	LeaveCriticalSection(&m_CriticalSection);
	return ULONG_PTR();
}

