/* === E T L =============================================================== */
/*!	\file _handle.h
**	$Id$
**	\brief Template Object Handle Implementation
**	\internal
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**	Copyright (c) 2007, 2008 Chris Moore
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**	General Public License for more details.
**	\endlegal
**
**	\note
**		This is an internal header file, included by other ETL headers.
**		You should not attempt to use it directly.
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__HANDLE_H
#define __ETL__HANDLE_H

/* === H E A D E R S ======================================================= */

#include <cassert>
#include <typeinfo>
#include <atomic>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

#define ETL_SELF_DELETING_SHARED_OBJECT

/* === C L A S S E S & S T R U C T S ======================================= */

#ifdef NDEBUG
#define assert_cast		static_cast
#else
#define assert_cast		dynamic_cast
#endif


namespace etl {

// Forward Declarations
template <class T> class handle;
template <class T> class loose_handle;
template <class T> class rhandle;


// ========================================================================
/*!	\class	shared_object _handle.h	ETL/handle
**	\brief	Shared Object Base Class
**	\see handle, loose_handle
**	\writeme
*/
class shared_object
{
private:
	mutable std::atomic<int> refcount;

protected:
	shared_object():refcount(0) { }
	shared_object(const shared_object&):refcount(0) { }
	shared_object& operator= (const shared_object&) { return *this; }

#ifdef ETL_SELF_DELETING_SHARED_OBJECT
	virtual ~shared_object() { }
#else
	~shared_object() { }
#endif

public:
	virtual void ref()const
	{
		++refcount;
	}

	//! Returns \c false if object needs to be deleted
	virtual bool unref()const
	{
		bool ret = (bool)(--refcount);
#ifdef ETL_SELF_DELETING_SHARED_OBJECT
		if (!ret)
			delete this;
#endif
		return ret;
	}

	//! Decrease reference counter without deletion of object
	//! Returns \c false if references exceed and object should be deleted
	virtual bool unref_inactive()const
	{
		return (bool)(--refcount);
	}

	int count()const { return refcount; }

}; // END of class shared_object

// ========================================================================
/*!	\class	virtual_shared_object _handle.h	ETL/handle
**	\brief	Virtual Shared Object Base Class
**	\see handle, loose_handle
**	\writeme
*/
class virtual_shared_object
{
protected:
	virtual_shared_object() { }
	virtual_shared_object(const virtual_shared_object&) { }
	virtual_shared_object& operator= (const virtual_shared_object&) { return *this; }
public:
	virtual ~virtual_shared_object() { }
	virtual void ref()const=0;
	virtual bool unref()const=0;
	virtual bool unref_inactive()const=0;
	virtual int count()const=0;
}; // END of class virtual_shared_object

// ========================================================================
/*!	\class	handle _handle.h	ETL/handle
**	\brief	Object Handle
**	\see shared_object, loose_handle
**	\writeme
*/
template <class T>
class handle
{
public:

	typedef T value_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef int count_type;
	typedef int size_type;

protected:
#ifdef _DEBUG
public:
#endif
	value_type *obj;		//!< Pointer to object

public:

	//! Default constructor - empty handle
	handle():obj(NULL) {}

	//! Constructor that constructs from a pointer to new object
	handle(pointer x):obj(x)
	{
		if(obj)
			obj->ref();
	}

	//! Default copy constructor
	handle(const handle<value_type> &x):obj(x.get())
	{
		if(obj)
			obj->ref();
	}

	//! Handle is released on deletion
	~handle() { detach(); }

	//! Template Assignment operator
	/*! \note This class may not be necessary, and may be removed
	**		at some point in the future.
	*/
	/*
	template <class U> handle<value_type> &
	operator=(const handle<U> &x)
	{
		if(x.get()==obj)
			return *this;

		detach();

		obj=static_cast<value_type*>(x.get());
		if(obj)obj->ref();
		return *this;
	}
	*/

	//! Assignment operator
	handle<value_type> &
	operator=(const handle<value_type> &x)
	{
		if(x.get()==obj)
			return *this;
		// add reference before detach
		pointer xobj(x.get());
		if(xobj) xobj->ref();
		detach();
		obj=xobj;
		return *this;
	}

	//! Swaps the values of two handles without reference counts
	handle<value_type> &
	swap(handle<value_type> &x)
	{
		pointer ptr=x.obj;
		x.obj=obj;
		obj=ptr;
		return *this;
	}

	//! Handle detach procedure
	/*! unref()'s the object and sets the internal object pointer to \c NULL */
	void
	detach()
	{
		pointer xobj(obj);
		obj=0;
#ifdef ETL_SELF_DELETING_SHARED_OBJECT
		if(xobj)
			xobj->unref();
#else
		if(xobj && !xobj->unref())
			delete xobj;
#endif
	}

	// This will be reintroduced with a new function
	//void release() { detach(); }

	void reset() { detach(); }

	bool empty()const { return obj==0; }

	//! Creates a new instance of a T object and puts it in the handle.
	/*! Uses the default constructor */
	void spawn() { operator=(handle(new T())); }

	//! Returns a constant handle to our object
	handle<const value_type> constant()const { assert(obj); return *this; }

	//! Returns number of instances
	count_type
	count()const
		{ return obj?obj->count():0; }

	//! Returns true if there is only one instance of the object
	bool
	unique()const
		{ assert(obj); return count()==1; }

	reference
	operator*()const
		{ assert(obj); return *obj; }

	pointer
	operator->()const
		{ assert(obj); return obj; }

	//! More explicit bool cast
	operator bool()const
		{ return obj!=NULL; }

	operator handle<const value_type>()const
	{ return handle<const value_type>(static_cast<const_pointer>(obj)); }

	//! <tt> static_cast\<\> </tt> wrapper
	template <class U> static handle<T> cast_static		(const handle<U> &x) { return handle<T>(static_cast		<T*>(x.get())); }
	//! <tt> dynamic_cast\<\> </tt> wrapper
	template <class U> static handle<T> cast_dynamic	(const handle<U> &x) { return handle<T>(dynamic_cast	<T*>(x.get())); }
	//! <tt> const_cast\<\> </tt> wrapper
	template <class U> static handle<T> cast_const		(const handle<U> &x) { return handle<T>(const_cast		<T*>(x.get())); }
	//! <tt> reinterpret_cast\<\> </tt> wrapper
	template <class U> static handle<T> cast_reinterpret(const handle<U> &x) { return handle<T>(reinterpret_cast<T*>(x.get())); }

	template <class U> static handle<T> cast_static		(const loose_handle<U> &x);
	template <class U> static handle<T> cast_dynamic	(const loose_handle<U> &x);
	template <class U> static handle<T> cast_const		(const loose_handle<U> &x);
	template <class U> static handle<T> cast_reinterpret(const loose_handle<U> &x);

	template <class U> static handle<T> cast_static		(const rhandle<U> &x);
	template <class U> static handle<T> cast_dynamic	(const rhandle<U> &x);
	template <class U> static handle<T> cast_const		(const rhandle<U> &x);
	template <class U> static handle<T> cast_reinterpret(const rhandle<U> &x);

	template <class U> static handle<T> cast_static		(U* x);
	template <class U> static handle<T> cast_dynamic	(U* x);
	template <class U> static handle<T> cast_const		(U* x);
	template <class U> static handle<T> cast_reinterpret(U* x);

	//! Returns pointer to the object that is being wrapped
	pointer get()const { return obj; }

	bool
	operator!()const
		{ return !obj; }

	//! static_cast<> overload -- Useful for implicit casts
	template <class U>
	operator handle<U>()const
	{ return handle<U>(obj); }

	template<typename U>
	bool type_is() const
	{ return dynamic_cast<const U*>(obj); }

	template<typename U>
	U* type_pointer() const
	{ return dynamic_cast<U*>(obj); }

	template<typename U>
	bool type_equal() const
	{ return typeid(*obj) == typeid(U); }
}; // END of template class handle

// ========================================================================
/*!	\class	rshared_object _handle.h	ETL/handle
**	\brief	Replaceable Shared Object Base Class
**	\see rhandle
**	\writeme
*/
class rshared_object : public shared_object
{
private:
	mutable int rrefcount;

public:
	void *front_;
	void *back_;

protected:
	rshared_object():rrefcount(0),front_(0),back_(0) { }
	rshared_object(const rshared_object &other): shared_object(other), rrefcount(0),front_(0),back_(0) { }
	rshared_object& operator= (const rshared_object&) { return *this; }

public:
	virtual void rref()const
		{ rrefcount++; }

	virtual void runref()const
	{
		assert(rrefcount>0);
		rrefcount--;
	}

	int rcount()const
		{ return rrefcount; }
}; // END of class rshared_object

// ========================================================================
/*!	\class	rhandle _handle.h	ETL/handle
**	\brief	Replaceable Object Handle
**	\see rshared_object, handle, loose_handle
**	\writeme
*/
template <class T>
class rhandle : public handle<T>
{
	friend class rshared_object;
public:

	typedef T value_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef int count_type;
	typedef int size_type;


	using handle<value_type>::count;
	using handle<value_type>::unique;
	using handle<value_type>::operator bool;
	using handle<value_type>::get;
	using handle<value_type>::operator*;
	using handle<value_type>::operator->;

	/*
	operator const handle<value_type>&()const
	{ return *this; }
	*/

private:
	using handle<value_type>::obj;

	rhandle<value_type> *prev_;
	rhandle<value_type> *next_;

	void add_to_rlist()
	{
//		value_type*& obj(handle<T>::obj); // Required to keep gcc 3.4.2 from barfing

		assert(obj);
		obj->rref();

		// If this is the first reversible handle
		if(!obj->front_)
		{
			obj->front_=obj->back_=this;
			prev_=next_=0;
			return;
		}

		prev_=reinterpret_cast<rhandle<value_type>*>(obj->back_);
		next_=0;
		prev_->next_=this;
		obj->back_=this;
	}

	void del_from_rlist()
	{
//		value_type*& obj(handle<T>::obj); // Required to keep gcc 3.4.2 from barfing
		assert(obj);
		obj->runref();

		// If this is the last reversible handle
		if(obj->front_==obj->back_)
		{
			obj->front_=obj->back_=0;
			prev_=next_=0;
			return;
		}

		if(!prev_)
			obj->front_=(void*)next_;
		else
			prev_->next_=next_;

		if(!next_)
			obj->back_=(void*)prev_;
		else
			next_->prev_=prev_;
	}

public:

	//! Default constructor - empty handle
	rhandle() {}

	//! Constructor that constructs from a pointer to new object
	rhandle(pointer x):handle<T>(x)
	{
//		value_type*& obj(handle<T>::obj); // Required to keep gcc 3.4.2 from barfing
		if(obj)add_to_rlist();
	}

	rhandle(const handle<value_type> &x):handle<T>(x)
	{
//		value_type*& obj(handle<T>::obj); // Required to keep gcc 3.4.2 from barfing
		if(obj)add_to_rlist();
	}

	//! Default copy constructor
	rhandle(const rhandle<value_type> &x):handle<T>(x)
	{
//		value_type*& obj(handle<T>::obj); // Required to keep gcc 3.4.2 from barfing
		if(obj)add_to_rlist();
	}

	//! Handle is released on deletion
	~rhandle() { detach(); }

	//! Template Assignment operator
	/*! \note This class may not be necessary, and may be removed
	**		at some point in the future.
	*/
	/*
	template <class U> const handle<value_type> &
	operator=(const handle<U> &x)
	{
		if(x.get()==obj)
			return *this;

		detach();

		obj=static_cast<value_type*>(x.get());
		if(obj)
		{
			obj->ref();
			add_to_rlist();
		}
		return *this;
	}
	*/

	//! Assignment operator
	rhandle<value_type> &
	operator=(const rhandle<value_type> &x)
	{
//		value_type*& obj(handle<T>::obj); // Required to keep gcc 3.4.2 from barfing
		if(x.get()==obj)
			return *this;

		detach();

		obj=x.get();
		if(obj)
		{
			obj->ref();
			add_to_rlist();
		}
		return *this;
	}

	rhandle<value_type>&
	operator=(const handle<value_type> &x)
	{
//		value_type*& obj(handle<T>::obj); // Required to keep gcc 3.4.2 from barfing
		if(x.get()==obj)
			return *this;

		detach();

		obj=x.get();
		if(obj)
		{
			obj->ref();
			add_to_rlist();
		}
		return *this;
	}

	rhandle<value_type>&
	operator=(value_type* x)
	{
//		value_type*& obj(handle<T>::obj); // Required to keep gcc 3.4.2 from barfing
		if(x==obj)
			return *this;

		detach();

		obj=x;
		if(obj)
		{
			obj->ref();
			add_to_rlist();
		}
		return *this;
	}

	//! Handle release procedure
	/*! unref()'s the object and sets the internal object pointer to \c NULL */
	void
	detach()
	{
//		value_type*& obj(handle<T>::obj); // Required to keep gcc 3.4.2 from barfing
		if(obj)del_from_rlist();
		handle<value_type>::detach();
		obj=0;
	}

	// This will be reintroduced with a new function
	//void release() { detach(); }

	void reset() { detach(); }

	//! Creates a new instance of a T object and puts it in the handle.
	/*! Uses the default constructor */
	void spawn() { operator=(handle<value_type>(new T())); }

	//! Returns number of reversible instances
	count_type
	rcount()const
	{
//		value_type*const& obj(handle<T>::obj); // Required to keep gcc 3.4.2 from barfing
		return obj?obj->rcount():0;
	}

	//! Returns true if there is only one instance of the object
	bool
	runique()const
	{
//		value_type*& obj(handle<T>::obj); // Required to keep gcc 3.4.2 from barfing
		assert(obj); return obj->front_==obj->back_;
	}

	//! \writeme
	int replace(const handle<value_type> &x)
	{
//		value_type*& obj(handle<T>::obj); // Required to keep gcc 3.4.2 from barfing
		assert(obj);
		assert(x.get()!=obj);

		if(x.get()==obj)
			return 0;

		rhandle<value_type> *iter;
		rhandle<value_type> *next;

		iter=reinterpret_cast<rhandle<value_type>*>(obj->front_);

		assert(iter);

		next=iter->next_;

		int i=0;
		#ifndef NDEBUG
		pointer obj_=obj;
		#endif

		for(;iter;iter=next,next=iter?iter->next_:0,i++)
		{
			assert(iter->get()==obj_);
			(*iter)=x;
		}

		assert(obj==x.get());

		return i;
	}

	//! Swaps the values of two handles without reference counts
	/*!	\warning not yet implemented. \writeme */
	handle<value_type> &
	swap(handle<value_type> &x);
	/*
	{
		assert(0);
		pointer ptr=x.obj;
		x.obj=obj;
		obj=ptr;
		return *this;
	}
	*/
}; // END of template class rhandle


// ========================================================================
/*!	\class	loose_handle _handle.h	ETL/handle
**	\brief	Loose Object Handle
**	\see shared_object, handle
**	\writeme
*/
template <class T>
class loose_handle
{
public:

	typedef T value_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef int count_type;
	typedef int size_type;

protected:
#ifdef _DEBUG
public:
#endif
	value_type *obj;		//!< Pointer to object

public:

	//! Default constructor - empty handle
	loose_handle():obj(0) {}

	//! Constructor that constructs from a pointer to new object
	loose_handle(pointer x):obj(x) { }

	//! Default copy constructor
	loose_handle(const loose_handle<value_type> &x):obj(x.get()) { }

	loose_handle(const handle<value_type> &x):obj(x.get()) { }

	template <class U> const loose_handle<value_type> &
	operator=(const handle<U> &x)
	{
		if(x.get()==obj)
			return *this;

		obj=x.get();
		return *this;
	}

	template <class U> const loose_handle<value_type> &
	operator=(const loose_handle<U> &x)
	{
		if(x.get()==obj)
			return *this;

		obj=x.get();
		return *this;
	}

	//! Assignment operator
	const loose_handle<value_type> &
	operator=(const loose_handle<value_type> &x)
	{
		if(x.get()==obj)
			return *this;

		obj=x.get();
		return *this;
	}

	//! Swaps the values of two handles without reference counts
	loose_handle<value_type> &
	swap(loose_handle<value_type> &x)
	{
		pointer ptr=x.obj;
		x.obj=obj;
		obj=ptr;
		return *this;
	}

	//! Handle release procedure
	void detach() { obj=0;	}

	// This will be reintroduced with a new function
	//void release() { detach(); }

	void reset() { detach(); }

	bool empty()const { return obj==0; }

	//! Returns a constant handle to our object
	loose_handle<const value_type> constant()const { return *this; }

	//! Returns number of instances
	count_type
	count()const
		{ return obj?obj->count():0; }

	reference
	operator*()const
		{ assert(obj); return *obj; }

	pointer
	operator->()const
		{ assert(obj); return obj; }

	//! static_cast<> overload
	//template <class U>
	//operator loose_handle<U>()const
	//{ return loose_handle<U>(static_cast<U*>(obj)); }

	//! static_cast<> overload (for consts)
	operator loose_handle<const value_type>()const
	{ return loose_handle<const value_type>(static_cast<const_pointer>(obj)); }

	operator handle<value_type>()const
	{ return handle<value_type>(obj); }

	operator rhandle<value_type>()const
	{ return rhandle<value_type>(obj); }

	//! Returns pointer to the object that is being wrapped
	pointer get()const { return obj; }

	//! More explicit bool cast
	operator bool()const
		{ return obj!=0; }

	bool
	operator!()const
		{ return !obj; }

	void ref() { if(obj)obj->ref(); }

	bool unref() { if(obj && !obj->unref()){ obj=0; return false; } return true; }

	template<typename U>
	bool type_is() const
	{ return dynamic_cast<const U*>(obj); }

	template<typename U>
	U* type_pointer() const
	{ return dynamic_cast<U*>(obj); }

	template<typename U>
	bool type_equal() const
	{ return typeid(*obj) == typeid(U); }
}; // END of template class loose_handle

// cast loose_handle<> -> handle<>
template <class T> template <class U> handle<T> handle<T>::cast_static	   (const loose_handle<U>& x) { return handle<T>(static_cast	 <T*>(x.get())); }
template <class T> template <class U> handle<T> handle<T>::cast_dynamic	   (const loose_handle<U>& x) { return handle<T>(dynamic_cast	 <T*>(x.get())); }
template <class T> template <class U> handle<T> handle<T>::cast_const	   (const loose_handle<U>& x) { return handle<T>(const_cast		 <T*>(x.get())); }
template <class T> template <class U> handle<T> handle<T>::cast_reinterpret(const loose_handle<U>& x) { return handle<T>(reinterpret_cast<T*>(x.get())); }

// cast rhandle_handle<> -> handle<>
template <class T> template <class U> handle<T> handle<T>::cast_static	   (const rhandle<U>&	   x) { return handle<T>(static_cast	 <T*>(x.get())); }
template <class T> template <class U> handle<T> handle<T>::cast_dynamic	   (const rhandle<U>&	   x) { return handle<T>(dynamic_cast	 <T*>(x.get())); }
template <class T> template <class U> handle<T> handle<T>::cast_const	   (const rhandle<U>&	   x) { return handle<T>(const_cast		 <T*>(x.get())); }
template <class T> template <class U> handle<T> handle<T>::cast_reinterpret(const rhandle<U>&	   x) { return handle<T>(reinterpret_cast<T*>(x.get())); }

// cast U* -> handle<>
template <class T> template <class U> handle<T> handle<T>::cast_static	   (U*					   x) { return handle<T>(static_cast	 <T*>(x));		 }
template <class T> template <class U> handle<T> handle<T>::cast_dynamic	   (U*					   x) { return handle<T>(dynamic_cast	 <T*>(x));		 }
template <class T> template <class U> handle<T> handle<T>::cast_const	   (U*					   x) { return handle<T>(const_cast		 <T*>(x));		 }
template <class T> template <class U> handle<T> handle<T>::cast_reinterpret(U*					   x) { return handle<T>(reinterpret_cast<T*>(x));		 }

// operator== for handle<>, loose_handle<> and T*
template <class T,class U> bool operator==(const handle		 <T>& lhs,const handle		<U>& rhs) { return (lhs.get()==rhs.get()); }
template <class T,class U> bool operator==(const loose_handle<T>& lhs,const loose_handle<U>& rhs) { return (lhs.get()==rhs.get()); }
template <class T,class U> bool operator==(const handle		 <T>& lhs,const loose_handle<U>& rhs) { return (lhs.get()==rhs.get()); }
template <class T,class U> bool operator==(const loose_handle<T>& lhs,const handle		<U>& rhs) { return (lhs.get()==rhs.get()); }
template <class T>		   bool operator==(const handle<T>&		  lhs,const T*				 rhs) { return (lhs.get()==rhs);	   }
template <class T>		   bool operator==(const loose_handle<T>& lhs,const T*				 rhs) { return (lhs.get()==rhs);	   }
template <class T>		   bool operator==(const T*				  lhs,const handle<T>&		 rhs) { return (lhs		 ==rhs.get()); }
template <class T>		   bool operator==(const T*				  lhs,const loose_handle<T>& rhs) { return (lhs		 ==rhs.get()); }

// operator!= for handle<>, loose_handle<> and T*
template <class T,class U> bool operator!=(const handle		 <T>& lhs,const handle		<U>& rhs) { return (lhs.get()!=rhs.get()); }
template <class T,class U> bool operator!=(const loose_handle<T>& lhs,const loose_handle<U>& rhs) { return (lhs.get()!=rhs.get()); }
template <class T,class U> bool operator!=(const handle		 <T>& lhs,const loose_handle<U>& rhs) { return (lhs.get()!=rhs.get()); }
template <class T,class U> bool operator!=(const loose_handle<T>& lhs,const handle		<U>& rhs) { return (lhs.get()!=rhs.get()); }
template <class T>		   bool operator!=(const handle<T>&		  lhs,const T*				 rhs) { return (lhs.get()!=rhs);	   }
template <class T>		   bool operator!=(const loose_handle<T>& lhs,const T*				 rhs) { return (lhs.get()!=rhs);	   }
template <class T>		   bool operator!=(const T*				  lhs,const handle<T>&		 rhs) { return (lhs		 !=rhs.get()); }
template <class T>		   bool operator!=(const T*				  lhs,const loose_handle<T>& rhs) { return (lhs		 !=rhs.get()); }

// operator< for handle<>, loose_handle<> and T*
template <class T,class U> bool operator<(const handle<T>&		  lhs,const handle<U>&		 rhs) { return (lhs.get()<rhs.get());  }
template <class T,class U> bool operator<(const loose_handle<T>&  lhs,const loose_handle<U>& rhs) { return (lhs.get()<rhs.get());  }
template <class T,class U> bool operator<(const handle<T>&		  lhs,const loose_handle<U>& rhs) { return (lhs.get()<rhs.get());  }
template <class T,class U> bool operator<(const loose_handle<T>&  lhs,const handle<U>&		 rhs) { return (lhs.get()<rhs.get());  }
template <class T>		   bool operator<(const handle<T>&		  lhs,const T*				 rhs) { return (lhs.get()<rhs);		   }
template <class T>		   bool operator<(const loose_handle<T>&  lhs,const T*				 rhs) { return (lhs.get()<rhs);		   }
template <class T>		   bool operator<(const T*				  lhs,const handle<T>&		 rhs) { return (lhs		 <rhs.get());  }
template <class T>		   bool operator<(const T*				  lhs,const loose_handle<T>& rhs) { return (lhs		 <rhs.get());  }

};

/* === E N D =============================================================== */

#endif
