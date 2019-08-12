/* ========================================================================
** Extended Template and Library
** Abstraction for a Generic Value Type
** $Id$
**
** Copyright (c) 2002 Adrian Bentley
** Copyright (c) 2010 Nikita Kitaev
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

#ifndef __ETL__VALUE_H
#define __ETL__VALUE_H

/* === H E A D E R S ======================================================= */
#include <algorithm>
#include <typeinfo>
#include <cassert>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

/*!	\note 	This class may be specialized to support binary compatibility
			for desired objects (e.g. point3,vector3,float[3]).
			However it MUST be declared within scope that you are using the
			values....

	\warning	If you specialize this class for something that isn't binary
				compatible, then your values could easily report belonging to
				the wrong types.
*/
template < typename T >
class value_store_type
{
public:
	typedef	T						value_type;
};

namespace etl {

/*!	\class	value	_value.h	ETL/value
	\brief	Abstraction of the concept of a generic value

	Modified from ideas for the boost::any type.  Added binary compatibility
	structure
*/
class value
{
	struct contentholder
	{
		virtual ~contentholder() {}
		virtual contentholder *clone() const = 0;
		virtual const std::type_info &type() const = 0;
	};

	contentholder	*content;

public:	//constructor interface
	value()
		:content(0)
	{
	}

	value(const value &v)
		:content( v.content ? v.content->clone() : 0 )
	{
	}

	/* Copies the object passed to it
	*/
	template < typename T >
	value(const T &v)
		:content( new holder< typename value_store_type<T>::value_type >
						(reinterpret_cast<const typename value_store_type<T>::value_type &>(v)) )
	{
	}

public: //modifier interface

	value & swap(value & rhs)
	{
		std::swap(content, rhs.content);
		return *this;
	}

	template<typename ValueType>
	value & operator=(const ValueType & rhs)
	{
		value(rhs).swap(*this);
		return *this;
	}

	value & operator=(const value & rhs)
	{
		value(rhs).swap(*this);
		return *this;
	}

public: //query interface

	bool empty() const
	{
		return content == 0;
	}

	const std::type_info & type() const
	{
		return content ? content->type() : typeid(void);
	}

private: //implementation interface

	template < typename T >
	class holder : public contentholder
	{
	public: //representation
		T	obj;

	public: //constructor interface

		holder(const T &o)
			:obj(o)
		{
		}

		holder(const holder<T> &h)
			:obj(h.obj)
		{
		}

	public: //accessor interface
		virtual contentholder *clone() const
		{
			return new holder(*this);
		}

		virtual const std::type_info &type() const
		{
			return typeid(T);
		}

	public: //allocation interface
		void *operator new(size_t size)
		{
			assert(size == sizeof(holder<T>));

			//use pool allocation at some point
			return malloc(size);
		}

		void operator delete(void *p)
		{
			assert(p);
			//use pool allocation at some point
			return free(p);
		}
	};

	template < typename ValueType >
	friend ValueType *value_cast(value *v);
};

/*!	Is thrown for bad value_casts (when using a reference...)
*/
class bad_value_cast : public std::bad_cast
{
public:
	virtual const char * what() const throw()
	{
		return "etl::bad_value_cast: " "failed conversion using boost::value_cast";
	}
};

/*!	Returns a pointer to the desired value type if the value_type and the internal
	binary format agree (mediated by using the value_store_type class), otherwise
	it returns 0.

	\see value_store_type
*/
template < typename ValueType >
ValueType *value_cast(value *v)
{
	assert(v);

	return ( typeid(typename value_store_type<ValueType>::value_type) == v->type() )
			? &static_cast<value::holder<ValueType> *>(v->content)->obj
			: 0;
}

/*!	Same as above except tweaked to allow const cast (possibly for purposes involving
	type agreement... if const impacts a typeid call I do not know...)
*/
template < typename ValueType >
const ValueType * value_cast(const value *v)
{
	return value_cast<ValueType>(const_cast<value *>(v));
}

/*!	Extract a copy of the internal object and will throw a bad_value_cast exception
	if the types do not agree.

	\note	I'm not sure why boost::any didn't use a reference here... there must be a reason...

	\see bad_value_cast
*/
template < typename ValueType >
ValueType value_cast(const value &v)
{
	const ValueType * result = value_cast<ValueType>(&v);
	if(!result)
		throw bad_value_cast();
	return *result;
}

};

/* === E N D =============================================================== */

#endif
