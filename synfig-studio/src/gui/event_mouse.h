/* === S Y N F I G ========================================================= */
/*!	\file event_mouse.h
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

#ifndef __SYNFIG_EVENT_MOUSE_H
#define __SYNFIG_EVENT_MOUSE_H

/* === H E A D E R S ======================================================= */

#include <gdkmm/types.h>
#include <gui/duck.h>
#include <gui/smach.h>

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
	synfig::Point pos;
	MouseButton button;
	float pressure;
	Gdk::ModifierType modifier;
	etl::handle<Duck> duck;

	EventMouse(EventKey id, MouseButton button, const synfig::Point& pos, Gdk::ModifierType modifier=Gdk::ModifierType(0), etl::handle<Duck> duck = etl::handle<Duck>()):
		Smach::event(id),
		pos(pos),
		button(button),
		pressure(button==BUTTON_NONE?0.0f:1.0f),
		modifier(modifier),
		duck(duck)
	{ }

	EventMouse(EventKey id, MouseButton button, const synfig::Point& pos, float pressure, Gdk::ModifierType modifier=Gdk::ModifierType(0), etl::handle<Duck> duck = etl::handle<Duck>()):
		Smach::event(id),
		pos(pos),
		button(button),
		pressure(pressure),
		modifier(modifier),
		duck(duck)
	{ }
}; // END of EventMouse

struct EventBox : public Smach::event
{
	synfig::Point p1,p2;
	MouseButton button;
	Gdk::ModifierType modifier;

	EventBox(EventKey id, const synfig::Point& p1,const synfig::Point& p2,MouseButton button=BUTTON_NONE, Gdk::ModifierType modifier=Gdk::ModifierType(0)):
		Smach::event(id),
		p1(p1),
		p2(p2),
		button(button),
		modifier(modifier)
	{ }

	EventBox(const synfig::Point& p1,const synfig::Point& p2,MouseButton button=BUTTON_NONE, Gdk::ModifierType modifier=Gdk::ModifierType(0)):
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
