/*! ========================================================================
** Extended Template and Library
** Random Number Generator Class Implementation
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

#ifndef __ETL__RANDOM_H
#define __ETL__RANDOM_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

_ETL_BEGIN_NAMESPACE

/*
class rand_source_xor
{
public:
	typedef int seed_type;
	typedef short value_type;

private:
	short entropy_pool[256];
	int pool_index;

public:
	random()
	{
		seed(0);
		mod=offset=0;
	}

	void seed(const seed_type &x)
	{ pool_index=0; }

	void add_entropy(value_type entropy)
	{
		int i;
		for(i=0;i<POOL_SIZE;i++)
			entropy^=(entropy_pool[i]^=entropy*i);
	}

	void add_entropy(const value_type *entropy, int size)
	{
	}

	short get_short()
	{
		if(pool_index>POOL_SIZE)
			pool_index=0;
		if(mod)
			return entropy_pool[pool_index++]%mod+offset;
		return entropy_pool[pool_index++];
	}
};
*/

template <class T,int POOL_SIZE=256>
class random
{
public:
	typedef T value_type;
	typedef int seed_type;

private:
	value_type entropy_pool[POOL_SIZE];
	int pool_index;

	value_type mod,offset;

public:
	random()
	{
		seed(0);
		mod=offset=0;
	}

	void seed(const seed_type &x __attribute__ ((unused)))
	{ pool_index=0; }

	void set_range(const value_type &floor,const value_type &ceil)
	{ mod=ceil-floor; offset=floor; }

	void set_range(const value_type &ceil)
	{ mod=ceil; }

	void add_entropy(value_type entropy)
	{
		int i;
		for(i=0;i<POOL_SIZE;i++)
			entropy^=(entropy_pool[i]^=entropy*i);
	}

	void add_entropy(const char *entropy)
	{
	}

	value_type operator()(void)
	{
		if(pool_index>POOL_SIZE)
			pool_index=0;
		if(mod)
			return entropy_pool[pool_index++]%mod+offset;
		return entropy_pool[pool_index++];
	}
};

/* === T Y P E D E F S ===================================================== */

_ETL_END_NAMESPACE

/* === E N D =============================================================== */

#endif

