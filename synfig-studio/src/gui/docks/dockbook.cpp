/* === S Y N F I G ========================================================= */
/*!	\file dockbook.cpp
**	\brief Template File
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

#include <synfig/general.h>

#include "docks/dockbook.h"
#include "docks/dockable.h"
#include "app.h"
#include "docks/dockmanager.h"
#include "docks/dockdroparea.h"

#include <gtkmm/window.h>
#include <gtkmm/image.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/menu.h>
#include <gtkmm/imagemenuitem.h>

#include <gui/localization.h>

#include <gui/exception_guard.h>

#include "canvasview.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

DockBook::DockBook():
	allow_empty(false)
{
	std::vector<Gtk::TargetEntry> listTargets;
	listTargets.push_back( Gtk::TargetEntry("SYNFIG_DOCK") );

	drag_dest_set(listTargets);
	//set_sensitive(true);
	set_receives_default(true);
	set_can_default(true);
	//add_events(Gdk::ALL_EVENTS_MASK);
	//set_extension_events(Gdk::EXTENSION_EVENTS_ALL);
	set_show_tabs(true);
	set_scrollable(true);
	deleting_=false;

	dock_area = manage(new DockDropArea(this));
	dock_area->hide();
	set_action_widget(dock_area, Gtk::PACK_END);
}

DockBook::~DockBook()
{
	deleting_=true;
	clear();
	DockManager::containers_to_remove_.erase(this);
}

void
DockBook::clear()
{
	// here the point: get_n_pages is fails because this is not notebook type
	// i didn't know why this happens, possibly because clear() is called from destructor
	// and 'this' is already deleted. Or, this function maybe never work right.
	// So here quick-hack again. Btw, as you can see from commented code later newly created 
	// dockbook is works fine, so this situation is reqired more detailed investigation.
	if (!GTK_IS_NOTEBOOK (this)) return; // because we always fail if 'this' is not notebook

	/*Gtk::Notebook note;
	int x = note.get_n_pages();

	DockBook db;
	x = db.get_n_pages();

	const Gtk::Notebook* nb = (const Gtk::Notebook*)this;*/
	//assert(GTK_IS_NOTEBOOK (nb));
	while (this->get_n_pages())
		remove(static_cast<Dockable&>(*get_nth_page(get_n_pages()-1)));
}

void
DockBook::on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time)
{
	if ((selection_data.get_length() >= 0) && (selection_data.get_format() == 8))
	{
		Dockable& dockable(**reinterpret_cast<Dockable**>(const_cast<guint8*>(selection_data.get_data())));
		if(dockable.get_parent()!=this)
			add(dockable);
		dockable.present();
		context->drag_finish(true, false, time);
		return;
	}

	context->drag_finish(false, false, time);
}

void
DockBook::add(Dockable& dockable, int position)
{
	DockManager::remove_widget_recursive(dockable);

	if(position==-1)
		append_page(dockable, " ");
	else
		insert_page(dockable, " ", position);

	refresh_tab(&dockable);

	dockable.signal_stock_id_changed().connect(
		sigc::bind(
			sigc::mem_fun(
				*this,
				&DockBook::refresh_tab
			),
			&dockable
		)
	);

	dockable.show();

	signal_changed_();
}

void
DockBook::refresh_tab(Dockable* dockable)
{
	Gtk::Widget* label(dockable->create_tab_label());

	label->signal_button_press_event().connect(
		sigc::bind(
			sigc::mem_fun(
				*this,
				&DockBook::tab_button_pressed
			),
			dockable
		)
	);

	set_tab_label(*dockable, *label);
	label->show();
}


void
DockBook::remove(Dockable& dockable)
{
	dockable.hide();
	remove_page(dockable);

	if(!deleting_)
	{
		signal_changed_();

		if(get_n_pages()==0)
			signal_empty()();
	}
}

void
DockBook::present()
{
	show();
	if (Gtk::Window *window = dynamic_cast<Gtk::Window*>(get_toplevel()))
		window->present();
}

synfig::String
DockBook::get_local_contents()const
{
	synfig::String ret;

	for(int i(0);i!=const_cast<DockBook*>(this)->get_n_pages();i++)
	{
		Dockable& dockable(static_cast<Dockable&>(*const_cast<DockBook*>(this)->get_nth_page(i)));

		if(i)
			ret+=", ";
		ret+=dockable.get_local_name();
	}

	return ret;
}

synfig::String
DockBook::get_contents()const
{
	synfig::String ret;

	for(int i(0);i!=const_cast<DockBook*>(this)->get_n_pages();i++)
	{
		Dockable& dockable(static_cast<Dockable&>(*const_cast<DockBook*>(this)->get_nth_page(i)));

		if(i)
			ret+=' ';
		ret+=dockable.get_name();
	}

	return ret;
}

void
DockBook::set_contents(const synfig::String& x)
{
	synfig::String str(x);
	while(!str.empty())
	{
		synfig::String::size_type separator=str.find_first_of(' ');
		synfig::String dock;
		if(separator==synfig::String::npos)
		{
			dock=str;
			str.clear();
		}
		else
		{
			dock=String(str.begin(),str.begin()+separator);
			str=String(str.begin()+separator+1,str.end());
		}

		try
		{
			add(App::dock_manager->find_dockable(dock));
		}catch(...) { }
	}
}

bool
DockBook::tab_button_pressed(GdkEventButton* event, Dockable* dockable)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	CanvasView *canvas_view = dynamic_cast<CanvasView*>(dockable);
	if (canvas_view && canvas_view != App::get_selected_canvas_view())
		App::set_selected_canvas_view(canvas_view);

	if(event->button!=3)
		return false;

	Gtk::Menu *tabmenu=manage(new class Gtk::Menu());
	tabmenu->signal_hide().connect(sigc::bind(sigc::ptr_fun(&delete_widget), tabmenu));

	Gtk::MenuItem *item = manage(new Gtk::MenuItem(_("Undock panel")));
	item->signal_activate().connect(sigc::mem_fun(*dockable, &Dockable::detach_to_pointer));
	item->show();
	tabmenu->append(*item);

	item = manage(new Gtk::ImageMenuItem(Gtk::StockID("gtk-close")));
	item->signal_activate().connect(
		sigc::bind(sigc::ptr_fun(&DockManager::remove_widget_by_pointer_recursive), dockable) );
	item->show();
	tabmenu->append(*item);

	tabmenu->popup(event->button,gtk_get_current_event_time());

	return true;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

void
DockBook::on_switch_page(Gtk::Widget* page, guint page_num)
{
	if (page != NULL && this->page_num(*page)) {
		CanvasView *canvas_view = dynamic_cast<CanvasView*>(page);
		if (canvas_view && canvas_view != App::get_selected_canvas_view())
			App::set_selected_canvas_view(canvas_view);
	}
	Notebook::on_switch_page(page, page_num);
}

void DockBook::set_dock_area_visibility(bool visible, DockBook* source)
{
	if (visible && source == this && get_n_pages() == 1)
		return;

	dock_area->set_visible(visible);
}
