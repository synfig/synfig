/* === S Y N F I G ========================================================= */
/*!	\file historytreestore.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/trees/historytreestore.h>

#include <gui/instance.h>
#include <gui/localization.h>

#include <synfig/general.h>
#include <synfig/valuenode.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static HistoryTreeStore::Model& ModelHack()
{
	static HistoryTreeStore::Model* model(nullptr);
	if(!model)model=new HistoryTreeStore::Model;
	return *model;
}

HistoryTreeStore::HistoryTreeStore(std::shared_ptr<studio::Instance> instance_):
	Gtk::TreeStore	(ModelHack()),
	instance_		(instance_),
	next_action_iter (children().end())
{
	instance_->signal_undo().connect(sigc::mem_fun(*this,&studio::HistoryTreeStore::on_undo));
	instance_->signal_redo().connect(sigc::mem_fun(*this,&studio::HistoryTreeStore::on_redo));
	instance_->signal_undo_stack_cleared().connect(sigc::mem_fun(*this,&studio::HistoryTreeStore::on_undo_stack_cleared));
	instance_->signal_redo_stack_cleared().connect(sigc::mem_fun(*this,&studio::HistoryTreeStore::on_redo_stack_cleared));
	instance_->signal_new_action().connect(sigc::mem_fun(*this,&studio::HistoryTreeStore::on_new_action));
	instance_->signal_action_status_changed().connect(sigc::mem_fun(*this,&studio::HistoryTreeStore::on_action_status_changed));
}

HistoryTreeStore::~HistoryTreeStore()
{
	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("HistoryTreeStore::~HistoryTreeStore(): Deleted");
}

Glib::RefPtr<HistoryTreeStore>
HistoryTreeStore::create(std::shared_ptr<studio::Instance> instance_)
{
	return Glib::RefPtr<HistoryTreeStore>(new HistoryTreeStore(instance_));
}

void
HistoryTreeStore::rebuild()
{
	synfigapp::Action::Stack::const_iterator iter;

	clear();

	const synfigapp::Action::Stack& undo_stack = instance()->undo_action_stack();

	for (iter = undo_stack.begin(); iter != undo_stack.end(); ++iter)
		insert_action(*(prepend()), *iter, true, false);

	next_action_iter = children().end();

	const synfigapp::Action::Stack& redo_stack = instance()->redo_action_stack();

	for(iter = redo_stack.begin(); iter != redo_stack.end(); ++iter)
		insert_action(*(append()), *iter, false, true);

	signal_undo_tree_changed()();
}

void
HistoryTreeStore::insert_action(Gtk::TreeRow row,std::shared_ptr<synfigapp::Action::Undoable> action, bool is_undo, bool is_redo)
{
	assert(action);

	row[model.action] = action;
	row[model.name] = static_cast<Glib::ustring>(action->get_local_name());
	row[model.is_active] = action->is_active();
	row[model.is_undo] = is_undo;
	row[model.is_redo] = is_redo;

	synfigapp::Action::CanvasSpecific *specific_action;
	specific_action=dynamic_cast<synfigapp::Action::CanvasSpecific*>(action.get());
	if(specific_action)
	{
		row[model.canvas] = specific_action->get_canvas();
		row[model.canvas_id] = specific_action->get_canvas()->get_id();
	}

	std::shared_ptr<synfigapp::Action::Group> group;
	group=std::shared_ptr<synfigapp::Action::Group>::cast_dynamic(action);
	if(group)
	{
		synfigapp::Action::ActionList::const_iterator iter;
		for(iter=group->action_list().begin();iter!=group->action_list().end();++iter)
		{
			Gtk::TreeRow child_row = *(append(row.children()));
			insert_action(child_row,*iter,is_undo,is_redo);
		}
	}
}


void
HistoryTreeStore::on_undo()
{
	if (next_action_iter == children().begin())
		return;

	--next_action_iter;
	next_action_iter->set_value(model.is_redo, true);
	next_action_iter->set_value(model.is_undo, false);

	signal_undo_tree_changed()();
}

void
HistoryTreeStore::on_redo()
{
	if (next_action_iter == children().end())
		return;

	next_action_iter->set_value(model.is_redo, false);
	next_action_iter->set_value(model.is_undo, true);
	++next_action_iter;

	signal_undo_tree_changed()();
}

void
HistoryTreeStore::on_undo_stack_cleared()
{
	Gtk::TreeModel::Children children_(children());
	Gtk::TreeModel::Children::iterator iter = children_.begin();

	while(iter != children_.end())
	{
		Gtk::TreeModel::Row row = *iter;
		if(row[model.is_undo])
			iter = erase(iter);
		else
			break;
	}
	next_action_iter = iter;
}

void
HistoryTreeStore::on_redo_stack_cleared()
{
	Gtk::TreeModel::Children children_(children());
	Gtk::TreeModel::Children::iterator iter = next_action_iter;

	while(iter != children_.end())
	{
		Gtk::TreeModel::Row row = *iter;
		if(row[model.is_redo])
			iter = erase(iter);
		else {
			error(_("Action history seems to be corrupted!"));
			rebuild();
			break;
		}
	}
	next_action_iter = children_.end();
}

void
HistoryTreeStore::on_new_action(std::shared_ptr<synfigapp::Action::Undoable> action)
{
	Gtk::TreeRow row;

	row=*insert(next_action_iter);

	insert_action(row,action);

	next_action_iter = row;
	++next_action_iter;

	signal_undo_tree_changed()();
}

void
HistoryTreeStore::on_action_status_changed(std::shared_ptr<synfigapp::Action::Undoable> action)
{
	Gtk::TreeModel::Children::iterator iter;
	Gtk::TreeModel::Children children_(children());

	for(iter=children_.begin(); iter != children_.end(); ++iter)
	{
		Gtk::TreeModel::Row row = *iter;
		if(action == row.get_value(model.action))
		{
			row[model.is_active]=action->is_active();
			return;
		}
	}
}

bool
HistoryTreeStore::search_func(const Glib::RefPtr<Gtk::TreeModel>&,int,const Glib::ustring& x,const Gtk::TreeModel::iterator& iter)
{
	const Model model;

	Glib::ustring substr(x.uppercase());
	Glib::ustring name((*iter)[model.name]);
	name=name.uppercase();

	return name.find(substr)==Glib::ustring::npos;
}
