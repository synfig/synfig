/* === S I N F G =========================================================== */
/*!	\file layertreestore.h
**	\brief Template Header
**
**	$Id: layergrouptreestore.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_LAYERGROUPTREESTORE_H
#define __SINFG_STUDIO_LAYERGROUPTREESTORE_H

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

class LayerGroupTreeStore :  public Gtk::TreeStore
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:
	typedef std::list<sinfg::Layer::Handle> LayerList;

	class Model : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<Glib::ustring> label;
		Gtk::TreeModelColumn<Glib::ustring> tooltip;
		
		Gtk::TreeModelColumn<Glib::ustring> group_name;
		Gtk::TreeModelColumn<Glib::ustring> parent_group_name;

		Gtk::TreeModelColumn<sinfg::Canvas::Handle> canvas;

		Gtk::TreeModelColumn<bool>						active;
		Gtk::TreeModelColumn<bool>						is_layer;
		Gtk::TreeModelColumn<bool>						is_group;
		Gtk::TreeModelColumn<sinfg::Layer::Handle>		layer;
		
		Gtk::TreeModelColumn<LayerList>		all_layers;
		Gtk::TreeModelColumn<LayerList>		child_layers;
		
		Model()
		{
			add(icon);
			add(label);
			add(group_name);
			add(parent_group_name);
			add(canvas);
			add(tooltip);
			add(active);
			add(layer);
			add(is_layer);
			add(is_group);
			add(all_layers);
			add(child_layers);
		}
	};

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:
	
	//! TreeModel for the layers
	const Model model;
	
	bool rebuilding;

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_;

	Glib::RefPtr<Gdk::Pixbuf> layer_icon;
	Glib::RefPtr<Gdk::Pixbuf> group_icon;

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


	void on_group_pair_added(sinfg::String group, etl::handle<sinfg::Layer> layer);
	void on_group_pair_removed(sinfg::String group, etl::handle<sinfg::Layer> layer);

	void on_activity();

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	bool on_layer_tree_event(GdkEvent *event);

	void on_layer_new_description(sinfg::Layer::Handle handle,sinfg::String desc);

	void on_layer_status_changed(sinfg::Layer::Handle handle,bool);

	bool find_layer_row_(const sinfg::Layer::Handle &handle, sinfg::Canvas::Handle canvas, Gtk::TreeModel::Children layers, Gtk::TreeModel::Children::iterator &iter, Gtk::TreeModel::Children::iterator &prev);

	bool find_group_row_(const sinfg::String &group, Gtk::TreeModel::Children layers, Gtk::TreeModel::Children::iterator &iter, Gtk::TreeModel::Children::iterator &prev);

	bool on_group_removed(sinfg::String group);
	bool on_group_changed(sinfg::String group);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	
	LayerGroupTreeStore(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_);
	~LayerGroupTreeStore();

	Gtk::TreeRow on_group_added(sinfg::String group);
	etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface() { return canvas_interface_; }
	etl::loose_handle<const sinfgapp::CanvasInterface> canvas_interface()const { return canvas_interface_; }
	etl::loose_handle<sinfgapp::CanvasInterface> get_canvas_interface()const { return canvas_interface_; }

	bool find_layer_row(const sinfg::Layer::Handle &handle, Gtk::TreeModel::Children::iterator &iter);

	bool find_group_row(const sinfg::String &group, Gtk::TreeModel::Children::iterator &iter);

	bool find_prev_layer_row(const sinfg::Layer::Handle &handle, Gtk::TreeModel::Children::iterator &iter);

	void rebuild();

	void refresh();

	void refresh_row(Gtk::TreeModel::Row &row);

	void set_row_layer(Gtk::TreeRow &row,sinfg::Layer::Handle &handle);

	static bool search_func(const Glib::RefPtr<TreeModel>&,int,const Glib::ustring&,const TreeModel::iterator&);

	/*
 -- ** -- S T A T I C   P U B L I C   M E T H O D S ---------------------------
	*/

public:
	
	static Glib::RefPtr<LayerGroupTreeStore> create(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_);

}; // END of class LayerGroupTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
