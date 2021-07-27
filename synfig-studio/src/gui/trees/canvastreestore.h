/* === S Y N F I G ========================================================= */
/*!	\file trees/canvastreestore.h
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

#ifndef __SYNFIG_STUDIO_CANVASTREESTORE_H
#define __SYNFIG_STUDIO_CANVASTREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gdkmm/pixbuf.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class CellRenderer_TimeTrack;
class CellRenderer_ValueBase;

	enum ColumnID
	{
		COLUMNID_ID,
		COLUMNID_VALUE,
		COLUMNID_TIME_TRACK,
		COLUMNID_TYPE,

		COLUMNID_END			//!< \internal
	};
#define	COLUMNID_NAME COLUMNID_ID

class CanvasTreeStore : virtual public Gtk::TreeStore
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

		Gtk::TreeModelColumn<synfig::Canvas::Handle> canvas;
		Gtk::TreeModelColumn<bool> is_canvas;

		Gtk::TreeModelColumn<synfig::ValueNode::Handle> value_node;
		Gtk::TreeModelColumn<bool> is_value_node;
		Gtk::TreeModelColumn<synfig::ValueBase> value;
		Gtk::TreeModelColumn<Glib::ustring> type;
		Gtk::TreeModelColumn<int> link_id;
		Gtk::TreeModelColumn<int> link_count;

		Gtk::TreeModelColumn<bool> is_editable;

		Gtk::TreeModelColumn<bool> is_shared;
		Gtk::TreeModelColumn<bool> is_exported;

		Gtk::TreeModelColumn<synfigapp::ValueDesc> value_desc;
		Gtk::TreeModelColumn<synfig::ParamDesc>	child_param_desc;

		Gtk::TreeModelColumn<Glib::ustring> tooltip;
		
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > interpolation_icon;
		Gtk::TreeModelColumn<bool> is_static;
		Gtk::TreeModelColumn<bool> interpolation_icon_visible;

		Model()
		{
			add(value);
			add(name);
			add(label);
			add(icon);
			add(type);
			add(id);
			add(canvas);
			add(value_node);
			add(is_canvas);
			add(is_value_node);

			add(is_shared);
			add(is_exported);
			add(is_editable);
			add(value_desc);
			add(child_param_desc);
			add(link_count);
			add(link_id);

			add(tooltip);
			add(interpolation_icon);
			add(is_static);
			add(interpolation_icon_visible);
		}
	};

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:

	const Model model;

	//std::multimap<etl::handle<ValueNode>, sigc::connection> connection_map;

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	std::shared_ptr<synfigapp::CanvasInterface> canvas_interface_;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:
	synfig::ValueNode::Handle expandable_bone_parent(synfig::ValueNode::Handle node);

protected:
	virtual void  get_value_vfunc (const Gtk::TreeModel::iterator& iter, int column, Glib::ValueBase& value)const;

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	CanvasTreeStore(std::shared_ptr<synfigapp::CanvasInterface> canvas_interface_);
	~CanvasTreeStore();

	std::shared_ptr<synfigapp::CanvasInterface> canvas_interface() { return canvas_interface_; }
	std::shared_ptr<const synfigapp::CanvasInterface> canvas_interface()const { return canvas_interface_; }

	virtual void rebuild_row(Gtk::TreeModel::Row &row, bool do_children=true);

	virtual void refresh_row(Gtk::TreeModel::Row &row, bool do_children=true);

	virtual void set_row(Gtk::TreeRow row,synfigapp::ValueDesc value_desc, bool do_children=true);

	bool find_first_value_desc(const synfigapp::ValueDesc& value_desc, Gtk::TreeIter& iter);
	bool find_next_value_desc(const synfigapp::ValueDesc& value_desc, Gtk::TreeIter& iter);

	bool find_first_value_node(const synfig::ValueNode::Handle& value_node, Gtk::TreeIter& iter);
	bool find_next_value_node(const synfig::ValueNode::Handle& value_node, Gtk::TreeIter& iter);


	static CellRenderer_ValueBase* add_cell_renderer_value(Gtk::TreeView::Column* column);

	static CellRenderer_TimeTrack* add_cell_renderer_value_node(Gtk::TreeView::Column* column);

	std::shared_ptr<synfigapp::CanvasInterface> get_canvas_interface()const { return canvas_interface_; }

	virtual void on_value_node_changed(synfig::ValueNode::Handle value_node)=0;

	/*
 -- ** -- P R O T E C T E D   M E T H O D S -----------------------------------
	*/

public:

}; // END of class CanvasTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
