/* === S Y N F I G ========================================================= */
/*!	\file guid.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GUID_H
#define __SYNFIG_GUID_H

/* === H E A D E R S ======================================================= */

#include "string.h"
#include <stdint.h>
#include <cassert>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class GUID
{
	union {
		struct {
			unsigned int a;
			unsigned int b;
			unsigned int c;
			unsigned int d;
		} u_32;
		struct {
			uint64_t a;
			uint64_t b;
		} u_64;

	} data;

	GUID(int);

public:
	GUID()
		{ make_unique(); }

	GUID(const String& str);

	static GUID zero() { return GUID(0); }
	static GUID hasher(const String& str);
	static GUID hasher(int i);
	static GUID hasher(const GUID &x);

	operator bool()const { return data.u_32.a||data.u_32.b||data.u_32.c||data.u_32.d; }

	uint64_t get_hi()const { return data.u_64.a; }
	uint64_t get_lo()const { return data.u_64.b; }

	uint64_t get_hi_hi()const { return data.u_32.a; }
	uint64_t get_hi_lo()const { return data.u_32.b; }
	uint64_t get_lo_hi()const { return data.u_32.c; }
	uint64_t get_lo_lo()const { return data.u_32.d; }

	void make_unique();
	String get_string()const;

	bool operator==(const GUID& rhs)const
		{ return data.u_64.a==rhs.data.u_64.a && data.u_64.b==rhs.data.u_64.b; }
	bool operator!=(const GUID& rhs)const
		{ return data.u_64.a!=rhs.data.u_64.a || data.u_64.b!=rhs.data.u_64.b; }
	bool operator<(const GUID& rhs)const
		{ return (data.u_64.a==rhs.data.u_64.a)?(data.u_64.b<rhs.data.u_64.b):(data.u_64.a<rhs.data.u_64.a); }
	bool operator>(const GUID& rhs)const
		{ return (data.u_64.a==rhs.data.u_64.a)?(data.u_64.b>rhs.data.u_64.b):(data.u_64.a>rhs.data.u_64.a); }
	bool operator<=(const GUID& rhs)const
		{ return operator<(rhs) || operator==(rhs); }
	bool operator>=(const GUID& rhs)const
		{ return operator>(rhs) || operator==(rhs); }

	//! Operator '^' (xor)
	/*! If A ^ B == C, then C ^ B == A and B ^ A == C.
	**	Also keep in mind that A ^ A == 0 and A ^ B ^ B = A. */
	GUID& operator^=(const GUID& rhs)
	{
		data.u_32.a^=rhs.data.u_32.a;
		data.u_32.b^=rhs.data.u_32.b;
		data.u_32.c^=rhs.data.u_32.c;
		data.u_32.d^=rhs.data.u_32.d;
		return *this;
	}
	GUID operator^(const GUID& rhs)const { return GUID(*this)^=rhs; }

	//! Operator '%' (alt-xor)
	/*! A % B != B % A. */
	GUID& operator%=(const GUID& rhs)
	{
		data.u_32.a^=rhs.data.u_32.b;
		data.u_32.b^=rhs.data.u_32.c;
		data.u_32.c^=rhs.data.u_32.d;
		data.u_32.d^=rhs.data.u_32.a;
		return *this;
	}
	GUID operator%(const GUID& rhs)const { return GUID(*this)%=rhs; }

};

class GUIDHash
{
public:
	size_t operator()(const GUID& guid)const
	{
		return
			guid.get_hi_hi()+
			guid.get_hi_lo()+
			guid.get_lo_hi()+
			guid.get_lo_lo();
	}
};

};

/* === E N D =============================================================== */

#endif
