/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_canvasspecific.h
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

#ifndef __SYNFIG_STUDIO_DOCK_CANVASSPECIFIC_H
#define __SYNFIG_STUDIO_DOCK_CANVASSPECIFIC_H

/* === H E A D E R S ======================================================= */

#include <gui/docks/dockable.h>
#include <gui/instance.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class CanvasView;
class Instance;

class Dock_CanvasSpecific : public Dockable
{
protected:
	virtual void init_instance_vfunc(etl::loose_handle<Instance> instance);

	virtual void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);

private:
	void init_canvas_view(CanvasView* canvas_view);
	void init_instance(etl::handle<Instance> instance);
	void delete_instance(etl::handle<Instance> instance);
	void canvas_view_changed();
	void changed_canvas_view(etl::loose_handle<CanvasView> canvas_view) { return changed_canvas_view_vfunc(canvas_view); }
public:

	etl::loose_handle<studio::CanvasView> get_canvas_view();
	etl::loose_handle<synfigapp::CanvasInterface> get_canvas_interface();

	Dock_CanvasSpecific(const synfig::String& name, const synfig::String& local_name, synfig::String icon="");
	virtual ~Dock_CanvasSpecific();
}; // END of Dock_CanvasSpecific

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
