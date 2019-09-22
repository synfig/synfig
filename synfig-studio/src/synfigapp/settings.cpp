/* === S Y N F I G ========================================================= */
/*!	\file settings.cpp
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

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sys/stat.h>
#include "settings.h"
#include <synfig/general.h>
#include <synfig/guid.h>

#include <synfigapp/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Settings::Settings()
{
}

Settings::~Settings()
{
}

synfig::String
Settings::get_value(const synfig::String& key)const
{
	synfig::String value;
	if(!get_value(key,value))
		return synfig::String();
	return value;
}

void
Settings::add_domain(Settings* domain, const synfig::String& name)
{
	domain_map[name]=domain;
}

void
Settings::remove_domain(const synfig::String& name)
{
	domain_map.erase(name);
}

bool
Settings::get_value(const synfig::String& key, synfig::String& value)const
{
	// Search for the value in any children domains
	DomainMap::const_iterator iter;
	for(iter=domain_map.begin();iter!=domain_map.end();++iter)
	{
		// if we have a domain hit
		if(key.size()>iter->first.size() && String(key.begin(),key.begin()+iter->first.size())==iter->first)
		{
			synfig::String key_(key.begin()+iter->first.size()+1,key.end());

			// If the domain has it, then we have got a hit
			if(iter->second->get_value(key_,value))
				return true;
		}
	}

	// Search for the value in our simple map
	if(simple_value_map.count(key))
	{
		value=simple_value_map.find(key)->second;
		return true;
	}

	// key not found
	return false;
}

bool
Settings::set_value(const synfig::String& key,const synfig::String& value)
{
	// Search for the key in any children domains
	DomainMap::iterator iter;
	for(iter=domain_map.begin();iter!=domain_map.end();++iter)
	{
		// if we have a domain hit
		if(key.size()>iter->first.size() && String(key.begin(),key.begin()+iter->first.size())==iter->first)
		{
			synfig::String key_(key.begin()+iter->first.size()+1,key.end());

			return iter->second->set_value(key_,value);
		}
	}

	simple_value_map[key]=value;
	return true;
}

//! Compare two key names, putting pref.* keys first
static bool
compare_pref_first (synfig::String first, synfig::String second)
{
	return	first.substr(0, 5) == "pref."
			?	second.substr(0, 5) == "pref."
				?	first < second
				:	true
			:	second.substr(0, 5) == "pref."
				?	false
				:	first < second;
}

Settings::KeyList
Settings::get_key_list()const
{
	KeyList key_list;

	// Get keys from the domains
	{
		DomainMap::const_iterator iter;
		for(iter=domain_map.begin();iter!=domain_map.end();++iter)
		{
			KeyList sub_key_list(iter->second->get_key_list());
			KeyList::iterator key_iter;
			for(key_iter=sub_key_list.begin();key_iter!=sub_key_list.end();++key_iter)
				key_list.push_back(iter->first+'.'+*key_iter);
		}
	}

	// Get keys from the simple variables
	{
		ValueBaseMap::const_iterator iter;
		for(iter=simple_value_map.begin();iter!=simple_value_map.end();++iter)
			key_list.push_back(iter->first);
	}

	// Sort the keys
	// We make sure the 'pref.*' keys come first to fix bug 1694393,
	// where windows were being created before the parameter
	// specifying which window manager hint to use had been loaded
	key_list.sort(compare_pref_first);

	return key_list;
}

bool
Settings::save_to_file(const synfig::String& filename)const
{
	synfig::String tmp_filename(filename+".TMP");

	try
	{
		std::ofstream file(tmp_filename.c_str());

		if(!file)return false;

		KeyList key_list(get_key_list());

		// Save the keys
		{
			KeyList::const_iterator iter;
			for(iter=key_list.begin();iter!=key_list.end();++iter)
			{
				if(!file)return false;
				String ret = get_value(*iter);
				if (ret != String()) file<<(*iter).c_str()<<'='<<(ret == "none" ? "normal" : ret).c_str()<<endl;
			}
		}

		if(!file)
			return false;
	}catch(...) { return false; }

#ifdef _WIN32

	// On Win32 platforms, rename() has bad behavior. Work around it.

	// Make random filename and ensure there's no file with such name exist
	struct stat buf;
	String old_file;
	do {
		synfig::GUID guid;
		old_file = filename+"."+guid.get_string().substr(0,8);
	} while (stat(old_file.c_str(), &buf) != -1);

	rename(filename.c_str(),old_file.c_str());
	if(rename(tmp_filename.c_str(),filename.c_str())!=0)
	{
		rename(old_file.c_str(),filename.c_str());
		return false;
	}
	remove(old_file.c_str());
#else
	if(rename(tmp_filename.c_str(),filename.c_str())!=0)
		return false;
#endif

	return true;
}

bool
Settings::load_from_file(const synfig::String& filename, const synfig::String& key_filter )
{
	std::ifstream file(filename.c_str());
	if(!file)
		return false;
	bool loaded_filter = false;
	while(file)
	{
		std::string line;
		getline(file,line);
		if(!line.empty() && ((line[0]>='a' && line[0]<='z')||
							 (line[0]>='A' && line[0]<='Z')||
							 (line[0]>='0' && line[0]<='9'))
							)
		{
			std::string::iterator equal(find(line.begin(),line.end(),'='));
			if(equal==line.end())
				continue;
			std::string key(line.begin(),equal);
			std::string value(equal+1,line.end());

			//synfig::info("Settings::load_from_file(): Trying Key \"%s\" with a value of \"%s\".",key.c_str(),value.c_str());
			try{
				if (key_filter=="" || (key==key_filter) )
				{
					if(!set_value(key,value))
						synfig::warning("Settings::load_from_file(): Key \"%s\" with a value of \"%s\" was rejected.",key.c_str(),value.c_str());
					else
						loaded_filter = true;
				}
			}
			catch(...)
			{
				synfig::error("Settings::load_from_file(): Attempt to set key \"%s\" with a value of \"%s\" has thrown an exception.",key.c_str(),value.c_str());
				throw;
			}
		}
	}
	if (!key_filter.empty() && !loaded_filter)
		return false;
	return true;
}
