/* === S Y N F I G ========================================================= */
/*!	\file valuenode_random.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "valuenode_random.h"
#include "synfig/valuenode_const.h"
#include "synfig/general.h"
#include "synfig/color.h"
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Random::ValueNode_Random(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	random.set_seed(time(NULL));

	set_link("radius",ValueNode_Const::create(Real(1)));
	set_link("seed",ValueNode_Const::create(random.get_seed()));
	set_link("speed",ValueNode_Const::create(Real(1)));
	set_link("smooth",ValueNode_Const::create(int(RandomNoise::SMOOTH_CUBIC)));

	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
		set_link("link",ValueNode_Const::create(value.get(Angle())));
		break;
	case ValueBase::TYPE_BOOL:
		set_link("link",ValueNode_Const::create(value.get(bool())));
		break;
	case ValueBase::TYPE_COLOR:
		set_link("link",ValueNode_Const::create(value.get(Color())));
		break;
	case ValueBase::TYPE_INTEGER:
		set_link("link",ValueNode_Const::create(value.get(int())));
		break;
	case ValueBase::TYPE_REAL:
		set_link("link",ValueNode_Const::create(value.get(Real())));
		break;
	case ValueBase::TYPE_TIME:
		set_link("link",ValueNode_Const::create(value.get(Time())));
		break;
	case ValueBase::TYPE_VECTOR:
		set_link("link",ValueNode_Const::create(value.get(Vector())));
		break;
	default:
		throw Exception::BadType(ValueBase::type_local_name(get_type()));
	}

	DCAST_HACK_ENABLE();
}

LinkableValueNode*
ValueNode_Random::create_new()const
{
	return new ValueNode_Random(get_type());
}

ValueNode_Random*
ValueNode_Random::create(const ValueBase &x)
{
	return new ValueNode_Random(x);
}

ValueNode_Random::~ValueNode_Random()
{
	unlink_all();
}

ValueBase
ValueNode_Random::operator()(Time t)const
{
	typedef const RandomNoise::SmoothType Smooth;

	Real	radius	= (*radius_)(t).get(Real());
	int		seed	= (*seed_)(t).get(int());
	int		smooth	= (*smooth_)(t).get(int());
	float	speed	= (*speed_ )(t).get(Real()) * t;

	random.set_seed(seed);

	switch(get_type())
	{
	case ValueBase::TYPE_ANGLE:
		return ((*link_)(t).get( Angle()) +
				Angle::deg(random(Smooth(smooth), 0, 0, 0, speed) * radius));

	case ValueBase::TYPE_BOOL:
		return round_to_int((*link_)(t).get(  bool()) +
							random(Smooth(smooth), 0, 0, 0, speed) * radius) > 0;

	case ValueBase::TYPE_COLOR:
		return (((*link_)(t).get( Color()) +
				 Color(random(Smooth(smooth), 0, 0, 0, speed),
					   random(Smooth(smooth), 1, 0, 0, speed),
					   random(Smooth(smooth), 2, 0, 0, speed), 0) * radius).clamped());

	case ValueBase::TYPE_INTEGER:
		return round_to_int((*link_)(t).get(   int()) +
							random(Smooth(smooth), 0, 0, 0, speed) * radius);

	case ValueBase::TYPE_REAL:
		return ((*link_)(t).get(  Real()) +
				random(Smooth(smooth), 0, 0, 0, speed) * radius);

	case ValueBase::TYPE_TIME:
		return ((*link_)(t).get(  Time()) +
				random(Smooth(smooth), 0, 0, 0, speed) * radius);

	case ValueBase::TYPE_VECTOR:
	{
		float length(random(Smooth(smooth), 0, 0, 0, speed) * radius);
		Angle::rad angle(random(Smooth(smooth), 1, 0, 0, speed) * PI);

		return ((*link_)(t).get(Vector()) +
				Vector(Angle::cos(angle).get(), Angle::sin(angle).get()) * length);
	}

	default:
		assert(0);
		break;
	}

	return ValueBase();
}


String
ValueNode_Random::get_name()const
{
	return "random";
}

String
ValueNode_Random::get_local_name()const
{
	return _("Random");
}

bool
ValueNode_Random::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_,   get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(radius_, ValueBase::TYPE_REAL);
	case 2: CHECK_TYPE_AND_SET_VALUE(seed_,   ValueBase::TYPE_INTEGER);
	case 3: CHECK_TYPE_AND_SET_VALUE(speed_,  ValueBase::TYPE_REAL);
	case 4: CHECK_TYPE_AND_SET_VALUE(smooth_, ValueBase::TYPE_INTEGER);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Random::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return link_;
	case 1: return radius_;
	case 2: return seed_;
	case 3: return speed_;
	case 4: return smooth_;
	}
	return 0;
}

int
ValueNode_Random::link_count()const
{
	return 5;
}

String
ValueNode_Random::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return "link";
	case 1: return "radius";
	case 2: return "seed";
	case 3: return "speed";
	case 4: return "smooth";
	}
	return String();
}

String
ValueNode_Random::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return _("Link");
	case 1: return _("Radius");
	case 2: return _("Seed");
	case 3: return _("Animation Speed");
	case 4: return _("Interpolation");
	}
	return String();
}

int
ValueNode_Random::get_link_index_from_name(const String &name)const
{
	if(name=="link"  ) return 0;
	if(name=="radius") return 1;
	if(name=="seed"  ) return 2;
	if(name=="speed" ) return 3;
	if(name=="smooth") return 4;
	throw Exception::BadLinkName(name);
}

bool
ValueNode_Random::check_type(ValueBase::Type type)
{
	return
		type==ValueBase::TYPE_ANGLE		||
		type==ValueBase::TYPE_BOOL		||
		type==ValueBase::TYPE_COLOR		||
		type==ValueBase::TYPE_INTEGER	||
		type==ValueBase::TYPE_REAL		||
		type==ValueBase::TYPE_TIME		||
		type==ValueBase::TYPE_VECTOR	;
}

ValueNode*
ValueNode_Random::clone(const GUID& deriv_guid)const
{
	ValueNode_Random* ret = (ValueNode_Random*)LinkableValueNode::clone(deriv_guid);
	ret->randomize_seed();
	return ret;
}

void
ValueNode_Random::randomize_seed()
{
	int i = get_link_index_from_name("seed");
	ValueNode::Handle link = get_link_vfunc(i);
	if(!link->is_exported() && link->get_name() == "constant")
	{
		int seed = time(NULL) + rand();
		if (seed < 0) seed = -seed;
		random.set_seed(seed);
		set_link(i, ValueNode_Const::create(seed));
	}
}
