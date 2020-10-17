/* === S Y N F I G ========================================================= */
/*!	\file cvs.cpp
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

#include <synfig/general.h>

#include "cvs.h"
#include <ETL/stringf>
#include <fstream>
#include <iostream>
#include <cstdlib>


#include <sys/types.h>
#include <sys/stat.h>

#include <cassert>

#include <synfigapp/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

#define cvs_command		synfig::String("cvs -z4")

#ifndef _WIN32
#define HAVE_STRPTIME
#endif

#if defined(__APPLE__) || defined(__OpenBSD__)
time_t _daylight_() { time_t t(time(0)); return localtime(&t)->tm_gmtoff; }
#define daylight _daylight_()
#elif defined(_MSC_VER)
time_t _daylight_() { time_t t(time(0)); return localtime(&t)->tm_isdst; }
#define daylight _daylight_()
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

CVSInfo::CVSInfo(const synfig::String& file_name)
{
	update_available_=false;
	set_file_name(file_name);
}

CVSInfo::CVSInfo():
	in_sandbox_(),
	in_repository_(),
	update_available_(false),
	original_timestamp_()
{ }

CVSInfo::~CVSInfo()
{
}

void
CVSInfo::set_file_name(const synfig::String& file_name)
{
	file_name_=file_name;

	std::ifstream file((dirname(file_name_)+"/CVS/Root").c_str());

	if(file)
	{
		in_sandbox_=true;
		calc_repository_info();
	}
	else
		in_sandbox_=false;
}

void
CVSInfo::calc_repository_info()
{
#ifdef _DEBUG
	synfig::info("in_sandbox() = %d",in_sandbox());
#endif

	if(!in_sandbox_)
		return;

	std::ifstream file((dirname(file_name_)+"/CVS/Entries").c_str());

	while(file)
	{
		String line;
		getline(file,line);
		if(line.find(basename(file_name_))!=String::npos)
		{
			in_repository_=true;
			String::size_type s,f;

			// Grab the version
			s=line.find('/',1);
			assert(s!=String::npos);
			s++;
			f=line.find('/',s+1);
			assert(f!=String::npos);
			cvs_version_=String(line,s,f-s);

			// Grab the time
#ifdef HAVE_STRPTIME
			s=f+1;
			f=line.find('/',s+1);
			assert(f!=String::npos);
			tm time_struct;
			strptime(String(line,s,f-s).c_str(),"%c",&time_struct);
			original_timestamp_=mktime(&time_struct);
#endif

			if(
				system(strprintf(
					"cd '%s' && cvs status '%s' | grep -q -e 'Needs Patch'",
					dirname(file_name_).c_str(),
					basename(file_name_).c_str()
				).c_str())==0
			)
			{
				synfig::info("UPDATE_AVAILABLE=TRUE");
				update_available_=true;
			}
			else
			{
				system(strprintf(
					"cd '%s' && cvs status '%s'",
					dirname(file_name_).c_str(),
					basename(file_name_).c_str()
				).c_str());
				synfig::info("UPDATE_AVAILABLE=FALSE");
				update_available_=false;
			}


#ifdef _DEBUG
			synfig::info("in_repository() = %d",in_repository());
			synfig::info("get_cvs_version() = %s",get_cvs_version().c_str());
			synfig::info("get_original_timestamp() = %s",ctime(&get_original_timestamp()));
			time_t t(get_current_timestamp());
			synfig::info("get_current_timestamp() = %s",ctime(&t));
			synfig::info("get_cvs_root() = %s",get_cvs_root().c_str());
			synfig::info("get_cvs_module() = %s",get_cvs_module().c_str());
#endif
			return;
		}
	}

	in_repository_=false;
	cvs_version_.clear();
	original_timestamp_=0;

#ifdef _DEBUG
	synfig::info("in_repository() = %d",in_repository());
#endif
}

bool
CVSInfo::in_sandbox()const
{
	return in_sandbox_;
}

bool
CVSInfo::in_repository()const
{
	if(!in_sandbox_)
		return false;
	return in_repository_;
}

bool
CVSInfo::is_modified()const
{
	if(!in_sandbox() || !in_repository())
		return false;
#ifdef _DEBUG
	synfig::info("%d-%d=%d",get_current_timestamp(),get_original_timestamp(),get_current_timestamp()-get_original_timestamp());
#endif
	return get_current_timestamp()!=get_original_timestamp() && abs(get_current_timestamp()-get_original_timestamp())!=3600;
}

bool
CVSInfo::is_updated()const
{
	return update_available_;
}

const synfig::String&
CVSInfo::get_cvs_version()const
{
	return cvs_version_;
}

const time_t&
CVSInfo::get_original_timestamp()const
{
	return original_timestamp_;
}

time_t
CVSInfo::get_current_timestamp()const
{
	struct stat st;
	if(stat(file_name_.c_str(),&st)<0)
	{
		synfig::error("Unable to get file stats");
		return false;
	}
	time_t ret((daylight-1)*3600);
	//ret+=timezone;
	ret+=st.st_mtime;
	return ret;
}

synfig::String
CVSInfo::get_cvs_root()const
{
	if(!in_sandbox_)
		return synfig::String();

	std::ifstream file((dirname(file_name_)+"/CVS/Root").c_str());

	if(file)
	{
		String ret;
		getline(file,ret);
		return ret;
	}

	return synfig::String();
}

synfig::String
CVSInfo::get_cvs_module()const
{
	if(!in_sandbox_)
		return synfig::String();

	std::ifstream file((dirname(file_name_)+"/CVS/Repository").c_str());

	if(file)
	{
		String ret;
		getline(file,ret);
		return ret;
	}

	return synfig::String();
}

// This function pre-processes the message so that we
// don't get any CVS syntax errors.
inline synfig::String fix_msg(const synfig::String& message)
{
	synfig::String ret;
	int i;
	for(i=0;i<(int)message.size();i++)
	{
		if(message[i]=='\'')
			ret+="'\"'\"'";
		else
			ret+=message[i];
	}
	return ret;
}

void
CVSInfo::cvs_add(const synfig::String& message)
{
	if(!in_sandbox_)
	{
		synfig::error("cvs_add(): Not in a sand box");
		throw int();
		return;
	}

	synfig::String command(strprintf("cd '%s' && %s add -m '%s' '%s'",dirname(file_name_).c_str(),cvs_command.c_str(),fix_msg(message).c_str(),basename(file_name_).c_str()));

	int ret(system(command.c_str()));

	calc_repository_info();

	switch(ret)
	{
	case 0:
		break;
	default:
		synfig::error("Unknown errorcode %d (\"%s\")",ret,command.c_str());
		throw(ret);
		break;
	}
}

void
CVSInfo::cvs_update()
{
	if(!in_sandbox_)
	{
		synfig::error("cvs_update(): Not in a sand box");
		throw int();
		return;
	}

	synfig::String command(strprintf("cd '%s' && %s update '%s'",dirname(file_name_).c_str(),cvs_command.c_str(),basename(file_name_).c_str()));

	int ret(system(command.c_str()));

	calc_repository_info();

	switch(ret)
	{
	case 0:
		break;
	default:
		synfig::error("Unknown errorcode %d (\"%s\")",ret,command.c_str());
		throw(ret);
		break;
	}
}

void
CVSInfo::cvs_commit(const synfig::String& message)
{
	if(!in_sandbox_)
	{
		synfig::error("cvs_commit(): Not in a sand box");
		throw int();
		return;
	}

	synfig::String command(strprintf("cd '%s' && %s commit -m '%s' '%s'",dirname(file_name_).c_str(),cvs_command.c_str(),fix_msg(message).c_str(),basename(file_name_).c_str()));

	int ret(system(command.c_str()));

	calc_repository_info();

	switch(ret)
	{
	case 0:
		break;
	default:
		synfig::error("Unknown errorcode %d (\"%s\")",ret,command.c_str());
		if(is_modified())
			throw(ret);
		break;
	}
}
