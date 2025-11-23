/* === S Y N F I G ========================================================= */
/*!	\file gui/instance.h
**	\brief writeme
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2012 Konstantin Dmitriev
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

#ifndef __SYNFIG_STUDIO_INSTANCE_H
#define __SYNFIG_STUDIO_INSTANCE_H

/* === H E A D E R S ======================================================= */

#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>

#include <ETL/handle>
#include <gtkmm/treestore.h>
#include <gui/trees/historytreestore.h>
#include <sigc++/sigc++.h>
#include <synfig/canvas.h>
#include <synfig/layers/layer_switch.h>
#include <synfigapp/instance.h>
#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */
#define DEFAULT_FILENAME_PREFIX _("Synfig Animation ") // will be followed by a different number for each document

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Menu; class ActionGroup; };

namespace studio {

class CanvasView;


class Instance : public synfigapp::Instance
{
public:
	typedef std::list< etl::handle<CanvasView> > CanvasViewList;

	enum Status
	{
		STATUS_OK,
		STATUS_ERROR,
		STATUS_CANCEL
	};

	class CanvasTreeModel : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Gtk::TreeModelColumn<Glib::ustring> icon_name;
		Gtk::TreeModelColumn<Glib::ustring> label;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> id;

		Gtk::TreeModelColumn<synfig::Canvas::Handle> canvas;
		Gtk::TreeModelColumn<bool> is_canvas;

		Gtk::TreeModelColumn<synfig::ValueNode::Handle> value_node;
		Gtk::TreeModelColumn<bool> is_value_node;
		Gtk::TreeModelColumn<synfig::ValueBase> value;
		Gtk::TreeModelColumn<Glib::ustring> type;
		Gtk::TreeModelColumn<int> link_id;
		Gtk::TreeModelColumn<int> link_count;

		Gtk::TreeModelColumn<bool> is_editable;

		Gtk::TreeModelColumn<synfigapp::ValueDesc> value_desc;

		CanvasTreeModel()
		{
			add(value);
			add(name);
			add(label);
			add(icon_name);
			add(type);
			add(id);
			add(canvas);
			add(value_node);
			add(is_canvas);
			add(is_value_node);

			add(is_editable);
			add(value_desc);
			add(link_count);
			add(link_id);
		}
	} canvas_tree_model;

private:

	sigc::signal<void,CanvasView*> signal_canvas_view_created_;
	sigc::signal<void,CanvasView*> signal_canvas_view_deleted_;

	sigc::signal<void> signal_undo_redo_status_changed_;

	//! Tree containing the canvases -- used for the "canvas browser"
	Glib::RefPtr<Gtk::TreeStore> canvas_tree_store_;

	//! Tree containing the actions -- used for the "canvas browser"
	Glib::RefPtr<HistoryTreeStore> history_tree_store_;

	//! Instance number
	int	id_;

	//! Used to calculate instance ID
	static int instance_count_;

	//! List of canvas view windows
	CanvasViewList canvas_view_list_;

	bool undo_status_;
	bool redo_status_;

	void set_undo_status(bool x);
	void set_redo_status(bool x);

protected:

	Instance(synfig::Canvas::Handle, synfig::FileSystem::Handle);

public:

	sigc::signal<void>& signal_undo_redo_status_changed() { return signal_undo_redo_status_changed_; }

	~Instance();

	sigc::signal<void,CanvasView*>& signal_canvas_view_created() { return signal_canvas_view_created_; }
	sigc::signal<void,CanvasView*>& signal_canvas_view_deleted() { return signal_canvas_view_deleted_; }

	synfig::Layer::Handle layer_inside_switch(synfig::Layer_Switch::Handle paste) const;

	bool get_undo_status()const { return undo_status_; }

	bool get_redo_status()const { return redo_status_; }

	int get_visible_canvases()const;

	Glib::RefPtr<Gtk::TreeStore> canvas_tree_store() { return canvas_tree_store_; }

	Glib::RefPtr<const Gtk::TreeStore> canvas_tree_store()const { return canvas_tree_store_; }

	Glib::RefPtr<HistoryTreeStore> history_tree_store() { return history_tree_store_; }

	Glib::RefPtr<const HistoryTreeStore> history_tree_store()const { return history_tree_store_; }

	//! Returns the number of instances that are currently open in the program
	static int get_count() { return instance_count_; }

	//Canvas::Handle get_canvas()const { return synfigapp::Instance::get_canvas(); }

	etl::handle<CanvasView>	find_canvas_view(synfig::Canvas::Handle canvas);

	//! Sets the focus to a specific canvas
	void focus(synfig::Canvas::Handle canvas);

	CanvasViewList & canvas_view_list() { return canvas_view_list_; }

	const CanvasViewList & canvas_view_list()const { return canvas_view_list_; }

	void run_plugin(std::string plugin_id, bool modify_canvas, std::vector<std::string> extra_args = {});

	bool save_as(const synfig::String &filename);

	//! returns true if the instance has a real filename associated with it, rather than the made up "synfig animation 1" or some such
	bool has_real_filename();

	//! Opens a "Save As" dialog, and then saves the composition to that file
	//! returns true if the save was successful
	bool dialog_save_as();

	bool dialog_export();

	void open();

	Status save();

	//! Closes the instance of this composition
	void close(bool remove_temporary_files = true);

	void revert();

	void update_all_titles();

	void refresh_canvas_tree();

	bool safe_revert();
	//! Perform necessary tests before close an instance
	//! returns true if it's safe to close the instance
	bool safe_close();

	void gather_uri(std::set<synfig::String> &x, const synfig::ValueNode::Handle &value_node) const;
	void gather_uri(std::set<synfig::String> &x, const synfig::Layer::Handle &layer) const;
	void gather_uri(std::set<synfig::String> &x, const synfig::Canvas::Handle &canvas) const;
	void gather_uri(std::set<synfig::String> &x, const synfigapp::SelectionManager::LayerList &layers) const;
	void gather_uri(std::map<synfig::String, synfig::String> &x, const synfigapp::SelectionManager::LayerList &layers) const;

	void add_actions_to_menu(Gtk::Menu *menu, const synfigapp::Action::ParamList &param_list, synfigapp::Action::Category category=synfigapp::Action::CATEGORY_ALL)const;
	void add_actions_to_menu(Gtk::Menu *menu, const synfigapp::Action::ParamList &param_list1,const synfigapp::Action::ParamList &param_list2, synfigapp::Action::Category category=synfigapp::Action::CATEGORY_ALL)const;

	void add_actions_to_group(const Glib::RefPtr<Gio::SimpleActionGroup>& action_group, const synfigapp::Action::ParamList& param_list, synfigapp::Action::Category category=synfigapp::Action::CATEGORY_ALL) const;
	void add_actions_to_group_and_menu(const Glib::RefPtr<Gio::SimpleActionGroup>& action_group, const std::string& action_group_name, const Glib::RefPtr<Gio::Menu>& menu, const synfigapp::Action::ParamList& param_list, synfigapp::Action::Category category = synfigapp::Action::CATEGORY_ALL) const;

	void add_special_layer_actions_to_menu(Gtk::Menu *menu, const synfig::Layer::Handle &layer) const;
	void add_special_layer_actions_to_menu(Gtk::Menu *menu, const synfigapp::SelectionManager::LayerList &layers) const;
	void add_special_layer_actions_to_group_and_menu(const Glib::RefPtr<Gio::SimpleActionGroup>& action_group, const std::string& action_group_name, const Glib::RefPtr<Gio::Menu>& menu, const synfigapp::SelectionManager::LayerList &layers) const;

	void process_action(synfig::String name, synfigapp::Action::ParamList param_list);

	void make_param_menu(Gtk::Menu *menu,synfig::Canvas::Handle canvas, synfigapp::ValueDesc value_desc, float location=0.5f, bool bezier=false);

	void make_param_menu(Gtk::Menu *menu,synfig::Canvas::Handle canvas,const std::list<synfigapp::ValueDesc>& value_desc_list, const synfigapp::ValueDesc &value_desc = synfigapp::ValueDesc());


	static void edit_waypoint(synfigapp::ValueDesc value_desc,synfig::Waypoint waypoint);

private:
	void insert_canvas(Gtk::TreeRow row,synfig::Canvas::Handle canvas);

public:
	static etl::handle<Instance> create(synfig::Canvas::Handle canvas, synfig::FileSystem::Handle container);
}; // END class Instance

}; // END namespace studio

/* === E N D =============================================================== */

#endif
