/* === S Y N F I G ========================================================= */
/*!	\file template.h
**	\brief Template Header
**
**	$Id: statemanager.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_STATEMANAGER_H
#define __SYNFIG_STATEMANAGER_H

/* === H E A D E R S ======================================================= */

#include <glibmm/refptr.h>
#include <vector>
#include "smach.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class ActionGroup; }

typedef unsigned int guint;

namespace studio {
	class StateManager
{
private:
	Glib::RefPtr<Gtk::ActionGroup> state_group;
	
	guint merge_id;
	std::vector<guint> merge_id_list;

	void change_state_(const Smach::state_base *state);

public:
	StateManager();

	~StateManager();
	
	void add_state(const Smach::state_base *state);

	Glib::RefPtr<Gtk::ActionGroup> get_action_group();
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
