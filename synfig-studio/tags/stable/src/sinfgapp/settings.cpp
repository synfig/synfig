/* === S I N F G =========================================================== */
/*!	\file template.cpp
**	\brief Template File
**
**	$Id: settings.cpp,v 1.2 2005/01/12 04:08:32 darco Exp $
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

#include <fstream>
#include <iostream>
#include "settings.h"
#include <sinfg/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;

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

sinfg::String
Settings::get_value(const sinfg::String& key)const
{
	sinfg::String value;
	if(!get_value(key,value))
		return sinfg::String();
	return value;
}

void
Settings::add_domain(Settings* domain, const sinfg::String& name)
{
	domain_map[name]=domain;
}

void
Settings::remove_domain(const sinfg::String& name)
{
	domain_map.erase(name);
}

bool
Settings::get_value(const sinfg::String& key, sinfg::String& value)const
{
	// Search for the value in any children domains
	DomainMap::const_iterator iter;
	for(iter=domain_map.begin();iter!=domain_map.end();++iter)
	{
		// if we have a domain hit
		if(key.size()>iter->first.size() && String(key.begin(),key.begin()+iter->first.size())==iter->first)
		{
			sinfg::String key_(key.begin()+iter->first.size()+1,key.end());
			
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
Settings::set_value(const sinfg::String& key,const sinfg::String& value)
{
	// Search for the key in any children domains
	DomainMap::iterator iter;
	for(iter=domain_map.begin();iter!=domain_map.end();++iter)
	{
		// if we have a domain hit
		if(key.size()>iter->first.size() && String(key.begin(),key.begin()+iter->first.size())==iter->first)
		{
			sinfg::String key_(key.begin()+iter->first.size()+1,key.end());
			
			return iter->second->set_value(key_,value);
		}
	}

	simple_value_map[key]=value;
	return true;
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
	key_list.sort();
	
	return key_list;
}

bool
Settings::save_to_file(const sinfg::String& filename)const
{
	sinfg::String tmp_filename(filename+".TMP");
	
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
				file<<*iter<<'='<<get_value(*iter)<<endl;
			}
		}
	
		if(!file)
			return false;
	}catch(...) { return false; }
	
#ifdef _WIN32
	char old_file[80]="sif.XXXXXXXX";
	mktemp(old_file);
	rename(filename.c_str(),old_file);	
	if(rename(tmp_filename.c_str(),filename.c_str())!=0)
	{
		rename(old_file,tmp_filename.c_str());
		return false;
	}
	remove(old_file);
#else
	if(rename(tmp_filename.c_str(),filename.c_str())!=0)
		return false;
#endif
	
	return true;
}

bool
Settings::load_from_file(const sinfg::String& filename)
{
	std::ifstream file(filename.c_str());
	if(!file)
		return false;
	while(file)
	{
		std::string line;
		getline(file,line);
		if(!line.empty() && ((line[0]>='a' && line[0]<='z')||(line[0]>='A' && line[0]<='Z')))
		{
			std::string::iterator equal(find(line.begin(),line.end(),'='));
			if(equal==line.end())
				continue;
			std::string key(line.begin(),equal);
			std::string value(equal+1,line.end());
			
			//sinfg::info("Settings::load_from_file(): Trying Key \"%s\" with a value of \"%s\".",key.c_str(),value.c_str());
			try{
			if(!set_value(key,value))
				sinfg::warning("Settings::load_from_file(): Key \"%s\" with a value of \"%s\" was rejected.",key.c_str(),value.c_str());
			}
			catch(...)
			{
				sinfg::error("Settings::load_from_file(): Attept to set key \"%s\" with a value of \"%s\" has thrown an exception.",key.c_str(),value.c_str());
				throw;
			}
		}
	}
	return true;
}
