#pragma once
#include<Windows.h>
#include<math.h>

using namespace std;
class _declspec (dllexport) _CArray
{
public:
	_CArray();
	~_CArray();
	BOOL      _CArray::WriteArray(PUINT8 BufferData, ULONG_PTR BufferLength);   //写入
	ULONG_PTR _CArray::ReallocateArray(ULONG_PTR BufferLength);
	ULONG_PTR _CArray::GetArrayMaximumLength();
	ULONG_PTR _CArray::GetArrayLength();                           //获得有效长度
	PUINT8    _CArray::GetArray(ULONG_PTR Position = 0);           //获取
	VOID      _CArray::ClearArray();                               //清空
	ULONG_PTR _CArray::DeallocateArray(ULONG_PTR BufferLength);    //回收数组
	ULONG_PTR _CArray::ReadArray(PUINT8 BufferData, ULONG_PTR BufferLength);    //读取
	ULONG_PTR _CArray::RemoveArray(ULONG_PTR BufferLength);        //移动数组
private:
	PUINT8    m_BufferData;
	PUINT8    m_CheckPosition;
	ULONG_PTR m_MaximumLength;
	CRITICAL_SECTION m_CriticalSection;
};

