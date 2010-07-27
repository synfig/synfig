/* === S Y N F I G ========================================================= */
/*!	\file target.cpp
**	\brief Target Class Implementation
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2010 Diego Barrios Romero
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "target.h"
#include "string.h"
#include "canvas.h"
#include "target_null.h"
#include "target_null_tile.h"
#include "targetparam.h"

using namespace synfig;
using namespace etl;
using namespace std;

synfig::Target::Book* synfig::Target::book_;
synfig::Target::ExtBook* synfig::Target::ext_book_;

static synfig::Gamma* default_gamma_;

/* === P R O C E D U R E S ================================================= */

bool
Target::subsys_init()
{
	book_=new synfig::Target::Book();
	ext_book_=new synfig::Target::ExtBook();
//! \todo Do not hard core gamma to 2.2
	default_gamma_=new synfig::Gamma(1.0/2.2);

	// At least one target must be available.
	book()["null"].factory =
		reinterpret_cast<synfig::Target::Factory>(&Target_Null::create);
	book()["null"].filename = "null";
	book()["null"].target_param = TargetParam();
	ext_book()["null"]="null";

	book()["null-tile"].factory =
		reinterpret_cast<synfig::Target::Factory>(&Target_Null_Tile::create);
	book()["null-tile"].filename = "null-tile";
	book()["null-tile"].target_param = TargetParam();
	ext_book()["null-tile"]="null-tile";

	return true;
}

bool
Target::subsys_stop()
{
	delete book_;
	delete ext_book_;
	delete default_gamma_;
	return true;
}

Target::Book&
Target::book()
{
	return *book_;
}

Target::ExtBook&
Target::ext_book()
{
	return *ext_book_;
}


/* === M E T H O D S ======================================================= */

Target::Target():
	quality_(4),
	gamma_(*default_gamma_),
	remove_alpha(false),
	avoid_time_sync_(false)
{
}

void
synfig::Target::set_canvas(etl::handle<Canvas> c)
{
	canvas=c;
	RendDesc desc=canvas->rend_desc();
	set_rend_desc(&desc);
}


Target::Handle
Target::create(const String &name, const String &filename,
			   synfig::TargetParam params)
{
	if(!book().count(name))
		return handle<Target>();

	return Target::Handle(book()[name].factory(filename.c_str(), params));
}
