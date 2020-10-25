/* === S Y N F I G ========================================================= */
/*!	\file transform.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "transform.h"
#include <algorithm>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::GUID
TransformStack::get_guid()const
{
	GUID ret(GUID::zero());

	for(const_iterator iter(begin());iter!=end();++iter)
		ret%=(*iter)->get_guid();
	return ret;
}

synfig::Vector
TransformStack::perform(const synfig::Vector& x)const
{
	synfig::Vector ret(x);

	if (getenv("SYNFIG_DEBUG_TRANSFORM_STACK")) printf("   PERFORM %s: %5.2f %5.2f", get_guid().get_string().substr(0,6).c_str(), ret[0], ret[1]);
	for(const_reverse_iterator iter(rbegin());iter!=rend();++iter)
	{
		ret=(*iter)->perform(ret);
		if (getenv("SYNFIG_DEBUG_TRANSFORM_STACK")) printf(" (%14s) %5.2f %5.2f", (*iter)->get_string().c_str(), ret[0], ret[1]);
	}
	if (getenv("SYNFIG_DEBUG_TRANSFORM_STACK")) printf("\n");

	return ret;
}

synfig::Vector
TransformStack::unperform(const synfig::Vector& x)const
{
	synfig::Vector ret(x);

	if (getenv("SYNFIG_DEBUG_TRANSFORM_STACK")) printf(" UNPERFORM %s: %5.2f %5.2f", get_guid().get_string().substr(0,6).c_str(), ret[0], ret[1]);
	for(const_iterator iter(begin());iter!=end();++iter)
	{
		ret=(*iter)->unperform(ret);
		if (getenv("SYNFIG_DEBUG_TRANSFORM_STACK")) printf(" (%14s) %5.2f %5.2f", (*iter)->get_string().c_str(), ret[0], ret[1]);
	}
	if (getenv("SYNFIG_DEBUG_TRANSFORM_STACK")) printf("\n");

	return ret;
}

synfig::Rect
TransformStack::perform(const synfig::Rect& x)const
{
	Point min(x.get_min());
	Point max(x.get_max());
	Rect ret(perform(min),perform(max));

	std::swap(min[1],max[1]);
	ret
		.expand(perform(min))
		.expand(perform(max))
	;
	return ret;
}

synfig::Rect
TransformStack::unperform(const synfig::Rect& x)const
{
	Point min(x.get_min());
	Point max(x.get_max());
	Rect ret(unperform(min),unperform(max));

	std::swap(min[1],max[1]);
	ret
		.expand(unperform(min))
		.expand(unperform(max))
	;
	return ret;
}

synfig::Rect
Transform::perform(const synfig::Rect& x)const
{
	if(x.area()>1000000000000.0)
		return Rect::full_plane();

	Point min(x.get_min());
	Point max(x.get_max());

	Rect ret(perform(min),perform(max));

	std::swap(min[1],max[1]);
	ret
		.expand(perform(min))
		.expand(perform(max))
	;
	return ret;
}

synfig::Rect
Transform::unperform(const synfig::Rect& x)const
{
	if(x.area()>1000000000000.0)
		return Rect::full_plane();

	Point min(x.get_min());
	Point max(x.get_max());

	Rect ret(unperform(min),unperform(max));

	std::swap(min[1],max[1]);
	ret
		.expand(unperform(min))
		.expand(unperform(max))
	;
	return ret;
}
