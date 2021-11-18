/* === S Y N F I G ========================================================= */
/*!	\file target_multi.cpp
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

#include "target_multi.h"
#include "string.h"
#include "surface.h"
#include "canvas.h"
#include "context.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Target_Multi::Target_Multi(Target_Scanline::Handle a,Target_Scanline::Handle b):
	a(a),
	b(b),
	buffer_a(),
	buffer_b()
{
}

Target_Multi::~Target_Multi()
{
}

void
Target_Multi::set_canvas(etl::handle<Canvas> c)
{
	canvas=c;
	RendDesc desc=canvas->rend_desc();
	a->set_canvas(c);
	b->set_canvas(c);
	set_rend_desc(&desc);
}

bool
Target_Multi::set_rend_desc(RendDesc *d)
{
	desc=*d;
	return a->set_rend_desc(d) && b->set_rend_desc(d);
}

bool
Target_Multi::init(ProgressCallback*)
{
	return a->init() && b->init();
}

bool
Target_Multi::add_frame(const synfig::Surface *surface, ProgressCallback *cb)
{
	return a->add_frame(surface, cb) && b->add_frame(surface, cb);
}

bool
Target_Multi::start_frame(ProgressCallback *cb)
{
	return a->start_frame(cb) && b->start_frame(cb);
}

void
Target_Multi::end_frame()
{
	a->end_frame();
	b->end_frame();
}

Color *
Target_Multi::start_scanline(int scanline)
{
	buffer_a=a->start_scanline(scanline);
	buffer_b=b->start_scanline(scanline);
	return buffer_a;
}

bool
Target_Multi::end_scanline()
{
	memcpy(buffer_b,buffer_a,sizeof(Color)*desc.get_w());
	return a->end_scanline() && b->end_scanline();
}
