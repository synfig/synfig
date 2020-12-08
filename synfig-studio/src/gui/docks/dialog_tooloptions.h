/* === S Y N F I G ========================================================= */
/*!	\file dialog_tooloptions.h
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

public:

	void clear();
	void set_widget(Gtk::Widget&);
	void set_name(const synfig::String& name);

	Dialog_ToolOptions();
	~Dialog_ToolOptions();
}; // END of Dialog_ToolOptions

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
