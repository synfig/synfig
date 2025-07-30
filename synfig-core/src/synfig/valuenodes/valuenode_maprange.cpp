/* === S Y N F I G ========================================================= */
/*!	\file valuenode_maprange.cpp
**	\brief Implementation of the "Map Range" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2025 Synfig Contributors
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

#include "valuenode_maprange.h"

#include <stdexcept>

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenodes/valuenode_const.h>
#include <synfig/valuenode_registry.h>
#include <synfig/vector.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_MapRange, RELEASE_VERSION_1_6_0, "map_range", N_("Map Range"))

/* === P R O C E D U R E S ================================================= */

template<class T>
T
linear_map_range(const T& value, const T& from_min, const T& from_max, const T& to_min, const T& to_max, bool clamp)
{
	if (from_min == from_max) {
		if (value < from_min)
			return to_min;
		return to_max;
	}

	T ret = to_min + (value - from_min) * Real(to_max - to_min) / Real(from_max - from_min);
	if (clamp)
		return synfig::clamp(ret, to_min, to_max);
	return ret;
}

template<class T>
T
linear_map_range(const ValueBase& value, const ValueBase& from_min, const ValueBase& from_max, const ValueBase& to_min, const ValueBase& to_max, bool clamp)
{
	return linear_map_range<T>(value.get(T()), from_min.get(T()), from_max.get(T()), to_min.get(T()), to_max.get(T()), clamp);
}

template<>
Angle
linear_map_range(const Angle& value, const Angle& from_min, const Angle& from_max, const Angle& to_min, const Angle& to_max, bool clamp)
{
	if (from_min == from_max) {
		if (value < from_min)
			return to_min;
		return to_max;
	}

	Angle ret = to_min + (value - from_min) * Angle::deg(to_max - to_min).get() / Angle::deg(from_max - from_min).get();
	if (clamp)
		return synfig::clamp(ret, to_min, to_max);
	return ret;
}

template<>
Vector
linear_map_range(const Vector& value, const Vector& from_min, const Vector& from_max, const Vector& to_min, const Vector& to_max, bool clamp)
{
	return Vector(
		linear_map_range<Real>(value[0], from_min[0], from_max[0], to_min[0], to_max[0], clamp),
		linear_map_range<Real>(value[1], from_min[1], from_max[1], to_min[1], to_max[1], clamp)
	);
}

/* === M E T H O D S ======================================================= */

ValueNode_MapRange::ValueNode_MapRange(const ValueBase& value)
	: LinkableValueNode(value.get_type())
{
	init_children_vocab();

	set_link("interpolation", ValueNode_Const::create(int(0)));
	set_link("clamp", ValueNode_Const::create(bool(true)));

	const Type& type(value.get_type());

	if (type == type_angle) {
		set_link("link", ValueNode_Const::create(value.get(Angle())));
		set_link("from_min", ValueNode_Const::create(Angle::deg(0)));
		set_link("from_max", ValueNode_Const::create(Angle::deg(90)));
		set_link("to_min", ValueNode_Const::create(Angle::deg(-180)));
		set_link("to_max", ValueNode_Const::create(Angle::deg(180)));
	} else if (type == type_integer) {
		set_link("link", ValueNode_Const::create(value.get(int())));
		set_link("from_min", ValueNode_Const::create(int(0)));
		set_link("from_max", ValueNode_Const::create(int(10)));
		set_link("to_min", ValueNode_Const::create(int(-5)));
		set_link("to_max", ValueNode_Const::create(int(5)));
	} else if (type == type_real) {
		set_link("link", ValueNode_Const::create(value.get(Real())));
		set_link("from_min", ValueNode_Const::create(Real(0)));
		set_link("from_max", ValueNode_Const::create(Real(1)));
		set_link("to_min", ValueNode_Const::create(Real(-1)));
		set_link("to_max", ValueNode_Const::create(Real(1)));
	} else if (type == type_time) {
		set_link("link", ValueNode_Const::create(value.get(Time())));
		set_link("from_min", ValueNode_Const::create(Time(0)));
		set_link("from_max", ValueNode_Const::create(Time(10)));
		set_link("to_min", ValueNode_Const::create(Time(0)));
		set_link("to_max", ValueNode_Const::create(Time(5)));
	} else if (type == type_vector) {
		set_link("link", ValueNode_Const::create(value.get(Vector())));
		set_link("from_min", ValueNode_Const::create(Vector(0, -1)));
		set_link("from_max", ValueNode_Const::create(Vector(1, 1)));
		set_link("to_min", ValueNode_Const::create(Vector(-1, 0)));
		set_link("to_max", ValueNode_Const::create(Vector(1, 1)));
	} else {
		assert(0);
		throw std::runtime_error(get_local_name() + _(":Bad type ") + type.description.local_name);
	}
}

LinkableValueNode*
ValueNode_MapRange::create_new() const
{
	return new ValueNode_MapRange(get_type());
}

ValueNode_MapRange*
ValueNode_MapRange::create(const ValueBase& value, etl::loose_handle<Canvas>)
{
	return new ValueNode_MapRange(value);
}

ValueNode_MapRange::~ValueNode_MapRange()
{
	unlink_all();
}

bool
ValueNode_MapRange::check_type(Type& type)
{
	return type == type_angle
		   || type == type_integer
		   || type == type_real
		   || type == type_time
		   || type == type_vector;
}

ValueBase
ValueNode_MapRange::operator()(Time t) const
{
	DEBUG_LOG("SYNFIG_DEBUG_VALUENODE_OPERATORS",
			  "%s:%d operator()\n", __FILE__, __LINE__);

	if (!interpolation_ || !clamp_ || !link_ || !from_min_ || !from_max_ || !to_min_ || !to_max_)
		throw std::runtime_error(strprintf("ValueNode_MapRange: %s",_("One or more of my parameters aren't set!")));

	const Type& type(get_type());
	const ValueBase from_min = (*from_min_)(t);
	const ValueBase from_max = (*from_max_)(t);
	const ValueBase to_min = (*to_min_)(t);
	const ValueBase to_max = (*to_max_)(t);
	const bool clamp = (*clamp_)(t).get(bool());

	if (to_min == to_max)
		return to_min;

	//  new_value = to_min + (value - from_min) x (to_max - to_min) / (from_max - from_min)
	if (type == type_angle) {
		return linear_map_range<Angle>((*link_)(t), from_min, from_max, to_min, to_max, clamp);
	} if (type == type_integer)
		return linear_map_range<int>((*link_)(t), from_min, from_max, to_min, to_max, clamp);
	if (type == type_real)
		return linear_map_range<Real>((*link_)(t), from_min, from_max, to_min, to_max, clamp);
	if (type == type_time)
		return linear_map_range<Time>((*link_)(t), from_min, from_max, to_min, to_max, clamp);
	if (type == type_vector) {
		return linear_map_range<Vector>((*link_)(t), from_min, from_max, to_min, to_max, clamp);
	}

	assert(0);
	return ValueBase();
}

LinkableValueNode::InvertibleStatus
ValueNode_MapRange::is_invertible(const Time& t, const ValueBase& target_value, int* link_index) const
{
	if (!t.is_valid())
		return INVERSE_ERROR_BAD_TIME;

	const Type& type = target_value.get_type();
	if (type != get_type())
		return INVERSE_ERROR_BAD_TYPE;

	if (!interpolation_) {
		if (link_index)
			*link_index = get_link_index_from_name("interpolation");
		return InvertibleStatus::INVERSE_ERROR_BAD_PARAMETER;
	}

	if (!clamp_) {
		if (link_index)
			*link_index = get_link_index_from_name("clamp");
		return InvertibleStatus::INVERSE_ERROR_BAD_PARAMETER;
	}

	if (!from_min_) {
		if (link_index)
			*link_index = get_link_index_from_name("from_min");
		return InvertibleStatus::INVERSE_ERROR_BAD_PARAMETER;
	}

	if (!from_max_) {
		if (link_index)
			*link_index = get_link_index_from_name("from_max");
		return InvertibleStatus::INVERSE_ERROR_BAD_PARAMETER;
	}

	if (!to_min_) {
		if (link_index)
			*link_index = get_link_index_from_name("to_min");
		return InvertibleStatus::INVERSE_ERROR_BAD_PARAMETER;
	}

	if (!to_max_) {
		if (link_index)
			*link_index = get_link_index_from_name("to_max");
		return InvertibleStatus::INVERSE_ERROR_BAD_PARAMETER;
	}

	if (link_index)
		*link_index = get_link_index_from_name("link");
	return InvertibleStatus::INVERSE_OK;
}

ValueBase
ValueNode_MapRange::get_inverse(const Time& t, const ValueBase& target_value) const
{
	/*const*/ Type& type(get_type());
	if (target_value.get_type() != type) {
		synfig::error(_("ValueNode_MapRange::get_inverse(): target_value type is different from the value node type (%s vs. %s)"), target_value.get_type().description.local_name.c_str(), type.description.local_name.c_str());
		return ValueBase(type);
	}
	const ValueBase from_min = (*from_min_)(t);
	const ValueBase from_max = (*from_max_)(t);
	const ValueBase to_min = (*to_min_)(t);
	const ValueBase to_max = (*to_max_)(t);
	const bool clamp = (*clamp_)(t).get(bool());


	if (from_min == from_max)
		return from_min;

	if (to_min == to_max) {
		if (target_value < to_min)
			return from_min;
	}

	ValueBase value;
	if (!clamp) {
		value = target_value;
	} else {
		if (type != type_vector) {
			value = synfig::clamp(target_value, to_min, to_max);
		} else {
			value = Vector(
				synfig::clamp(target_value.get(Vector())[0], to_min.get(Vector())[0], to_max.get(Vector())[0]),
				synfig::clamp(target_value.get(Vector())[1], to_min.get(Vector())[1], to_max.get(Vector())[1])
			);
		}
	}

	if (type == type_angle) {
		return linear_map_range<Angle>(value, to_min, to_max, from_min, from_max, false);
	} if (type == type_integer)
		return linear_map_range<int>(value, to_min, to_max, from_min, from_max, false);
	if (type == type_real)
		return linear_map_range<Real>(value, to_min, to_max, from_min, from_max, false);
	if (type == type_time)
		return linear_map_range<Time>(value, to_min, to_max, from_min, from_max, false);
	if (type == type_vector) {
		return linear_map_range<Vector>(value, to_min, to_max, from_min, from_max, false);
	}
	return ValueBase();
}

bool
ValueNode_MapRange::set_link_vfunc(int i, ValueNode::Handle value)
{
	assert(i >= 0 && i < link_count());

	switch (i) {
		case 0: CHECK_TYPE_AND_SET_VALUE(interpolation_, type_integer);
		case 1: CHECK_TYPE_AND_SET_VALUE(clamp_,         type_bool);
		case 2: CHECK_TYPE_AND_SET_VALUE(link_,          get_type());
		case 3: CHECK_TYPE_AND_SET_VALUE(from_min_,      get_type());
		case 4: CHECK_TYPE_AND_SET_VALUE(from_max_,      get_type());
		case 5: CHECK_TYPE_AND_SET_VALUE(to_min_,        get_type());
		case 6: CHECK_TYPE_AND_SET_VALUE(to_max_,        get_type());
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_MapRange::get_link_vfunc(int i) const
{
	assert(i >= 0 && i < link_count());

	switch (i) {
		case 0: return interpolation_;
		case 1: return clamp_;
		case 2: return link_;
		case 3: return from_min_;
		case 4: return from_max_;
		case 5: return to_min_;
		case 6: return to_max_;
		default: return nullptr;
	}
}

LinkableValueNode::Vocab
ValueNode_MapRange::get_children_vocab_vfunc() const
{
	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc("interpolation")
		.set_local_name(_("Interpolation"))
		.set_description(_("The mathematical function to interpolate the value in the target range"))
		.set_hint("enum")
		.add_enum_value(Interpolation::LINEAR, "linear", _("Linear"))
	);

	ret.push_back(ParamDesc("clamp")
		.set_local_name(_("Clamp"))
		.set_description(_("If enabled, the output value is clamped to the target range"))
	);

	ret.push_back(ParamDesc("link")
		.set_local_name(_("Link"))
		.set_description(_("The value to be remapped to target range"))
	);

	ret.push_back(ParamDesc("from_min")
		.set_local_name(_("From Min"))
		.set_description(_("The lower bound of the range to remap from"))
	);

	ret.push_back(ParamDesc("from_max")
		.set_local_name(_("From Max"))
		.set_description(_("The upper bound of the range to remap from"))
	);

	ret.push_back(ParamDesc("to_min")
		.set_local_name(_("To Min"))
		.set_description(_("The lower bound of the target range"))
	);

	ret.push_back(ParamDesc("to_max")
		.set_local_name(_("To Max"))
		.set_description(_("The upper bound of the target range"))
	);

	return ret;
}
