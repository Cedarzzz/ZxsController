#include "stdafx.h"
#include "_ConfigFile.h"



_CConfigFile::_CConfigFile()
{
	InitConfigFile();
}


_CConfigFile::~_CConfigFile()
{
}

BOOL _CConfigFile::InitConfigFile()
{
	CHAR FileFullPath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, FileFullPath, MAX_PATH);   //获得当前可执行文件的绝对路径

	CHAR* v1 = NULL;


	v1 = strstr(FileFullPath, ".");
	if (v1 != NULL)
	{
		*v1 = '\0';
		strcat(FileFullPath, ".ini");
	}

	//创建一个ini文件
	HANDLE FileHandle = CreateFileA(FileFullPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE,   //独占
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN, NULL);  //同步  异步
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	m_FileFullPath = FileFullPath;

	ULONG HighLength = 0;
	ULONG LowLength = GetFileSize(FileHandle, &HighLength);  //获得文件大小
	if (LowLength > 0 || HighLength > 0)    //不是空文件
	{
		CloseHandle(FileHandle);
		return FALSE;
	}
	CloseHandle(FileHandle);
	WritePrivateProfileStringA("Settings", "ListenPort", "2356", m_FileFullPath.c_str());
	WritePrivateProfileStringA("Settings", "MaxConnection", "10", m_FileFullPath.c_str());
	return TRUE;

}
int  _CConfigFile::GetInt(string MainKey, string SubKey)   //"Setting"
{
	return ::GetPrivateProfileIntA(MainKey.c_str(), SubKey.c_str(), 0, m_FileFullPath.c_str());   //ini文件的绝对路径
}
BOOL _CConfigFile::SetInt(string MainKey, string SubKey, int BufferData)
{
	string v1;
	sprintf((char*)v1.c_str(), "%d", BufferData);
	return ::WritePrivateProfileStringA(MainKey.c_str(), SubKey.c_str(), v1.c_str(), m_FileFullPath.c_str());
}