/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_sublayer.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2015 Max May
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

#ifndef __SYNFIG_STUDIO_WIDGET_SUBLAYER_H
#define __SYNFIG_STUDIO_WIDGET_SUBLAYER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <string>
#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class Widget_Sublayer : public Gtk::ComboBox
{
	synfigapp::ValueDesc value_desc;
	std::string value;
protected:
class Model : public Gtk::TreeModel::ColumnRecord
	{
		public:

		Model()
		{ add(value); add(name); }

		Gtk::TreeModelColumn<std::string> value;
		Gtk::TreeModelColumn<Glib::ustring> name;
	};
	Model enum_model;
	Glib::RefPtr<Gtk::ListStore> enum_TreeModel;

public:

	Widget_Sublayer();
	~Widget_Sublayer();

	void set_value_desc(const synfigapp::ValueDesc &x);
	void refresh();

	void set_value(std::string data);
	std::string get_value() const;
	virtual void on_changed();
}; // END of class Widget_Sublayer
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
