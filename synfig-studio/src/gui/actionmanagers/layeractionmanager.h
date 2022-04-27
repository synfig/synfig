/* === S Y N F I G ========================================================= */
/*!	\file layeractionmanager.h
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

#ifndef __SYNFIG_LAYER_ACTION_MANAGER_H
#define __SYNFIG_LAYER_ACTION_MANAGER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/uimanager.h>
#include <giomm/simpleaction.h>
#include <giomm/menu.h>
#include <synfigapp/canvasinterface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class LayerTree;

class LayerActionManager
{
	Glib::RefPtr<Gtk::UIManager> ui_manager_;
	LayerTree* layer_tree_;
	etl::handle<synfigapp::CanvasInterface> canvas_interface_;

	Glib::RefPtr<Gtk::ActionGroup>	action_group_;
	Gtk::UIManager::ui_merge_id 	menu_popup_id_;
	Gtk::UIManager::ui_merge_id 	menu_main_id_;


	Glib::RefPtr<Gtk::ActionGroup> action_group_copy_paste;

	Glib::RefPtr<Gtk::Action>	action_cut_;
	Glib::RefPtr<Gtk::Action>	action_copy_;
	Glib::RefPtr<Gtk::Action>	action_paste_;

	Glib::RefPtr<Gio::SimpleAction> simp_action_cut_;
	Glib::RefPtr<Gio::SimpleAction> simp_action_copy_;
	Glib::RefPtr<Gio::SimpleAction> simp_action_paste_;

	Glib::RefPtr<Gtk::Action>	action_amount_inc_;
	Glib::RefPtr<Gtk::Action>	action_amount_dec_;

	Glib::RefPtr<Gio::SimpleAction> simp_action_amount_inc_;
	Glib::RefPtr<Gio::SimpleAction> simp_action_amount_dec_;

	Glib::RefPtr<Gtk::Action>	action_select_all_child_layers_;
	sigc::connection			select_all_child_layers_connection;

	std::list<synfig::Layer::Handle> clipboard_;

	sigc::connection selection_changed_connection;

	bool queued;
	sigc::connection queue_refresh_connection;

	std::list<sigc::connection> update_connection_list;

	void cut();
	void copy();
	void paste();
	void export_dup_nodes(synfig::Layer::Handle, synfig::Canvas::Handle, int &);

	void amount_inc();
	void amount_dec();
	void set_action_inc_dec_menu(Glib::RefPtr<Gio::Menu>& menu, const char* name_inc, const char* name_dec, bool isActive);

public:
	void queue_refresh();

	LayerActionManager();
	~LayerActionManager();

	void set_ui_manager(const Glib::RefPtr<Gtk::UIManager> &x);
	Glib::RefPtr<Gtk::UIManager> get_ui_manager()const { return ui_manager_; }

	void set_layer_tree(LayerTree* x);
	LayerTree* get_layer_tree()const { return layer_tree_; }

	void set_canvas_interface(const etl::handle<synfigapp::CanvasInterface> &x);
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const { return canvas_interface_; }

	etl::loose_handle<synfigapp::Instance> get_instance()const { return canvas_interface_->get_instance(); }

	void refresh();
	void clear();

	Glib::RefPtr<Gtk::Action> get_action_select_all_child_layers() { return action_select_all_child_layers_; }
}; // END of LayerActionManager

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
