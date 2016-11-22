#ifndef _PTRLIST_H_
#define _PTRLIST_H_

// TEMPLATE CLASS ptrlist
template<class T> 
  class ptrlist
{
protected:
	class _Node 
  {
  public:
		_Node *_Next,*_Prev;
    bool _Owns;
		T  *_Value;
    _Node(T *_V,bool _O) // : _Next(*q) . _Owns(_O) , _Value(_V)
    { 
      _Next=_Prev=NULL;
      _Owns=_O; _Value=_V; 
    }
    _Node(_Node *p,T *_V,bool _O) // : _Next(*q) . _Owns(_O) , _Value(_V)
    { _Prev=p;
      if ((_Next=p->_Next)!=NULL)
      {
        _Next->_Prev=this;
      }
      p->_Next=this;
      _Owns=_O; _Value=_V; 	
    }
    _Node(T *_V,_Node *n,bool _O) // : _Next(*q) . _Owns(_O) , _Value(_V)
    { _Prev=NULL;
      if ((_Next=n)!=NULL)
      {
        _Next->_Prev=this;
      }
      _Owns=_O; _Value=_V; 	
    }
    ~_Node()
    {
      if (_Owns) delete _Value;
      if (_Next) delete _Next;			
    }
  };
	friend class _Node;
public:
	typedef ptrlist<T> _Myt;
		// CLASS const_iterator
	class iterator 
  {
	public:
		iterator(_Node * _P=NULL)	: _Ptr(_P) {}
    iterator(const _Myt& _Lst) : _Ptr(_Lst._Head) {}
    iterator &operator=(const _Myt& _Lst)
    {
      _Ptr=_Lst._Head;
      return *this;
    }
		T * operator->() const
    {return (_Ptr ? _Ptr->_Value : NULL); }
		iterator& operator++()
    { _Ptr = _Ptr ? _Ptr->_Next : NULL;
			return (*this); 
    }
		iterator& operator--()
    {
      _Ptr = _Ptr ? _Ptr->_Prev : NULL;
			return (*this); 
    }
		iterator operator++(int)
    {
      iterator t(_Ptr);
      _Ptr= _Ptr ? _Ptr->_Next : NULL;
			return t; 
    }
		iterator operator--(int)
    {
      iterator t(_Ptr);
      _Ptr= _Ptr ? _Ptr->_Prev : NULL;
			return t; 
    }
		bool operator==(const iterator& _X) const
			{return (_Ptr == _X._Ptr); }
		bool operator!=(const iterator& _X) const
			{return (!(*this == _X)); }
    operator bool() const { return _Ptr!=NULL; }
    bool operator!() const { return _Ptr==NULL; } 
    operator T *() const { return _Ptr ? _Ptr->_Value : NULL; }
    _Node *GetPtr() { return _Ptr; }
	protected:
		_Node * _Ptr;
		};
	friend class iterator;
  ptrlist(bool _O=true) // : _Head(NULL) : _Size(0): _Owns(_O) {}
  {
    _Head=NULL; _Size=0; _Owns=_O;
  }
  ~ptrlist() { clear(); }
	iterator top()	{return (iterator(_Head)); }
  iterator find(T *_X)
  {
    iterator it=top();
    while ((T *)it && it!=_X) it++;
    return it;
  }
  iterator operator[](int ix)
  {
    iterator i=top();
    while ((T *)i && ix--) ++i;
    return i;
  }
  iterator end()
  {
    iterator i=top();
    if (i)
    {
      int ix=_Size-1;
      while ((T *)i && ix--) ++i;
    }
    return i;
  }

	int size() const {return (_Size); }
	bool empty() const {return (size() == 0); }
  T* push(T *_X) { return push(_X,_Owns); }
  T* push(T *_X,bool _O)
  {
    if (!_Size++) _Head=new _Node(_X,_O);
    else _Head=new _Node(_X,_Head,_O);
    return _X;
  }
  T* append(T *_X) { return append(_X,_Owns); }
	T* append(T *_X,bool _O)
	{
    if (!_Size++) _Head=new _Node(_X,_O);
    else
    {
      _Node *p;
      for(p=_Head;p->_Next;p=p->_Next);
      new _Node(p,_X,_O);
    }
    return _X;
  }
  T * insert_before(iterator i,T *_X,bool _O)
  {
    if (i) --i;
    if (i)
    {
      ++_Size;
      new _Node(i.GetPtr(),_X,_O);
      return _X;
    }
    else 
      return push(_X,_O);
  } 
  T * insert_before(iterator i,T *_X) { return insert_before(i,_X,_Owns); }
  T * insert_after(iterator i,T *_X) { return insert_after(i,_X,_Owns); }
  T * insert_after(iterator i,T *_X,bool _O)
  {
    if (i)
    {
      ++_Size;
      new _Node(i.GetPtr(),_X,_O);
      return _X;
    }
    else 
     return append(_X,_O);
  }
  T * replace(iterator i,T *_X,bool _D=false)
  {
    if (i)
    {
      _Node *n=i.GetPtr();
      if (n->_Owns && _D)
        delete n->_Value;
      n->_Value=_X;
    }
    return _X;
  }
  void remove(iterator it,bool _D=false)
  {
    _Node *n=it.GetPtr();
    if (n)
    {
      if (n==_Head)
      {
        _Head=_Head->_Next;
        if (_Head)
          _Head->_Prev=NULL;
      }
      else 
      {
        if (n->_Prev) n->_Prev->_Next=n->_Next;
        if (n->_Next) n->_Next->_Prev=n->_Prev;
      }
      n->_Next=NULL;
      if (_D) n->_Owns=false;
      delete n;
      --_Size;
    }
  }
  void remove(T *_X,bool _D=false)
  {
    remove(find(_X),_D);
  }
  void pop()
  {
     remove(top());
  }

  T* RemoveTop()
  {
    T* pRes=top();
    // alt: if (pRes) pop();
    if (pRes) remove(top(),true);
    return pRes;
  }

	void clear()
	{ delete _Head; _Head=NULL; _Size=0; }
protected:
	_Node * _Head;
	int     _Size;
  bool    _Owns;
	};

// TEMPLATE CLASS ptrlist
template<class T> 
  class DbtObjPtrList
{
protected:
	class _Node 
  {
  public:
		_Node *_Next,*_Prev;
    bool _Owns;
		T  *_Value;
    _Node(T *_V,bool _O) // : _Next(*q) . _Owns(_O) , _Value(_V)
    { 
      _Next=_Prev=NULL;
      _Owns=_O; _Value=_V; 
    }
    _Node(_Node *p,T *_V,bool _O) // : _Next(*q) . _Owns(_O) , _Value(_V)
    { _Prev=p;
      if ((_Next=p->_Next)!=NULL)
      {
        _Next->_Prev=this;
      }
      p->_Next=this;
      _Owns=_O; _Value=_V; 	
    }
    _Node(T *_V,_Node *n,bool _O) // : _Next(*q) . _Owns(_O) , _Value(_V)
    { _Prev=NULL;
      if ((_Next=n)!=NULL)
      {
        _Next->_Prev=this;
      }
      _Owns=_O; _Value=_V; 	
    }
    ~_Node()
    {
			/*
      if (_Owns) delete _Value;
			*/
			if (_Owns) _Value->Release();
      //if (_Next) delete _Next;			
    }
  };
	friend class _Node;
public:
	typedef DbtObjPtrList<T> _Myt;
		// CLASS const_iterator
	class iterator 
  {
	public:
		iterator(_Node * _P=NULL)	: _Ptr(_P) {}
    iterator(const _Myt& _Lst) : _Ptr(_Lst._Head) {}
    iterator &operator=(const _Myt& _Lst)
    {
      _Ptr=_Lst._Head;
      return *this;
    }
		T * operator->() const
    {return (_Ptr ? _Ptr->_Value : NULL); }
		iterator& operator++()
    { _Ptr = _Ptr ? _Ptr->_Next : NULL;
			return (*this); 
    }
		iterator& operator--()
    {
      _Ptr = _Ptr ? _Ptr->_Prev : NULL;
			return (*this); 
    }
		iterator operator++(int)
    {
      iterator t(_Ptr);
      _Ptr= _Ptr ? _Ptr->_Next : NULL;
			return t; 
    }
		iterator operator--(int)
    {
      iterator t(_Ptr);
      _Ptr= _Ptr ? _Ptr->_Prev : NULL;
			return t; 
    }
		bool operator==(const iterator& _X) const
			{return (_Ptr == _X._Ptr); }
		bool operator!=(const iterator& _X) const
			{return (!(*this == _X)); }
    operator bool() const { return _Ptr!=NULL; }
    bool operator!() const { return _Ptr==NULL; } 
    operator T *() const { return _Ptr ? _Ptr->_Value : NULL; }
    _Node *GetPtr() { return _Ptr; }
	protected:
		_Node * _Ptr;
		};
	friend class iterator;
  DbtObjPtrList(bool _O=true) // : _Head(NULL) : _Size(0): _Owns(_O) {}
  {
    _Head=NULL; _Size=0; _Owns=_O;
  }
  ~DbtObjPtrList() { clear(); }
	iterator top()	{return (iterator(_Head)); }
  iterator find(T *_X)
  {
    iterator it=top();
    while ((T *)it && it!=_X) it++;
    return it;
  }
  iterator operator[](int ix)
  {
    iterator i=top();
    while ((T *)i && ix--) ++i;
    return i;
  }
  iterator end()
  {
    iterator i=top();
    if (i)
    {
      int ix=_Size-1;
      while ((T *)i && ix--) ++i;
    }
    return i;
  }

	int size() const 
  {
    return (_Size); 
  }
	bool empty() const 
  {
    return (size() == 0); 
  }
  T* push(T *_X) { return push(_X,_Owns); }
  T* push(T *_X,bool _O)
  {
    if (!_Size++) _Head=new _Node(_X,_O);
    else _Head=new _Node(_X,_Head,_O);
    return _X;
  }
  T* append(T *_X) { return append(_X,_Owns); }
	T* append(T *_X,bool _O)
	{
    if (!_Size++) _Head=new _Node(_X,_O);
    else
    {
      _Node *p;
      for(p=_Head;p->_Next;p=p->_Next);
      new _Node(p,_X,_O);
    }
    return _X;
  }
  T * insert_before(iterator i,T *_X,bool _O)
  {
    if (i) --i;
    if (i)
    {
      ++_Size;
      new _Node(i.GetPtr(),_X,_O);
      return _X;
    }
    else 
      return push(_X,_O);
  } 
  T * insert_before(iterator i,T *_X) { return insert_before(i,_X,_Owns); }
  T * insert_after(iterator i,T *_X) { return insert_after(i,_X,_Owns); }
  T * insert_after(iterator i,T *_X,bool _O)
  {
    if (i)
    {
      ++_Size;
      new _Node(i.GetPtr(),_X,_O);
      return _X;
    }
    else 
     return append(_X,_O);
  }
  T * replace(iterator i,T *_X,bool _D=false)
  {
    if (i)
    {
      _Node *n=i.GetPtr();
      if (n->_Owns && _D)
				/*
        delete n->_Value;
				*/
				n->_Value.Release();
      n->_Value=_X;
    }
    return _X;
  }
  void remove(iterator it,bool _D=false)
  {
    _Node *n=it.GetPtr();
    if (n)
    {
      if (n==_Head)
      {
        _Head=_Head->_Next;
        if (_Head)
          _Head->_Prev=NULL;
      }
      else 
      {
        if (n->_Prev) n->_Prev->_Next=n->_Next;
        if (n->_Next) n->_Next->_Prev=n->_Prev;
      }
      n->_Next=NULL;
      if (_D) n->_Owns=false;
      delete n;
      --_Size;
    }
  }
  void remove(T *_X,bool _D=false)
  {
    remove(find(_X),_D);
  }
  void pop()
  {
     remove(top());
  }

  T* RemoveTop()
  {
    T* pRes=top();
    if (pRes) pop();
    return pRes;
  }

	void clear()
	//{ delete _Head; _Head=NULL; _Size=0; }
  { 
    while( ! empty() ) 
      pop(); 
  }
protected:
	_Node * _Head;
	int     _Size;
  bool    _Owns;
	};

#endif

