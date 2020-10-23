/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_enum.h
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

#ifndef __SYNFIG_STUDIO_WIDGET_ENUM_H
#define __SYNFIG_STUDIO_WIDGET_ENUM_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <synfig/paramdesc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class Widget_Enum : public Gtk::ComboBox
{
	synfig::ParamDesc param_desc;
	int value;
protected:
class Model : public Gtk::TreeModel::ColumnRecord
	{
		public:

		Model()
		{ add(icon); add(value); add(local_name); }

		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<int> value;
		Gtk::TreeModelColumn<Glib::ustring> local_name;
	};
	Model enum_model;
	Glib::RefPtr<Gtk::ListStore> enum_TreeModel;

public:

	Widget_Enum();
	~Widget_Enum();

	void set_param_desc(const synfig::ParamDesc &x);
	void set_icon(Gtk::TreeNodeChildren::size_type index,const Glib::RefPtr<Gdk::Pixbuf> &icon);
	void refresh();

	void set_value(int data);
	int get_value() const;
	virtual void on_changed();
}; // END of class Widget_Enum
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
