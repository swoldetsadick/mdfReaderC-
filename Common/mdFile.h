/*********************************************************************************************
  Copyright 2011 Michael Bührer & Bernd Sparrer. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are
  permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice, this list of
        conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright notice, this list
        of conditions and the following disclaimer in the documentation and/or other materials
        provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY Michael Bührer  & Bernd Sparrer ``AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Michael Bührer OR Bernd Sparrer
  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those of the
  authors and should not be interpreted as representing official policies, either expressed
  or implied, of Michael Bührer & Bernd Sparrer.
***********************************************************************************************/
#pragma once

#include "mdfTypes.h"
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#else
#define fopen64 fopen
#define fseeko64 _fseeki64
#define ftello64 _ftelli64
#endif

#if (!_MDF4_WIN)
#undef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE NULL
#endif

//----------------------------------------------------------------------
// mDirectFile - a File that can read/write to absolute locations
// and keeps track of the size
// used for MDF3 and MDF4 files
//----------------------------------------------------------------------
class mDirectFile
{
public:
  mDirectFile() : m_hFile(INVALID_HANDLE_VALUE),m_cp(-1),m_Size(0)
  {
  }
  virtual ~mDirectFile()
  {
    Close();
  }
  virtual void Close()
  {
    if (isOpen())
    {
#if _MDF4_WIN
      CloseHandle(m_hFile);
#else
      fclose(m_hFile);
#endif
    }
    m_cp=-1;
    m_Size=0;
    m_hFile=INVALID_HANDLE_VALUE;
  }
	bool OpenRead(M_FILENAME PathName)
  {
#if _MDF4_WIN
    HANDLE hFile=CreateFile(PathName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
#else
    FILE *hFile=fopen64(PathName,"rb");
#endif
    return Assign(hFile);
  }
  bool OpenUpdate(M_FILENAME PathName)
  {
#if _MDF4_WIN
    HANDLE hFile=CreateFile(PathName,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
#else
    FILE *hFile=fopen64(PathName,"r+b");
#endif
    return Assign(hFile);
  }
  bool OpenCreate(M_FILENAME PathName)
  {
#if _MDF4_WIN
    HANDLE hFile=CreateFile(PathName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
#else
    FILE *hFile=fopen64(PathName,"wb");
#endif
    return Assign(hFile);
  }

  bool isOpen() const { return m_hFile!=INVALID_HANDLE_VALUE; }
  BOOL ReadAt(M_LINK lAt,M_UINT32 szBytes,void * Into);
  BOOL WriteAt(M_LINK lAt,M_UINT32 szBytes,const void * Data);
  void Seek(M_LINK ToPos) 
  {
    if (isOpen()) 
    {
#if _MDF4_WIN
      LARGE_INTEGER sr;
      memcpy(&sr,&ToPos,sizeof(sr));
      SetFilePointerEx(m_hFile,sr,&sr,FILE_BEGIN);
      memcpy(&m_cp,&sr,sizeof(sr));
#else
      fseeko64(m_hFile, ToPos, SEEK_SET);
      m_cp = ToPos;
#endif
    }
  }
  M_LINK AtEnd() const { return m_Size; }
  M_LINK Request(M_SIZE szBytes,bool bWrite=false);

  static M_FILENAME stripPath(M_FILENAME ExtPath)
  {
#if _MDF4_WIN
    M_FILENAME e=_tcsrchr(ExtPath,_T('\\'));
    if (e==NULL)
      e=_tcsrchr(ExtPath,_T(':'));
#else
    M_FILENAME e=strrchr(ExtPath,_T('\\'));
    if (e==NULL)
      e=strrchr(ExtPath,_T(':'));
#endif
    if (e) ExtPath=e+1;
    return ExtPath;
  }
  static bool DeleteFile(M_FILENAME Path)
  {
#if _MDF4_WIN
    return ::DeleteFile(Path)!=FALSE;
#else
    return unlink(Path)==0;
#endif
  }

  M_LINK GetSize(void) { return m_Size; }
	M_INT64 GetPos(void) { return m_cp; }
	void SetEnd(M_INT64 end);
protected:
  bool Assign(
#if _MDF4_WIN
		HANDLE
#else
		FILE *
#endif
		hFile)
  {
    Close();
    if ((m_hFile=hFile)!=INVALID_HANDLE_VALUE)
    {
#if _MDF4_WIN
      LARGE_INTEGER fSize;
      GetFileSizeEx(m_hFile,&fSize);
      m_Size=fSize.QuadPart;
#else
      M_LINK pos = ftello64(m_hFile);
      fseeko64(m_hFile, 0, SEEK_END);
      m_Size=ftello64(m_hFile);
      fseeko64(m_hFile, pos, SEEK_SET);
#endif
      return true;
    }
    return false;
  }
  void Position(M_LINK dwTo);

protected:
#if _MDF4_WIN
  HANDLE m_hFile;
#else
	FILE *m_hFile;
#endif
  M_LINK m_cp;
  M_LINK m_Size;
};
