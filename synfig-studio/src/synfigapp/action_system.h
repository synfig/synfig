/* === S Y N F I G ========================================================= */
/*!	\file action_system.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIGAPP_ACTIONSYSTEM_H
#define __SYNFIGAPP_ACTIONSYSTEM_H

/* === H E A D E R S ======================================================= */

#include <set>

#include <sigc++/sigc++.h>

#include <ETL/handle>

#include <synfig/canvas.h>

#include "action.h"
#include "uimanager.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class CanvasInterface;

namespace Action {





class System;

//! Passive action grouping class
class PassiveGrouper
{
	typedef std::set< etl::handle<CanvasInterface> > RedrawSet;

	etl::loose_handle<System> instance_;
	synfig::String name_;
	int depth_;
	RedrawSet redraw_set_;
	bool finished_;

public:
	PassiveGrouper(etl::loose_handle<System> instance_,synfig::String name_);

	~PassiveGrouper();

	const synfig::String &get_name()const { return name_; }

	void set_name(const synfig::String &x) { name_=x; }

	etl::loose_handle<System> get_instance() { return instance_; }

	void request_redraw(etl::handle<CanvasInterface>);

	etl::handle<Action::Group> finish();

	void cancel();

	void inc_depth() { depth_++; }

	void dec_depth() { depth_--; }

	const int &get_depth()const { return depth_; }
}; // END of class Action::PassiveGrouper

typedef std::list< etl::handle<Action::Undoable> > Stack;

class System : public etl::shared_object, public sigc::trackable
{
	friend class PassiveGrouper;

	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	Stack undo_action_stack_;
	Stack redo_action_stack_;

	synfig::String most_recent_action_name_;

	std::list<PassiveGrouper*> group_stack_;

	sigc::signal<void,bool> signal_undo_status_;
	sigc::signal<void,bool> signal_redo_status_;
	sigc::signal<void,etl::handle<Action::Undoable> > signal_new_action_;
	sigc::signal<void> signal_undo_stack_cleared_;
	sigc::signal<void> signal_redo_stack_cleared_;
	sigc::signal<void> signal_undo_;
	sigc::signal<void> signal_redo_;
	sigc::signal<void,etl::handle<Action::Undoable> > signal_action_status_changed_;

	mutable sigc::signal<void,bool> signal_unsaved_status_changed_;

	//! If this is non-zero, then the changes have not yet been saved.
	mutable int action_count_;

	etl::handle<UIInterface> ui_interface_;

	bool clear_redo_stack_on_new_action_;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	bool undo_(etl::handle<UIInterface> uim);
	bool redo_(etl::handle<UIInterface> uim);

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	System();
	~System();

	/*
	template <typename T> bool
	perform_action(T x)
	{
		etl::handle<Action::Base> action((Action::Base*)new T(x));
		return perform_action(action);
	}
	*/

	synfig::String get_most_recent_action_name() { return most_recent_action_name_; }

	bool get_clear_redo_stack_on_new_action()const { return clear_redo_stack_on_new_action_; }

	void set_clear_redo_stack_on_new_action(bool x) { clear_redo_stack_on_new_action_=x; }

	void request_redraw(etl::handle<CanvasInterface>);

	bool perform_action(etl::handle<Action::Base> action);

	bool set_action_status(etl::handle<Action::Undoable> action, bool x);

	const Stack &undo_action_stack()const { return undo_action_stack_; }

	const Stack &redo_action_stack()const { return redo_action_stack_; }

	//! Undoes the last action
	bool undo();

	//! Redoes the last undone action
	bool redo();

	//! Clears the undo stack.
	void clear_undo_stack();

	//! Clears the redo stack.
	void clear_redo_stack();

	//! Increments the action counter
	/*! \note You should not have to call this under normal circumstances.
	**	\see dec_action_count(), reset_action_count(), get_action_count() */
	void inc_action_count()const;

	//! Decrements the action counter
	/*! \note You should not have to call this under normal circumstances.
	**	\see inc_action_count(), reset_action_count(), get_action_count() */
	void dec_action_count()const;

	//! Resets the action counter
	/*! \note You should not have to call this under normal circumstances.
	**	\see inc_action_count(), dec_action_count(), get_action_count() */
	void reset_action_count()const;

	//! Returns the number of actions performed since last save.
	/*!	\see inc_action_count(), dec_action_count(), reset_action_count() */
	int get_action_count()const { return action_count_; }

	void set_ui_interface(const etl::handle<UIInterface> &uim) { assert(uim); ui_interface_=uim; }
	void unset_ui_interface() { ui_interface_=new DefaultUIInterface(); }
	const etl::handle<UIInterface> &get_ui_interface() { return ui_interface_; }

	/*
 -- ** -- S I G N A L   I N T E R F A C E S -----------------------------------
	*/

public:

	sigc::signal<void,bool>& signal_unsaved_status_changed() { return signal_unsaved_status_changed_; }

	sigc::signal<void,bool>& signal_undo_status() { return signal_undo_status_; }

	sigc::signal<void,bool>& signal_redo_status() { return signal_redo_status_; }

	sigc::signal<void>& signal_undo_stack_cleared() { return signal_undo_stack_cleared_; }

	sigc::signal<void>& signal_redo_stack_cleared() { return signal_redo_stack_cleared_; }

	sigc::signal<void>& signal_undo() { return signal_undo_; }

	sigc::signal<void>& signal_redo() { return signal_redo_; }

	//!	Called whenever an undoable action is processed and added to the stack.
	sigc::signal<void,etl::handle<Action::Undoable> >& signal_new_action() { return signal_new_action_; }

	sigc::signal<void,etl::handle<Action::Undoable> >& signal_action_status_changed() { return signal_action_status_changed_; }

}; // END of class Action::System


}; // END of namespace synfigapp::Action
}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
