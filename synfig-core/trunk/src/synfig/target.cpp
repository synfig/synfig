/* === S I N F G =========================================================== */
/*!	\file target.cpp
**	\brief Target Class Implementation
**
**	$Id: target.cpp,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $
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

#define SINFG_NO_ANGLE

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "target.h"
#include "string.h"
#include "canvas.h"
#include "target_null.h"
#include "target_null_tile.h"

using namespace sinfg;
using namespace etl;
using namespace std;

sinfg::Target::Book* sinfg::Target::book_;
sinfg::Target::ExtBook* sinfg::Target::ext_book_;

static sinfg::Gamma* default_gamma_;

/* === P R O C E D U R E S ================================================= */

bool
Target::subsys_init()
{
	book_=new sinfg::Target::Book();
	ext_book_=new sinfg::Target::ExtBook();
	
	default_gamma_=new sinfg::Gamma(1.0/2.2);
	//default_gamma_->set_black_level(0.05); // Default to 5% black level.
	
	book()["null"]=std::pair<sinfg::Target::Factory,String>(Target_Null::create,"null");
	ext_book()["null"]="null";
	book()["null-tile"]=std::pair<sinfg::Target::Factory,String>(Target_Null_Tile::create,"null-tile");
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
	remove_alpha(false)
{
}

void
sinfg::Target::set_canvas(Canvas::Handle c)
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
