/* === S I N F G =========================================================== */
/*!	\file dockbook.cpp
**	\brief Template File
**
**	$Id: dockbook.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include "dockbook.h"
#include "dockable.h"
#include "app.h"
#include "dockmanager.h"

#include <gtkmm/image.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/menu.h>

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

DockBook::DockBook()
{
	std::list<Gtk::TargetEntry> listTargets;
	listTargets.push_back( Gtk::TargetEntry("DOCK") );

	drag_dest_set(listTargets);
	//set_sensitive(true);
	set_flags(get_flags()|Gtk::RECEIVES_DEFAULT|Gtk::HAS_GRAB);
	//add_events(Gdk::ALL_EVENTS_MASK);
	//set_extension_events(Gdk::EXTENSION_EVENTS_ALL);
	set_show_tabs(true);
	deleting_=false;
}

DockBook::~DockBook()
{
	deleting_=true;
	clear();
}

void
DockBook::clear()
{
	while(get_n_pages())
	{
		remove(static_cast<Dockable&>(*get_nth_page(get_n_pages()-1)));
	}
}

void
DockBook::on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time)
{
	if ((selection_data.get_length() >= 0) && (selection_data.get_format() == 8))
	{
		Dockable& dockable(**reinterpret_cast<Dockable**>(const_cast<guint8*>(selection_data.get_data())));
		if(dockable.parent_!=this)
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
	dockable.detach();
	
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
	
	dockable.parent_=this;

	dockable.show();

	//set_current_page(get_n_pages()-1);
		
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
	dockable.parent_=0;

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
}

sinfg::String
DockBook::get_local_contents()const
{
	sinfg::String ret;
	
	for(int i(0);i!=const_cast<DockBook*>(this)->get_n_pages();i++)
	{
		Dockable& dockable(static_cast<Dockable&>(*const_cast<DockBook*>(this)->get_nth_page(i)));
		
		if(i)
			ret+=", ";
		ret+=dockable.get_local_name();
	}
	
	return ret;
}

sinfg::String
DockBook::get_contents()const
{
	sinfg::String ret;
	
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
DockBook::set_contents(const sinfg::String& x)
{
	sinfg::String str(x);
	while(!str.empty())
	{
		unsigned int separator=str.find_first_of(' ');
		sinfg::String dock;
		if(separator==sinfg::String::npos)
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
	if(event->button!=3)
		return false;
	
	Gtk::Menu *tabmenu=manage(new class Gtk::Menu());
	
	tabmenu->items().push_back(
		Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-close"),
			sigc::mem_fun(*dockable,&Dockable::detach)
		)
	);

	tabmenu->popup(event->button,gtk_get_current_event_time());
	
	return true;
}
