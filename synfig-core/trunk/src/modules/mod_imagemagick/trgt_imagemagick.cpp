/* === S Y N F I G ========================================================= */
/*!	\file trgt_imagemagick.cpp
**	\brief ppm Target Module
**
**	\legal
** $Id$
**
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#define SYNFIG_TARGET

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <ETL/stringf>
#include "trgt_imagemagick.h"
#include <stdio.h>
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
#include <unistd.h>
#include <algorithm>
#include <functional>
#include <ETL/clock>
#include <ETL/misc>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

#if defined(HAVE_FORK) && defined(HAVE_PIPE) && defined(HAVE_WAITPID)
 #define UNIX_PIPE_TO_PROCESSES
#elif defined(HAVE__SPAWNLP) && defined(HAVE__PIPE) && defined(HAVE_CWAIT)
 #define WIN32_PIPE_TO_PROCESSES
#endif

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(imagemagick_trgt);
SYNFIG_TARGET_SET_NAME(imagemagick_trgt,"imagemagick");
SYNFIG_TARGET_SET_EXT(imagemagick_trgt,"miff");
SYNFIG_TARGET_SET_VERSION(imagemagick_trgt,"0.1");
SYNFIG_TARGET_SET_CVS_ID(imagemagick_trgt,"$Id$");

/* === M E T H O D S ======================================================= */

imagemagick_trgt::imagemagick_trgt(const char *Filename)
{
	pid=-1;
	file=NULL;
	filename=Filename;
	multi_image=false;
	buffer=NULL;
	color_buffer=0;
}

imagemagick_trgt::~imagemagick_trgt()
{
	if(file){
		fclose(file);
		int status;
#if defined(WIN32_PIPE_TO_PROCESSES)
		cwait(&status,pid,0);
#elif defined(UNIX_PIPE_TO_PROCESSES)
		waitpid(pid,&status,0);
#endif
	}
	file=NULL;
	delete [] buffer;
	delete [] color_buffer;
}

bool
imagemagick_trgt::set_rend_desc(RendDesc *given_desc)
{
	if(filename_extension(filename) == ".xpm")
		pf=PF_RGB;
	else
		pf=PF_RGB|PF_A;

	desc=*given_desc;
	return true;
}

bool
imagemagick_trgt::init()
{
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;

	delete [] buffer;
	buffer=new unsigned char[channels(pf)*desc.get_w()];
	delete [] color_buffer;
	color_buffer=new Color[desc.get_w()];
	return true;
}

void
imagemagick_trgt::end_frame()
{
	if(file)
	{
		fputc(0,file);
		fflush(file);
		fclose(file);
		int status;
#if defined(WIN32_PIPE_TO_PROCESSES)
		cwait(&status,pid,0);
#elif defined(UNIX_PIPE_TO_PROCESSES)
		waitpid(pid,&status,0);
#endif
	}
	file=NULL;
	imagecount++;
}

bool
imagemagick_trgt::start_frame(synfig::ProgressCallback *cb)
{
	const char *msg=_("Unable to open pipe to imagemagick's convert utility");

	string newfilename;

	if (multi_image)
		newfilename = (filename_sans_extension(filename) +
					   etl::strprintf(".%04d",imagecount) +
					   filename_extension(filename));
	else
		newfilename = filename;

#if defined(WIN32_PIPE_TO_PROCESSES)

	int p[2];
	int stdin_fileno, stdout_fileno;

	if(_pipe(p, 512, O_BINARY | O_NOINHERIT) < 0) {
		if(cb) cb->error(N_(msg));
		else synfig::error(N_(msg));
		return false;
	}

	// Save stdin/stdout so we can restore them later
	stdin_fileno  = _dup(_fileno(stdin));
	stdout_fileno = _dup(_fileno(stdout));

	// convert should read from the pipe
	if(_dup2(p[0], _fileno(stdin)) != 0) {
		if(cb) cb->error(N_(msg));
		else synfig::error(N_(msg));
		return false;
	}

	/*
	convert accepts the output filename on the command-line
	if(_dup2(_fileno(output), _fileno(stdout)) != 0) {
		if(cb) cb->error(N_(msg));
		else synfig::error(N_(msg));
		return false;
	}
	*/

	pid = _spawnlp(_P_NOWAIT, "convert", "convert",
		"-depth", "8",
		"-size", strprintf("%dx%d", desc.get_w(), desc.get_h()).c_str(),
		((channels(pf) == 4) ? "rgba:-[0]" : "rgb:-[0]"),
		"-density", strprintf("%dx%d", round_to_int(desc.get_x_res()/39.3700787402), round_to_int(desc.get_y_res()/39.3700787402)).c_str(),
		newfilename.c_str(),
		(const char *)NULL);

	if( pid < 0) {
		if(cb) cb->error(N_(msg));
		else synfig::error(N_(msg));
		return false;
	}

	// Restore stdin/stdout
	if(_dup2(stdin_fileno, _fileno(stdin)) != 0) {
		if(cb) cb->error(N_(msg));
		else synfig::error(N_(msg));
		return false;
	}
	if(_dup2(stdout_fileno, _fileno(stdout)) != 0) {
		if(cb) cb->error(N_(msg));
		else synfig::error(N_(msg));
		return false;
	}
	close(stdin_fileno);
	close(stdout_fileno);

	// Close the pipe read end - convert uses it
	close(p[0]);
	
	// We write data to the write end of the pipe
	file = fdopen(p[1], "wb");

#elif defined(UNIX_PIPE_TO_PROCESSES)

	int p[2];
  
	if (pipe(p)) {
		if(cb) cb->error(N_(msg));
		else synfig::error(N_(msg));
		return false;
	};
  
	pid = fork();
  
	if (pid == -1) {
		if(cb) cb->error(N_(msg));
		else synfig::error(N_(msg));
		return false;
	}
	
	if (pid == 0){
		// Child process
		// Close pipeout, not needed
		close(p[1]);
		// Dup pipeout to stdin
		if( dup2( p[0], STDIN_FILENO ) == -1 ){
			if(cb) cb->error(N_(msg));
			else synfig::error(N_(msg));
			return false;
		}
		// Close the unneeded pipeout
		close(p[0]);
		execlp("convert", "convert",
			"-depth", "8",
			"-size", strprintf("%dx%d", desc.get_w(), desc.get_h()).c_str(),
			((channels(pf) == 4) ? "rgba:-[0]" : "rgb:-[0]"),
			"-density", strprintf("%dx%d", round_to_int(desc.get_x_res()/39.3700787402), round_to_int(desc.get_y_res()/39.3700787402)).c_str(),
			newfilename.c_str(),
			(const char *)NULL);
		// We should never reach here unless the exec failed
		if(cb) cb->error(N_(msg));
		else synfig::error(N_(msg));
		return false;
	} else {
		// Parent process
		// Close pipein, not needed
		close(p[0]);
		// Save pipeout to file handle, will write to it later
		file = fdopen(p[1], "wb");
	}

#else
	#error There are no known APIs for creating child processes
#endif

	if(!file)
	{
		if(cb)cb->error(N_(msg));
		else synfig::error(N_(msg));
		return false;
	}

	//etl::yield();

	return true;
}

Color *
imagemagick_trgt::start_scanline(int /*scanline*/)
{
	return color_buffer;
}

bool
imagemagick_trgt::end_scanline(void)
{
	if(!file)
		return false;

	convert_color_format(buffer, color_buffer, desc.get_w(), pf, gamma());

	if(!fwrite(buffer,channels(pf),desc.get_w(),file))
		return false;

	return true;
}
