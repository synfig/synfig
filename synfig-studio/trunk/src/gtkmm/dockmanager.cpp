/* === S Y N F I G ========================================================= */
/*!	\file dockmanager.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "dockmanager.h"
#include <stdexcept>
#include "dockable.h"
#include "dockdialog.h"
#include <synfigapp/settings.h>
#include <synfigapp/main.h>
#include <gdkmm/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === P R O C E D U R E S ================================================= */

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
#define SCALE_FACTOR	(1280)
	virtual bool get_value(const synfig::String& key_, synfig::String& value)const
	{
		int screen_w(Gdk::screen_width());
		int screen_h(Gdk::screen_height());

		if(key_.size()>6 && String(key_.begin(),key_.begin()+6)=="dialog")try
		{
			synfig::String key(key_.begin()+7,key_.end());
			synfig::String::size_type separator=key.find_first_of('.');
			int id(atoi(synfig::String(key.begin(),key.begin()+separator).c_str()));
			key=synfig::String(key.begin()+separator+1,key.end());

			DockDialog& dock_dialog(dock_manager->find_dock_dialog(id));

			if(key=="contents_size")
			{
				dock_dialog.rebuild_sizes();
				vector<int>::const_iterator iter(dock_dialog.get_dock_book_sizes().begin());
				vector<int>::const_iterator end(dock_dialog.get_dock_book_sizes().end());
				value.clear();
				for(;iter!=end;++iter)
					value+=strprintf("%d ",(*iter)*SCALE_FACTOR/screen_h);
				return true;
			}
			if(key=="pos")
			{
				int x,y; dock_dialog.get_position(x,y);
				value=strprintf("%d %d",x*SCALE_FACTOR/screen_w,y*SCALE_FACTOR/screen_h);
				return true;
			}
			if(key=="size")
			{
				int x,y; dock_dialog.get_size(x,y);
				value=strprintf("%d %d",x*SCALE_FACTOR/screen_w,y*SCALE_FACTOR/screen_h);
				return true;
			}
			if(key=="contents")
			{
				value=dock_dialog.get_contents();
				return true;
			}
			if(key=="comp_selector")
			{
				value=dock_dialog.get_composition_selector()?"1":"0";
				return true;
			}
		}catch (...) { return false; }
		return synfigapp::Settings::get_value(key_,value);
	}

	virtual bool set_value(const synfig::String& key_,const synfig::String& value)
	{
		int screen_w(Gdk::screen_width());
		int screen_h(Gdk::screen_height());

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

				vector<int> data;
				String::size_type n=0;
				String value_(value);
				while(value_.size() && value_.size()>n){
					value_=String(value_.begin()+n,value_.end());
					int size;
					if(!strscanf(value_,"%d",&size))
						break;
					size=size*screen_h/SCALE_FACTOR;
					data.push_back(size);

					n=value_.find(" ");
					if(n==String::npos)
						break;
					n++;
				}
				dock_dialog.set_dock_book_sizes(data);
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
				x=x*screen_w/SCALE_FACTOR;
				y=y*screen_h/SCALE_FACTOR;
				dock_dialog.move(x,y);
				return true;
			}
			if(key=="size")
			{
				int x,y;
				if(!strscanf(value,"%d %d",&x, &y))
					return false;
				x=x*screen_w/SCALE_FACTOR;
				y=y*screen_h/SCALE_FACTOR;
				dock_dialog.set_default_size(x,y);
				dock_dialog.resize(x,y);
				return true;
			}
			if(key=="contents")
			{
				dock_dialog.set_contents(value);
				return true;
			}
			if(key=="comp_selector")
			{
				if(value.empty() || value[0]=='0')
					dock_dialog.set_composition_selector(false);
				else
					dock_dialog.set_composition_selector(true);
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
		synfig::info("DockManager::~DockManager(): Deleting dockable \"%s\"",dockable->get_name().c_str());
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
			x.detach();
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
	dock_dialog->show();
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
