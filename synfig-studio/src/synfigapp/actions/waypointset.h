/* === S Y N F I G ========================================================= */
/*!	\file waypointset.h
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_APP_ACTION_WAYPOINTSET_H
#define __SYNFIG_APP_ACTION_WAYPOINTSET_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/action.h>
#include <synfig/waypoint.h>
#include <synfig/valuenodes/valuenode_animated.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Instance;

namespace Action {

class WaypointSet :
	public Undoable,
	public CanvasSpecific
{
private:

	synfig::ValueNode_Animated::Handle value_node;

	std::vector<synfig::Waypoint> waypoints;
	std::vector<synfig::Waypoint> old_waypoints;

	std::vector<synfig::Waypoint> overwritten_waypoints;

public:

	WaypointSet();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void perform();
	virtual void undo();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
