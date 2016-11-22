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
#include <stddef.h>
#include <stdlib.h>


template<class T> class dynArray
{
public:
  dynArray(size_t init=0) : m_t(NULL),m_n(0)
  {
    add(init);
  }
  ~dynArray()
  {
    free(m_t);
		m_t = 0;
  }
  size_t getSize() const { return m_n; }
  size_t getBytes() const { return m_n*sizeof(T); }
  T *add(size_t n)
  {
    T *r=NULL;
//		if (n==0) n=1; // B.Sp.: Andernfalls wird m_t nicht mehr freigegeben und bleibt als memory leak übrig.
    if (m_n==0)
    {
			if (n==0)
				return NULL;
      return m_t=(T *)calloc(m_n=n,sizeof(T));
    }
    else
    {
      T *r=(T *)realloc(m_t, (m_n+n)*sizeof(T));
      if(r)
      {
        m_t=r;
        r=m_t+m_n; m_n+=n;
        memset(r, 0, n*sizeof(T));
        return r;
      }
      else
      {
        free(m_t);
        m_t = NULL;
        return NULL;
      }
    }
  }
  T *get(size_t index=0)
  {
    if (index>=m_n)
      add(index+1-m_n);
    return m_t+index;
  }
  void *resize(size_t nBytes)
  {
    m_n=0;
    free(m_t);
    m_t=NULL;
    if (nBytes)
    {
      nBytes+=sizeof(T)-1;
      m_n=nBytes/sizeof(T);
      m_t=(T *)calloc(m_n,sizeof(T));
    }
    return m_t;
  }
  size_t m_n;
  T  * m_t;
};
