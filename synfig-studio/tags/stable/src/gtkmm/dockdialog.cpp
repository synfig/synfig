/* === S I N F G =========================================================== */
/*!	\file dockdialog.cpp
**	\brief Template File
**
**	$Id: dockdialog.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include "app.h"
#include <sigc++/adaptors/hide.h>

#include "dockdialog.h"
#include "dockbook.h"
#include "dockmanager.h"
#include "widget_compselect.h"
#include <sinfg/general.h>
#include <sinfg/uniqueid.h>
#include <gtkmm/table.h>
#include <sigc++/hide.h>
#include <sigc++/slot.h>
#include <sigc++/retype_return.h>
#include <sigc++/retype.h>
#include "canvasview.h"
#include <gtkmm/paned.h>
#include <gtkmm/box.h>
#include <sinfgapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

#define GRAB_HINT_DATA(y)	{ \
		String x; \
		if(sinfgapp::Main::settings().get_value(String("pref.")+y+"_hints",x)) \
		{ \
			set_type_hint((Gdk::WindowTypeHint)atoi(x.c_str()));	\
		} \
	}

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

DockDialog::DockDialog():
	Gtk::Window(Gtk::WINDOW_TOPLEVEL)
{
	composition_selector_=false;
	is_deleting=false;
	is_horizontal=false;
	last_dock_book=0;
	box=0;
	
	widget_comp_select=new Widget_CompSelect();
	
	// Give ourselves an ID that is most likely unique
	set_id(sinfg::UniqueID().get_uid()^reinterpret_cast<int>(this));
	
	// Set up the window
	//set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);
	set_title("Dock Dialog");
	GRAB_HINT_DATA("dock_dialog");
	
	// Register with the dock manager
	App::dock_manager->dock_dialog_list_.push_back(this);

	
	// connect our signals	
	signal_delete_event().connect(
		sigc::hide(
			sigc::mem_fun(*this,&DockDialog::close)
		)
	);
	
/*
	App::signal_canvas_view_focus().connect(
		sigc::hide(
			sigc::mem_fun(
				*this,
				&DockDialog::refresh_accel_group
			)
		)
	);
*/

	add_accel_group(App::ui_manager()->get_accel_group());
	App::signal_present_all().connect(sigc::mem_fun(*this,&DockDialog::present));
}

DockDialog::~DockDialog()
{
	empty_sig.disconnect();

	is_deleting=true;

	DEBUGPOINT();

	// Remove all of the dock books
	for(;!dock_book_list.empty();dock_book_list.pop_front())
	{
		dock_book_list.front()->clear();

		// UGLY HACK
		// The following line really should be uncommented,
		// but it causes crashes. Without it, a small
		// memory hole is created--but at least it doesn't crash
		// delete dock_book_list.front();
		
		// Oddly enough, the following line should
		// theoreticly do the same thing after this
		// class is destroyed, but it doesn't seem to
		// caues a crash.
		manage(dock_book_list.front());
	}

	// Remove us from the dock manager
	if(App::dock_manager)try{
		std::list<DockDialog*>::iterator iter;
		for(iter=App::dock_manager->dock_dialog_list_.begin();iter!=App::dock_manager->dock_dialog_list_.end();++iter)
			if(*iter==this)
			{
				App::dock_manager->dock_dialog_list_.erase(iter);
				break;
			}
	}
	catch(...)
	{
		sinfg::warning("DockDialog::~DockDialog(): Exception thrown when trying to remove from dock manager...?");
	}

	delete widget_comp_select;

	DEBUGPOINT();
}

void
DockDialog::drop_on_prepend(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time)
{
	if ((selection_data.get_length() >= 0) && (selection_data.get_format() == 8))
	{
		Dockable& dockable(**reinterpret_cast<Dockable**>(const_cast<guint8*>(selection_data.get_data())));
		prepend_dock_book()->add(dockable);
		context->drag_finish(true, false, time);
		return;
	}
	
	context->drag_finish(false, false, time);
}

void
DockDialog::drop_on_append(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time)
{
	if ((selection_data.get_length() >= 0) && (selection_data.get_format() == 8))
	{
		Dockable& dockable(**reinterpret_cast<Dockable**>(const_cast<guint8*>(selection_data.get_data())));
		append_dock_book()->add(dockable);
		context->drag_finish(true, false, time);
		return;
	}
	
	context->drag_finish(false, false, time);
}


void
DockDialog::on_hide()
{
	Gtk::Window::on_hide();
	close();
}

DockBook*
DockDialog::prepend_dock_book()
{
	if(is_deleting)return 0;
		
	dock_book_list.push_front(new DockBook);
	last_dock_book=dock_book_list.front();


	last_dock_book->signal_empty().connect(
		sigc::bind(
			sigc::mem_fun(*this,&DockDialog::erase_dock_book),
			last_dock_book
		)
	);

	dock_book_sizes_.insert(dock_book_sizes_.begin(),225);
	refresh();
	return last_dock_book;
}

DockBook*
DockDialog::append_dock_book()
{
	if(is_deleting)return 0;
		
	dock_book_list.push_back(new DockBook);
	last_dock_book=dock_book_list.back();
	last_dock_book->signal_empty().connect(
		sigc::bind(
			sigc::mem_fun(*this,&DockDialog::erase_dock_book),
			last_dock_book
		)
	);
	last_dock_book->signal_changed().connect(
		sigc::mem_fun(*this,&DockDialog::refresh_title)
	);
	last_dock_book->signal_changed().connect(
		sigc::mem_fun(*this,&DockDialog::refresh_title)
	);
	dock_book_sizes_.push_back(225);

	//last_dock_book->show();
	refresh();
	return last_dock_book;
}

void
DockDialog::erase_dock_book(DockBook* dock_book)
{
	if(is_deleting)return;

	std::list<DockBook*>::iterator iter;
	for(iter=dock_book_list.begin();iter!=dock_book_list.end();++iter)
		if(*iter==dock_book)
		{
			dock_book_list.erase(iter);

			if(dock_book_list.empty())
			{
				last_dock_book=0;
				close();
				return;
			}
			else
			{
				if(last_dock_book==dock_book)
					last_dock_book=dock_book_list.front();
			}
			
			refresh();
			
			return;
		}
}

void
DockDialog::refresh()
{
	sinfg::info("dock_book_list.size()=%d",dock_book_list.size());
	//remove();

	if(dock_book_list.empty())
		return;
	
	if(box)delete box;
	box=(manage(is_horizontal?(Gtk::Box*)new Gtk::HBox:(Gtk::Box*)new Gtk::VBox));
	add(*box);
	
	box->pack_start(*widget_comp_select,false,true);

	Gtk::Button* append_button(manage(new Gtk::Button));
	Gtk::Button* prepend_button(manage(new Gtk::Button));
	
	std::list<Gtk::TargetEntry> listTargets;
	listTargets.push_back( Gtk::TargetEntry("DOCK") );

	append_button->drag_dest_set(listTargets);
	prepend_button->drag_dest_set(listTargets);

	append_button->signal_drag_data_received().connect(
		sigc::mem_fun(*this,&DockDialog::drop_on_append)
	);

	prepend_button->signal_drag_data_received().connect(
		sigc::mem_fun(*this,&DockDialog::drop_on_prepend)
	);
	
	box->pack_start(*prepend_button,false,true);
	box->pack_end(*append_button,false,true);

	//prepend_button->show();
	//append_button->show();
	pannels_.clear();
	
	if(dock_book_list.size()==1)
	{
		box->pack_start(get_dock_book(),true,true);
	}
	else
	{
		Gtk::Paned* parent(manage(is_horizontal?(Gtk::Paned*)new Gtk::HPaned:(Gtk::Paned*)new Gtk::VPaned));
		
		pannels_.push_back(parent);
		
		if(pannels_.size()<=dock_book_sizes_.size())
			pannels_.back()->set_position(dock_book_sizes_[pannels_.size()-1]);
		pannels_.back()->property_position().signal_changed().connect(
			sigc::mem_fun(*this,&DockDialog::rebuild_sizes)
		);
		//parent->show();
		parent->add1(*dock_book_list.front());
		//dock_book_list.front()->show();
	 
		box->pack_start(*parent,true,true);
		
		std::list<DockBook*>::iterator iter,next;
		for(next=dock_book_list.begin(),next++,iter=next++;next!=dock_book_list.end();iter=next++)
		{
			Gtk::Paned* current(manage(is_horizontal?(Gtk::Paned*)new Gtk::HPaned:(Gtk::Paned*)new Gtk::VPaned));
			pannels_.push_back(current);
			
			if(pannels_.size()<=dock_book_sizes_.size())
				pannels_.back()->set_position(dock_book_sizes_[pannels_.size()-1]);
			pannels_.back()->property_position().signal_changed().connect(
				sigc::mem_fun(*this,&DockDialog::rebuild_sizes)
			);


			parent->add2(*current);

			current->add1(**iter);
			//(*iter)->show();
			//current->show();
			
			parent=current;
		}
		parent->add2(**iter);
		//(*iter)->show();
	}
	
	box->show_all();
	if(!composition_selector_)
		widget_comp_select->hide();
	rebuild_sizes();
}

void
DockDialog::rebuild_sizes()
{
	unsigned int i=0;
	dock_book_sizes_.clear();
	for(i=0;i<pannels_.size();i++)
	{
		dock_book_sizes_.push_back(pannels_[i]->get_position());
	}
}

void
DockDialog::set_dock_book_sizes(const std::vector<int>& new_sizes)
{
	unsigned int i=0;
	for(i=0;i<pannels_.size() && i<new_sizes.size();i++)
	{
		pannels_[i]->set_position(new_sizes[i]);
	}
	dock_book_sizes_=new_sizes;
	//rebuild_sizes();
}

void
DockDialog::refresh_accel_group()
{
/*
	if(last_accel_group_)
	{
		last_accel_group_->unlock();
		remove_accel_group(last_accel_group_);
		last_accel_group_=Glib::RefPtr<Gtk::AccelGroup>();
	}
	
	etl::loose_handle<CanvasView> canvas_view(App::get_selected_canvas_view());
	if(canvas_view)
	{
		last_accel_group_=canvas_view->get_accel_group();
		last_accel_group_->lock();
		add_accel_group(last_accel_group_);
	}
*/
	etl::loose_handle<CanvasView> canvas_view(App::get_selected_canvas_view());
	if(canvas_view)
	{
		canvas_view->mainmenu.accelerate(*this);
	}
}

bool
DockDialog::close()
{
	sinfg::info("DockDialog::close(): DELETED!");
	empty_sig.disconnect();
	//get_dock_book().clear();
	delete this;	
	return true;
}

DockBook&
DockDialog::get_dock_book()
{
	if(!last_dock_book)
		return *append_dock_book();
	return *last_dock_book;
}

const DockBook&
DockDialog::get_dock_book()const
{
	return *last_dock_book;
}


sinfg::String
DockDialog::get_contents()const
{
	sinfg::String ret;

	std::list<DockBook*>::const_iterator iter;
	for(iter=dock_book_list.begin();iter!=dock_book_list.end();++iter)
	{
		if(!ret.empty())
			ret+=is_horizontal?" | ":" - ";
		ret+=(*iter)->get_contents();
	}
		
	
	return ret;
}

void
DockDialog::set_contents(const sinfg::String& z)
{
	int x,y;
	get_size(x,y);

	sinfg::String str(z);
	while(!str.empty())
	{
		unsigned int separator=str.find_first_of('-');
		{
			unsigned int sep2=str.find_first_of('|');
			if(separator!=sinfg::String::npos || sep2!=sinfg::String::npos)
			{
				if((separator==sinfg::String::npos || sep2<separator) && sep2!=sinfg::String::npos)
				{
					separator=sep2;
					is_horizontal=true;
				}
				else
					is_horizontal=false;
			}
		}
		
		sinfg::String book_contents;
		if(separator==sinfg::String::npos)
		{
			book_contents=str;
			str.clear();
		}
		else
		{
			book_contents=String(str.begin(),str.begin()+separator);
			str=String(str.begin()+separator+1,str.end());
		}
		
		try
		{
			append_dock_book()->set_contents(book_contents);
		}catch(...) { }
	}

	resize(x,y);
}

void
DockDialog::set_composition_selector(bool x)
{
	if(x==get_composition_selector())
		return;
	composition_selector_=x;
	if(x)
		widget_comp_select->show();
	else
		widget_comp_select->hide();
}

void
DockDialog::refresh_title()
{
	if(is_deleting)return;
	if(dock_book_list.size())
	{
		sinfg::String title;

		std::list<DockBook*>::const_iterator iter;
		for(iter=dock_book_list.begin();iter!=dock_book_list.end();++iter)
		{
			if(!title.empty())
				title+=", ";
			title+=(*iter)->get_local_contents();
		}
		set_title(title);
	}
	else
		set_title(_("Empty Dock Dialog"));
}
