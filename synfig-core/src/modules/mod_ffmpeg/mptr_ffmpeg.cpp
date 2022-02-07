/* === S Y N F I G ========================================================= */
/*!	\file mptr_ffmpeg.cpp
**	\brief FFMPEG Importer Module
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "mptr_ffmpeg.h"
#include <cstdio>
#include <synfig/general.h>
#include <synfig/localization.h>
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
#include <iostream>
#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace etl;

#if defined(HAVE_FORK) && defined(HAVE_PIPE) && defined(HAVE_WAITPID)
 #include <unistd.h> // for popen
 #define UNIX_PIPE_TO_PROCESSES
#else
 #define WIN32_PIPE_TO_PROCESSES
#endif

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(ffmpeg_mptr);
SYNFIG_IMPORTER_SET_NAME(ffmpeg_mptr,"ffmpeg");
SYNFIG_IMPORTER_SET_EXT(ffmpeg_mptr,"avi"); // not working look at the main.cpp for list opf available extensions
SYNFIG_IMPORTER_SET_VERSION(ffmpeg_mptr,"0.1");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(ffmpeg_mptr, false);

/* === M E T H O D S ======================================================= */

bool ffmpeg_mptr::is_animated()
{
	return true;
}

bool
ffmpeg_mptr::seek_to(const Time& time)
{
	//if(frame<cur_frame || !file)
	//{
		if(file)
		{
#if defined(WIN32_PIPE_TO_PROCESSES)
			_pclose(file);
#elif defined(UNIX_PIPE_TO_PROCESSES)
			fclose(file);
			int status;
			waitpid(pid,&status,0);
#endif
		}
		
		// FIXME: 24 fps is hardcoded now, but in fact we have to get it from canvas
		//float position = (frame+1)/24; // ffmpeg didn't work with 0 frame
		//float position = 1000/24; // ffmpeg didn't work with 0 frame
		const std::string position = time.get_string(Time::FORMAT_NORMAL);

#if defined(WIN32_PIPE_TO_PROCESSES)

		std::string command;
		
		String binary_path = synfig::get_binary_path("");
		if (binary_path != "")
			binary_path = etl::dirname(binary_path)+ETL_DIRECTORY_SEPARATOR;
		binary_path += "ffmpeg.exe";

		command=strprintf("\"%s\" -ss %s -i \"%s\" -vframes 1 -an -f image2pipe -vcodec ppm -\n", binary_path.c_str(), position.c_str(), identifier.filename.c_str());
		
		// This covers the dumb cmd.exe behavior.
		// See: http://eli.thegreenplace.net/2011/01/28/on-spaces-in-the-paths-of-programs-and-files-on-windows/
		command = "\"" + command + "\"";

		file=_popen(command.c_str(),POPEN_BINARY_READ_TYPE);

#elif defined(UNIX_PIPE_TO_PROCESSES)

		int p[2];

		if (pipe(p)) {
			std::cerr<<"Unable to open pipe to ffmpeg (no pipe)"<<std::endl;
			return false;
		};

		pid = fork();

		if (pid == -1) {
			std::cerr<<"Unable to open pipe to ffmpeg (pid == -1)"<<std::endl;
			return false;
		}

		if (pid == 0){
			// Child process
			// Close pipein, not needed
			close(p[0]);
			// Dup pipein to stdout
			if( dup2( p[1], STDOUT_FILENO ) == -1 ){
				std::cerr<<"Unable to open pipe to ffmpeg (dup2( p[1], STDOUT_FILENO ) == -1)"<<std::endl;
				return false;
			}
			// Close the unneeded pipein
			close(p[1]);
			/*std::string command = strprintf("\"%s\" -ss '%s' -i \"%s\" -vframes 1 -an -f image2pipe -vcodec ppm -\n", "ffmpeg", position2, identifier.filename.c_str());
			synfig::warning("ffmpeg command: '%s'", command.c_str());*/
			execlp("ffmpeg", "ffmpeg", "-ss", position.c_str(), "-i", identifier.filename.c_str(), "-vframes", "1","-an", "-f", "image2pipe", "-vcodec", "ppm", "-", (const char *)NULL);
			// We should never reach here unless the exec failed
			std::cerr<<"Unable to open pipe to ffmpeg (exec failed)"<<std::endl;
			_exit(1);
		} else {
			// Parent process
			// Close pipeout, not needed
			close(p[1]);
			// Save pipein to file handle, will read from it later
			file = fdopen(p[0], "rb");
		}

#else
	#error There are no known APIs for creating child processes
#endif

		if(!file)
		{
			std::cerr<<"Unable to open pipe to ffmpeg"<<std::endl;
			return false;
		}
		cur_frame=-1;
	//}

	//while(cur_frame<frame-1)
	//{
	//	cerr<<"Seeking to..."<<frame<<'('<<cur_frame<<')'<<endl;
	//	if(!grab_frame())
	//		return false;
	//}
	return true;
}

bool
ffmpeg_mptr::grab_frame(void)
{
	if(!file)
	{
		std::cerr<<"unable to open "<<identifier.filename.c_str()<<std::endl;
		return false;
	}
	int w,h;
	float divisor;
	char cookie[2];
	cookie[0]=fgetc(file);

	if(feof(file))
		return false;

	cookie[1]=fgetc(file);

	if(cookie[0]!='P' || cookie[1]!='6')
	{
		std::cerr<<"stream not in PPM format \""<<cookie[0]<<cookie[1]<<'"'<<std::endl;
		return false;
	}

	fgetc(file);
	fscanf(file,"%d %d\n",&w,&h);
	fscanf(file,"%f",&divisor);
	fgetc(file);

	if(feof(file))
		return false;

	frame.set_wh(w, h);
	const ColorReal k = 1/255.0;
	for(int y = 0; y < frame.get_h(); ++y)
		for(int x = 0; x < frame.get_w(); ++x)
		{
			if(feof(file))
				return false;
			ColorReal r = k*(unsigned char)fgetc(file);
			ColorReal g = k*(unsigned char)fgetc(file);
			ColorReal b = k*(unsigned char)fgetc(file);
			frame[y][x] = Color(r, g, b);
		}
	cur_frame++;
	return true;
}

ffmpeg_mptr::ffmpeg_mptr(const synfig::FileSystem::Identifier &identifier):
	synfig::Importer(identifier)
{
#ifdef HAVE_TERMIOS_H
	tcgetattr (0, &oldtty);
#endif
	file=NULL;
	fps=23.98;
	cur_frame=-1;
}

ffmpeg_mptr::~ffmpeg_mptr()
{
	if(file)
	{
#if defined(WIN32_PIPE_TO_PROCESSES)
		_pclose(file);
#elif defined(UNIX_PIPE_TO_PROCESSES)
		fclose(file);
		int status;
		waitpid(pid,&status,0);
#endif
	}
#ifdef HAVE_TERMIOS_H
	tcsetattr(0,TCSANOW,&oldtty);
#endif
}

bool
ffmpeg_mptr::get_frame(synfig::Surface &surface, const synfig::RendDesc &/*renddesc*/, Time time, synfig::ProgressCallback *)
{
	synfig::warning("time: %f", (float)time);
	if(!seek_to(time))
		return false;
	if(!grab_frame())
		return false;

	surface=frame;
	return true;
}
