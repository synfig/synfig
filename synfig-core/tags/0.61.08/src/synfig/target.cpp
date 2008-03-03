/* === S Y N F I G ========================================================= */
/*!	\file target.cpp
**	\brief Target Class Implementation
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

#define SYNFIG_NO_ANGLE

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "target.h"
#include "string.h"
#include "canvas.h"
#include "target_null.h"
#include "target_null_tile.h"

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

	default_gamma_=new synfig::Gamma(1.0/2.2);
	//default_gamma_->set_black_level(0.05); // Default to 5% black level.

	book()["null"]=std::pair<synfig::Target::Factory,String>(Target_Null::create,"null");
	ext_book()["null"]="null";
	book()["null-tile"]=std::pair<synfig::Target::Factory,String>(Target_Null_Tile::create,"null-tile");
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
Target::create(const String &name, const String &filename)
{
	if(!book().count(name))
		return handle<Target>();

	return Target::Handle(book()[name].first(filename.c_str()));
}
