/* === S Y N F I G ========================================================= */
/*!	\file duck.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "duck.h"
#include <ETL/misc>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
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
			synfig::error("%d ducks not yet deleted!",counter);
	}
} _duck_counter;

int _DuckCounter::counter(0);


/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Duck::Duck():
	rotations(synfig::Angle::deg(0)),
	origin(0,0),
	scalar(1),
	editable(false),
	radius_(false),
	tangent_(false),
	hover_(false)
{ duck_count++; _DuckCounter::counter++; }

Duck::Duck(const synfig::Point &point):
	type_(TYPE_NONE),
	point(point),
	rotations(synfig::Angle::deg(0)),
	origin(0,0),
	scalar(1),
	guid_(0),
	editable(false),
	radius_(false),
	tangent_(false)
{ duck_count++; _DuckCounter::counter++;}

Duck::Duck(const synfig::Point &point,const synfig::Point &origin):
	point(point),
	rotations(synfig::Angle::deg(0)),
	origin(origin),
	scalar(1),
	guid_(0),
	editable(false),
	radius_(true),
	tangent_(false)
{ duck_count++; _DuckCounter::counter++;}

Duck::~Duck() { duck_count--; _DuckCounter::counter--;}

synfig::GUID
Duck::get_data_guid()const
{
	if(value_desc_.is_value_node())
		return value_desc_.get_value_node()->get_guid();
	return GUID::hasher(get_name());
}

void
Duck::set_name(const synfig::String &x)
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

synfig::Point
Duck::get_trans_point()const
{
	return transform_stack_.perform(get_sub_trans_point());
}

void
Duck::set_trans_point(const synfig::Point &x)
{
	set_sub_trans_point(transform_stack_.unperform(x));
}

//! Sets the origin point.
void
Duck::set_origin(const synfig::Point &x)
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
synfig::Point
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
synfig::Point
Duck::get_trans_origin()const
{
	return transform_stack_.perform(get_sub_trans_origin());
}

synfig::Point
Duck::get_sub_trans_point()const
{
	return get_point()*get_scalar()+get_sub_trans_origin();
}

void
Duck::set_sub_trans_point(const synfig::Point &x)
{
	if (get_type() == Duck::TYPE_TANGENT ||
		get_type() == Duck::TYPE_ANGLE)
	{
		Angle old_angle = get_point().angle();
		set_point((x-get_sub_trans_origin())/get_scalar());
		Angle change = get_point().angle() - old_angle;
		while (change < Angle::deg(-180)) change += Angle::deg(360);
		while (change > Angle::deg(180)) change -= Angle::deg(360);
		int old_quarters = round_to_int(Angle::deg(rotations).get()/90);
		rotations += change;
		int new_quarters = round_to_int(Angle::deg(rotations).get()/90);
		if (old_quarters != new_quarters)
			synfig::info("rotation: %.2f turns", new_quarters/4.0);
	}
	else
		set_point((x-get_sub_trans_origin())/get_scalar());
}

synfig::Point
Duck::get_sub_trans_origin()const
{
	return origin_duck?origin_duck->get_sub_trans_point():origin;
}
