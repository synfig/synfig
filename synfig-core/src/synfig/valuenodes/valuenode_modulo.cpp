/* === S Y N F I G ========================================================= */
/*!	\file valuenode_modulo.cpp
**	\brief Implementation of the "Modulo" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2022 Synfig Contributors
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

#include "valuenode_modulo.h"
#include "valuenode_const.h"

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/vector.h>
#include <synfig/angle.h>
#include <synfig/real.h>

#include <ETL/misc>
#include <ETL/stringf>

#include <stdexcept>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Modulo, RELEASE_VERSION_1_6_0, "modulo", N_("Modulo"))

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Modulo::ValueNode_Modulo(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("divisor",ValueNode_Const::create(int(2)));
	set_link("scalar",ValueNode_Const::create(Real(1.0)));
	Type& type(value.get_type());

	if (!check_type(type)) {
		assert(0);
		throw std::runtime_error(get_local_name()+_(":Bad type ")+type.description.local_name);
	}

	set_link("link", ValueNode_Const::create(value));
}

LinkableValueNode*
ValueNode_Modulo::create_new()const
{
	return new ValueNode_Modulo(get_type());
}

ValueNode_Modulo*
ValueNode_Modulo::create(const ValueBase& value, etl::loose_handle<Canvas>)
{
	return new ValueNode_Modulo(value);
}

synfig::ValueNode_Modulo::~ValueNode_Modulo()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_Modulo::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	if(!dividend || !divisor)
		throw std::runtime_error(strprintf("ValueNode_Modulo: %s",_("One or both of my parameters aren't set!")));
	Type &type(get_type());
	if (type == type_angle)
		return Angle::deg(int(Angle::deg((*dividend)(t).get(Angle())).get())%(*divisor)(t).get(int()))*(*scalar)(t).get(Real());
	if (type == type_integer)
		return int((*dividend)(t).get(int())%(*divisor)(t).get(int()))*(*scalar)(t).get(Real());
	if (type == type_real)
		return Real(int((*dividend)(t).get(Real()))%(*divisor)(t).get(int()))*(*scalar)(t).get(Real());
	if (type == type_time)
		return Time(int((*dividend)(t).get(Time()))%(*divisor)(t).get(int()))*(*scalar)(t).get(Real());

	assert(0);
	return ValueBase();
}

bool
ValueNode_Modulo::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(dividend, get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(divisor, type_integer);
	case 2: CHECK_TYPE_AND_SET_VALUE(scalar, type_real);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Modulo::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
		case 0: return dividend;
		case 1: return divisor;
		case 2: return scalar;
		default: return 0;
	}
}

bool
ValueNode_Modulo::check_type(Type &type)
{
	return type==type_angle
		|| type==type_integer
		|| type==type_real
		|| type==type_time;
}

LinkableValueNode::Vocab
ValueNode_Modulo::get_children_vocab_vfunc() const
{
	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("Left hand side of the modulo (dividend or numerator)"))
	);

	ret.push_back(ParamDesc(ValueBase(),"divisor")
		.set_local_name(_("Divisor"))
		.set_description(_("Right hand side of the modulo (divisor or denominator)"))
	);

		ret.push_back(ParamDesc(ValueBase(),"scalar")
		.set_local_name(_("Scalar"))
		.set_description(_("Value to multiply the result of modulo operation"))
	);

	return ret;
}

LinkableValueNode::InvertibleStatus
ValueNode_Modulo::is_invertible(const Time& t, const ValueBase& target_value, int* link_index) const
{
	if (!t.is_valid())
		return INVERSE_ERROR_BAD_TIME;
	if (approximate_zero((*scalar)(t).get(Real()))) {
		if (link_index)
			*link_index = get_link_index_from_name("scalar");
		return INVERSE_ERROR_BAD_PARAMETER;
	}
	if (approximate_zero((*divisor)(t).get(int()))) {
		if (link_index)
			*link_index = get_link_index_from_name("divisor");
		return INVERSE_ERROR_BAD_PARAMETER;
	}
	const Type& type = target_value.get_type();
	if (type != get_type())
		return INVERSE_ERROR_BAD_TYPE;

	if (link_index)
		*link_index = get_link_index_from_name("link");
	return INVERSE_OK;
}

ValueBase
ValueNode_Modulo::get_inverse(const Time& t, const ValueBase& target_value) const
{
	Real scalar_value = (*scalar)(t).get(Real());

	if (approximate_zero(scalar_value))
		throw std::runtime_error(strprintf("ValueNode_%s: %s: %s",get_name().c_str(),_("Attempting to get the inverse of a non invertible Valuenode"),_("Scalar is zero")));

	const Type& type = target_value.get_type();
	const int max_value = (*divisor)(t).get(int()) - 1;

	if (type == type_angle) {
		int int_target = etl::round_to_int(Angle::deg(target_value.get(Angle())).get()) / scalar_value;
		Angle::deg ret = Angle::deg(int_target / scalar_value);
		return std::min(Angle::deg(max_value), std::max(Angle::deg(-max_value), ret));
	}
	if (type == type_integer) {
		int ret = target_value.get(int()) / scalar_value;
		return std::min(max_value, std::max(-max_value, ret));
	}
	if (type == type_real) {
		int int_target = etl::round_to_int(target_value.get(Real()));
		Real ret = int_target / scalar_value;
		return std::min(Real(max_value), std::max(Real(-max_value), ret));
	}
	if (type == type_time) {
		int int_target = etl::round_to_int(target_value.get(Time()));
		Time ret = int_target / scalar_value;
		return std::min(Time(max_value), std::max(Time(-max_value), ret));
	}
	throw std::runtime_error(strprintf("ValueNode_%s: %s: %s",get_name().c_str(),_("Attempting to get the inverse of a non invertible Valuenode"),_("Invalid value type")));
}
