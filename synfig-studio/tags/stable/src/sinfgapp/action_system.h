/* === S I N F G =========================================================== */
/*!	\file action_system.h
**	\brief Template Header
**
**	$Id: action_system.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFGAPP_ACTIONSYSTEM_H
#define __SINFGAPP_ACTIONSYSTEM_H

/* === H E A D E R S ======================================================= */

#include "action.h"
#include <sigc++/signal.h>
#include <sigc++/object.h>
#include <ETL/handle>
#include <sinfg/canvas.h>
#include "uimanager.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {

class CanvasInterface;
	
namespace Action {


	
	
	
class System;

//! Passive action grouping class
class PassiveGrouper
{
	etl::loose_handle<System> instance_;
	sinfg::String name_;
	bool redraw_requested_;
	int depth_;
	etl::handle<CanvasInterface> canvas_interface_;
public:

	PassiveGrouper(etl::loose_handle<System> instance_,sinfg::String name_);

	~PassiveGrouper();

	const sinfg::String &get_name()const { return name_; }

	void set_name(const sinfg::String &x) { name_=x; }

	etl::loose_handle<System> get_instance() { return instance_; }
	
	void request_redraw(etl::handle<CanvasInterface>);
	
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

	etl::handle<Action::Base> most_recent_action_;

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

	const etl::handle<Action::Base>& get_most_recent_action() { return most_recent_action_; }

	bool get_clear_redo_stack_on_new_action()const { return clear_redo_stack_on_new_action_; }
	
	void set_clear_redo_stack_on_new_action(bool x) { clear_redo_stack_on_new_action_=x; }

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


}; // END of namespace sinfgapp::Action
}; // END of namespace sinfgapp

/* === E N D =============================================================== */

#endif
