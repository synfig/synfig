/* === S Y N F I G ========================================================= */
/*! \file handle.cpp
**  \brief Handle Smart Pointer Test
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This file is part of Synfig.
**
** Synfig is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** Synfig is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <ETL/handle>

#include <list>
#include <map>
#include <vector>

#include "test_base.h"

/* === M A C R O S ========================================================= */

#define NUMBER_OF_OBJECTS	40000

/* === C L A S S E S ======================================================= */

struct my_test_obj : public etl::rshared_object
{
	static int instance_count;
	int my_id;
	my_test_obj(int my_id=0):my_id(my_id)
	{
		instance_count++;
	}

	virtual ~my_test_obj()
	{
		if(instance_count==0)
			printf("Error, instance count is going past zero!\n");
		instance_count--;
	}

	bool operator<(const my_test_obj &rhs)const
	{
		return my_id<rhs.my_id;
	}
};

struct my_other_test_obj : public my_test_obj
{
	static int instance_count;
	my_other_test_obj(int my_id=0):my_test_obj(my_id)
	{
		instance_count++;
	}
	virtual ~my_other_test_obj()
	{
		if(instance_count==0)
			printf("Error, instance count is going past zero!\n");
		instance_count--;
	}
};

int my_test_obj::instance_count=0;
int my_other_test_obj::instance_count=0;

typedef etl::handle<my_test_obj> obj_handle;
typedef etl::rhandle<my_test_obj> robj_handle;
typedef etl::handle<my_other_test_obj> other_obj_handle;
typedef std::list< obj_handle > obj_list;
typedef std::list< other_obj_handle > other_obj_list;
typedef std::list< robj_handle > robj_list;

/* === P R O C E D U R E S ================================================= */

void
handle_basic_test()
{
	my_test_obj::instance_count=0;

	{
		etl::handle<my_test_obj> obj_handle(new my_test_obj(rand()));
	}

	ASSERT_EQUAL(0, my_test_obj::instance_count);

	{
		std::map<std::string, etl::handle<my_test_obj> > my_map;
		etl::handle<my_test_obj> obj_handle(new my_test_obj(rand()));
		my_map["bleh"]=obj_handle;
	}

	ASSERT_EQUAL(0, my_test_obj::instance_count);

	etl::handle<my_test_obj> obj_handle(new my_test_obj(rand()));

	ASSERT(obj_handle == obj_handle.constant());
}

void
handle_general_use_test()
{
	my_test_obj::instance_count=0;

	obj_list my_list, my_other_list;
	int i;

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back( obj_handle(new my_test_obj(rand())) );

	my_other_list=my_list;
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, my_test_obj::instance_count);

	my_list.sort();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, my_test_obj::instance_count);

	my_list.clear();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, my_test_obj::instance_count);

	{
		obj_handle a(new my_test_obj(27)), b(new my_test_obj(42));
		a.swap(b);
		ASSERT_EQUAL(42, a->my_id);
		ASSERT_EQUAL(27, b->my_id);
	}

	my_other_list.clear();
	ASSERT_EQUAL(0, my_test_obj::instance_count);
}

struct ListItem
{
	robj_handle obj;
	int bleh;
	int blah;
	ListItem(robj_handle obj,int bleh=1, int blah=2):
		obj(obj),bleh(bleh),blah(blah) { }
};

void
rhandle_general_use_test()
{
	my_test_obj::instance_count = 0;

	robj_list my_list;
	int i;

	robj_handle obj = new my_test_obj(rand());
	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back(obj);

	obj_list my_other_list(my_list.begin(),my_list.end());



	ASSERT_EQUAL(NUMBER_OF_OBJECTS*2+1, obj.count());

	ASSERT_EQUAL(NUMBER_OF_OBJECTS+1, obj.rcount());

	my_list.sort();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS+1, obj.rcount());

	{robj_handle bleh(obj);}

	ASSERT_EQUAL(NUMBER_OF_OBJECTS+1, obj.rcount());

	my_other_list.clear();

	ASSERT(obj.rcount() == obj.count());

	ASSERT_EQUAL(NUMBER_OF_OBJECTS + 1, obj.rcount());

	robj_handle new_obj = new my_test_obj(rand());

	int replacements = obj.replace(new_obj);

	ASSERT_EQUAL(NUMBER_OF_OBJECTS + 1, replacements);

	ASSERT(obj == new_obj);

	{
		robj_handle bleh(obj);
		robj_handle blah(obj.get());
	}


	my_list.clear();
	obj.detach();
	new_obj.detach();

	ASSERT_EQUAL(0, my_test_obj::instance_count);

	std::vector<ListItem> my_item_list;
	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_item_list.push_back(ListItem(new my_test_obj(rand()),3,4));


	for(i=0;i<100;i++)
	{
		int src,dest;
		src=rand()%NUMBER_OF_OBJECTS;
		dest=rand()%NUMBER_OF_OBJECTS;
		ListItem tmp(my_item_list[src]);
		assert(tmp.obj.rcount()>=2);
		my_item_list.erase(my_item_list.begin()+src);
		assert(tmp.obj.rcount()>=1);
		my_item_list.insert(my_item_list.begin()+dest,tmp);
		assert(tmp.obj.rcount()>=2);
	}

	my_item_list.clear();

	ASSERT_EQUAL(0, my_test_obj::instance_count);
}

void
handle_inheritance_test()
{
	my_test_obj::instance_count = 0;
	my_other_test_obj::instance_count = 0;

	other_obj_list my_other_list;
	int i;

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_other_list.push_back( other_obj_handle(new my_other_test_obj(rand())) );

	obj_list my_list(my_other_list.begin(),my_other_list.end());
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, my_test_obj::instance_count);

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back( other_obj_handle(new my_other_test_obj(rand())) );
	ASSERT_EQUAL(NUMBER_OF_OBJECTS * 2, my_other_test_obj::instance_count);
	ASSERT(my_test_obj::instance_count == my_other_test_obj::instance_count);

	my_list.sort();
	my_other_list.sort();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS * 2, my_test_obj::instance_count);

	my_list.clear();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, my_test_obj::instance_count);

	my_other_list.clear();
	ASSERT_EQUAL(0, my_test_obj::instance_count);
}

void test_func(etl::handle<my_test_obj> handle)
{
	if(handle) { int i=handle.count(); i++; }
}

void
loose_handle_test()
{
	my_test_obj::instance_count=0;

	etl::loose_handle<my_test_obj> obj_handle_loose;
	etl::handle<my_test_obj> obj_handle2;

	{
		etl::handle<my_test_obj> obj_handle(new my_test_obj(rand()));
		ASSERT_EQUAL(1, my_test_obj::instance_count);

		obj_handle_loose=obj_handle;
		ASSERT(obj_handle == obj_handle_loose);

		obj_handle2=obj_handle_loose;
		ASSERT_EQUAL(1, my_test_obj::instance_count);

		test_func(obj_handle_loose);
		ASSERT_EQUAL(1, my_test_obj::instance_count);
	}

	{
		etl::loose_handle<my_test_obj> a(new my_test_obj(27)), b(new my_test_obj(42));
		a.swap(b);
		ASSERT_EQUAL(42, a->my_id);
		ASSERT_EQUAL(27, b->my_id);
	}

	ASSERT_EQUAL(3, my_test_obj::instance_count);
}

void
handle_cast_test()
{
	etl::handle<my_test_obj> obj;
	etl::handle<my_other_test_obj> other_obj;
	etl::loose_handle<my_other_test_obj> loose_obj;

	other_obj.spawn();
	loose_obj = other_obj;

	obj = etl::handle<my_test_obj>::cast_dynamic(loose_obj);

	ASSERT(obj == other_obj);
}

/* === E N T R Y P O I N T ================================================= */

int main()
{
	TEST_SUITE_BEGIN()

	TEST_FUNCTION(handle_basic_test);
	TEST_FUNCTION(handle_cast_test);
	TEST_FUNCTION(handle_general_use_test);
	TEST_FUNCTION(handle_inheritance_test);

	TEST_FUNCTION(loose_handle_test);

	TEST_FUNCTION(rhandle_general_use_test);

	TEST_SUITE_END()

	return tst_exit_status;
}
