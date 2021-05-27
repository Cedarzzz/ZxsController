#pragma once
#include <Windows.h>
#include <mmeapi.h>
#include <MMSYSTEM.h>
#include <MMReg.h>

#pragma comment(lib,"Winmm.lib")

class CAudio
{
public:
	CAudio();
	~CAudio();
	BOOL CAudio::PlayRecordData(LPBYTE BufferData, ULONG BufferLength);
	BOOL CAudio::OnInitWaveOut();
private:
	GSM610WAVEFORMAT m_GSM610Wavefmt;
	BOOL m_IsWaveOutUsed;
	BOOL m_IsWaveInUsed;
	ULONG m_BufferLength;
	LPWAVEHDR m_InAudioHeader[2];   //两个U盘头信息
	LPBYTE    m_InAudioData[2];        //两个真正的U盘

	HWAVEIN m_WaveInHandle;         //录音设备句柄
	DWORD   m_WaveInIndex;
	HANDLE  m_EventHandle1;
	HANDLE  m_EventHandle2;
	HANDLE   m_WaveInThreadProcedure;

	HWAVEOUT m_WaveOutHandle;
	DWORD m_WaveOutIndex;
	LPWAVEHDR m_OutAudioHeader[2];  //两个头
    LPBYTE m_OutAudioData[2];       //两个数据  保持声音的连接
};

