/* === S I N F G =========================================================== */
/*!	\file dockmanager.cpp
**	\brief Template File
**
**	$Id: dockmanager.cpp,v 1.2 2005/01/12 07:03:42 darco Exp $
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

#include "dockmanager.h"
#include <stdexcept>
#include "dockable.h"
#include "dockdialog.h"
#include <sinfgapp/settings.h>
#include <sinfgapp/main.h>
#include <gdkmm/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === P R O C E D U R E S ================================================= */

class studio::DockSettings : public sinfgapp::Settings
{
	DockManager* dock_manager;
	
public:
	DockSettings(DockManager* dock_manager):dock_manager(dock_manager)
	{
		sinfgapp::Main::settings().add_domain(this,"dock");
	}
	
	virtual ~DockSettings()
	{
		sinfgapp::Main::settings().remove_domain("dock");
	}
#define SCALE_FACTOR	(1280)
	virtual bool get_value(const sinfg::String& key_, sinfg::String& value)const
	{
		int screen_w(Gdk::screen_width());
		int screen_h(Gdk::screen_height());
		
		if(key_.size()>6 && String(key_.begin(),key_.begin()+6)=="dialog")try
		{
			sinfg::String key(key_.begin()+7,key_.end());
			int separator=key.find_first_of('.');
			int id(atoi(sinfg::String(key.begin(),key.begin()+separator).c_str()));
			key=sinfg::String(key.begin()+separator+1,key.end());
			
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
		return sinfgapp::Settings::get_value(key_,value);
	}

	virtual bool set_value(const sinfg::String& key_,const sinfg::String& value)
	{
		int screen_w(Gdk::screen_width());
		int screen_h(Gdk::screen_height());

		if(key_.size()>6 && String(key_.begin(),key_.begin()+6)=="dialog")
		{
			sinfg::String key(key_.begin()+7,key_.end());
			int separator=key.find_first_of('.');
			int id(atoi(sinfg::String(key.begin(),key.begin()+separator).c_str()));
			key=sinfg::String(key.begin()+separator+1,key.end());
			
			DockDialog& dock_dialog(dock_manager->find_dock_dialog(id));

			if(key=="contents_size")
			{
				try {
					
				vector<int> data;
				int n=0;
				String value_(value);
				while(value_.size() && (signed)value_.size()>n && n>=0){
					value_=String(value_.begin()+n,value_.end());
					int size;
					if(!strscanf(value_,"%d",&size))
						break;
					size=size*screen_h/SCALE_FACTOR;
					data.push_back(size);

					n=value_.find(" ");
					if((unsigned)n!=String::npos)
						n++;
						
				}
				dock_dialog.set_dock_book_sizes(data);
				}
				catch(...)
				{
					sinfg::error("Exception caught!!!");
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
		return sinfgapp::Settings::set_value(key_,value);
	}
	
	virtual KeyList get_key_list()const
	{
		sinfgapp::Settings::KeyList ret(sinfgapp::Settings::get_key_list());

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
		sinfg::info("DockManager::~DockManager(): Deleting dockable \"%s\"",dockable->get_name().c_str());
		dockable_list_.pop_back();		
		delete dockable;
	}
}

void
DockManager::register_dockable(Dockable& x)
{
	dockable_list_.push_back(&x);
	sinfg::info("DockManager::register_dockable(): Registered dockable \"%s\"",dockable_list_.back()->get_name().c_str());
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
			sinfg::info("DockManager::unregister_dockable(): \"%s\" has been Unregistered",x.get_name().c_str());
			return true;
		}
	}
	return false;
}

Dockable&
DockManager::find_dockable(const sinfg::String& x)
{
	std::list<Dockable*>::iterator iter;
	for(iter=dockable_list_.begin();iter!=dockable_list_.end();++iter)
		if((*iter)->get_name()==x)
			return **iter;
	
	throw std::runtime_error("DockManager::find_dockable(): not found");
}

void
DockManager::present(sinfg::String x)
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
