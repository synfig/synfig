/* === S Y N F I G ========================================================= */
/*!	\file iconcontroller.h
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

#ifndef __SYNFIG_STUDIO_ICONCONTROLLER_H
#define __SYNFIG_STUDIO_ICONCONTROLLER_H

/* === H E A D E R S ======================================================= */

#include <gdkmm/cursor.h>
#include <gtkmm/iconfactory.h>

#include <synfig/interpolation.h>
#include <synfig/type.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig { class ValueNode; class Layer; }

namespace synfigapp { namespace Action { struct BookEntry; };};

namespace studio {


class IconController
{
	Glib::RefPtr<Gtk::IconFactory> icon_factory;
	void init_icon(const synfig::String &name, const synfig::String &iconfile, const synfig::String& desc);

public:
	IconController();
	~IconController();

	void init_icons(const synfig::String& path_to_icons);
	static Glib::RefPtr<Gdk::Cursor> get_normal_cursor();
	static Glib::RefPtr<Gdk::Cursor> get_tool_cursor(const Glib::ustring& name,const Glib::RefPtr<Gdk::Window>& window);
};

std::string layer_icon_name(const synfig::String &layer);
Glib::RefPtr<Gdk::Pixbuf> get_tree_pixbuf_layer(const synfig::String &layer);

std::string value_icon_name(synfig::Type &type);
std::string interpolation_icon_name(synfig::Interpolation type);
std::string valuenode_icon_name(etl::handle<synfig::ValueNode> value_node);
Glib::RefPtr<Gdk::Pixbuf> get_tree_pixbuf(synfig::Type &type);
Gtk::StockID get_action_stock_id(const synfigapp::Action::BookEntry& action);

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
