/* === S I N F G =========================================================== */
/*!	\file layertreestore.h
**	\brief Template Header
**
**	$Id: layertreestore.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_LAYERTREESTORE_H
#define __SINFG_STUDIO_LAYERTREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treestore.h>
#include <sinfgapp/canvasinterface.h>
#include <sinfg/value.h>
#include <sinfg/valuenode.h>
#include <gtkmm/treeview.h>

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

	class Model : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<Glib::ustring> label;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> id;

		Gtk::TreeModelColumn<sinfg::Canvas::Handle> canvas;

		Gtk::TreeModelColumn<Glib::ustring> tooltip;


		Gtk::TreeModelColumn<bool>						active;
		Gtk::TreeModelColumn<sinfg::Layer::Handle>		layer;
		Gtk::TreeModelColumn<sinfg::Canvas::Handle> 			contained_canvas;

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
			add(layer);
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

	etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_;

	Glib::RefPtr<Gdk::Pixbuf> layer_icon;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	/*
 -- ** -- P R O T E C T E D   M E T H O D S -----------------------------------
	*/

private:

	virtual void set_value_impl (const Gtk::TreeModel::iterator& row, int column, const Glib::ValueBase& value);
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

	void on_layer_new_description(sinfg::Layer::Handle handle,sinfg::String desc);

	void on_layer_added(sinfg::Layer::Handle handle);

	void on_layer_removed(sinfg::Layer::Handle handle);

	void on_layer_inserted(sinfg::Layer::Handle handle,int depth);

	void on_layer_moved(sinfg::Layer::Handle handle,int depth, sinfg::Canvas::Handle canvas);

	void on_layer_status_changed(sinfg::Layer::Handle handle,bool);

	void on_layer_lowered(sinfg::Layer::Handle handle);

	void on_layer_raised(sinfg::Layer::Handle handle);

	void on_layer_param_changed(sinfg::Layer::Handle handle,sinfg::String param_name);

	//void on_value_node_added(sinfg::ValueNode::Handle value_node);

	//void on_value_node_deleted(sinfg::ValueNode::Handle value_node);

	//void on_value_node_changed(sinfg::ValueNode::Handle value_node);

	//void on_value_node_replaced(sinfg::ValueNode::Handle replaced_value_node,sinfg::ValueNode::Handle new_value_node);

	bool find_layer_row_(const sinfg::Layer::Handle &handle, sinfg::Canvas::Handle canvas, Gtk::TreeModel::Children layers, Gtk::TreeModel::Children::iterator &iter, Gtk::TreeModel::Children::iterator &prev);

	bool find_canvas_row_(sinfg::Canvas::Handle canvas, sinfg::Canvas::Handle parent, Gtk::TreeModel::Children layers, Gtk::TreeModel::Children::iterator &iter);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	
	LayerTreeStore(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_);
	~LayerTreeStore();

	etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface() { return canvas_interface_; }
	etl::loose_handle<const sinfgapp::CanvasInterface> canvas_interface()const { return canvas_interface_; }
	etl::loose_handle<sinfgapp::CanvasInterface> get_canvas_interface()const { return canvas_interface_; }

	bool find_canvas_row(sinfg::Canvas::Handle canvas, Gtk::TreeModel::Children::iterator &iter);

	bool find_layer_row(const sinfg::Layer::Handle &handle, Gtk::TreeModel::Children::iterator &iter);

	bool find_prev_layer_row(const sinfg::Layer::Handle &handle, Gtk::TreeModel::Children::iterator &iter);

	void rebuild();

	void refresh();

	void refresh_row(Gtk::TreeModel::Row &row);

	void set_row_layer(Gtk::TreeRow &row,sinfg::Layer::Handle &handle);

	static int z_sorter(const Gtk::TreeModel::iterator &rhs,const Gtk::TreeModel::iterator &lhs);
	static int index_sorter(const Gtk::TreeModel::iterator &rhs,const Gtk::TreeModel::iterator &lhs);

	//void set_row_param(Gtk::TreeRow &row,sinfg::Layer::Handle &handle,const std::string& name, const std::string& local_name, const sinfg::ValueBase &value, etl::handle<sinfg::ValueNode> value_node,sinfg::ParamDesc *param_desc);

	//virtual void set_row(Gtk::TreeRow row,sinfgapp::ValueDesc value_desc);
	static bool search_func(const Glib::RefPtr<TreeModel>&,int,const Glib::ustring&,const TreeModel::iterator&);

	/*
 -- ** -- S T A T I C   P U B L I C   M E T H O D S ---------------------------
	*/

public:
	
	static Glib::RefPtr<LayerTreeStore> create(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_);


}; // END of class LayerTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
