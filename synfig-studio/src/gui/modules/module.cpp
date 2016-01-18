/* === S Y N F I G ========================================================= */
/*!	\file gtkmm/module.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "module.h"

#include <synfig/general.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
//using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Module::Module():status_(false)
{
}

Module::~Module()
{
	stop();
}

bool
Module::get_status()const
{
	return status_;
}

bool
Module::start()
{
	if(!get_status())
		status_=start_vfunc();
	return get_status();
}

bool
Module::stop()
{
	if(get_status() && count()<=1 && stop_vfunc())
	{
		status_=false;
		return true;
	}
	return false;
}
