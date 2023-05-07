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

struct MyTestObj : public etl::rshared_object
{
	static int instance_count;
	int my_id;
	explicit MyTestObj(int my_id=0):my_id(my_id)
	{
		instance_count++;
	}

	virtual ~MyTestObj()
	{
		if(instance_count==0)
			printf("Error, instance count is going past zero!\n");
		instance_count--;
	}

	bool operator<(const MyTestObj &rhs)const
	{
		return my_id<rhs.my_id;
	}
};

struct MyOtherTestObj : public MyTestObj
{
	static int instance_count;
	explicit MyOtherTestObj(int my_id=0):MyTestObj(my_id)
	{
		instance_count++;
	}
	virtual ~MyOtherTestObj()
	{
		if(instance_count==0)
			printf("Error, instance count is going past zero!\n");
		instance_count--;
	}
};

int MyTestObj::instance_count=0;
int MyOtherTestObj::instance_count=0;

typedef etl::handle<MyTestObj> ObjHandle;
typedef etl::rhandle<MyTestObj> RObjHandle;
typedef etl::handle<MyOtherTestObj> OtherObjHandle;
typedef std::list<ObjHandle> ObjList;
typedef std::list<OtherObjHandle> OtherObjList;
typedef std::list<RObjHandle> RObjList;


/* === P R O C E D U R E S ================================================= */

void
handle_basic_test()
{
	MyTestObj::instance_count=0;

	{
		etl::handle<MyTestObj> obj_handle(new MyTestObj(rand()));
	}

	ASSERT_EQUAL(0, MyTestObj::instance_count);

	{
		std::map<std::string, etl::handle<MyTestObj> > my_map;
		etl::handle<MyTestObj> obj_handle(new MyTestObj(rand()));
		my_map["bleh"]=obj_handle;
	}

	ASSERT_EQUAL(0, MyTestObj::instance_count);

	etl::handle<MyTestObj> obj_handle(new MyTestObj(rand()));

	ASSERT(obj_handle == obj_handle.constant());
}

void
handle_general_use_test()
{
	MyTestObj::instance_count=0;

	ObjList my_list, my_other_list;
	int i;

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back( ObjHandle(new MyTestObj(rand())) );

	my_other_list=my_list;
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, MyTestObj::instance_count);

	my_list.sort();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, MyTestObj::instance_count);

	my_list.clear();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, MyTestObj::instance_count);

	{
		ObjHandle a(new MyTestObj(27)), b(new MyTestObj(42));
		a.swap(b);
		ASSERT_EQUAL(42, a->my_id);
		ASSERT_EQUAL(27, b->my_id);
	}

	my_other_list.clear();
	ASSERT_EQUAL(0, MyTestObj::instance_count);
}

struct ListItem
{
	RObjHandle obj;
	int bleh;
	int blah;
	explicit ListItem(RObjHandle obj,int bleh=1, int blah=2):
		obj(obj),bleh(bleh),blah(blah) { }
};

void
rhandle_general_use_test()
{
	MyTestObj::instance_count = 0;

	RObjList my_list;
	int i;

	RObjHandle obj = new MyTestObj(rand());
	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back(obj);

	ObjList my_other_list(my_list.begin(),my_list.end());



	ASSERT_EQUAL(NUMBER_OF_OBJECTS*2+1, obj.count());

	ASSERT_EQUAL(NUMBER_OF_OBJECTS+1, obj.rcount());

	my_list.sort();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS+1, obj.rcount());

	{RObjHandle bleh(obj);}

	ASSERT_EQUAL(NUMBER_OF_OBJECTS+1, obj.rcount());

	my_other_list.clear();

	ASSERT(obj.rcount() == obj.count());

	ASSERT_EQUAL(NUMBER_OF_OBJECTS + 1, obj.rcount());

	RObjHandle new_obj = new MyTestObj(rand());

	int replacements = obj.replace(new_obj);

	ASSERT_EQUAL(NUMBER_OF_OBJECTS + 1, replacements);

	ASSERT(obj == new_obj);

	{
		RObjHandle bleh(obj);
		RObjHandle blah(obj.get());
	}


	my_list.clear();
	obj.detach();
	new_obj.detach();

	ASSERT_EQUAL(0, MyTestObj::instance_count);

	std::vector<ListItem> my_item_list;
	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_item_list.push_back(ListItem(new MyTestObj(rand()),3,4));


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

	ASSERT_EQUAL(0, MyTestObj::instance_count);
}

void
handle_inheritance_test()
{
	MyTestObj::instance_count = 0;
	MyOtherTestObj::instance_count = 0;

	OtherObjList my_other_list;
	int i;

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_other_list.push_back( OtherObjHandle(new MyOtherTestObj(rand())) );

	ObjList my_list(my_other_list.begin(),my_other_list.end());
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, MyTestObj::instance_count);

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back( OtherObjHandle(new MyOtherTestObj(rand())) );
	ASSERT_EQUAL(NUMBER_OF_OBJECTS * 2, MyOtherTestObj::instance_count);
	ASSERT(MyTestObj::instance_count == MyOtherTestObj::instance_count);

	my_list.sort();
	my_other_list.sort();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS * 2, MyTestObj::instance_count);

	my_list.clear();
	ASSERT_EQUAL(NUMBER_OF_OBJECTS, MyTestObj::instance_count);

	my_other_list.clear();
	ASSERT_EQUAL(0, MyTestObj::instance_count);
}

void test_func(etl::handle<MyTestObj> handle)
{
	if(handle) { int i=handle.count(); i++; }
}

void
loose_handle_test()
{
	MyTestObj::instance_count=0;

	etl::loose_handle<MyTestObj> obj_handle_loose;
	etl::handle<MyTestObj> obj_handle2;

	{
		etl::handle<MyTestObj> obj_handle(new MyTestObj(rand()));
		ASSERT_EQUAL(1, MyTestObj::instance_count);

		obj_handle_loose=obj_handle;
		ASSERT(obj_handle == obj_handle_loose);

		obj_handle2=obj_handle_loose;
		ASSERT_EQUAL(1, MyTestObj::instance_count);

		test_func(obj_handle_loose);
		ASSERT_EQUAL(1, MyTestObj::instance_count);
	}

	{
		etl::loose_handle<MyTestObj> a(new MyTestObj(27)), b(new MyTestObj(42));
		a.swap(b);
		ASSERT_EQUAL(42, a->my_id);
		ASSERT_EQUAL(27, b->my_id);
	}

	ASSERT_EQUAL(3, MyTestObj::instance_count);
}

void
handle_cast_test()
{
	etl::handle<MyTestObj> obj;
	etl::handle<MyOtherTestObj> other_obj;
	etl::loose_handle<MyOtherTestObj> loose_obj;

	other_obj.spawn();
	loose_obj = other_obj;

	obj = etl::handle<MyTestObj>::cast_dynamic(loose_obj);

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
