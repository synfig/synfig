/* === S Y N F I G ========================================================= */
/*!	\file renderer_brush_overlay.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERER_BRUSH_OVERLAY_H
#define __SYNFIG_RENDERER_BRUSH_OVERLAY_H

/* === H E A D E R S ======================================================= */

#include "workarearenderer.h"
#include <synfig/surface.h>
#include <synfig/rect.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

	class Renderer_BrushOverlay : public studio::WorkAreaRenderer
	{
	private:
		synfig::Surface overlay_surface;
		synfig::Rect overlay_rect;
		bool overlay_enabled;

	public:
		Renderer_BrushOverlay();
		~Renderer_BrushOverlay();

		void set_overlay_surface(const synfig::Surface& surface, const synfig::Rect& rect);
		void clear_overlay();
		void enable_overlay(bool enabled = true);

	protected:
		void render_vfunc(const Glib::RefPtr<Gdk::Window>& drawable, const Gdk::Rectangle& expose_area) override;
		bool get_enabled_vfunc() const override;
	};

}; // END of namespace studio

#endif
