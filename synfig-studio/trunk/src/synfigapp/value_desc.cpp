/* === S Y N F I G ========================================================= */
/*!	\file value_desc.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2008 Chris Moore
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

#include "value_desc.h"
#include <string>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === M E T H O D S ======================================================= */

String
ValueDesc::get_description()const
{
	String description(_("ValueDesc"));

	if (parent_is_layer_param())
	{
		description = strprintf("'%s' -> %s", // layer -> parameter
						 get_layer()->get_non_empty_description().c_str(),
						 get_param_name().c_str());
		if (is_exported())
			description += strprintf(" (%s)", get_value_node()->get_id().c_str());
	}
	else if (parent_is_value_node())
	{
		synfig::LinkableValueNode::Handle value_node(synfig::LinkableValueNode::Handle::cast_reinterpret(get_parent_value_node()));
		synfig::Node* node;
		for(node=value_node.get();!node->parent_set.empty() && !dynamic_cast<Layer*>(node);node=*node->parent_set.begin());
		Layer::Handle parent_layer(dynamic_cast<Layer*>(node));
		if(parent_layer)
			description = strprintf("'%s' => %s", // layer -> sub-parameter
							 parent_layer->get_non_empty_description().c_str(),
							 value_node->link_local_name(get_index()).c_str());
		else
			description = value_node->link_local_name(get_index()); // sub-parameter
		if (is_exported())
			description += strprintf(" (%s)", get_value_node()->get_id().c_str());
	}
	else if (is_exported())
		description = strprintf(_("ValueNode (%s)"), get_value_node()->get_id().c_str());

	return description;
}
