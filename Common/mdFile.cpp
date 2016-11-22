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
#include "stdafx.h"
#include "mdFile.h"
#include <stdio.h>
#ifdef WIN32
#include <io.h>
#else
#include <sys/io.h>
#include <unistd.h>
#endif
#if !_MDF4_WIN
  #define ATLTRACE
  #define TRACE
#endif


//-----------------------------------------------------------------
// class mDirectFile
// Note: Multithreaded support?
//-----------------------------------------------------------------
BOOL mDirectFile::ReadAt(M_LINK lAt,M_UINT32 szBytes,void * Into)
{
  if (isOpen())
  {
    Position(lAt);
#if _MDF4_WIN
#ifdef _DEBUG
		M_UINT32 dwRead;
		BOOL bSuccess = ReadFile(m_hFile,Into,szBytes,&dwRead,NULL);
		if (!bSuccess)
		{
			M_UINT32 dwError = GetLastError();
			TRACE("ReadFile() returned error %X\n",dwError);
		}
		if (dwRead==szBytes)
#else
    if (ReadFile(m_hFile,Into,szBytes,&dwRead,NULL) && dwRead==szBytes)
#endif
#else
    if (fread(Into,1,szBytes,m_hFile)==szBytes)
#endif
    {
      m_cp+=szBytes;
      return TRUE;
    }
  }
  return FALSE;
}
BOOL mDirectFile::WriteAt(M_LINK lAt,M_UINT32 szBytes,const void * Data)
{
  if (isOpen())
  {
    Position(lAt);
#if _MDF4_WIN
    M_UINT32 dwWritten;
    if (WriteFile(m_hFile,Data,szBytes,&dwWritten,NULL) && dwWritten==szBytes)
#else
		if (fwrite( Data, 1, szBytes, m_hFile)==szBytes)
#endif
    {
      m_cp+=szBytes;
      if (m_cp>m_Size) m_Size=m_cp;
      return TRUE;
    }
  }
  return FALSE;
}

M_LINK mDirectFile::Request(M_SIZE szBytes,bool bWrite/*=false*/)
{
  M_LINK lEnd=m_Size;
	lEnd = (lEnd+7)/8;
	lEnd *= 8;
	szBytes += lEnd - m_Size;
  if (bWrite)
  {
#if _MDF4_WIN
    LARGE_INTEGER sr;
    memcpy(&sr,&szBytes,sizeof(sr));
    SetFilePointerEx(m_hFile,sr,&sr,FILE_END);
    SetEndOfFile(m_hFile);
    memcpy(&m_Size,&sr,sizeof(sr));
#else
		m_Size = szBytes + m_Size;
#ifdef WIN32
		fseeko64(m_hFile, m_Size, SEEK_SET);
		_chsize_s(_fileno(m_hFile), m_Size);
#else
		ftruncate64(fileno(m_hFile), m_Size);
#endif
#endif
    m_cp=m_Size;
  }
  else
  {
    m_Size+=szBytes;
  }
  return lEnd;
}
void mDirectFile::Position(M_LINK dwTo)
{
  if (isOpen() /* && m_cp!=dwTo */)
  {
    m_cp=dwTo;
 #if _MDF4_WIN
    LARGE_INTEGER sr, np;
    memcpy(&sr,&dwTo,sizeof(sr));
   BOOL bSuccess = SetFilePointerEx(m_hFile,sr,&np,FILE_BEGIN);
#else
    BOOL bSuccess = fseeko64(m_hFile, dwTo,SEEK_SET) == 0;
#endif
		if (!bSuccess)
		{
#if _MDF4_WIN
			M_UINT32 dwError = GetLastError();
			TRACE("SetFilePointerEx() returned error %X\n",dwError);
#endif
		}
  }
}
void mDirectFile::SetEnd(M_INT64 end)
{
 #if _MDF4_WIN
  LARGE_INTEGER sr;
  memcpy(&sr,&end,sizeof(sr));
  SetFilePointerEx(m_hFile,sr,&sr,FILE_END);
  SetEndOfFile(m_hFile);
  memcpy(&m_Size,&sr,sizeof(sr));
#else
	m_Size = end;
#ifdef WIN32
	fseeko64(m_hFile, end, SEEK_SET);
	_chsize_s(_fileno(m_hFile), end);
#else
	ftruncate64(fileno(m_hFile), end);
#endif
#endif
  m_cp=m_Size;
}
