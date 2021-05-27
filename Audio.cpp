#include "stdafx.h"
#include "Audio.h"


CAudio::CAudio()
{
	m_IsWaveOutUsed = FALSE;
	m_IsWaveInUsed = FALSE;
	m_WaveInIndex = 0;
	m_WaveOutIndex = 0;

	//创建两个事件
	m_EventHandle1 = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_EventHandle2 = CreateEvent(NULL, TRUE, FALSE, NULL);

	//音频格式
	memset(&m_GSM610Wavefmt, 0, sizeof(GSM610WAVEFORMAT));
	m_GSM610Wavefmt.wfx.wFormatTag = WAVE_FORMAT_GSM610;
	m_GSM610Wavefmt.wfx.nChannels = 1;
	m_GSM610Wavefmt.wfx.nSamplesPerSec = 8000;
	m_GSM610Wavefmt.wfx.nAvgBytesPerSec = 1625;
	m_GSM610Wavefmt.wfx.nBlockAlign = 65;
	m_GSM610Wavefmt.wfx.wBitsPerSample = 0;
	m_GSM610Wavefmt.wfx.cbSize = 2;
	m_GSM610Wavefmt.wfx.nSamplesPerSec = 320;

	m_BufferLength = 1000;

	int i = 0;
	//两个U盘
	for (i = 0; i < 2; i++)
	{
		m_InAudioHeader[i] = new WAVEHDR;
		m_InAudioData[i] = new BYTE[m_BufferLength];

		m_OutAudioHeader[i] = new WAVEHDR;
		m_OutAudioData[i] = new BYTE[m_BufferLength];
	}
}


CAudio::~CAudio()
{
	int i = 0;
	if (m_IsWaveInUsed)
	{
		waveInStop(m_WaveInHandle);
		waveInReset(m_WaveInHandle);
		for (i = 0; i < 2; i++)
		{
			waveInUnprepareHeader(m_WaveInHandle, m_InAudioHeader[i], sizeof(WAVEHDR));
		}
		waveInClose(m_WaveInHandle);
		TerminateThread(m_WaveInThreadProcedure, -1);  //为了安全
	}
	for (i = 0; i < 2; i++)
	{
		delete m_InAudioData[i];
		m_InAudioData[i] = NULL;
		delete m_InAudioHeader[i];
		m_InAudioHeader[i] = NULL;
		
	}

	CloseHandle(m_EventHandle1);
	CloseHandle(m_EventHandle2);
	CloseHandle(m_WaveInThreadProcedure);


	if (m_IsWaveOutUsed)
	{
		waveOutReset(m_WaveOutHandle);
		for (i = 0; i < 2; i++)
		{
			waveOutUnprepareHeader(m_WaveOutHandle, m_InAudioHeader[i], sizeof(WAVEHDR));
		}
		waveOutClose(m_WaveOutHandle);
	}
	for (i = 0; i < 2; i++)
	{
		delete m_OutAudioData[i];
		m_OutAudioData[i] = NULL;
		delete m_OutAudioHeader[i];
		m_OutAudioHeader[i] = NULL;
		
	}
}
BOOL CAudio::PlayRecordData(LPBYTE BufferData, ULONG BufferLength)
{
	if (!m_IsWaveOutUsed&&!OnInitWaveOut())
	{
		return FALSE;
	}
	for (int i = 0; i < BufferLength; i += m_BufferLength)
	{
		memcpy(m_OutAudioData[m_WaveOutIndex], BufferData, m_BufferLength);
		waveOutWrite(m_WaveOutHandle,m_OutAudioHeader[m_WaveOutIndex],sizeof(WAVEHDR));
		m_WaveOutIndex = 1 - m_WaveOutIndex;
	}

	return TRUE;

}
BOOL CAudio::OnInitWaveOut()
{
	if (!waveOutGetNumDevs())
	{
		return FALSE;
	}
	int i;
	for (i = 0; i < 2; i++)
	{
		memset(m_OutAudioData[i], 0, m_BufferLength);  //声音数据
	}
	MMRESULT mmResult;
	mmResult = waveOutOpen(&m_WaveOutHandle, (WORD)WAVE_MAPPER, &(m_GSM610Wavefmt.wfx), (LONG)0, (LONG)0, CALLBACK_NULL);
	if (mmResult != MMSYSERR_ERROR)
	{
		return FALSE;
	}
	for (i = 0; i < 2; i++)
	{
		//磁带关联
		m_OutAudioHeader[i]->lpData = (LPSTR)m_OutAudioData[i];
		m_OutAudioHeader[i]->dwBufferLength = m_BufferLength;
		m_OutAudioHeader[i]->dwFlags = 0;
		m_OutAudioHeader[i]->dwLoops = 0;
		waveOutPrepareHeader(m_WaveOutHandle, m_OutAudioHeader[i], sizeof(WAVEHDR));
	}

	m_IsWaveOutUsed = TRUE;
	return TRUE;
}