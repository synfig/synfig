/* === S I N F G =========================================================== */
/*!	\file duck.cpp
**	\brief Template File
**
**	$Id: duck.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "duck.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

int studio::Duck::duck_count(0);

struct _DuckCounter
{
	static int counter;
	~_DuckCounter()
	{
		if(counter)
			sinfg::error("%d ducks not yet deleted!",counter);
	}
} _duck_counter;

int _DuckCounter::counter(0);


/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Duck::Duck():
	origin(0,0),
	scalar(1),
	editable(false),
	radius_(false),
	tangent_(false)
{ duck_count++; _DuckCounter::counter++; }

Duck::Duck(const sinfg::Point &point):
	type_(TYPE_NONE),
	point(point),
	origin(0,0),
	scalar(1),
	guid_(0),
	editable(false),
	radius_(false),
	tangent_(false)
{ duck_count++; _DuckCounter::counter++;}

Duck::Duck(const sinfg::Point &point,const sinfg::Point &origin):
	point(point),
	origin(origin),
	scalar(1),
	guid_(0),
	editable(false),
	radius_(true),
	tangent_(false)
{ duck_count++; _DuckCounter::counter++;}

Duck::~Duck() { duck_count--; _DuckCounter::counter--;}

sinfg::GUID
Duck::get_data_guid()const
{
	if(value_desc_.is_value_node())
		return value_desc_.get_value_node()->get_guid();
	return GUID::hasher(get_name());
}

void
Duck::set_name(const sinfg::String &x)
{
	name=x;
	if(guid_==GUID::zero())
	{
		guid_=GUID::hasher(name);
	}
}


bool
Duck::operator==(const Duck &rhs)const
{
	if(this==&rhs)
		return true;
	return
		name==rhs.name &&
		scalar==rhs.scalar &&
		type_==rhs.type_ &&
		transform_stack_.size()==rhs.transform_stack_.size();
		//true;
		//(origin_duck?*origin_duck==*rhs.origin_duck:origin==rhs.origin) &&
		//(shared_point?*shared_point==*rhs.shared_point:point==rhs.point) ;
}

sinfg::Point
Duck::get_trans_point()const
{
	return transform_stack_.perform(get_sub_trans_point());
}
	
void
Duck::set_trans_point(const sinfg::Point &x)
{
	set_sub_trans_point(transform_stack_.unperform(x));
}

//! Sets the origin point.
void
Duck::set_origin(const sinfg::Point &x)
{
	origin=x; origin_duck=0;
}

//! Sets the origin point as another duck
void
Duck::set_origin(const etl::handle<Duck> &x)
{
	origin_duck=x;
}

//! Retrieves the origin location
sinfg::Point
Duck::get_origin()const
{
	return origin_duck?origin_duck->get_point():origin;
}

//! Retrieves the origin duck
const etl::handle<Duck> &
Duck::get_origin_duck() const
{
	return origin_duck;
}

//! Retrieves the origin location
sinfg::Point
Duck::get_trans_origin()const
{
	return transform_stack_.perform(get_sub_trans_origin());
}

sinfg::Point
Duck::get_sub_trans_point()const
{
	return get_point()*get_scalar()+get_sub_trans_origin();
}

void
Duck::set_sub_trans_point(const sinfg::Point &x)
{
	set_point((x-get_sub_trans_origin())/get_scalar());
}

sinfg::Point
Duck::get_sub_trans_origin()const
{
	return origin_duck?origin_duck->get_sub_trans_point():origin;
}
