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

#include <gui/autorecover.h>

#include <glibmm/main.h>

#include <gui/app.h>
#include <gui/instance.h>

#include <synfig/filesystemtemporary.h>
#include <synfig/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

AutoRecover::AutoRecover():
	enabled(1),
	timeout_ms(15000)
{ }

AutoRecover::~AutoRecover()
{
	set_timer(false, 0);
}

void
AutoRecover::set_timer(bool enabled, int timeout_ms)
{
	if (timeout_ms < 0) timeout_ms = 0;
	if (this->enabled != enabled || this->timeout_ms != timeout_ms)
	{
		bool env_enabled = !getenv("SYNFIG_DISABLE_AUTO_SAVE");
		if (this->enabled && this->timeout_ms > 0 && env_enabled)
			connection.disconnect();

		this->enabled = enabled;
		this->timeout_ms = timeout_ms;

		if (this->enabled && this->timeout_ms > 0 && env_enabled)
			connection = Glib::signal_timeout().connect(
				sigc::bind_return(
					sigc::mem_fun(*this, &AutoRecover::auto_backup),
					true ),
				this->timeout_ms );
	}
}

void
AutoRecover::auto_backup()
{
	int total = (int)App::instance_list.size();
	int count = 0;
	try
	{
		for(std::list< etl::handle<Instance> >::iterator i = App::instance_list.begin(); i != App::instance_list.end(); ++i)
			try
			{
				if ((*i)->backup())
					++count;
			}
			catch(...)
			{
				synfig::error("AutoRecover::auto_backup(): UNKNOWN EXCEPTION THROWN.");
			}
	}
	catch(...)
	{
		synfig::error("AutoRecover::auto_backup(): UNKNOWN EXCEPTION THROWN.");
	}

	// Also go ahead and save the settings
	App::save_settings();

	//if (count)
	//	synfig::info("AutoRecover::auto_backup(): %d Files backed up.", count);
	if (count != total)
		synfig::error("AutoRecover::auto_backup(): %d FILES NOT BACKED UP.", total - count);
}

bool
AutoRecover::recovery_needed()const
{
	FileSystem::FileList files;
	FileSystemTemporary::scan_temporary_directory("instance", files, App::get_temporary_directory());
	return !files.empty();
}

bool
AutoRecover::recover(int &number_recovered)
{
	bool success = false;
	number_recovered = 0;

	FileSystem::FileList files;
	if (FileSystemTemporary::scan_temporary_directory("instance", files, App::get_temporary_directory()))
	{
		success = true;
		for(FileSystem::FileList::const_iterator i = files.begin(); i != files.end(); ++i)
			if (App::open_from_temporary_filesystem(App::get_temporary_directory() + ETL_DIRECTORY_SEPARATOR + *i))
				++number_recovered;
			else
				success=false;
	}
	return success;
}

bool
AutoRecover::clear_backups()
{
	bool success = false;
	FileSystem::FileList files;
	if (FileSystemTemporary::scan_temporary_directory("instance", files, App::get_temporary_directory()))
	{
		success = true;
		for(FileSystem::FileList::const_iterator i = files.begin(); i != files.end(); ++i)
		{
			// FileSystemTemporary will clear opened temporary files in destructor
			String filename = App::get_temporary_directory() + ETL_DIRECTORY_SEPARATOR + *i;
			bool s = false;
			try { s = FileSystemTemporary("").open_temporary(filename); }
			catch (...)
			{
				synfig::warning("Autobackup file is not recoverable. Forcing to remove.");
			}
			if (!s)
			{
				FileSystemNative::instance()->file_remove(filename);
				success = false;
			}
		}
	}
	return success;
}
