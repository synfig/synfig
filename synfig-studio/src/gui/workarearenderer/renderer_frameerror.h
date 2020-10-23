/* === S Y N F I G ========================================================= */
/*!	\file renderer_frameerror.h
**	\brief Workarea renderer of frame rendering error messages of current frame
**
**	$Id$
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

#ifndef SYNFIG_STUDIO_RENDERER_FRAMEERROR_H
#define SYNFIG_STUDIO_RENDERER_FRAMEERROR_H

#include "workarearenderer.h"

namespace studio {

class Renderer_FrameError : public studio::WorkAreaRenderer
{
public:
	~Renderer_FrameError();

	void render_vfunc(const Glib::RefPtr<Gdk::Window>& drawable, const Gdk::Rectangle& expose_area);
};

}; // END of namespace studio

#endif // SYNFIG_STUDIO_RENDERER_FRAMEERROR_H
