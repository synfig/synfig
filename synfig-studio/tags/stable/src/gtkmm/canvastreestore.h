/* === S I N F G =========================================================== */
/*!	\file canvastreestore.h
**	\brief Template Header
**
**	$Id: canvastreestore.h,v 1.1.1.1 2005/01/07 03:34:35 darco Exp $
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

#ifndef __SINFG_STUDIO_CANVASTREESTORE_H
#define __SINFG_STUDIO_CANVASTREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treestore.h>
#include <sinfgapp/canvasinterface.h>
#include <gdkmm/pixbuf.h>
#include <sinfgapp/value_desc.h>
#include <gtkmm/treeview.h>

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

		Gtk::TreeModelColumn<sinfg::Canvas::Handle> canvas;
		Gtk::TreeModelColumn<bool> is_canvas;

		Gtk::TreeModelColumn<sinfg::ValueNode::Handle> value_node;
		Gtk::TreeModelColumn<bool> is_value_node;
		Gtk::TreeModelColumn<sinfg::ValueBase> value;
		Gtk::TreeModelColumn<Glib::ustring> type;
		Gtk::TreeModelColumn<int> link_id;
		Gtk::TreeModelColumn<int> link_count;

		Gtk::TreeModelColumn<bool> is_editable;

		Gtk::TreeModelColumn<bool> is_shared;
		Gtk::TreeModelColumn<bool> is_exported;
	
		Gtk::TreeModelColumn<sinfgapp::ValueDesc> value_desc;
	
		Gtk::TreeModelColumn<Glib::ustring> tooltip;

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
			add(link_count);
			add(link_id);
			
			add(tooltip);
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

	etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

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
	
	CanvasTreeStore(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_);
	~CanvasTreeStore();

	etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface() { return canvas_interface_; }
	etl::loose_handle<const sinfgapp::CanvasInterface> canvas_interface()const { return canvas_interface_; }

	virtual void rebuild_row(Gtk::TreeModel::Row &row, bool do_children=true);

	virtual void refresh_row(Gtk::TreeModel::Row &row, bool do_children=true);

	virtual void set_row(Gtk::TreeRow row,sinfgapp::ValueDesc value_desc, bool do_children=true);

	bool find_first_value_desc(const sinfgapp::ValueDesc& value_desc, Gtk::TreeIter& iter);
	bool find_next_value_desc(const sinfgapp::ValueDesc& value_desc, Gtk::TreeIter& iter);

	bool find_first_value_node(const sinfg::ValueNode::Handle& value_node, Gtk::TreeIter& iter);
	bool find_next_value_node(const sinfg::ValueNode::Handle& value_node, Gtk::TreeIter& iter);
	
	
	static CellRenderer_ValueBase* add_cell_renderer_value(Gtk::TreeView::Column* column);

	static CellRenderer_TimeTrack* add_cell_renderer_value_node(Gtk::TreeView::Column* column);

	etl::loose_handle<sinfgapp::CanvasInterface> get_canvas_interface()const { return canvas_interface_; }

	virtual void on_value_node_changed(sinfg::ValueNode::Handle value_node)=0;

	/*
 -- ** -- P R O T E C T E D   M E T H O D S -----------------------------------
	*/

public:
	
}; // END of class CanvasTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
