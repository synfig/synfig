/* === S Y N F I G ========================================================= */
/*!	\file event_keyboard.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#ifndef __SYNFIG_EVENT_KEYBOARD_H
#define __SYNFIG_EVENT_KEYBOARD_H

/* === H E A D E R S ======================================================= */

#include <gdkmm/types.h>
#include <gui/smach.h>
#include <synfig/vector.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

struct EventKeyboard : public Smach::event
{
	guint keyval;
	Gdk::ModifierType modifier;

	EventKeyboard(EventKey id, guint keyval, Gdk::ModifierType modifier = Gdk::ModifierType(0)):
		Smach::event(id),
		keyval(keyval),
		modifier(modifier)
	{ }
}; // END of EventKeyboard

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
