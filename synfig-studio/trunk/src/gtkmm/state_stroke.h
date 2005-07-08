/* === S Y N F I G ========================================================= */
/*!	\file state_stroke.h
**	\brief Template Header
**
**	$Id: state_stroke.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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
