/* === S I N F G =========================================================== */
/*!	\file template.h
**	\brief Template Header
**
**	$Id: groupactionmanager.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_GROUP_ACTION_MANAGER_H
#define __SINFG_GROUP_ACTION_MANAGER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/uimanager.h>
#include <gtkmm/treeview.h>
#include <sinfgapp/canvasinterface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class LayerGroupTree;

class GroupActionManager
{
	Glib::RefPtr<Gtk::UIManager> ui_manager_;
	LayerGroupTree* group_tree_;
	etl::handle<sinfgapp::CanvasInterface> canvas_interface_;

	Glib::RefPtr<Gtk::ActionGroup>	action_group_;
	Gtk::UIManager::ui_merge_id 	popup_id_;
	
	sigc::connection selection_changed_connection;

	bool queued;
	sigc::connection queue_refresh_connection;

private:
	
	void on_action_add();

public:
	void queue_refresh();

	GroupActionManager();
	~GroupActionManager();

	void set_ui_manager(const Glib::RefPtr<Gtk::UIManager> &x);
	Glib::RefPtr<Gtk::UIManager> get_ui_manager()const { return ui_manager_; }

	void set_group_tree(LayerGroupTree* x);
	LayerGroupTree* get_group_tree()const { return group_tree_; }

	void set_canvas_interface(const etl::handle<sinfgapp::CanvasInterface> &x);
	etl::handle<sinfgapp::CanvasInterface> get_canvas_interface()const { return canvas_interface_; }

	void refresh();
	void clear();
}; // END of GroupActionManager

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
