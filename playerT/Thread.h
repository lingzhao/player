#pragma once

class CThread
{
private:
	HANDLE m_hThread;
	DWORD  m_dwThreadID;
	BOOL   m_bSuspend;
protected:
	virtual void Excute()=0;
public:
	static DWORD CALLBACK ThreadProc(LPVOID lpParameter)
	{
		CThread *caller=(CThread*)lpParameter;
		if (caller)
			caller->Excute();
		return 0;
	}


	CThread(BOOL bCreateSuspened=TRUE):m_hThread(NULL),m_dwThreadID(NULL)
	{
		m_hThread=CreateThread(NULL,0,ThreadProc,(void*)this,
			bCreateSuspened?CREATE_SUSPENDED:0,&m_dwThreadID);

		m_bSuspend=bCreateSuspened?TRUE:FALSE;
	}

	~CThread(void)
	{
		if (!m_hThread)
			CloseHandle(m_hThread);
	}


	void Resume()
	{
		if (m_bSuspend==TRUE&& m_hThread!=NULL)
		{
			ResumeThread(m_hThread);
			m_bSuspend=FALSE;
		}
	}

	void Suspend()
	{
		if (m_bSuspend==FALSE && m_hThread!=NULL)
		{
			SuspendThread(m_hThread);
			m_bSuspend=TRUE;
		}
	}

	BOOL Suspended() { return m_bSuspend; }

};