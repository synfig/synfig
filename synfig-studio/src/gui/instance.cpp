/* === S Y N F I G ========================================================= */
/*!	\file gtkmm/instance.cpp
**	\brief writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2008, 2011 Carlos López
**	Copyright (c) 2009 Nikita Kitaev
**	Copyright (c) 2012 Konstantin Dmitriev
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
#include <gtkmm/menuitem.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/button.h>

#include <iostream>
#include "canvasview.h"
#include "app.h"
#include <sigc++/signal.h>
#include <sigc++/adaptors/hide.h>
#include "docks/dock_toolbox.h"
#include "onemoment.h"
#include <synfig/savecanvas.h>

#include "autorecover.h"
#include <sigc++/retype_return.h>
#include <sigc++/retype.h>
//#include <sigc++/hide.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/valuenodes/valuenode_duplicate.h>
#include "widgets/widget_waypointmodel.h"
#include <gtkmm/actiongroup.h>
#include "iconcontroller.h"
#include "workarea.h"
#include <sys/stat.h>
#include <errno.h>
#include <ETL/stringf>

#include "general.h"

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;
using namespace sigc;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

int studio::Instance::instance_count_=0;

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Instance::Instance(synfig::Canvas::Handle canvas, etl::handle< synfig::FileContainerTemporary > container):
	synfigapp::Instance		(canvas, container),
	canvas_tree_store_		(Gtk::TreeStore::create(canvas_tree_model)),
	history_tree_store_		(HistoryTreeStore::create(this)),
	undo_status_(false),
	redo_status_(false)
{
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
		if((*iter)->get_visible())
			count++;
	return count;
}

handle<Instance>
Instance::create(synfig::Canvas::Handle canvas, etl::handle< synfig::FileContainerTemporary > container)
{
	// Construct a new instance
	handle<Instance> instance(new Instance(canvas, container));

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
Instance::find_canvas_view(etl::handle<synfig::Canvas> canvas)
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
Instance::focus(etl::handle<synfig::Canvas> canvas)
{
	handle<CanvasView> canvas_view=find_canvas_view(canvas);
	assert(canvas_view);
	canvas_view->present();
}

void
Instance::set_undo_status(bool x)
{
	undo_status_=x;
	App::dock_toolbox->update_tools();
	signal_undo_redo_status_changed()();
}

void
Instance::set_redo_status(bool x)
{
	redo_status_=x;
	App::dock_toolbox->update_tools();
	signal_undo_redo_status_changed()();
}

void
studio::Instance::run_plugin(std::string plugin_path)
{
	handle<synfigapp::UIInterface> uim = this->find_canvas_view(this->get_canvas())->get_ui_interface();

	string message = strprintf(_("Do you realy want to add skeleton to document \"%s\"?" ),
				this->get_canvas()->get_name().c_str());

	string details = strprintf(_("This operation cannot be undone and all undo history will be cleared."));

	int answer = uim->confirmation(
				message,
				details,
				_("Cancel"),
				_("Proceed"),
				synfigapp::UIInterface::RESPONSE_OK);

	if(answer == synfigapp::UIInterface::RESPONSE_OK){
	
		OneMoment one_moment;

		Canvas::Handle canvas(this->get_canvas());
		
		synfigapp::PluginLauncher launcher(canvas);
		
		Time cur_time;
		cur_time = canvas->get_time();

		this->close();

		if(canvas->count()!=1)
		{
			one_moment.hide();
			App::dialog_message_1b(
					"ERROR",
					_("The plugin operation has failed."),
					_("This can be due to current file "
						"being referenced by another composition that is already open, "
						"or because of an internal error in Synfig Studio. Try closing "
						"any compositions that might reference this file and try again, "
						"or restart Synfig Studio."),
					_("Close"));

			one_moment.show();

		} else {
			bool result;
			result = launcher.execute( plugin_path, App::get_base_path() );
			if (!result){
				one_moment.hide();
				App::dialog_message_1b(
						"Error",
						launcher.get_output(),
						"details",
						_("Close"));

				one_moment.show();
				
			}
		}
	
	
		canvas=0;

		App::open_as(launcher.get_result_path(),launcher.get_original_path());
	
	
		etl::handle<Instance> new_instance = App::instance_list.back();
		new_instance->inc_action_count(); // This file isn't saved! mark it as such

		// Restore time cursor position
		canvas = App::instance_list.back()->get_canvas();
		etl::handle<synfigapp::CanvasInterface> new_canvas_interface(new_instance->find_canvas_interface(canvas));
		new_canvas_interface->set_time(cur_time);

	}
	return;
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
		App::add_recent_file(etl::handle<Instance>(this));
		return true;
	}
	return false;
}

void
studio::Instance::open()
{
	App::dialog_open(get_file_name());
}

Instance::Status
studio::Instance::save()
{
	// if we don't have a real filename yet then we need to ask where to save it
	if (!has_real_filename())
	{
		if (dialog_save_as())
			return STATUS_OK;
		else
			return STATUS_CANCEL;
	}

	if (synfigapp::Instance::save())
	{
		App::add_recent_file(etl::handle<Instance>(this));
		return STATUS_OK;
	}
	string msg(strprintf(_("Unable to save to '%s'"), get_file_name().c_str()));
	App::dialog_message_1b(
			"ERROR",
			msg.c_str(),
			"details",
			_("Close"));

	return STATUS_ERROR;
}

// the filename will be set to "Synfig Animation 1" or some such when first created
// and will be changed to an absolute path once it has been saved
// so if it still begins with "Synfig Animation " then we don't have a real filename yet
bool
studio::Instance::has_real_filename()
{
	return get_file_name().find(App::custom_filename_prefix.c_str()) != 0;
}

bool
studio::Instance::dialog_save_as()
{
	string filename = get_file_name();
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
					//! \todo Fix big involving "Save As" with referenced compositions
					string msg(strprintf(_("There is currently a bug when using \"SaveAs\"\n"
						"on a composition that is being referenced by other\n"
						"files that are currently open. Close these\n"
						"other files first before trying to use \"SaveAs\".")));
					App::dialog_message_1b(
							"ERROR",
							msg.c_str(),
							"details",
							_("Close"));

					return false;
				}
				if(parent_layer)
					break;
			}
		}
	}

	if (has_real_filename())
		filename = absolute_path(filename);

	// show the canvas' name if it has one, else its ID
	while (App::dialog_save_file((_("Please choose a file name") +
								  String(" (") +
								  (canvas->get_name().empty() ? canvas->get_id() : canvas->get_name()) +
								  ")"),
								 filename, ANIMATION_DIR_PREFERENCE))
	{
		// If the filename still has wildcards, then we should
		// continue looking for the file we want
		string base_filename = basename(filename);
		if (find(base_filename.begin(),base_filename.end(),'*')!=base_filename.end())
			continue;

		// if file extension is not recognized, then forced to .sifz
		if (filename_extension(filename) == "")
			filename+=".sifz";

		// forced to .sifz, the below code is not need anymore
		try
		{
			String ext(filename_extension(filename));
			// todo: ".sfg" literal and others
			if (ext != ".sif" && ext != ".sifz" && ext != ".sfg" && !App::dialog_message_2b(
				_("Unknown extension"),
				_("You have given the file name an extension which I do not recognize. "
					"Are you sure this is what you want?"),
				Gtk::MESSAGE_QUESTION,
				_("Cancel"),
				_("Sure"))
			)
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
				string msg(strprintf(_("Unable to check whether '%s' exists."), filename.c_str()));
				App::dialog_message_1b(
						"ERROR",
						msg.c_str(),
						"details",
						_("Close"));

				continue;
			}

			// If the file exists and the user doesn't want to overwrite it, keep prompting for a filename
			string message = strprintf(_("A file named \"%s\" already exists. "
							"Do you want to replace it?"),
						basename(filename).c_str());

			string details = strprintf(_("The file already exists in \"%s\". "
							"Replacing it will overwrite its contents."),
						basename(dirname(filename)).c_str());

			if ((stat_return == 0) && !App::dialog_message_2b(
				message,
				details,
				Gtk::MESSAGE_QUESTION,
				_("Use Another Name…"),
				_("Replace"))
			)
				continue;
		}

		if(save_as(filename))
		{
			synfig::set_file_version(ReleaseVersion(RELEASE_VERSION_END-1));
			return true;
		}
		string msg(strprintf(_("Unable to save to '%s'"), filename.c_str()));
		App::dialog_message_1b(
				"ERROR",
				msg.c_str(),
				"details",
				_("Close"));
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

	/*
	We need to hide some panels when instance is closed.
	This is done to avoid the crash when two conditions met:
	 1) the list is scrolled down
	 2) user closes file
	*/
	handle<CanvasView> canvas_view=find_canvas_view(get_canvas());
	Gtk::Widget* tree_view_keyframes = canvas_view->get_ext_widget("keyframes");
	tree_view_keyframes->hide();

	Gtk::Widget* tree_view_params = canvas_view->get_ext_widget("params");
	tree_view_params->hide();

	Gtk::Widget* tree_view_children = canvas_view->get_ext_widget("children");
	tree_view_children->hide();

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
	while(studio::App::events_pending())studio::App::iteration(true);

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
Instance::insert_canvas(Gtk::TreeRow row, synfig::Canvas::Handle canvas)
{
	CanvasTreeModel canvas_tree_model;
	assert(canvas);

	row[canvas_tree_model.icon] = Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-canvas"),Gtk::ICON_SIZE_SMALL_TOOLBAR);
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
}

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
		App::dialog_message_1b(
				"ERROR",
				_("You must first add this composition to the repository"),
				"details",
				_("Close"));

		return;
	}
	try
	{
		string message;

		if(synfigapp::Instance::get_action_count())
		{
			if (!App::dialog_message_2b(
				_("CVS Commit"),
				_("This will save any changes you have made. Are you sure?"),
				Gtk::MESSAGE_QUESTION,
				_("Cancel"),
				_("Commit"))
			)
				return;

			save();
		}

		if(!is_modified())
		{
			App::dialog_message_1b(
					"ERROR",
					_("The local copy of the file hasn't been changed since the last update. Nothing to commit!"),
					"details",
					_("Close"));

			return;
		}

		if(!App::dialog_entry(_("CVS Commit"),
				_("Log Message: "),
				message,
				_("Cancel"),
				_("Commit"))
		)
			return;

		OneMoment one_moment;
		cvs_commit(message);
	}
	catch(...)
	{
		App::dialog_message_1b(
				"ERROR",
				_("An error has occurred when trying to COMMIT"),
				"details",
				_("Close"));
	}
	update_all_titles();
}

void
Instance::dialog_cvs_add()
{
	calc_repository_info();
	if(in_repository())
	{
		App::dialog_message_1b(
				"ERROR",
				_("This composition has already been added to the repository"),
				"details",
				_("Close"));
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
		App::dialog_message_1b(
				"ERROR",
				_("An error has occurred when trying to ADD"),
				"details",
				_("Close"));
	}
	update_all_titles();
}

void
Instance::dialog_cvs_update()
{
	calc_repository_info();
	if(!in_repository())
	{
		App::dialog_message_1b(
				"ERROR",
				_("This file is not under version control, so there is nothing to update from!"),
				"details",
				_("Close"));

		return;
	}
	if(!is_updated())
	{
		App::dialog_message_1b(
				"INFO",
				_("This file is up-to-date"),
				"details",
				_("Close"));

		return;
	}

	try
	{
		String filename(get_file_name());
		if(synfigapp::Instance::get_action_count())
		{
			if (!App::dialog_message_2b(
				_("CVS Update"),
				_("This will save any changes you have made. Are you sure?"),
				Gtk::MESSAGE_QUESTION,
				_("Cancel"),
				_("Update"))
			)
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
		App::dialog_message_1b(
				"ERROR",
				_("An error has occurred when trying to UPDATE"),
				"details",
				_("Close"));
	}
	//update_all_titles();
}

void
Instance::dialog_cvs_revert()
{
	calc_repository_info();
	if(!in_repository())
{
		App::dialog_message_1b(
				"ERROR",
				_("This file is not under version control, so there is nothing to revert to!"),
				"details",
				_("Close"));
		return;
	}
	try
	{
		String filename(get_file_name());

		if (!App::dialog_message_2b(
			_("CVS Revert"),
			_("This will abandon all changes you have made since the last time you "
				"performed a commit operation. This cannot be undone! "
				"Are you sure you want to do this?"),
			Gtk::MESSAGE_QUESTION,
			_("Cancel"),
			_("Revert"))
		)
			return;

		OneMoment one_moment;

		// Remove the old file
		if(remove(get_file_name().c_str())!=0)
		{
			App::dialog_message_1b(
					"ERROR",
					_("Unable to remove previous version"),
					"details",
					_("Close"));

			return;
		}

		cvs_update();
		revert();
	}
	catch(...)
	{
		App::dialog_message_1b(
				"ERROR",
				_("An error has occurred when trying to UPDATE"),
				"details",
				_("Close"));
	}
	//update_all_titles();
}

void
Instance::revert()
{
	OneMoment one_moment;

	String filename(get_file_name());

	Canvas::Handle canvas(get_canvas());

	close();

	if(canvas->count()!=1)
	{
		one_moment.hide();
		App::dialog_message_1b(
				"ERROR",
				_("The revert operation has failed."),
				_("This can be due to it being referenced by another composition"
					" that is already open, or because of an internal error "
					"in Synfig Studio. Try closing any compositions that "
					"might reference this composition and try again, or "
					"restart Synfig Studio."),
				_("Close"));

		one_moment.show();
	}
	canvas=0;

	App::open(filename);
}

bool
Instance::safe_revert()
{
	if(synfigapp::Instance::get_action_count())
	{
		if (!App::dialog_message_2b(
			_("Revert to saved"),
			_("You will lose any changes you have made since your last save."
				"Are you sure?"),
			Gtk::MESSAGE_QUESTION,
			_("Cancel"),
			_("Revert"))
		)
			return false;
	}

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
		App::dialog_message_1b(
				"ERROR",
				_("The animation is currently playing so the window cannot be closed."),
				"details",
				_("Thanks!"));

		return false;
	}
	if(get_action_count())
		do
		{
			string message = strprintf(_("Save changes to document \"%s\" before closing?"),
					basename(get_file_name()).c_str() );

			string details = (_("If you don't save, changes from the last time you saved "
					"will be permanently lost."));

			int answer=uim->yes_no_cancel(
						message,
						details,
						_("Close without Saving"),
						_("Cancel"),
						_("Save"),
						synfigapp::UIInterface::RESPONSE_YES
			);

			if(answer == synfigapp::UIInterface::RESPONSE_YES){
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
		string message = strprintf(_("Commit changes of \"%s\" to  the CVS repository?"),
				basename(get_file_name()).c_str());

		string details = (_("If you don't commit, changes not yet on the CVS repository will "
				"be permanently lost."));

		int answer=uim->yes_no_cancel(
					message,
					details,
					_("Close without Committing"),
					_("Cancel"),
					_("Commit…"),
					synfigapp::UIInterface::RESPONSE_YES
		);

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

	// if(candidate_list.empty())
	// 	synfig::warning("%s:%d Action CandidateList is empty!", __FILE__, __LINE__);

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
			Gtk::MenuItem *item = Gtk::manage(new Gtk::ImageMenuItem(
				*Gtk::manage(new Gtk::Image(get_action_stock_id(*iter),Gtk::ICON_SIZE_MENU)),
				iter->local_name ));
			item->signal_activate().connect(
				sigc::bind(
					sigc::bind(
						sigc::mem_fun(
							*const_cast<studio::Instance*>(this),
							&studio::Instance::process_action ),
						param_list ),
					iter->name ));
			item->show_all();
			menu->append(*item);
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
			Gtk::MenuItem *item = Gtk::manage(new Gtk::ImageMenuItem(
				*Gtk::manage(new Gtk::Image(get_action_stock_id(*iter),Gtk::ICON_SIZE_MENU)),
				iter->local_name ));
			item->signal_activate().connect(
				sigc::bind(
					sigc::bind(
						sigc::mem_fun(
							*const_cast<studio::Instance*>(this),
							&studio::Instance::process_action ),
						param_list2 ),
					iter->name ));
			item->show_all();
			menu->append(*item);
		}
	}

	for(iter=candidate_list.begin();iter!=candidate_list.end();++iter)
	{
		if(!(iter->category&synfigapp::Action::CATEGORY_HIDDEN))
		{
			Gtk::MenuItem *item = Gtk::manage(new Gtk::ImageMenuItem(
				*Gtk::manage(new Gtk::Image(get_action_stock_id(*iter),Gtk::ICON_SIZE_MENU)),
				iter->local_name ));
			item->signal_activate().connect(
				sigc::bind(
					sigc::bind(
						sigc::mem_fun(
							*const_cast<studio::Instance*>(this),
							&studio::Instance::process_action ),
						param_list ),
					iter->name ));
			item->show_all();
			menu->append(*item);
		}
	}
}

void
Instance::process_action(synfig::String name, synfigapp::Action::ParamList param_list)
{
	if (getenv("SYNFIG_DEBUG_ACTIONS"))
		synfig::info("%s:%d process_action: '%s'", __FILE__, __LINE__, name.c_str());

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
				if(iter->get_value_provided())
				{
					synfigapp::Action::Param param;
					if (action->get_param(iter->get_name(),param))
					{
						if(param.get_type()==synfigapp::Action::Param::TYPE_STRING)
							str = param.get_string();
					}
				}
				String button2 = _("Export");
				String label = _("Name: ");

				// export and rename value dialog
				if (entry.name == "ValueNodeRename") button2 = _("Rename");
				// set layer description dialog
				if (entry.name == "LayerSetDesc")
				{
					button2 = _("Set");
					label = _("Description: ");
				}

				if(!studio::App::dialog_entry(entry.local_name,
							label,
							//iter->get_local_name()+": "+iter->get_desc(),
							str,
							_("Cancel"),
							button2))
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
Instance::make_param_menu(Gtk::Menu *menu,synfig::Canvas::Handle canvas, synfigapp::ValueDesc value_desc, float location, bool bezier)
{
	Gtk::Menu& parammenu(*menu);
	synfigapp::ValueDesc value_desc2(value_desc);
	etl::handle<synfigapp::CanvasInterface> canvas_interface(find_canvas_interface(canvas));

	if(!canvas_interface)
		return;

	Gtk::MenuItem *item = NULL;

	synfigapp::Action::ParamList param_list,param_list2;
	param_list=canvas_interface->generate_param_list(value_desc);
	param_list.add("origin",location);

#ifdef BLINEPOINT_MENU_IS_VERTEX_MENU
	if(value_desc.get_value_type()==type_bline_point && value_desc.is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		param_list2=canvas_interface->generate_param_list(
			synfigapp::ValueDesc(
				ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node())
				,ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node())
                                                           ->get_link_index_from_name("point")
			)
		);
		param_list2.add("origin",location);
	}
#endif	// BLINEPOINT_MENU_IS_VERTEX_MENU

	// Populate the convert menu by looping through
	// the ValueNode book and find the ones that are
	// relevant.

	// show the 'Convert' sub-menu if this valuedesc is anything other than either:
	//   the 'Index' parameter of a Duplicate layer
	// or
	//   a Duplicate ValueNode whose parent is not a (layer or ValueNode)
	if (!((value_desc.parent_is_layer() &&
		   value_desc.get_layer()->get_name() == "duplicate" &&
		   value_desc.get_param_name() == "index") ||
		  (value_desc.is_value_node() &&
		   ValueNode_Duplicate::Handle::cast_dynamic(value_desc.get_value_node()) &&
		   !(value_desc.parent_is_layer() ||
			 value_desc.parent_is_value_node()))))
	{
		Gtk::Menu *convert_menu=Gtk::manage(new Gtk::Menu());
		LinkableValueNode::Book::const_iterator iter;
		for(iter=LinkableValueNode::book().begin();iter!=LinkableValueNode::book().end();++iter)
		{
			if(iter->second.check_type(value_desc.get_value_type()))
			{
				item = Gtk::manage(new Gtk::MenuItem(iter->second.local_name));
				item->signal_activate().connect(
					sigc::hide_return(
						sigc::bind(
							sigc::bind(
								sigc::mem_fun(*canvas_interface.get(),&synfigapp::CanvasInterface::convert),
								iter->first ),
							value_desc )));
				item->show();
				convert_menu->append(*item);
			}
		}

		item = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::CONVERT));
		item->set_submenu(*convert_menu);
		item->show();
		parammenu.append(*item);
	}
	
	// Interpolation menu: Show it only if
	// the value description is constant or animated (layer parameter or exported) and not static
	// and value is not canvas type
	if (((value_desc.is_const() && !value_desc.get_static()) || value_desc.is_animated())
	&&
		value_desc.get_value_type()!= type_canvas
		)
	{
		Gtk::Menu *param_interpolation_menu=Gtk::manage(new Gtk::Menu());
		synfigapp::Action::ParamList param_list;
		param_list.add("canvas", get_canvas());
		param_list.add("value_desc", value_desc);
		param_list.add("canvas_interface",canvas_interface);

		/////// Default
		param_list.add("new_value", INTERPOLATION_UNDEFINED);
		item = Gtk::manage(new Gtk::MenuItem(_("Default")));
		item->signal_activate().connect(
			sigc::bind(
				sigc::bind(
					sigc::mem_fun(*const_cast<studio::Instance*>(this),&studio::Instance::process_action),
					param_list ),
				"ValueDescSetInterpolation" ));
		item->show();
		param_interpolation_menu->append(*item);
		param_list.erase("new_value");

		#define ADD_IMAGE_MENU_ITEM(Interpolation, StockId, Text) \
		param_list.add("new_value", Interpolation); \
		item = Gtk::manage(new Gtk::ImageMenuItem( \
			*Gtk::manage(new Gtk::Image(Gtk::StockID(StockId), Gtk::IconSize::from_name("synfig-small_icon"))), \
			_(Text) )); \
		item->signal_activate().connect( \
			sigc::bind( \
				sigc::bind( \
					sigc::mem_fun(*const_cast<studio::Instance*>(this),&studio::Instance::process_action), \
					param_list ), \
				"ValueDescSetInterpolation" )); \
		item->show_all(); \
		param_interpolation_menu->append(*item); \
		param_list.erase("new_value");
		
		
		ADD_IMAGE_MENU_ITEM(INTERPOLATION_TCB, "synfig-interpolation_type_tcb", _("TCB"));
		ADD_IMAGE_MENU_ITEM(INTERPOLATION_LINEAR, "synfig-interpolation_type_linear", _("Linear"));
		ADD_IMAGE_MENU_ITEM(INTERPOLATION_HALT, "synfig-interpolation_type_ease", _("Ease"));
		ADD_IMAGE_MENU_ITEM(INTERPOLATION_CONSTANT, "synfig-interpolation_type_const", _("Constant"));
		ADD_IMAGE_MENU_ITEM(INTERPOLATION_CLAMPED, "synfig-interpolation_type_clamped", _("Clamped"));
		
		#undef ADD_IMAGE_MENU_ITEM

		item = Gtk::manage(new Gtk::MenuItem(_("Interpolation")));
		item->set_submenu(*param_interpolation_menu);
		item->show();
		parammenu.append(*item);
	}

	synfigapp::Action::Category categories = synfigapp::Action::CATEGORY_VALUEDESC|synfigapp::Action::CATEGORY_VALUENODE;
	if (bezier)
		categories = categories|synfigapp::Action::CATEGORY_BEZIER;

	const DuckList selected_ducks(find_canvas_view(canvas)->get_work_area()->get_selected_ducks());
	for(DuckList::const_iterator iter=selected_ducks.begin();iter!=selected_ducks.end();++iter)
	{
		synfigapp::ValueDesc selected_value_desc((*iter)->get_value_desc());
		if(selected_value_desc.is_valid() && value_desc != selected_value_desc)
			param_list.add("selected_value_desc",selected_value_desc);
	}

	if(param_list2.empty())
		add_actions_to_menu(&parammenu, param_list,categories);
	else
		add_actions_to_menu(&parammenu, param_list2,param_list,categories);

	if((value_desc2.get_value_type()==type_bline_point || value_desc2.get_value_type()==type_width_point)
	 && value_desc2.is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc2.get_value_node()))
	{
		// the index=0 is position for widthpoint and vertex for blinepoint
		value_desc2=synfigapp::ValueDesc(ValueNode_Composite::Handle::cast_dynamic(value_desc2.get_value_node()),0);
	}

	if(value_desc2.is_value_node() && ValueNode_Animated::Handle::cast_dynamic(value_desc2.get_value_node()))
	{
		ValueNode_Animated::Handle value_node(ValueNode_Animated::Handle::cast_dynamic(value_desc2.get_value_node()));

		try
		{
			// try to find a waypoint at the current time - if we
			// can't, we don't want the menu entry - an exception is thrown
			WaypointList::iterator iter(value_node->find(canvas->get_time()));
			std::set<synfig::Waypoint, std::less<UniqueID> > waypoint_set;
			waypoint_set.insert(*iter);

			item = Gtk::manage(new Gtk::MenuItem(_("Edit Waypoint")));
			item->signal_activate().connect(
				sigc::bind(
					sigc::bind(
						sigc::bind(
							sigc::mem_fun(*find_canvas_view(canvas),&studio::CanvasView::on_waypoint_clicked_canvasview),
							-1 ),
						waypoint_set ),
					value_desc2 ));
			item->show();
			parammenu.append(*item);
		}
		catch(...)
		{ }
	}
	//// Add here the rest of actions here for specific single value descriptions
	//
	// Specific actions for Widthpoints (Composite)
	if(value_desc.get_value_type()==type_width_point && value_desc.is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueNode_Composite::Handle wpoint_composite(ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()));
		synfigapp::Action::ParamList param_list;
		param_list.add("canvas",canvas);
		param_list.add("canvas_interface",canvas_interface);
		param_list.add("time",canvas_interface->get_time());
		item = Gtk::manage(new Gtk::SeparatorMenuItem());
		item->show();
		parammenu.append(*item);


		#define ADD_IMAGE_MENU_ITEM(Type, StockId, Text) \
			param_list.add("new_value", ValueBase((int)WidthPoint::Type)); \
			item = Gtk::manage(new Gtk::ImageMenuItem( \
				*Gtk::manage(new Gtk::Image(Gtk::StockID(StockId),Gtk::IconSize::from_name("synfig-small_icon"))), \
				_(Text) )); \
			item->signal_activate().connect( \
				sigc::bind( \
					sigc::bind( \
						sigc::mem_fun(*const_cast<studio::Instance*>(this),&studio::Instance::process_action), \
						param_list ), \
					"ValueDescSet" )); \
			item->show_all(); \
			parammenu.append(*item); \
			param_list.erase("new_value");

		////// Before //////////////////
		param_list.add("value_desc",synfigapp::ValueDesc(wpoint_composite, wpoint_composite->get_link_index_from_name("side_before")));

		ADD_IMAGE_MENU_ITEM(TYPE_INTERPOLATE, "synfig-interpolate_interpolation", "Interpolate")
		ADD_IMAGE_MENU_ITEM(TYPE_ROUNDED, "synfig-rounded_interpolation", "Rounded")
		ADD_IMAGE_MENU_ITEM(TYPE_SQUARED, "synfig-squared_interpolation", "Squared")
		ADD_IMAGE_MENU_ITEM(TYPE_PEAK, "synfig-peak_interpolation", "Peak")
		ADD_IMAGE_MENU_ITEM(TYPE_FLAT, "synfig-flat_interpolation", "Flat")

		////// After ///////////////////////
		param_list.erase("value_desc");
		param_list.add("value_desc",synfigapp::ValueDesc(wpoint_composite, wpoint_composite->get_link_index_from_name("side_after")));

		ADD_IMAGE_MENU_ITEM(TYPE_INTERPOLATE, "synfig-interpolate_interpolation", "Interpolate")
		ADD_IMAGE_MENU_ITEM(TYPE_ROUNDED, "synfig-rounded_interpolation", "Rounded")
		ADD_IMAGE_MENU_ITEM(TYPE_SQUARED, "synfig-squared_interpolation", "Squared")
		ADD_IMAGE_MENU_ITEM(TYPE_PEAK, "synfig-peak_interpolation", "Peak")
		ADD_IMAGE_MENU_ITEM(TYPE_FLAT, "synfig-flat_interpolation", "Flat")

		///////
		item = Gtk::manage(new Gtk::SeparatorMenuItem());
		item->show();
		parammenu.append(*item);

		/////// Set WIDTH to ZERO
		param_list.erase("value_desc");
		param_list.erase("new_value");
		param_list.add("value_desc",synfigapp::ValueDesc(wpoint_composite, wpoint_composite->get_link_index_from_name("width")));
		param_list.add("new_value", ValueBase(Real(0.0)));
		item = Gtk::manage(new Gtk::MenuItem(_("Set width to zero")));
		item->signal_activate().connect(
			sigc::bind(
				sigc::bind(
					sigc::mem_fun(*const_cast<studio::Instance*>(this),&studio::Instance::process_action),
					param_list ),
				"ValueDescSet" ));
		item->show();
		parammenu.append(*item);

		/////// Set WIDTH to DEFAULT
		param_list.erase("new_value");
		param_list.add("value_desc",synfigapp::ValueDesc(wpoint_composite, wpoint_composite->get_link_index_from_name("width")));
		param_list.add("new_value", ValueBase(Real(1.0)));
		item = Gtk::manage(new Gtk::MenuItem(_("Set width to default")));
		item->signal_activate().connect(
			sigc::bind(
				sigc::bind(
					sigc::mem_fun(*const_cast<studio::Instance*>(this),&studio::Instance::process_action),
					param_list ),
				"ValueDescSet" ));
		item->show();
		parammenu.append(*item);
	}
}

void
edit_several_waypoints(etl::handle<CanvasView> canvas_view, std::list<synfigapp::ValueDesc> value_desc_list)
{
	etl::handle<synfigapp::CanvasInterface> canvas_interface(canvas_view->canvas_interface());

	Gtk::Dialog dialog(
		"Edit Multiple Waypoints",
		true
	);

	Widget_WaypointModel widget_waypoint_model;
	widget_waypoint_model.show();

	dialog.get_vbox()->pack_start(widget_waypoint_model);

	dialog.add_button(_("Cancel"), 0);
	dialog.add_button(_("Apply"), 1);
	dialog.show();

	if(dialog.run()==0 || widget_waypoint_model.get_waypoint_model().is_trivial())
		return;
	synfigapp::Action::PassiveGrouper group(canvas_interface->get_instance().get(),_("Set Waypoints"));

	std::list<synfigapp::ValueDesc>::iterator iter;
	for(iter=value_desc_list.begin();iter!=value_desc_list.end();++iter)
	{
		synfigapp::ValueDesc value_desc(*iter);

		if(!value_desc.is_valid())
			continue;

		ValueNode_Animated::Handle value_node;
		// Check if we are dealing with a BLinePoint or a WidthPoint value desc
		// If so, then change the value desc to be the position or the point.

		if(value_desc.is_value_node() && value_desc.parent_is_linkable_value_node())
		{
			synfig::ValueNode_Composite::Handle compo(synfig::ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()));
			if(compo && compo->get_type() == type_width_point)
			{
				value_desc=synfigapp::ValueDesc(compo, compo->get_link_index_from_name("position"));
				//value_node=ValueNode_Animated::Handle::cast_dynamic(compo->get_link(compo->get_link_index_from_name("position")));
			}
			if(compo && compo->get_type() == type_bline_point)
			{
				value_desc=synfigapp::ValueDesc(compo, compo->get_link_index_from_name("point"));
				//value_node=ValueNode_Animated::Handle::cast_dynamic(compo->get_link(compo->get_link_index_from_name("point")));
			}
		}

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
				action=synfigapp::Action::create("ValueDescConnect");
				action->set_param("dest",value_desc);
				action->set_param("src",ValueNode::Handle(value_node));
			}
			else
			{
				action=synfigapp::Action::create("ValueNodeReplace");
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
			synfigapp::Action::Handle action(synfigapp::Action::create("WaypointSetSmart"));

			if(!action)
			{
				canvas_view->get_ui_interface()->error(_("Unable to find WaypointSetSmart action"));
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
Instance::make_param_menu(Gtk::Menu *menu,synfig::Canvas::Handle canvas,const std::list<synfigapp::ValueDesc>& value_desc_list, const synfigapp::ValueDesc &value_desc)
{
	etl::handle<synfigapp::CanvasInterface> canvas_interface(find_canvas_interface(canvas));

	synfigapp::Action::ParamList param_list;
	param_list=canvas_interface->generate_param_list(value_desc_list);

	add_actions_to_menu(menu, param_list,synfigapp::Action::CATEGORY_VALUEDESC|synfigapp::Action::CATEGORY_VALUENODE);

	// Add the edit waypoints option if that might be useful
	if(canvas->rend_desc().get_time_end()-Time::epsilon()>canvas->rend_desc().get_time_start())
	{
		Gtk::MenuItem *item = Gtk::manage(new Gtk::MenuItem(_("Edit Waypoints")));
		item->signal_activate().connect(
			sigc::bind(
				sigc::bind(
					sigc::ptr_fun(&edit_several_waypoints),
					value_desc_list ),
				find_canvas_view(canvas) ));
		item->show();
		menu->append(*item);
	}
	// add here the rest of specific actions for multiple selected value_descs
	if (value_desc.is_valid())
		make_param_menu(menu,canvas,value_desc, 0.f, false);
}
