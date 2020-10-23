/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_color.h
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

#ifndef __SYNFIG_STUDIO_DIALOG_COLOR_H
#define __SYNFIG_STUDIO_DIALOG_COLOR_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>
#include <synfig/color.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_ColorEdit;

class Dialog_Color : public Gtk::Dialog
{
	Widget_ColorEdit* color_edit_widget;

	sigc::signal<void,synfig::Color> signal_edited_;

	void create_color_edit_widget();
	void create_set_color_button(const char *stock_id,
			const Glib::ustring& tip_text, int index,
			const sigc::slot0<void>& callback);
	void create_close_button();

	void on_color_changed();
	void on_set_oc_pressed();
	void on_set_fc_pressed();
	bool on_close_pressed();

public:
	Dialog_Color();
	~Dialog_Color();

	sigc::signal<void,synfig::Color>& signal_edited() { return signal_edited_; }

	void set_color(const synfig::Color& x);
	synfig::Color get_color() const;
	void reset();
}; // END of Dialog_Color

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
