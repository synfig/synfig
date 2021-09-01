/* === S Y N F I G ========================================================= */
/*!	\file valuelink.cpp
**	\brief ValueBase Link Implementation File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#include <synfig/general.h>

#include "valuelink.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

using studio::ValueBaseLink;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

//structors
ValueBaseLink::ValueBaseLink()
{
	assert(0); //CHECK: This class does not appear to be used.
}

ValueBaseLink::~ValueBaseLink()
{
}

//link access

ValueNode::LooseHandle ValueBaseLink::get_link_vfunc(int i)const
{
	/*list_type::const_iterator	it = list.begin();

	while(it != list.end() && i-- > 0)
	{
		++it;
	}

	if(it == list.end())
	{
		return ValueNode::LooseHandle();
	}else
	{
		return *it;
	}*/
	if(i >= 0 && i < (int)list.size())
	{
		return list[i];
	}else
	{
		return ValueNode::LooseHandle();
	}
}

//more link access
int ValueBaseLink::link_count()const
{
	return list.size();
}

String ValueBaseLink::link_local_name(int i)const
{
	ValueNode::LooseHandle h = get_link(i);

	if(h)
	{
		return h->get_local_name();
	}else return String();
}

String ValueBaseLink::link_name(int i)const
{
	ValueNode::LooseHandle h = get_link(i);

	if(h)
	{
		return h->get_name();
	}else return String();
}

int ValueBaseLink::get_link_index_from_name(const synfig::String &name)const
{
	for(int i = 0; i < link_count(); ++i)
		if (link_name(i) == name) return i;
	throw Exception::BadLinkName(name);
}

//list management stuff
ValueBaseLink::list_type::const_iterator ValueBaseLink::findlink(synfig::ValueNode::Handle x) const
{
	for(list_type::const_iterator i = list.begin(); i != list.end(); ++i)
	{
		if(*i == x)
		{
			return i;
		}
	}

	return list.end();
}
ValueBaseLink::list_type::iterator ValueBaseLink::findlink(synfig::ValueNode::Handle x)
{
	for(list_type::iterator i = list.begin(); i != list.end(); ++i)
	{
		if(*i == x)
		{
			return i;
		}
	}

	return list.end();
}

void ValueBaseLink::add(synfig::ValueNode::Handle v)
{
	list_type::iterator i = findlink(v);

	if(i != list.end())
	{
		list.push_back(v);
	}
}

void ValueBaseLink::remove(synfig::ValueNode::Handle v)
{
	list_type::iterator i = findlink(v);

	if(i != list.end())
	{
		if(i != list.end()-1)
		{
			*i = list.back();
		}
		list.pop_back();
	}
}
