/*! ========================================================================
** Extended Template and Library Test Suite
** Generic Value Test
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
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <ETL/value>
#include <stdio.h>

/* === M A C R O S ========================================================= */

using namespace etl;

/* === C L A S S E S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === E N T R Y P O I N T ================================================= */

struct stupidv
{
	float x,y;

	stupidv(float xin=0, float yin=0) :x(xin),y(yin) {}
	void print() const
	{
		printf("(x=%f,y=%f)\n",x,y);
	}
};

struct stupidp
{
	float z,w;

	stupidp(float zin=0, float win=0) :z(zin),w(win) {}

	void print() const
	{
		printf("(z=%f,w=%f)\n",z,w);
	}
};

template <>
class value_store_type<stupidp>
{
	public:
	typedef stupidv	value_type;
};

int main()
{
	try
	{
	value	v(10.0); //construction
	value	v2;		 //default construct...

	//get type...
	printf("type of 10.0: %s\n", v.type().name());

	v2 = 1; //assignment
	printf("type of 1: %s\n", v2.type().name());

	//extract int test

	int *pi = value_cast<int>(&v2);
	printf("v2 is an int(%p)\n", pi);
	printf("	%d\n", value_cast<int>(v2));

	printf("	const version: %d\n", value_cast<int>(value(5)));

	v = 'c'; //assignment again...
	printf("type of c: %s\n", v.type().name());

	v2 = v; //value assignment
	printf("type of v2 , v: %s , %s\n", v2.type().name(), v.type().name());

	//random type test
	v = stupidv(0,1);
	printf("type of vec: %s\n", v.type().name());

	//type cast with binary change test
	value_cast<stupidp>(&v)->print();
	value_cast<stupidv>(stupidp(5,10)).print(); //copy test

	printf("type of v: %s\n", v.type().name());
	printf("type of v2: %s\n", v2.type().name());
	v.swap(v2);
	printf("type of v: %s\n", v.type().name());
	printf("type of v2: %s\n", v2.type().name());

	// test the exception throwing...
	value_cast<int>(stupidp(6,66));

	}catch(const etl::bad_value_cast &e)
	{
		printf("	Exploded: %s\n",e.what());
	}catch(...)
	{
		printf("	Exploded\n");
	}

	return 0;
}
