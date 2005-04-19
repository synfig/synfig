/* === S I N F G =========================================================== */
/*!	\file template.cpp
**	\brief Template File
**
**	$Id: transform.cpp,v 1.2 2005/01/24 05:00:18 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
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

#include "transform.h"
#include <algorithm>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

sinfg::GUID
TransformStack::get_guid()const
{
	GUID ret(0);
	
	for(const_iterator iter(begin());iter!=end();++iter)
		ret%=(*iter)->get_guid();
	return ret;
}

sinfg::Vector
TransformStack::perform(const sinfg::Vector& x)const
{
	sinfg::Vector ret(x);
	
	for(const_reverse_iterator iter(rbegin());iter!=rend();++iter)
		ret=(*iter)->perform(ret);
	
	return ret;
}

sinfg::Vector
TransformStack::unperform(const sinfg::Vector& x)const
{
	sinfg::Vector ret(x);
	
	for(const_iterator iter(begin());iter!=end();++iter)
		ret=(*iter)->unperform(ret);
	
	return ret;
}

sinfg::Rect
TransformStack::perform(const sinfg::Rect& x)const
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

sinfg::Rect
TransformStack::unperform(const sinfg::Rect& x)const
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

sinfg::Rect
Transform::perform(const sinfg::Rect& x)const
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

sinfg::Rect
Transform::unperform(const sinfg::Rect& x)const
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
