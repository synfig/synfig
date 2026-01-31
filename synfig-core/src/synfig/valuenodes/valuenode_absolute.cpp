/* === S Y N F I G ========================================================= */
/*!	\file valuenode_absolute.cpp
**	\brief Implementation of the "Absolute" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
**  Copyright (c) 2025 BobSynfig
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

#include "valuenode_absolute.h"
#include "valuenode_const.h"

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <stdexcept>
#include <synfig/misc.h>
#include <synfig/angle.h>
#include <synfig/real.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Absolute, RELEASE_VERSION_1_6_0, "absolute", N_("Absolute"))

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Absolute::ValueNode_Absolute(const ValueBase& value):
	LinkableValueNode( value.get_type() )
{
	init_children_vocab();

	Type& type( value.get_type() );
		
	if (type == type_angle)
		set_link("link", ValueNode_Const::create( value.get( Angle() ) ) );
	else
	if (type == type_integer)
		set_link("link", ValueNode_Const::create( value.get( int()   ) ) );
	else
	if (type == type_real)
		set_link("link", ValueNode_Const::create( value.get( Real()  ) ) );
	else
	{
		assert(0);
		throw std::runtime_error( get_local_name() + _(":Bad type ") + type.description.local_name);
	}

	assert(value_node);
	assert(value_node->get_type() == type);
	assert(get_type() == type);
}

LinkableValueNode*
ValueNode_Absolute::create_new() const
{
	return new ValueNode_Absolute( get_type() );
}

ValueNode_Absolute*
ValueNode_Absolute::create(const ValueBase& value, etl::loose_handle<Canvas>)
{
	return new ValueNode_Absolute( value );
}

synfig::ValueNode_Absolute::~ValueNode_Absolute()
{
	unlink_all();
}

synfig::ValueBase
synfig::ValueNode_Absolute::operator()(Time t) const
{
	DEBUG_LOG("SYNFIG_DEBUG_VALUENODE_OPERATORS",
		"%s:%d operator()\n", __FILE__, __LINE__);

	if (!value_node)
		throw std::runtime_error( 
				strprintf( "ValueNode_Absolute: %s",
				           _("My parameter isn't set!" )
				)
			);
	else
	if ( get_type() == type_angle  )
		return 
			Angle::deg(
				std::abs(
					Angle::deg(
						(*value_node)(t).get( Angle() )
					).get()
				)
			);
	else
	if ( get_type() == type_integer)
	    return std::abs( (*value_node)(t).get( int()   ) );
	else
	if ( get_type() == type_real   )
		return std::abs( (*value_node)(t).get( Real()  ) );

	assert(0);
	return ValueBase();
}

synfig::ValueBase
synfig::ValueNode_Absolute::get_inverse(const Time& t, const synfig::ValueBase& target_value) const
{
	const Type& target_type = target_value.get_type();
	
	if (target_type == type_angle)
		return target_value.get( Angle() );
	
    if (target_type == type_integer)
		return target_value.get( int()   );
	
	if (target_type == type_real)
		return target_value.get( Real()  );
	
	throw std::runtime_error(
		strprintf( "ValueNode_%s: %s: %s",
				   get_name().c_str(),
				   _("Attempting to get the inverse of a non invertible Valuenode"),
				   _("Invalid value type")
	    )
	);
}

LinkableValueNode::InvertibleStatus
synfig::ValueNode_Absolute::is_invertible(const Time& t, const ValueBase& target_value, int* link_index) const
{
	if ( !t.is_valid() )
		return INVERSE_ERROR_BAD_TIME;

	const Type& type = target_value.get_type();
	
	if ( type != type_angle   &&
		 type != type_real    &&
	     type != type_integer  )
		return INVERSE_ERROR_BAD_TYPE;

	if (link_index)
		*link_index = get_link_index_from_name("link");

	return INVERSE_OK;
}

bool
ValueNode_Absolute::set_link_vfunc(int i, ValueNode::Handle value)
{
	assert( i>=0 && i<link_count() );

	switch(i)
	{
		case 0: CHECK_TYPE_AND_SET_VALUE(value_node, get_type());
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Absolute::get_link_vfunc(int i) const
{
	assert( i>=0 && i<link_count() );

	if (i == 0) return value_node;
	
	return nullptr;
}

bool
ValueNode_Absolute::check_type(Type &type)
{
	return
		type == type_angle   ||
		type == type_integer ||
		type == type_real;
}

LinkableValueNode::Vocab
ValueNode_Absolute::get_children_vocab_vfunc() const
{
	if ( children_vocab.size() )
		return children_vocab;
	
	LinkableValueNode::Vocab ret;

	ret.push_back( 
		ParamDesc("link")
			.set_local_name ( _("Link") )
			.set_description( _("The value node used to obtain the absolute value") )
	);

	return ret;
}
