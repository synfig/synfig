/* === S Y N F I G ========================================================= */
/*!	\file trees/layergrouptreestore.h
**	\brief Layer set tree model
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2017 caryoscelus
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

#ifndef __SYNFIG_STUDIO_LAYERGROUPTREESTORE_H
#define __SYNFIG_STUDIO_LAYERGROUPTREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treestore.h>
#include <synfigapp/canvasinterface.h>

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
	typedef std::list<synfig::Layer::Handle> LayerList;

	class Model : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<Glib::ustring> label;
		Gtk::TreeModelColumn<Glib::ustring> tooltip;

		Gtk::TreeModelColumn<Glib::ustring> group_name;
		Gtk::TreeModelColumn<Glib::ustring> parent_group_name;

		Gtk::TreeModelColumn<synfig::Canvas::Handle> canvas;

		Gtk::TreeModelColumn<bool>						active;
		Gtk::TreeModelColumn<bool>						is_layer;
		Gtk::TreeModelColumn<bool>						is_group;
		Gtk::TreeModelColumn<synfig::Layer::Handle>		layer;

		Gtk::TreeModelColumn<LayerList>		all_layers;
		Gtk::TreeModelColumn<LayerList>		child_layers;

		Gtk::TreeModelColumn<float> z_depth;

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
			add(z_depth);
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

	std::shared_ptr<synfigapp::CanvasInterface> canvas_interface_;

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


	void on_group_pair_added(synfig::String group, std::shared_ptr<synfig::Layer> layer);
	void on_group_pair_removed(synfig::String group, std::shared_ptr<synfig::Layer> layer);

	void on_activity();

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	bool on_layer_tree_event(GdkEvent *event);

	void on_layer_new_description(synfig::Layer::Handle handle,synfig::String desc);

	void on_layer_status_changed(synfig::Layer::Handle handle,bool);

	bool find_layer_row_(const synfig::Layer::Handle &handle, synfig::Canvas::Handle canvas, Gtk::TreeModel::Children layers, Gtk::TreeModel::Children::iterator &iter, Gtk::TreeModel::Children::iterator &prev);

	bool find_group_row_(const synfig::String &group, Gtk::TreeModel::Children layers, Gtk::TreeModel::Children::iterator &iter, Gtk::TreeModel::Children::iterator &prev);

	bool on_group_removed(synfig::String group);
	bool on_group_changed(synfig::String group);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	LayerGroupTreeStore(std::shared_ptr<synfigapp::CanvasInterface> canvas_interface_);
	~LayerGroupTreeStore();

	Gtk::TreeRow on_group_added(synfig::String group);
	std::shared_ptr<synfigapp::CanvasInterface> canvas_interface() { return canvas_interface_; }
	std::shared_ptr<const synfigapp::CanvasInterface> canvas_interface()const { return canvas_interface_; }
	std::shared_ptr<synfigapp::CanvasInterface> get_canvas_interface()const { return canvas_interface_; }

	bool find_layer_row(const synfig::Layer::Handle &handle, Gtk::TreeModel::Children::iterator &iter);

	bool find_group_row(const synfig::String &group, Gtk::TreeModel::Children::iterator &iter);

	bool find_prev_layer_row(const synfig::Layer::Handle &handle, Gtk::TreeModel::Children::iterator &iter);

	void rebuild();

	void refresh();

	/// Re-apply current sorting
	void resort();

	void refresh_row(Gtk::TreeModel::Row &row);

	void set_row_layer(Gtk::TreeRow &row,synfig::Layer::Handle &handle);

	static bool search_func(const Glib::RefPtr<TreeModel>&,int,const Glib::ustring&,const TreeModel::iterator&);

	/*
 -- ** -- S T A T I C   P U B L I C   M E T H O D S ---------------------------
	*/

public:

	static Glib::RefPtr<LayerGroupTreeStore> create(std::shared_ptr<synfigapp::CanvasInterface> canvas_interface_);

}; // END of class LayerGroupTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
