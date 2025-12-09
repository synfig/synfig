/* === S Y N F I G ========================================================= */
/*!	\file modules/mod_palette/dock_paledit.h
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

#ifndef __SYNFIG_STUDIO_DOCK_PAL_EDIT_H
#define __SYNFIG_STUDIO_DOCK_PAL_EDIT_H

/* === H E A D E R S ======================================================= */

#include <giomm/simpleactiongroup.h>
#include <gtkmm/table.h>

#include <gui/docks/dockable.h>

#include <synfig/color.h>
#include <synfig/palette.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {
class CanvasInterface;
};

namespace studio {

class PaletteSettings;

class Dock_PalEdit : public Dockable
{
	friend class PaletteSettings;

	Glib::RefPtr<Gio::SimpleActionGroup> action_group_;

	synfig::Palette palette_;

	Gtk::Table table;

	void on_add_pressed();

	void on_save_pressed();

	void on_open_pressed();

	void show_menu(int i);

	sigc::signal<void> signal_changed_;


private:
	int add_color(const synfig::Color& x);
	bool check_hex_format(const std::string& hexcolor);
	void add_from_clipboard();
	void copy_color(int i);
	void set_color(synfig::Color x, int i);
	void erase_color(int i);

	void select_fill_color(int i);
	void select_outline_color(int i);
	synfig::Color get_color(int i)const;
	void edit_color(int i);

public:
	void set_palette(const synfig::Palette& x);
	const synfig::Palette& get_palette()const { return palette_; }

	int size()const;

	void set_default_palette();

	void refresh();

	const sigc::signal<void>& signal_changed() { return signal_changed_; }

	Dock_PalEdit();
	~Dock_PalEdit();
}; // END of Dock_PalEdit

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
