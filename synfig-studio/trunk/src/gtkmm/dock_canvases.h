/* === S I N F G =========================================================== */
/*!	\file dialog_canvases.h
**	\brief Template Header
**
**	$Id: dock_canvases.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_STUDIO_DIALOG_CANVASES_H
#define __SINFG_STUDIO_DIALOG_CANVASES_H

/* === H E A D E R S ======================================================= */

#include "dockable.h"
#include <gtkmm/treeview.h>
#include "instance.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dock_Canvases : public Dockable
{	
	Gtk::TreeView *canvas_tree;
	//Gtk::Menu	menu;
	etl::loose_handle<studio::Instance>	selected_instance;

private:
	
	void set_selected_instance_(etl::handle<studio::Instance> x);

	etl::loose_handle<studio::Instance> get_selected_instance() { return selected_instance; }

	etl::loose_handle<sinfg::Canvas> get_selected_canvas();

	etl::loose_handle<studio::CanvasView> get_selected_canvas_view();

	void set_selected_instance(etl::loose_handle<studio::Instance> x);

	void set_selected_instance_signal(etl::handle<studio::Instance> x);

	void new_instance(etl::handle<studio::Instance> x);

	void delete_instance(etl::handle<studio::Instance> x);

	void refresh_instances();

	bool close();
	
	void on_row_activate(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *);
	//bool on_tree_event(GdkEvent *event);



	void on_action_toggle(const Glib::ustring& path);
	Gtk::Widget* create_canvas_tree();

public:

	Dock_Canvases();
	~Dock_Canvases();
}; // END of Dock_Canvases

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
