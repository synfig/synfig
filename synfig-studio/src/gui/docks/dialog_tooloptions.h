/* === S Y N F I G ========================================================= */
/*!	\file dialog_tooloptions.h
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

#ifndef __SYNFIG_STUDIO_DIALOG_TOOLOPTIONS_H
#define __SYNFIG_STUDIO_DIALOG_TOOLOPTIONS_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include "docks/dockable.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dialog_ToolOptions : public Dockable
{
	Gtk::Label empty_label;
	Gtk::VBox sub_vbox_;
	 Gtk::Widget* primary_focus_widget_;

public:

	void clear();
	void set_widget(Gtk::Widget&);
	void focus_first_editable();
	Dialog_ToolOptions();
	~Dialog_ToolOptions();
	    void set_primary_focus_widget(Gtk::Widget* widget);
    

    void focus_primary_widget();

}; // END of Dialog_ToolOptions

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
