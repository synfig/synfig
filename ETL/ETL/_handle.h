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
#include <memory>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

#define ETL_SELF_DELETING_SHARED_OBJECT

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

// Forward Declarations
template <class T> class rhandle;


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
/*!	\class	rshared_object _handle.h	ETL/handle
**	\brief	Replaceable Shared Object Base Class
**	\see rhandle
**	\writeme
*/
class rshared_object
{
private:
	mutable int rrefcount;

public:
	void *front_;
	void *back_;

protected:
	rshared_object():rrefcount(0),front_(nullptr),back_(nullptr) { }
	rshared_object(const rshared_object &other): rrefcount(0),front_(nullptr),back_(nullptr) { }
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
    bool runique() const
    { return front_ == back_; }
}; // END of class rshared_object

// ========================================================================
/*!	\class	rhandle _handle.h	ETL/handle
**	\brief	Replaceable Object Handle
**	\see rshared_object, handle, loose_handle
**	\writeme
*/
template <class T>
class rhandle
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

public:
    std::shared_ptr<value_type> obj;
private:
	rhandle<value_type> *prev_;
	rhandle<value_type> *next_;

	void add_to_rlist()
	{
		assert(obj);
		obj->rref();

		// If this is the first reversible handle
		if(!obj->front_)
		{
			obj->front_=obj->back_=this;
			prev_=next_= nullptr;
			return;
		}

		prev_=reinterpret_cast<rhandle<value_type>*>(obj->back_);
		next_= nullptr;
		prev_->next_=this;
		obj->back_=this;
	}

	void del_from_rlist()
	{
		assert(obj);
		obj->runref();

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
	rhandle() : obj(nullptr){}

	//! Constructor that constructs from a pointer to new object
	rhandle(pointer x):obj(x)
	{
		if(obj)add_to_rlist();
	}

	rhandle(const std::shared_ptr<value_type> &x):obj(x)
	{
		if(obj)add_to_rlist();
	}

	//! Default copy constructor
	rhandle(const rhandle<value_type> &x):obj(x.obj)
	{
		if(obj)add_to_rlist();
	}

	//! Handle is released on deletion
	~rhandle() { detach(); }

	//! Assignment operator
	rhandle<value_type> &
	operator=(const rhandle<value_type> &x)
	{
		if(obj==x.obj)
			return *this;

		detach(); // tests are needed

		obj=x.obj;
		if(obj)add_to_rlist();
		return *this;
	}

    rhandle<value_type> &
    operator=(const std::shared_ptr<value_type> &x)
    {
        if(x==obj)
            return *this;

        detach();

        obj=x;
        if(obj)add_to_rlist();
        return *this;
    }

    //! Returns pointer to the object that is being wrapped
    pointer get() const { return obj.get(); }

    operator rhandle<const value_type>()const
    { return rhandle<const value_type>(static_cast<const_pointer>(obj)); }

    operator std::shared_ptr<value_type>() const { return obj; }

    template <class U> rhandle<T> cast_static	   (const rhandle<U>& x);
    template <class U> rhandle<T> cast_dynamic	   (const rhandle<U>& x);
    template <class U> rhandle<T> cast_const	   (const rhandle<U>& x);
    template <class U> rhandle<T> cast_reinterpret(const rhandle<U>& x);

    template <class U> rhandle<T> cast_static	   (const std::shared_ptr<U>& x);
    template <class U> rhandle<T> cast_dynamic	   (const std::shared_ptr<U>& x);
    template <class U> rhandle<T> cast_const	   (const std::shared_ptr<U>& x);
    template <class U> rhandle<T> cast_reinterpret(const std::shared_ptr<U>& x);

	//! Handle release procedure
	void
	detach()
	{
		if(obj)del_from_rlist();
		obj.reset();
	}

	void reset() { detach(); }

	//! Creates a new instance of a T object and puts it in the handle.
	/*! Uses the default constructor */
	void spawn() { obj = std::make_shared<value_type>(new T); }

	//! Returns number of reversible instances
	count_type rcount()const
	{ return obj?obj->rcount():0; }

    count_type use_count()const
    { return obj.use_count(); }

    bool unique()const
    { return obj.unique(); }

    operator bool()const
    { return obj!= nullptr; }

    template<class U> bool operator==(const rhandle<U> &rhs ) { return obj == rhs.obj; }

    template<class U> bool operator!=(const rhandle<U> &rhs ) { return obj != rhs.obj; }

    template<class U> bool operator<(const rhandle<U> &rhs ) { return obj < rhs.obj; }

    reference operator*()const { return *obj.get(); }

    pointer operator->()const { return obj.get(); }

	//! Returns true if there is only one instance of the object
	bool runique()const { return obj->runique(); }

	//! \writeme
	int replace(const std::shared_ptr<value_type> &x)
	{
		assert(obj);
		assert(x!=obj);

		if(x==obj)
			return 0;

		rhandle<value_type> *iter;
		rhandle<value_type> *next;

		iter=reinterpret_cast<rhandle<value_type>*>(obj->front_);

		assert(iter);

		next=iter->next_;

		int i=0;
		#ifndef NDEBUG
        std::shared_ptr<value_type> obj_=obj;
		#endif

		for(;iter;iter=next,next=iter?iter->next_:nullptr,i++)
		{
			assert(iter->get()==obj_.get());
			(*iter)=x;
		}

        assert(obj==x);

		return i;
	}

	//! Swaps the values of two handles without reference counts
	/*!	\warning not yet implemented. \writeme */
    std::shared_ptr<value_type> &
    swap(std::shared_ptr<value_type> &x){
        obj.swap(x);
    }
}; // END of template class rhandle

template <class T, class U> bool operator==(const std::shared_ptr<T>& lhs, const etl::rhandle<U>& rhs){ return (lhs.get()==rhs.get()); }
template <class T, class U> bool operator==(const rhandle<T>& lhs, const std::shared_ptr<U>& rhs)     { return (lhs.get()==rhs.get()); }
template <class T>          bool operator==(const std::shared_ptr<T>& lhs, const T* rhs)              { return (lhs.get()==rhs); }
template <class T>          bool operator==(const rhandle<T>& lhs, const T* rhs)                      { return (lhs.get()==rhs); }
template <class T>          bool operator==(const T* lhs, const std::shared_ptr<T>& rhs)              { return (lhs==rhs.get()); }
template <class T>          bool operator==(const T* lhs, const rhandle<T>& rhs)                      { return (lhs==rhs.get()); }

template <class T, class U> bool operator!=(const std::shared_ptr<T>& lhs, const etl::rhandle<U>& rhs) { return (lhs.get()!=rhs.get()); }
template <class T, class U> bool operator!=(const rhandle<T>& lhs, const std::shared_ptr<U>& rhs)      { return (lhs.get()!=rhs.get()); }
template <class T>          bool operator!=(const std::shared_ptr<T>& lhs, const T* rhs)               { return (lhs.get()!=rhs); }
template <class T>          bool operator!=(const rhandle<T>& lhs, const T* rhs)                       { return (lhs.get()!=rhs); }
template <class T>          bool operator!=(const T* lhs, const std::shared_ptr<T>& rhs)               { return (lhs!=rhs.get()); }
template <class T>          bool operator!=(const T* lhs, const rhandle<T>& rhs)                       { return (lhs!=rhs.get()); }
};

/* === E N D =============================================================== */

#endif
