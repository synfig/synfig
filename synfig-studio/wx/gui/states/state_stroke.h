/* === S Y N F I G ========================================================= */
/*!	\file state_stroke.h
**	\brief Template Header
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

#ifndef __SYNFIG_STUDIO_STATE_STROKE_H
#define __SYNFIG_STUDIO_STATE_STROKE_H

/* === H E A D E R S ======================================================= */

#include "canvasview.h"
#include "workarea.h"
#include <sigc++/object.h>
#include "duckmatic.h"
#include <synfig/blinepoint.h>
#include <list>
#include <ETL/smart_ptr>
#include "eventkey.h"
#include <gdkmm/types.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class StateStroke_Context;

class StateStroke : public Smach::state<StateStroke_Context>
{
public:
	StateStroke();
	~StateStroke();
}; // END of class StateStroke

extern StateStroke state_stroke;

struct EventStroke : public Smach::event
{
	etl::smart_ptr<std::list<synfig::Point> > stroke_data;
	etl::smart_ptr<std::list<synfig::Real> > width_data;
	Gdk::ModifierType modifier;

	EventStroke(etl::smart_ptr<std::list<synfig::Point> > stroke_data,
			etl::smart_ptr<std::list<synfig::Real> > width_data,
			Gdk::ModifierType modifier=Gdk::ModifierType(0)
	):
		Smach::event(EVENT_WORKAREA_STROKE),
		stroke_data(stroke_data),
		width_data(width_data),
		modifier(modifier)
	{ }
}; // END of EventStroke

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
