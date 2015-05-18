/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/blur.cpp
**	\brief Blur
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "blur.h"

#endif

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

// BlurBase

void
rendering::BlurBase::set_type(Type x)
	{ if (get_type() != x) { type = x; changed_common_data(); } }

void
rendering::BlurBase::set_size(const Vector &x)
	{ if (get_size() != x) { size = x; changed_common_data(); } }

void
rendering::BlurBase::set_surface(const Surface::Handle &x)
	{ if (get_surface() != x) { surface = x; changed_common_data(); } }

void
rendering::BlurBase::set_task(const Task::Handle &x)
	{ if (get_task() != x) { task = x; changed_common_data(); } }

void
rendering::BlurBase::apply_common_data(const BlurBase &data)
{
	set_type(data.get_type());
	set_size(data.get_size());
	set_surface(data.get_surface());
	set_task(data.get_task());
}

void
rendering::BlurBase::changed_common_data()
{
	bool repeat = true;
	while(repeat)
	{
		repeat = false;
		for(DependentObject::AlternativeList::const_iterator i = get_alternatives().begin(); i != get_alternatives().end(); ++i)
			if (!Handle::cast_dynamic(*i))
				{ remove_alternative(*i); repeat = true; break; }
	}

	for(DependentObject::AlternativeList::const_iterator i = get_alternatives().begin(); i != get_alternatives().end(); ++i)
		Handle::cast_dynamic(*i)->apply_common_data(*this);
}

/* === E N T R Y P O I N T ================================================= */
