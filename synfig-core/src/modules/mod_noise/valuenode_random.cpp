/* === S Y N F I G ========================================================= */
/*!	\file valuenode_random.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#include "valuenode_random.h"
#include <synfig/valuenodes/valuenode_const.h>
#include <synfig/valuenode_registry.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include "synfig/color.h"
#include <synfig/vector.h>

#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Random, RELEASE_VERSION_0_61_08, "random", "Random")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Random::ValueNode_Random(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	random.set_seed(time(NULL));

	set_link("radius",ValueNode_Const::create(Real(1)));
	set_link("seed",ValueNode_Const::create(random.get_seed()));
	set_link("speed",ValueNode_Const::create(Real(1)));
	set_link("smooth",ValueNode_Const::create(int(RandomNoise::SMOOTH_CUBIC)));
	set_link("loop",ValueNode_Const::create(Real(0)));

	Type &type(get_type());
	if (type == type_angle)
		set_link("link",ValueNode_Const::create(value.get(Angle())));
	else
	if (type == type_bool)
		set_link("link",ValueNode_Const::create(value.get(bool())));
	else
	if (type == type_color)
		set_link("link",ValueNode_Const::create(value.get(Color())));
	else
	if (type == type_integer)
		set_link("link",ValueNode_Const::create(value.get(int())));
	else
	if (type == type_real)
		set_link("link",ValueNode_Const::create(value.get(Real())));
	else
	if (type == type_time)
		set_link("link",ValueNode_Const::create(value.get(Time())));
	else
	if (type == type_vector)
		set_link("link",ValueNode_Const::create(value.get(Vector())));
	else
		throw Exception::BadType(type.description.local_name);
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
	float	speed	= (*speed_ )(t).get(Real());
	int		loop	= int((((*loop_ )(t).get(Real())) * speed) + 0.5);
	speed *= t;

	random.set_seed(seed);

	Type &type(get_type());
	if (type == type_angle)
		return ((*link_)(t).get( Angle()) +
				Angle::deg(random(Smooth(smooth), 0, 0, 0, speed, loop) * radius));
	if (type == type_bool)
		return round_to_int((*link_)(t).get(  bool()) +
							random(Smooth(smooth), 0, 0, 0, speed, loop) * radius) > 0;
	if (type == type_color)
		return (((*link_)(t).get( Color()) +
				 Color(random(Smooth(smooth), 0, 0, 0, speed, loop),
					   random(Smooth(smooth), 1, 0, 0, speed, loop),
					   random(Smooth(smooth), 2, 0, 0, speed, loop), 0) * radius).clamped());
	if (type == type_integer)
		return round_to_int((*link_)(t).get(   int()) +
							random(Smooth(smooth), 0, 0, 0, speed, loop) * radius);
	if (type == type_real)
		return ((*link_)(t).get(  Real()) +
				random(Smooth(smooth), 0, 0, 0, speed, loop) * radius);
	if (type == type_time)
		return ((*link_)(t).get(  Time()) +
				random(Smooth(smooth), 0, 0, 0, speed, loop) * radius);
	if (type == type_vector)
	{
		float length(random(Smooth(smooth), 0, 0, 0, speed, loop) * radius);
		Angle::rad angle(random(Smooth(smooth), 1, 0, 0, speed, loop) * PI);
		return ((*link_)(t).get(Vector()) +
				Vector(Angle::cos(angle).get(), Angle::sin(angle).get()) * length);
	}

	assert(0);
	return ValueBase();
}

bool
ValueNode_Random::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_,   get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(radius_, type_real);
	case 2: CHECK_TYPE_AND_SET_VALUE(seed_,   type_integer);
	case 3: CHECK_TYPE_AND_SET_VALUE(speed_,  type_real);
	case 4: CHECK_TYPE_AND_SET_VALUE(smooth_, type_integer);
	case 5: CHECK_TYPE_AND_SET_VALUE(loop_,  type_real);
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
	case 5: return loop_;
	}
	return 0;
}

bool
ValueNode_Random::check_type(Type &type)
{
	return
		type==type_angle	||
		type==type_bool		||
		type==type_color	||
		type==type_integer	||
		type==type_real		||
		type==type_time		||
		type==type_vector;
}

ValueNode::Handle
ValueNode_Random::clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid)const
{
	etl::handle<ValueNode_Random> ret =
		etl::handle<ValueNode_Random>::cast_dynamic(
			LinkableValueNode::clone(canvas, deriv_guid) );
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

LinkableValueNode::Vocab
ValueNode_Random::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("The value node source that provides the central value"))
	);

	ret.push_back(ParamDesc(ValueBase(),"radius")
		.set_local_name(_("Radius"))
		.set_description(_("The value of the maximum random difference"))
	);

	ret.push_back(ParamDesc(ValueBase(),"seed")
		.set_local_name(_("Seed"))
		.set_description(_("Seeds the random number generator"))
	);

	ret.push_back(ParamDesc(ValueBase(),"speed")
		.set_local_name(_("Speed"))
		.set_description(_("Defines how often a new random value is chosen (in choices per second) "))
	);

	ret.push_back(ParamDesc(ValueBase(),"smooth")
		.set_local_name(_("Interpolation"))
		.set_description(_("Determines how the value is interpolated from one random choice to the next"))
		.set_hint("enum")
		.add_enum_value(RandomNoise::SMOOTH_DEFAULT,"default",_("No interpolation"))
		.add_enum_value(RandomNoise::SMOOTH_LINEAR,"linear",_("Linear"))
		.add_enum_value(RandomNoise::SMOOTH_COSINE,"cosine",_("Cosine"))
		.add_enum_value(RandomNoise::SMOOTH_SPLINE,"spline",_("Spline"))
		.add_enum_value(RandomNoise::SMOOTH_CUBIC,"cubic",_("Cubic"))
	);


	ret.push_back(ParamDesc(ValueBase(),"loop")
		.set_local_name(_("Loop Time"))
		.set_description(_("Makes the random value repeat after the given time"))
	);
	return ret;
}
