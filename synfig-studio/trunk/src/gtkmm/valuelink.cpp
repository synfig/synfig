/* === S I N F G =========================================================== */
/*!	\file valuelink.cpp
**	\brief ValueBase Link Implementation File
**
**	$Id: valuelink.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#include "valuelink.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;

using studio::ValueBaseLink;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

//structors
ValueBaseLink::ValueBaseLink()
{
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

int ValueBaseLink::get_link_index_from_name(const String &name)const
{
	throw Exception::BadLinkName(name);
}

//list management stuff
ValueBaseLink::list_type::const_iterator ValueBaseLink::findlink(ValueNode::Handle x) const
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
ValueBaseLink::list_type::iterator ValueBaseLink::findlink(ValueNode::Handle x)
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

void ValueBaseLink::add(ValueNode::Handle v)
{
	list_type::iterator i = findlink(v);
	
	if(i != list.end())
	{
		list.push_back(v);
	}
}

void ValueBaseLink::remove(ValueNode::Handle v)
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
