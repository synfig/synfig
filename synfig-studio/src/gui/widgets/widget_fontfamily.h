/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_fontfamily.h
**	\brief Widget to select font family
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2020 Rodolfo Ribeiro Gomes
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

#ifndef SYNFIG_STUDIO_WIDGET_FONTFAMILY_H
#define SYNFIG_STUDIO_WIDGET_FONTFAMILY_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/comboboxtext.h>
#include <gtkmm/liststore.h>
#include <string>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class Widget_FontFamily : public Gtk::ComboBoxText
{
	std::string value;

protected:
class Model : public Gtk::TreeModel::ColumnRecord
	{
		public:

		Model();

		Gtk::TreeModelColumn<std::string> value;
	};
	Model enum_model;
	static Glib::RefPtr<Gtk::ListStore> enum_TreeModel;

	void init_fontconfig();

public:

	Widget_FontFamily();
	~Widget_FontFamily();

	void set_value(const std::string& data);
	std::string get_value() const;

	sigc::signal<void>& signal_activate() { return signal_activate_; }
private:
	virtual void on_changed();

	sigc::signal<void> signal_activate_;
}; // END of class Widget_FontFamily
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
