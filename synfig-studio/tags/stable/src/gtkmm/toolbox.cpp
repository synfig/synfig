/*! ========================================================================
** Sinfg
** Template File
** $Id: toolbox.cpp,v 1.3 2005/01/13 20:23:01 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
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
#include "dialog_gradient.h"
#include "dialog_color.h"
#include "dialog_tooloptions.h"
#include "dialog_preview.h"
#include "dockable.h"
#include "dockmanager.h"
#include "dockdialog.h"

#include "widget_defaults.h"

#include <sinfgapp/main.h>

#endif

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;
using namespace SigC;

/* === M A C R O S ========================================================= */

#define GRAB_HINT_DATA(y)	{ \
		String x; \
		if(sinfgapp::Main::settings().get_value(String("pref.")+y+"_hints",x)) \
		{ \
			set_type_hint((Gdk::WindowTypeHint)atoi(x.c_str())); \
		} \
	}

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

#define TOGGLE_TOOLBOX_BUTTON(button,stockid,tooltip)	\
	button = manage(new class Gtk::ToggleButton());	\
	icon=manage(new Gtk::Image(Gtk::StockID(stockid),Gtk::IconSize(4)));	\
	button->add(*icon);	\
	tooltips.set_tip(*button,tooltip);	\
	icon->show();	\
	button->show()

#define TOOLBOX_BUTTON(button,stockid,tooltip)	\
	button = manage(new class Gtk::Button());	\
	icon=manage(new Gtk::Image(Gtk::StockID(stockid),Gtk::IconSize(4)));	\
	button->add(*icon);	\
	tooltips.set_tip(*button,tooltip);	\
	icon->show();	\
	button->show()

#define ADD_TOOLBOX_BUTTON(button,stockid,tooltip)	Gtk::Button *TOOLBOX_BUTTON(button,stockid,tooltip)

void
save_selected_instance()
{
	if(!studio::App::get_selected_instance())
	{
		App::dialog_error_blocking("Cannot save","Nothing to save");
		return;
	}

	if(!studio::App::get_selected_instance()->save())
		App::dialog_error_blocking("Save - Error","Unable to save file");
}

void
save_as_selected_instance()
{
	if(!studio::App::get_selected_instance())
	{
		App::dialog_error_blocking("Cannot save as","Nothing to save");
		return;
	}

	studio::App::get_selected_instance()->dialog_save_as();
}

void
close_selected_instance()
{
	etl::handle<studio::Instance> instance=studio::App::get_selected_instance();

	if(!instance)
	{
		App::dialog_error_blocking("Cannot close","Nothing to close");
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

static void
show_dialog_color()
{
	App::dialog_color->present();
}

Toolbox::Toolbox():
	Gtk::Window(Gtk::WINDOW_TOPLEVEL),
	dialog_settings(this,"toolbox")
{
	recent_files_menu= manage(new class Gtk::Menu());
	
	Gtk::Menu	*filemenu	=manage(new class Gtk::Menu());

	dock_dialogs=manage(new class Gtk::Menu());

	dock_dialogs->items().push_back(Gtk::Menu_Helpers::MenuElem("Canvases, History",sigc::ptr_fun(_create_stock_dialog1)));
	dock_dialogs->items().push_back(Gtk::Menu_Helpers::MenuElem("Layers, Children , Params",sigc::ptr_fun(_create_stock_dialog2)));
	dock_dialogs->items().push_back(Gtk::Menu_Helpers::SeparatorElem());

	
	filemenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::NEW,
		sigc::ptr_fun(&studio::App::new_instance)));	
	filemenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::OPEN,
		sigc::ptr_fun(&studio::App::dialog_open)));	

	filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Open Recent"),*recent_files_menu));
	
	filemenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("sinfg-saveall"),
		sigc::ptr_fun(&studio::App::dialog_not_implemented)));
	filemenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::CLOSE,
		sigc::ptr_fun(close_selected_instance)));
	filemenu->items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Dialogs"),*dock_dialogs));

	//filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Canvas Browser..."),
	//	sigc::mem_fun(studio::App::show_comp_view)));
	//filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Gradient Editor..."),
	//	sigc::mem_fun(show_dialog_gradient)));
	//filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Tool Options"),
	//	sigc::mem_fun(show_dialog_tool_options)));
	//filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Colors..."),
	//	sigc::mem_fun(show_dialog_color)));
	//filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Color Palette..."),
	//	sigc::mem_fun(show_dialog_palette)));
	filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Input Devices..."),
		sigc::ptr_fun(&show_dialog_input)));
	filemenu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Setup..."),
		sigc::ptr_fun(&studio::App::show_setup)));

	filemenu->items().push_back(Gtk::Menu_Helpers::SeparatorElem());	
	filemenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID(Gtk::Stock::QUIT),
		sigc::ptr_fun(studio::App::quit)));	
	
	Gtk::Menu	*helpmenu = manage(new class Gtk::Menu());
	helpmenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::HELP,
		sigc::ptr_fun(studio::App::dialog_not_implemented)));	
	helpmenu->items().push_back(Gtk::Menu_Helpers::SeparatorElem());	
	helpmenu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("sinfg-about"),
		sigc::ptr_fun(studio::App::dialog_about)));	
	
	Gtk::MenuBar *menubar1 = manage(new class Gtk::MenuBar());
	menubar1->items().push_back(Gtk::Menu_Helpers::MenuElem("_File",*filemenu));
	menubar1->items().push_back(Gtk::Menu_Helpers::MenuElem("_Help",*helpmenu));

	
	menubar1->show();
	
	Gtk::Image *icon;
	
	ADD_TOOLBOX_BUTTON(button_new,"gtk-new","New");
	ADD_TOOLBOX_BUTTON(button_open,"gtk-open","Open");
	ADD_TOOLBOX_BUTTON(button_save,"gtk-save","Save");
	ADD_TOOLBOX_BUTTON(button_saveas,"gtk-save-as","SaveAs");
	ADD_TOOLBOX_BUTTON(button_save_all,"sinfg-saveall","Save All");
	TOOLBOX_BUTTON(button_undo,"gtk-undo","Undo");
	TOOLBOX_BUTTON(button_redo,"gtk-redo","Redo");
	ADD_TOOLBOX_BUTTON(button_about,"sinfg-about","About Sinfg Studio");
	ADD_TOOLBOX_BUTTON(button_color,"sinfg-color","Color Dialog");
	
	TOOLBOX_BUTTON(button_rotoscope_bline,"sinfg-rotoscope_bline",_("Old Rotoscope BLine"));
	TOOLBOX_BUTTON(button_rotoscope_polygon,"sinfg-rotoscope_polygon",_("Rotoscope Polygon"));
	TOOLBOX_BUTTON(button_eyedrop,"sinfg-eyedrop",_("Eyedrop Tool"));
	TOOLBOX_BUTTON(button_rotoscope,"sinfg-rotoscope_bline",_("Rotoscope 2"));
	


	button_about->signal_clicked().connect(sigc::ptr_fun(studio::App::dialog_about));
	button_new->signal_clicked().connect(sigc::ptr_fun(studio::App::new_instance));
	button_open->signal_clicked().connect(sigc::ptr_fun(studio::App::dialog_open));
	button_save->signal_clicked().connect(sigc::ptr_fun(save_selected_instance));
	button_saveas->signal_clicked().connect(sigc::ptr_fun(save_as_selected_instance));
	button_save_all->signal_clicked().connect(sigc::ptr_fun(studio::App::dialog_not_implemented));
	button_undo->signal_clicked().connect(sigc::ptr_fun(studio::App::undo));
	button_redo->signal_clicked().connect(sigc::ptr_fun(studio::App::redo));
	button_color->signal_clicked().connect(sigc::ptr_fun(show_dialog_color));

	// Create the file button cluster
	Gtk::Table *file_buttons=manage(new class Gtk::Table(4, 4, false));
	file_buttons->attach(*button_new,0,1,0,1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	file_buttons->attach(*button_open,1,2,0,1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	file_buttons->attach(*button_save,2,3,0,1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	file_buttons->attach(*button_saveas,3,4,0,1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	file_buttons->attach(*button_save_all,0,1,1,2, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	file_buttons->attach(*button_undo,1,2,1,2, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	file_buttons->attach(*button_redo,2,3,1,2, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	file_buttons->attach(*button_about,3,4,1,2, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	//file_buttons->attach(*button_color,0,1,2,3, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	file_buttons->show();

	tool_table=manage(new class Gtk::Table(4, 4, false));
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
	table1->attach(*menubar1, 0, 1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK, 0, 0);
	table1->attach(*file_buttons, 0, 1, 1, 2, Gtk::FILL|Gtk::EXPAND,Gtk::EXPAND|Gtk::FILL, 0, 0);
	//table1->attach(*manage(new Gtk::Label(_("Tools"))), 0, 1, 2, 3, Gtk::FILL|Gtk::EXPAND,Gtk::EXPAND|Gtk::FILL, 0, 0);
	table1->attach(*handle_tools, 0, 1, 3, 4, Gtk::FILL|Gtk::EXPAND,Gtk::EXPAND|Gtk::FILL, 0, 0);
	table1->attach(*handle_defaults, 0, 1, 4, 5, Gtk::FILL|Gtk::EXPAND,Gtk::EXPAND|Gtk::FILL, 0, 0);
	table1->show_all();
	
	
	
	// Set the parameters for this window
	add(*table1);
	set_title("Synfig Studio");
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
	button_rotoscope_bline->set_sensitive(false);
	button_rotoscope->set_sensitive(false);
	button_rotoscope_polygon->set_sensitive(false);	
	button_eyedrop->set_sensitive(false);	


	std::list<Gtk::TargetEntry> listTargets;
	listTargets.push_back( Gtk::TargetEntry("text/plain") );
	listTargets.push_back( Gtk::TargetEntry("image") );
//	listTargets.push_back( Gtk::TargetEntry("image/x-sif") );

	drag_dest_set(listTargets);
	signal_drag_data_received().connect( sigc::mem_fun(*this, &studio::Toolbox::on_drop_drag_data_received) );
	
	App::dock_manager->signal_dockable_registered().connect(sigc::mem_fun(*this,&Toolbox::dockable_registered));
	
	changing_state_=false;
	
	GRAB_HINT_DATA("toolbox");
	add_accel_group(App::ui_manager()->get_accel_group());
	
	App::signal_present_all().connect(sigc::mem_fun(*this,&Toolbox::present));
}

Toolbox::~Toolbox()
{
	hide();
	//studio::App::cb.task("Toolbox: I was nailed!");
	//studio::App::quit();

	if(studio::App::toolbox==this)
		studio::App::toolbox=NULL;

}

void
Toolbox::set_active_state(const String& statename)
{
	std::map<sinfg::String,Gtk::ToggleButton *>::iterator iter;

	changing_state_=true;
	
	sinfgapp::Main::set_state(statename);
	
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
Toolbox::change_state(const sinfg::String& statename)
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
			sinfg::error("Unknown state \"%s\"",statename.c_str());
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
		{
			if(state->get_name()==String("normal"))
			{
				canvas_view->get_smach().egress();				
			}
			else
			{
				canvas_view->get_smach().enter(state);
			}
		}
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

void
Toolbox::add_state(const Smach::state_base *state)
{
	Gtk::Image *icon;

	assert(state);

	String name=state->get_name();
	
	Gtk::ToggleButton* button;
	button=manage(new class Gtk::ToggleButton());

	icon=manage(new Gtk::Image(Gtk::StockID("sinfg-"+name),Gtk::IconSize(4)));
	button->add(*icon);
	tooltips.set_tip(*button,name);
	icon->show();
	button->show();

	

	
	int row=state_button_map.size()/4;
	int col=state_button_map.size()%4;

	tool_table->attach(*button,col,col+1,row,row+1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	
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
	// These next several lines just adjust the rotoscope buttons
	// so that they are only clickable when they should be.
	if(instance && App::get_selected_canvas_view())
	{
		std::map<sinfg::String,Gtk::ToggleButton *>::iterator iter;
		
		for(iter=state_button_map.begin();iter!=state_button_map.end();++iter)
			iter->second->set_sensitive(true);
	}
	else
	{
		std::map<sinfg::String,Gtk::ToggleButton *>::iterator iter;
		
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
	// Check to see if the file is already on the list.
	// If it is, then remove it from the list
	for(iter=App::get_recent_files().begin();iter!=App::get_recent_files().end();iter++)
		recent_files_menu->items().push_back(Gtk::Menu_Helpers::MenuElem(basename(*iter),
			sigc::hide_return(sigc::bind(sigc::ptr_fun(&App::open),*iter))
		));
	
	// HACK
	show();
}

void
Toolbox::on_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data_, guint info, guint time)
{
	// We will make this true once we have a solid drop
	bool success(false);
	
	if ((selection_data_.get_length() >= 0) && (selection_data_.get_format() == 8))
	{
		sinfg::String selection_data((gchar *)(selection_data_.get_data()));

		// For some reason, GTK hands us a list of URL's seperated
		// by not only Carrage-Returns, but also Line-Feeds.
		// Line-Feeds will mess us up. Remove all the line-feeds.
		while(selection_data.find_first_of('\r')!=sinfg::String::npos)
			selection_data.erase(selection_data.begin()+selection_data.find_first_of('\r'));

		std::stringstream stream(selection_data);

		while(stream)
		{
			sinfg::String filename,URI;
			getline(stream,filename);
			
			// If we don't have a filename, move on.
			if(filename.empty())
				continue;
			
			// Make sure this URL is of the "file://" type.
			URI=String(filename.begin(),filename.begin()+sizeof("file://")-1);
			if(URI!="file://")
			{
				sinfg::warning("Unknown URI (%s) in \"%s\"",URI.c_str(),filename.c_str());
				continue;
			}
			
			// Strip the "file://" part from the filename
			filename=sinfg::String(filename.begin()+sizeof("file://")-1,filename.end());
		
			sinfg::info("Attempting to open "+filename);		
			if(App::open(filename))
				success=true;
			else
				sinfg::error("Drop failed: Unable to open "+filename);
		}
	}
	else
		sinfg::error("Drop failed: bad selection data");

	// Finish the drag
	context->drag_finish(success, false, time);
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
