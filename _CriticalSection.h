#pragma once
#include <Windows.h>
#include<iostream>
using namespace std;
class _CCriticalSection
{
public:
	_CCriticalSection(CRITICAL_SECTION& CriticalSection);
	~_CCriticalSection();
	void _CCriticalSection::Unlock();
	void _CCriticalSection::Lock();
private:
	CRITICAL_SECTION* m_CriticalSection;
};

