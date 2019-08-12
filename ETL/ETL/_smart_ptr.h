/* ========================================================================
** Extended Template and Library
** Template Smart Pointer Implementation
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This package is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** === N O T E S ===========================================================
**
** This is an internal header file, included by other ETL headers.
** You should not attempt to use it directly.
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__SMART_PTR_H
#define __ETL__SMART_PTR_H

/* === H E A D E R S ======================================================= */

#include <cassert>
#include "_ref_count.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

template <class T>
struct generic_deleter
{
	void operator()(T* x)const { delete x; }
};

template <class T>
struct array_deleter
{
	void operator()(T* x)const { delete [] x; }
};

// ========================================================================
/*!	\class	smart_ptr	_smart_ptr.h	ETL/smart_ptr
**	\brief	Object Smart Pointer
**	\see loose_smart_ptr
**	\writeme
*/
template <class T, class D=generic_deleter<T> >
class smart_ptr
{
public:

	typedef T value_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef int count_type;
	typedef int size_type;
	typedef D destructor_type;

#ifdef DOXYGEN_SHOULD_SKIP_THIS		// #ifdef is not a typo
private:
#endif
	value_type *obj;		//!< \internal Pointer to object
	reference_counter refcount;

public:
	// Private constructor for convenience
	smart_ptr(value_type* obj,reference_counter refcount):obj(obj),refcount(refcount) {  }

	//! Default constructor - empty smart_ptr
	smart_ptr():obj(0),refcount(false) {}

	//! Constructor that constructs from a pointer to new object
	/*! A new smart_ptr is created with a pointer
		to a newly allocated object. We need
		to be explicit with this so we don't
		accidentally have two smart_ptrs for one
		object -- that would be bad.	*/
	explicit smart_ptr(value_type* x):obj(x),refcount(x?true:false) {  }

	//! Template copy constructor
	/*! This template constructor allows us to cast
		smart_ptrs much like we would pointers. */
#ifdef _WIN32
	template <class U>
	smart_ptr(const smart_ptr<U> &x):obj((pointer)&*x.obj),refcount(x.refcount())
		{ }
#endif

	//! Default copy constructor
	/*! The template above is not good enough
		for all compilers. We need to explicitly
		define the copy constructor for this
		class to work on those compilers. */
	smart_ptr(const smart_ptr<value_type> &x):obj(x.obj),refcount(x.refcount) {  }

	explicit smart_ptr(const value_type &x):obj(new value_type(x)) { }

	//! smart_ptr is released on deletion
	~smart_ptr() { if(refcount.unique()) destructor_type()(obj); }

	//! Template Assignment operator
	template <class U> const smart_ptr<value_type> &
	operator=(const smart_ptr<U> &x)
	{
		if(x.get()==obj)
			return *this;

		reset();

		if(x.obj)
		{
			obj=(pointer)x.get();
			refcount=x.refcount;
		}

		return *this;
	}

	//! Assignment operator
	const smart_ptr<value_type> &
	operator=(const smart_ptr<value_type> &x)
	{
		if(x.get()==obj)
			return *this;

		reset();

		if(x.obj)
		{

			obj=(pointer)x.get();
			refcount=x.refcount;
		}

		return *this;
	}

	//! smart_ptr reset procedure
	void
	reset()
	{
		if(obj)
		{
			if(refcount.unique()) destructor_type()(obj);
			refcount.detach();
			obj=0;
		}
	}

	void spawn() { operator=(smart_ptr(new T)); }

	//! Returns number of instances
	const count_type& count()const { return refcount; }

	//! Returns true if there is only one instance of the object
	bool unique()const { return refcount.unique(); }

	//! Returns a constant handle to our object
	smart_ptr<const value_type> constant() { return *this; }

	reference operator*()const { assert(obj); return *obj; }

	pointer	operator->()const { assert(obj); return obj; }


	operator smart_ptr<const value_type>()const
	{ return smart_ptr<const value_type>(static_cast<const_pointer>(obj)); }

	//! static_cast<> wrapper
	template <class U> static
	smart_ptr<T> cast_static(const smart_ptr<U> &x)
	{ if(!x)return NULL; return smart_ptr<T>(static_cast<T*>(x.get()),x.refcount); }

	//! dynamic_cast<> wrapper
	template <class U> static
	smart_ptr<T> cast_dynamic(const smart_ptr<U> &x)
	{ if(!x)return 0; return smart_ptr<T>(dynamic_cast<T*>(x.get()),x.refcount); }

	//! const_cast<> wrapper
	template <class U> static
	smart_ptr<T> cast_const(const smart_ptr<U> &x)
	{ if(!x)return 0; return smart_ptr<T>(const_cast<T*>(x.get()),x.refcount); }

	pointer get()const { return obj; }

	//! More explicit bool cast
	operator bool()const { return obj!=0; }

	bool operator!()const { return !obj; }

	//! Overloaded cast operator -- useful for implicit casts
	template <class U>
	operator smart_ptr<U>()
	{
		// This next line should provide a syntax check
		// to make sure that this cast makes sense.
		// If it doesn't, this should have a compiler error.
		// Otherwise, it should get optimized right out
		// of the code.
		//(U*)obj;

		return *reinterpret_cast<smart_ptr<U>*>(this);
	}

}; // END of template class smart_ptr

// ========================================================================
/*!	\class	loose_smart_ptr	_smart_ptr.h	ETL/smart_ptr
**	\brief	Loose Object Smart Pointer
**	\see smart_ptr
**	\writeme
*/
template <class T>
class loose_smart_ptr
{
public:

	typedef T value_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef int count_type;
	typedef int size_type;

private:
	value_type *obj;		//!< \internal Pointer to object
	weak_reference_counter refcount;	//!< \internal Pointer to object's reference counter

public:

	//! Default constructor - empty smart_ptr
	loose_smart_ptr():obj(0),refcount(0) {}

	//! Default copy constructor
	loose_smart_ptr(const loose_smart_ptr<value_type> &x):obj(x.get()),refcount(x.refcount) { }

	loose_smart_ptr(const smart_ptr<value_type> &x):obj(x.get()),refcount(x.refcount) { }

	void reset() { obj=0,refcount=0; }

	operator smart_ptr<value_type>()
	{
		return smart_ptr<value_type>(static_cast<pointer>(obj),refcount);
	}

	operator smart_ptr<const value_type>()
	{
		return smart_ptr<const value_type>(static_cast<const_pointer>(obj),refcount);
	}

	//! Returns number of instances
	const count_type& count()const { return refcount; }

	bool unique()const { return refcount.unique(); }

	reference operator*()const { assert(obj); return *obj; }

	pointer	operator->()const { assert(obj); return obj; }

	pointer get()const { return obj; }

	bool operator!()const { return !obj; }
};

template <class T,class U> bool
operator==(const smart_ptr<T> &lhs,const smart_ptr<U> &rhs)
	{ return (lhs.get()==rhs.get()); }
template <class T,class U> bool
operator==(const loose_smart_ptr<T> &lhs,const loose_smart_ptr<U> &rhs)
	{ return (lhs.get()==rhs.get()); }
template <class T,class U> bool
operator==(const smart_ptr<T> &lhs,const loose_smart_ptr<U> &rhs)
	{ return (lhs.get()==rhs.get()); }
template <class T,class U> bool
operator==(const loose_smart_ptr<T> &lhs,const smart_ptr<U> &rhs)
	{ return (lhs.get()==rhs.get()); }
template <class T> bool
operator==(const smart_ptr<T> &lhs,const T *rhs)
	{ return (lhs.get()==rhs); }
template <class T> bool
operator==(const loose_smart_ptr<T> &lhs,const T *rhs)
	{ return (lhs.get()==rhs); }
template <class T> bool
operator==(const T *lhs,const smart_ptr<T> &rhs)
	{ return (lhs==rhs.get()); }
template <class T> bool
operator==(const T *lhs,const loose_smart_ptr<T> &rhs)
	{ return (lhs==rhs.get()); }


template <class T,class U> bool
operator!=(const smart_ptr<T> &lhs,const smart_ptr<U> &rhs)
	{ return (lhs.get()!=rhs.get()); }
template <class T,class U> bool
operator!=(const loose_smart_ptr<T> &lhs,const loose_smart_ptr<U> &rhs)
	{ return (lhs.get()!=rhs.get()); }
template <class T,class U> bool
operator!=(const smart_ptr<T> &lhs,const loose_smart_ptr<U> &rhs)
	{ return (lhs.get()!=rhs.get()); }
template <class T,class U> bool
operator!=(const loose_smart_ptr<T> &lhs,const smart_ptr<U> &rhs)
	{ return (lhs.get()!=rhs.get()); }
template <class T> bool
operator!=(const smart_ptr<T> &lhs,const T *rhs)
	{ return (lhs.get()!=rhs); }
template <class T> bool
operator!=(const loose_smart_ptr<T> &lhs,const T *rhs)
	{ return (lhs.get()!=rhs); }
template <class T> bool
operator!=(const T *lhs,const smart_ptr<T> &rhs)
	{ return (lhs!=rhs.get()); }
template <class T> bool
operator!=(const T *lhs,const loose_smart_ptr<T> &rhs)
	{ return (lhs!=rhs.get()); }


template <class T,class U> bool
operator<(const smart_ptr<T> &lhs,const smart_ptr<U> &rhs)
	{ return (lhs.get()<rhs.get()); }
template <class T,class U> bool
operator<(const loose_smart_ptr<T> &lhs,const loose_smart_ptr<U> &rhs)
	{ return (lhs.get()<rhs.get()); }
template <class T,class U> bool
operator<(const smart_ptr<T> &lhs,const loose_smart_ptr<U> &rhs)
	{ return (lhs.get()<rhs.get()); }
template <class T,class U> bool
operator<(const loose_smart_ptr<T> &lhs,const smart_ptr<U> &rhs)
	{ return (lhs.get()<rhs.get()); }
template <class T> bool
operator<(const smart_ptr<T> &lhs,const T *rhs)
	{ return (lhs.get()<rhs); }
template <class T> bool
operator<(const loose_smart_ptr<T> &lhs,const T *rhs)
	{ return (lhs.get()<rhs); }
template <class T> bool
operator<(const T *lhs,const smart_ptr<T> &rhs)
	{ return (lhs<rhs.get()); }
template <class T> bool
operator<(const T *lhs,const loose_smart_ptr<T> &rhs)
	{ return (lhs<rhs.get()); }

};

/* === E N D =============================================================== */

#endif
