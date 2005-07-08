/* === S Y N F I G ========================================================= */
/*!	\file keyframeactionmanager.h
**	\brief Template Header
**
**	$Id: keyframeactionmanager.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_KEYFRAME_ACTION_MANAGER_H
#define __SYNFIG_KEYFRAME_ACTION_MANAGER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/uimanager.h>
#include <gtkmm/treeview.h>
#include <synfigapp/canvasinterface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class KeyframeTree;

class KeyframeActionManager
{
	sigc::signal<void> signal_show_keyframe_properties_;
		
	Glib::RefPtr<Gtk::UIManager> ui_manager_;
	//Glib::RefPtr<Gtk::TreeSelection> tree_selection_;
	KeyframeTree* keyframe_tree_;
	etl::handle<synfigapp::CanvasInterface> canvas_interface_;

	Glib::RefPtr<Gtk::ActionGroup>	action_group_;
	Gtk::UIManager::ui_merge_id 	popup_id_;
	
	
	sigc::connection selection_changed_connection;

	bool queued;
	sigc::connection queue_refresh_connection;
	sigc::connection time_changed_connection;
	
	void on_add_keyframe();
	void on_keyframe_properties();
	
public:
	sigc::signal<void>& signal_show_keyframe_properties() { return signal_show_keyframe_properties_; }

	void queue_refresh();

	KeyframeActionManager();
	~KeyframeActionManager();

	void set_ui_manager(const Glib::RefPtr<Gtk::UIManager> &x);
	Glib::RefPtr<Gtk::UIManager> get_ui_manager()const { return ui_manager_; }

	void set_keyframe_tree(KeyframeTree* x);
	KeyframeTree* get_keyframe_tree()const { return keyframe_tree_; }

	void set_canvas_interface(const etl::handle<synfigapp::CanvasInterface> &x);
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const { return canvas_interface_; }

	void refresh();
	void clear();
}; // END of KeyframeActionManager

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
