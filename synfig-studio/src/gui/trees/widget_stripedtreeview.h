/* === S Y N F I G ========================================================= */
/*!	\file widget_stripedtreeview.h
**	\brief Base class for TreeView widget with striped rows
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2023 Synfig contributors
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

#ifndef SYNFIG_STUDIO_WIDGETSTRIPEDTREEVIEW_H
#define SYNFIG_STUDIO_WIDGETSTRIPEDTREEVIEW_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treeview.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

/**
 * A TreeView that allows CSS to alternate row colors.
 *
 * This ability is broken in native Gtk.
 * https://gitlab.gnome.org/GNOME/gtk/-/issues/581
 *
 * Css classes "even" and "odd" are used to define the background colors. *
 */
class Widget_StripedTreeView : public Gtk::TreeView
{
public:
	Widget_StripedTreeView();

protected:
	bool on_draw(const ::Cairo::RefPtr<::Cairo::Context>& cr) override;
};

}

#endif // SYNFIG_STUDIO_WIDGETSTRIPEDTREEVIEW_H
