#pragma once
#include "MySerialize.h"
#include "LrcMng.h"
#include <iostream>
#include <fstream>
#include <string>
#include <list>

//for parse the ID3 tag
#include <direct.h>
#include <stdlib.h>
#include <tbytevector.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <id3v2header.h>
#include <id3v1tag.h>
#include <apetag.h>
#include <taglib.h>
#include <fileref.h>
#include <tbytevector.h>
using namespace TagLib;

class PlayList;

class PlayListItem: public SerializeObj
{
public:
	virtual int SerializeB(FILE* pFile);
	virtual int ReSerialize(FILE* pFile);

	bool operator==(const PlayListItem &other)
	{
		if(&other == this )
			return true;
		return false;
	}
public:
	PlayListItem(PlayList *playlist):
		m_pPL(playlist)
		,playCount(0),starLvl(1),indexInListView(-1)
		,pPicBuf(NULL),img(NULL),year(0)
		,m_bLrcInner(FALSE),m_bLrcFromLrcFile(FALSE)
		,bUnsynLyc(FALSE),m_bStatus(UNKNOWN)
	{
	}

	PlayListItem(PlayList *playlist,std::tstring *url):
		m_pPL(playlist),url(*url)
		,playCount(0),starLvl(1),indexInListView(-1)
		,pPicBuf(NULL),img(NULL),year(0)
		,m_bLrcInner(FALSE),m_bLrcFromLrcFile(FALSE)
		,bUnsynLyc(FALSE),m_bStatus(UNKNOWN)
	{
	}

	~PlayListItem();
public:
	PlayList *m_pPL;
public:
	int indexInListView;
	std::tstring url;
	UINT          playCount;
	UINT		  starLvl;
	//-----id3 info-------
	std::tstring  title,artist,album,genre,comment;
	UINT year;
	TagLib::ByteVector *pPicBuf;
	CImage *img;
	//cimg_library_suffixed::CImg   *img;
		
	//lrc
	BOOL m_bLrcInner;
	std::tstring  lyricInner;

	BOOL m_bLrcFromLrcFile;
	std::tstring  lycPath;
	LrcLines      lyricFromLrcFile;

	BOOL bUnsynLyc;
public:
	
	enum ID3Status
	{
		UNKNOWN,
		INVALIE,
		ID3V2,
		ID3V1
	};
	ID3Status m_bStatus;

	BOOL  ScanId3Info(BOOL bRetainPic=FALSE);
	const TCHAR* GetTitle(){return title.c_str();}
	BOOL  GetLrcFileFromLib();
	BOOL  HaveKeywords(TCHAR *keywords);
private:
	void Buf2Img(BYTE* lpRsrc,DWORD len);
	BOOL  IsInalid();//abondaned
	BOOL LrcFileMacth(std::tstring &filename);
};


typedef PlayListItem MyTrack;


class PlayList:
	public SerializeObj
{
public:
	virtual int SerializeB(FILE* pFile);
	virtual int ReSerialize(FILE* pFile);
public:
	list<PlayListItem> m_songList;
	std::tstring       m_playlistName;
	std::tstring       m_saveLocation;
private:
	PlayListItem *lastPlayingItem,*curPlayingItem,
		*nextPlayingItem,*curSelectedItem;
	
public:
	void SetCurPlaying(PlayListItem* item,BOOL scanID3=TRUE);
	PlayListItem* lastTrack(){return lastPlayingItem;}
	PlayListItem* curTrack(){return curPlayingItem;};
	PlayListItem* nextTrack();
public:
	PlayList(void);
	PlayList(std::tstring &name);
	~PlayList(void);
	
public:
	//operation
	void DeleteTrack(PlayListItem* track);

public:
	HANDLE hAddDir;
	BOOL AddFolderByThread(LPCTSTR pszFolder);
	void scanAllId3Info();
	BOOL AddFolder(LPCTSTR pszFolder,BOOL bIncludeDir=FALSE);
	void TerminateAddDirThread();
public:
	PlayListItem* GetNextTrackByOrder(BOOL bMoveCur=TRUE);
};