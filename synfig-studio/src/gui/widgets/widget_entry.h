/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_entry.h
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

#ifndef __SYNFIG_STUDIO_WIDGET_ENTRY_H
#define __SYNFIG_STUDIO_WIDGET_ENTRY_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/entry.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Entry : public Gtk::Entry
{
	bool first_selection = false;

protected:

	void after_event(GdkEvent *event);
	bool on_event(GdkEvent* event);

public:

	Widget_Entry();
	~Widget_Entry();
}; // END of class Widget_Entry

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
