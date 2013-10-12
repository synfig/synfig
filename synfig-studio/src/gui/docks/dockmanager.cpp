/* === S Y N F I G ========================================================= */
/*!	\file dockmanager.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "docks/dockmanager.h"
#include <stdexcept>
#include "docks/dockable.h"
#include "docks/dockbook.h"
#include "docks/dockdialog.h"
#include <synfigapp/settings.h>
#include <synfigapp/main.h>
#include <gdkmm/general.h>

#include "general.h"

#include <gtkmm/paned.h>
#include <gtkmm/box.h>
#include <gtkmm/window.h>


#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === P R O C E D U R E S ================================================= */

namespace studio {
	class DockLinkPoint {
	public:
		Gtk::Bin *bin;
		Gtk::Paned *paned;
		Gtk::Window *window;
		bool is_first;

		DockLinkPoint(): bin(NULL), paned(NULL), window(NULL), is_first(false) { }
		explicit DockLinkPoint(Gtk::Bin *bin): bin(bin), paned(NULL), window(NULL), is_first(false) { }
		explicit DockLinkPoint(Gtk::Paned *paned, bool is_first): bin(NULL), paned(paned), window(NULL), is_first(is_first) { }
		explicit DockLinkPoint(Gtk::Window *window): bin(NULL), paned(NULL), window(window), is_first(false) { }
		explicit DockLinkPoint(Gtk::Widget &widget) {
			Gtk::Container *container = widget.get_parent();
			bin = dynamic_cast<Gtk::Bin*>(container);
			paned = dynamic_cast<Gtk::Paned*>(container);
			window = dynamic_cast<Gtk::Window*>(container);
			is_first = paned != NULL && paned->get_child1() == &widget;
		}

		bool is_valid() { return bin || paned || window; }

		void unlink() {
			if (paned && is_first && paned->get_child1())
				paned->remove(*paned->get_child1());
			else
			if (paned && !is_first && paned->get_child2())
				paned->remove(*paned->get_child2());
			else
			if (window)
				window->remove();
			if (bin)
				bin->remove();
		}

		void link(Gtk::Widget &widget)
		{
			if (paned && is_first)
				paned->add1(widget);
			else
			if (paned && !is_first)
				paned->add2(widget);
			else
			if (window)
				window->add(widget);
			else
			if (bin)
				bin->add(widget);
		}
	};
}

class studio::DockSettings : public synfigapp::Settings
{
	DockManager* dock_manager;

public:
	DockSettings(DockManager* dock_manager):dock_manager(dock_manager)
	{
		synfigapp::Main::settings().add_domain(this,"dock");
	}

	virtual ~DockSettings()
	{
		synfigapp::Main::settings().remove_domain("dock");
	}
	virtual bool get_value(const synfig::String& key_, synfig::String& value)const
	{

		if(key_.size()>6 && String(key_.begin(),key_.begin()+6)=="dialog")try
		{
			synfig::String key(key_.begin()+7,key_.end());
			synfig::String::size_type separator=key.find_first_of('.');
			int id(atoi(synfig::String(key.begin(),key.begin()+separator).c_str()));
			key=synfig::String(key.begin()+separator+1,key.end());

			DockDialog& dock_dialog(dock_manager->find_dock_dialog(id));

			if(key=="contents_size")
			{
				// TODO:
				//dock_dialog.rebuild_sizes();
				//vector<int>::const_iterator iter(dock_dialog.get_dock_book_sizes().begin());
				//vector<int>::const_iterator end(dock_dialog.get_dock_book_sizes().end());
				//value.clear();
				//for(;iter!=end;++iter)
				//	value+=strprintf("%d ",*iter);
				return true;
			}
			if(key=="pos")
			{
				int x,y; dock_dialog.get_position(x,y);
				value=strprintf("%d %d",x,y);
				return true;
			}
			if(key=="size")
			{
				int x,y; dock_dialog.get_size(x,y);
				value=strprintf("%d %d",x,y);
				return true;
			}
			if(key=="contents")
			{
				// TODO:
				//value=dock_dialog.get_contents();
				return true;
			}
			if(key=="comp_selector")
			{
				// TODO:
				//value=dock_dialog.get_composition_selector()?"1":"0";
				return true;
			}
		}catch (...) { return false; }
		return synfigapp::Settings::get_value(key_,value);
	}

	virtual bool set_value(const synfig::String& key_,const synfig::String& value)
	{

		if(key_.size()>6 && String(key_.begin(),key_.begin()+6)=="dialog")
		{
			synfig::String key(key_.begin()+7,key_.end());
			synfig::String::size_type separator=key.find_first_of('.');
			int id(atoi(synfig::String(key.begin(),key.begin()+separator).c_str()));
			key=synfig::String(key.begin()+separator+1,key.end());

			DockDialog& dock_dialog(dock_manager->find_dock_dialog(id));

			if(key=="contents_size")
			{
				try {
				int width, height;
				Gtk::IconSize::lookup(Gtk::IconSize(4),width,height);
				vector<int> data;
				String::size_type n=0;
				String value_(value);
				while(value_.size() && value_.size()>n){
					value_=String(value_.begin()+n,value_.end());
					int size;
					if(!strscanf(value_,"%d",&size))
						break;

					data.push_back(size);

					n=value_.find(" ");
					if(n==String::npos)
						break;
					n++;
				}
				// TODO:
				//dock_dialog.set_dock_book_sizes(data);
				}
				catch(...)
				{
					synfig::error("Exception caught!!!");
					return false;
				}
				return true;
			}
			if(key=="pos")
			{
				int x,y;
				if(!strscanf(value,"%d %d",&x, &y))
					return false;
				//synfig::info("dock_manager. move to: %d, %d", x,y);
				dock_dialog.move(x,y);
				return true;
			}
			if(key=="size")
			{
				int x,y;
				if(!strscanf(value,"%d %d",&x, &y))
					return false;
				//synfig::info("dock_manager. size to: %d, %d", x,y);
				dock_dialog.set_default_size(x,y);
				dock_dialog.resize(x,y);
				return true;
			}
			if(key=="contents")
			{
				// TODO:
				//dock_dialog.set_contents(value);
				return true;
			}
			if(key=="comp_selector")
			{
				// TODO:
				//if(value.empty() || value[0]=='0')
				//	dock_dialog.set_composition_selector(false);
				//else
				//	dock_dialog.set_composition_selector(true);
				return true;
			}
		}
		return synfigapp::Settings::set_value(key_,value);
	}

	virtual KeyList get_key_list()const
	{
		synfigapp::Settings::KeyList ret(synfigapp::Settings::get_key_list());

		std::list<DockDialog*>::const_iterator iter;
		for(iter=dock_manager->dock_dialog_list_.begin();iter!=dock_manager->dock_dialog_list_.end();++iter)
		{
			ret.push_back(strprintf("dialog.%d.contents",(*iter)->get_id()));
			ret.push_back(strprintf("dialog.%d.comp_selector",(*iter)->get_id()));
			ret.push_back(strprintf("dialog.%d.pos",(*iter)->get_id()));
			ret.push_back(strprintf("dialog.%d.size",(*iter)->get_id()));
			ret.push_back(strprintf("dialog.%d.contents_size",(*iter)->get_id()));
		}
		return ret;
	}
};

/* === M E T H O D S ======================================================= */

DockManager::DockManager():
	dock_settings(new DockSettings(this))
{
}

DockManager::~DockManager()
{
	while(!dock_dialog_list_.empty())
	{
		dock_dialog_list_.back()->close();
	}
	while(!dockable_list_.empty())
	{
		Dockable* dockable(dockable_list_.back());
		// synfig::info("DockManager::~DockManager(): Deleting dockable \"%s\"",dockable->get_name().c_str());
		dockable_list_.pop_back();
		delete dockable;
	}
}

void
DockManager::register_dockable(Dockable& x)
{
	dockable_list_.push_back(&x);
	// synfig::info("DockManager::register_dockable(): Registered dockable \"%s\"",dockable_list_.back()->get_name().c_str());
	signal_dockable_registered()(&x);
}

bool
DockManager::unregister_dockable(Dockable& x)
{
	std::list<Dockable*>::iterator iter;
	for(iter=dockable_list_.begin();iter!=dockable_list_.end();++iter)
	{
		if(&x==*iter)
		{
			remove_widget_recursive(x);
			dockable_list_.erase(iter);
			synfig::info("DockManager::unregister_dockable(): \"%s\" has been Unregistered",x.get_name().c_str());
			return true;
		}
	}
	return false;
}

Dockable&
DockManager::find_dockable(const synfig::String& x)
{
	std::list<Dockable*>::iterator iter;
	for(iter=dockable_list_.begin();iter!=dockable_list_.end();++iter)
		if((*iter)->get_name()==x)
			return **iter;

	throw std::runtime_error("DockManager::find_dockable(): not found");
}

void
DockManager::present(synfig::String x)
{
	try
	{
		find_dockable(x).present();
	}
	catch(...)
	{
	}
}

DockDialog&
DockManager::find_dock_dialog(int id)
{
	std::list<DockDialog*>::iterator iter;
	for(iter=dock_dialog_list_.begin();iter!=dock_dialog_list_.end();++iter)
		if((*iter)->get_id()==id)
			return **iter;

	DockDialog* dock_dialog(new DockDialog());
	dock_dialog->set_id(id);
	return *dock_dialog;
}

const DockDialog&
DockManager::find_dock_dialog(int id)const
{
	std::list<DockDialog*>::const_iterator iter;
	for(iter=dock_dialog_list_.begin();iter!=dock_dialog_list_.end();++iter)
		if((*iter)->get_id()==id)
			return **iter;

	throw std::runtime_error("DockManager::find_dock_dialog(int id)const: not found");
}

void
DockManager::show_all_dock_dialogs()
{
	std::list<DockDialog*>::iterator iter;
	for(iter=dock_dialog_list_.begin();iter!=dock_dialog_list_.end();++iter)
		(*iter)->present();
}

bool
DockManager::swap_widgets(Gtk::Widget &widget1, Gtk::Widget &widget2)
{
	DockLinkPoint point1(widget1);
	DockLinkPoint point2(widget2);
	if (point1.is_valid() && point2.is_valid())
	{
		point1.unlink();
		point2.unlink();
		point1.link(widget2);
		point2.link(widget1);
		return true;
	}
	return false;
}

void
DockManager::remove_widget_recursive(Gtk::Widget &widget)
{
	DockLinkPoint link(widget);
	if (link.is_valid())
	{
		link.unlink();
		if (link.paned)
		{
			Gtk::Widget &widget = link.is_first
								? *link.paned->get_child2()
								: *link.paned->get_child1();
			DockLinkPoint paned_link(*link.paned);
			if (paned_link.is_valid())
			{
				link.paned->remove(widget);
				paned_link.unlink();
				paned_link.link(widget);
				delete link.paned;
			}
		}
		else
		if (link.window) link.window->hide();
	}
	else
	if (widget.get_parent())
	{
		DockBook *book = dynamic_cast<DockBook*>(widget.get_parent());
		widget.get_parent()->remove(widget);
		if (book && book->pages().empty())
		{
			remove_widget_recursive(*book);
			delete book;
		}
	}
}


bool
DockManager::add_widget(Gtk::Widget &dest_widget, Gtk::Widget &src_widget, bool vertical, bool first)
{
	if (&src_widget == &dest_widget) return false;

	// check for src widget is parent for dest_widget
	for(Gtk::Widget *parent = src_widget.get_parent(); parent != NULL; parent = parent->get_parent())
		if (parent == &dest_widget)
			return swap_widgets(src_widget, dest_widget);

	// unlink dest_widget
	DockLinkPoint dest_link(dest_widget);
	if (!dest_link.is_valid()) return false;
	dest_link.unlink();

	// unlink src_widget
	remove_widget_recursive(src_widget);

	// create new paned and link all
	Gtk::Paned *paned = manage(vertical ? (Gtk::Paned*)new Gtk::VPaned() : (Gtk::Paned*)new Gtk::HPaned());
	paned->show();
	DockLinkPoint(paned, first).link(src_widget);
	DockLinkPoint(paned, !first).link(dest_widget);
	dest_link.link(*paned);
	return true;
}

bool
DockManager::add_dockable(Gtk::Widget &dest_widget, Dockable &dockable, bool vertical, bool first)
{
	DockBook *book = manage(new DockBook());
	book->show();
	if (add_widget(dest_widget, *book, vertical, first))
	{
		book->add(dockable);
		return true;
	}
	delete book;
	return false;
}


