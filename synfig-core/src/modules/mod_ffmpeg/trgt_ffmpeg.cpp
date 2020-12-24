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

#include <synfig/localization.h>
#include <synfig/general.h>
#include <synfig/soundprocessor.h>

#include <cstdio>
#include <glib/gstdio.h>
#include <thread>

#include "trgt_ffmpeg.h"

#if HAVE_SYS_WAIT_H
 #include <sys/wait.h>
#endif

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;

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

/* === M E T H O D S ======================================================= */

ffmpeg_trgt::ffmpeg_trgt(const char *Filename, const synfig::TargetParam &params):
	pid(-1),
	imagecount(0),
	multi_image(false),
	file(NULL),
	filename(Filename),
	sound_filename(""),
	buffer(NULL),
	color_buffer(NULL),
	bitrate()
{
	set_alpha_mode(TARGET_ALPHA_MODE_FILL);

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
		std::this_thread::yield();
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

	// Remove temporary sound file
	if (g_file_test(sound_filename.c_str(), G_FILE_TEST_EXISTS)) {
		if( g_remove(sound_filename.c_str()) != 0 ) {
			synfig::warning("Error deleting temporary sound file (%s).", sound_filename.c_str());
		}
	}
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
ffmpeg_trgt::init(ProgressCallback *cb=NULL)
{

	bool with_sound = true;
	if(!SoundProcessor::subsys_init()) {
		if (cb) cb->error(_("Unable to initialize Sound subsystem"));
		with_sound = false;
	} else {
		synfig::SoundProcessor soundProcessor;
		soundProcessor.set_infinite(false);
		get_canvas()->fill_sound_processor(soundProcessor);
		// Generate random filename here
		do {
			synfig::GUID guid;
			sound_filename = String(filename)+"."+guid.get_string().substr(0,8)+".wav";
		} while (g_file_test(sound_filename.c_str(), G_FILE_TEST_EXISTS));

		soundProcessor.do_export(sound_filename);

		if (!g_file_test(sound_filename.c_str(), G_FILE_TEST_EXISTS)) {
			with_sound = false;
		}
	}

	String ffmpeg_binary_path;
#if defined(WIN32_PIPE_TO_PROCESSES)
	// Windows always have ffmpeg
	std::string binary_path = etl::dirname(synfig::get_binary_path(".")) + "/ffmpeg.exe";
	if (g_access(binary_path.c_str(), F_OK ) != -1 ) { // File found
		ffmpeg_binary_path = "\"" + binary_path + "\"";
	}
#else
	// Some Linux OS may have `avconv` instead of `ffmpeg`, so let's check both
	const std::vector<std::string> binary_choices = {"ffmpeg", "avconv"};
	for (const auto& bin_name : binary_choices) {
		std::string command = "which " + bin_name;
		String result;
		FILE* pipe = popen(command.c_str(), "r");
		if (!pipe) {
			continue;
		}
		char buf[128];
		while(!feof(pipe)) {
			if(fgets(buf, 128, pipe) != NULL)
				result += buf;
		}
		/* `which` exit status
		 * 0      if all specified commands are found and executable
		 * 1      if one or more specified commands is nonexistent or not executable
		 * 2      if an invalid option is specified */
		synfig::info("\"%s\" --> %s", command.c_str(), result.c_str());
		if (pclose(pipe) == 0) { // If we need to check non-zero exit code we should use `WEXITSTATUS` for this
			ffmpeg_binary_path = bin_name; // we can use `result` value here, but in this case we need to remove trailing `\n`
			break;
		}
	}
#endif

	if (ffmpeg_binary_path.empty()) {
		if (cb) cb->error(_("Error: No FFmpeg binary found.\n\nPlease install \"ffmpeg\" or \"avconv\" (libav-tools package)."));
		return false;
	}
	
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;
	// this should avoid conflicts with locale settings
	synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
	
	std::string video_codec_real = (video_codec == "libx264-lossless" ? "libx264" : video_codec);

	std::vector<std::string> vargs;
	vargs.emplace_back(ffmpeg_binary_path);
	if (with_sound) {
		vargs.emplace_back("-i");
#if defined(WIN32_PIPE_TO_PROCESSES)
		vargs.emplace_back("\"" + sound_filename + "\"");
#else
		vargs.emplace_back(sound_filename);
#endif
	}
	vargs.emplace_back("-f");
	vargs.emplace_back("image2pipe");
	vargs.emplace_back("-vcodec");
	vargs.emplace_back("ppm");
	vargs.emplace_back("-r");
	vargs.emplace_back(etl::strprintf("%f", desc.get_frame_rate()));
	vargs.emplace_back("-i");
	vargs.emplace_back("pipe:");
	vargs.emplace_back("-metadata");
	vargs.emplace_back(etl::strprintf("title=\"%s\"", get_canvas()->get_name().c_str()));
	vargs.emplace_back("-vcodec");
	vargs.emplace_back(video_codec_real);
	vargs.emplace_back("-b:v");
	vargs.emplace_back(etl::strprintf("%ik", bitrate));
	if (video_codec == "libx264-lossless") {
		vargs.emplace_back("-tune");
		vargs.emplace_back("fastdecode");
		vargs.emplace_back("-pix_fmt");
		vargs.emplace_back("yuv420p");
		vargs.emplace_back("-qp");
		vargs.emplace_back("0");
	}
	vargs.emplace_back("-acodec");
	// MPEG-1 cannot work with 'le' audio, it requires 'be'
	vargs.emplace_back(video_codec == "mpeg1video" ? "pcm_s16be" : "pcm_s16le");
	vargs.emplace_back("-y");
	vargs.emplace_back("-shortest");
	// We need "--" to separate filename from arguments (for the case when filename starts with "-")
	if ( filename.substr(0,1) == "-" )
		vargs.emplace_back("--");

#if defined(WIN32_PIPE_TO_PROCESSES)
	vargs.emplace_back("\"" + filename + "\"");
#else
	vargs.emplace_back(filename);
#endif

#if defined(WIN32_PIPE_TO_PROCESSES)

	String command;
	for( std::vector<std::string>::size_type i = 0; i != vargs.size(); i++ )
	{
		command+=" "+vargs[i];
	}
	command+="\n";

	// This covers the dumb cmd.exe behavior.
	// See: http://eli.thegreenplace.net/2011/01/28/on-spaces-in-the-paths-of-programs-and-files-on-windows/
	command = "\"" + command + "\"";
	
	const wchar_t* wcommand = reinterpret_cast<const wchar_t*>(g_utf8_to_utf16(command.c_str(), -1, NULL, NULL, NULL));
	const wchar_t* wmode = reinterpret_cast<const wchar_t*>(g_utf8_to_utf16(POPEN_BINARY_WRITE_TYPE, -1, NULL, NULL, NULL));
	
	file=_wpopen(wcommand, wmode);

#elif defined(UNIX_PIPE_TO_PROCESSES)

	int p[2];

	if (pipe(p)) {
		synfig::error(_("Unable to open pipe to ffmpeg (no pipe)"));
		return false;
	}

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
		
		char *args[vargs.size()+1];
		size_t idx = 0;
		for( std::vector<std::string>::size_type i = 0; i != vargs.size(); i++ )
		{
			//std::vector<char> writable(vargs[i].begin(), vargs[i].end());
			//writable.push_back('\0');
			//args[idx++] = &writable[0];
			//args[idx++] = strdup(vargs[i].c_str());
			args[idx++] = &vargs[i][0];
			//synfig::info(&vargs[i][0]);
		}
		args[idx++] = (char *)NULL;
		
		execvp(ffmpeg_binary_path.c_str(), args);

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

	color_to_pixelformat(buffer, color_buffer, PF_RGB, 0, desc.get_w());

	if(!fwrite(buffer,1,desc.get_w()*3,file))
		return false;

	return true;
}
