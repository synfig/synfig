/* === S I N F G =========================================================== */
/*!	\file autorecover.cpp
**	\brief Template File
**
**	$Id: autorecover.cpp,v 1.1.1.1 2005/01/07 03:34:35 darco Exp $
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

#include "autorecover.h"

//#include <unistd.h>
#include "app.h"
#include <sinfg/savecanvas.h>
#include <sinfg/loadcanvas.h>
#include <fstream>
#include <iostream>
#include "instance.h"

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

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
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
			sinfg::error("UNABLE TO CREATE \"%s\"",get_shadow_directory().c_str());
	}
	else
	{
		sinfg::info("Created directory \"%s\"",get_shadow_directory().c_str());
	}
}

AutoRecover::~AutoRecover()
{
}

sinfg::String
AutoRecover::get_shadow_directory()
{
	return Glib::build_filename(App::get_user_app_directory(),"tmp");
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
	if(timeout)
		auto_backup_connect=Glib::signal_timeout().connect(sigc::ptr_fun(&AutoRecover::auto_backup),timeout);
//		auto_backup_connect=App::main.get_context()->signal_timeout().connect(sigc::mem_fun(&AutoRecover::auto_backup),timeout);
}

sinfg::String
AutoRecover::get_shadow_file_name(const sinfg::String& filename)
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
	
//	return dirname(filename) + ETL_DIRECTORY_SEPERATOR + ".shadow_" + basename(filename);
}

bool
AutoRecover::cleanup_pid(int pid)
{
#ifdef HAVE_FORK
	int status=0;
	if(waitpid(pid,&status,WNOHANG)==-1)
	{
		sinfg::info("PID %d isn't a zombie yet",pid);
		return true;
	}
	if(WEXITSTATUS(status)!=0)
	{
		sinfg::error("Autobackup seems to have failed! (PID=%d)",pid);
	}
	else
		sinfg::info("PID=%d has been cleaned up",pid);
#endif
	return false;
}

bool
AutoRecover::auto_backup()
{
	int pid(0);

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
			std::list<etl::handle<Instance> >::iterator iter;
	
			std::string filename=App::get_config_file("autorecovery");
			std::ofstream file(filename.c_str());
			
			int savecount(0);
			
			for(iter=App::instance_list.begin();iter!=App::instance_list.end();++iter)
			{
				// If this file hasn't even been changed
				// since it was last saved, then don't bother
				// backing it up.
				if((*iter)->get_action_count()==0)
					continue;
					
				Canvas::Handle canvas((*iter)->get_canvas());
				file<<canvas->get_file_name()<<endl;
				save_canvas(get_shadow_file_name(canvas->get_file_name()),canvas);
				savecount++;
			}
			
			if(savecount)
				sinfg::info("AutoRecover::auto_backup(): %d Files backed up.",savecount);
		}
		catch(...)
		{
			sinfg::error("AutoRecover::auto_backup(): UNKNOWN EXCEPTION THROWN.");
			sinfg::error("AutoRecover::auto_backup(): FILES NOT BACKED UP.");
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
AutoRecover::recover()
{
	std::string filename=App::get_config_file("autorecovery");
	std::ifstream file(filename.c_str());
	if(!file)
		return false;
	bool success=true;
	
	while(file)
	{
		std::string filename;
		getline(file,filename);
		if(filename.empty())
			continue;

		// Open the file
		if(App::open_as(get_shadow_file_name(filename),filename))
		{
			// Correct the file name
			App::instance_list.back()->set_file_name(filename);
			
			// This file isn't saved! mark it as such
			App::instance_list.back()->inc_action_count();
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
AutoRecover::clear_backup(sinfg::Canvas::Handle canvas)
{
	if(canvas)
		remove(get_shadow_file_name(canvas->get_file_name()).c_str());
}
