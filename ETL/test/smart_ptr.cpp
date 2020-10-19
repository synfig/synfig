/*! ========================================================================
** Extended Template and Library Test Suite
** Smart Pointer Template Class Test
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

//#define DEBUGPOINT()	fprintf(stderr,__FILE__":%d: DEBUGPOINT\n",__LINE__)
#define DEBUGPOINT()

/* === H E A D E R S ======================================================= */

#include <ETL/smart_ptr>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>

/* === M A C R O S ========================================================= */

#define NUMBER_OF_OBJECTS	40000


/* === C L A S S E S ======================================================= */

struct my_test_obj
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

typedef etl::smart_ptr<my_test_obj> obj_smart_ptr;
typedef etl::smart_ptr<my_other_test_obj> other_obj_smart_ptr;
typedef std::list< obj_smart_ptr > obj_list;
typedef std::list< other_obj_smart_ptr > other_obj_list;

int smart_ptr_basic_test(void)
{
	printf("smart_ptr: Size of a smart_ptr: %u\n",(unsigned int)sizeof(obj_smart_ptr));
	printf("smart_ptr: Size of a reference_counter: %u\n",(unsigned int)sizeof(etl::reference_counter));


	printf("smart_ptr: Basic test: ");
	my_test_obj::instance_count=0;

	{
		etl::smart_ptr<my_test_obj> obj_smart_ptr(new my_test_obj(rand()));
	}

	if(my_test_obj::instance_count!=0)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: on create/destroy, instance count=%d, should be zero.\n",__LINE__,my_test_obj::instance_count);
		return 1;
	}

	{
		DEBUGPOINT();
		std::map<std::string, etl::smart_ptr<my_test_obj> > my_map;
		DEBUGPOINT();
		//etl::smart_ptr<my_test_obj> obj_smart_ptr(new my_test_obj(rand()));
		etl::smart_ptr<my_test_obj> temp;
		temp.spawn();
		DEBUGPOINT();
		temp.reset();
		DEBUGPOINT();
		my_map["bleh"]=temp;
		DEBUGPOINT();
	}

	if(my_test_obj::instance_count!=0)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: on create/destroy, instance count=%d, should be zero.\n",__LINE__,my_test_obj::instance_count);
		return 1;
	}

	etl::smart_ptr<my_test_obj> obj_smart_ptr(new my_test_obj(rand()));

	if(obj_smart_ptr != obj_smart_ptr.constant())
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: on call to smart_ptr<>::constant().\n",__LINE__);
		return 1;
	}

	printf("PASSED\n");

	return 0;
}

int smart_ptr_general_use_test(void)
{
	printf("smart_ptr: General-use test: ");
	my_test_obj::instance_count=0;

	obj_list my_list, my_other_list;
	int i;

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back( obj_smart_ptr(new my_test_obj(rand())) );

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

int smart_ptr_inheritance_test(void)
{
	printf("smart_ptr: Inheritance test: ");
	my_test_obj::instance_count=0;
	my_other_test_obj::instance_count=0;

	other_obj_list my_other_list;
	int i;

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_other_list.push_back( other_obj_smart_ptr(new my_other_test_obj(rand())) );

	obj_list my_list(my_other_list.begin(),my_other_list.end());
	if(my_test_obj::instance_count!=NUMBER_OF_OBJECTS)
	{
		printf("FAILED!\n");
		printf(__FILE__":%d: On copy, instance count=%d, should be %d.\n",__LINE__,my_test_obj::instance_count,NUMBER_OF_OBJECTS);
		return 1;
	}

	for(i=0;i<NUMBER_OF_OBJECTS;i++)
		my_list.push_back( other_obj_smart_ptr(new my_other_test_obj(rand())) );
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

/* === E N T R Y P O I N T ================================================= */

int main()
{
	int error=0;

	error+=smart_ptr_basic_test();
	error+=smart_ptr_general_use_test();
	error+=smart_ptr_inheritance_test();

	return error;
}
