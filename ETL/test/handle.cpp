/*! ========================================================================
** Extended Template and Library Test Suite
** Handle Template Class Test
** $Id$
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>

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

int handle_basic_test()
{
	printf("handle: Size of a handle: %u\n",(unsigned int)sizeof(etl::handle<int>));
	printf("handle: Size of a loose_handle: %u\n",(unsigned int)sizeof(etl::loose_handle<int>));
	printf("handle: Size of a rhandle: %u\n",(unsigned int)sizeof(etl::rhandle<int>));
	printf("handle: Size of a shared_object: %u\n",(unsigned int)sizeof(etl::shared_object));
	printf("handle: Size of a rshared_object: %u\n",(unsigned int)sizeof(etl::rshared_object));

	printf("handle: Basic test: ");
	my_test_obj::instance_count=0;

	{
		etl::handle<my_test_obj> obj_handle(new my_test_obj(rand()));
	}

	if(my_test_obj::instance_count!=0)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: on create/destroy, instance count=%d, should be zero.\n",__LINE__,my_test_obj::instance_count);
		return 1;
	}

	{
		std::map<std::string, etl::handle<my_test_obj> > my_map;
		etl::handle<my_test_obj> obj_handle(new my_test_obj(rand()));
		my_map["bleh"]=obj_handle;
	}

	if(my_test_obj::instance_count!=0)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: on create/destroy, instance count=%d, should be zero.\n",__LINE__,my_test_obj::instance_count);
		return 1;
	}

	etl::handle<my_test_obj> obj_handle(new my_test_obj(rand()));

	if(obj_handle != obj_handle.constant())
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: on call to handle<>::constant().\n",__LINE__);
		return 1;
	}

	printf("PASSED\n");

	return 0;
}

int handle_general_use_test(void)
{
	printf("handle: General-use test: ");
	my_test_obj::instance_count=0;

	obj_list my_list, my_other_list;
	int i;

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back( obj_handle(new my_test_obj(rand())) );

	my_other_list=my_list;
	if(my_test_obj::instance_count!=NUMBER_OF_OBJECTS)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On copy, instance count=%d, should be %d.\n",__LINE__,my_test_obj::instance_count,NUMBER_OF_OBJECTS);
		return 1;
	}

	my_list.sort();
	if(my_test_obj::instance_count!=NUMBER_OF_OBJECTS)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On copy, instance count=%d, should be %d.\n",__LINE__,my_test_obj::instance_count,NUMBER_OF_OBJECTS);
		return 1;
	}

	my_list.clear();
	if(my_test_obj::instance_count!=NUMBER_OF_OBJECTS)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On copy's clear, instance count=%d, should be %d.\n",__LINE__,my_test_obj::instance_count,NUMBER_OF_OBJECTS);
		return 1;
	}

	{
		obj_handle a(new my_test_obj(27)), b(new my_test_obj(42));
		a.swap(b);
		if (a->my_id != 42 || b->my_id != 27)
		{
			printf("FAILED!\n");
			printf(__FILE__":%d: On swap (27,42) gave (%d,%d), should be (42,27).\n",__LINE__,a->my_id,b->my_id);
			return 1;
		}
	}

	my_other_list.clear();
	if(my_test_obj::instance_count)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On clear, instance count=%d, should be zero.\n",__LINE__,my_test_obj::instance_count);
		return 1;
	}

	printf("PASSED\n");

	return 0;
}

	struct ListItem
	{
		robj_handle obj;
		int bleh;
		int blah;
		ListItem(robj_handle obj,int bleh=1, int blah=2):
			obj(obj),bleh(bleh),blah(blah) { }
	};

int rhandle_general_use_test(void)
{


	printf("rhandle: General-use test: ");
	my_test_obj::instance_count=0;

	robj_list my_list;
	int i;

	robj_handle obj=	new my_test_obj(rand());
	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back(obj);

	obj_list my_other_list(my_list.begin(),my_list.end());



	if(obj.count()!=NUMBER_OF_OBJECTS*2+1)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On copy, handle count=%d, should be %d.\n",__LINE__,obj.count(),NUMBER_OF_OBJECTS*2+1);
		return 1;
	}

	if(obj.rcount()!=NUMBER_OF_OBJECTS+1)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On copy, rhandle count=%d, should be %d.\n",__LINE__,obj.rcount(),NUMBER_OF_OBJECTS+1);
		return 1;
	}

	my_list.sort();
	if(obj.rcount()!=NUMBER_OF_OBJECTS+1)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On copy, instance count=%d, should be %d.\n",__LINE__,obj.rcount(),NUMBER_OF_OBJECTS+1);
		return 1;
	}

	{robj_handle bleh(obj);}

	if(obj.rcount()!=NUMBER_OF_OBJECTS+1)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On copy, instance count=%d, should be %d.\n",__LINE__,obj.rcount(),NUMBER_OF_OBJECTS+1);
		return 1;
	}

	my_other_list.clear();

	if(obj.rcount()!=obj.count())
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On copy's clear, handle count (%d) != rhandle count (%d)\n",__LINE__,obj.count(),obj.rcount());
		return 1;
	}

	if(obj.rcount()!=NUMBER_OF_OBJECTS+1)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On copy's clear, instance count=%d, should be %d.\n",__LINE__,obj.rcount(),NUMBER_OF_OBJECTS+1);
		return 1;
	}

	robj_handle new_obj=	new my_test_obj(rand());

	int replacements=obj.replace(new_obj);

	if(replacements!=NUMBER_OF_OBJECTS+1)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: Only managed to replace %d, should have replaced %d\n",__LINE__,replacements,NUMBER_OF_OBJECTS+1);
		return 1;
	}

	if(obj!=new_obj)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On replace, handles should be equal.\n",__LINE__);
		return 1;
	}

	{
		robj_handle bleh(obj);
		robj_handle blah(obj.get());
	}


	my_list.clear();
	obj.detach();
	new_obj.detach();

	if(my_test_obj::instance_count)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On clear, instance count=%d, should be zero.\n",__LINE__,my_test_obj::instance_count);
		return 1;
	}



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

	if(my_test_obj::instance_count)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On clear, instance count=%d, should be zero.\n",__LINE__,my_test_obj::instance_count);
		return 1;
	}

	printf("PASSED\n");

	return 0;
}

int handle_inheritance_test(void)
{
	printf("handle: Inheritance test: ");
	my_test_obj::instance_count=0;
	my_other_test_obj::instance_count=0;

	other_obj_list my_other_list;
	int i;

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_other_list.push_back( other_obj_handle(new my_other_test_obj(rand())) );

	obj_list my_list(my_other_list.begin(),my_other_list.end());
	if(my_test_obj::instance_count!=NUMBER_OF_OBJECTS)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On copy, instance count=%d, should be %d.\n",__LINE__,my_test_obj::instance_count,NUMBER_OF_OBJECTS);
		return 1;
	}

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back( other_obj_handle(new my_other_test_obj(rand())) );
	if(my_other_test_obj::instance_count!=NUMBER_OF_OBJECTS*2 ||
	   my_test_obj::instance_count!=my_other_test_obj::instance_count)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On inherited copy, instance count=%d, should be %d.\n",__LINE__,my_test_obj::instance_count,NUMBER_OF_OBJECTS*2);
		return 1;
	}

	my_list.sort();
	my_other_list.sort();
	if(my_test_obj::instance_count!=NUMBER_OF_OBJECTS*2)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On sort, instance count=%d, should be %d.\n",__LINE__,my_test_obj::instance_count,NUMBER_OF_OBJECTS*2);
		return 1;
	}

	my_list.clear();
	if(my_test_obj::instance_count!=NUMBER_OF_OBJECTS)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On clear, instance count=%d, should be %d.\n",__LINE__,my_test_obj::instance_count,NUMBER_OF_OBJECTS);
		return 1;
	}

	my_other_list.clear();
	if(my_test_obj::instance_count)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On clear, instance count=%d, should be zero.\n",__LINE__,my_test_obj::instance_count);
		return 1;
	}

	printf("PASSED\n");

	return 0;
}

void test_func(etl::handle<my_test_obj> handle)
{
	if(handle) { int i=handle.count(); i++; }
}

int loose_handle_test(void)
{
	printf("handle: loose_handle test: ");
	my_test_obj::instance_count=0;

	etl::loose_handle<my_test_obj> obj_handle_loose;
	etl::handle<my_test_obj> obj_handle2;

	{
		etl::handle<my_test_obj> obj_handle(new my_test_obj(rand()));
		if(my_test_obj::instance_count!=1)
		{
			printf("FAILED!\n");
			printf(__FILE__":%d: on handle assignment from new object, instance count=%d, should be 1.\n",__LINE__,my_test_obj::instance_count);
			return 1;
		}

		obj_handle_loose=obj_handle;
		if(obj_handle!=obj_handle_loose)
		{
			printf("FAILED!\n");
			printf(__FILE__":%d: on loose_handle assignment\n",__LINE__);
			return 1;
		}

		obj_handle2=obj_handle_loose;
		if(my_test_obj::instance_count!=1)
		{
			printf("FAILED!\n");
			printf(__FILE__":%d: on handle assignment from loose_handle, instance count=%d, should be 1.\n",__LINE__,my_test_obj::instance_count);
			return 1;
		}

		test_func(obj_handle_loose);
		if(my_test_obj::instance_count!=1)
		{
			printf("FAILED!\n");
			printf(__FILE__":%d: on handle assignment from loose_handle, instance count=%d, should be 1.\n",__LINE__,my_test_obj::instance_count);
			return 1;
		}
	}

	{
		etl::loose_handle<my_test_obj> a(new my_test_obj(27)), b(new my_test_obj(42));
		a.swap(b);
		if (a->my_id != 42 || b->my_id != 27)
		{
			printf("FAILED!\n");
			printf(__FILE__":%d: on loose_handle swap (27,42) gave (%d,%d), should be (42,27).\n",__LINE__,a->my_id,b->my_id);
			return 1;
		}
	}

	if(my_test_obj::instance_count!=3)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: on create/destroy, instance count=%d, should be 3.\n",__LINE__,my_test_obj::instance_count);
		return 1;
	}

	printf("PASSED\n");
	return 0;
}

int handle_cast_test()
{
	printf("handle: casting test: ");

	etl::handle<my_test_obj> obj;
	etl::handle<my_other_test_obj> other_obj;
	etl::loose_handle<my_other_test_obj> loose_obj;

	other_obj.spawn();
	loose_obj=other_obj;

	obj=etl::handle<my_test_obj>::cast_dynamic(loose_obj);

	if(obj!=other_obj)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: on handle assignment from loose_handle.\n",__LINE__);
		return 1;
	}

	printf("PASSED\n");
	return 0;
}

/* === E N T R Y P O I N T ================================================= */

int main()
{
	int error=0;

	error+=handle_basic_test();
	error+=handle_cast_test();
	error+=handle_general_use_test();
	error+=handle_inheritance_test();
	error+=loose_handle_test();
	error+=rhandle_general_use_test();

	return error;
}
