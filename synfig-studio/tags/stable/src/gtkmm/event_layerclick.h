/* === S I N F G =========================================================== */
/*!	\file event_layerclick.h
**	\brief Template Header
**
**	$Id: event_layerclick.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_EVENT_LAYERCLICK_H
#define __SINFG_EVENT_LAYERCLICK_H

/* === H E A D E R S ======================================================= */

#include <sinfg/vector.h>
#include "event_mouse.h"
#include <sinfg/layer.h>
#include "smach.h"
#include <gdkmm/types.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
	
struct EventLayerClick : public Smach::event
{
	sinfg::Point pos;
	MouseButton button;
	etl::loose_handle<sinfg::Layer> layer;
	Gdk::ModifierType modifier;
	
	EventLayerClick(etl::loose_handle<sinfg::Layer> layer, MouseButton button, const sinfg::Point& pos, Gdk::ModifierType modifier=Gdk::ModifierType(0)):
		Smach::event(EVENT_WORKAREA_LAYER_CLICKED),
		pos(pos),
		button(button),
		layer(layer),
		modifier(modifier)
	{ }
}; // END of EventLayerClick

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
