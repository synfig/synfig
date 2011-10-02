/* === S Y N F I G ========================================================= */
/*!	\file wplistconverter.cpp
**	\brief Implementation for a converter between widths and positions
**	form a extended device and a equivalent ValueNode_WPList
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011 Carlos López
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

#endif

#include "wplistconverter.h"


/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

WPListConverter::WPListConverter()
{
	err2max=0.01;
}

void
WPListConverter::operator()(std::list<synfig::WidthPoint> &wp_out, const std::list<synfig::Point> &p, const std::list<synfig::Real> &w)
{
}

/* === E N T R Y P O I N T ================================================= */


