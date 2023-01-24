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

#include <ETL/stringf>

#include <synfig/general.h>
#include <synfig/localization.h>

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
		pipe = nullptr;
		
		// FIXME: 24 fps is hardcoded now, but in fact we have to get it from canvas
		//float position = (frame+1)/24; // ffmpeg didn't work with 0 frame
		//float position = 1000/24; // ffmpeg didn't work with 0 frame
		const std::string position = time.get_string(Time::FORMAT_NORMAL);

		OS::RunArgs args;
		args.push_back({"-ss", position});
		args.push_back("-i");
		args.push_back(filesystem::Path(identifier.filename));
		args.push_back({"-vframes", "1"});
		args.push_back("-an");
		args.push_back({"-f", "image2pipe"});
		args.push_back({"-vcodec", "ppm"});
		args.push_back("-");

#ifdef _WIN32
		String binary_path = synfig::get_binary_path("");
		if (binary_path != "")
			binary_path = etl::dirname(binary_path)+ETL_DIRECTORY_SEPARATOR;
		binary_path += "ffmpeg.exe";
#else
		String binary_path = "ffmpeg";
#endif
		pipe = OS::run_async(binary_path, args, OS::RUN_MODE_READ);

		if(!pipe)
		{
			synfig::error(_("Unable to open pipe to ffmpeg"));
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
	if(!pipe)
	{
		synfig::error(_("unable to open %s"), identifier.filename.c_str());
		return false;
	}
	int w,h;
	float divisor;
	char cookie[2];
	cookie[0]=pipe->getc();

	if(pipe->eof())
		return false;

	cookie[1]=pipe->getc();

	if(cookie[0]!='P' || cookie[1]!='6')
	{
		synfig::error(_("stream not in PPM format \"%c%c\""), cookie[0], cookie[1]);
		return false;
	}

	pipe->getc();
	pipe->scanf("%d %d\n",&w,&h);
	pipe->scanf("%f",&divisor);
	pipe->getc();

	if(pipe->eof())
		return false;

	frame.set_wh(w, h);
	const ColorReal k = 1/255.0;
	for(int y = 0; y < frame.get_h(); ++y)
		for(int x = 0; x < frame.get_w(); ++x)
		{
			if(pipe->eof())
				return false;
			ColorReal r = k*(unsigned char)pipe->getc();
			ColorReal g = k*(unsigned char)pipe->getc();
			ColorReal b = k*(unsigned char)pipe->getc();
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
	pipe=nullptr;
	fps=23.98;
	cur_frame=-1;
}

ffmpeg_mptr::~ffmpeg_mptr()
{
	pipe = nullptr;
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
