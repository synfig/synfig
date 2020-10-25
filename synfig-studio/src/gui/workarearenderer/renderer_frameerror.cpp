/* === S Y N F I G ========================================================= */
/*!	\file renderer_frameerror.h
**	\brief Workarea renderer of frame rendering error messages of current frame
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "renderer_frameerror.h"

#include <gui/workarea.h>
#include <gui/workarearenderer/renderer_canvas.h>

#endif

/* === U S I N G =========================================================== */


using namespace studio;

studio::Renderer_FrameError::~Renderer_FrameError()
{
}

void Renderer_FrameError::render_vfunc(const Glib::RefPtr<Gdk::Window>& drawable, const Gdk::Rectangle& /*expose_area*/)
{
	if (!get_work_area())
		return;

	const etl::handle<Renderer_Canvas> renderer_canvas = get_work_area()->get_renderer_canvas();
	if (renderer_canvas) {
		std::set<std::string> message_set;
		renderer_canvas->get_rendering_error_messages_for_time(get_work_area()->get_time(), message_set);
		if (!message_set.empty()) {
			std::string full_msg;
			for (const std::string& msg : message_set)
				full_msg.append(msg + "\n");

			Pango::FontDescription font;
			font.set_family("Monospace");

			int text_pixel_width = drawable->get_width() - 30;
			if (text_pixel_width < 30)
				text_pixel_width = 30;

			Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_work_area()->get_pango_context()));
			layout->set_text(full_msg);
			layout->set_font_description(font);
			layout->set_width(text_pixel_width * PANGO_SCALE);

			Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();
			cr->save();

			cr->set_source_rgb(1,0,0);
			cr->move_to(10, 30);
			layout->show_in_cairo_context(cr);

			cr->restore();
		}
	}
}
