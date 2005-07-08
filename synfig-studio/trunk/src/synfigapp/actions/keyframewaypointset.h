/* === S Y N F I G ========================================================= */
/*!	\file keyframewaypointset.h
**	\brief Template File
**
**	$Id: keyframewaypointset.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SYNFIG_APP_ACTION_KEYFRAMEWAYPOINTSET_H
#define __SYNFIG_APP_ACTION_KEYFRAMESET_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/action.h>
#include <synfig/keyframe.h>
#include <synfig/waypoint.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Instance;

namespace Action {

class KeyframeWaypointSet :
	public Super
{
private:

	synfig::Keyframe keyframe;
	synfig::Waypoint::Model waypoint_model;


	void process_value_desc(const synfigapp::ValueDesc& value_desc);

public:

	KeyframeWaypointSet();

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
