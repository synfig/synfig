/* === S Y N F I G ========================================================= */
/*!	\file renderer_background.cpp
 **	\brief Implementation of the background renderer. Usually a checkerboard
 **
 **	$Id$
 **
 **	\legal
 **	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
 **	Copyright (c) 2013 Carlos López
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

#include "renderer_background.h"
#include <gui/workarea.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_Background::~Renderer_Background()
	{ }

bool
Renderer_Background::get_enabled_vfunc()const
	{ return true; }

void
Renderer_Background::render_vfunc(
	const Glib::RefPtr<Gdk::Window>& drawable,
	const Gdk::Rectangle& /*expose_area*/ )
{
    assert(get_work_area());
    if(!get_work_area())
        return;

    VectorInt offset = get_work_area()->get_windows_offset();
    int w=get_w();
    int h=get_h();

    Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();
    cr->save();
    cr->set_source(get_work_area()->get_background_pattern());
    cr->rectangle(offset[0], offset[1], w, h);
    cr->clip();
    cr->paint();
    cr->restore();
}
