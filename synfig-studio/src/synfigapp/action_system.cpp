/* === S Y N F I G ========================================================= */
/*!	\file action_system.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "action_system.h"
#include "instance.h"
#include "canvasinterface.h"

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfigapp;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */



Action::System::System():
	action_count_(0)
{
	unset_ui_interface();
	clear_redo_stack_on_new_action_=false;
}

Action::System::~System()
{
}

bool
Action::System::perform_action(etl::handle<Action::Base> action)
{
	if (getenv("SYNFIG_DEBUG_ACTIONS"))
		synfig::info("%s:%d perform_action: '%s'", __FILE__, __LINE__, action->get_name().c_str());

	handle<UIInterface> uim(get_ui_interface());

	assert(action);

	if(!action->is_ready())
	{
		uim->error(action->get_local_name()+": "+_("Action is not ready."));
		return false;
	}

	most_recent_action_name_=action->get_name();

	static bool inuse=false;

	if(inuse) return false;

	inuse=true;
	try {

	assert(action);

	Action::CanvasSpecific* canvas_specific(dynamic_cast<Action::CanvasSpecific*>(action.get()));

	if(canvas_specific && canvas_specific->get_canvas())
	{
		handle<CanvasInterface> canvas_interface=static_cast<Instance*>(this)->find_canvas_interface(canvas_specific->get_canvas());
		assert(canvas_interface);
		uim=canvas_interface->get_ui_interface();
	}

	handle<Action::Undoable> undoable_action=handle<Action::Undoable>::cast_dynamic(action);

	// If we cannot undo this action, make sure
	// that the user knows this.
	if(!undoable_action)
	{
		string message = strprintf(_("Do you want to do action \"%s\"?"), action->get_local_name().c_str());
		string details = _("This action cannot be undone.");

		if(uim->confirmation(
			message,
			details,
			_("Cancel"),
			_("Continue"),
			UIInterface::RESPONSE_CANCEL
			) == UIInterface::RESPONSE_CANCEL
		)
			return false;
		else
		{
			// Because this action cannot be undone,
			// we need to clear the undo stack
			clear_undo_stack();
		}
	}
	else
		assert(undoable_action->is_active());

	// Perform the action
	try { action->perform(); }
	catch(Action::Error err)
	{
		uim->task(action->get_local_name()+' '+_("Failed"));
		inuse=false;

		if(err.get_type()!=Action::Error::TYPE_UNABLE)
		{
			if(err.get_desc().empty())
				uim->error(action->get_local_name()+": "+strprintf("%d",err.get_type()));
			else
				uim->error(action->get_local_name()+": "+err.get_desc());
		}

		// If action failed for whatever reason, just return false and do
		// not add the action onto the list
		return false;
	}
	catch(std::exception err)
	{
		uim->task(action->get_local_name()+' '+_("Failed"));
		inuse=false;

		uim->error(action->get_local_name()+": "+err.what());

		// If action failed for whatever reason, just return false and do
		// not add the action onto the list
		return false;
	}
	catch(...)
	{
		uim->task(action->get_local_name()+' '+_("Failed"));
		inuse=false;

		// If action failed for whatever reason, just return false and do
		// not add the action onto the list
		return false;
	}

	// Clear the redo stack
	if(clear_redo_stack_on_new_action_)
		clear_redo_stack();

	if(!group_stack_.empty())
		group_stack_.front()->inc_depth();
	else
		inc_action_count();

	// Push this action onto the action list if we can undo it
	if(undoable_action)
	{
		// If necessary, signal the change in status of undo
		if(undo_action_stack_.empty()) signal_undo_status_(true);

		// Add it to the list
		undo_action_stack_.push_front(undoable_action);

		// Signal that a new action has been added
		if(group_stack_.empty())
			signal_new_action()(undoable_action);
	}

	inuse=false;

	uim->task(action->get_local_name()+' '+_("Successful"));

	// If the action has "dirtied" the preview, signal it.
	if(0)if(canvas_specific && canvas_specific->is_dirty())
	{
		Canvas::Handle canvas=canvas_specific->get_canvas();
		if(!group_stack_.empty())
			group_stack_.front()->request_redraw(canvas_specific->get_canvas_interface());
		else
		{
			handle<CanvasInterface> canvas_interface=static_cast<Instance*>(this)->find_canvas_interface(canvas);
			assert(canvas_interface);
			//canvas_interface->signal_dirty_preview()();
		}
	}

	}catch(...) { inuse=false; throw; }

	return true;
}

bool
synfigapp::Action::System::undo_(etl::handle<UIInterface> uim)
{
	handle<Action::Undoable> action(undo_action_stack().front());
	most_recent_action_name_=action->get_name();

	try { if(action->is_active()) action->undo(); }
	catch(Action::Error err)
	{
		if(err.get_type()!=Action::Error::TYPE_UNABLE)
		{
			if(err.get_desc().empty())
				uim->error(action->get_local_name()+_(" (Undo): ")+strprintf("%d",err.get_type()));
			else
				uim->error(action->get_local_name()+_(" (Undo): ")+err.get_desc());
		}

		return false;
	}
	catch(std::runtime_error x)
	{
		uim->error(x.what());
		return false;
	}
	catch(...)
	{
		return false;
	}

	dec_action_count();

	if(redo_action_stack_.empty())	signal_redo_status()(true);

	redo_action_stack_.push_front(undo_action_stack_.front());
	undo_action_stack_.pop_front();

	if(undo_action_stack_.empty())	signal_undo_status()(false);

	if(!group_stack_.empty())
		group_stack_.front()->dec_depth();

	signal_undo_();

	return true;
}

bool
synfigapp::Action::System::undo()
{
	//! \todo This function is not exception safe!
	static bool inuse=false;

	// If there is nothing on the action list, there is nothing to undo
	if(undo_action_stack().empty() || inuse)
		return false;

	handle<Action::Undoable> action=undo_action_stack().front();
	Action::CanvasSpecific* canvas_specific(dynamic_cast<Action::CanvasSpecific*>(action.get()));

	handle<UIInterface> uim;
	if(canvas_specific && canvas_specific->get_canvas())
	{
		Canvas::Handle canvas=canvas_specific->get_canvas();
		handle<CanvasInterface> canvas_interface=static_cast<Instance*>(this)->find_canvas_interface(canvas);
		assert(canvas_interface);
		uim=canvas_interface->get_ui_interface();
	}
	else
		uim=get_ui_interface();

	inuse=true;

	if(!undo_(uim))
	{
		uim->error(undo_action_stack_.front()->get_local_name()+": "+_("Failed to undo."));
		inuse=false;
		return false;
	}

	inuse=false;

	// If the action has "dirtied" the preview, signal it.
	if(0)if(action->is_active() && canvas_specific && canvas_specific->is_dirty())
	{
		Canvas::Handle canvas=canvas_specific->get_canvas();
		if(!group_stack_.empty())
			group_stack_.front()->request_redraw(canvas_specific->get_canvas_interface());
		else
		{
			handle<CanvasInterface> canvas_interface=static_cast<Instance*>(this)->find_canvas_interface(canvas);
			assert(canvas_interface);
			//canvas_interface->signal_dirty_preview()();
		}
	}

	return true;
}

bool
Action::System::redo_(etl::handle<UIInterface> uim)
{
	handle<Action::Undoable> action(redo_action_stack().front());
	most_recent_action_name_=action->get_name();

	try { if(action->is_active()) action->perform(); }
	catch(Action::Error err)
	{
		if(err.get_type()!=Action::Error::TYPE_UNABLE)
		{
			if(err.get_desc().empty())
				uim->error(action->get_local_name()+_(" (Redo): ")+strprintf("%d",err.get_type()));
			else
				uim->error(action->get_local_name()+_(" (Redo): ")+err.get_desc());
		}

		return false;
	}
	catch(std::runtime_error x)
	{
		uim->error(x.what());
		return false;
	}
	catch(...)
	{
		return false;
	}

	inc_action_count();

	if(undo_action_stack_.empty())	signal_undo_status()(true);

	undo_action_stack_.push_front(redo_action_stack_.front());
	redo_action_stack_.pop_front();

	if(redo_action_stack_.empty())	signal_redo_status()(false);

	if(!group_stack_.empty())
		group_stack_.front()->inc_depth();

	signal_redo_();

	return true;
}

bool
Action::System::redo()
{
	//! \todo This function is not exception safe!
	static bool inuse=false;

	// If there is nothing on the action list, there is nothing to undo
	if(redo_action_stack_.empty() || inuse)
		return false;

	inuse=true;

	handle<Action::Undoable> action=redo_action_stack().front();
	Action::CanvasSpecific* canvas_specific(dynamic_cast<Action::CanvasSpecific*>(action.get()));

	handle<UIInterface> uim;
	if(canvas_specific && canvas_specific->get_canvas())
	{
		Canvas::Handle canvas=canvas_specific->get_canvas();
		handle<CanvasInterface> canvas_interface=static_cast<Instance*>(this)->find_canvas_interface(canvas);
		assert(canvas_interface);
		uim=canvas_interface->get_ui_interface();
	}
	else
		uim=get_ui_interface();

	if(!redo_(uim))
	{
		uim->error(redo_action_stack_.front()->get_local_name()+": "+_("Failed to redo."));
		inuse=false;
		return false;
	}

	inuse=false;

	// If the action has "dirtied" the preview, signal it.
	if(0)if(action->is_active() && canvas_specific && canvas_specific->is_dirty())
	{
		Canvas::Handle canvas=canvas_specific->get_canvas();
		if(!group_stack_.empty())
			group_stack_.front()->request_redraw(canvas_specific->get_canvas_interface());
		else
		{
			handle<CanvasInterface> canvas_interface=static_cast<Instance*>(this)->find_canvas_interface(canvas);
			assert(canvas_interface);
			//canvas_interface->signal_dirty_preview()();
		}
	}

	return true;
}

void
Action::System::inc_action_count()const
{
	action_count_++;
	if(action_count_==1)
		signal_unsaved_status_changed_(true);
	if(!action_count_)
		signal_unsaved_status_changed_(false);
}

void
Action::System::dec_action_count()const
{
	action_count_--;
	if(action_count_==-1)
		signal_unsaved_status_changed_(true);
	if(!action_count_)
		signal_unsaved_status_changed_(false);
}

void
Action::System::reset_action_count()const
{
	if(!action_count_)
		return;

	action_count_=0;
	signal_unsaved_status_changed_(false);
}

void
Action::System::clear_undo_stack()
{
	if(undo_action_stack_.empty()) return;
	undo_action_stack_.clear();
	signal_undo_status_(false);
	signal_undo_stack_cleared_();
}

void
Action::System::clear_redo_stack()
{
	if(redo_action_stack_.empty()) return;
	redo_action_stack_.clear();
	signal_redo_status_(false);
	signal_redo_stack_cleared_();
}

bool
Action::System::set_action_status(etl::handle<Action::Undoable> action, bool x)
{
	Stack::iterator iter;
	int failed=false;

	if(action->is_active()==x)
		return true;

	handle<Action::Undoable> cur_pos=undo_action_stack_.front();

	Action::CanvasSpecific* canvas_specific(dynamic_cast<Action::CanvasSpecific*>(action.get()));

	handle<UIInterface> uim=new ConfidentUIInterface();

	iter=find(undo_action_stack_.begin(),undo_action_stack_.end(),action);
	if(iter!=undo_action_stack_.end())
	{
		while(undo_action_stack_.front()!=action)
		{
			if(!undo_(uim))
			{
				return false;
			}
		}
		if(!undo_(uim))
		{
			return false;
		}

		action->set_active(x);

		if(redo_(get_ui_interface()))
		{
			signal_action_status_changed_(action);
		}
		else
		{
			action->set_active(!x);
			failed=true;
		}


		while(undo_action_stack_.front()!=cur_pos)
		{
			if(!redo_(uim))
			{
				redo_action_stack_.front()->set_active(false);
				signal_action_status_changed_(redo_action_stack_.front());
			}
		}

		if(!failed && canvas_specific && canvas_specific->is_dirty())
		{
			Canvas::Handle canvas=canvas_specific->get_canvas();
			handle<CanvasInterface> canvas_interface=static_cast<Instance*>(this)->find_canvas_interface(canvas);
			assert(canvas_interface);
			//canvas_interface->signal_dirty_preview()();
		}

		return true;
	}

	iter=find(redo_action_stack_.begin(),redo_action_stack_.end(),action);
	if(iter!=redo_action_stack_.end())
	{
		action->set_active(x);
		signal_action_status_changed_(action);




		if(canvas_specific && canvas_specific->is_dirty())
		{
			Canvas::Handle canvas=canvas_specific->get_canvas();
			handle<CanvasInterface> canvas_interface=static_cast<Instance*>(this)->find_canvas_interface(canvas);
			assert(canvas_interface);
			//canvas_interface->signal_dirty_preview()();
		}

		return true;
	}

	return false;
}

Action::PassiveGrouper::PassiveGrouper(etl::loose_handle<System> instance_,synfig::String name_):
	instance_(instance_),
	name_(name_),
	redraw_requested_(false),
	depth_(0)
{
	// Add this group onto the group stack
	instance_->group_stack_.push_front(this);
}

void
Action::PassiveGrouper::request_redraw(etl::handle<CanvasInterface> x)
{
/*	if(instance_->group_stack_.empty())
	{
		if(x!=canvas_interface_)
		{
			x->signal_dirty_preview()();
		}

		redraw_requested_=false;
	}
	else
	{
		if(instance_->group_stack_.back()==this)
		{
			redraw_requested_=true;
		}
		else
		{
			instance_->group_stack_.back()->request_redraw(x);
			redraw_requested_=false;
		}
	}
*/
	if(x)
	{
		redraw_requested_=true;
		canvas_interface_=x;
	}
}

Action::PassiveGrouper::~PassiveGrouper()
{
	assert(instance_->group_stack_.front()==this);

	// Remove this group from the group stack
	instance_->group_stack_.pop_front();

	handle<Action::Group> group;

	if(depth_==1)
	{
		handle<Action::Undoable> action(instance_->undo_action_stack_.front());

		group=handle<Action::Group>::cast_dynamic(action);

		if(group)
		{
			// If the only action inside of us is a group,
			// then we should rename the group to our name.
			group->set_name(name_);
		}
		else
		{
			Action::CanvasSpecific* canvas_specific(dynamic_cast<Action::CanvasSpecific*>(action.get()));

			if(0)if(canvas_specific && canvas_specific->is_dirty() && canvas_specific->get_canvas_interface())
			{
				if(instance_->group_stack_.empty())
					request_redraw(canvas_specific->get_canvas_interface());
			}
		}

		if(instance_->group_stack_.empty())
		{
			instance_->inc_action_count();
			instance_->signal_new_action()(instance_->undo_action_stack_.front());
		}
		else
			instance_->group_stack_.front()->inc_depth();

	}
	else
	if(depth_>0)
	{
		group=new Action::Group(name_);

		for(int i=0;i<depth_;i++)
	//	for(;depth_;depth_--)
		{
			handle<Action::Undoable> action(instance_->undo_action_stack_.front());
			Action::CanvasSpecific* canvas_specific(dynamic_cast<Action::CanvasSpecific*>(action.get()));

			if(0)if(canvas_specific && canvas_specific->is_dirty())
			{
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

		if(0)if(group->is_dirty())
			request_redraw(group->get_canvas_interface());
		//	group->get_canvas_interface()->signal_dirty_preview()();

		if(instance_->group_stack_.empty())
		{
			instance_->inc_action_count();
			instance_->signal_new_action()(instance_->undo_action_stack_.front());
		}
		else
			instance_->group_stack_.front()->inc_depth();
	}

	if(0)if(redraw_requested_)
	{
		if(instance_->group_stack_.empty())
		{
			assert(canvas_interface_);
			canvas_interface_->signal_dirty_preview()();
		}
		else
		{
			instance_->group_stack_.front()->request_redraw(canvas_interface_);
			redraw_requested_=false;
		}
	}
}

void
Action::PassiveGrouper::cancel()
{
	bool error=false;

	// Cancel any groupers that may be on top of us first
	//while(instance_->group_stack_.front()!=this)
	//	instance_->group_stack_.front()->cancel();

	synfig::warning("Cancel depth: %d",depth_);

	while(depth_)
		if(!instance_->undo())
		{
			error=true;
			break;
		}

	if(error)
		instance_->get_ui_interface()->error(_("State restore failure"));
	else
		redraw_requested_=false;
}
