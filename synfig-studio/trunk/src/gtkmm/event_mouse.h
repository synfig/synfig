/* === S I N F G =========================================================== */
/*!	\file event_mouse.h
**	\brief Template Header
**
**	$Id: event_mouse.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_EVENT_MOUSE_H
#define __SINFG_EVENT_MOUSE_H

/* === H E A D E R S ======================================================= */

#include <sinfg/vector.h>
#include "smach.h"
#include <gdkmm/types.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

enum MouseButton
{
	BUTTON_NONE,
	BUTTON_LEFT,
	BUTTON_MIDDLE,
	BUTTON_RIGHT,
	BUTTON_UP,
	BUTTON_DOWN,
	
	BUTTON_END
};

struct EventMouse : public Smach::event
{
	sinfg::Point pos;
	MouseButton button;
	float pressure;
	Gdk::ModifierType modifier;
	
	EventMouse(EventKey id, MouseButton button, const sinfg::Point& pos, Gdk::ModifierType modifier=Gdk::ModifierType(0)):
		Smach::event(id),
		pos(pos),
		button(button),
		pressure(button==BUTTON_NONE?0.0f:1.0f),
		modifier(modifier)
	{ }

	EventMouse(EventKey id, MouseButton button, const sinfg::Point& pos, float pressure, Gdk::ModifierType modifier=Gdk::ModifierType(0)):
		Smach::event(id),
		pos(pos),
		button(button),
		pressure(pressure),
		modifier(modifier)
	{ }
}; // END of EventMouse

struct EventBox : public Smach::event
{
	sinfg::Point p1,p2;
	MouseButton button;
	Gdk::ModifierType modifier;
	
	EventBox(EventKey id, const sinfg::Point& p1,const sinfg::Point& p2,MouseButton button=BUTTON_NONE, Gdk::ModifierType modifier=Gdk::ModifierType(0)):
		Smach::event(id),
		p1(p1),
		p2(p2),
		button(button),
		modifier(modifier)
	{ }

	EventBox(const sinfg::Point& p1,const sinfg::Point& p2,MouseButton button=BUTTON_NONE, Gdk::ModifierType modifier=Gdk::ModifierType(0)):
		Smach::event(EVENT_WORKAREA_BOX),
		p1(p1),
		p2(p2),
		button(button),
		modifier(modifier)
	{ }
}; // END of EventBox


}; // END of namespace studio

/* === E N D =============================================================== */

#endif
