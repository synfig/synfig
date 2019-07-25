/* === S Y N F I G ========================================================= */
/*!	\file trees/keyframetreestore.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#ifndef __SYNFIG_STUDIO_KEYFRAMETREESTORE_H
#define __SYNFIG_STUDIO_KEYFRAMETREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/liststore.h>
#include <synfigapp/canvasinterface.h>
#include <gdkmm/pixbuf.h>
#include <synfig/keyframe.h>
#include <map>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

//class TreeRowReferenceHack;
//#define TreeRowReferenceHack Gtk::TreeRowReference


namespace studio {

class KeyframeTreeStore_Class;
#if GLIB_CHECK_VERSION(2, 37, 5)
class KeyframeTreeStore :
	public Gtk::TreeModel,
	public Gtk::TreeDragSource,
	public Gtk::TreeDragDest,
	public Glib::Object
#else
class KeyframeTreeStore :
	public Glib::Object,
	public Gtk::TreeModel,
	public Gtk::TreeDragSource,
	public Gtk::TreeDragDest
#endif
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	class Model : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Gtk::TreeModelColumn<synfig::Time> time;
		Gtk::TreeModelColumn<Glib::ustring> description;
		Gtk::TreeModelColumn<synfig::Keyframe> keyframe;
		Gtk::TreeModelColumn<synfig::Time> time_delta;
		Gtk::TreeModelColumn<bool> active;

		Model()
		{
			add(time);
			add(description);
			add(keyframe);
			add(time_delta);
			add(active);
		}
	};

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:

	const Model model;

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_;

	//! Unique stamp for this TreeModel.
	int stamp_;

	//std::map<synfig::Keyframe,TreeRowReferenceHack> path_table_;

	synfig::KeyframeList old_keyframe_list;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	void add_keyframe(synfig::Keyframe);

	void remove_keyframe(synfig::Keyframe);

	void change_keyframe(synfig::Keyframe);

	static int sorter(const Gtk::TreeModel::iterator &,const Gtk::TreeModel::iterator &);

	bool iterator_sane(const GtkTreeIter* iter)const;

	bool iterator_sane(const Gtk::TreeModel::iterator& iter)const;

	void dump_iterator(const GtkTreeIter* iter, const Glib::ustring &name)const;

	void dump_iterator(const Gtk::TreeModel::iterator& iter, const Glib::ustring &name)const;

	//! Resets the iterator stamp for this model.
	/*!	This should be called whenever the class is
	**	constructed	or when large numbers of
	**	iterators become invalid. */
	void reset_stamp();

	//void reset_path_table();

	/*
 -- ** -- V I R T U A L   F U N C T I O N S -----------------------------------
	*/

protected:

	using Gtk::TreeModel::set_value_impl;
	using Gtk::TreeModel::get_flags_vfunc;
	using Gtk::TreeModel::get_n_columns_vfunc;
	using Gtk::TreeModel::get_column_type_vfunc;
	using Gtk::TreeModel::ref_node_vfunc;
	using Gtk::TreeModel::unref_node_vfunc;

	virtual void  set_value_impl (const Gtk::TreeModel::iterator& row, int column, const Glib::ValueBase& value);
	virtual Gtk::TreeModelFlags get_flags_vfunc ();
	virtual int   get_n_columns_vfunc ();
	virtual GType get_column_type_vfunc (int index);
	virtual bool  iter_next_vfunc (const iterator& iter, iterator& iter_next) const;
	virtual bool  get_iter_vfunc (const Gtk::TreeModel::Path& path, iterator& iter_next)const;
	virtual bool  iter_nth_root_child_vfunc (int n, iterator& iter)const;
	virtual Gtk::TreeModel::Path get_path_vfunc (const iterator& iter)const;
	virtual void  ref_node_vfunc (iterator& iter)const;
	virtual void  unref_node_vfunc (iterator& iter)const;
	virtual void  get_value_vfunc (const Gtk::TreeModel::iterator& iter, int column, Glib::ValueBase& value)const;
	virtual bool  iter_is_valid (const iterator& iter) const;
	virtual int   iter_n_root_children_vfunc () const;

	//virtual bool  iter_nth_child_vfunc (GtkTreeIter* iter, const GtkTreeIter* parent, int n);
	//virtual bool  iter_children_vfunc (GtkTreeIter* iter, const GtkTreeIter* parent);
	//virtual bool  iter_has_child_vfunc (const GtkTreeIter* iter);
	//virtual int  iter_n_children_vfunc (const GtkTreeIter* iter);
	//virtual bool  iter_parent_vfunc (GtkTreeIter* iter, const GtkTreeIter* child);

	/*
	virtual bool  get_sort_column_id_vfunc (int* sort_column_id, Gtk::SortType* order);
	virtual void  set_sort_column_id_vfunc (int sort_column_id, Gtk::SortType order);
	virtual void  set_sort_func_vfunc (int sort_column_id, GtkTreeIterCompareFunc func, void* data, GtkDestroyNotify destroy);
	virtual void  set_default_sort_func_vfunc (GtkTreeIterCompareFunc func, void* data, GtkDestroyNotify destroy);
	virtual bool  has_default_sort_func_vfunc ();
	*/

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	KeyframeTreeStore(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_);
	~KeyframeTreeStore();

	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface() { return canvas_interface_; }
	etl::loose_handle<const synfigapp::CanvasInterface> canvas_interface()const { return canvas_interface_; }

	const synfig::Canvas::Handle& get_canvas()const { return canvas_interface()->get_canvas(); }

	Gtk::TreeModel::Row find_row(const synfig::Keyframe &keyframe);

	bool find_keyframe_path(const synfig::Keyframe &keyframe, Gtk::TreeModel::Path &path);
	/*
 -- ** -- S T A T I C  M E T H O D S ------------------------------------------
	*/

public:

	static Glib::RefPtr<KeyframeTreeStore> create(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_);

	static int time_sorter(const Gtk::TreeModel::iterator &rhs,const Gtk::TreeModel::iterator &lhs);
	static int description_sorter(const Gtk::TreeModel::iterator &rhs,const Gtk::TreeModel::iterator &lhs);

}; // END of class KeyframeTreeStore


}; // END of namespace studio

/* === E N D =============================================================== */

#endif
