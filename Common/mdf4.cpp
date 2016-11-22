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
#include "mdf4.h"
#if _MDF4_WIN
  #include <ppl.h>
#include <vector>
#endif
#include <math.h>

#define USE_MINIZLIB 
#ifdef USE_MINIZLIB
#pragma optimize( "gt", on )
#include "miniz.c"
#pragma optimize( "gt", off )
#else
#include ".\zlib14\zlib.h"
#endif

M_UNICODE MDF4LibGetVersionString()
{
	return MDF4LIB_VERSION;
}

//-----------------------------------------------------------------
// class m4Block
//-----------------------------------------------------------------
// BOOL WriteHdr and FixedPart
// - write the Header (common Header+LinkSection) to disk file
// returns @ of data section (0 on error)
//-----------------------------------------------------------------
M_LINK m4Block::WriteHdr()
{
  M_LINK At=0;
  if (m_File && m_At)
  {
    // determine size of data section
    At=m_At;
    M_SIZE szData=getSize();
    M_UINT32 szFixed=getFixedSize();
    // Fill in Header section
    m_Hdr.hdr_len=sizeof(m4BlockHdr)+szData+szFixed;
    m_Hdr.hdr_links=m_Links.getSize();
    m_Hdr.hdr_len+=m_Links.getBytes();
    ATLTRACE("WR (%8I64X) %I64d: %s %I64d=%I64d\n",At,At,&m_Hdr.hdr_hdr,m_Hdr.hdr_len,At+m_Hdr.hdr_len);
    if (m_File->WriteAt(At,sizeof(m_Hdr),&m_Hdr))
    {
      // Write the link section
      At+=sizeof(m_Hdr);
      if (m_Hdr.hdr_links)
      {
        M_UINT32 szLinks=m_Links.getBytes();
        if (m_File->WriteAt(At,szLinks,m_Links.get()))
          At+=szLinks;
        else
          At=0;
      }
      if (At && szFixed)
      {
        //ATLTRACE("  W %I64d:%u=%I64d\n",At,szFixed,At+szFixed);
        if (m_File->WriteAt(At,szFixed,getFixedPart()))
          At+=szFixed;
        else
          At=0;
      }
    }
  }
  return At;
}

//-----------------------------------------------------------------
// BOOL Write
// - write the whole block to disk file
// return s TRUE on success, FALSE on error
//-----------------------------------------------------------------
BOOL m4Block::Save(bool bVar,bool bFixed)
{
  M_LINK At;
  M_UINT64 szData=getSize();
  M_UINT64 szRemain=szData;
  if (bFixed)
    At=WriteHdr();
  else
    At=m_At+sizeof(m_Hdr)+m_Links.getBytes()+getFixedSize();
  if (At && bVar)
  {
    M_UINT64 szData=getSize();
    // Write the data parts (until getData() returns NULL
    for(int Index=0; ; Index++)
    {
      M_SIZE szRemain=szData;
      PVOID pdata=getData(Index,szRemain);
      if (pdata==NULL || szRemain==0) 
        return TRUE;
      if (!m_File->WriteAt(At,(M_UINT32)szRemain,pdata))
        return FALSE;
      if ((szData-=szRemain)==0) 
        return TRUE;
      At+=szRemain;
    }
  }
  return At!=0;
}
//-----------------------------------------------------------------
// M_LINK Create(m4DirectFile *f,bool bWrite=false)
// - reserve space for the whole block and optionally write to disk
// returns the address (LINK) of the allocated block in the file
//-----------------------------------------------------------------
M_LINK m4Block::Create(mDirectFile *f,int bWrite /*=0*/)
{
  if (f)
  {
    assert(m_File==NULL || m_File==f);
    m_File=f;
  }
  if (m_File)
  {
    if (m_At==0)
    {
      M_SIZE sz=sizeof(m4BlockHdr)+m_Links.getBytes();
      sz+=getFixedSize()+getSize();
      m_At=m_File->Request(sz);
    }
    if (m_At && bWrite)
      Save((bWrite&2)!=0,(bWrite&1)!=0);
    return m_At;
  }
  return 0;
}

//--------------------------------------------------------------------------
// ##HD Header Block
//--------------------------------------------------------------------------
M4FHBlock *M4HDBlock::addHistory(M4FHBlock *fh)
{
  if (fh==NULL)
  {
    delete m_fhNext;
    m_fhNext=NULL;
  }
  else
  {
    assert(fh->hdrID()==M4ID_FH);
    M_LINK fhAt=fh->Create(m_File,3);
    if (fhAt)
    {
      if (m_fhNext)
      {
        m_fhNext->setLink(M4FHBlock::fh_fh_next,fhAt);
        m_fhNext->WriteHdr();
        delete m_fhNext;
      }
      else
      {
        setLink(hd_fh_first,fhAt);
      }
      m_fhNext=fh;
    }
  }
  return fh;
}

M4EVBlock *M4HDBlock::addEvent(M4EVBlock *ev)
{
  if (ev==NULL)
  {
    delete m_evNext;
    m_evNext=NULL;
  }
  else
  {
    assert(ev->hdrID()==M4ID_EV);
    M_LINK evAt=ev->Create(m_File,3);
    if (evAt)
    {
      if (m_evNext)
      {
        m_evNext->setLink(M4EVBlock::ev_ev_next,evAt);
        m_evNext->WriteHdr();
        delete m_evNext;
		    m_evNext=NULL;
      }
      else
      {
        setLink(hd_ev_first,evAt);
      }
      m_evNext=ev;
    }
  }
  return ev;
}

M4ATBlock *M4HDBlock::addAttachment(M4ATBlock *at)
{
  if (at==NULL)
  {
    delete m_atNext;
    m_atNext=NULL;
  }
  else
  {
    assert(at->hdrID()==M4ID_AT);
    M_LINK atAt=at->Create(m_File,3);
    if (atAt)
    {
      if (m_atNext)
      {
        m_atNext->setLink(M4ATBlock::at_at_next,atAt);
        m_atNext->WriteHdr();
        delete m_atNext;
      }
      else
      {
        setLink(hd_at_first,atAt);
      }
      m_atNext=at;
    }
  }
  return at;
}
M4DGBlock *M4HDBlock::addDataGroup(M4DGBlock *dg)
{
  if (dg==NULL)
  {
    delete m_dgNext;
    m_dgNext=NULL;
  }
  else
  {
    assert(dg->hdrID()==M4ID_DG);
    M_LINK dgAt=dg->Create(m_File,3);
    if (dgAt)
    {
      if (m_dgNext)
      {
        m_dgNext->setLink(M4DGBlock::dg_dg_next,dgAt);
        m_dgNext->WriteHdr();
        delete m_dgNext;
      }
      else
      {
        setLink(hd_dg_first,dgAt);
      }
      m_dgNext=dg;
    }
  }
  return dg;
}
void M4HDBlock::Prepare()
{
  addAttachment(NULL);
  addHistory(NULL);
  addDataGroup(NULL);
  addEvent(NULL);
  m_bPrepared=true;
}
//--------------------------------------------------------------------------
// FH FileHistory Block
//--------------------------------------------------------------------------
M4FHBlock::M4FHBlock(MDF4File *File) // Create and insert the time
{
#if _MDF4_WIN     // [B] WIN Support
  M_DATE::now(fh_time);
#else             // [e] WIN Support  
//#  error routine to fill M_DATE with current time
#endif            // [E] WIN Support 
  if (File)
    Create(File);
}
M4FHBlock::M4FHBlock()
{
}
BOOL M4FHBlock::setComment(m4Block &md)
{
  // cannot call twice
  if (!hasLink(fh_md_comment))
  {
    assert(md.hdrID()==M4ID_MD);
    assert(m_File);
    M_LINK mdAt=md.Create(m_File,3);
    if (mdAt)
    {
      setLink(fh_md_comment,mdAt);
      return TRUE;
    }
  }
  return FALSE;
}

//-----------------------------------------------------
// ##AT Attachment
//-----------------------------------------------------
M4ATBlock::M4ATBlock(mDirectFile *f,M_UINT16 creatorIndex)
{
  m_File=f;
  at_creator_index=creatorIndex;
}
BOOL M4ATBlock::LinkFile(M_FILENAME ExtPath,bool bPath/*=true*/,bool bMD5 /*=false*/)
{
  mDirectFile dFile;
  if (!dFile.OpenRead(ExtPath)) return FALSE;
  at_original_size=dFile.AtEnd();
#if _MDF4_MD5       // [B]---- MD5  Support
  // calculate the md5 checksum
  if (bMD5)
  {
    MD5Context md5;
    M_UINT8 Buffer[4096];

    M_SIZE nBytes=at_original_size;
    M_LINK At=0;
    while (nBytes)
    {
      M_UINT32 uiBytes=nBytes>4096 ? 4096 : (M_UINT32)nBytes;
      dFile.ReadAt(At,uiBytes,Buffer);
      md5.Update(Buffer,uiBytes);
      nBytes-=uiBytes;
      At+=uiBytes;
    }
    md5.Final(at_md5_checksum);
    at_flags|=AT_FL_CHECKSUM;
  }
#endif            // [E]---- MD5  Support
  dFile.Close();
  if (!bPath)
  {
    ExtPath=mDirectFile::stripPath(ExtPath);
  }
  M_LINK el;
#if _MDF4_UTF8    // [B]---- UTF8  Support   
  M4TXBlock ep(ExtPath);
  el=ep.Create(m_File,3);
#elif _MDF4_WIN
  _bstr_t aPath(ExtPath);
  M4TXBlock ep(aPath);
  el=ep.Create(m_File,3);
#else             // [e]---- UTF8  Support   
#  error UT8 support without WIN _bstr_t
#endif           // [E]---- UTF8  Support    
  setLink(at_tx_filename,el);
  Create(m_File,3);
  return TRUE;
}
BOOL M4ATBlock::EmbedFile(M_FILENAME ExtPath,bool bZip/*=false*/,bool bMD5/*=false*/)
{
  mDirectFile fd;
  if (!fd.OpenRead(ExtPath))
    return FALSE;
  if (Open(bZip,bMD5))
  {
    M_UINT64 FileSize=fd.AtEnd();
    M_LINK   FilePos=0;
    M_UINT8 TmpBuffer[4096];
    while (FileSize)
    {
      M_UINT32 uiBytes=FileSize>4096 ? 4096 : (M_UINT32)FileSize;
      if (!fd.ReadAt(FilePos,uiBytes,TmpBuffer))
        return FALSE;
      if (!Write(TmpBuffer,uiBytes))
        return FALSE;
      FileSize-=uiBytes;
      FilePos+=uiBytes;
    }
    Close();
    fd.Close();
    return TRUE;
  }
  fd.Close();
  return FALSE;
}
BOOL M4ATBlock::Open(bool bZip/*=false*/,bool bMD5/*=false*/)
{
  assert(m_At==0);
  assert(at_original_size==0);
  if (bMD5) at_flags|=AT_FL_CHECKSUM;
  if (Create(m_File,0))
  {
    m_embeddedPos=WriteHdr();
    return TRUE;
  }
  return FALSE;
}
BOOL M4ATBlock::Write(void *Data,M_UINT32 Size)
{
#if _MDF4_MD5      // [B]---- MD5  Support   
  if (at_flags&AT_FL_CHECKSUM)
    m_md5.Update((unsigned char *)Data,Size);
#endif            // [E]---- MD5  Support    
  if (m_File->WriteAt(m_embeddedPos,Size,Data))
  {
    m_embeddedPos+=Size;
    at_embedded_size+=Size;
    return TRUE;
  }
  return FALSE;
}
void M4ATBlock::Close()
{
  at_original_size=at_embedded_size;
#if _MDF4_MD5   // [B]---- MD5  Support   
  if (at_flags&AT_FL_CHECKSUM)
    m_md5.Final(at_md5_checksum);
#endif          // [B]---- MD5  Support   
  at_flags|=AT_FL_EMBEDDED;
  Save(false);
}

//--------------------------------------------------------------------------
// ##DG DataGroup
//--------------------------------------------------------------------------
M4CGBlock *M4DGBlock::addChannelGroup(M4CGBlock *cg)
{
  if (cg==NULL)
  {
    cg_delete();
    Save();
  }
  else
  {
    assert(cg->hdrID()==M4ID_CG);
    M_LINK cgAt=cg->Create(m_File,3);
    if (cgAt)
    {
      if (m_cgNext)
      {
        m_cgNext->setLink(M4CGBlock::cg_cg_next,cgAt);
        m_cgNext->WriteHdr();
        cg_delete();
      }
      else
      {
        setLink(dg_cg_first,cgAt);
      }
      m_cgNext=cg;
    }
  }
  return cg;
}
void M4DGBlock::cg_delete()
{
  if (m_cgNext)
  {
    delete m_cgNext;
    m_cgNext=NULL;
  }
}


//--------------------------------------------------------------------------
// ##CG ChannelGroup
//--------------------------------------------------------------------------

M4CNBlock *M4CGBlock::addChannel(M4CNBlock *cn, BOOL bNoDelParent/*=FALSE*/)
{
  if (cn)
  {
    assert(cn->hdrID()==M4ID_CN);
    M_LINK cnAt=cn->Create(m_File,3);
    if (cnAt)
    {
      if (m_cnNext)
      {
        m_cnNext->setLink(M4CNBlock::cn_cn_next,cnAt);
        m_cnNext->WriteHdr();
        if(!bNoDelParent)
          delete m_cnNext;
      }
      else
      {
        setLink(cg_cn_first,cnAt);
      }
      m_cnNext=cn;
    }
  }
  return cn;
}

#if _MDF4_XML      // [B]---- MSXML  Support   
//--------------------------------------------------------------------------
// ##MD Meta Data Block w. XML support
//--------------------------------------------------------------------------
M4MDBlock::M4MDBlock(M_UNICODE rootName,M_UNICODE txText)
{
  m_Hdr.hdr_id=M4ID_MD;
  // Create the XML DOM Document object
  if (CreateInstance())
  {
    try
    {
      // Create the root node
		  m_Doc->appendChild(m_Doc->createElement(rootName));
      // add the (required) <TX> element
      element(L"TX")->text=txText;
    }
    catch(...)
    {
      assert(FALSE);
      m_Doc=NULL;
    }
  }
}

MSXML2::IXMLDOMNodePtr M4MDBlock::element(M_UNICODE elemName)
{
  MSXML2::IXMLDOMNodePtr child;
  try
  {
    child=m_Doc->documentElement;
    child=child->selectSingleNode(elemName);
    if (child)
    {
    }
    else
    {
      child=m_Doc->createElement(elemName);
      m_Doc->documentElement->appendChild(child);
    }
  }
  catch(...)
  {
  }
  return child;
}
MSXML2::IXMLDOMNodePtr M4MDBlock::createNode(M_UNICODE elemName)
{
  MSXML2::IXMLDOMNodePtr child;
  try
  {
    child=m_Doc->createElement(elemName);
  }
  catch(...)
  {
  }
  return child;
}
void M4MDBlock::closeXML()
{
  if (m_Doc)
  {
    Append((M_UNICODE)m_Doc->xml);
    m_Doc=NULL;
  }
}
void M4MDBlock::md_element(M_UNICODE elemName,M_UNICODE elemValue,M_UNICODE elemParent/*=NULL*/)
{
  MSXML2::IXMLDOMNodePtr child;
  try
  {
    if (elemParent)
    {
      MSXML2::IXMLDOMNodePtr parent;
      parent=element(elemParent);
      child=m_Doc->createElement(elemName);
      parent->appendChild(child);
    }
    else
    {
      child=element(elemName);
    }
    child->text=elemValue;
  }
  catch(...)
  {
  }
}

//----------------------------------------------
// MD for HD blocks (XML: HDComment)
//----------------------------------------------
M4HDComment::M4HDComment(M_UNICODE txText) : M4MDBlock(L"HDcomment",txText)
{
}
void M4HDComment::setTimeSource(M_UNICODE ts)
{
  MSXML2::IXMLDOMNodePtr c;
  try
  {
    c=element(L"time_source");
    c->text=ts;
  }
  catch(...) {}
}

//-------------------------------------------------------------------------
// M4FHComment - a MD Block for FH.fh_md_comment
//-------------------------------------------------------------------------
M4FHComment::M4FHComment(M_UNICODE txText) : M4MDBlock(L"FHcomment",txText)
{
}

void M4FHComment::fh_element(M_UNICODE elemName,M_UNICODE elemValue)
{
  try
  {
    element(elemName)->text=elemValue;
  }
  catch(...) {}
}

void M4FHComment::closeXML()
{
  if (m_Doc)
  {
		common_element(L"Free_MDF4_Lib",MDF4LibGetVersionString());
    Append((M_UNICODE)m_Doc->xml);
    m_Doc=NULL;
  }
}

//-------------------------------------------------------------------------
// CreateInstance - create a MSXML Parser object
// Return           : TRUE on Success
//-------------------------------------------------------------------------
BOOL M4MDBlock::CreateInstance(void)
{
  HRESULT hr = S_FALSE;
  hr = (hr == S_OK) ? hr : m_Doc.CreateInstance( __uuidof(MSXML2::DOMDocument60) );
  hr = (hr == S_OK) ? hr : m_Doc.CreateInstance( __uuidof(MSXML2::DOMDocument30) );
  hr = (hr == S_OK) ? hr : m_Doc.CreateInstance( __uuidof(MSXML2::DOMDocument40) );
  hr = (hr == S_OK) ? hr : m_Doc.CreateInstance( __uuidof(MSXML2::DOMDocument26) );
  hr = (hr == S_OK) ? hr : m_Doc.CreateInstance( __uuidof(MSXML2::DOMDocument) );
  if( hr != S_OK ) 
  {
    // did u CoInitialize ?
    assert(FALSE);
    return FALSE;
  }
  m_Doc->setProperty( _bstr_t(_T("SelectionLanguage")), _variant_t(_T("XPath")) );	// 3.0 only
  m_Doc->setProperty( _bstr_t(_T("AllowXsltScript")), _variant_t(true));
  m_Doc->setProperty( _bstr_t(_T("AllowDocumentFunction")), _variant_t(true));
  m_Doc->resolveExternals = VARIANT_TRUE; 
  m_Doc->preserveWhiteSpace = VARIANT_FALSE;
  m_Doc->validateOnParse = VARIANT_FALSE;
  return TRUE;
}
MSXML2::IXMLDOMNodePtr M4MDBlock::common_element(M_UNICODE elemName,M_UNICODE elemValue)
{
  MSXML2::IXMLDOMNodePtr c;
  MSXML2::IXMLDOMElementPtr e;
  try
  {
    c=common();
    e=m_Doc->createElement(_T("e"));
    e->text=elemValue;
    e->setAttribute(_T("name"),elemName);
    c->appendChild(e);
  }
  catch(...)
  {
  }

  return e;
}

#endif         // [E]---- MSXML  Support   

//--------------------------------------------------------------------------
// MDF4File
//--------------------------------------------------------------------------
MDF4File::MDF4File()
{
}
MDF4File::~MDF4File()
{
  Close();
}
void MDF4File::Close()
{
  m_Hdr.Close();
#if _MDF4_WIN
  __super::Close();
#else
  mDirectFile::Close();
#endif
}

BOOL MDF4File::Create(M_FILENAME strPathName, const char *strProducer /*=0*/, int iVersion /* = 400 */)
{
  assert(sizeof(m_Id)==64);
  memset(&m_Id,0,sizeof(m_Id));
  if (OpenCreate(strPathName))
  {
    char tVersion[20];
    // fill the Id Block
    m3ccopy(m_Id.id_file,8,"MDF",true);
#if _MDF4_WIN
    sprintf_s(tVersion,20,"%d.%02d",iVersion/100, iVersion % 100);
#else
    sprintf(tVersion,"%d.%02d",iVersion/100, iVersion % 100);
#endif
    m3ccopy(m_Id.id_vers,8,tVersion,true);
    if (strProducer)
      m3acopy(m_Id.id_prog,strProducer,true);
    else
      m3acopy(m_Id.id_prog,"mdflib",true);
    m_Id.id_ver=iVersion;
    if (WriteAt(0,sizeof(m_Id),&m_Id))
    {
      // initialize & create the HD Block
      if (m_Hdr.Create(this,3))
      {
        return TRUE;
      }
    }
    Close();
    DeleteFile(strPathName);
  }
  return FALSE;
}

#if _MDF4_WIN        // [B] WIN Support
// TimeDifference FILETIME(1.1.1970)-FILETIME(0)
#define FT19700101 116444736000000000LL
void M_DATE::set(M_DATE &d,const FILETIME &ft)
{
  // FILETIME is in 100-nanoseconds since January 1, 1601 (UTC).
  memcpy(&d.time_ns,&ft,sizeof(__int64));
  d.time_ns=(d.time_ns-FT19700101)*100;
  // ft is UTC-Time; determine TimeZone offset
  TIME_ZONE_INFORMATION ZoneInfo;
  DWORD zis=GetTimeZoneInformation(&ZoneInfo);
  if (zis==TIME_ZONE_ID_UNKNOWN)
     d.time_flags&=~HD_TF_TIMEOFFS;
  else
  {
    d.time_flags|=HD_TF_TIMEOFFS;
    d.tz_offset_min=(M_INT16)(-ZoneInfo.Bias);
    d.dst_offset_min=0;
    if (zis==TIME_ZONE_ID_DAYLIGHT)
    {
			tm t1;
			t1.tm_isdst = 0;
			try
			{
				CTime t(ft);
				t.GetLocalTm(&t1);
			}
			catch(...)
			{
			}
			if (t1.tm_isdst)
	      d.dst_offset_min=(M_INT16)(-ZoneInfo.DaylightBias);
    }
  }
}
void M_DATE::get(const M_DATE &d,FILETIME &ft)
{
	M_UINT64 ui64Time = d.time_ns;
  // Interpret flags 
	if (d.time_flags & HD_TF_TIMEOFFS)
	{
		ui64Time += (M_UINT64)d.tz_offset_min * 60 * 1000000000;
		ui64Time += (M_UINT64)d.dst_offset_min * 60 * 1000000000;
	}
  unsigned __int64 ct = ui64Time/100+FT19700101;
  // FILETIME is in 100-nanoseconds since January 1, 1601 (UTC).
  memcpy(&ft,&ct,sizeof(__int64));
}
void M_DATE::get(const M_DATE &d,SYSTEMTIME &st)
{
  FILETIME ft;
  get(d,ft);
  FileTimeToSystemTime(&ft,&st);
}
void M_DATE::now(M_DATE &d)
{
  FILETIME ft;
  SYSTEMTIME st;
  GetSystemTime(&st);
  SystemTimeToFileTime(&st,&ft);
  set(d,ft);
}

void MDF4File::setFileTime(const FILETIME &ft)
{
  M_DATE::set(m_Hdr.hd_start_time,ft);
  m_Hdr.Save(false);
}
void MDF4File::setFileTime()
{
  M_DATE::now(m_Hdr.hd_start_time);
  m_Hdr.Save(false);
}

#endif

#if !_MDF4_WONLY          // [B]---- READ  Support   
BOOL m4Block::Read(mDirectFile *File,M_LINK At,const m4BlockHdr &h)
{
  //assert(m_File==NULL);
  m_File=File; m_At=At;
  m_Hdr=h;
  // common Header already read
  M_SIZE szRemain=m_Hdr.hdr_len;
  At+=sizeof(m_Hdr);
  szRemain-=sizeof(m_Hdr);
  if (m_Hdr.hdr_links)
  {
    M_UINT32 szLinks=(M_UINT32)m_Hdr.hdr_links*sizeof(M_LINK);
    PVOID pLinks=m_Links.resize(szLinks);
    if (!m_File->ReadAt(At,szLinks,pLinks))
      return FALSE;
    At+=szLinks;
    szRemain-=szLinks;
  }
  // call virtual function to read content
  return readData(At,szRemain);
}

BOOL M4HDBlock::Load(mDirectFile *f)
{
  M_LINK At=0x40; // Fixed Position
  m4BlockHdr h;
  m_bPrepared=false; // to be sure
  if (f->ReadAt(At,sizeof(m4BlockHdr),&h))
  {
    if (h.hdr_id==M4ID_HD && h.hdr_hdr==M4ID_ID)
    {
      return Read(f,0x40,h);
    }
  }
  return FALSE;
}

BOOL MDF4File::Open(M_FILENAME strPathName, BOOL bUpdate)
{
  assert(sizeof(m_Id)==64);
	bool bResult;
	if (bUpdate)
		bResult = OpenUpdate(strPathName);
	else 
		bResult = OpenRead(strPathName);
  if (bResult)
  {
    // read the Id Block
    if (ReadAt(0,sizeof(m_Id),&m_Id))
    {
      if (memcmp(m_Id.id_file,"MDF     ",8)==0 && m_Id.id_ver>=400)
      {
        if (m_Hdr.Load(this))
        {
          return TRUE;
        }
      }
    }
    Close();
  }
  return FALSE;
}
bool MDF4File::LoadBlkHdr(M_LINK At,m4BlockHdr &h)
{
  if (At && ReadAt(At,sizeof(h),&h))
  {
    if (h.hdr_hdr!=M4ID_ID)
			ATLTRACE("LoadBlkHdr @0x%I64X: h.hdr_hdr!=M4ID_ID\n",At);
    return (h.hdr_hdr==M4ID_ID);
  }
  return false;
}

m4Block *MDF4File::LoadBlock(M_LINK At)
{
  m4BlockHdr h;
  m4Block *b=NULL;
  if (At && ReadAt(At,sizeof(h),&h))
  {
    assert(h.hdr_hdr==M4ID_ID);
    if (h.hdr_hdr==M4ID_ID) switch (h.hdr_id)
    {
    case M4ID_TX: b=new M4TXBlock; break;
    case M4ID_MD: b=new M4MDBlock; break;
    case M4ID_FH: b=new M4FHBlock; break;
    case M4ID_CH: b=new M4CHBlock; break;
    case M4ID_AT: b=new M4ATBlock; break;
    case M4ID_EV: b=new M4EVBlock; break;
    case M4ID_DG: b=new M4DGBlock; break;
    case M4ID_CG: b=new M4CGBlock; break;
    case M4ID_SI: b=new M4SIBlock; break;
    case M4ID_CN: b=new M4CNBlock; break;
    case M4ID_CC: b=new M4CCBlock; break;
    case M4ID_CA: b=new M4CABlock; break;
    case M4ID_DT: b=new M4DTBlock; break;
    case M4ID_SR: b=new M4SRBlock; break;
    case M4ID_RD: b=new M4RDBlock; break;
    case M4ID_SD: b=new M4SDBlock; break;
    case M4ID_DL: b=new M4DLBlock; break;
    case M4ID_DZ: b=new M4DZBlock; break;
    case M4ID_HL: b=new M4HLBlock; break;
    default:
         assert(FALSE);
    }
    if (b)
    {
      if (b->Read(this,At,h))
        return b;
      delete b;
      b=NULL;
    }
  }
  return NULL;
}
#if 0
m4Block *MDF4File::LoadLink(m4Block &parent,int linkNo)
{
  return LoadBlock(parent.getLink(linkNo));
}
#endif
m4Block *MDF4File::LoadLink(m4Block &parent,int linkNo,M_UINT16 id)
{
	m4Block *p = LoadBlock(parent.getLink(linkNo));
	if (p && id && p->hdrID()!=id)
	{
#ifdef _DEBUG
		char c1,c2,d1,d2;
		c1 = id&0xFF;
		c2 = id>>8;
		d1 = p->hdrID()&0xFF;
		d2 = p->hdrID()>>8;
		ATLTRACE("Expected block type ##%c%c is actually ##%c%c\n",c1,c2,d1,d2);
#endif
		delete p;
		p = NULL;
	}
  return p;
}
#endif  // [E]---- READ  Support    

//////////////////////////////////////////////////////////////////////////////////////////
// Helpers for SR-Blocks
//
  /* Signed-Unsigned Wandlungstabellen */
  static M_UINT32 signed_max[33] = {
    0x00000000L, /* Gibts nicht */
    0x00000001UL-1UL,  0x00000002UL-1UL,  0x00000004UL-1UL,  0x00000008UL-1UL,  //lint !e778 (Info -- Constant expression evaluates to 0 in operation '-')
    0x00000010UL-1UL,  0x00000020UL-1UL,  0x00000040UL-1UL,  0x00000080UL-1UL,
    0x00000100UL-1UL,  0x00000200UL-1UL,  0x00000400UL-1UL,  0x00000800UL-1UL,
    0x00001000UL-1UL,  0x00002000UL-1UL,  0x00004000UL-1UL,  0x00008000UL-1UL,
    0x00010000UL-1UL,  0x00020000UL-1UL,  0x00040000UL-1UL,  0x00080000UL-1UL,
    0x00100000UL-1UL,  0x00200000UL-1UL,  0x00400000UL-1UL,  0x00800000UL-1UL,
    0x01000000UL-1UL,  0x02000000UL-1UL,  0x04000000UL-1UL,  0x08000000UL-1UL,
    0x10000000UL-1UL,  0x20000000UL-1UL,  0x40000000UL-1UL,  0x80000000UL-1UL
  };

M_UINT32 MDF4File::get_signed_max(M_INT32 idx)
{
	if (idx >= 0 && idx < 33)
		return signed_max[idx];
	return 0;
}
  static M_UINT32 signed_sub[33] = {
    0x00000000UL, /* Gibts nicht */
    0x00000002UL,  0x00000004UL,  0x00000008UL,  0x00000010UL,
    0x00000020UL,  0x00000040UL,  0x00000080UL,  0x00000100UL,
    0x00000200UL,  0x00000400UL,  0x00000800UL,  0x00001000UL,
    0x00002000UL,  0x00004000UL,  0x00008000UL,  0x00010000UL,
    0x00020000UL,  0x00040000UL,  0x00080000UL,  0x00100000UL,
    0x00200000UL,  0x00400000UL,  0x00800000UL,  0x01000000UL,
    0x02000000UL,  0x04000000UL,  0x08000000UL,  0x10000000UL,
    0x20000000UL,  0x40000000UL,  0x80000000UL,  0x00000000UL
  };
M_UINT32 MDF4File::get_signed_sub(M_INT32 idx)
{
	if (idx >= 0 && idx < 33)
		return signed_sub[idx];
	return 0;
}
/*
  static BYTE byte_mask[9] = {
    0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,
    0xff
  };
*/
  static M_UINT32 dword_mask[33] = {
    0x00000000L,0x00000001L,0x00000003L,0x00000007L,0x0000000fL,0x0000001fL,0x0000003fL,0x0000007fL,
    0x000000ffL,0x000001ffL,0x000003ffL,0x000007ffL,0x00000fffL,0x00001fffL,0x00003fffL,0x00007fffL,
    0x0000ffffL,0x0001ffffL,0x0003ffffL,0x0007ffffL,0x000fffffL,0x001fffffL,0x003fffffL,0x007fffffL,
    0x00ffffffL,0x01ffffffL,0x03ffffffL,0x07ffffffL,0x0fffffffL,0x1fffffffL,0x3fffffffL,0x7fffffffL,
    0xffffffffL
  };

M_UINT32 MDF4File::get_dword_mask(M_INT32 idx)
{
	if (idx >= 0 && idx < 33)
		return dword_mask[idx];
	return 0;
}
//lint -e799 (Info -- numerical constant '0x00ffffffffffffff' larger than unsigned long)

  static M_INT64 i64_mask[65] = {
    0x0000000000000000,0x0000000000000001,0x0000000000000003,0x0000000000000007,
    0x000000000000000f,0x000000000000001f,0x000000000000003f,0x000000000000007f,
    0x00000000000000ff,0x00000000000001ff,0x00000000000003ff,0x00000000000007ff,
    0x0000000000000fff,0x0000000000001fff,0x0000000000003fff,0x0000000000007fff,
    0x000000000000ffff,0x000000000001ffff,0x000000000003ffff,0x000000000007ffff,
    0x00000000000fffff,0x00000000001fffff,0x00000000003fffff,0x00000000007fffff,
    0x0000000000ffffff,0x0000000001ffffff,0x0000000003ffffff,0x0000000007ffffff,
    0x000000000fffffff,0x000000001fffffff,0x000000003fffffff,0x000000007fffffff,
    0x00000000ffffffff,0x00000001ffffffff,0x00000003ffffffff,0x00000007ffffffff,
    0x0000000fffffffff,0x0000001fffffffff,0x0000003fffffffff,0x0000007fffffffff,
    0x000000ffffffffff,0x000001ffffffffff,0x000003ffffffffff,0x000007ffffffffff,
    0x00000fffffffffff,0x00001fffffffffff,0x00003fffffffffff,0x00007fffffffffff,
    0x0000ffffffffffff,0x0001ffffffffffff,0x0003ffffffffffff,0x0007ffffffffffff,
    0x000fffffffffffff,0x001fffffffffffff,0x003fffffffffffff,0x007fffffffffffff,
    0x00ffffffffffffff,0x01ffffffffffffff,0x03ffffffffffffff,0x07ffffffffffffff,
    0x0fffffffffffffff,0x1fffffffffffffff,0x3fffffffffffffff,0x7fffffffffffffff,
    0xffffffffffffffff
  };
M_INT64 MDF4File::get_i64_mask(M_INT32 idx)
{
	if (idx >= 0 && idx < 65)
		return i64_mask[idx];
	return 0;
}

M_UINT32 MDF4File::CountChannels(M4CGBlock *cg)
{
	M_UINT32 nChan = 0;
	M4CNBlock *cn = (M4CNBlock *)LoadLink(*cg, M4CGBlock::cg_cn_first);
	while (cn)
	{
		nChan++; 
		M4CNBlock *p = (M4CNBlock *)LoadLink(*cn, M4CNBlock::cn_cn_next, M4ID_CN);
		delete cn;
		cn = p;
	}
	return nChan;
}

M4CNBlock *MDF4File::FindMasterChannel(M4CGBlock *cg, M_UINT32 *pnChan)
{
	M4CNBlock *cnResult=NULL; 
	BOOL bFlag; 
	*pnChan = 0;
	M4CNBlock *cn = (M4CNBlock *)LoadLink(*cg, M4CGBlock::cg_cn_first);
	while (cn)
	{
		(*pnChan)++; 
		if (cn->cn_type == CN_T_MASTER || cn->cn_type == CN_T_VMASTER)
			cnResult = cn, bFlag = TRUE;
		else
			bFlag = FALSE;
		M4CNBlock *p = (M4CNBlock *)LoadLink(*cn, M4CNBlock::cn_cn_next, M4ID_CN);
		if (!bFlag)
			delete cn;
		cn = p;
	}
	return cnResult;
}

void MDF4File::FillThree(M4DGBlock *dg, M4CGBlock *cg, Three *pBuffer)
{
	M4CNBlock *cn = (M4CNBlock *)LoadLink(*cg, M4CGBlock::cg_cn_first);
	while (cn)
	{
	  if (cn->cn_data_type<CN_D_STRING)
		{
			pBuffer->flags.bInteger = cn->cn_data_type<=CN_D_SINT_BE;
			pBuffer->flags.bSigned = cn->cn_data_type>=CN_D_SINT_LE;
			if (cn->cn_type == CN_T_MASTER)
				pBuffer->flags.bMaster = 1;
			else if (cn->cn_type == CN_T_VMASTER)
				pBuffer->flags.bMaster = 2;
			pBuffer->flags.bMotorola = (cn->cn_data_type & 1);
			pBuffer->flags.sbit = cn->cn_bit_offset + cn->cn_byte_offset*8 + dg->dg_rec_id_size;
			pBuffer->flags.nbit = cn->cn_bit_count;
			pBuffer->flags.nbytes = (cn->cn_bit_count+7) >> 3;
			if (cn->cn_flags & CN_F_INVALID)
				pBuffer->iInvalBitPos = cn->cn_inval_bit_pos;
			else
				pBuffer->iInvalBitPos = -1;
		}
		if (pBuffer->flags.bMaster)
			pBuffer->Mean = 1E308;
		else
			pBuffer->Mean = 0;
		pBuffer->Min = 1E308;
		pBuffer->Max = -1E308;
		M4CNBlock *p = (M4CNBlock *)LoadLink(*cn, M4CNBlock::cn_cn_next, M4ID_CN);
		delete cn;
		cn = p;
		pBuffer++;
	}
}

void MDF4File::ResetThree(Three *pBuffer, M_UINT32 nChan)
{
	for (M_UINT32 i=0; i<nChan; i++)
	{
		if (pBuffer->flags.bMaster)
			pBuffer->Mean = 1E308;
		else
			pBuffer->Mean = 0;
		pBuffer->Min = 1E308;
		pBuffer->Max = -1E308;
		pBuffer->n = 0;
		pBuffer->nMiss = 0;
		pBuffer++;
	}
}

double MDF4File::Interpret(M_UINT8 *pRecord, Three *pBuffer, M_UINT32 RecLen, bool bValueOnly/*=false*/)
{
	double val = 1E308;
	M_UINT32 iInvalBitPos = pBuffer->iInvalBitPos;
	if (iInvalBitPos >= 0 && iInvalBitPos != -1) // signed/unsigend!
	{
		M_UINT8 *p = pRecord + RecLen + iInvalBitPos/8;
		if (*p & (1 << (iInvalBitPos%8)))
		{
			pBuffer->n++;
			pBuffer->nMiss++;
			return val; // no value 
		}
	}
	M_UINT32 sbit = pBuffer->flags.sbit & 7;
  pRecord += pBuffer->flags.sbit >> 3; // Byte Offset
	M_INT64 dummy = 0;
	M_UINT8 *ptr = (M_UINT8*)&dummy;
	if (!pBuffer->flags.bInteger)
	{
		if (pBuffer->flags.nbit == 32)
		{
			if (pBuffer->flags.bMotorola)
				MovAndSwap( pRecord, ptr, pBuffer->flags.nbytes);
			else
				*(M_INT32*)ptr = *((M_INT32*)pRecord);
			val = (double)*((float*)ptr);
		}
		else
		{
			if (pBuffer->flags.bMotorola)
				MovAndSwap( pRecord, ptr, pBuffer->flags.nbytes);
			else
				*(double*)ptr = *((double*)pRecord);
			val = *((double*)ptr);
		}
	}
	else if (pBuffer->flags.bInteger && pBuffer->flags.nbit > 32)
	{
		M_INT64 i64Val;
		if (pBuffer->flags.bMotorola)
		{
			MovAndSwap( pRecord, ptr, pBuffer->flags.nbytes);
			i64Val = *((M_INT64*)ptr);
		}
		else if (pBuffer->flags.nbytes < 4)
		{
			memcpy(ptr, pRecord, pBuffer->flags.nbytes);
			i64Val = *((M_INT64*)ptr);
		}
		else
			i64Val = *((M_INT64*)pRecord);
		i64Val = (i64Val>>sbit) & get_i64_mask(pBuffer->flags.nbit);
		val = (double)i64Val;
	}
	else
	{
		if (pBuffer->flags.bMotorola)
			MovAndSwap( pRecord, ptr, pBuffer->flags.nbytes);
		else
			memcpy(ptr, pRecord, pBuffer->flags.nbytes);
		M_UINT32 dwValue = *(M_UINT32*)ptr;
		if (pBuffer->flags.nbit > 16)
		{
			dwValue = (dwValue>>sbit) & get_dword_mask(pBuffer->flags.nbit);
			if (pBuffer->flags.bSigned && dwValue > get_signed_max(pBuffer->flags.nbit)) 
				val = (double)(long)dwValue;
			else 
				val = (double)dwValue;
		}
		else 
		{
			M_UINT16 wValue = (dwValue>>sbit) & (M_UINT16)get_dword_mask(pBuffer->flags.nbit);
			if (pBuffer->flags.bSigned && dwValue > get_signed_max(pBuffer->flags.nbit)) 
			{
				if (pBuffer->flags.nbit <= 8)
					val = (double)(char)dwValue;
				else 
					val = (double)(short)dwValue;
			}
			else
				val = (double)dwValue;
		}
	}
	if (bValueOnly)
		return val;
	if (pBuffer->flags.bMaster == 1)
	{
		if (pBuffer->Mean == 1E308) // not yet set
			pBuffer->Mean = val;
	}
	else if (pBuffer->flags.bMaster == 0) 
	{
		pBuffer->Mean += val;
		if (pBuffer->Min > val)
			pBuffer->Min = val;
		if (pBuffer->Max < val)
			pBuffer->Max = val;
	}
	pBuffer->n++;
	return val;
}

double MDF4File::InterpretSR(M_UINT8 *pRecord, Three *pBuffer, M_UINT32 RecLen, bool bValueOnly/*=false*/)
{
	double value = 1E308;
	double master = 1E308;
	M_UINT32 iInvalBitPos = pBuffer->iInvalBitPos;
	if (iInvalBitPos >= 0 && iInvalBitPos != -1) // signed/unsigend!
	{
		M_UINT8 *p = pRecord + RecLen*3 + iInvalBitPos/8;
		if (*p & (1 << (iInvalBitPos%8)))
		{
			pBuffer->n++;
			pBuffer->nMiss++;
			return value; // no value 
		}
	}
	M_UINT32 sbit = pBuffer->flags.sbit & 7;
	for (int i=0; i<3; i++)
	{
		M_UINT8 *ptr = pRecord+i*RecLen + (pBuffer->flags.sbit >> 3); // Byte Offset
		if (!pBuffer->flags.bInteger)
		{
			if (pBuffer->flags.nbit == 32)
			{
			float val;
			if (pBuffer->flags.bMotorola)
				MovAndSwap( ptr, (M_UINT8*)&val, 4);
			else
				val = *(float*)(ptr);
			value = val;
			}
			else // double
			{
				double val;
				if (pBuffer->flags.bMotorola)
					MovAndSwap( ptr, (M_UINT8*)&val, 8);
				else
					val = *(double*)ptr;
				value = val;
			}
		}
		else if (pBuffer->flags.bInteger && pBuffer->flags.nbit > 32)
		{
			M_INT64 val;
			if (pBuffer->flags.bMotorola)
				MovAndSwap( ptr, (M_UINT8*)&val, 8);
			else
				val = *(M_INT64*)ptr;
			val = (val>>sbit) & get_i64_mask(pBuffer->flags.nbit);
			value = val;
		}
		else if (pBuffer->flags.bSigned)
		{
			M_INT32 val;
			if (pBuffer->flags.bMotorola)
				MovAndSwap( ptr, (M_UINT8*)&val, pBuffer->flags.nbytes);
			else
				val = *(M_INT32*)ptr;
			val = (val>>sbit) & get_dword_mask(pBuffer->flags.nbit);
			if (val > (M_INT32)get_signed_max(pBuffer->flags.nbit)) 
			{
				if (pBuffer->flags.nbit > 16)
					val = get_signed_max(pBuffer->flags.nbit);
				else if (pBuffer->flags.nbit > 8)
					val = (double)(short)val;
				else
					val = (double)(char)val;
			}
			else 
				val = (double)val;
			value = val;
		}
		else // unsigned
		{
			M_UINT32 val;
			if (pBuffer->flags.bMotorola)
				MovAndSwap( ptr, (M_UINT8*)&val, pBuffer->flags.nbytes);
			else
				val = *(M_UINT32*)ptr;
			val = (val>>sbit) & get_dword_mask(pBuffer->flags.nbit);
			value = val;
		}
		if (bValueOnly && i==0)
			return value;
		if (i == 0)
		{
			if (pBuffer->flags.bMaster == 1)
			{
				if (pBuffer->Mean == 1E308) // not yet set
					pBuffer->Mean = value;
				master = value;
			}
			else if (pBuffer->flags.bMaster == 0) 
			{
				pBuffer->Mean += value;
			}
		}
		else if (i == 1)
		{
			if (pBuffer->Min > value)
				pBuffer->Min = value;
		}
		else if (i == 2)
		{
			if (pBuffer->Max < value)
				pBuffer->Max = value;
		}
	}
	pBuffer->n++;
	if (pBuffer->flags.bMaster == 1)
		return master;
	return pBuffer->Mean;
}

void MDF4File::Encode(M_UINT8 *pRecord, Three *pBuffer, M_UINT32 RecLen)
{
	// Record layout:
	// Mean values | Min values | Max values | invalidation bytes
	if (pBuffer->n == pBuffer->nMiss) // no data
	{
		M_UINT32 iInvalBitPos = pBuffer->iInvalBitPos;
		if (iInvalBitPos >= 0 && iInvalBitPos != -1) // signed/unsigend!
		{
			M_UINT8 *p = pRecord + RecLen*3 + iInvalBitPos/8;
			*p |=  (1 << (iInvalBitPos%8));
		}
		return; // no value 
	}
	M_UINT32 sbit = pBuffer->flags.sbit & 7;
  pRecord += pBuffer->flags.sbit >> 3; // Byte Offset
	M_INT64 dummy = 0;
	M_UINT8 *ptr = (M_UINT8*)&dummy;
	if (pBuffer->flags.bMaster == 0 && pBuffer->n>1)
		pBuffer->Mean /= pBuffer->n-pBuffer->nMiss;
	if (!pBuffer->flags.bInteger)
	{
		if (pBuffer->flags.nbit == 32)
		{
			float val;
			for (int i=0; i<3; i++)
			{
				if (i==0) val = (float)pBuffer->Mean;
				else if (i==1) val = (float)pBuffer->Min;
				else if (i==2) val = (float)pBuffer->Max;
				if (pBuffer->flags.bMotorola)
					MovAndSwap( (M_UINT8*)&val, pRecord+i*RecLen, 4);
				else
					*(float*)(pRecord+i*RecLen) = val;
			}
		}
		else
		{
			double val;
			for (int i=0; i<3; i++)
			{
				if (i==0) val = pBuffer->Mean;
				else if (i==1) val = pBuffer->Min;
				else if (i==2) val = pBuffer->Max;
				if (pBuffer->flags.bMotorola)
					MovAndSwap( (M_UINT8*)&val, pRecord+i*RecLen, 8);
				else
					*(double*)(pRecord+i*RecLen) = val;
			}
		}
	}
	else if (pBuffer->flags.bInteger && pBuffer->flags.nbit > 32)
	{
		M_INT64 val;
		for (int i=0; i<3; i++)
		{
			if (i==0) val = (M_INT64)pBuffer->Mean;
			else if (i==1) val = (M_INT64)pBuffer->Min;
			else if (i==2) val = (M_INT64)pBuffer->Max;
			val &= get_i64_mask(pBuffer->flags.nbit);
			val <<= sbit;
			if (pBuffer->flags.bMotorola)
				OrAndSwap( (M_UINT8*)&val, pRecord+i*RecLen, 8);
			else
				*(M_INT64*)(pRecord+i*RecLen) |= val;
		}
	}
	else if (pBuffer->flags.bSigned)
	{
		M_INT32 val;
		for (int i=0; i<3; i++)
		{
			if (i==0) val = (M_INT32)pBuffer->Mean;
			else if (i==1) val = (M_INT32)pBuffer->Min;
			else if (i==2) val = (M_INT32)pBuffer->Max;
			val &= get_dword_mask(pBuffer->flags.nbit);
			val <<= sbit;
			if (pBuffer->flags.bMotorola)
				OrAndSwap( (M_UINT8*)&val, pRecord+i*RecLen, pBuffer->flags.nbytes);
			else
				//*(M_INT64*)(pRecord+i*RecLen) |= val;
				memcpy( pRecord+i*RecLen, &val, pBuffer->flags.nbytes);
		}
	}
	else // unsigned
	{
		M_UINT32 val;
		for (int i=0; i<3; i++)
		{
			if (i==0) val = (M_UINT32)pBuffer->Mean;
			else if (i==1) val = (M_UINT32)pBuffer->Min;
			else if (i==2) val = (M_UINT32)pBuffer->Max;
			val &= get_dword_mask(pBuffer->flags.nbit);
			val <<= sbit;
			if (pBuffer->flags.bMotorola)
				OrAndSwap( (M_UINT8*)&val, pRecord+i*RecLen, pBuffer->flags.nbytes);
			else
				*(M_INT64*)(pRecord+i*RecLen) |= val;
		}
	}
}

M4SRBlock *MDF4File::CreateSRBlock(M4DGBlock *dg, M4CGBlock *cg, double dt, double tRange, 
	   double OldXFactor, double OldXOffset, M_UINT32 uiMaxBlockSize/*= 0x40000*/, BOOL bCalcStats/*=FALSE*/)
{
	M_UINT32 nChan;
	double rawdt = dt;
	BOOL bVirt=FALSE; // virtual time channel?
	double FactorXSig=1, OffsetXSig=0; // calc raw value to phys time value
	// Create a new block of type SR
	M4SRBlock *sr = new M4SRBlock();
	if (!sr)
		return NULL;
	sr->sr_interval = dt;
	sr->sr_cycle_count = 0;
	sr->sr_flags = 0;
	if (cg->cg_inval_bytes)
		sr->sr_flags = SR_F_INVALIDATION;
	sr->sr_sync_type = SR_S_TIME;
	M4CNBlock *cn = FindMasterChannel(cg, &nChan);
	if (cn)
	{
		if (cn->cn_type == CN_T_VMASTER)
			bVirt = TRUE;
		if (cn->cn_sync_type != 0)
			sr->sr_sync_type = cn->cn_sync_type;
		if (cn->hasLink(M4CNBlock::cn_cc_conversion))
		{
			M4CCBlock *cc = (M4CCBlock *)LoadLink( *cn, M4CNBlock::cn_cc_conversion);
			if (cc)
			{
				assert(cc->cc_type == CC_T_LINEAR || cc->cc_type == CC_T_IDENTITY);
				if (cc->cc_type == CC_T_LINEAR && *cc->m_var.get(1)!=0.0)
					rawdt = dt / *cc->m_var.get(1);
				FactorXSig = *cc->m_var.get(1);
				OffsetXSig = *cc->m_var.get(0);
				delete cc;
			}
		}
		delete cn;
	}

	// Is there a previous SR block which could be used to create the current?
	BOOL bSRFlag = FALSE;
	if (cg->hasLink(M4CGBlock::cg_sr_first))
  {
		M4SRBlock *b = (M4SRBlock *)LoadLink(*cg, M4CGBlock::cg_sr_first, M4ID_SR);
		double fac = dt / b->sr_interval; // dt should be a multiple of b->sr_interval 
		fac -= floor(fac);
		if (fac < 0.000001)
			bSRFlag = TRUE;
		delete b;
	}
	
	M_LINK srAt=sr->Create(this,3);
  if (srAt)
  {
		if (bSRFlag)
			CreateSRBlockFromSR(sr, dg, cg, dt, tRange, nChan, OldXFactor, OldXOffset, FactorXSig, OffsetXSig, uiMaxBlockSize);
		else
			CreateSRBlockFromDG(sr, dg, cg, dt, tRange, nChan, OldXFactor, OldXOffset, FactorXSig, OffsetXSig, uiMaxBlockSize, bCalcStats);

		if (cg->hasLink(M4CGBlock::cg_sr_first)) // goto the end of the queue
    {
			M4SRBlock *p, *b = (M4SRBlock *)LoadLink(*cg, M4CGBlock::cg_sr_first, M4ID_SR);
			while (b && b->hasLink(M4SRBlock::sr_sr_next))
			{
				p = (M4SRBlock *)LoadLink(*b, M4SRBlock::sr_sr_next, M4ID_SR);
				delete b;
				b = p;
			}
      b->setLink(M4SRBlock::sr_sr_next,srAt);
      b->WriteHdr();
      delete b;
    }
    else
    {
      cg->setLink(M4CGBlock::cg_sr_first,srAt);
			cg->WriteHdr();
    }
		sr->Save();
  }
  return sr;
}

void MDF4File::CreateSRBlockFromSR(M4SRBlock *sr, M4DGBlock *dg, M4CGBlock *cg, double dt, double tRange,
          		                      M_UINT32 nChan, M_REAL OldXFactor, M_REAL OldXOffset, M_REAL FactorXSig, M_REAL OffsetXSig, M_UINT32 uiMaxBlockSize)
{
	M4SRBlock *p, *srcSR = (M4SRBlock *)LoadLink(*cg, M4CGBlock::cg_sr_first, M4ID_SR);
	while (srcSR && srcSR->hasLink(M4SRBlock::sr_sr_next))
	{
		p = (M4SRBlock *)LoadLink(*srcSR, M4SRBlock::sr_sr_next, M4ID_SR);
		delete srcSR;
		srcSR = p;
	}
	double fac = dt / srcSR->sr_interval; // dt should be a multiple of srcSR->sr_interval
	M_UINT32 scans = floor(fac);

	double Time, val, limit, FirstTime, LastLimit;
	M_UINT32 blkSize,dlSize, nRecordsExpected, nBlocks, nPossible;
	M_UINT32 RecLenSR = (M_UINT32)(cg->cg_data_bytes * 3 + cg->cg_inval_bytes); 
	Three *pBuffer = (Three*)calloc(nChan, sizeof(Three));
	M_UINT8 *pInRecord = (M_UINT8*)malloc(RecLenSR);
	M_UINT8 *pOutRecord = (M_UINT8*)malloc(RecLenSR);
	int nIdxTime;
	// Calc block size assuming almost equidistant data
	nPossible = uiMaxBlockSize / RecLenSR;
	nRecordsExpected = ceil(tRange / dt);
	if (nRecordsExpected==0)
		return;
	if (nRecordsExpected > nPossible)
		blkSize = nPossible;
	else
		blkSize = nRecordsExpected;
	if (blkSize==0)
		return;
	nBlocks = (nRecordsExpected+blkSize-1)/blkSize;
	blkSize *= RecLenSR;
	if (nBlocks < 10)
		dlSize = nBlocks;
	else dlSize = 10;

	FillThree(dg, cg, pBuffer);
	m4DataStream *stream = new m4DataStream( this, srcSR->getLink(M4SRBlock::sr_data));
	stream->Seek(0);
	m4DataStream *out;
	BOOL bPending = FALSE;
	BOOL bVirtMaster = FALSE;
	if (m_Id.id_ver == 400 || cg->cg_cycle_count < 100)
		out = new m4DataStream(sr,blkSize,dlSize);
	else
		out = new m4DZStream(sr,blkSize,RecLenSR,dlSize);
	sr->Save();
	for (M_UINT32 ch = 0; ch < nChan; ch++)
	{
		if (pBuffer[ch].flags.bMaster)
		{
			nIdxTime = ch;
			break;
		}
	}

	M_UINT64 n = 1;
	for (M_UINT64 i = 0; i<srcSR->sr_cycle_count; i++)
	{
		if (RecLenSR == stream->Read( RecLenSR, pInRecord))
		{
			if (pBuffer[nIdxTime].flags.bMaster == 1)
				Time = InterpretSR(pInRecord, &pBuffer[nIdxTime], cg->cg_data_bytes, true) * FactorXSig + OffsetXSig;
			else
				Time = OldXOffset + srcSR->sr_interval * i;
			if (i)
			{
				bPending = TRUE;
				BOOL bOK;
				if (bVirtMaster)
					bOK = limit - Time < OldXFactor;
				else
					bOK = Time >= limit;
				if (bOK)
				{
					memset(pOutRecord, 0, RecLenSR);
					bPending = FALSE; // no values to write ?
					for (M_UINT32 ch = 0; ch < nChan; ch++)
					{
						if (pBuffer[ch].flags.bMaster==0 && pBuffer[ch].nMiss<pBuffer[ch].n)
						{
							bPending = TRUE; // no values to write ?
							break;
						}
					}
					if (bPending)
					{
						pBuffer[nIdxTime].Mean = (LastLimit-OffsetXSig)/FactorXSig;
						for (M_UINT32 ch = 0; ch < nChan; ch++)
						{
							if (pBuffer[ch].flags.bMaster!=2)
								Encode(pOutRecord, &pBuffer[ch], cg->cg_data_bytes);
						}
						out->Write(RecLenSR,pOutRecord);
						sr->sr_cycle_count++;
					}
					ResetThree(pBuffer, nChan);
					LastLimit = limit;
					while (limit <= Time)
					{
						n++;
						limit = FirstTime + (double)n * dt;
					}
					bPending = FALSE;
				}
			}
			for (M_UINT32 ch = 0; ch < nChan; ch++)
			{
				if (pBuffer[ch].flags.bMaster == 2)
				{
					val = OldXOffset + srcSR->sr_interval * i;
					if (pBuffer[ch].Mean == 1E308) // not yet set
						pBuffer[ch].Mean = val;
					pBuffer[ch].n++;
				}
				else
					val = InterpretSR(pInRecord, &pBuffer[ch], cg->cg_data_bytes);
				if (pBuffer[ch].flags.bMaster)
				{
					if (pBuffer[ch].flags.bMaster == 2)
						Time = val;
					else
						Time = val * FactorXSig + OffsetXSig;
					if (i==0)
					{
						if (pBuffer[ch].flags.bMaster == 2)
							bVirtMaster = TRUE;
						FirstTime = Time;
						LastLimit = FirstTime;
						limit = Time + dt;
					}
				}
			}
		}
		else
			break;
	}
	if (bPending)
	{
		memset(pOutRecord, 0, RecLenSR);
		for (M_UINT32 ch = 0; ch < nChan; ch++)
		{
			if (pBuffer[ch].flags.bMaster && pBuffer[ch].n==0)
			{
				bPending = FALSE; // no values to write
				break;
			}
			if (pBuffer[ch].flags.bMaster!=2)
				Encode(pOutRecord, &pBuffer[ch], cg->cg_data_bytes);
		}
		if (bPending)
		{
			out->Write(RecLenSR,pOutRecord);
			sr->sr_cycle_count++;
		}
	}
	out->Flush();
	delete out;
	sr->Save();

	delete stream;
	free(pOutRecord);
	free(pInRecord);
	free(pBuffer);
}

void MDF4File::CalculateMinMax(M4DGBlock *dg, M4CGBlock *cg, M_REAL OldXFactor, M_REAL OldXOffset)
{
	double Time, LastTime, val;
	double TimeMin=1E308, TimeMax=-1E308;
	M_UINT32 nIdxTime;
	M_UINT32 RecLen = (M_UINT32)cg->getRecordSize() + dg->dg_rec_id_size; 
	M_UINT32 RecLenSR = (M_UINT32)(cg->cg_data_bytes * 3 + cg->cg_inval_bytes);
	M_BYTE *pReadBuffer, *pReadPtr;
	M_UINT32 dwReadBufferSize, dwNRecordsInReadBuffer, dwDataRead, dwNRecordsLast, nBuffers, nBuffersRead;
	M_UINT8 *pInRecord = (M_UINT8*)malloc(RecLen);
	M_UINT32 nChan;
	double FactorXSig=1.0, OffsetXSig=0.0;
	M4CNBlock *cn = FindMasterChannel(cg, &nChan);
	if (cn)
	{
		if (cn->hasLink(M4CNBlock::cn_cc_conversion))
		{
			M4CCBlock *cc = (M4CCBlock *)LoadLink( *cn, M4CNBlock::cn_cc_conversion);
			if (cc)
			{
				assert(cc->cc_type == CC_T_LINEAR || cc->cc_type == CC_T_IDENTITY);
				if (cc->cc_type == CC_T_LINEAR && *cc->m_var.get(1)!=0.0)
				FactorXSig = *cc->m_var.get(1);
				OffsetXSig = *cc->m_var.get(0);
				delete cc;
			}
		}
		delete cn;
	}
	Three *pBuffer = (Three*)calloc(nChan, sizeof(Three));
	BOOL bTimeCalc = FactorXSig!=1.0 || OffsetXSig!=0.0;
	Stats *pStatistic = (Stats*)calloc(nChan, sizeof(Stats));

	// Use a ReadBuffer for buffering input data
	dwReadBufferSize = RecLen;
	dwNRecordsInReadBuffer = 0xA00000 / RecLen;
	if (dwNRecordsInReadBuffer > cg->cg_cycle_count)
		dwNRecordsInReadBuffer = cg->cg_cycle_count;
	dwNRecordsLast = dwNRecordsInReadBuffer;
	nBuffers = cg->cg_cycle_count / dwNRecordsInReadBuffer;
	if (cg->cg_cycle_count % dwNRecordsInReadBuffer)
	{
		nBuffers++;
		dwNRecordsLast = cg->cg_cycle_count % dwNRecordsInReadBuffer; 
	}
	dwReadBufferSize = RecLen * dwNRecordsInReadBuffer;
	pReadBuffer = (M_BYTE*)malloc(dwReadBufferSize);
	if (pReadBuffer == NULL)
		return;

	FillThree(dg, cg, pBuffer);
	m4DataStream *stream = dg->ReadStream();
	stream->Seek(0);
	BOOL bVirtMaster = FALSE;
	for (M_UINT32 ch = 0; ch < nChan; ch++)
	{
		if (pBuffer[ch].flags.bMaster)
			nIdxTime = ch;
		pStatistic[ch].Min = 1E308;
		pStatistic[ch].Max = -1E308;
		pStatistic[ch].mindt = 1E308;
		pStatistic[ch].maxdt = -1E308;
	}
	M_UINT64 n = 1; 
	dwDataRead = stream->Read( dwReadBufferSize, pReadBuffer);
	nBuffersRead = 1;
	pReadPtr = pReadBuffer;
	for (M_UINT64 i = 0; i<cg->cg_cycle_count; i++)
	{
		if (dwDataRead == dwReadBufferSize || dwDataRead == dwNRecordsLast * RecLen)
		{
			if (i && (i%dwNRecordsInReadBuffer)==0)
			{
				if (nBuffersRead == nBuffers-1)
					dwDataRead = stream->Read( dwNRecordsLast * RecLen, pReadBuffer);
				else
					dwDataRead = stream->Read( dwReadBufferSize, pReadBuffer);
				pReadPtr = pReadBuffer;
				nBuffersRead++;
			}
			memcpy(pInRecord, pReadPtr, RecLen);
			pReadPtr += RecLen;
			if (pBuffer[nIdxTime].flags.bMaster == 1)
			{
				Time = Interpret(pInRecord, &pBuffer[nIdxTime], cg->cg_data_bytes + dg->dg_rec_id_size, true);
				if (bTimeCalc)
					Time = Time * FactorXSig + OffsetXSig;
			}
			else
				Time = OldXOffset + OldXFactor * i;
			if (TimeMin > Time) TimeMin = Time;
			else if (TimeMax < Time) TimeMax = Time;
			if (i)
			{
				double dt = Time - LastTime;
				if (pStatistic[nIdxTime].mindt > dt)
					pStatistic[nIdxTime].mindt = dt;
				if (pStatistic[nIdxTime].maxdt < dt)
					pStatistic[nIdxTime].maxdt = dt;
				pStatistic[nIdxTime].Max = Time;
			}
			else
				pStatistic[nIdxTime].Min = Time;
			for (M_UINT32 ch = 0; ch < nChan; ch++)
			{
				if (pBuffer[ch].flags.bMaster == 2)
				{
					val = OldXOffset + OldXFactor * i;
					if (pBuffer[ch].Mean == 1E308) // not yet set
						pBuffer[ch].Mean = val;
					pBuffer[ch].n++;
				}
				else
					val = Interpret(pInRecord, &pBuffer[ch], cg->cg_data_bytes + dg->dg_rec_id_size);
				if (pBuffer[ch].flags.bMaster)
				{
					if (pBuffer[ch].flags.bMaster == 2 || !bTimeCalc)
						Time = val;
					else
						Time = val * FactorXSig + OffsetXSig;
					if (i==0)
					{
						if (pBuffer[ch].flags.bMaster == 2)
							bVirtMaster = TRUE;
						LastTime= Time;
					}
					else
						LastTime = Time;
				}
			}
		}
		else
			break;
	}
	for (M_UINT32 ch = 0; ch < nChan; ch++)
	{
		if (pBuffer[ch].flags.bMaster!=2)
		{
			if (pStatistic)
			{
				Stats *p = pStatistic + ch;
				if (ch == nIdxTime)
				{
				}
				else
				{
					if (p->Min > pBuffer[ch].Min)
						p->Min = pBuffer[ch].Min;
					if (p->Max < pBuffer[ch].Max)
						p->Max = pBuffer[ch].Max;
				}
			}
		}
	}
	delete stream;
	free(pInRecord);
	free(pBuffer);
	free(pReadBuffer);

	if (pStatistic)
	{
		M4CNBlock *cn = (M4CNBlock *)LoadLink(*cg, M4CGBlock::cg_cn_first);
		int ch = 0;
		while (cn)
		{
			if (cn->cn_data_type<CN_D_STRING)
			{
				if (cn->cn_type == CN_T_MASTER || cn->cn_type == CN_T_VMASTER)
				{
#if _MDF4_WIN
					/*******************
					<CNComment>
						<TX>Time Channel</TX>
						<raster min="1.000" max="1.000">1.000</raster>
						<axis_monotony>MON_INCREASE</axis_monotony>
					</CNComment>			
					*******************/
					double mindt, maxdt, xFactor=1;
          BOOL bMon = TRUE;
          if(cn->cn_type == CN_T_MASTER)
					{
						mindt = pStatistic[nIdxTime].mindt;
						maxdt = pStatistic[nIdxTime].maxdt;
            if(cg->cg_cycle_count>1)
              xFactor = (TimeMax-TimeMin)/(cg->cg_cycle_count-1);
            bMon = maxdt >= xFactor && mindt <= xFactor;
            if(bMon)
            {
              bMon = maxdt < xFactor * 1.01;
              if(bMon)
                bMon = mindt > xFactor * 0.99;
            }
          }
					else
						mindt = maxdt = OldXFactor;
					wchar_t wszBuffer[256];
					wchar_t *p=L"MON_INCREASE";

					if (bMon)
          {
            p = L"STRICT_INCREASE";
            if(GetIdBlk()->id_ver >= 410)
              cn->cn_flags |= CN_F_MONOTONOUS;
          }
          if(GetIdBlk()->id_ver < 410)
					{
						_swprintf(wszBuffer, 
							L"<CNcomment>" 
							L"<TX>Time Channel</TX>"
							L"<raster min=\"%.15lg\" max=\"%.15lg\">%.15lg</raster>"
							L"<axis_monotony>%s</axis_monotony>"
							L"</CNcomment>",
							mindt, maxdt, OldXFactor, p);
					}
					else
					{
						_swprintf(wszBuffer, 
							L"<CNcomment>" 
							L"<TX>Time Channel</TX>"
							L"<raster min=\"%.15lg\" max=\"%.15lg\">%.15lg</raster>"
							L"</CNcomment>",
							mindt, maxdt, OldXFactor);
					}
          M4MDBlock md;
          md.Append(wszBuffer);
					md.Save();
					if (cn->hasLink(M4CNBlock::cn_md_comment))
						cn->setLink(M4CNBlock::cn_md_comment, 0);
					cn->setComment(md);
#endif
					if ((cn->cn_flags & CN_F_RANGE)==0)
					{
						cn->cn_flags |= CN_F_RANGE;
						cn->cn_val_range_min = TimeMin/FactorXSig;
						cn->cn_val_range_max = TimeMax/FactorXSig;
					}
					cn->Save();
				}
				else
				{
          if(pStatistic[ch].Min	!= 1E308) // valid min/max
          {
            cn->cn_flags &= ~CN_F_ALLINVALID;
            cn->cn_flags |= CN_F_RANGE;
            cn->cn_val_range_min = pStatistic[ch].Min;
            cn->cn_val_range_max = pStatistic[ch].Max;
          }
          else // all invalid
            cn->cn_flags |= CN_F_ALLINVALID;
          cn->Save();
				}
			}
			M4CNBlock *p = (M4CNBlock *)LoadLink(*cn, M4CNBlock::cn_cn_next, M4ID_CN);
			delete cn;
			cn = p;
			ch++;
		}
		free(pStatistic);
	}
}

void MDF4File::CreateSRBlockFromDG(M4SRBlock *sr, M4DGBlock *dg, M4CGBlock *cg, double dt, double tRange,
          		                     M_UINT32 nChan, M_REAL OldXFactor, M_REAL OldXOffset, M_REAL FactorXSig, M_REAL OffsetXSig,
																	 M_UINT32 uiMaxBlockSize, BOOL bCalcStats)
{
	double Time, LastTime, val, limit, FirstTime, LastLimit;
	double TimeMin=1E308, TimeMax=-1E308;
	M_UINT32 blkSize,dlSize, nRecordsExpected, nBlocks, nPossible, nIdxTime;
	M_UINT32 RecLen = (M_UINT32)cg->getRecordSize() + dg->dg_rec_id_size; 
	M_UINT32 RecLenSR = (M_UINT32)(cg->cg_data_bytes * 3 + cg->cg_inval_bytes);
	M_BYTE *pReadBuffer, *pReadPtr;
	M_UINT32 dwReadBufferSize, dwNRecordsInReadBuffer, dwDataRead, dwNRecordsLast, nBuffers, nBuffersRead;
	Three *pBuffer = (Three*)calloc(nChan, sizeof(Three));
	M_UINT8 *pInRecord = (M_UINT8*)malloc(RecLen);
	M_UINT8 *pOutRecord = (M_UINT8*)malloc(RecLenSR);
	BOOL bTimeCalc = FactorXSig!=1.0 || OffsetXSig!=0.0;

	Stats *pStatistic = NULL;
	if (bCalcStats)
		pStatistic = (Stats*)calloc(nChan, sizeof(Stats));

	// Use a ReadBuffer for buffering input data
	dwReadBufferSize = RecLen;
	dwNRecordsInReadBuffer = 0xA00000 / RecLen;
	if (dwNRecordsInReadBuffer > cg->cg_cycle_count)
		dwNRecordsInReadBuffer = cg->cg_cycle_count;
	dwNRecordsLast = dwNRecordsInReadBuffer;
	nBuffers = cg->cg_cycle_count / dwNRecordsInReadBuffer;
	if (cg->cg_cycle_count % dwNRecordsInReadBuffer)
	{
		nBuffers++;
		dwNRecordsLast = cg->cg_cycle_count % dwNRecordsInReadBuffer; 
	}
	dwReadBufferSize = RecLen * dwNRecordsInReadBuffer;
	pReadBuffer = (M_BYTE*)malloc(dwReadBufferSize);
	if (pReadBuffer == NULL)
		return;

	// Calc block size assuming almost equidistant data
	nPossible = uiMaxBlockSize / RecLenSR;
	nRecordsExpected = ceil(tRange / dt);
	if (nRecordsExpected == 0)
		return;
	if (nRecordsExpected > nPossible)
		blkSize = nPossible;
	else
		blkSize = nRecordsExpected;
	if (blkSize == 0)
		return;
	nBlocks = (nRecordsExpected+blkSize-1)/blkSize;
	blkSize *= RecLenSR;
	if (nBlocks == 1)
		dlSize = 0;
	else if (nBlocks < 20)
		dlSize = nBlocks;
	else dlSize = 20;

	FillThree(dg, cg, pBuffer);
	m4DataStream *stream = dg->ReadStream();
	stream->Seek(0);
	m4DataStream *out;
	BOOL bPending = FALSE;
	BOOL bVirtMaster = FALSE;
	if (m_Id.id_ver == 400 || cg->cg_cycle_count < 100)
		out = new m4DataStream(sr,blkSize,dlSize);
	else
		out = new m4DZStream(sr,blkSize,RecLenSR,dlSize);
	sr->Save();
	for (M_UINT32 ch = 0; ch < nChan; ch++)
	{
		if (pBuffer[ch].flags.bMaster)
		{
			nIdxTime = ch;
			break;
		}
	}
	
	M_UINT64 n = 1; 
	dwDataRead = stream->Read( dwReadBufferSize, pReadBuffer);
	nBuffersRead = 1;
	pReadPtr = pReadBuffer;
	for (M_UINT64 i = 0; i<cg->cg_cycle_count; i++)
	{
		if (dwDataRead == dwReadBufferSize || dwDataRead == dwNRecordsLast * RecLen)
		{
			if (i && (i%dwNRecordsInReadBuffer)==0)
			{
				if (nBuffersRead == nBuffers-1)
					dwDataRead = stream->Read( dwNRecordsLast * RecLen, pReadBuffer);
				else
					dwDataRead = stream->Read( dwReadBufferSize, pReadBuffer);
				pReadPtr = pReadBuffer;
				nBuffersRead++;
			}
			memcpy(pInRecord, pReadPtr, RecLen);
			pReadPtr += RecLen;
			if (pBuffer[nIdxTime].flags.bMaster == 1)
			{
				Time = Interpret(pInRecord, &pBuffer[nIdxTime], cg->cg_data_bytes + dg->dg_rec_id_size, true);
				if (bTimeCalc)
					Time = Time * FactorXSig + OffsetXSig;
			}
			else
				Time = OldXOffset + OldXFactor * i;
			if (TimeMin > Time) TimeMin = Time;
			else if (TimeMax < Time) TimeMax = Time;
			if (i)
			{
				bPending = TRUE;
				if (Time >= limit && i < cg->cg_cycle_count-1)
				{
					//TRACE("dt = %.10lf\n", (FirstTime + (double)n * dt) - LastLimit);
					if (pBuffer[nIdxTime].flags.bMaster == 1)
					{
						if (bTimeCalc)
							pBuffer[nIdxTime].Mean = (LastLimit-OffsetXSig)/FactorXSig;
						else
							pBuffer[nIdxTime].Mean = LastLimit;
					}
					else
						pBuffer[nIdxTime].Mean = LastLimit; // Virtual Master channel
					memset(pOutRecord, 0, RecLenSR);
					bPending = FALSE; // no values to write ?
					for (M_UINT32 ch = 0; ch < nChan; ch++)
					{
						if (pBuffer[ch].flags.bMaster==0 && pBuffer[ch].nMiss<pBuffer[ch].n)
						{
							bPending = TRUE; // no values to write
							break;
						}
					}
					if (bPending)
					{
						if (bTimeCalc && pBuffer[nIdxTime].flags.bMaster == 1)
						{
							if (pBuffer[nIdxTime].flags.bInteger)
							{
								pBuffer[nIdxTime].Min = floor(pBuffer[nIdxTime].Min/FactorXSig+0.5);
								pBuffer[nIdxTime].Max = floor(pBuffer[nIdxTime].Max/FactorXSig+0.5);
							}
							else
							{
								pBuffer[nIdxTime].Min = pBuffer[nIdxTime].Min/FactorXSig;
								pBuffer[nIdxTime].Max = pBuffer[nIdxTime].Max/FactorXSig;
							}
						}
						for (M_UINT32 ch = 0; ch < nChan; ch++)
						{
							if (pBuffer[ch].flags.bMaster!=2)
							{
								Encode(pOutRecord, &pBuffer[ch], cg->cg_data_bytes);
								//if (ch == nIdxTime)
								//	TRACE("%.5lf %.5lf %.5lf\n", pBuffer[ch].Mean, pBuffer[ch].Min, pBuffer[ch].Max);
								if (pStatistic)
								{
									Stats *p = pStatistic + ch;
									if (ch == nIdxTime)
									{
										if (p->uiInitialized==0)
										{
											p->uiInitialized++;
											p->mindt = pBuffer[ch].Min;
											p->maxdt = pBuffer[ch].Max;
										}
										else
										{
											if (p->mindt > pBuffer[ch].Min)
												p->mindt = pBuffer[ch].Min;
											if (p->maxdt > pBuffer[ch].Max)
												p->maxdt = pBuffer[ch].Max;
										}
									}
									else
									{
										if (p->uiInitialized==0)
										{
											p->uiInitialized++;
											p->Min = pBuffer[ch].Min;
											p->Max = pBuffer[ch].Max;
										}
										else
										{
											if (p->Min > pBuffer[ch].Min)
												p->Min = pBuffer[ch].Min;
											if (p->Max < pBuffer[ch].Max)
												p->Max = pBuffer[ch].Max;
										}
									}
								}
							}
						}
						out->Write(RecLenSR,pOutRecord);
						sr->sr_cycle_count++;
					}
					ResetThree(pBuffer, nChan);
					LastLimit = limit;
					while (limit <= Time)
					{
						n++;
						limit = FirstTime + (double)n * dt;
					}
					bPending = FALSE;
				}
			}
			for (M_UINT32 ch = 0; ch < nChan; ch++)
			{
				if (pBuffer[ch].flags.bMaster == 2)
				{
					val = OldXOffset + OldXFactor * i;
					if (pBuffer[ch].Mean == 1E308) // not yet set
						pBuffer[ch].Mean = val;
					pBuffer[ch].n++;
				}
				else
					val = Interpret(pInRecord, &pBuffer[ch], cg->cg_data_bytes + dg->dg_rec_id_size);
				if (pBuffer[ch].flags.bMaster)
				{
					if (pBuffer[ch].flags.bMaster == 2 || !bTimeCalc)
						Time = val;
					else
						Time = val * FactorXSig + OffsetXSig;
					if (i==0)
					{
						if (pBuffer[ch].flags.bMaster == 2)
							bVirtMaster = TRUE;
						LastTime= Time;
						limit = Time + dt;
						LastLimit = Time;
						//limit = Time + dt;
						FirstTime = Time;
					}
					else
					{
						double diff = Time-LastTime;
						if (pBuffer[ch].Min > diff)
							pBuffer[ch].Min = diff;
						if (pBuffer[ch].Max < diff)
							pBuffer[ch].Max = diff;
						LastTime = Time;
					}
				}
			}
		}
		else
			break;
	}
	if (bPending)
	{
		pBuffer[nIdxTime].Mean = LastLimit;
		memset(pOutRecord, 0, RecLenSR);
		if (bTimeCalc && pBuffer[nIdxTime].flags.bMaster == 1)
		{
			if (pBuffer[nIdxTime].flags.bInteger)
			{
				pBuffer[nIdxTime].Min = floor(pBuffer[nIdxTime].Min/FactorXSig+0.5);
				pBuffer[nIdxTime].Max = floor(pBuffer[nIdxTime].Max/FactorXSig+0.5);
			}
			else
			{
				pBuffer[nIdxTime].Min = pBuffer[nIdxTime].Min/FactorXSig;
				pBuffer[nIdxTime].Max = pBuffer[nIdxTime].Max/FactorXSig;
			}
		}
		for (M_UINT32 ch = 0; ch < nChan; ch++)
		{
			if (pBuffer[ch].flags.bMaster && pBuffer[ch].n==0)
				bPending = FALSE; // no values to write
			if (pBuffer[ch].flags.bMaster!=2)
			{
				Encode(pOutRecord, &pBuffer[ch], cg->cg_data_bytes);
				if (pStatistic)
				{
					Stats *p = pStatistic + ch;
					if (ch == nIdxTime)
					{
						if (p->mindt > pBuffer[ch].Min)
							p->mindt = pBuffer[ch].Min;
						if (p->maxdt > pBuffer[ch].Max)
							p->maxdt = pBuffer[ch].Max;
					}
					else
					{
						if (p->Min > pBuffer[ch].Min)
							p->Min = pBuffer[ch].Min;
						if (p->Max < pBuffer[ch].Max)
							p->Max = pBuffer[ch].Max;
					}
				}
			}
		}
		if (bPending)
		{
			out->Write(RecLenSR,pOutRecord);
			sr->sr_cycle_count++;
		}
	}

	out->Flush();
	delete out;
	sr->Save();

	delete stream;
	free(pOutRecord);
	free(pInRecord);
	free(pBuffer);
	free(pReadBuffer);

	if (pStatistic)
	{
		M4CNBlock *cn = (M4CNBlock *)LoadLink(*cg, M4CGBlock::cg_cn_first);
		int ch = 0;
		while (cn)
		{
			if (cn->cn_data_type<CN_D_STRING)
			{
				if (cn->cn_type == CN_T_MASTER || cn->cn_type == CN_T_VMASTER)
				{
#if _MDF4_WIN
					/*******************
					<CNComment>
						<TX>Time Channel</TX>
						<raster min="1.000" max="1.000">1.000</raster>
						<axis_monotony>MON_INCREASE</axis_monotony>
					</CNComment>			
					*******************/
					double mindt, maxdt, xFactor=1;
          BOOL bMon = TRUE;
          if(cn->cn_type == CN_T_MASTER)
					{
						mindt = pStatistic[nIdxTime].mindt * FactorXSig;
						maxdt = pStatistic[nIdxTime].maxdt * FactorXSig;
            if(cg->cg_cycle_count>1)
              xFactor = (TimeMax-TimeMin)/(cg->cg_cycle_count-1);
            bMon = maxdt >= xFactor && mindt <= xFactor;
            if(bMon)
            {
              bMon = maxdt < xFactor * 1.01;
              if(bMon)
                bMon = mindt > xFactor * 0.99;
            }
          }
					else
						mindt = maxdt = OldXFactor;
					wchar_t wszBuffer[256];
					wchar_t *p=L"MON_INCREASE";

					if (bMon)
          {
            p = L"STRICT_INCREASE";
            if(GetIdBlk()->id_ver >= 410)
              cn->cn_flags |= CN_F_MONOTONOUS;
          }
          if(GetIdBlk()->id_ver < 410)
					{
						_swprintf(wszBuffer, 
							L"<CNcomment>" 
							L"<TX>Time Channel</TX>"
							L"<raster min=\"%.15lg\" max=\"%.15lg\">%.15lg</raster>"
							L"<axis_monotony>%s</axis_monotony>"
							L"</CNcomment>",
							mindt, maxdt, OldXFactor, p);
					}
					else
					{
						_swprintf(wszBuffer, 
							L"<CNcomment>" 
							L"<TX>Time Channel</TX>"
							L"<raster min=\"%.15lg\" max=\"%.15lg\">%.15lg</raster>"
							L"</CNcomment>",
							mindt, maxdt, OldXFactor);
					}
          M4MDBlock md;
          md.Append(wszBuffer);
					md.Save();
					if (cn->hasLink(M4CNBlock::cn_md_comment))
						cn->setLink(M4CNBlock::cn_md_comment, 0);
					cn->setComment(md);
#endif
					if ((cn->cn_flags & CN_F_RANGE)==0)
					{
						cn->cn_flags |= CN_F_RANGE;
						cn->cn_val_range_min = OldXOffset;
						cn->cn_val_range_max = OldXOffset+tRange;
					}
					cn->Save();
				}
				else
				{
          if(pStatistic[ch].Min	!= 1E308) // valid min/max
          {
            cn->cn_flags |= CN_F_RANGE;
            cn->cn_val_range_min = pStatistic[ch].Min;
            cn->cn_val_range_max = pStatistic[ch].Max;
          }
          else // all invalid
            cn->cn_flags |= CN_F_ALLINVALID;
          cn->Save();
				}
			}
			M4CNBlock *p = (M4CNBlock *)LoadLink(*cn, M4CNBlock::cn_cn_next, M4ID_CN);
			delete cn;
			cn = p;
			ch++;
		}
		free(pStatistic);
	}
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

m4DataStream::m4DataStream()
{
  m_dg = false;
  m_File = NULL;
	m_pHL = NULL;
  m_dl = NULL;
  m_db = NULL;
  m_currentSize = 0;
  m_currentPos = 0;
  m_currentOff = 0;
  m_blkType = 0;
	m_firstEntry = NULL;
	m_nEntries = 0;
  m_bValid = false;
  m_firstEntry = NULL;
  m_nEntries = 0;
	m_ZipCompressionLevel = Z_DEFAULT_COMPRESSION;
}

// ctor for write operations
m4DataStream::m4DataStream(M4DGBlock *dg,M_UINT32 BlkSize,M_UINT32 dlEntries/*=5*/,M_UINT16 dtID/*=M4ID_DT*/)
{
  M_LINK l_data;
	m_ZipCompressionLevel = Z_DEFAULT_COMPRESSION;
	m_pHL = NULL;
  m_File=(MDF4File *)dg->m_File;
	m_bValid=true;
  m_dg=true;
  m_blkType=dtID;
  m_db=new M4DataBlock(dtID,BlkSize);
  if (dlEntries) 
  {
    m_dl=new M4DLBlock(dlEntries,BlkSize);
    M_LINK l_dl=m_dl->Reset(m_File);
	  l_data=m_db->Reset(m_File);
    m_dl->addLink(l_data);
    l_data=l_dl;
  }
  else
  {
    m_dl=NULL;
	  l_data=m_db->Reset(m_File);
  }
  dg->setLink(dg->dg_data,l_data);
  dg->Save();
}

m4DataStream::m4DataStream(M4SRBlock *sr,M_UINT32 BlkSize,M_UINT32 dlEntries/*=5*/,M_UINT16 dtID/*=M4ID_RD*/)
{
  M_LINK l_data;
	m_ZipCompressionLevel = Z_DEFAULT_COMPRESSION;
	m_pHL = NULL;
  m_File=(MDF4File *)sr->m_File;
	m_bValid=true;
  m_dg=true;
  m_blkType=dtID;
  m_db=new M4DataBlock(dtID,BlkSize);
  if (dlEntries) 
  {
    m_dl=new M4DLBlock(dlEntries,BlkSize);
    M_LINK l_dl=m_dl->Reset(m_File);
	  l_data=m_db->Reset(m_File);
    m_dl->addLink(l_data);
    l_data=l_dl;
  }
  else
  {
    m_dl=NULL;
	  l_data=m_db->Reset(m_File);
  }
	sr->setLink(sr->sr_data,l_data);
  sr->Save();
}

// Write to the stream
bool m4DataStream::Write(M_UINT32 nBytes, const void *Memory)
{
  const M_BYTE *bMemory=(const M_BYTE *)Memory;
  assert(m_dg);
  assert(m_db);
  while (nBytes)
  {
    M_UINT32 doWrite,canWrite=m_db->CanWrite();
    doWrite=(canWrite>nBytes) ? nBytes : canWrite;
    // we need a new Block
    if (doWrite==0) 
    {
      // without a ##DL this fails
      if (m_dl==NULL) 
				return false;
      if (m_dl->isFull())
			{
				// It is possible, that the DT block to be written is large than the file size.
				// So we have to enlarge the file before we can request a new DL.
				M_INT64 diff = m_db->getSize() + m_db->m_At - m_File->AtEnd();
				if (diff > 0)
				{
					m_File->Request(diff);
				}
        m_dl->Reset(NULL);
			}
      // create a new block
      M_LINK lNo=m_db->Reset(m_File);
      // and add it to the DL
      m_dl->addLink(lNo);
    }
    else
    {
      m_db->DoWrite(doWrite,bMemory);
      bMemory+=doWrite; nBytes-=doWrite;
    }
  }
  return true;
}
// 
void m4DataStream::AddRecords(M_UINT32 nRecs/*=1*/,M_UINT64 RecId/*=0*/)
{
  assert(m_dg);
  m_File->AddRecords(nRecs,RecId);
}
void m4DataStream::Flush(BOOL bKeepSize /*= FALSE*/)
{
  assert(m_dg);
  if (m_dl)
    m_dl->Save();
  m_File->FlushRecordCount();
  if (m_db)
	{
		M_SIZE save; 
		if (bKeepSize)
		{
			save = m_db->getSize();
			m_db->setSize(m_db->getBlkSize());
		}
    m_db->Save();
		if (bKeepSize)
		{
			m_db->setSize(save);
	    m_db->Save(false, true);
			delete m_db;
			m_db = NULL;
		}
		else
		{
			M_INT64 diff = m_File->GetSize() - m_File->GetPos();
      if(diff > (M_INT64)m_db->getBlkSize() || m_db->getBlkSize() == m_db->getSize())
        diff = 0;
			if (diff != 0)
				m_File->SetEnd(-diff);
		}
	}
}
void m4DataStream::Close()
{
  if (m_dg)
	{
		Flush();
	}
	if (m_dl)
	{
		delete m_dl;
		m_dl = NULL;
	}
	if (m_db)
	{
		delete m_db;
		m_db = NULL;
	}

	dlEntry *p = (dlEntry *)m_List.m_t;
  for (size_t i = 0; i < m_List.m_n; i++, p++)
	{
		if (p->pBuffer)
		{
			delete p->pBuffer;
			p->pBuffer = NULL;
		}
	}
#if 0
	M_UNICODE *e = (M_UNICODE *)m_ErrorList.m_t;
  for (size_t i=0; i<m_ErrorList.m_n; i++, e++)
	{
		if (e)
		{
			free(e);
			*e = NULL;
		}
	}
	m_ErrorList.m_n = 0;
#endif
}

m4DataStream *M4DGBlock::WriteStream(M_UINT32 blkSize,M_UINT32 dlSize/*=5*/)
{
  m4DataStream *s=new m4DataStream(this,blkSize,dlSize);
  Save();
  return s;
}
m4DZStream *M4DGBlock::DZWriteStream(M_UINT32 blkSize,M_UINT32 recordSize,M_UINT32 dlSize/*=5*/)
{
  m4DZStream *s = new m4DZStream(this,blkSize,recordSize,dlSize);
  return s;
}
m4DataStreamEx *M4DGBlock::WriteStreamEx(M_UINT32 blkSize, M_UINT32 dlSize/*=5*/)
{
	m4DataStreamEx *s=new m4DataStreamEx(this, blkSize, dlSize);
	Save();
	return s;
}
m4DZStreamEx *M4DGBlock::DZWriteStreamEx(M_UINT32 blkSize, M_UINT32 recordSize, M_UINT32 dlSize/*=5*/)
{
	m4DZStreamEx *s = new m4DZStreamEx(this, blkSize, recordSize, dlSize);
	return s;
}

void MDF4File::addRecordCount(M4CGBlock *cg,M_UINT64 id/*=0*/)
{
  idCount c;
  cg->adrRecordCount(c,id);
  id=c.id;
  m_recCnt[id]=c;
}
void MDF4File::AddRecords(M_UINT32 nRecs/*=1*/,M_UINT64 recId/*=0*/)
{
  idCounts::iterator i=m_recCnt.find(recId);
  if (i!=m_recCnt.end())
    i->second.cnt+=nRecs;
}
void MDF4File::FlushRecordCount()
{
  for(idCounts::iterator i=m_recCnt.begin(); i!=m_recCnt.end(); i++)
  {
    M_UINT64 c=i->second.cnt;
    if (c) WriteAt(i->second.aov,sizeof(c),&c);
  }
}

#if !_MDF4_WONLY     // [B] READ Support
m4DataStream::m4DataStream(MDF4File *File,M_LINK At)
{
  m_dg=false;
  m_File=File;
	m_pHL = NULL;
  m_dl=NULL;
  m_db=NULL;
  m_currentSize=0;
  m_currentPos=0;
  m_currentOff=0;
  m_blkType=0;
	m_firstEntry=NULL;
	m_nEntries=0;
  m_bValid=initSize(At);
  m_firstEntry=m_List.get(0);
  m_nEntries=m_List.getSize();
}
m4DataStream::m4DataStream(m4DataStream &org,M_LINK Position/*=0*/)
{
  m_dg=false;
  m_File=org.m_File;
	m_pHL = NULL;
  m_dl=NULL;
  m_db=NULL;
  m_currentSize=org.m_currentSize;
  m_currentPos=0;
  m_currentOff=0;
  m_blkType=org.m_blkType;
  m_bValid=org.m_bValid;
  m_firstEntry=org.m_firstEntry;
  m_nEntries=org.m_nEntries;
  Seek(Position);
}

//-------------------------------------------------------------------------
// determine the size of a data list by traversing through the blocks
//-------------------------------------------------------------------------
bool m4DataStream::initSize(M_LINK At)
{
  dlEntry   *e;
  // read the 1st block
  m4Block *firstBlock=m_File->LoadBlock(At);
  if (firstBlock)
	{
		M_UINT16 Id=firstBlock->hdrID();
		if (Id == M4ID_HL)
		{
			m_pHL = (M4HLBlock*)firstBlock;
			firstBlock=m_File->LoadBlock(m_pHL->getLink(m4HLRecord::hl_dl_first));
		}
	}
  if (firstBlock)
  {
    M_UINT16 Id=firstBlock->hdrID();
    // see if this is a data list (##DL)
    if (Id==M4ID_DL)
    {
      // typecast to the DL block
      M4DLBlock *dl=(M4DLBlock *)firstBlock;
      while (At && dl)
      {
        assert(dl->hdrID()==M4ID_DL);
        if (dl->hdrID()!=M4ID_DL)
        {
          // illegal stream
          m_List.resize(0);
          m_currentSize=0;
          delete dl;
          return false;
        }
        M_LINK nxtDL=dl->getLink(m4DLRecord::dl_dl_next);
        M_UINT32 lno=(M_UINT32)dl->dl_data;
        M_UINT32 nl=dl->dl_count;
        e=m_List.add(nl);
        for(M_UINT32 il=0; il<nl; il++,e++)
        {
          m4BlockHdr blk;
          M_LINK el=dl->getLink(lno+il);
          // read the BlockHdr of the linked block
          if (!m_File->LoadBlkHdr(el,blk))
						return false;
          // it should have no links 
          //assert(blk.hdr_links==0);
          // it should be one of ##DT,##SD,##SR,##RD,##DZ 
          BOOL bOK = (blk.hdr_id==M4ID_DT || blk.hdr_id==M4ID_SD 
						|| blk.hdr_id==M4ID_SR || blk.hdr_id==M4ID_RD
						|| blk.hdr_id==M4ID_DZ);
					if (!bOK)
					{
						ATLTRACE("m4DataStream::initSize: blk.hdr_id!=M4ID_DT | M4ID_SD | M4ID_SR | M4ID_RD | M4ID_DZ\n");
						return false;
					}
          // all types should be the same except in case of mixed DZ
          if (m_blkType==0) m_blkType=blk.hdr_id;
          else
          {
            assert(m_blkType==M4ID_DZ || m_blkType==blk.hdr_id);
						if (m_blkType!=M4ID_DZ && m_blkType!=blk.hdr_id)
							return false;
          }
          // determine the raw size of the block (no header or links)
          M_SIZE eSize=blk.hdr_len-sizeof(blk)-blk.hdr_links*sizeof(M_LINK);
          bool bLastBlk=nxtDL==0 && (il+1)==nl;
					M_UINT64  dz_org_data_length=0;
					if (blk.hdr_id==M4ID_DZ)
					{
						m4DZRecord rec;
						m_File->ReadAt(el+sizeof(blk), sizeof(m4DZRecord), &rec);
						dz_org_data_length = rec.dz_org_data_length;
					}
					e->pBuffer = NULL;
					e->BlkID = blk.hdr_id;
          // in a equal-length-DL
          if (dl->dl_flags&DL_F_EQUAL)
          {
            // all but the last must be same size
            if (!bLastBlk)
            {
							M_SIZE TestSize = *dl->get(0);
							if (m_blkType!=M4ID_DZ)
							{
		            assert(eSize==TestSize);
								if (eSize!=TestSize)
									return false;
							}
            }
            e->At=el+sizeof(m4BlockHdr);
						if (dz_org_data_length)
							e->Size = dz_org_data_length;
						else
							e->Size=eSize;
            m_currentSize+=e->Size;
          }
          // in a Dl w/o equal_length
          else
          {
						if (m_blkType!=M4ID_DZ)
							assert(m_currentSize==*dl->get(il)); // the offset should be the current size
            e->At=el+sizeof(m4BlockHdr);
						if (dz_org_data_length)
							e->Size = dz_org_data_length;
						else
							e->Size=eSize;
            m_currentSize+=e->Size;
          }
        }
        // process DL-List(dl)
        At=nxtDL;
        delete dl;
        if (At)
          dl=(M4DLBlock *)m_File->LoadBlock(At);
      }
    }
    else
    {
      // One of the Data Blocks (no DL)
      if (Id==M4ID_DT || Id==M4ID_SR || Id==M4ID_SD || Id==M4ID_RD || Id==M4ID_DZ)
      {
        M4DataBlock *b=(M4DataBlock *)firstBlock;
				M4DZBlock *pDZ = NULL;
				if (Id==M4ID_DZ)
					pDZ = (M4DZBlock *)b;
        dlEntry *e=m_List.add(1);
				e->pBuffer = NULL;
				e->BlkID = Id;
        e->At=At+sizeof(m4BlockHdr);
				if (pDZ)
					e->Size = pDZ->dz_org_data_length;
				else
					e->Size=b->getSize();
        m_currentSize=e->Size;
        delete b;
      }
    }
  }
  e=m_List.add(1);
  e->Size=0;
  Seek(0);
	//ATLTRACE("m4DataStream::initSize: finished (%lu)\n", m_List.m_n);
	return true;
}

// Seek to a position in the stream
M_LINK m4DataStream::Seek(M_LINK Position)
{
  if (Position>m_currentSize) m_currentPos=Position=m_currentSize;
  else                        m_currentPos=Position;
  // find the current entry in the list
  M_UINT32 iEntry,nEntries=m_nEntries;
  dlEntry *e=m_firstEntry;
  m_currentEntry=NULL;
  M_LINK eOffset=0;
  for(iEntry=0; iEntry<nEntries; iEntry++,e++)
  {
    if (e->Size>(Position-eOffset))
    {
      // we found it
      m_currentEntry=e;
      m_currentOff=Position-eOffset;
      break;
    }
    else 
    {
      eOffset+=e->Size;
    }
  }
  //assert(m_currentEntry);
  if (m_currentEntry==NULL)
  {
    m_currentEntry=m_List.get(0);
    return m_currentPos=0;
  }
  return m_currentPos;
}
// Read from the current position
M_UINT32 m4DataStream::Read(M_UINT32 nBytes,void *Into)
{
  M_BYTE *bInto=(M_BYTE *)Into;
  M_UINT32 doRead,didRead=0;
  while (nBytes && m_currentEntry && m_currentEntry->Size)
  {
    M_SIZE szRemain=m_currentEntry->Size-m_currentOff;
    doRead=szRemain<nBytes ? (M_UINT32)szRemain : nBytes;
		if (m_currentEntry->BlkID != M4ID_DZ)
		{
			//TRACE("m4: ReadAt( n=%lu, ofs=%lu)\n",doRead,(DWORD)m_currentOff);
			if (!m_File->ReadAt(m_currentEntry->At+m_currentOff,doRead,bInto)) 
				break;
		}
		else
		{
			if (m_currentEntry->pBuffer == NULL)
			{
				//ATLTRACE("m4: ReadDZBlock( n=%lu, ofs=%lu)\n",(DWORD)m_currentEntry->Size,(DWORD)m_currentEntry->At);
				m_currentEntry->pBuffer = (M_BYTE*)ReadDZBlock(m_currentEntry->At, m_currentEntry->Size);
			}
			if (m_currentEntry->pBuffer)
				memcpy(bInto, m_currentEntry->pBuffer+m_currentOff, doRead);
			else
				break;
		}
    bInto+=doRead;
    m_currentOff+=doRead;
    m_currentPos+=doRead;
    didRead+=doRead;
    nBytes-=doRead;
    if (m_currentOff==m_currentEntry->Size)
    {
      m_currentOff=0;
			if (m_currentEntry->pBuffer)
			{
				delete m_currentEntry->pBuffer;
				m_currentEntry->pBuffer=NULL;
			}
      ++m_currentEntry;
    }
  }
  return didRead;
}

int m4DataStream::Inflate(M_BYTE* dst, M_UINT32 *pdwSize, M_BYTE *src, M_UINT32 dwCompressedLength)
{
  int ret;

  z_stream strm; 
	memset(&strm, 0, sizeof(z_stream));
	ret = inflateInit(&strm);
	if (ret!=Z_OK)
		return ret;
	strm.avail_in = dwCompressedLength;
  strm.next_in = src;
	strm.avail_out = *pdwSize;
	strm.next_out = dst;
	ret = inflate(&strm,Z_FINISH); //Z_NO_FLUSH);
	inflateEnd(&strm);
  return ret;
}

int m4DataStream::Deflate(M_BYTE* dst, M_UINT32 *pdwCompressedLength, M_BYTE *src, M_UINT32 dwSize)
{
  int ret;
  z_stream strm; 
	memset(&strm, 0, sizeof(z_stream));
	ret = deflateInit(&strm,m_ZipCompressionLevel);
	if (ret!=Z_OK)
		return ret;
#ifndef USE_MINIZLIB
	strm.data_type = Z_BINARY;
#endif
	strm.avail_in = dwSize;
  strm.next_in = src;
	strm.avail_out = *pdwCompressedLength;
	strm.next_out = dst;
	ret = deflate(&strm,Z_FINISH);
	deflateEnd(&strm);
	*pdwCompressedLength = strm.total_out;
  return ret;
}

#if _MDF4_WIN
#pragma optimize( "gt", on )
#define CACHE_LINE  32
  #define CACHE_ALIGN __declspec(align(CACHE_LINE))
#else
#define CACHE_ALIGN
#endif
BOOL m4DataStream::Transpose(M_BYTE *dst, M_UINT32 dwSize, M_UINT32 dwRecordSize)
{
	CACHE_ALIGN M_BYTE *pBuffer = (M_BYTE*)calloc(dwSize,1);
	if (pBuffer == NULL)
		return FALSE;
	M_BYTE *p = pBuffer;
	M_UINT32 j;
	M_UINT32 RxC;
	CACHE_ALIGN M_INT32 R,C,r=0,c=0,tidx=0;
	memcpy( pBuffer, dst, dwSize);
	C = dwSize/dwRecordSize;
	R = dwRecordSize;
	RxC = R*C;
  // overwrite source from copy
#if 0
	for(j=0; j<RxC; j++)
	{
		r = j/C;
		c = j%C;
		tidx = c*R + r;
		dst[tidx] = *p++;
	}
#else
  c = 0; tidx = -R;
  for(j = 0; j<RxC; j++)
  {
    if(c++ == C)
    {
      tidx = j/C;
      c = 1;
    }
    else
      tidx+=R;
    dst[tidx] = *p++;
  }

#endif
	free(pBuffer);
	return TRUE;
}

#if (_MDF4_WIN==1) & (_MSC_VER >= 1800)

//
// parallelTranspose - changed transpose-algorithm that handles columns/rows
// seperately (instead of intermixed as in simpleTranspose)
// this allows for the main loop to to run on multiple cores if template parameter PPL is true)
// if PPL is false the same algorithm is used but in sequential loop
template<bool PPL, bool SKIP>
void parallelTranspose(const byteBuffer &unzipped, unsigned unzippedSize, byteBuffer &result, unsigned recordSize)
{
  // determines rows and columns
  unsigned rows = recordSize;
  unsigned cols = static_cast<unsigned>(unzippedSize / rows);

  // get src and destination buffers
  const uint8_t *z = unzipped.data();   // source ptr
  uint8_t *r = result.data();           // destination ptr


  // parallelization strategy
  //  the inner loop should be larger than the outer loop
  // otherwise we would have many tasks which have "nothing" to do
  if(SKIP || rows >= cols)
  {
    // # of records > recordSize => 
    //   each column (byte in the record) handles all records
    if(PPL)
    {
      concurrency::parallel_for(0u, cols, [&](unsigned i)
      {
        const uint8_t *src = z + i;
        uint8_t *dst = r + i*rows;
        unsigned r = rows;
        while(r--)
        {
          *dst++ = *src;
          src += cols;
        }
      });
    }
    else for(unsigned i = 0; i < cols; ++i)
    {
      // then process each column/row (may be parallelized)
      const uint8_t *src = z + i;
      uint8_t *dst = r + i*rows;
      unsigned r = rows;
      while(r--)
      {
        *dst++ = *src;
        src += cols;
      }
    }
  }
  else
  {
    // # of records < recordSize => 
    //   each record  handles all columns
    if(PPL)
    {
      concurrency::parallel_for(0u, rows, [&](unsigned i)
      {
        const uint8_t *src = z + i*cols;
        uint8_t *dst = r + i;
        unsigned c = cols;
        while(c--)
        {
          *dst = *src++;
          dst += rows;
        }
      });
    }
    else for(unsigned i = 0; i < rows; ++i)
    {
      const uint8_t *src = z + i*cols;
      uint8_t *dst = r + i;
      unsigned c = cols;
      while(c--)
      {
        *dst = *src++;
        dst += rows;
      }
    }
  }
	unsigned rest = unzippedSize % rows;
	if (rest)
	{
		for (unsigned i = unzippedSize - rest; i < unzippedSize; i++)
		{
			result[i] = unzipped[i];
		}
	}
}
#endif // (_MDF4_WIN==1) & (_MSC_VER >= 1800)


#pragma optimize( "gt", off )
#if (_MDF4_WIN==1) & (_MSC_VER >= 1800)

void* m4DataStream::ReadDZBlock(M_LINK At, M_SIZE Size)
{
	void *p = NULL;
	M_UINT32 dwSize = (M_UINT32)Size;
	M_BYTE *dst = new M_BYTE[Size];
	if (m_DZReadBuffer.size() < Size)
		m_DZReadBuffer.resize(Size);

	M4DZBlock *pDZ = (M4DZBlock*)m_File->LoadBlock(At - sizeof(m4BlockHdr));
  if(pDZ->getSize() >= pDZ->dz_data_length)
	{
    M_SIZE i64Read = pDZ->dz_data_length;
    int res = Inflate(m_DZReadBuffer.data(), &dwSize, (M_BYTE*)pDZ->getData(0, i64Read), pDZ->dz_data_length);
		if (res == Z_STREAM_END)
		{
			if (pDZ->dz_zip_type == 1) // Transpose
			{
				if (m_TransposedBuffer.size() < Size)
					m_TransposedBuffer.resize(Size);
        parallelTranspose<true, true>(m_DZReadBuffer, dwSize, m_TransposedBuffer, pDZ->dz_zip_parameter);
        memcpy(dst, m_TransposedBuffer.data(), Size);
			}
      else
        memcpy(dst, m_DZReadBuffer.data(), Size);
			p = (void*)dst;
		}
		else
		{
#ifndef UNICODE
			char szBuffer[128];
			sprintf(szBuffer,"Invalid DZBlock: Cannot inflate -  @0x%016I64X\n",At);
			OutputDebugString(szBuffer);
			//AddError(szBuffer, 2);
#endif
			delete dst;
		}
	}
	else
		delete dst;
	delete pDZ;
	return p;
}
#else
void* m4DataStream::ReadDZBlock(M_LINK At, M_SIZE Size)
{
  void *p = NULL;
  M_UINT32 dwSize = (M_UINT32)Size;
  M_BYTE *dst = new M_BYTE[Size];
  M4DZBlock *pDZ = (M4DZBlock*)m_File->LoadBlock(At - sizeof(m4BlockHdr));
  if(pDZ->getSize() >= pDZ->dz_data_length)
  {
    M_SIZE i64Read = pDZ->dz_data_length;
    int res = Inflate(dst, &dwSize, (M_BYTE*)pDZ->getData(0, i64Read), pDZ->dz_data_length);
    if(res == Z_STREAM_END)
    {
      if(pDZ->dz_zip_type == 1) // Transpose
      {
        Transpose(dst, dwSize, pDZ->dz_zip_parameter);
			}
      p = (void*)dst;
    }
    else
    {
#if _MDF4_WIN
#ifndef UNICODE
      char szBuffer[128];
      sprintf(szBuffer, "Invalid DZBlock: Cannot inflate -  @0x%016I64X\n", At);
      OutputDebugString(szBuffer);
      //AddError(szBuffer, 2);
#endif
#endif
      delete dst;
    }
  }
  else
    delete dst;
  //free(src);
  delete pDZ;
  return p;
}
#endif

m4DataStream *M4DGBlock::ReadStream()
{
	m4DataStream *pStream = new m4DataStream((MDF4File *)m_File,getLink(dg_data));
	if (pStream->IsValid())
		return pStream;
	delete pStream;
	return NULL;
}
#endif             // [E] READ Support


//--------------------------------------------------------------------------------
// m4DZStream - a helper class to write to a DZ-Block (4.1)
//--------------------------------------------------------------------------------

// ctor for write operations

m4DZStream::m4DZStream(M4DGBlock *dg,M_UINT32 BlkSize,M_UINT32 RecordSize,M_UINT32 dlEntries/*=5*/)
       : m4DataStream()
{
  M_LINK l_dl, hl_data;
  m_File=(MDF4File *)dg->m_File;
  m_dg=true;
  m_blkType=M4ID_DZ;
	// Create a DZ block and prepare it, do not save it (size is unknown)
	m_dz = new M4DZBlockWr(m_blkType,BlkSize);
	// Create HL block and add a DL list to it
	m_pHL = new M4HLBlock;
	if (m_pHL && m_dz)
	{
		hl_data = m_pHL->Create(m_File);
		m_pHL->hl_zip_type = 1;
		m_pHL->hl_flags = HL_F_EQUALLENGTH;
		memset(m_pHL->hl_reserved, 0, sizeof(m_pHL->hl_reserved));
		m_pHL->Save();
	  if (dlEntries<5) 
			dlEntries=5; 
    m_dl=new M4DLBlock(dlEntries,BlkSize);
    l_dl=m_dl->Reset(m_File);
	  m_pHL->setLink(m_pHL->hl_dl_first,l_dl);
		m_pHL->Save();
	  dg->setLink(dg->dg_data,hl_data);

		m_dz->dz_org_block_type[0] = 'D';
		m_dz->dz_org_block_type[1] = 'T';
		m_dz->dz_org_data_length = BlkSize;
		m_dz->dz_zip_parameter = RecordSize;
		m_dz->dz_zip_type = 1;
		m_bValid=true;
	}
  dg->Save();
}
m4DZStream::m4DZStream(M4SRBlock *sr,M_UINT32 BlkSize,M_UINT32 RecordSize,M_UINT32 dlEntries/*=5*/)
       : m4DataStream()
{
  M_LINK l_dl, hl_data;
  m_File=(MDF4File *)sr->m_File;
  m_dg=true;
  m_blkType=M4ID_DZ;
	// Create a DZ block and prepare it, do not save it (size is unknown)
	m_dz = new M4DZBlockWr(m_blkType,BlkSize);
	// Create HL block and add a DL list to it
	m_pHL = new M4HLBlock;
	if (m_pHL && m_dz)
	{
		hl_data = m_pHL->Create(m_File);
		m_pHL->hl_zip_type = 1;
		m_pHL->hl_flags = HL_F_EQUALLENGTH;
		memset(m_pHL->hl_reserved, 0, sizeof(m_pHL->hl_reserved));
		m_pHL->Save();
	  if (dlEntries<5) 
			dlEntries=5; 
    m_dl=new M4DLBlock(dlEntries,BlkSize);
    l_dl=m_dl->Reset(m_File);
	  m_pHL->setLink(m_pHL->hl_dl_first,l_dl);
		m_pHL->Save();
	  sr->setLink(sr->sr_data,hl_data);

		m_dz->dz_org_block_type[0] = 'R';
		m_dz->dz_org_block_type[1] = 'D';
		m_dz->dz_org_data_length = BlkSize;
		m_dz->dz_zip_parameter = RecordSize;
		m_dz->dz_zip_type = 1;
		m_bValid=true;
	}
  sr->Save();
}

bool m4DZStream::Write(M_UINT32 nBytes, const void *Memory)
{
  const M_BYTE *bMemory = (const M_BYTE *)Memory;
  assert(m_dg);
  assert(m_dz);
  while (nBytes)
  {
    M_UINT32 doWrite,canWrite = m_dz->CanWrite();
    doWrite = (canWrite > nBytes) ? nBytes : canWrite;
    // we need a new Block
    if (doWrite == 0) 
    {
      // without a ##DL this fails
      if (m_dl == NULL) return false;
      if(m_dl->isFull())
      {
        m_dl->Reset(NULL);
      }
      // Deflate, transpose & save
      M_SIZE szTemp;
      M_UINT32 dwSize = m_dz->getBlkSize();
      M_UINT32 dwCSize = dwSize + dwSize/1000 + 12;
      M_BYTE *dst = (M_BYTE*)malloc(dwCSize);
      M_BYTE *src = (M_BYTE *)m_dz->getData(0, szTemp);
      if (m_dz->dz_zip_type == 1) // Transpose
        Transpose(src, dwSize, m_dz->dz_org_data_length/m_dz->dz_zip_parameter);
      if (Deflate(dst, &dwCSize, src, m_dz->dz_org_data_length) == Z_STREAM_END)
      {
        m_dz->dz_data_length = dwCSize;
        m_dz->setSize(dwCSize);
        M_SIZE szMax = sizeof(m4DZRecord)+dwCSize; // no fixed part, no links
        m_dz->m_File = m_File;
        m_dz->m_At=m_File->Request(szMax, true);
        M_LINK at = m_dz->WriteHdr();
        m_File->WriteAt(at, m_dz->dz_data_length, dst);
        // and add it to the DL
        m_dl->addLink(m_dz->m_At);
        m_dl->Save();
      }
      else
      {
#if _MDF4_WIN
        OutputDebugString(_T("Invalid DZBlock: Cannot deflate\n"));
#endif
        free(dst);
        return false;
      }
      free(dst);
      // clean up block
      m_dz->Reset();
    }
    else
    {
      m_dz->DoWrite(doWrite,bMemory);
      bMemory+=doWrite; nBytes-=doWrite;
    }
  }
  return true;
}

void m4DZStream::Flush(BOOL bKeepSize /*= FALSE, ignored*/)
{
  assert(m_dg);
  if (m_dz && m_dl && m_dz->getSize()>0)
  {
    if(m_dl->isFull())
    {
      m_dl->Reset(NULL);
    }
    M_SIZE szTemp;
    M_UINT32 dwSize = m_dz->getSize();
    m_dz->dz_org_data_length = dwSize;
    M_UINT32 dwCSize = dwSize + dwSize/1000 + 12;
		M_BYTE *dst = (M_BYTE*)malloc(dwCSize);
		M_BYTE *src = (M_BYTE *)m_dz->getData(0, szTemp);
    if(m_dz->dz_zip_type == 1) // Transpose
      Transpose(src, dwSize, m_dz->dz_org_data_length/m_dz->dz_zip_parameter);
    int res = Deflate(dst, &dwCSize, src, m_dz->dz_org_data_length);
    if(res == Z_STREAM_END)
    {
      m_dz->dz_data_length = dwCSize;
      m_dz->setSize(dwCSize);
      M_SIZE szMax = sizeof(m4DZRecord)+dwCSize; // no fixed part, no links
      m_dz->m_File = m_File;
      m_dz->m_At=m_File->Request(szMax, true);
      M_LINK at = m_dz->WriteHdr();
      m_File->WriteAt(at, m_dz->dz_data_length, dst);
      // and add it to the DL
      m_dl->addLink(m_dz->m_At);
      m_dl->Save();
      m_dz->Reset();
    }
    free(dst);
  }
  m_File->FlushRecordCount();
}
void m4DZStream::Close()
{
  if (m_dg)
    Flush();
  delete m_dl; m_dl=NULL;
  delete m_dz; m_dz=NULL;
	dlEntry *p = (dlEntry *)m_List.m_t;
}


m4DataStreamEx::m4DataStreamEx(M4DGBlock *dg, M_UINT32 BlkSize, M_UINT32 dlEntries, M_UINT16 dtID) : m4DataStream()
{
	assert(dg != NULL);
	m_File = (MDF4File*)dg->m_File;
	m_bValid = true;
	m_dg = true;
	m_blkType = dtID;
	m_db = new M4DataBlock(dtID, BlkSize);
	if(dlEntries)
	{
		// Get the data list block from the data group (dg)
		m_dl = (M4DLBlock*)m_File->LoadLink(*dg, M4DGBlock::dg_data, M4ID_DL);
		// If no data list then
		if(m_dl == NULL)
		{
			// Create the new data list
			m_dl = new M4DLBlock(dlEntries, BlkSize);
			// Add the data list to the data group
			dg->setLink(dg->dg_data, m_dl->Reset(m_File));
			// Add the new data block to the data list
			M_LINK l_data = m_db->Reset(m_File);
			m_dl->addLink(l_data);
			m_dl->Save();
		}
		else // find last DL block
		{
			while (m_dl->hasLink(M4DLBlock::dl_dl_next))
			{
				M4DLBlock* p = m_dl;
				m_dl = (M4DLBlock*)m_File->LoadLink(*m_dl, M4DLBlock::dl_dl_next, M4ID_DL);
				delete p;
			}
			m_dl->setMaxLinks(dlEntries);
			// get last data block
			m4Block *data = m_File->LoadBlock(m_dl->getLink(M4DLBlock::dl_data + m_dl->dl_count-1));
			if (data)
			{
				BOOL bReUse = FALSE;
				M_INT64 diff = 0;
				M4DataBlock* db = (M4DataBlock*)data;
				M_UINT32 dwSize = db->getBlkSize();
				if (dwSize < BlkSize) // partly empty block or last block, reuse it
				{
					diff = m_File->AtEnd() - (db->m_At + db->m_Hdr.hdr_len);
					if ( diff <= 8 || diff == BlkSize-dwSize) // at the end of file or partly used?
					{
						diff = m_File->AtEnd() - db->m_At; 
						bReUse = TRUE;
					}
				}
				M_BYTE *dst = new M_BYTE[dwSize];
				if(m_File->ReadAt(db->m_At+sizeof(m4BlockHdr), dwSize, dst))
				{
					if (bReUse)
					{
						db->setBlkSize(BlkSize);
						db->ReallocBuffer(BlkSize);
						db->setSize(0);
						db->DoWrite(dwSize, dst);
						delete m_db;
						m_db = db;
						db = NULL;
						M_INT64 diff = m_db->m_At + m_db->getBlkSize() + sizeof(m4BlockHdr) - m_File->GetSize();
						if (diff > 0)
							m_File->SetEnd(diff);
					}
					else
					{
						M_LINK l_data = m_db->Reset(m_File);
						m_db->DoWrite(dwSize, dst);
						if(m_dl->dl_count)
							m_dl->setLink(M4DLBlock::dl_data + m_dl->dl_count-1, m_db->m_At);
					}
				}
				delete dst;
				if (db)
					delete db;
			}
		}
	}
	else
	{
		M_LINK l_data = m_db->Reset(m_File);
		dg->setLink(dg->dg_data, l_data);
	}
	dg->Save();
}
m4DataStreamEx::~m4DataStreamEx()
{
}

m4DZStreamEx::m4DZStreamEx(M4DGBlock *dg, M_UINT32 BlkSize, M_UINT32 recordSize, M_UINT32 dlEntries) : m4DZStream()
{
	assert(dg != NULL);
	m_File = (MDF4File*)dg->m_File;
	m_bValid = true;
	m_dg = true;
	m_blkType=M4ID_DZ;
	// Create a DZ block and prepare it, do not save it (size is unknown)
	m_dz = new M4DZBlockWr(m_blkType, BlkSize);
	m_dz->dz_org_block_type[0] = 'D';
	m_dz->dz_org_block_type[1] = 'T';
	m_dz->dz_org_data_length = BlkSize;
	m_dz->dz_zip_parameter = recordSize;
	m_dz->dz_zip_type = 1;
	M4HLBlock *hl = NULL;
	m4Block *b = m_File->LoadLink(*dg, M4DGBlock::dg_data);
	if (b == NULL)
	{
		// Create HL block and add a DL list to it
	  M_LINK l_dl, hl_data;
		m_pHL = new M4HLBlock;
		hl_data = m_pHL->Create(m_File);
		m_pHL->hl_zip_type = 1;
		m_pHL->hl_flags = HL_F_EQUALLENGTH;
		memset(m_pHL->hl_reserved, 0, sizeof(m_pHL->hl_reserved));
		m_pHL->Save();
	  if (dlEntries<5) 
			dlEntries=5; 
    m_dl=new M4DLBlock(dlEntries,BlkSize);
    l_dl=m_dl->Reset(m_File);
	  m_pHL->setLink(m_pHL->hl_dl_first,l_dl);
		m_pHL->Save();
	  dg->setLink(dg->dg_data,hl_data);
		dg->Save();
		return;
	}
	if(b->hdrID() == M4ID_HL)
	{
		hl = (M4HLBlock*)b;
		m_dl = (M4DLBlock*)m_File->LoadLink(*hl, M4HLBlock::hl_dl_first, M4ID_DL);
		while(m_dl->hasLink(M4DLBlock::dl_dl_next))
		{
			M4DLBlock *p = (M4DLBlock*)m_File->LoadLink(*m_dl, M4DLBlock::dl_dl_next, M4ID_DL);
			if(p)
			{
				delete m_dl;
				m_dl = p;
			}
		}
	}
	else if(b->hdrID() == M4ID_DL)
		m_dl = (M4DLBlock*)b;
	// If no data list then
	if(m_dl == NULL)
	{
		// Create the new data list
		m_dl = new M4DLBlock(dlEntries, BlkSize);
		// Add the data list to the data group
		dg->setLink(hl->hl_dl_first, m_dl->Reset(m_File));
	}
	else
	{
		m_dl->setMaxLinks(dlEntries);
		// get last data block
		m4Block *data = m_File->LoadLink(*m_dl, M4DLBlock::dl_data + m_dl->dl_count-1);
		if(data && data->hdrID() == M4ID_DZ && m_dz)
		{
			BOOL bReUse = FALSE;
			M_INT64 diff = 0;
			M4DZBlock *pDZ = (M4DZBlock *)data;
			M_UINT32 dwSize = pDZ->dz_org_data_length;
			if (dwSize < BlkSize) // partly empty block, can I reuse it?
			{
				diff = m_File->AtEnd() - (pDZ->m_At + pDZ->m_Hdr.hdr_len);
				if ( diff <= 8) // at the end of file ?
				{
					diff = m_File->AtEnd() - pDZ->m_At; 
					bReUse = TRUE;
				}
			}
			M_BYTE *dst = new M_BYTE[dwSize];
			M_BYTE *src = (M_BYTE *)calloc(pDZ->dz_data_length, 1);
			if(m_File->ReadAt(pDZ->m_At+sizeof(m4BlockHdr)+sizeof(m4DZRecord), (M_UINT32)pDZ->dz_data_length, (void*)src))
			{
				int res = Inflate(dst, &dwSize, src, pDZ->dz_data_length);
				if(res == Z_STREAM_END)
				{
					if(pDZ->dz_zip_type == 1) // Transpose
					{
						Transpose(dst, dwSize, pDZ->dz_zip_parameter);
					}
					if (bReUse)
						m_File->SetEnd(-diff);
					m_dz->DoWrite(dwSize, dst);
					if(m_dl->dl_count)
					{
						m_dl->setLink(M4DLBlock::dl_data + m_dl->dl_count-1, 0);
						m_dl->dl_count--;
					}
				}
				else
				{
#if MDF4_WIN
#ifndef UNICODE
					char szBuffer[128];
					sprintf(szBuffer, "Invalid DZBlock: Cannot inflate -  @0x%016I64X\n", pDZ->m_At);
					OutputDebugString(szBuffer);
					//AddError(szBuffer, 2);
#endif
#endif
				}
			}
			delete dst;
			free(src);
			delete data;
		}
	}
	if (hl)
		delete hl;
}
m4DZStreamEx::~m4DZStreamEx()
{
}



