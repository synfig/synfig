/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_link.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2014 Jérôme Blanchi
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

#ifndef __SYNFIG_STUDIO_WIDGET_LINK_H
#define __SYNFIG_STUDIO_WIDGET_LINK_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/togglebutton.h>
#include "synfig/string.h"

#include "general.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Link: public Gtk::ToggleButton
{
	synfig::String tooltip_inactive_;
	synfig::String tooltip_active_;

	Gtk::Image *icon_off_;
	Gtk::Image *icon_on_;

protected:
	void on_toggled();

public:
	Widget_Link(const std::string &tlt_inactive = _("Link Values"), const std::string &tlt_active = _("Unlink Values"));
	~Widget_Link();
}; // END of class Widget_Link

} // END of namespace studio

/* === E N D =============================================================== */

#endif /* WIDGET_LINK_H_ */
