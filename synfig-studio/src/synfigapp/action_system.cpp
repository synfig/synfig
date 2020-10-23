/* === S Y N F I G ========================================================= */
/*!	\file action_system.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/general.h>

#include "action_system.h"
#include "instance.h"
#include "canvasinterface.h"

#include <synfigapp/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

namespace {
	class Lock {
	private:
		int &counter;
	public:
		Lock(int &counter): counter(counter) { ++counter; }
		~Lock() { --counter; }
	};
}

/* === M E T H O D S ======================================================= */


Action::System::System():
	action_count_(0)
{
	unset_ui_interface();
	clear_redo_stack_on_new_action_=false;
}

Action::System::~System()
	{ }

void
Action::System::request_redraw(etl::handle<CanvasInterface> x)
{
	if (!x) return;
	if (!group_stack_.empty())
		{ group_stack_.front()->request_redraw(x); return; }
	x->signal_dirty_preview()();
}

bool
Action::System::perform_action(etl::handle<Action::Base> action)
{
	assert(action);
	if (getenv("SYNFIG_DEBUG_ACTIONS"))
		synfig::info("%s:%d perform_action: '%s'", __FILE__, __LINE__, action->get_name().c_str());

	etl::handle<UIInterface> uim = get_ui_interface();
	if (!action->is_ready()) {
		uim->error(action->get_local_name()+": "+_("Action is not ready."));
		return false;
	}

	most_recent_action_name_ = action->get_name();

	static int inuse = 0;
	if (inuse) return false;
	Lock lock(inuse);

	Action::CanvasSpecific *canvas_specific = dynamic_cast<Action::CanvasSpecific*>(action.get());

	if (canvas_specific && canvas_specific->get_canvas())
		uim = static_cast<Instance*>(this)->find_canvas_interface(canvas_specific->get_canvas())->get_ui_interface();

	// If we cannot undo this action, make sure
	// that the user knows this.
	etl::handle<Action::Undoable> undoable_action = etl::handle<Action::Undoable>::cast_dynamic(action);
	assert(!undoable_action || undoable_action->is_active());
	if (!undoable_action) {
		String message = etl::strprintf(_("Do you want to do action \"%s\"?"), action->get_local_name().c_str());
		String details = _("This action cannot be undone.");
		UIInterface::Response response = uim->confirmation(
			message,
			details,
			_("Cancel"),
			_("Continue"),
			UIInterface::RESPONSE_CANCEL );
		if (response == UIInterface::RESPONSE_CANCEL)
			return false;
		// Because this action cannot be undone,
		// we need to clear the undo stack
		clear_undo_stack();
	}

	// Perform the action
	try { action->perform(); }
	catch (const Action::Error& err) {
		uim->task(action->get_local_name()+' '+_("Failed"));
		if (err.get_type() != Action::Error::TYPE_UNABLE) {
			if (err.get_desc().empty())
				uim->error(action->get_local_name() + ": " + etl::strprintf("%d", err.get_type()));
			else
				uim->error(action->get_local_name() + ": " + err.get_desc());
		}
		// If action failed for whatever reason, just return false and do
		// not add the action onto the list
		return false;
	} catch (std::exception& err) {
		uim->task(action->get_local_name() + ' ' + _("Failed"));
		uim->error(action->get_local_name() + ": " + err.what());
		// If action failed for whatever reason, just return false and do
		// not add the action onto the list
		return false;
	} catch(...) {
		uim->task(action->get_local_name() + ' ' + _("Failed"));
		// If action failed for whatever reason, just return false and do
		// not add the action onto the list
		return false;
	}

	// Clear the redo stack
	if (clear_redo_stack_on_new_action_)
		clear_redo_stack();

	if (!group_stack_.empty())
		group_stack_.front()->inc_depth();
	else
		inc_action_count();

	// Push this action onto the action list if we can undo it
	if (undoable_action) {
		// If necessary, signal the change in status of undo
		if(undo_action_stack_.empty()) signal_undo_status_(true);

		// Add it to the list
		undo_action_stack_.push_front(undoable_action);

		// Signal that a new action has been added
		if(group_stack_.empty())
			signal_new_action()(undoable_action);
	}

	uim->task(action->get_local_name()+' '+_("Successful"));

	// If the action has "dirtied" the preview, signal it.
	if (canvas_specific && canvas_specific->is_dirty())
		request_redraw(canvas_specific->get_canvas_interface());

	return true;
}

bool
synfigapp::Action::System::undo_(etl::handle<UIInterface> uim)
{
	etl::handle<Action::Undoable> action = undo_action_stack().front();
	most_recent_action_name_ = action->get_name();

	try { if (action->is_active()) action->undo(); }
	catch (Action::Error &err) {
		if(err.get_type() != Action::Error::TYPE_UNABLE) {
			if(err.get_desc().empty())
				uim->error(action->get_local_name() + _(" (Undo): ") + etl::strprintf("%d",err.get_type()));
			else
				uim->error(action->get_local_name() + _(" (Undo): ") + err.get_desc());
		}
		return false;
	} catch (std::runtime_error &x) {
		uim->error(x.what());
		return false;
	} catch (...) {
		return false;
	}

	dec_action_count();

	if (redo_action_stack_.empty()) signal_redo_status()(true);

	redo_action_stack_.push_front(undo_action_stack_.front());
	undo_action_stack_.pop_front();

	if (undo_action_stack_.empty()) signal_undo_status()(false);

	if (!group_stack_.empty())
		group_stack_.front()->dec_depth();

	signal_undo_();

	return true;
}

bool
synfigapp::Action::System::undo()
{
	static int inuse = 0;
	if (inuse) return false;
	Lock lock(inuse);

	// If there is nothing on the action list, there is nothing to undo
	if (undo_action_stack().empty())
		return false;

	etl::handle<Action::Undoable> action = undo_action_stack().front();
	Action::CanvasSpecific *canvas_specific = dynamic_cast<Action::CanvasSpecific*>(action.get());

	etl::handle<UIInterface> uim = get_ui_interface();
	if (canvas_specific && canvas_specific->get_canvas())
		uim = static_cast<Instance*>(this)->find_canvas_interface(canvas_specific->get_canvas())->get_ui_interface();

	if (!undo_(uim)) {
		uim->error(undo_action_stack_.front()->get_local_name()+": "+_("Failed to undo."));
		return false;
	}

	// If the action has "dirtied" the preview, signal it.
	if(action->is_active() && canvas_specific && canvas_specific->is_dirty())
		request_redraw(canvas_specific->get_canvas_interface());

	return true;
}

bool
Action::System::redo_(etl::handle<UIInterface> uim)
{
	etl::handle<Action::Undoable> action = redo_action_stack().front();
	most_recent_action_name_ = action->get_name();

	try { if(action->is_active()) action->perform(); }
	catch (const Action::Error& err) {
		if (err.get_type() != Action::Error::TYPE_UNABLE) {
			if(err.get_desc().empty())
				uim->error(action->get_local_name() + _(" (Redo): ") + etl::strprintf("%d", err.get_type()));
			else
				uim->error(action->get_local_name() + _(" (Redo): ") + err.get_desc());
		}
		return false;
	} catch (const std::runtime_error &x) {
		uim->error(x.what());
		return false;
	} catch(...) {
		return false;
	}

	inc_action_count();

	if (undo_action_stack_.empty()) signal_undo_status()(true);

	undo_action_stack_.push_front(redo_action_stack_.front());
	redo_action_stack_.pop_front();

	if (redo_action_stack_.empty()) signal_redo_status()(false);

	if(!group_stack_.empty())
		group_stack_.front()->inc_depth();

	signal_redo_();

	return true;
}

bool
Action::System::redo()
{
	static int inuse = 0;
	if (inuse) return false;
	Lock lock(inuse);

	// If there is nothing on the action list, there is nothing to undo
	if (redo_action_stack_.empty())
		return false;

	etl::handle<Action::Undoable> action = redo_action_stack().front();
	Action::CanvasSpecific *canvas_specific = dynamic_cast<Action::CanvasSpecific*>(action.get());

	etl::handle<UIInterface> uim = get_ui_interface();
	if (canvas_specific && canvas_specific->get_canvas())
		uim = static_cast<Instance*>(this)->find_canvas_interface(canvas_specific->get_canvas())->get_ui_interface();

	if (!redo_(uim)) {
		uim->error(redo_action_stack_.front()->get_local_name()+": "+_("Failed to redo."));
		return false;
	}

	// If the action has "dirtied" the preview, signal it.
	if (action->is_active() && canvas_specific && canvas_specific->is_dirty())
		request_redraw(canvas_specific->get_canvas_interface());

	return true;
}

void
Action::System::inc_action_count() const
{
	action_count_++;
	if (action_count_ == 1)
		signal_unsaved_status_changed_(true);
	if (!action_count_)
		signal_unsaved_status_changed_(false);
}

void
Action::System::dec_action_count() const
{
	action_count_--;
	if(action_count_==-1)
		signal_unsaved_status_changed_(true);
	if(!action_count_)
		signal_unsaved_status_changed_(false);
}

void
Action::System::reset_action_count() const
{
	if (!action_count_)
		return;
	action_count_ = 0;
	signal_unsaved_status_changed_(false);
}

void
Action::System::clear_undo_stack()
{
	if (undo_action_stack_.empty()) return;
	undo_action_stack_.clear();
	signal_undo_status_(false);
	signal_undo_stack_cleared_();
}

void
Action::System::clear_redo_stack()
{
	if (redo_action_stack_.empty()) return;
	redo_action_stack_.clear();
	signal_redo_status_(false);
	signal_redo_stack_cleared_();
}

bool
Action::System::set_action_status(etl::handle<Action::Undoable> action, bool x)
{
	if (action->is_active() == x)
		return true;

	etl::handle<Action::Undoable> cur_pos = undo_action_stack_.front();
	Action::CanvasSpecific *canvas_specific = dynamic_cast<Action::CanvasSpecific*>(action.get());

	etl::handle<UIInterface> uim = new ConfidentUIInterface();

	Stack::iterator iter = std::find(undo_action_stack_.begin(), undo_action_stack_.end(), action);
	if (iter != undo_action_stack_.end()) {
		while(undo_action_stack_.front() != action)
			if (!undo_(uim)) return false;
		if (!undo_(uim)) return false;

		action->set_active(x);

		bool success = redo_(get_ui_interface());
		if (success)
			signal_action_status_changed_(action);
		else
			action->set_active(!x);

		while(undo_action_stack_.front() != cur_pos)
			if (!redo_(uim)) {
				redo_action_stack_.front()->set_active(false);
				signal_action_status_changed_(redo_action_stack_.front());
			}

		if (success && canvas_specific && canvas_specific->is_dirty())
			request_redraw(canvas_specific->get_canvas_interface());

		return true;
	}

	iter = std::find(redo_action_stack_.begin(), redo_action_stack_.end(), action);
	if (iter!=redo_action_stack_.end()) {
		action->set_active(x);
		signal_action_status_changed_(action);
		if (canvas_specific && canvas_specific->is_dirty())
			request_redraw(canvas_specific->get_canvas_interface());
		return true;
	}

	return false;
}

Action::PassiveGrouper::PassiveGrouper(etl::loose_handle<System> instance_,synfig::String name_):
	instance_(instance_),
	name_(name_),
	depth_(0),
	finished_(false)
{
	// Add this group onto the group stack
	instance_->group_stack_.push_front(this);
}

void
Action::PassiveGrouper::request_redraw(etl::handle<CanvasInterface> x)
	{ if (x) redraw_set_.insert(x); }

Action::PassiveGrouper::~PassiveGrouper()
	{ if (!finished_) finish(); }

etl::handle<Action::Group>
Action::PassiveGrouper::finish()
{
	assert(!finished_);
	if (finished_) return etl::handle<Action::Group>();
	finished_ = true;

	// Remove this group from the group stack
	assert(instance_->group_stack_.front() == this);
	instance_->group_stack_.pop_front();

	etl::handle<Action::Group> group;
	if (depth_ == 1) {
		etl::handle<Action::Undoable> action = instance_->undo_action_stack_.front();
		group = etl::handle<Action::Group>::cast_dynamic(action);
		if (group) {
			// If the only action inside of us is a group,
			// then we should rename the group to our name.
			group->set_name(name_);
		} else
		if (Action::CanvasSpecific* canvas_specific = dynamic_cast<Action::CanvasSpecific*>(action.get()))
			if (canvas_specific->is_dirty() && canvas_specific->get_canvas_interface())
				if (instance_->group_stack_.empty())
					request_redraw(canvas_specific->get_canvas_interface());

		if (instance_->group_stack_.empty()) {
			instance_->inc_action_count();
			instance_->signal_new_action()(instance_->undo_action_stack_.front());
		} else
			instance_->group_stack_.front()->inc_depth();
	} else
	if (depth_ > 1) {
		group = new Action::Group(name_);

		for(int i=0; i < depth_; i++) {
			etl::handle<Action::Undoable> action = instance_->undo_action_stack_.front();

			if (Action::CanvasSpecific* canvas_specific = dynamic_cast<Action::CanvasSpecific*>(action.get()))
				if (canvas_specific->is_dirty()) {
					group->set_dirty(true);
					group->set_canvas(canvas_specific->get_canvas());
					group->set_canvas_interface(canvas_specific->get_canvas_interface());
				}

			// Copy the action from the undo stack to the group
			group->add_action_front(action);

			// Remove the action from the undo stack
			instance_->undo_action_stack_.pop_front();
		}

		// Push the group onto the stack
		instance_->undo_action_stack_.push_front(group);

		if(group->is_dirty())
			request_redraw(group->get_canvas_interface());

		if(instance_->group_stack_.empty()) {
			instance_->inc_action_count();
			instance_->signal_new_action()(instance_->undo_action_stack_.front());
		} else
			instance_->group_stack_.front()->inc_depth();
	}

	// redraw request
	for(RedrawSet::const_iterator i = redraw_set_.begin(); i != redraw_set_.end(); ++i)
		instance_->request_redraw(*i);
	redraw_set_.clear();

	return group;
}

void
Action::PassiveGrouper::cancel()
{
	assert(!finished_);
	if (finished_) return;

	// Cancel any groupers that may be on top of us first
	//while(instance_->group_stack_.front()!=this)
	//	instance_->group_stack_.front()->cancel();
	synfig::warning("Cancel depth: %d",depth_);

	bool success = true;
	while(success && depth_)
		if (!instance_->undo())
			success = false;

	if (success)
		redraw_set_.clear();
	else
		instance_->get_ui_interface()->error(_("State restore failure"));
}
