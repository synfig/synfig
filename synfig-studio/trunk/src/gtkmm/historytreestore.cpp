/* === S I N F G =========================================================== */
/*!	\file historytreestore.cpp
**	\brief Template File
**
**	$Id: historytreestore.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "historytreestore.h"
#include <sinfg/valuenode.h>
#include "iconcontroler.h"
#include <sinfg/valuenode_timedswap.h>
#include <gtkmm/button.h>
#include <sinfgapp/action.h>
#include "instance.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static HistoryTreeStore::Model& ModelHack()
{
	static HistoryTreeStore::Model* model(0);
	if(!model)model=new HistoryTreeStore::Model;
	return *model;
}

HistoryTreeStore::HistoryTreeStore(etl::loose_handle<studio::Instance> instance_):
	Gtk::TreeStore	(ModelHack()),
	instance_		(instance_)
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
	sinfg::info("HistoryTreeStore::~HistoryTreeStore(): Deleted");
}

Glib::RefPtr<HistoryTreeStore>
HistoryTreeStore::create(etl::loose_handle<studio::Instance> instance_)
{
	return Glib::RefPtr<HistoryTreeStore>(new HistoryTreeStore(instance_));
}

void
HistoryTreeStore::rebuild()
{
	sinfgapp::Action::Stack::const_iterator iter;
	
	clear();
	
	for(iter=instance()->undo_action_stack().begin();iter!=instance()->undo_action_stack().end();++iter)
	{
		insert_action(*(prepend()),*iter,true,true,false);	
	}
	curr_row=*children().end();
	for(iter=instance()->redo_action_stack().begin();iter!=instance()->redo_action_stack().end();++iter)
	{
		insert_action(*(append()),*iter,true,false,true);	
	}		
}

void
HistoryTreeStore::insert_action(Gtk::TreeRow row,etl::handle<sinfgapp::Action::Undoable> action, bool is_active, bool is_undo, bool is_redo)
{
	assert(action);

	row[model.action] = action;
	row[model.name] = static_cast<Glib::ustring>(action->get_local_name());
	row[model.is_active] = action->is_active();
	row[model.is_undo] = is_undo;
	row[model.is_redo] = is_redo;
	
	sinfgapp::Action::CanvasSpecific *specific_action;
	specific_action=dynamic_cast<sinfgapp::Action::CanvasSpecific*>(action.get());
	if(specific_action)
	{
		row[model.canvas] = specific_action->get_canvas();
		row[model.canvas_id] = specific_action->get_canvas()->get_id();		
	}

	etl::handle<sinfgapp::Action::Group> group;
	group=etl::handle<sinfgapp::Action::Group>::cast_dynamic(action);
	if(group)
	{
		sinfgapp::Action::ActionList::const_iterator iter;
		for(iter=group->action_list().begin();iter!=group->action_list().end();++iter)
		{
			Gtk::TreeRow child_row = *(append(row.children()));
			insert_action(child_row,*iter,true,is_undo,is_redo);
		}
	}
	
	//row[model.icon] = Gtk::Button().render_icon(Gtk::StockID("sinfg-canvas"),Gtk::ICON_SIZE_SMALL_TOOLBAR);	
}


void
HistoryTreeStore::on_undo()
{
	refresh();
}

void
HistoryTreeStore::on_redo()
{
	refresh();
}

void
HistoryTreeStore::on_undo_stack_cleared()
{
	Gtk::TreeModel::Children::iterator iter,next;
	Gtk::TreeModel::Children children_(children());
	
	for(next=children_.begin(),iter=next++; iter != children_.end(); iter=(next!=children_.end())?next++:next)
	{
		Gtk::TreeModel::Row row = *iter;
		if(row[model.is_undo])
			erase(iter);
	}
}

void
HistoryTreeStore::on_redo_stack_cleared()
{
	Gtk::TreeModel::Children::iterator iter,next;
	Gtk::TreeModel::Children children_(children());
	
	for(next=children_.begin(),iter=next++; iter != children_.end(); iter=(next!=children_.end())?next++:next)
	{
		Gtk::TreeModel::Row row = *iter;
		if(row[model.is_redo])
			erase(iter);
	}
}

void
HistoryTreeStore::on_new_action(etl::handle<sinfgapp::Action::Undoable> action)
{
//	Gtk::TreeRow row = *(append());
	Gtk::TreeRow row;
	Gtk::TreeModel::Children::iterator iter;
	for(iter=children().begin(); iter != children().end(); ++iter)
	{
		Gtk::TreeModel::Row row = *iter;
		if(row[model.is_redo])
		{
			break;
		}
	}

	row=*insert(iter);

	insert_action(row,action);
}

void
HistoryTreeStore::on_action_status_changed(etl::handle<sinfgapp::Action::Undoable> action)
{
	Gtk::TreeModel::Children::iterator iter;
	Gtk::TreeModel::Children children_(children());
	
	for(iter=children_.begin(); iter != children_.end(); ++iter)
	{
		Gtk::TreeModel::Row row = *iter;
		if(action == (etl::handle<sinfgapp::Action::Undoable>)row[model.action])
		{
			row[model.is_active]=action->is_active();
			return;
		}
	}	
}
