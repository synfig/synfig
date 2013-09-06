/* === S Y N F I G ========================================================= */
/*!	\file toolbox.cpp
**	\brief writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2008 Paul Wise
**	Copyright (c) 2009 Nikita Kitaev
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gtk/gtk.h>
#include <gtkmm/uimanager.h>

#include <gtkmm/ruler.h>
#include <gtkmm/arrow.h>
#include <gtkmm/image.h>
#include <gdkmm/pixbufloader.h>
#include <gtkmm/viewport.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/menubar.h>
#include <gtkmm/menu.h>
#include <gtkmm/button.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/stock.h>
#include <gtkmm/handlebox.h>
#include <gtkmm/accelmap.h>

#include <gtkmm/inputdialog.h>

#include <sigc++/signal.h>
#include <sigc++/hide.h>
#include <sigc++/slot.h>
#include <sigc++/retype_return.h>
#include <sigc++/retype.h>

#include <sstream>

#include "toolbox.h"
#include "instance.h"
#include "app.h"
#include "canvasview.h"
#include "dialogs/dialog_gradient.h"
#include "dialogs/dialog_color.h"
#include "docks/dialog_tooloptions.h"
#include "dialogs/dialog_preview.h"
#include "docks/dockable.h"
#include "docks/dockmanager.h"
#include "docks/dockdialog.h"

#include "widgets/widget_defaults.h"

#include <synfigapp/main.h>

#include "general.h"

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;
using namespace sigc;

/* === M A C R O S ========================================================= */

#define GRAB_HINT_DATA(y,default)	{ \
		String x; \
		if(synfigapp::Main::settings().get_value(String("pref.")+y+"_hints",x)) \
		{ \
			set_type_hint((Gdk::WindowTypeHint)atoi(x.c_str()));	\
		} else {\
			set_type_hint(default); \
		} \
	}

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

#define TOGGLE_TOOLBOX_BUTTON(button,stockid,tooltip)	\
	button = manage(new class Gtk::ToggleButton());	\
	icon=manage(new Gtk::Image(Gtk::StockID(stockid),Gtk::IconSize(4)));	\
	button->add(*icon);	\
	button->set_tooltip_text(tooltip);	\
	icon->show();	\
	button->show()

#define TOOLBOX_BUTTON(button,stockid,tooltip)	\
	button = manage(new class Gtk::Button());	\
	icon=manage(new Gtk::Image(Gtk::StockID(stockid),Gtk::IconSize(4)));	\
	button->add(*icon);	\
	button->set_tooltip_text(tooltip);	\
	icon->show();	\
	button->show()

#define ADD_TOOLBOX_BUTTON(button,stockid,tooltip)	Gtk::Button *TOOLBOX_BUTTON(button,stockid,tooltip)

void
save_selected_instance()
{
	if(!studio::App::get_selected_instance())
	{
		App::dialog_error_blocking(_("Cannot save"),_("Nothing to save"));
		return;
	}

	studio::App::get_selected_instance()->save();
}

void
save_as_selected_instance()
{
	if(!studio::App::get_selected_instance())
	{
		App::dialog_error_blocking(_("Cannot save as"),_("Nothing to save"));
		return;
	}

	studio::App::get_selected_instance()->dialog_save_as();
}

void
save_all()
{
	std::list<etl::handle<Instance> >::iterator iter;
	for(iter=App::instance_list.begin();iter!=App::instance_list.end();iter++)
		(*iter)->save();
}

void
close_selected_instance()
{
	etl::handle<studio::Instance> instance=studio::App::get_selected_instance();

	if(!instance)
	{
		App::dialog_error_blocking(_("Cannot close"),_("Nothing to close"));
		return;
	}

	instance->safe_close();

	//assert(instance.unique());
}


static void
show_dialog_input()
{
	App::dialog_input->present();
}

void _create_stock_dialog1()
{
	DockDialog* dock_dialog(new DockDialog);
	dock_dialog->set_contents("canvases history");
	dock_dialog->set_composition_selector(true);
	dock_dialog->present();
}
void _create_stock_dialog2()
{
	DockDialog* dock_dialog(new DockDialog);
	dock_dialog->set_contents("layers children keyframes | params");
	dock_dialog->present();
}

Toolbox::Toolbox():
	Gtk::Window(Gtk::WINDOW_TOPLEVEL),
	dialog_settings(this,"toolbox")
{
	GRAB_HINT_DATA(
		"toolbox",
//#ifdef __APPLE__
		Gdk::WINDOW_TYPE_HINT_NORMAL
//#else
//		Gdk::WINDOW_TYPE_HINT_UTILITY
//#endif
	);
	set_keep_above(false);
	set_role("toolbox");



	recent_files_menu= manage(new class Gtk::Menu());

	Gtk::Menu	*filemenu	=manage(new class Gtk::Menu());

	dock_dialogs=manage(new class Gtk::Menu());

	dock_dialogs->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Vertical Dock: Canvases, History"),sigc::ptr_fun(_create_stock_dialog1)));
	dock_dialogs->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Horizontal Dock: Layers, Library, Parameters"),sigc::ptr_fun(_create_stock_dialog2)));
	dock_dialogs->items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	dock_dialogs->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Reset Windows to Original Layout"),sigc::ptr_fun(App::reset_initial_window_configuration)));
	dock_dialogs->items().push_back(Gtk::Menu_Helpers::SeparatorElem());


	filemenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::NEW,
		sigc::ptr_fun(&studio::App::new_instance)));
	filemenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::OPEN,
		sigc::bind(sigc::ptr_fun(&studio::App::dialog_open), "")));

	filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Open Recent"),*recent_files_menu));

	filemenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("synfig-saveall"),
		sigc::ptr_fun(save_all)));
	filemenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::CLOSE,
		sigc::ptr_fun(close_selected_instance)));
	filemenu->items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Panels"),*dock_dialogs));

	filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Input Devices..."),
		sigc::ptr_fun(&show_dialog_input)));
	filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Setup..."),
		sigc::ptr_fun(&studio::App::show_setup)));
	filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Reset to default Setup values"),
		sigc::ptr_fun(&studio::App::reset_initial_preferences)));

	filemenu->items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	filemenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID(Gtk::Stock::QUIT),
		sigc::ptr_fun(studio::App::quit)));

#define WIKI(title,page)											\
	helpmenu->items().push_back(Gtk::Menu_Helpers::MenuElem(title,	\
		sigc::bind(sigc::ptr_fun(&studio::App::open_url),String("http://synfig.org/wiki")+page)))

#define SITE(title,page)											\
	helpmenu->items().push_back(Gtk::Menu_Helpers::MenuElem(title,	\
		sigc::bind(sigc::ptr_fun(&studio::App::open_url),String("http://synfig.org/cms")+page)))

	Gtk::Menu	*helpmenu = manage(new class Gtk::Menu());
	helpmenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::HELP, sigc::ptr_fun(studio::App::dialog_help)));
	helpmenu->items().push_back(Gtk::Menu_Helpers::SeparatorElem());

	/* TRANSLATORS: Help menu entry */ WIKI(_("Tutorials"),					/* TRANSLATORS: a wiki page */ _("/Category:Tutorials")				);
	/* TRANSLATORS: Help menu entry */ WIKI(_("Reference"),					/* TRANSLATORS: a wiki page */ _("/Category:Reference")				);
	/* TRANSLATORS: Help menu entry */ WIKI(_("Frequently Asked Questions"),/* TRANSLATORS: a wiki page */ _("/FAQ")					);
	helpmenu->items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	/* TRANSLATORS: Help menu entry */ SITE(_("Get Support"),				/* TRANSLATORS: a website page */ _("/en/support")			);
	helpmenu->items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	helpmenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("synfig-about"),
		sigc::ptr_fun(studio::App::dialog_about)));

	Gtk::MenuBar *menubar1 = manage(new class Gtk::MenuBar());
	menubar1->items().push_back(Gtk::Menu_Helpers::MenuElem(_("_File"),*filemenu));
	menubar1->items().push_back(Gtk::Menu_Helpers::MenuElem(_("_Help"),*helpmenu));


	menubar1->show();

	Gtk::Image *icon;

	ADD_TOOLBOX_BUTTON(button_new,"gtk-new",_("New..."));
	ADD_TOOLBOX_BUTTON(button_open,"gtk-open",_("Open..."));
	ADD_TOOLBOX_BUTTON(button_save,"gtk-save",_("Save"));
	ADD_TOOLBOX_BUTTON(button_saveas,"gtk-save-as",_("Save As..."));
	ADD_TOOLBOX_BUTTON(button_save_all,"synfig-saveall",_("Save All"));
	TOOLBOX_BUTTON(button_undo,"gtk-undo",_("Undo"));
	TOOLBOX_BUTTON(button_redo,"gtk-redo",_("Redo"));
	ADD_TOOLBOX_BUTTON(button_setup,"gtk-properties",_("Setup"));
	ADD_TOOLBOX_BUTTON(button_about,"synfig-about",_("About Synfig Studio"));
	ADD_TOOLBOX_BUTTON(button_help,"gtk-help",_("Help"));

	button_setup->signal_clicked().connect(sigc::ptr_fun(studio::App::show_setup));
	button_about->signal_clicked().connect(sigc::ptr_fun(studio::App::dialog_about));
	button_help->signal_clicked().connect(sigc::ptr_fun(studio::App::dialog_help));
	button_new->signal_clicked().connect(sigc::ptr_fun(studio::App::new_instance));
	button_open->signal_clicked().connect(sigc::bind(sigc::ptr_fun(studio::App::dialog_open), ""));
	button_save->signal_clicked().connect(sigc::ptr_fun(save_selected_instance));
	button_saveas->signal_clicked().connect(sigc::ptr_fun(save_as_selected_instance));
	button_save_all->signal_clicked().connect(sigc::ptr_fun(save_all));
	button_undo->signal_clicked().connect(sigc::ptr_fun(studio::App::undo));
	button_redo->signal_clicked().connect(sigc::ptr_fun(studio::App::redo));

	// Create the file button cluster
	Gtk::Table *file_buttons=manage(new class Gtk::Table());

	file_buttons->attach(*button_new,      0,1, 0,1, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
	file_buttons->attach(*button_open,     1,2, 0,1, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
	file_buttons->attach(*button_save,     2,3, 0,1, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
	file_buttons->attach(*button_saveas,   3,4, 0,1, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
	file_buttons->attach(*button_save_all, 4,5, 0,1, Gtk::EXPAND, Gtk::EXPAND, 0, 0);

	file_buttons->attach(*button_undo,     0,1, 1,2, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
	file_buttons->attach(*button_redo,     1,2, 1,2, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
	file_buttons->attach(*button_setup,    2,3, 1,2, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
	file_buttons->attach(*button_about,    3,4, 1,2, Gtk::EXPAND, Gtk::EXPAND, 0, 0);
	file_buttons->attach(*button_help,     4,5, 1,2, Gtk::EXPAND, Gtk::EXPAND, 0, 0);

	file_buttons->show();

	tool_table=manage(new class Gtk::Table());
	tool_table->show();
	Gtk::HandleBox* handle_tools(manage(new Gtk::HandleBox()));
	handle_tools->add(*tool_table);
	handle_tools->show();
	handle_tools->set_handle_position(Gtk::POS_TOP);
	handle_tools->set_snap_edge(Gtk::POS_TOP);

	Widget_Defaults* widget_defaults(manage(new Widget_Defaults()));
	widget_defaults->show();
	Gtk::HandleBox* handle_defaults(manage(new Gtk::HandleBox()));
	handle_defaults->add(*widget_defaults);
	handle_defaults->show();
	handle_defaults->set_handle_position(Gtk::POS_TOP);
	handle_defaults->set_snap_edge(Gtk::POS_TOP);

	// Create the toplevel table
	Gtk::Table *table1 = manage(new class Gtk::Table(1, 2, false));
	table1->set_row_spacings(0);
	table1->set_col_spacings(0);
	table1->attach(*menubar1,        0,1, 0,1, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK, 0, 0);
	table1->attach(*file_buttons,    0,1, 1,2, Gtk::FILL|Gtk::EXPAND,Gtk::EXPAND|Gtk::FILL, 0, 0);
	table1->attach(*handle_tools,    0,1, 2,3, Gtk::FILL|Gtk::EXPAND,Gtk::EXPAND|Gtk::FILL, 0, 0);
	table1->attach(*handle_defaults, 0,1, 3,4, Gtk::FILL|Gtk::EXPAND,Gtk::EXPAND|Gtk::FILL, 0, 0);
	table1->show_all();

	// Set the parameters for this window
	add(*table1);
	set_title(_("Synfig Studio"));
	set_modal(false);
	property_window_position().set_value(Gtk::WIN_POS_NONE);
	signal_delete_event().connect(sigc::ptr_fun(App::shutdown_request));
	set_resizable(false);

	App::signal_instance_selected().connect(
		sigc::hide(
			sigc::mem_fun(*this,&studio::Toolbox::update_undo_redo)
		)
	);

	App::signal_recent_files_changed().connect(
			sigc::mem_fun(*this,&studio::Toolbox::on_recent_files_changed)
	);

	button_undo->set_sensitive(false);
	button_redo->set_sensitive(false);

	std::list<Gtk::TargetEntry> listTargets;
	listTargets.push_back( Gtk::TargetEntry("text/plain") );
	listTargets.push_back( Gtk::TargetEntry("image") );
//	listTargets.push_back( Gtk::TargetEntry("image/x-sif") );

	drag_dest_set(listTargets);
	signal_drag_data_received().connect( sigc::mem_fun(*this, &studio::Toolbox::on_drop_drag_data_received) );

	App::dock_manager->signal_dockable_registered().connect(sigc::mem_fun(*this,&Toolbox::dockable_registered));

	changing_state_=false;


	add_accel_group(App::ui_manager()->get_accel_group());

	App::signal_present_all().connect(sigc::mem_fun0(*this,&Toolbox::present));
}

Toolbox::~Toolbox()
{
	hide();
	//studio::App::cb.task(_("Toolbox: I was nailed!"));
	//studio::App::quit();

	if(studio::App::toolbox==this)
		studio::App::toolbox=NULL;

}

void
Toolbox::set_active_state(const synfig::String& statename)
{
	std::map<synfig::String,Gtk::ToggleButton *>::iterator iter;

	changing_state_=true;

	synfigapp::Main::set_state(statename);

	try
	{

		for(iter=state_button_map.begin();iter!=state_button_map.end();++iter)
		{
			if(iter->first==statename)
			{
				if(!iter->second->get_active())
					iter->second->set_active(true);
			}
			else
			{
				if(iter->second->get_active())
					iter->second->set_active(false);
			}
		}
	}
	catch(...)
	{
		changing_state_=false;
		throw;
	}
	changing_state_=false;
}

void
Toolbox::change_state(const synfig::String& statename)
{
	etl::handle<studio::CanvasView> canvas_view(studio::App::get_selected_canvas_view());
	if(canvas_view)
	{
		if(statename==canvas_view->get_smach().get_state_name())
		{
			return;
		}

		if(state_button_map.count(statename))
		{
			state_button_map[statename]->clicked();
		}
		else
		{
			synfig::error("Unknown state \"%s\"",statename.c_str());
		}
	}
}

void
Toolbox::change_state_(const Smach::state_base *state)
{
	if(changing_state_)
		return;
	changing_state_=true;

	try
	{
		etl::handle<studio::CanvasView> canvas_view(studio::App::get_selected_canvas_view());
		if(canvas_view)
				canvas_view->get_smach().enter(state);
		else
			refresh();
	}
	catch(...)
	{
		changing_state_=false;
		throw;
	}

	changing_state_=false;
}


/*! \fn Toolbox::add_state(const Smach::state_base *state)
 *  \brief Add and connect a toogle button to the toolbox defined by a state
 *  \param state a const pointer to Smach::state_base
*/
void
Toolbox::add_state(const Smach::state_base *state)
{
	Gtk::Image *icon;

	assert(state);

	String name=state->get_name();

	Gtk::StockItem stock_item;
	Gtk::Stock::lookup(Gtk::StockID("synfig-"+name),stock_item);

	Gtk::ToggleButton* button;
	button=manage(new class Gtk::ToggleButton());

	Gtk::AccelKey key;
	//Have a look to global fonction init_ui_manager() from app.cpp for "accel_path" definition
	Gtk::AccelMap::lookup_entry ("<Actions>/action_group_state_manager/state-"+name, key);
	//Gets the accelerator representation for labels
	Glib::ustring accel_path = key.get_abbrev ();

	icon=manage(new Gtk::Image(stock_item.get_stock_id(),Gtk::IconSize(4)));
	button->add(*icon);
	button->set_tooltip_text(stock_item.get_label()+" "+accel_path);
	icon->show();
	button->show();

	int row=state_button_map.size()/5;
	int col=state_button_map.size()%5;

	tool_table->attach(*button,col,col+1,row,row+1, Gtk::EXPAND, Gtk::EXPAND, 0, 0);

	state_button_map[name]=button;

	button->signal_clicked().connect(
		sigc::bind(
			sigc::mem_fun(*this,&studio::Toolbox::change_state_),
			state
		)
	);

	refresh();
}


void
Toolbox::update_undo_redo()
{
	etl::handle<Instance> instance=App::get_selected_instance();
	if(instance)
	{
		button_undo->set_sensitive(instance->get_undo_status());
		button_redo->set_sensitive(instance->get_redo_status());
	}

	// This should probably go elsewhere, but it should
	// work fine here with no troubles.
	// These next several lines just adjust the tool buttons
	// so that they are only clickable when they should be.
	if(instance && App::get_selected_canvas_view())
	{
		std::map<synfig::String,Gtk::ToggleButton *>::iterator iter;

		for(iter=state_button_map.begin();iter!=state_button_map.end();++iter)
			iter->second->set_sensitive(true);
	}
	else
	{
		std::map<synfig::String,Gtk::ToggleButton *>::iterator iter;

		for(iter=state_button_map.begin();iter!=state_button_map.end();++iter)
			iter->second->set_sensitive(false);
	}

	etl::handle<CanvasView> canvas_view=App::get_selected_canvas_view();
	if(canvas_view && canvas_view->get_smach().get_state_name())
	{
		set_active_state(canvas_view->get_smach().get_state_name());
	}
	else
		set_active_state("none");

}

void
Toolbox::on_recent_files_changed()
{
	while(recent_files_menu->get_children().size())
		recent_files_menu->remove(**recent_files_menu->get_children().begin());

	list<string>::const_iterator iter;
	for(iter=App::get_recent_files().begin();iter!=App::get_recent_files().end();iter++)
	{
		string raw = basename(*iter), quoted;
		size_t pos = 0, last_pos = 0;

		// replace _ in filenames by __ or it won't show up in the menu
		for (pos = last_pos = 0; (pos = raw.find('_', pos)) != string::npos; last_pos = pos)
			quoted += raw.substr(last_pos, ++pos - last_pos) + '_';
		quoted += raw.substr(last_pos);

		recent_files_menu->items().push_back(Gtk::Menu_Helpers::MenuElem(quoted,
			sigc::hide_return(sigc::bind(sigc::ptr_fun(&App::open),*iter))
		));
	}

	// HACK
	show();
}

void
Toolbox::on_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int /*x*/, int /*y*/, const Gtk::SelectionData& selection_data_, guint /*info*/, guint time)
{
	// We will make this true once we have a solid drop
	bool success(false);

	if ((selection_data_.get_length() >= 0) && (selection_data_.get_format() == 8))
	{
		synfig::String selection_data((gchar *)(selection_data_.get_data()));

		// For some reason, GTK hands us a list of URLs separated
		// by not only Carriage-Returns, but also Line-Feeds.
		// Line-Feeds will mess us up. Remove all the line-feeds.
		while(selection_data.find_first_of('\r')!=synfig::String::npos)
			selection_data.erase(selection_data.begin()+selection_data.find_first_of('\r'));

		std::stringstream stream(selection_data);

		while(stream)
		{
			synfig::String filename,URI;
			getline(stream,filename);

			// If we don't have a filename, move on.
			if(filename.empty())
				continue;

			// Make sure this URL is of the "file://" type.
			URI=String(filename.begin(),filename.begin()+sizeof("file://")-1);
			if(URI!="file://")
			{
				synfig::warning("Unknown URI (%s) in \"%s\"",URI.c_str(),filename.c_str());
				continue;
			}

			// Strip the "file://" part from the filename
			filename=synfig::String(filename.begin()+sizeof("file://")-1,filename.end());

			synfig::info("Attempting to open "+filename);
			if(App::open(filename))
				success=true;
			else
				synfig::error("Drop failed: Unable to open "+filename);
		}
	}
	else
		synfig::error("Drop failed: bad selection data");

	// Finish the drag
	context->drag_finish(success, false, time);
}

bool
Toolbox::on_key_press_event(GdkEventKey* event)
{
	Gtk::Widget* focused_widget = get_focus();
	if(focused_widget && focused_widget_has_priority(focused_widget))
	{
		if(focused_widget->event((GdkEvent*)event))
		return true;
	}
	else if(Gtk::Window::on_key_press_event(event))
			return true;
		else
			if (focused_widget) return focused_widget->event((GdkEvent*)event);
	return false;
}

bool
Toolbox::focused_widget_has_priority(Gtk::Widget * focused)
{
	if(dynamic_cast<Gtk::Entry*>(focused))
		return true;
	return false;
}

void
Toolbox::dockable_registered(Dockable* x)
{
	dock_dialogs->items().push_back(
		Gtk::Menu_Helpers::MenuElem(
			x->get_local_name(),
			sigc::mem_fun(
				*x,
				&Dockable::present
			)
		)
	);
}
