/* === S Y N F I G ========================================================= */
/*!	\file renderer_bonesetup.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "renderer_bonesetup.h"

#include <gui/localization.h>
#include <gui/workarea.h>
#include <cassert>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_BoneSetup::~Renderer_BoneSetup()
{
}

bool
Renderer_BoneSetup::get_enabled_vfunc()const
{
	return true;
}

void
Renderer_BoneSetup::render_vfunc(const Glib::RefPtr<Gdk::Window>& drawable,
								 const Gdk::Rectangle& /*expose_area*/ )
{
	assert(get_work_area());
	if(!get_work_area())
		return;

	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();
	Canvas::Handle canvas(get_work_area()->get_canvas());

	// Print out the bonesetup
	{
		Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_work_area()->get_pango_context()));

		bool recursive(get_work_area()->get_type_mask() & Duck::TYPE_BONE_RECURSIVE);
		if (recursive)
		{
			int w, h;
			layout->set_text(_("Bone Recursive Scale Mode"));
			layout->get_size(w, h);
			get_work_area()->bonesetup_width = int(w*1.0/Pango::SCALE);
			get_work_area()->bonesetup_height = int(h*1.0/Pango::SCALE);
		}
		else
			get_work_area()->timecode_width = get_work_area()->timecode_height = 0;

		Gdk::RGBA c("#5f0000");
		cr->set_source_rgb(c.get_red(), c.get_green(), c.get_blue());
		cr->move_to(bonesetup_x, bonesetup_y);
		layout->show_in_cairo_context(cr);
	}
}
