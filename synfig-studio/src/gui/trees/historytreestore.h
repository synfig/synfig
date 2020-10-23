/* === S Y N F I G ========================================================= */
/*!	\file trees/historytreestore.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#ifndef __SYNFIG_STUDIO_HISTORYTREESTORE_H
#define __SYNFIG_STUDIO_HISTORYTREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gdkmm/pixbuf.h>
#include <gtkmm/treestore.h>
#include <synfigapp/action.h>

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
		Gtk::TreeModelColumn<etl::handle<synfigapp::Action::Undoable> > action;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<bool> is_active;
		Gtk::TreeModelColumn<bool> is_undo;
		Gtk::TreeModelColumn<bool> is_redo;

		Gtk::TreeModelColumn<Glib::ustring> canvas_id;
		Gtk::TreeModelColumn<synfig::Canvas::Handle> canvas;

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
	//! points to next action
	Gtk::TreeIter next_action_iter;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	void insert_action(Gtk::TreeRow row, etl::handle<synfigapp::Action::Undoable> action, bool is_undo=true, bool is_redo=false);

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	sigc::signal<void> signal_undo_tree_changed_;

	/*
 -- ** -- S I G N A L   I N T E R F A C E S -----------------------------------
	*/

public:

	sigc::signal<void>& signal_undo_tree_changed() { return signal_undo_tree_changed_; }

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	void on_undo();

	void on_redo();

	void on_undo_stack_cleared();

	void on_redo_stack_cleared();

	void on_new_action(etl::handle<synfigapp::Action::Undoable> action);

	void on_action_status_changed(etl::handle<synfigapp::Action::Undoable> action);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	etl::loose_handle<studio::Instance> instance() { return instance_; }
	etl::loose_handle<const studio::Instance> instance()const { return instance_; }

	//! Use this method carefully: if redo action stack refers to not-yet-(re)created objects
	//! - like vertex movements -, the Action::get_local_name() can make App crash!
	void rebuild();

	static bool search_func(const Glib::RefPtr<TreeModel>&,int,const Glib::ustring&,const TreeModel::iterator&);

	static Glib::RefPtr<HistoryTreeStore> create(etl::loose_handle<studio::Instance> instance);

	/*
 -- ** -- P R O T E C T E D   M E T H O D S -----------------------------------
	*/

protected:

	HistoryTreeStore(etl::loose_handle<studio::Instance> instance_);
	~HistoryTreeStore();

}; // END of class HistoryTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
