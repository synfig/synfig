/* === S Y N F I G ========================================================= */
/*!	\file layertreestore.h
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

#ifndef __SYNFIG_STUDIO_LAYERTREESTORE_H
#define __SYNFIG_STUDIO_LAYERTREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treestore.h>
#include <synfigapp/canvasinterface.h>
#include <synfig/value.h>
#include <pangomm.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class LayerTreeStore : virtual public Gtk::TreeStore
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	enum RecordType
	{
		RECORD_TYPE_LAYER,
		RECORD_TYPE_GHOST
	};

	class Model : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<Glib::ustring> label;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> id;

		Gtk::TreeModelColumn<synfig::Canvas::Handle> canvas;

		Gtk::TreeModelColumn<Glib::ustring> tooltip;


		Gtk::TreeModelColumn<bool>						active;
		Gtk::TreeModelColumn<bool>						exclude_from_rendering;
		Gtk::TreeModelColumn<Pango::Style>				style;
		Gtk::TreeModelColumn<Pango::Weight>				weight;
		Gtk::TreeModelColumn<Pango::Underline>			underline;
		Gtk::TreeModelColumn<bool>						strikethrough;

		Gtk::TreeModelColumn<RecordType>				record_type;
		Gtk::TreeModelColumn<synfig::Layer::Handle>		layer;
		Gtk::TreeModelColumn<bool>			    		layer_impossible;
		Gtk::TreeModelColumn<Glib::ustring>			    ghost_label;
		Gtk::TreeModelColumn<synfig::Canvas::Handle> 	contained_canvas;

		Gtk::TreeModelColumn<bool>						children_lock;

		Gtk::TreeModelColumn<float> z_depth;
		Gtk::TreeModelColumn<int> index;

		Model()
		{
			add(icon);
			add(label);
			add(name);
			add(id);
			add(canvas);
			add(tooltip);
			add(active);
			add(exclude_from_rendering);
			add(style);
			add(weight);
			add(underline);
			add(strikethrough);
			add(record_type);
			add(layer);
			add(layer_impossible);
			add(ghost_label);
			add(contained_canvas);
			add(z_depth);
			add(index);
			add(children_lock);
		}
	};

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:

	//! TreeModel for the layers
	const Model model;

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	bool queued;

	sigc::connection queue_connection;

	std::map<synfig::Layer::Handle, sigc::connection> subcanvas_changed_connections;
	std::map<synfig::Layer::Handle, sigc::connection> switch_changed_connections;

	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_;

	Glib::RefPtr<Gdk::Pixbuf> layer_icon;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	/*
 -- ** -- P R O T E C T E D   M E T H O D S -----------------------------------
	*/

private:
	template<typename T>
	void set_gvalue_tpl(Glib::ValueBase& value, const T &v, bool use_assign_operator = false) const;

	virtual void  set_value_impl (const Gtk::TreeModel::iterator& row, int column, const Glib::ValueBase& value);
	virtual void  get_value_vfunc (const Gtk::TreeModel::iterator& iter, int column, Glib::ValueBase& value)const;

	virtual bool  row_draggable_vfunc (const TreeModel::Path& path)const;
	virtual bool  drag_data_get_vfunc (const TreeModel::Path& path, Gtk::SelectionData& selection_data)const;
	virtual bool  drag_data_delete_vfunc (const TreeModel::Path& path);
	virtual bool  drag_data_received_vfunc (const TreeModel::Path& dest, const Gtk::SelectionData& selection_data);
	virtual bool  row_drop_possible_vfunc (const TreeModel::Path& dest, const Gtk::SelectionData& selection_data)const;

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	bool on_layer_tree_event(GdkEvent *event);

	void on_layer_new_description(synfig::Layer::Handle handle,synfig::String desc);

	void on_layer_added(synfig::Layer::Handle handle);

	void on_layer_removed(synfig::Layer::Handle handle);

	void on_layer_inserted(synfig::Layer::Handle handle,int depth);

	void on_layer_moved(synfig::Layer::Handle handle,int depth, synfig::Canvas::Handle canvas);

	void on_layer_status_changed(synfig::Layer::Handle handle,bool);

	void on_layer_exclude_from_rendering_changed(synfig::Layer::Handle handle,bool);

	void on_layer_z_range_changed(synfig::Layer::Handle handle,bool);

	void on_layer_lowered(synfig::Layer::Handle handle);

	void on_layer_raised(synfig::Layer::Handle handle);

	void on_layer_param_changed(synfig::Layer::Handle handle,synfig::String param_name);

	//void on_value_node_added(synfig::ValueNode::Handle value_node);

	//void on_value_node_deleted(synfig::ValueNode::Handle value_node);

	//void on_value_node_changed(synfig::ValueNode::Handle value_node);

	//void on_value_node_replaced(synfig::ValueNode::Handle replaced_value_node,synfig::ValueNode::Handle new_value_node);

	bool find_layer_row_(const synfig::Layer::Handle &handle, synfig::Canvas::Handle canvas, Gtk::TreeModel::Children layers, Gtk::TreeModel::Children::iterator &iter, Gtk::TreeModel::Children::iterator &prev);

	bool find_canvas_row_(synfig::Canvas::Handle canvas, synfig::Canvas::Handle parent, Gtk::TreeModel::Children layers, Gtk::TreeModel::Children::iterator &iter);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	LayerTreeStore(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_);
	~LayerTreeStore();

	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface() { return canvas_interface_; }
	etl::loose_handle<const synfigapp::CanvasInterface> canvas_interface()const { return canvas_interface_; }
	etl::loose_handle<synfigapp::CanvasInterface> get_canvas_interface()const { return canvas_interface_; }

	bool find_canvas_row(synfig::Canvas::Handle canvas, Gtk::TreeModel::Children::iterator &iter);

	bool find_layer_row(const synfig::Layer::Handle &handle, Gtk::TreeModel::Children::iterator &iter);

	bool find_prev_layer_row(const synfig::Layer::Handle &handle, Gtk::TreeModel::Children::iterator &iter);

	void queue_rebuild();

	void rebuild();

	void refresh();

	void refresh_row(Gtk::TreeModel::Row &row);

	void set_row_layer(Gtk::TreeRow &row, const synfig::Layer::Handle &handle);
	void set_row_ghost(Gtk::TreeRow &row, const synfig::String &label, int depth);

	static int z_sorter(const Gtk::TreeModel::iterator &rhs,const Gtk::TreeModel::iterator &lhs);
	static int index_sorter(const Gtk::TreeModel::iterator &rhs,const Gtk::TreeModel::iterator &lhs);

	//void set_row_param(Gtk::TreeRow &row,synfig::Layer::Handle &handle,const std::string& name, const std::string& local_name, const synfig::ValueBase &value, etl::handle<synfig::ValueNode> value_node,synfig::ParamDesc *param_desc);

	//virtual void set_row(Gtk::TreeRow row,synfigapp::ValueDesc value_desc);
	static bool search_func(const Glib::RefPtr<TreeModel>&,int,const Glib::ustring&,const TreeModel::iterator&);

	/*
 -- ** -- S T A T I C   P U B L I C   M E T H O D S ---------------------------
	*/

public:

	static Glib::RefPtr<LayerTreeStore> create(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_);


}; // END of class LayerTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
