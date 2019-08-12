/*! ========================================================================
** Extended Template Library
** Trivializing Template Class Implementation
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

#ifndef __ETL__TRIVIAL_H
#define __ETL__TRIVIAL_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

/*! ========================================================================
** \class	Trivial
** \brief	Trivializes the constructor of a given class
**
** This class makes the given type 'trivial',
** effectively disabling the constructor and
** destructor. (This is useful for unions)
** Some extra casting may be necessary to get
** it to work properly.
*/
template <class T>
class trivial
{
	typedef T value_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef const T* const_pointer;

	char data[sizeof(T)];
public:
	operator reference()
	{ return *reinterpret_cast<pointer>(data); }

	// HACK - Rather dangerous
	//operator reference()const
	//{ return *reinterpret_cast<pointer>(const_cast<char *>(data)); }

	operator const_reference()const
	{ return *reinterpret_cast<const_pointer>(data); }

	reference get()
	{ return *reinterpret_cast<pointer>(data); }

	const_reference get()const
	{ return *reinterpret_cast<const_pointer>(data); }

	void construct()
	{ new(&get()) value_type(); }

	void destruct()
	{ get().~value_type(); }

	void destroy() { destruct(); }

	template<class U> reference
	operator=(const U &rhs)
	{ return get()=rhs; }

	template<class U>reference
	operator=(const trivial<U> &rhs)
	{ return get()=rhs.get(); }

	template<class U> reference
	operator+=(const U &rhs)
	{ return get()+=rhs; }

	template<class U> reference
	operator-=(const U &rhs)
	{ return get()-=rhs; }

	template<class U> reference
	operator*=(const U &rhs)
	{ return get()*=rhs; }

	template<class U> reference
	operator/=(const U &rhs)
	{ return get()/=rhs; }

	template<class U> reference
	operator%=(const U &rhs)
	{ return get()%=rhs; }

	template<class U> reference
	operator^=(const U &rhs)
	{ return get()^=rhs; }

	template<class U> reference
	operator&=(const U &rhs)
	{ return get()&=rhs; }

	template<class U> reference
	operator>>=(const U &rhs)
	{ return get()>>=rhs; }

	template<class U> reference
	operator<<=(const U &rhs)
	{ return get()<<=rhs; }

	operator bool()const
	{ return get(); }

	bool operator!()const
	{ return !get(); }
}; // END of template class trivial

};

//#include <iostream>

/*
template<typename T, typename _CharT, class _Traits> std::basic_istream<_CharT, _Traits>&
operator>>(std::basic_istream<_CharT, _Traits>& s, etl::trivial<T>& rhs)
{ return s>>(T)(rhs); }

template<typename T,typename _CharT, class _Traits> std::basic_ostream<_CharT, _Traits>&
operator<<(std::basic_ostream<_CharT, _Traits>& s, const etl::trivial<T>& rhs)
{ return s<<(T)(rhs); }
*/

/*
template<typename T> std::istream&
operator>>(std::istream& s, etl::trivial<T>& rhs)
{ return s>>(T)(rhs); }

template<typename T> std::ostream&
operator<<(std::ostream& s, const etl::trivial<T>& rhs)
{ return s<<(T)(rhs); }
*/

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
