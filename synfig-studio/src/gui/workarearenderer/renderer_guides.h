/* === S Y N F I G ========================================================= */
/*!	\file renderer_guides.h
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

#ifndef __SYNFIG_RENDERER_GUIDES_H
#define __SYNFIG_RENDERER_GUIDES_H

/* === H E A D E R S ======================================================= */

#include "workarearenderer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Renderer_Guides : public studio::WorkAreaRenderer
{
public:
	~Renderer_Guides();

	std::list<float>& get_guide_list_x();
	std::list<float>& get_guide_list_y();

protected:
	void render_vfunc(const Glib::RefPtr<Gdk::Window>& drawable, const Gdk::Rectangle& expose_area);
	bool event_vfunc(GdkEvent* event);

	bool get_enabled_vfunc()const;
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
