/* === S Y N F I G ========================================================= */
/*!	\file mptr_imagemagick.cpp
**	\brief ImageMagick Importer Module
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2015 Jérôme Blanchi
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mptr_imagemagick.h"
#include <cstdio>
#include <sys/types.h>
#if HAVE_SYS_WAIT_H
 #include <sys/wait.h>
#endif
#if HAVE_IO_H
 #include <io.h>
#endif
#if HAVE_PROCESS_H
 #include <process.h>
#endif
#if HAVE_FCNTL_H
 #include <fcntl.h>
#endif

#include <ETL/stringf>

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/filesystemnative.h>
#include <synfig/filesystemtemporary.h>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace etl;

#if defined(HAVE_FORK) && defined(HAVE_PIPE) && defined(HAVE_WAITPID)
 #define UNIX_PIPE_TO_PROCESSES
 #include <unistd.h>
#else
 #define WIN32_PIPE_TO_PROCESSES
#endif

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(imagemagick_mptr);
SYNFIG_IMPORTER_SET_NAME(imagemagick_mptr,"imagemagick");
SYNFIG_IMPORTER_SET_EXT(imagemagick_mptr,"miff");
SYNFIG_IMPORTER_SET_VERSION(imagemagick_mptr,"0.1");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(imagemagick_mptr, false);

/* === M E T H O D S ======================================================= */


imagemagick_mptr::imagemagick_mptr(const synfig::FileSystem::Identifier &identifier):
synfig::Importer(identifier),
file(nullptr)
//cur_frame(0)
{ }

imagemagick_mptr::~imagemagick_mptr()
{
	if (file) {
#if defined(WIN32_PIPE_TO_PROCESSES)
		_pclose(file);
#elif defined(UNIX_PIPE_TO_PROCESSES)
		fclose(file);
#endif
	}
}

bool
imagemagick_mptr::get_frame(synfig::Surface &surface, const synfig::RendDesc &renddesc, Time /*time*/, synfig::ProgressCallback *cb)
{
	if(identifier.filename.empty() || !identifier.file_system)
	{
		if(cb)cb->error(_("No file to load"));
		else synfig::error(_("No file to load"));
		return false;
	}

	bool is_temporary_file = false;
	std::string filename=identifier.file_system->get_real_filename(identifier.filename);
	std::string target_filename=FileSystemTemporary::generate_system_temporary_filename("imagemagick", ".png");

	std::string filename_extension = etl::filename_extension(identifier.filename);

	if (filename.empty()) {
		is_temporary_file = true;
		filename = FileSystemTemporary::generate_system_temporary_filename("imagemagick", filename_extension);

		// try to copy file to a temp file
		if (!FileSystem::copy(identifier.file_system, identifier.filename, identifier.file_system, filename))
		{
			if(cb)cb->error(_("Cannot create temporary file of ")+ identifier.filename);
			else synfig::error(_("Cannot create temporary file of ")+ identifier.filename);
			return false;
		}
	}

#if defined(WIN32_PIPE_TO_PROCESSES)

	if(file)
		_pclose(file);

	std::string command;

	if(identifier.filename.find("psd")!=String::npos)
		command=strprintf("convert \"%s\" -flatten \"png32:%s\"\n",filename.c_str(),target_filename.c_str());
	else
		command=strprintf("convert \"%s\" \"png32:%s\"\n",filename.c_str(),target_filename.c_str());

	if(system(command.c_str())!=0)
		return false;

#elif defined(UNIX_PIPE_TO_PROCESSES)

	std::string output="png32:"+target_filename;

	pid_t pid = fork();

	if (pid == -1) {
		return false;
	}

	if (pid == 0){
		// Child process
		if(identifier.filename.find("psd")!=String::npos)
			execlp("convert", "convert", filename.c_str(), "-flatten", output.c_str(), (const char*)nullptr);
		else
			execlp("convert", "convert", filename.c_str(), output.c_str(), (const char*)nullptr);
		// We should never reach here unless the exec failed
		return false;
	}

	int status;
	waitpid(pid, &status, 0);
	if( (WIFEXITED(status) && WEXITSTATUS(status) != 0) || !WIFEXITED(status) )
		return false;

#else
	#error There are no known APIs for creating child processes
#endif

	if(is_temporary_file)
		identifier.file_system->file_remove(filename);

	Importer::Handle importer(Importer::open(synfig::FileSystem::Identifier(synfig::FileSystemNative::instance(), target_filename)));

	if(!importer)
	{
		if(cb)cb->error(_("Unable to open ")+target_filename);
		else synfig::error(_("Unable to open ")+target_filename);
		return false;
	}

	if(!importer->get_frame(surface,renddesc,0,cb))
	{
		if(cb)cb->error(_("Unable to get frame from ")+target_filename);
		else synfig::error(_("Unable to get frame from ")+target_filename);
		return false;
	}

	if(!surface)
	{
		if(cb)cb->error(_("Bad surface from ")+target_filename);
		else synfig::error(_("Bad surface from ")+target_filename);
		return false;
	}

	if(1)
	{
		// remove odd premultiplication
		for(int i=0;i<surface.get_w()*surface.get_h();i++)
		{
			Color c(surface[0][i]);

			if(c.get_a())
			{
				surface[0][i].set_r(c.get_r()/c.get_a()/c.get_a());
				surface[0][i].set_g(c.get_g()/c.get_a()/c.get_a());
				surface[0][i].set_b(c.get_b()/c.get_a()/c.get_a());
			}
			else
			{
				surface[0][i].set_r(0);
				surface[0][i].set_g(0);
				surface[0][i].set_b(0);
			}
			surface[0][i].set_a(c.get_a());
		}
	}

	Surface bleh(surface);
	surface=bleh;

	remove(target_filename.c_str());
	return true;
}
