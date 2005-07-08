/* === S Y N F I G ========================================================= */
/*!	\file canvasproperties.h
**	\brief Template Header
**
**	$Id: canvasproperties.h,v 1.1.1.1 2005/01/07 03:34:35 darco Exp $
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

#ifndef __SYNFIG_GTKMM_CANVASPROPERTIES_H
#define __SYNFIG_GTKMM_CANVASPROPERTIES_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>

#include <gtkmm/dialog.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/table.h>
#include <gtkmm/entry.h>
#include <gtkmm/tooltips.h>

#include "renddesc.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class TreeView; };
namespace synfigapp { class CanvasInterface; };

namespace studio
{	
class CanvasProperties  :  public Gtk::Dialog
{
	Gtk::Tooltips tooltips;

	etl::handle<synfigapp::CanvasInterface> canvas_interface_;
	Widget_RendDesc widget_rend_desc;
	Gtk::Entry entry_id;
	Gtk::Entry entry_name;
	Gtk::Entry entry_description;

	bool dirty_rend_desc;
	
	Gtk::TreeView* meta_data_tree_view;
	void on_button_meta_data_add();
	void on_button_meta_data_delete();
	
public:
	CanvasProperties(Gtk::Window& parent,etl::handle<synfigapp::CanvasInterface> canvas_interface);
	~CanvasProperties();

	void refresh();
	void update_title();
private:
	void on_rend_desc_changed();

	Gtk::Widget& create_meta_data_view();

	void on_ok_pressed();
	void on_apply_pressed();
	void on_cancel_pressed();
}; // END of class CanvasProperties

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
