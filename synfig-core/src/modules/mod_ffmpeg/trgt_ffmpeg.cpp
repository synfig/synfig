/* === S Y N F I G ========================================================= */
/*!	\file trgt_ffmpeg.cpp
**	\brief ppm Target Module
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2010 Diego Barrios Romero
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

#include <ETL/stringf>
#include "trgt_ffmpeg.h"
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

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

#if defined(HAVE_FORK) && defined(HAVE_PIPE) && defined(HAVE_WAITPID)
 #define UNIX_PIPE_TO_PROCESSES
#else
 #define WIN32_PIPE_TO_PROCESSES
#endif

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(ffmpeg_trgt);
SYNFIG_TARGET_SET_NAME(ffmpeg_trgt,"ffmpeg");
SYNFIG_TARGET_SET_EXT(ffmpeg_trgt,"mpg");
SYNFIG_TARGET_SET_VERSION(ffmpeg_trgt,"0.1");
SYNFIG_TARGET_SET_CVS_ID(ffmpeg_trgt,"$Id$");

/* === M E T H O D S ======================================================= */

ffmpeg_trgt::ffmpeg_trgt(const char *Filename,
						 const synfig::TargetParam& params)
{
	pid=-1;
	file=NULL;
	filename=Filename;
	multi_image=false;
	buffer=NULL;
	color_buffer=0;
	set_remove_alpha();

	// Set default video codec and bitrate if they weren't given.
	if (params.video_codec == "none")
		video_codec = "mpeg1video";
	else
		video_codec = params.video_codec;

	if (params.bitrate == -1)
		bitrate = 200;
	else
		bitrate = params.bitrate;
}

ffmpeg_trgt::~ffmpeg_trgt()
{
	if(file)
	{
		etl::yield();
		sleep(1);
#if defined(WIN32_PIPE_TO_PROCESSES)
		pclose(file);
#elif defined(UNIX_PIPE_TO_PROCESSES)
		fclose(file);
		int status;
		waitpid(pid,&status,0);
#endif
	}
	file=NULL;
	delete [] buffer;
	delete [] color_buffer;
}

bool
ffmpeg_trgt::set_rend_desc(RendDesc *given_desc)
{
	//given_desc->set_pixel_format(PF_RGB);

	// Make sure that the width and height
	// are multiples of 8
	given_desc->set_w((given_desc->get_w()+4)/8*8);
	given_desc->set_h((given_desc->get_h()+4)/8*8);

	/*
	// Valid framerates:
	// 23.976, 24, 25, 29.97, 30, 50 ,59.94, 60
	float fps=given_desc->get_frame_rate();
	if(fps <24.0)
		given_desc->set_frame_rate(23.976);
	if(fps>=24.0 && fps <25.0)
		given_desc->set_frame_rate(24);
	if(fps>=25.0 && fps <29.97)
		given_desc->set_frame_rate(25);
	if(fps>=29.97 && fps <30.0)
		given_desc->set_frame_rate(29.97);
	if(fps>=29.97 && fps <30.0)
		given_desc->set_frame_rate(29.97);
	if(fps>=30.0 && fps <50.0)
		given_desc->set_frame_rate(30.0);
	if(fps>=50.0 && fps <59.94)
		given_desc->set_frame_rate(50);
	if(fps>=59.94)
		given_desc->set_frame_rate(59.94);
    */

	desc=*given_desc;

	return true;
}

bool
ffmpeg_trgt::init()
{
	synfig::info("ffmpeg_trgt::init called...");
	
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;
	// this should avoid conflicts with locale settings
	synfig::ChangeLocale change_locale(LC_NUMERIC, "C");

#if defined(WIN32_PIPE_TO_PROCESSES)

	string command;

	String binary_path = synfig::get_binary_path("");
	if (binary_path != "")
		binary_path = etl::dirname(binary_path)+ETL_DIRECTORY_SEPARATOR;
	binary_path += "ffmpeg.exe";

	if( filename.c_str()[0] == '-' )
		command = strprintf("\"%s\" -f image2pipe -vcodec ppm -an"
							" -r %f -i pipe: -loop 1"
							" -metadata title=\"%s\" "
							" -vcodec %s -b %ik"
							" -y -- \"%s\"\n",
							binary_path.c_str(),
							desc.get_frame_rate(),
							get_canvas()->get_name().c_str(),
							video_codec.c_str(), bitrate,
							filename.c_str());
	else
		command = strprintf("\"%s\" -f image2pipe -vcodec ppm -an"
							" -r %f -i pipe: -loop 1"
							" -metadata title=\"%s\" "
							"-vcodec %s -b %ik"
							" -y -- \"%s\"\n",
							binary_path.c_str(),
							desc.get_frame_rate(),
							get_canvas()->get_name().c_str(),
							video_codec.c_str(), bitrate,
							filename.c_str());

	// This covers the dumb cmd.exe behavior.
	// See: http://eli.thegreenplace.net/2011/01/28/on-spaces-in-the-paths-of-programs-and-files-on-windows/
	command = "\"" + command + "\"";

	file=popen(command.c_str(),POPEN_BINARY_WRITE_TYPE);

#elif defined(UNIX_PIPE_TO_PROCESSES)

	int p[2];

	if (pipe(p)) {
		synfig::error(_("Unable to open pipe to ffmpeg (no pipe)"));
		return false;
	};

	pid = fork();

	if (pid == -1) {
		synfig::error(_("Unable to open pipe to ffmpeg (pid == -1)"));
		return false;
	}

	if (pid == 0){
		// Child process
		// Close pipeout, not needed
		close(p[1]);
		// Dup pipeout to stdin
		if( dup2( p[0], STDIN_FILENO ) == -1 ){
			synfig::error(_("Unable to open pipe to ffmpeg (dup2( p[0], STDIN_FILENO ) == -1)"));
			return false;
		}
		// Close the unneeded pipeout
		close(p[0]);
		if( filename.c_str()[0] == '-' )
		{
			// x264 codec needs -vpre hq parameters
			if (video_codec == "libx264")
				execlp("ffmpeg", "ffmpeg", "-f", "image2pipe", "-vcodec",
					   "ppm", "-an", "-r",
					   strprintf("%f", desc.get_frame_rate()).c_str(),
					   "-i", "pipe:", "-loop", "1", "-metadata",
						strprintf("title=\"%s\"", get_canvas()->get_name().c_str()).c_str(),
						"-vcodec", video_codec.c_str(),
						"-b", strprintf("%ik", bitrate).c_str(),
						"-vpre", "hq",
						"-y", "--", filename.c_str(), (const char *)NULL);
			else
				execlp("ffmpeg", "ffmpeg", "-f", "image2pipe", "-vcodec",
					   "ppm", "-an", "-r",
					   strprintf("%f", desc.get_frame_rate()).c_str(),
					   "-i", "pipe:", "-loop", "1", "-metadata",
						strprintf("title=\"%s\"", get_canvas()->get_name().c_str()).c_str(),
						"-vcodec", video_codec.c_str(),
						"-b", strprintf("%ik", bitrate).c_str(),
						"-y", "--", filename.c_str(), (const char *)NULL);
		}
		else
		{
			if (video_codec == "libx264")
				execlp("ffmpeg", "ffmpeg", "-f", "image2pipe", "-vcodec",
					   "ppm", "-an", "-r",
					   strprintf("%f", desc.get_frame_rate()).c_str(),
					   "-i", "pipe:", "-loop", "1",
					   "-metadata",
					   strprintf("title=\"%s\"", get_canvas()->get_name().c_str()).c_str(),
					   "-vcodec", video_codec.c_str(),
					   "-b", strprintf("%ik", bitrate).c_str(),
					   "-vpre", "hq",
					   "-y", filename.c_str(), (const char *)NULL);
			else
				execlp("ffmpeg", "ffmpeg", "-f", "image2pipe", "-vcodec",
					   "ppm", "-an", "-r",
					   strprintf("%f", desc.get_frame_rate()).c_str(),
					   "-i", "pipe:", "-loop", "1",
					   "-metadata",
					   strprintf("title=\"%s\"", get_canvas()->get_name().c_str()).c_str(),
					   "-vcodec", video_codec.c_str(),
					   "-b", strprintf("%ik", bitrate).c_str(),
					   "-y", filename.c_str(), (const char *)NULL);
		}

		// We should never reach here unless the exec failed
		synfig::error(_("Unable to open pipe to ffmpeg (exec failed)"));
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

	// etl::yield();

	if(!file)
	{
		synfig::error(_("Unable to open pipe to ffmpeg (no file)"));
		return false;
	}

	return true;
}

void
ffmpeg_trgt::end_frame()
{
	//fprintf(file, " ");
	fflush(file);
	imagecount++;
}

bool
ffmpeg_trgt::start_frame(synfig::ProgressCallback */*callback*/)
{
	int w=desc.get_w(),h=desc.get_h();

	if(!file)
		return false;

	fprintf(file, "P6\n");
	fprintf(file, "%d %d\n", w, h);
	fprintf(file, "%d\n", 255);

	delete [] buffer;
	buffer=new unsigned char[3*w];
	delete [] color_buffer;
	color_buffer=new Color[w];

	return true;
}

Color *
ffmpeg_trgt::start_scanline(int /*scanline*/)
{
	return color_buffer;
}

bool
ffmpeg_trgt::end_scanline()
{
	if(!file)
		return false;

	convert_color_format(buffer, color_buffer, desc.get_w(), PF_RGB, gamma());

	if(!fwrite(buffer,1,desc.get_w()*3,file))
		return false;

	return true;
}
