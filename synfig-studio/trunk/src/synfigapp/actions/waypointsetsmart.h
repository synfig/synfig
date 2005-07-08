/* === S Y N F I G ========================================================= */
/*!	\file waypointsetsmart.h
**	\brief Template File
**
**	$Id: waypointsetsmart.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_APP_ACTION_WAYPOINTSETSMART_H
#define __SYNFIG_APP_ACTION_WAYPOINTSETSMART_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/action.h>
#include <synfigapp/value_desc.h>
#include <synfig/valuenode_animated.h>

#include <list>
#include <set>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

namespace Action {

class WaypointSetSmart :
	public Super
{
private:

	synfig::ValueNode_Animated::Handle value_node;
	synfig::Waypoint waypoint;
	//synfig::WaypointModel waypoint_model;
	bool time_set;

	void calc_waypoint();
	void enclose_waypoint(const synfig::Waypoint& waypoint);

	std::set<synfig::Time> times;

public:

	WaypointSetSmart();

	static ParamVocab get_param_vocab();
	static bool is_canidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
