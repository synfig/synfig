/* === S Y N F I G ========================================================= */
/*!	\file gtkmm/instance.cpp
**	\brief writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "instance.h"
#include <cassert>
#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <iostream>
#include <gtkmm/button.h>
#include "canvasview.h"
#include "app.h"
#include <sigc++/signal.h>
#include <sigc++/adaptors/hide.h>
#include "toolbox.h"
#include "onemoment.h"

#include "autorecover.h"
#include <sigc++/retype_return.h>
#include <sigc++/retype.h>
//#include <sigc++/hide.h>
#include <synfig/valuenode_composite.h>
#include "widget_waypointmodel.h"
#include <gtkmm/actiongroup.h>
#include "iconcontroller.h"
#include <sys/stat.h>
#include <errno.h>

#include "general.h"

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;
using namespace SigC;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

int studio::Instance::instance_count_=0;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Instance::Instance(Canvas::Handle canvas):
	synfigapp::Instance		(canvas),
//	canvas_tree_store_		(Gtk::TreeStore::create(CanvasTreeModel())),
//	canvas_tree_store_		(Gtk::TreeStore::create()),
	history_tree_store_		(HistoryTreeStore::create(this)),
	undo_status_(false),
	redo_status_(false)
{
	CanvasTreeModel model;
	canvas_tree_store_=Gtk::TreeStore::create(model);

	id_=instance_count_++;

	// Connect up all the signals
	signal_filename_changed().connect(sigc::mem_fun(*this,&studio::Instance::update_all_titles));
	signal_unsaved_status_changed().connect(sigc::hide(sigc::mem_fun(*this,&studio::Instance::update_all_titles)));
	signal_undo_status().connect(sigc::mem_fun(*this,&studio::Instance::set_undo_status));
	signal_redo_status().connect(sigc::mem_fun(*this,&studio::Instance::set_redo_status));

	signal_saved().connect(
		sigc::hide_return(
			sigc::ptr_fun(
				studio::AutoRecover::auto_backup
			)
		)
	);

	canvas_tree_store_=Gtk::TreeStore::create(canvas_tree_model);

	refresh_canvas_tree();
}

Instance::~Instance()
{
}

int
Instance::get_visible_canvases()const
{
	int count(0);
	CanvasViewList::const_iterator iter;
	for(iter=canvas_view_list_.begin();iter!=canvas_view_list_.end();++iter)
		if((*iter)->is_visible())
			count++;
	return count;
}

handle<Instance>
Instance::create(Canvas::Handle canvas)
{
	// Construct a new instance
	handle<Instance> instance(new Instance(canvas));

	// Add the new instance to the application's instance list
	App::instance_list.push_back(instance);

	// Set up the instance with the default UI manager
	instance->synfigapp::Instance::set_ui_interface(App::get_ui_interface());

	// Signal the new instance
	App::signal_instance_created()(instance);

	// And then make sure that is has been selected
	App::set_selected_instance(instance);

	// Create the initial window for the root canvas
	instance->focus(canvas);

	return instance;
}

handle<CanvasView>
Instance::find_canvas_view(Canvas::Handle canvas)
{
	if(!canvas)
		return 0;

	while(canvas->is_inline())
		canvas=canvas->parent();

	CanvasViewList::iterator iter;

	for(iter=canvas_view_list().begin();iter!=canvas_view_list().end();iter++)
		if((*iter)->get_canvas()==canvas)
			return *iter;

	return CanvasView::create(this,canvas);
}

void
Instance::focus(Canvas::Handle canvas)
{
	handle<CanvasView> canvas_view=find_canvas_view(canvas);
	assert(canvas_view);
	canvas_view->present();
}

void
Instance::set_undo_status(bool x)
{
	undo_status_=x;
	App::toolbox->update_undo_redo();
	signal_undo_redo_status_changed()();
}

void
Instance::set_redo_status(bool x)
{
	redo_status_=x;
	App::toolbox->update_undo_redo();
	signal_undo_redo_status_changed()();
}

bool
studio::Instance::save_as(const synfig::String &file_name)
{
	if(synfigapp::Instance::save_as(file_name))
	{
		// after changing the filename, update the render settings with the new filename
		list<handle<CanvasView> >::iterator iter;
		for(iter=canvas_view_list().begin();iter!=canvas_view_list().end();iter++)
			(*iter)->render_settings.set_entry_filename();
		App::add_recent_file(file_name);
		return true;
	}
	return false;
}

Instance::Status
studio::Instance::save()
{
	// the filename will be set to "Synfig Animation 1" or some such when first created
	// and will be changed to an absolute path once it has been saved
	// so if it still begins with "Synfig Animation " then we need to ask where to save it
	if(get_file_name().find(DEFAULT_FILENAME_PREFIX)==0)
		if (dialog_save_as())
			return STATUS_OK;
		else
			return STATUS_CANCEL;

	if (synfigapp::Instance::save())
		return STATUS_OK;

	App::dialog_error_blocking("Save - Error","Unable to save to '" + get_file_name() + "'");
	return STATUS_ERROR;
}

bool
studio::Instance::dialog_save_as()
{
	string filename=basename(get_file_name());
	Canvas::Handle canvas(get_canvas());

	{
		OneMoment one_moment;
		std::set<Node*>::iterator iter;
		for(iter=canvas->parent_set.begin();iter!=canvas->parent_set.end();++iter)
		{
			synfig::Node* node(*iter);
			for(;!node->parent_set.empty();node=*node->parent_set.begin())
			{
				Layer::Handle parent_layer(dynamic_cast<Layer*>(node));
				if(parent_layer && parent_layer->get_canvas()->get_root()!=get_canvas())
				{
					App::dialog_error_blocking("SaveAs - Error",
						"There is currently a bug when using \"SaveAs\"\n"
						"on a composition that is being referenced by other\n"
						"files that are currently open. Close these\n"
						"other files first before trying to use \"SaveAs\"."
					);

					return false;
				}
				if(parent_layer)
					break;
			}
		}
	}

	// show the canvas' name if it has one, else its ID
	while(App::dialog_save_file(_("Choose a Filename to Save As") +
								String(" (") +
								(canvas->get_name().empty()
								 ? canvas->get_id()
								 : canvas->get_name()) +
								") ...", filename))
	{
		// If the filename still has wildcards, then we should
		// continue looking for the file we want
		if(find(filename.begin(),filename.end(),'*')!=filename.end())
			continue;

		if (filename_extension(filename) == "")
			filename+=".sifz";

		try
		{
			String ext(filename_extension(filename));
			if(ext!=".sif" && ext!=".sifz" && !App::dialog_yes_no(_("Unknown extension"),
				_("You have given the file name an extension\nwhich I do not recognize. Are you sure this is what you want?")))
				continue;
		}
		catch(...)
		{
			continue;
		}

		{
			struct stat	s;
			int stat_return = stat(filename.c_str(), &s);

			// if stat() fails with something other than 'file doesn't exist', there's been a real
			// error of some kind.  let's give up now and ask for a new path.
			if (stat_return == -1 && errno != ENOENT)
			{
				perror(filename.c_str());
				App::dialog_error_blocking("SaveAs - Error","Unable to check whether '" + filename + "' exists.");
				continue;
			}

			// if the file exists and the user doesn't want to overwrite it, keep prompting for a filename
			if ((stat_return == 0) &&
				!App::dialog_yes_no("File exists",
									"A file named '" +
									filename +
									"' already exists.\n\n"
									"Do you want to replace it with the file you are saving?"))
				continue;
		}

		if(save_as(filename))
			return true;

		App::dialog_error_blocking("SaveAs - Error","Unable to save to '" + filename + "'");
	}

	return false;
}

void
Instance::update_all_titles()
{
	list<handle<CanvasView> >::iterator iter;
	for(iter=canvas_view_list().begin();iter!=canvas_view_list().end();iter++)
		(*iter)->update_title();
}

void
Instance::close()
{
	// This will increase the reference count so we don't get DELETED
	// until we are ready
	handle<Instance> me(this);

	// Make sure we aren't selected as the current instance
	if(studio::App::get_selected_instance()==this)
		studio::App::set_selected_instance(0);

	// Turn-off/clean-up auto recovery
	studio::App::auto_recover->clear_backup(get_canvas());

	// Remove us from the active instance list
	std::list<etl::handle<studio::Instance> >::iterator iter;
	for(iter=studio::App::instance_list.begin();iter!=studio::App::instance_list.end();iter++)
		if(*iter==this)
			break;
	assert(iter!=studio::App::instance_list.end());
	if(iter!=studio::App::instance_list.end())
		studio::App::instance_list.erase(iter);

	// Send out a signal that we are being deleted
	studio::App::signal_instance_deleted()(this);

	// Hide all of the canvas views
	for(std::list<etl::handle<CanvasView> >::iterator iter=canvas_view_list().begin();iter!=canvas_view_list().end();iter++)
		(*iter)->hide();

	// Consume pending events before deleting the canvas views
	while(studio::App::events_pending())studio::App::iteration(false);

	// Delete all of the canvas views
	canvas_view_list().clear();

	// If there is another open instance to select,
	// go ahead and do so. If not, never mind.
	if(studio::App::instance_list.empty())
	{
		studio::App::set_selected_canvas_view(0);
		studio::App::set_selected_instance(0);
	}
	else
		studio::App::instance_list.front()->canvas_view_list().front()->present();
}


void
Instance::insert_canvas(Gtk::TreeRow row,Canvas::Handle canvas)
{
	CanvasTreeModel canvas_tree_model;
	assert(canvas);

	row[canvas_tree_model.icon] = Gtk::Button().render_icon(Gtk::StockID("synfig-canvas"),Gtk::ICON_SIZE_SMALL_TOOLBAR);
	row[canvas_tree_model.id] = canvas->get_id();
	row[canvas_tree_model.name] = canvas->get_name();
	if(canvas->is_root())
		row[canvas_tree_model.label] = basename(canvas->get_file_name());
	else
	if(!canvas->get_id().empty())
		row[canvas_tree_model.label] = canvas->get_id();
	else
	if(!canvas->get_name().empty())
		row[canvas_tree_model.label] = canvas->get_name();
	else
		row[canvas_tree_model.label] = _("[Unnamed]");

	row[canvas_tree_model.canvas] = canvas;
	row[canvas_tree_model.is_canvas] = true;
	row[canvas_tree_model.is_value_node] = false;

	{
		synfig::Canvas::Children::iterator iter;
		synfig::Canvas::Children &children(canvas->children());

		for(iter=children.begin();iter!=children.end();iter++)
			insert_canvas(*(canvas_tree_store()->append(row.children())),*iter);
	}

	/*
	if(!canvas->value_node_list().empty())
	{
		Gtk::TreeRow valuenode_row = *(canvas_tree_store()->append(row.children()));

		valuenode_row[canvas_tree_model.label] = "<defs>";
		valuenode_row[canvas_tree_model.canvas] = canvas;
		valuenode_row[canvas_tree_model.is_canvas] = false;
		valuenode_row[canvas_tree_model.is_value_node] = false;

		synfig::ValueNodeList::iterator iter;
		synfig::ValueNodeList &value_node_list(canvas->value_node_list());

		for(iter=value_node_list.begin();iter!=value_node_list.end();iter++)
			insert_value_node(*(canvas_tree_store()->append(valuenode_row.children())),canvas,*iter);
	}
	*/
}


/*
void
Instance::insert_value_node(Gtk::TreeRow row,Canvas::Handle canvas,etl::handle<synfig::ValueNode> value_node)
{
	CanvasTreeModel canvas_tree_model;
	assert(value_node);
	assert(canvas);

	row[canvas_tree_model.id] = value_node->get_id();
	row[canvas_tree_model.name] = value_node->get_id();
	row[canvas_tree_model.label] = value_node->get_id();
	row[canvas_tree_model.canvas] = canvas;
	row[canvas_tree_model.value_node] = value_node;
	row[canvas_tree_model.is_canvas] = false;
	row[canvas_tree_model.is_value_node] = true;
	row[canvas_tree_model.icon] = Gtk::Button().render_icon(valuenode_icon(value_node),Gtk::ICON_SIZE_SMALL_TOOLBAR);
}
*/

void
Instance::refresh_canvas_tree()
{
	canvas_tree_store()->clear();
	Gtk::TreeRow row = *(canvas_tree_store()->prepend());
	insert_canvas(row,get_canvas());
}

void
Instance::dialog_cvs_commit()
{
	calc_repository_info();
	if(!in_repository())
	{
		App::dialog_error_blocking(_("Error"),_("You must first add this composition to the repository"));
		return;
	}
	try
	{
		string message;

		if(synfigapp::Instance::get_action_count())
		{
			if(!App::dialog_yes_no(_("CVS Commit"), _("This will save any changes you have made. Are you sure?")))
				return;
			save();
		}

		if(!is_modified())
		{
			App::dialog_error_blocking(_("Error"),_("The local copy of the file hasn't been changed since the last update.\nNothing to commit!"));
			return;
		}

		if(!App::dialog_entry(_("CVS Commit"),_("Enter a log message describing the changes you have made"), message))
			return;

		OneMoment one_moment;
		cvs_commit(message);
	}
	catch(...)
	{
		App::dialog_error_blocking(_("Error"),_("An error has occurred when trying to COMMIT"));
	}
	update_all_titles();
}

void
Instance::dialog_cvs_add()
{
	calc_repository_info();
	if(in_repository())
	{
		App::dialog_error_blocking(_("Error"),_("This composition has already been added to the repository"));
		return;
	}
	try
	{
		string message;

		//if(!App::dialog_entry(_("CVS Add"),_("Enter a log message describing the file"), message))
		//	return;
		OneMoment one_moment;
		cvs_add();
	}
	catch(...)
	{
		App::dialog_error_blocking(_("Error"),_("An error has occurred when trying to ADD"));
	}
	update_all_titles();
}

void
Instance::dialog_cvs_update()
{
	calc_repository_info();
	if(!in_repository())
	{
		App::dialog_error_blocking(_("Error"),_("This file is not under version control, so there is nothing to update from!"));
		return;
	}
	if(!is_updated())
	{
		App::dialog_error_blocking(_("Info"),_("This file is up-to-date"));
		return;
	}

	try
	{
		String filename(get_file_name());
		if(synfigapp::Instance::get_action_count())
		{
			if(!App::dialog_yes_no(_("CVS Update"), _("This will save any changes you have made. Are you sure?")))
				return;
			save();
		}
		OneMoment one_moment;
		time_t oldtime=get_original_timestamp();
		cvs_update();
		calc_repository_info();
		// If something has been updated...
		if(oldtime!=get_original_timestamp())
		{
			revert();
		}
	}
	catch(...)
	{
		App::dialog_error_blocking(_("Error"),_("An error has occurred when trying to UPDATE"));
	}
	//update_all_titles();
}

void
Instance::dialog_cvs_revert()
{
	calc_repository_info();
	if(!in_repository())
	{
		App::dialog_error_blocking(_("Error"),_("This file is not under version control, so there is nothing to revert to!"));
		return;
	}
	try
	{
		String filename(get_file_name());
		if(!App::dialog_yes_no(_("CVS Revert"),
			_("This will abandon all changes you have made\nsince the last time you performed a commit\noperation. This cannot be undone! Are you sure\nyou want to do this?")
		))
			return;

		OneMoment one_moment;

		// Remove the old file
		if(remove(get_file_name().c_str())!=0)
		{
			App::dialog_error_blocking(_("Error"),_("Unable to remove previous version"));
			return;
		}

		cvs_update();
		revert();
	}
	catch(...)
	{
		App::dialog_error_blocking(_("Error"),_("An error has occurred when trying to UPDATE"));
	}
	//update_all_titles();
}

void
Instance::_revert(Instance *instance)
{
	OneMoment one_moment;

	String filename(instance->get_file_name());

	Canvas::Handle canvas(instance->get_canvas());

	instance->close();

	if(canvas->count()!=1)
	{
		one_moment.hide();
		App::dialog_error_blocking(_("Error: Revert Failed"),_("The revert operation has failed. This can be due to it being\nreferenced by another composition that is already open, or\nbecause of an internal error in Synfig Studio. Try closing any\ncompositions that might reference this composition and try\nagain, or restart Synfig Studio."));
		one_moment.show();
	}
	canvas=0;

	App::open(filename);
}

void
Instance::revert()
{
	// Schedule a revert to occur in a few moments
	Glib::signal_timeout().connect(
		sigc::bind_return(
			sigc::bind(
				sigc::ptr_fun(&Instance::_revert),
				this
			),
			false
		)
		,500
	);
}

bool
Instance::safe_revert()
{
	if(synfigapp::Instance::get_action_count())
		if(!App::dialog_yes_no(_("Revert to saved"), _("You will lose any changes you have made since your last save.\nAre you sure?")))
			return false;
	revert();
	return true;
}

bool
Instance::safe_close()
{
	handle<CanvasView> canvas_view = find_canvas_view(get_canvas());
	handle<synfigapp::UIInterface> uim=canvas_view->get_ui_interface();

	// if the animation is currently playing, closing the window will cause a crash,
	// so don't allow it
	if (canvas_view->is_playing())
	{
		canvas_view->present();
		App::dialog_error_blocking("Close Error", "The animation is currently playing so the window cannot be closed.");
		return false;
	}
	if(get_action_count())
		do
		{
			string str=strprintf(_("Would you like to save your changes to %s?"),basename(get_file_name()).c_str() );
			int answer=uim->yes_no_cancel(get_canvas()->get_name(),str,synfigapp::UIInterface::RESPONSE_YES);
			if(answer==synfigapp::UIInterface::RESPONSE_YES)
			{
				enum Status status = save();
				if (status == STATUS_OK) break;
				else if (status == STATUS_CANCEL) return false;
			}
			if(answer==synfigapp::UIInterface::RESPONSE_NO)
				break;
			if(answer==synfigapp::UIInterface::RESPONSE_CANCEL)
				return false;
		} while (true);

	if(is_modified())
	{
		string str=strprintf(_("%s has changes not yet on the CVS repository.\nWould you like to commit these changes?"),basename(get_file_name()).c_str());
		int answer=uim->yes_no_cancel(get_canvas()->get_name(),str,synfigapp::UIInterface::RESPONSE_YES);

		if(answer==synfigapp::UIInterface::RESPONSE_YES)
			dialog_cvs_commit();
		if(answer==synfigapp::UIInterface::RESPONSE_CANCEL)
			return false;
	}

	close();

	return true;
}


void
Instance::add_actions_to_group(const Glib::RefPtr<Gtk::ActionGroup>& action_group, synfig::String& ui_info,   const synfigapp::Action::ParamList &param_list, synfigapp::Action::Category category)const
{
	synfigapp::Action::CandidateList candidate_list;
	synfigapp::Action::CandidateList::iterator iter;

	candidate_list=compile_candidate_list(param_list,category);

	candidate_list.sort();

	if(candidate_list.empty())
		synfig::warning("%s:%d Action CandidateList is empty!", __FILE__, __LINE__);

	for(iter=candidate_list.begin();iter!=candidate_list.end();++iter)
	{
		Gtk::StockID stock_id(get_action_stock_id(*iter));

		if(!(iter->category&synfigapp::Action::CATEGORY_HIDDEN))
		{
			action_group->add(Gtk::Action::create(
				"action-"+iter->name,
				stock_id,
				iter->local_name,iter->local_name
			),
				sigc::bind(
					sigc::bind(
						sigc::mem_fun(
							*const_cast<studio::Instance*>(this),
							&studio::Instance::process_action
						),
						param_list
					),
					iter->name
				)
			);
			ui_info+=strprintf("<menuitem action='action-%s' />",iter->name.c_str());
		}
	}
}

void
Instance::add_actions_to_menu(Gtk::Menu *menu, const synfigapp::Action::ParamList &param_list,synfigapp::Action::Category category)const
{
	synfigapp::Action::CandidateList candidate_list;
	synfigapp::Action::CandidateList::iterator iter;

	candidate_list=compile_candidate_list(param_list,category);

	candidate_list.sort();

	if(candidate_list.empty())
		synfig::warning("%s:%d Action CandidateList is empty!", __FILE__, __LINE__);

	for(iter=candidate_list.begin();iter!=candidate_list.end();++iter)
	{
		if(!(iter->category&synfigapp::Action::CATEGORY_HIDDEN))
		{
			Gtk::Image* image(manage(new Gtk::Image()));
			Gtk::Stock::lookup(get_action_stock_id(*iter),Gtk::ICON_SIZE_MENU,*image);

			/*
			if(iter->task=="raise")
				Gtk::Stock::lookup(Gtk::Stock::GO_UP,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="lower")
				Gtk::Stock::lookup(Gtk::Stock::GO_DOWN,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="move_top")
				Gtk::Stock::lookup(Gtk::Stock::GOTO_TOP,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="move_bottom")
				Gtk::Stock::lookup(Gtk::Stock::GOTO_BOTTOM,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="remove")
				Gtk::Stock::lookup(Gtk::Stock::DELETE,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="set_on")
				Gtk::Stock::lookup(Gtk::Stock::YES,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="set_off")
				Gtk::Stock::lookup(Gtk::Stock::NO,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="duplicate")
				Gtk::Stock::lookup(Gtk::Stock::COPY,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="remove")
				Gtk::Stock::lookup(Gtk::Stock::DELETE,Gtk::ICON_SIZE_MENU,*image);
			else
			{
				Gtk::Stock::lookup(Gtk::StockID("synfig-"+iter->name),Gtk::ICON_SIZE_MENU,*image) ||
				Gtk::Stock::lookup(Gtk::StockID("gtk-"+iter->task),Gtk::ICON_SIZE_MENU,*image) ||
				Gtk::Stock::lookup(Gtk::StockID("synfig-"+iter->task),Gtk::ICON_SIZE_MENU,*image);
			}
			*/
			menu->items().push_back(
				Gtk::Menu_Helpers::ImageMenuElem(
					iter->local_name,
					*image,
					sigc::bind(
						sigc::bind(
							sigc::mem_fun(
								*const_cast<studio::Instance*>(this),
								&studio::Instance::process_action
							),
							param_list
						),
						iter->name
					)
				)
			);
		}
	}
}

void
Instance::add_actions_to_menu(Gtk::Menu *menu, const synfigapp::Action::ParamList &param_list,const synfigapp::Action::ParamList &param_list2,synfigapp::Action::Category category)const
{
	synfigapp::Action::CandidateList candidate_list;
	synfigapp::Action::CandidateList candidate_list2;

	synfigapp::Action::CandidateList::iterator iter;

	candidate_list=compile_candidate_list(param_list,category);
	candidate_list2=compile_candidate_list(param_list2,category);

	candidate_list.sort();

	if(candidate_list.empty())
		synfig::warning("%s:%d Action CandidateList is empty!", __FILE__, __LINE__);
	if(candidate_list2.empty())
		synfig::warning("%s:%d Action CandidateList2 is empty!", __FILE__, __LINE__);

	// Separate out the candidate lists so that there are no conflicts
	for(iter=candidate_list.begin();iter!=candidate_list.end();++iter)
	{
		synfigapp::Action::CandidateList::iterator iter2(candidate_list2.find(iter->name));
		if(iter2!=candidate_list2.end())
			candidate_list2.erase(iter2);
	}

	for(iter=candidate_list2.begin();iter!=candidate_list2.end();++iter)
	{
		if(!(iter->category&synfigapp::Action::CATEGORY_HIDDEN))
		{
			Gtk::Image* image(manage(new Gtk::Image()));
			Gtk::Stock::lookup(get_action_stock_id(*iter),Gtk::ICON_SIZE_MENU,*image);

/*			if(iter->task=="raise")
				Gtk::Stock::lookup(Gtk::Stock::GO_UP,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="lower")
				Gtk::Stock::lookup(Gtk::Stock::GO_DOWN,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="move_top")
				Gtk::Stock::lookup(Gtk::Stock::GOTO_TOP,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="move_bottom")
				Gtk::Stock::lookup(Gtk::Stock::GOTO_BOTTOM,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="remove")
				Gtk::Stock::lookup(Gtk::Stock::DELETE,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="set_on")
				Gtk::Stock::lookup(Gtk::Stock::YES,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="set_off")
				Gtk::Stock::lookup(Gtk::Stock::NO,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="duplicate")
				Gtk::Stock::lookup(Gtk::Stock::COPY,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="remove")
				Gtk::Stock::lookup(Gtk::Stock::DELETE,Gtk::ICON_SIZE_MENU,*image);
			else
			{
				Gtk::Stock::lookup(Gtk::StockID("synfig-"+iter->name),Gtk::ICON_SIZE_MENU,*image) ||
				Gtk::Stock::lookup(Gtk::StockID("gtk-"+iter->task),Gtk::ICON_SIZE_MENU,*image) ||
				Gtk::Stock::lookup(Gtk::StockID("synfig-"+iter->task),Gtk::ICON_SIZE_MENU,*image);
			}
*/
			menu->items().push_back(
				Gtk::Menu_Helpers::ImageMenuElem(
					iter->local_name,
					*image,
					sigc::bind(
						sigc::bind(
							sigc::mem_fun(
								*const_cast<studio::Instance*>(this),
								&studio::Instance::process_action
							),
							param_list2
						),
						iter->name
					)
				)
			);
		}
	}

	for(iter=candidate_list.begin();iter!=candidate_list.end();++iter)
	{
		if(!(iter->category&synfigapp::Action::CATEGORY_HIDDEN))
		{
			Gtk::Image* image(manage(new Gtk::Image()));
			Gtk::Stock::lookup(get_action_stock_id(*iter),Gtk::ICON_SIZE_MENU,*image);
/*			if(iter->task=="raise")
				Gtk::Stock::lookup(Gtk::Stock::GO_UP,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="lower")
				Gtk::Stock::lookup(Gtk::Stock::GO_DOWN,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="move_top")
				Gtk::Stock::lookup(Gtk::Stock::GOTO_TOP,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="move_bottom")
				Gtk::Stock::lookup(Gtk::Stock::GOTO_BOTTOM,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="remove")
				Gtk::Stock::lookup(Gtk::Stock::DELETE,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="set_on")
				Gtk::Stock::lookup(Gtk::Stock::YES,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="set_off")
				Gtk::Stock::lookup(Gtk::Stock::NO,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="duplicate")
				Gtk::Stock::lookup(Gtk::Stock::COPY,Gtk::ICON_SIZE_MENU,*image);
			else if(iter->task=="remove")
				Gtk::Stock::lookup(Gtk::Stock::DELETE,Gtk::ICON_SIZE_MENU,*image);
			else
			{
				Gtk::Stock::lookup(Gtk::StockID("synfig-"+iter->name),Gtk::ICON_SIZE_MENU,*image) ||
				Gtk::Stock::lookup(Gtk::StockID("gtk-"+iter->task),Gtk::ICON_SIZE_MENU,*image) ||
				Gtk::Stock::lookup(Gtk::StockID("synfig-"+iter->task),Gtk::ICON_SIZE_MENU,*image);
			}
*/
			menu->items().push_back(
				Gtk::Menu_Helpers::ImageMenuElem(
					iter->local_name,
					*image,
					sigc::bind(
						sigc::bind(
							sigc::mem_fun(
								*const_cast<studio::Instance*>(this),
								&studio::Instance::process_action
							),
							param_list
						),
						iter->name
					)
				)
			);
		}
	}
}

void
Instance::process_action(String name, synfigapp::Action::ParamList param_list)
{
	assert(synfigapp::Action::book().count(name));

	synfigapp::Action::BookEntry entry(synfigapp::Action::book().find(name)->second);

	synfigapp::Action::Handle action(entry.factory());

	if(!action)
	{
		synfig::error("Bad Action");
		return;
	}

	action->set_param_list(param_list);

	synfigapp::Action::ParamVocab param_vocab(entry.get_param_vocab());
	synfigapp::Action::ParamVocab::const_iterator iter;

	for(iter=param_vocab.begin();iter!=param_vocab.end();++iter)
	{
		if(!iter->get_mutual_exclusion().empty() && param_list.count(iter->get_mutual_exclusion()))
			continue;

		// If the parameter is optionally user-supplied,
		// and has not been already provided in the param_list,
		// then we should go ahead and see if we can
		// provide that data.
		if(iter->get_user_supplied() && param_list.count(iter->get_name())==0)
		{
			switch(iter->get_type())
			{
			case synfigapp::Action::Param::TYPE_STRING:
			{
				String str;
				if(!studio::App::dialog_entry(entry.local_name, iter->get_local_name()+":"+iter->get_desc(),str))
					return;
				action->set_param(iter->get_name(),str);
				break;
			}
			default:
				synfig::error("Unsupported user-supplied action parameter");
				return;
				break;
			}
		}
	}

	if(!action->is_ready())
	{
		synfig::error("Action not ready");
		return;
	}

	perform_action(action);
}

void
Instance::make_param_menu(Gtk::Menu *menu,synfig::Canvas::Handle canvas, synfigapp::ValueDesc value_desc, float location)
{
	Gtk::Menu& parammenu(*menu);

	etl::handle<synfigapp::CanvasInterface> canvas_interface(find_canvas_interface(canvas));

	if(!canvas_interface)
		return;

	synfigapp::Action::ParamList param_list,param_list2;
	param_list=canvas_interface->generate_param_list(value_desc);
	param_list.add("origin",location);

	if(value_desc.get_value_type()==ValueBase::TYPE_BLINEPOINT && value_desc.is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		param_list2=canvas_interface->generate_param_list(
			synfigapp::ValueDesc(
				ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node())
				,0
			)
		);
		param_list2.add("origin",location);
	}


	// Populate the convert menu by looping through
	// the ValueNode book and find the ones that are
	// relevant.
	{
		Gtk::Menu *convert_menu=manage(new Gtk::Menu());
		LinkableValueNode::Book::const_iterator iter;
		for(iter=LinkableValueNode::book().begin();iter!=LinkableValueNode::book().end();++iter)
		{
			if(iter->second.check_type(value_desc.get_value_type()))
			{
				convert_menu->items().push_back(Gtk::Menu_Helpers::MenuElem(iter->second.local_name,
					sigc::hide_return(
						sigc::bind(
							sigc::bind(
								sigc::mem_fun(*canvas_interface.get(),&synfigapp::CanvasInterface::convert),
								iter->first
							),
							value_desc
						)
					)
				));
			}
		}

		parammenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::CONVERT,*convert_menu));
	}

	if(param_list2.empty())
		add_actions_to_menu(&parammenu, param_list,synfigapp::Action::CATEGORY_VALUEDESC|synfigapp::Action::CATEGORY_VALUENODE);
	else
		add_actions_to_menu(&parammenu, param_list2,param_list,synfigapp::Action::CATEGORY_VALUEDESC|synfigapp::Action::CATEGORY_VALUENODE);

	if(value_desc.get_value_type()==ValueBase::TYPE_BLINEPOINT && value_desc.is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		value_desc=synfigapp::ValueDesc(ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()),0);
	}

	if(value_desc.is_value_node() && ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueNode_Animated::Handle value_node(ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node()));

		try
		{
			WaypointList::iterator iter(value_node->find(canvas->get_time()));
			parammenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Edit Waypoint"),
				sigc::bind(
					sigc::bind(
						sigc::bind(
							sigc::mem_fun(*find_canvas_view(canvas),&studio::CanvasView::on_waypoint_clicked),
							-1
						),
						*iter
					),
					value_desc
				)
			));
		}
		catch(...)
		{
		}
	}
}

void
edit_several_waypoints(etl::handle<CanvasView> canvas_view, std::list<synfigapp::ValueDesc> value_desc_list)
{
	etl::handle<synfigapp::CanvasInterface> canvas_interface(canvas_view->canvas_interface());

	Gtk::Dialog dialog(
		"Edit Multiple Waypoints",		// Title
		true,		// Modal
		true		// use_separator
	);

	Widget_WaypointModel widget_waypoint_model;
	widget_waypoint_model.show();

	dialog.get_vbox()->pack_start(widget_waypoint_model);


	dialog.add_button(Gtk::StockID("gtk-apply"),1);
	dialog.add_button(Gtk::StockID("gtk-cancel"),0);
	dialog.show();

	DEBUGPOINT();
	if(dialog.run()==0 || widget_waypoint_model.get_waypoint_model().is_trivial())
		return;
	DEBUGPOINT();
	synfigapp::Action::PassiveGrouper group(canvas_interface->get_instance().get(),_("Set Waypoints"));

	std::list<synfigapp::ValueDesc>::iterator iter;
	for(iter=value_desc_list.begin();iter!=value_desc_list.end();++iter)
	{
		synfigapp::ValueDesc value_desc(*iter);

		if(!value_desc.is_valid())
			continue;

		ValueNode_Animated::Handle value_node;

		// If this value isn't a ValueNode_Animated, but
		// it is somewhat constant, then go ahead and convert
		// it to a ValueNode_Animated.
		if(!value_desc.is_value_node() || ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueBase value;
			if(value_desc.is_value_node())
				value=ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node())->get_value();
			else
				value=value_desc.get_value();

			value_node=ValueNode_Animated::create(value,canvas_interface->get_time());

			synfigapp::Action::Handle action;

			if(!value_desc.is_value_node())
			{
				action=synfigapp::Action::create("value_desc_connect");
				action->set_param("dest",value_desc);
				action->set_param("src",ValueNode::Handle(value_node));
			}
			else
			{
				action=synfigapp::Action::create("value_node_replace");
				action->set_param("dest",value_desc.get_value_node());
				action->set_param("src",ValueNode::Handle(value_node));
			}

			action->set_param("canvas",canvas_view->get_canvas());
			action->set_param("canvas_interface",canvas_interface);


			if(!canvas_interface->get_instance()->perform_action(action))
			{
				canvas_view->get_ui_interface()->error(_("Unable to convert to animated waypoint"));
				group.cancel();
				return;
			}
		}
		else
		{
			if(value_desc.is_value_node())
				value_node=ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node());
		}


		if(value_node)
		{

			synfigapp::Action::Handle action(synfigapp::Action::create("waypoint_set_smart"));

			if(!action)
			{
				canvas_view->get_ui_interface()->error(_("Unable to find waypoint_set_smart action"));
				group.cancel();
				return;
			}


			action->set_param("canvas",canvas_view->get_canvas());
			action->set_param("canvas_interface",canvas_interface);
			action->set_param("value_node",ValueNode::Handle(value_node));
			action->set_param("time",canvas_interface->get_time());
			action->set_param("model",widget_waypoint_model.get_waypoint_model());

			if(!canvas_interface->get_instance()->perform_action(action))
			{
				canvas_view->get_ui_interface()->error(_("Unable to set a specific waypoint"));
				group.cancel();
				return;
			}
		}
		else
		{
			//get_canvas_view()->get_ui_interface()->error(_("Unable to animate a specific valuedesc"));
			//group.cancel();
			//return;
		}

	}
}

void
Instance::make_param_menu(Gtk::Menu *menu,synfig::Canvas::Handle canvas,const std::list<synfigapp::ValueDesc>& value_desc_list)
{
	etl::handle<synfigapp::CanvasInterface> canvas_interface(find_canvas_interface(canvas));

	synfigapp::Action::ParamList param_list;
	param_list=canvas_interface->generate_param_list(value_desc_list);

	add_actions_to_menu(menu, param_list,synfigapp::Action::CATEGORY_VALUEDESC|synfigapp::Action::CATEGORY_VALUENODE);

	// Add the edit waypoints option if that might be useful
	if(canvas->rend_desc().get_time_end()-Time::epsilon()>canvas->rend_desc().get_time_start())
	{
		menu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Edit Waypoints"),
			sigc::bind(
				sigc::bind(
					sigc::ptr_fun(
						&edit_several_waypoints
					),
					value_desc_list
				),
				find_canvas_view(canvas)
			)
		));
	}
}
