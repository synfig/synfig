/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_distance.h
**	\brief Template Header
**
**	$Id$
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

#ifndef __SYNFIG_STUDIO_WIDGET_DISTANCE_H
#define __SYNFIG_STUDIO_WIDGET_DISTANCE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>
#include <synfig/distance.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Distance : public Gtk::SpinButton
{
	mutable synfig::Distance distance_;

	Glib::RefPtr<Gtk::Adjustment> adjustment;

protected:

	int	on_input(double* new_value);
	bool on_output();

	bool on_key_press_event(GdkEventKey* event);
	bool on_key_release_event(GdkEventKey* event);

public:
	void set_value(const synfig::Distance &data);
	synfig::Distance get_value()const;
	Widget_Distance();
	~Widget_Distance();
}; // END of class Widget_Distance

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
