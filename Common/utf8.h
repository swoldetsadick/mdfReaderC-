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
#ifndef _UTF8_H_
#define _UTF8_H_

#if !_MDF4_WIN
#ifdef UNICODE
#define TCHAR wchar_t
#else
#define TCHAR char
#endif
#include <stdio.h>
#endif

// byte_buffer class is an in-memory dynamic buffer implementation.
template <class T>  class tBuffer
{
  T*  _body;
  size_t          _allocated;
  size_t          _size;
public:
  T *reserve(size_t size)
  {
    size_t newsize = _size + size;
    if( newsize > _allocated )
    {
      _allocated *= 2;
      if(_allocated < newsize)
         _allocated = newsize;
      T *newbody = new T[_allocated];
      memcpy(newbody,_body,_size*sizeof(T));
      delete[] _body;
      _body = newbody;
    }
    return _body + _size;
  }
   tBuffer() : _size(0)
   {
     _body = new T[_allocated = 256];
   }
   ~tBuffer()
   {
     delete[] _body;
   }
   void reset()
   {
     _size=0;
     if (_allocated)
       memset(_body,0,_allocated*sizeof(T));
   }
   T *init(size_t bufSize)
   {
     size_t size=(bufSize+sizeof(T)-1)/sizeof(T);
     if (size>_allocated)
     {
       delete[] _body;
       _allocated=size;
       _body=new T[_allocated];
     }
     _size=size-1;
     return _body;
   }
   const T * data()
   {
     if(_size == _allocated) reserve(1);
     _body[_size] = '\0';
     return _body;
   }
   size_t length() const         { return _size; }
   void push(T c)    { *reserve(1) = c; ++_size; }
   void push(const T *pc, size_t sz)
   {
     memcpy(reserve(sz),pc,sz*sizeof(T));
     _size += sz;
   }
};

typedef tBuffer<unsigned char> byte_buffer;
typedef tBuffer<wchar_t> wc_buffer;

class Indent
{
public:
  Indent(int n=0,int i=2) : m_n(n),m_i(i)
  {
  }
  Indent(const Indent &i)
  {
    m_n=i.m_n;
    m_i=i.m_i;
  }
  Indent& operator++()
  {
    m_n+=m_i;
    return *this;
  }
  Indent operator++(int)
  {
    Indent r(*this);
    m_n+=m_i;
    return r;
  }
  int width() const
  {
    return m_n;
  }
protected:
  int m_n;
  int m_i;
};

class utf8out : public byte_buffer
{
public:
  void cvt(wchar_t c)
  {
    if (c < (1 << 7))
    {
       push ((unsigned char)c);
    }
    else
#if 0 // (sizeof wchar_t)>2
      if (c < (1 << 11))
#endif
    {
      push ((c >> 6) | 0xc0);
      push ((c & 0x3f) | 0x80);
    }
#if 0 // sizeof(wchar_t)>2
    else if (c < (1 << 16))
    {
      push ((c >> 12) | 0xe0);
      push (((c >> 6) & 0x3f) | 0x80);
      push ((c & 0x3f) | 0x80);
    }
    else if (c < (1 << 21))
    {
      push ((c >> 18) | 0xe0);
      push (((c >> 12) & 0x3f) | 0x80);
      push (((c >> 6) & 0x3f) | 0x80);
      push ((c & 0x3f) | 0x80);
    }
#endif
  }
  utf8out& operator << (const Indent &i)
  {
    int n=i.width();
    while (n--)
      cvt(_T(' '));
    return *this;
  }
  utf8out& operator << (wchar_t c)
  {
    cvt(c);
    return *this;
  }
  utf8out& operator << (const wchar_t *pc)
  {
    if (pc)
      while (*pc)
        cvt(*pc++);
    return *this;
  }
  utf8out& operator << (const char *pc)
  {
    if (pc)
      while (*pc)
        push(*pc++);
    return *this;
  }
#ifndef WIN32
char *itoa(int val, char *pszBuffer, int base)
{
  sprintf(pszBuffer,"%d",val);
  return pszBuffer;
}
#endif
  utf8out& operator << (long lv)
  {
    TCHAR achBuffer[20],*pc;
#ifdef WIN32
    _itot_s(lv,achBuffer,20,10);
#else
    itoa(lv,achBuffer,10);
#endif
    pc=achBuffer;
    if (pc)
      while (*pc)
        cvt(*pc++);
    return *this;
  }
  utf8out& operator << (double dv)
  {
    TCHAR achBuffer[50],*pc;
#ifdef WIN32
#ifdef UNICODE
    wsprintf(achBuffer,_T("%g"),dv);
#else
		sprintf(achBuffer, _T("%g"), dv);
#endif
#else
    sprintf(achBuffer,_T("%g"),dv);
#endif
    pc=achBuffer;
    if (pc)
      while (*pc)
        cvt(*pc++);
    return *this;
  }
  utf8out& eol()
  {
    cvt(_T('\r'));
    cvt(_T('\n'));
    return *this;
  }
};
class utf8cvt
{
public:
  utf8cvt()
  {
    clear();
  }
  void clear()
  {
    _lead=0;
    _blank=L' ';
  }
  bool feed(unsigned char in,wchar_t &out)
  {
    if (_lead)
    {
      if ((in&0xC0)!=0x80)
      {
        _lead=0;
        out=_blank;
        return true;
      }
      else
      {
        --_lead;
        _clead|=(in&0x3F)<<(_lead*6);
        if (!_lead)
        {
          if (_clead>0xFFFF)
          {
            out=_blank;
          }
          else
          {
            out=(wchar_t)_clead;
          }
          return true;
        }
      }
    }
    else if ((in&0x80)==0)
    {
      out=(wchar_t)in;
      return true;
    }
    else switch (in&0xF0)
    {
    case 0xF0: _lead=3; _clead=(in&0x7)<<18; break;
    case 0xE0: _lead=2; _clead=(in&0xF)<<12; break;
    case 0xC0: _lead=1; _clead=(in&0x1F)<<6; break;
    default:   _lead=0; out=_blank; return true;
    }
    return false;
  }
protected:
  int _lead;
  unsigned long _clead;
  wchar_t _blank;
};

class utf8in : public utf8cvt,public wc_buffer
{
public:
  utf8in()
  {
  }
  utf8in(const unsigned char *buf)
  {
    while (*buf)
      put(*buf++);
    put(0);
  }
  void put(const unsigned char val)
  {
    wchar_t out;
    if (feed(val,out))
    {
      if (out==L'\n')
        push(L'\r');
      push(out);
    }
  }
  void put(const unsigned char *buf,size_t size)
  {
    if (buf) while(size--)
    {
      put(*buf++);
    }
  }
  utf8in& operator << (const unsigned char val)
  {
    put(val);
    return *this;
  }
};

#endif  // _UTF8_H_
