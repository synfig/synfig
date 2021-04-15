/* === S Y N F I G ========================================================= */
/*!	\file mptr_imagemagick.cpp
**	\brief ppm Target Module
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2015 Jérôme Blanchi
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
**
** === N O T E S ===========================================================
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
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/filesystemnative.h>
#include <synfig/filesystemtemporary.h>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
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
file(NULL)
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
//#define HAS_LIBPNG 1

#if 1
	if(identifier.filename.empty() || !identifier.file_system)
	{
		if(cb)cb->error(_("No file to load"));
		else synfig::error(_("No file to load"));
		return false;
	}

	bool is_temporary_file = false;
	string filename=identifier.file_system->get_real_filename(identifier.filename);
	string target_filename=FileSystemTemporary::generate_system_temporary_filename("imagemagick");

	if (filename.empty()) {
		is_temporary_file = true;
		filename = FileSystemTemporary::generate_system_temporary_filename("imagemagick");

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

	string command;

	if(identifier.filename.find("psd")!=String::npos)
		command=strprintf("convert \"%s\" -flatten \"png32:%s\"\n",filename.c_str(),target_filename.c_str());
	else
		command=strprintf("convert \"%s\" \"png32:%s\"\n",filename.c_str(),target_filename.c_str());

	if(system(command.c_str())!=0)
		return false;

#elif defined(UNIX_PIPE_TO_PROCESSES)

	string output="png32:"+target_filename;

	pid_t pid = fork();

	if (pid == -1) {
		return false;
	}

	if (pid == 0){
		// Child process
		if(identifier.filename.find("psd")!=String::npos)
			execlp("convert", "convert", filename.c_str(), "-flatten", output.c_str(), (const char *)NULL);
		else
			execlp("convert", "convert", filename.c_str(), output.c_str(), (const char *)NULL);
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

#else

#error This code contains tempfile and arbitrary shell command execution vulnerabilities

	if(file)
		pclose(file);

	string command;

	if(identifier.filename.empty())
	{
		if(cb)cb->error(_("No file to load"));
		else synfig::error(_("No file to load"));
		return false;
	}

	command=strprintf("convert \"%s\" -flatten ppm:-\n",identifier.filename.c_str());

	file=popen(command.c_str(),POPEN_BINARY_READ_TYPE);

	if(!file)
	{
		if(cb)cb->error(_("Unable to open pipe to imagemagick"));
		else synfig::error(_("Unable to open pipe to imagemagick"));
		return false;
	}
	int w,h;
	float divisor;
	char cookie[2];

	while((cookie[0]=fgetc(file))!='P' && !feof(file));

	if(feof(file))
	{
		if(cb)cb->error(_("Reached end of stream without finding PPM header"));
		else synfig::error(_("Reached end of stream without finding PPM header"));
		return false;
	}

	cookie[1]=fgetc(file);

	if(cookie[0]!='P' || cookie[1]!='6')
	{
		if(cb)cb->error(string(_("stream not in PPM format"))+" \""+cookie[0]+cookie[1]+'"');
		else synfig::error(string(_("stream not in PPM format"))+" \""+cookie[0]+cookie[1]+'"');
		return false;
	}

	fgetc(file);
	fscanf(file,"%d %d\n",&w,&h);
	fscanf(file,"%f",&divisor);
	fgetc(file);

	if(feof(file))
	{
		if(cb)cb->error(_("Premature end of file (after header)"));
		else synfig::error(_("Premature end of file (after header)"));
		return false;
	}

	int x;
	int y;
	frame.set_wh(w,h);
	for(y=0;y<frame.get_h();y++)
		for(x=0;x<frame.get_w();x++)
		{
			if(feof(file))
			{
				if(cb)cb->error(_("Premature end of file"));
				else synfig::error(_("Premature end of file"));
				return false;
			}
			float b=gamma().r_U8_to_F32((unsigned char)fgetc(file));
			float g=gamma().g_U8_to_F32((unsigned char)fgetc(file));
			float r=gamma().b_U8_to_F32((unsigned char)fgetc(file));
/*
			float b=(float)(unsigned char)fgetc(file)/divisor;
			float g=(float)(unsigned char)fgetc(file)/divisor;
			float r=(float)(unsigned char)fgetc(file)/divisor;
*/
			frame[y][x]=Color(
				b,
				g,
				r,
				1.0
			);
		}

	surface=frame;

	return true;
#endif


}
