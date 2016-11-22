#pragma once
#include "mdf4.h"
#include "Ptrlist.h"
//#include <conio.h>

#if !_MDF4_DUMP
#error please #define _MDF4_DUMP 1
#endif

#if _MDF4_WONLY
#error DUMP is impossible with WONLY
#endif

#if (!_MDF4_WIN)
#define LPCTSTR const char*
#endif
class m4TreeItem;
typedef ptrlist<m4TreeItem> m4Tree;
class m4TreeItem
{
public:
  m4TreeItem(LPCTSTR Text=NULL)
  {
		*m_Text = 0;
    *m_Definition = 0;
  }
  virtual ~m4TreeItem()
  {
  }
	virtual void Show()
	{
    printf("%s\n",m_Text);
    printf("%s\nPress ENTER..",m_Definition);
		getc(stdin);
		printf("\n");
    for(m4Tree::iterator s=m_Tree; s; ++s)
      s->Show();
	}

  virtual m4TreeItem *GetFile()
  {
    return NULL;
  }
  M_CHAR m_Text[1024];
  M_CHAR m_Definition[4096];
  m4Tree    m_Tree;
};
class m4TreeFile : public m4TreeItem
{
public:
  m4TreeFile(LPCTSTR Title) : m4TreeItem(Title)
  {
  }
  virtual ~m4TreeFile()
  {
  }
  virtual m4TreeItem *GetFile()
  {
    return this;
  }
  bool Open(LPCTSTR Path);
  MDF4File m_m4;
};
class m4TreeLink : public m4TreeItem
{
public:
  m4TreeLink(LPCTSTR linkNam,M_LINK linkAddr)
  {
    sprintf(m_Text,"%s=%016I64X",linkNam,linkAddr);
  }
};
