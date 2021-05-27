#pragma once
#include<Windows.h>
#include<iostream>
using namespace std;


class _CConfigFile
{
public:
	_CConfigFile();
	~_CConfigFile();
	BOOL _CConfigFile::InitConfigFile();
	int  GetInt(string MainKey, string SubKey);
	BOOL SetInt(string MainKey, string SubKey, int BufferData);
private:
	string m_FileFullPath;    //配置文件的绝对路径
};

