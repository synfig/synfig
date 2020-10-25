/* === S Y N F I G ========================================================= */
/*!	\file dialogs/canvasproperties.h
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

#ifndef __SYNFIG_GTKMM_CANVASPROPERTIES_H
#define __SYNFIG_GTKMM_CANVASPROPERTIES_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>

#include <gui/renddesc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class TreeView; }
namespace synfigapp { class CanvasInterface; }

namespace studio
{

class CanvasProperties  :  public Gtk::Dialog
{
	etl::handle<synfigapp::CanvasInterface> canvas_interface_;
	Widget_RendDesc widget_rend_desc;
	Gtk::Entry entry_id;
	Gtk::Entry entry_name;
	Gtk::Entry entry_description;

	bool dirty_rend_desc;

	//Gtk::TreeView* meta_data_tree_view;
	//void on_button_meta_data_add();
	//void on_button_meta_data_delete();

public:
	CanvasProperties(Gtk::Window& parent,etl::handle<synfigapp::CanvasInterface> canvas_interface);
	~CanvasProperties();

	void refresh();
	void update_title();
private:
	void on_rend_desc_changed();

	//Gtk::Widget& create_meta_data_view();

	void on_ok_pressed();
	void on_apply_pressed();
	void on_cancel_pressed();
}; // END of class CanvasProperties

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
