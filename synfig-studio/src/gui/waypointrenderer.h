/* === S Y N F I G ========================================================= */
/*!	\file waypointrenderer.h
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2019 Rodolfo R. Gomes
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

#ifndef __SYNFIG_STUDIO_WAYPOINTRENDERER_H
#define __SYNFIG_STUDIO_WAYPOINTRENDERER_H

/* === H E A D E R S ======================================================= */
#include <gui/timeplotdata.h>
#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class WaypointRenderer
{
public:
	static void
	render_time_point_to_window(
		const Cairo::RefPtr<Cairo::Context> &cr,
		const Gdk::Rectangle& area,
		const synfig::TimePoint &tp,
		bool selected,
		bool hover, bool double_outline);

	//! Callback called at every iteration of \ref foreach_visible_waypoint
	//! \param tp A visible TimePoint
	//! \param t The current time (including effects of layers: offset and zoom)
	//! \param data Custom data passed to \ref foreach_visible_waypoint
	/// \return Callback should return true to stop for-each loop
	typedef bool ForeachCallback(const synfig::TimePoint &tp, const synfig::Time &t, void *data);

	static void foreach_visible_waypoint(
		const synfigapp::ValueDesc &value_desc,
		const studio::TimePlotData &time_plot_data,
		std::function<ForeachCallback> foreach_callback,
		void* data = nullptr);

	static const synfig::Node::time_set & get_times_from_valuedesc(const synfigapp::ValueDesc &v);

}; // END of class WaypointRenderer

}; // END of namespace studio

/* === E N D =============================================================== */

#endif // WAYPOINTRENDERER_H
