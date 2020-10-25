/* === S Y N F I G ========================================================= */
/*!	\file guid.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "guid.h"
#include "quick_rng.h"
#include <ETL/stringf>

#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define MANUAL_GUID_CALC

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

#define GUID_RNG quick_rng
//#define GUID_RNG subtractive_rng



#ifdef MANUAL_GUID_CALC
#include <time.h>
static GUID_RNG _a, _b;
static void _set_up_rand_long_long(uint64_t &x);
static void _get_rand_long_long(uint64_t &x);
static void (*get_rand_long_long)(uint64_t&)=_set_up_rand_long_long;
static void _set_up_rand_long_long(uint64_t &x)
{
#ifdef _DEBUG
	// synfig::info("Starting up GUID system...");
#endif
	_a=GUID_RNG(time(0)+clock());
	_b=GUID_RNG(clock());
	get_rand_long_long=_get_rand_long_long;
	_get_rand_long_long(x);
}

static void _get_rand_long_long(uint64_t &x)
{
	//subtractive_rng _c(clock());
	unsigned short* data(reinterpret_cast<unsigned short *>(&x));
	data[0]=_a(65536);
	data[1]=_a(65536);
	data[2]=_a(65536);
	data[3]=_a(65536);
}

#else
// Use OS-Dependent method

#ifdef _WIN32
// Win32
static void get_rand_long_long(uint64_t &x)
{
	_GUID* guid(reinterpret_cast<_GUID*>(&x));
	CoCreateGuid(guid);
}

#else
// Unix
static int rand_fd;
static void _set_up_rand_long_long(uint64_t &x);
static void _get_rand_long_long(uint64_t &x);
static void (*get_rand_long_long)(uint64_t&)=_set_up_rand_long_long;
static void _set_up_rand_long_long(uint64_t &x)
{
#ifdef _DEBUG
	// synfig::info("Starting up GUID system...");
#endif
	rand_fd=open("/dev/urandom",O_RDONLY);
	get_rand_long_long=_get_rand_long_long;
	_get_rand_long_long(x);
}

static void _get_rand_long_long(uint64_t &x){	read(rand_fd,&x,sizeof(x));}

#endif
#endif



void
synfig::GUID::make_unique()
{
	get_rand_long_long(data.u_64.a);
	get_rand_long_long(data.u_64.b);
}

synfig::GUID
synfig::GUID::hasher(const String& str)
{
	/* http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2003/n1456.html says:
	 *
	 *   "Some earlier hash table implementations gave char* special
	 *    treatment: it specialized the default hash function to look
	 *    at character array being pointed to, rather than the pointer
	 *    itself. This proposal removes that special treatment."
	 *
	 * Unfortunately, the older implementation doesn't seem to want to
	 * accept Strings, so we're left with this conditional compilation.
	 */
	std::hash<String> string_hash_;
	const unsigned int seed(string_hash_(str));

	GUID_RNG random(seed);
	GUID ret(0);
	ret.data.u_32.a=random(~(unsigned int)0);
	ret.data.u_32.b=random(~(unsigned int)0);
	ret.data.u_32.c=random(~(unsigned int)0);
	ret.data.u_32.d=random(~(unsigned int)0);
	return ret;
}

synfig::GUID
synfig::GUID::hasher(int i)
{
	GUID ret(0);
	GUID_RNG random(i);
	ret.data.u_32.a=random(~(unsigned int)0);
	ret.data.u_32.b=random(~(unsigned int)0);
	ret.data.u_32.c=random(~(unsigned int)0);
	ret.data.u_32.d=random(~(unsigned int)0);
	return ret;
}

synfig::GUID
synfig::GUID::hasher(const GUID &x)
{
	GUID ret(0);
	ret.data.u_32.a=GUID_RNG(x.data.u_32.d)(~(unsigned int)0);
	ret.data.u_32.b=GUID_RNG(x.data.u_32.a)(~(unsigned int)0);
	ret.data.u_32.c=GUID_RNG(x.data.u_32.b)(~(unsigned int)0);
	ret.data.u_32.d=GUID_RNG(x.data.u_32.c)(~(unsigned int)0);
	return ret;
}

String
synfig::GUID::get_string()const
{
	return strprintf("%08X%08X%08X%08X",data.u_32.a,data.u_32.b,data.u_32.c,data.u_32.d);
}

synfig::GUID::GUID(int i)
{
	assert(!i);
	memset(&data, 0, sizeof (data));
}

synfig::GUID::GUID(const String &str)
{
	strscanf(str,"%08X%08X%08X%08X",&data.u_32.a,&data.u_32.b,&data.u_32.c,&data.u_32.d);
}
