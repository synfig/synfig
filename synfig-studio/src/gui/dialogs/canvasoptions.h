/* === S Y N F I G ========================================================= */
/*!	\file dialogs/canvasoptions.h
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

#ifndef __SYNFIG_GTKMM_CANVASOPTIONS_H
#define __SYNFIG_GTKMM_CANVASOPTIONS_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>

#include "ETL/handle"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk {
	class CheckButton;
};

namespace studio
{

class CanvasView;
class Widget_Vector;

class CanvasOptions  :  public Gtk::Dialog
{
	etl::loose_handle<CanvasView> canvas_view_;

	Gtk::CheckButton * toggle_grid_snap;
	Gtk::CheckButton * toggle_grid_show;

	Widget_Vector * vector_grid_size;

	Gtk::CheckButton * toggle_time_snap;

	const Glib::RefPtr<Gtk::Builder>& builder;
	void set_canvas_view(CanvasView* canvas_view);
public:
	CanvasOptions(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	static CanvasOptions * create(Gtk::Window& parent, CanvasView * canvas_view);
	~CanvasOptions();

	void refresh();
	void update_title();
private:

	void on_grid_snap_toggle();
	void on_grid_show_toggle();

	void on_ok_pressed();
	void on_apply_pressed();
	void on_cancel_pressed();
}; // END of class CanvasOptions

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
