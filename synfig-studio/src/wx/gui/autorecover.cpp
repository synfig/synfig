/* === S Y N F I G ========================================================= */
/*!	\file autorecover.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "autorecover.h"

#ifdef __OpenBSD__
#include <errno.h>
#elif defined(HAVE_SYS_ERRNO_H)
#include <sys/errno.h>
#endif
//#include <unistd.h>
#include "app.h"
#include <synfig/savecanvas.h>
#include <synfig/loadcanvas.h>
#include <synfigapp/main.h>
#include <fstream>
#include <iostream>
#include "instance.h"

#include <glibmm/miscutils.h>
#include <glibmm/main.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifdef _WIN32
#define mkdir(x,y) mkdir(x)
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

AutoRecover::AutoRecover()
{
	// Three Minutes
	set_timeout(3*60*1000);

	if(mkdir(get_shadow_directory().c_str(),ACCESSPERMS)<0)
	{
		if(errno!=EEXIST)
			synfig::error("UNABLE TO CREATE \"%s\"",get_shadow_directory().c_str());
	}
	else
	{
		synfig::info("Created directory \"%s\"",get_shadow_directory().c_str());
	}
}

AutoRecover::~AutoRecover()
{
}

synfig::String
AutoRecover::get_shadow_directory()
{
	return Glib::build_filename(synfigapp::Main::get_user_app_directory(),"tmp");
}

int
AutoRecover::pid()
{
//	return getpid();
	return 0;
}

void
AutoRecover::set_timeout(int milliseconds)
{
	timeout=milliseconds;
	auto_backup_connect.disconnect();
	if(timeout && !getenv("SYNFIG_DISABLE_AUTO_SAVE"))
		auto_backup_connect=Glib::signal_timeout().connect(sigc::ptr_fun(&AutoRecover::auto_backup),timeout);
}

synfig::String
AutoRecover::get_shadow_file_name(const synfig::String& filename)
{
	unsigned int hash1(0xdeadbeef);
	unsigned int hash2(0x83502529);
	char* str_hash1(reinterpret_cast<char*>(&hash1));
	char* str_hash2(reinterpret_cast<char*>(&hash2));

	// First we need to hash up the directory
	{
		String pool(dirname(filename));

		while(pool.size()>4)
		{
			str_hash1[0]^=pool[1];str_hash1[1]^=pool[2];str_hash1[2]^=pool[3];str_hash1[3]^=pool[0];
			str_hash2[3]+=pool[0];str_hash2[2]+=pool[1];str_hash2[1]+=pool[2];str_hash2[0]+=pool[3];
			swap(hash1,hash2);
			pool=String(pool,4,pool.size());
		}
		while(pool.size())
		{
			str_hash1[0]^=pool[0];
			str_hash1[2]^=pool[0];
			str_hash2[1]^=pool[0];
			str_hash2[3]^=pool[0];
			swap(hash1,hash2);
			pool=String(pool,1,pool.size());
		}
	}
	hash1^=hash2;

	return Glib::build_filename(get_shadow_directory(),strprintf("%08X-%s",hash1,basename(filename).c_str()));

//	return dirname(filename) + ETL_DIRECTORY_SEPARATOR + ".shadow_" + basename(filename);
}

bool
AutoRecover::cleanup_pid(int pid)
{
#ifdef HAVE_FORK
	int status=0;
	if(waitpid(pid,&status,WNOHANG)==-1)
	{
		synfig::info("PID %d isn't a zombie yet",pid);
		return true;
	}
	if(WEXITSTATUS(status)!=0)
	{
		synfig::error("Autobackup seems to have failed! (PID=%d)",pid);
	}
//	else
//		synfig::info("PID=%d has been cleaned up",pid);
#endif
	return false;
}

bool
AutoRecover::auto_backup()
{
	int pid(0);

#ifdef HAVE_FORK
#undef HAVE_FORK
#endif

#ifdef HAVE_FORK
	pid=fork();
#endif

	if(pid<=0)
	{
#ifdef HAVE_SETPRIORITY
		// make us low priority so that we don't
		// cause the machine to slow down too much
		setpriority(PRIO_PROCESS,0,15);
#endif

		try
		{
			std::list< etl::handle<Instance> >::iterator iter;

			std::string filename=App::get_config_file("autorecovery");
			std::ofstream file(filename.c_str());

			//int savedcount = 0;

			for(iter=App::instance_list.begin();iter!=App::instance_list.end();++iter)
			{
				// If this file hasn't even been changed
				// since it was last saved, then don't bother
				// backing it up.
				if((*iter)->get_action_count()==0)
					continue;

				Canvas::Handle canvas((*iter)->get_canvas());

				// todo: literal "container:project.sifz"
				FileSystem::Handle file_system = canvas->get_identifier().file_system;
				if (file_system && (*iter)->get_container())
				{
					if (save_canvas(file_system->get_identifier("#project.sifz"), canvas, false))
					{
						if ((*iter)->get_container()->save_temporary())
						{
							file << (*iter)->get_container()->get_temporary_filename_base().c_str() << endl;
							file << canvas->get_file_name().c_str() << endl;
							//savedcount++;
						}
					}
				}
			}

			//if(savecount)
			//	synfig::info("AutoRecover::auto_backup(): %d Files backed up.",savecount);
		}
		catch(...)
		{
			synfig::error("AutoRecover::auto_backup(): UNKNOWN EXCEPTION THROWN.");
			synfig::error("AutoRecover::auto_backup(): FILES NOT BACKED UP.");
		}

#ifdef HAVE_FORK
		if(pid==0)
		{
			_exit(0);
		}
#endif
	}

#ifdef HAVE_FORK
	Glib::signal_timeout().connect(
		sigc::bind(
			sigc::ptr_fun(&AutoRecover::cleanup_pid),
			pid
		),
		60*1000
	);
#endif

	// Also go ahead and save the settings
	App::save_settings();

	return true;
}

bool
AutoRecover::recovery_needed()const
{
	std::string filename=App::get_config_file("autorecovery");
	std::ifstream file(filename.c_str());
	if(!file)
		return false;

	while(file)
	{
		std::string filename;
		getline(file,filename);
		if(!filename.empty())
			return true;
	}

	return false;
}

bool
AutoRecover::recover(int& number_recovered)
{
	std::string filename=App::get_config_file("autorecovery");
	std::ifstream file(filename.c_str());
	number_recovered = 0;
	if(!file)
		return false;
	bool success=true;

	while(file)
	{
		std::string container_filename_base;
		std::string canvas_filename;

		getline(file,container_filename_base);
		if (!file || container_filename_base.empty())
			continue;

		getline(file,canvas_filename);
		if(canvas_filename.empty())
			continue;

		// Open the file
		if(App::open_from_temporary_container_as(container_filename_base,canvas_filename))
		{
			// Correct the file name
			App::instance_list.back()->set_file_name(canvas_filename);

			// This file isn't saved! mark it as such
			App::instance_list.back()->inc_action_count();

			number_recovered++;
		}
		else
			success=false;
	}

	return success;
}

void
AutoRecover::normal_shutdown()
{
	// Turn off the timer
	auto_backup_connect.disconnect();

	std::string filename=App::get_config_file("autorecovery");
	remove(filename.c_str());
}

void
AutoRecover::clear_backup(synfig::Canvas::Handle canvas)
{
	if(canvas)
		remove(get_shadow_file_name(canvas->get_file_name()).c_str());
}
