/* === S I N F G =========================================================== */
/*!	\file historytreestore.h
**	\brief Template Header
**
**	$Id: historytreestore.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_HISTORYTREESTORE_H
#define __SINFG_STUDIO_HISTORYTREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treestore.h>
#include <sinfgapp/canvasinterface.h>
#include <gdkmm/pixbuf.h>
#include <sinfgapp/action.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Instance;
	
class HistoryTreeStore : virtual public Gtk::TreeStore
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	class Model : public Gtk::TreeModel::ColumnRecord
	{
	public:
	public:
		Gtk::TreeModelColumn<etl::handle<sinfgapp::Action::Undoable> > action;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<bool> is_active;
		Gtk::TreeModelColumn<bool> is_undo;
		Gtk::TreeModelColumn<bool> is_redo;

		Gtk::TreeModelColumn<Glib::ustring> canvas_id;
		Gtk::TreeModelColumn<sinfg::Canvas::Handle> canvas;

		Model()
		{
			add(action);
			add(name);
			add(icon);
			add(is_active);
			add(is_undo);
			add(is_redo);
			add(canvas_id);
			add(canvas);
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

	etl::loose_handle<studio::Instance> instance_;
	Gtk::TreeIter curr_row; 

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:
	
	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	void on_undo();
	
	void on_redo();
	
	void on_undo_stack_cleared();
	
	void on_redo_stack_cleared();
	
	void on_new_action(etl::handle<sinfgapp::Action::Undoable> action);

	void on_action_status_changed(etl::handle<sinfgapp::Action::Undoable> action);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	
	HistoryTreeStore(etl::loose_handle<studio::Instance> instance_);
	~HistoryTreeStore();

	etl::loose_handle<studio::Instance> instance() { return instance_; }
	etl::loose_handle<const studio::Instance> instance()const { return instance_; }

	void rebuild();

	void refresh() { rebuild(); }

	void insert_action(Gtk::TreeRow row,etl::handle<sinfgapp::Action::Undoable> action, bool is_active=true, bool is_undo=true, bool is_redo=false);

	/*
 -- ** -- P R O T E C T E D   M E T H O D S -----------------------------------
	*/

public:

	static Glib::RefPtr<HistoryTreeStore> create(etl::loose_handle<studio::Instance> instance);
	
}; // END of class HistoryTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
