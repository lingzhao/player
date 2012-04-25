#include "stdafx.h"
#include "BasicPlayer.h"
#include "MusicFile.h"
#include "PlayerThread.h"
#include "SpectrumAnalyser.h"
#include "Mp3File.h"
#include "WaveFileEx.h"
#include "mainfrm.h"
#include "PlayList.h"

 CBasicPlayer* CBasicPlayer::shared()
 {
 	static CBasicPlayer *pGlobalBasePlayer=NULL;
 	if(!pGlobalBasePlayer)
 		pGlobalBasePlayer=new CBasicPlayer;
 	return pGlobalBasePlayer;
 }

 void CPlayerController::Excute()
 {
	 while(1)
	 {
		 ::WaitForSingleObject(decayEvent,INFINITE);
		 const static int indecayLen1=13;
		 static double indecay1[indecayLen1]={-9000, -8000, -7000, -6000, -5000, -4000, -3000, -2000, -1000, -500, -400, -200, 0};
		 static double indecay2[indecayLen1]={ -50 , -100,  -200,  -500,  -800, -1000, -2000, -3000, -4000, -5000, -6000, -8000, -9000};

		 if (m_pPlayerThread->m_pPlayer->bDecay)    
		 {
			 for (int i=0;i<indecayLen1;i++)
			 {
				 m_pPlayerThread->m_lpDSBuffer->SetVolume(indecay2[i]);
				 Sleep(50);
			 }
			 m_pPlayerThread->m_lpDSBuffer->Stop();
		 }
		 else
		 {
			 m_pPlayerThread->m_lpDSBuffer->Play(0,0,DSBPLAY_LOOPING);
			 for (int i=0;i<indecayLen1;i++)
			 {
				 m_pPlayerThread->m_lpDSBuffer->SetVolume(indecay1[i]);
				 Sleep(100);
			 }
			 
		 }
	 }
 }

 CBasicPlayer :: CBasicPlayer(void):
 m_bStopped(TRUE),m_bPaused(TRUE),
	 m_pFile(NULL),m_bFileEnd(FALSE)
{
	m_hPausedEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hPauseEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hWStartEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	m_pPlayerThread=new CPlayerThread(this);
	m_pPlayerThread->Resume();
	m_pSpectrumAnalyser=new CSpectrumAnalyser;

	//test
	ctl=new CPlayerController(m_pPlayerThread);
	ctl->Resume();
}

CBasicPlayer :: ~CBasicPlayer(void)
{
	if(!m_pPlayerThread) delete m_pPlayerThread;
	if (!m_pSpectrumAnalyser) delete m_pSpectrumAnalyser; 
}

const TCHAR* CBasicPlayer::playNextPlaylistItem()
{
	PlayListItem *item=MyLib::GetPlayListObj().GetNextTrackByOrder();
	if(item  && open((LPTSTR) item->url.c_str()) )
	{
		play();
		return item->title.c_str();
	}
	return NULL;
}

BOOL CBasicPlayer::open( LPTSTR filepath )
{
	int len=_tcslen(filepath);
	TCHAR* p=filepath+len;
	while (p--)
		if ((TCHAR)(*p)=='.')
		{
			p++;
			break;
		}
	
	if(m_pFile)
		delete m_pFile;
	m_bFileEnd=FALSE;
	
	if (_tcscmp(p,_T("wav"))==0)
		m_pFile=new WaveFile();
	if (_tcscmp(p,_T("mp3"))==0)
		m_pFile=new Mp3File();
	if (_tcscmp(p,_T("wma"))==0)
		m_pFile=new Mp3File();

	return m_pFile->OpenAndReadID3Info(filepath);
}


void CBasicPlayer::play()
{
	if (m_bFileEnd)	return;
	
	if (!m_bStopped)
		if (m_bPaused)      //未停止且暂停
		{
			pause();        //继续
			return;
		}
		else               //正在播放
			stop();        //先停止


	m_pFile->ResetFile();
	m_pPlayerThread->reset();
	m_pPlayerThread->CleanDSBuffer();
	m_pPlayerThread->WriteDataToDSBuf();
	m_pPlayerThread->WriteDataToDSBuf();
	m_pPlayerThread->m_lpDSBuffer->Play( 0, 0, DSBPLAY_LOOPING);
	
	::SetEvent(m_hWStartEvent);

	m_bStopped=FALSE;
	m_bPaused=FALSE;
}




void CBasicPlayer::decayPlay(BOOL bDecay)
{
	const static int decayLen=13;
	static double decay[decayLen]={0.8, 0.7, 0.6, 0.5, 0.4, 0.35, 0.3, 0.25, 0.2, 0.15, 0.1, 0.05, 0.01};

	//信号“变强”英文没找到,用 in deycay.
	const static int indecayLen=13;
	static double indecay[indecayLen]={0.03, 0.07, 0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 1};

	
	const static int indecayLen1=13;
	static double indecay1[indecayLen1]={-900, -800, -700, -600, -500, -400, -300, -200, -100, -50, -40, -20, 0};
	static double indecay2[indecayLen1]={ -10,  -20,  -50,  -80, -100, -200, -300, -400, -500, -600, -800, -900};
	

	if (bDecay)    
	{
		for (int i=0;i<decayLen;i++)
		{
			m_pPlayerThread->m_lpDSBuffer->SetVolume(indecay1[i]);
			Sleep(100);
			

			//m_pFile->SetOutVolume(decay[i]);
			//m_pPlayerThread->WriteDataToDSBuf();
		}
	}
	else
	{
		for (int i=0;i<indecayLen;i++)
		{
			m_pPlayerThread->m_lpDSBuffer->SetVolume(indecay2[i]);
			Sleep(100);
			//m_pFile->SetOutVolume(indecay[i]);
			//m_pPlayerThread->WriteDataToDSBuf();
			if (i==3)
				m_pPlayerThread->m_lpDSBuffer->Play( 0, 0, DSBPLAY_LOOPING);
		}
	}
}

void CBasicPlayer::pause()
{
	if (m_bStopped)return;

	if (m_bPaused)    //resume
	{
		m_bPaused=FALSE;

		bDecay=FALSE;
		::SetEvent(ctl->decayEvent);

		//decayPlay(FALSE);
		//::SetEvent(m_hWStartEvent);
	}
	else                        //pause       
	{
		m_bPaused=TRUE;

// 		::SetEvent(m_hPauseEvent);
// 		::WaitForSingleObject(m_hPausedEvent,INFINITE);
// 		::ResetEvent(m_hWStartEvent);
		
		bDecay=TRUE;
		::SetEvent(ctl->decayEvent);
		//decayPlay();
		//m_pPlayerThread->m_lpDSBuffer->Stop();

	}
}

BOOL CBasicPlayer::stoped()
{
	return m_bStopped;
}

void CBasicPlayer::stop()
{
	if(!m_bStopped)
	{
		m_pPlayerThread->m_cs->Enter();
		::ResetEvent(m_hWStartEvent);
		m_pPlayerThread->m_lpDSBuffer->Stop();
		m_pPlayerThread->m_cs->Leave();

		m_bStopped=TRUE;
		m_bPaused=TRUE;

		::PostMessage(this->m_pMainFrame->m_hWnd,WM_TRACKPOS,0,100);
	}
}