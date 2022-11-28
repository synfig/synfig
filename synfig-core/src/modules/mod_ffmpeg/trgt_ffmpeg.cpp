/* === S Y N F I G ========================================================= */
/*!	\file trgt_ffmpeg.cpp
**	\brief FFMPEG Target Module
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2010 Diego Barrios Romero
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

#include "trgt_ffmpeg.h"

#include <ETL/stringf>

#include <synfig/filesystemnative.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/soundprocessor.h>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(ffmpeg_trgt);
SYNFIG_TARGET_SET_NAME(ffmpeg_trgt,"ffmpeg");
SYNFIG_TARGET_SET_EXT(ffmpeg_trgt,"mpg");
SYNFIG_TARGET_SET_VERSION(ffmpeg_trgt,"0.1");

/* === M E T H O D S ======================================================= */

bool
ffmpeg_trgt::does_video_codec_support_alpha_channel(const synfig::String &video_codec) const
{
	const std::vector<const char*> valid_codecs = {
		"libvpx-vp8", "libvpx-vp9", "hap"
	};
	return std::find(valid_codecs.begin(), valid_codecs.end(), video_codec) != valid_codecs.end();
}

ffmpeg_trgt::ffmpeg_trgt(const char *Filename, const synfig::TargetParam &params):
	imagecount(0),
	multi_image(false),
	pipe(nullptr),
	filename(Filename),
	sound_filename(""),
	bitrate()
{
	// Set default video codec and bitrate if they weren't given.
	if (params.video_codec == "none")
		video_codec = "mpeg1video";
	else
		video_codec = params.video_codec;

	if (params.bitrate == -1)
		bitrate = 200;
	else
		bitrate = params.bitrate;

	if (does_video_codec_support_alpha_channel(video_codec))
		set_alpha_mode(TARGET_ALPHA_MODE_KEEP);
	else
		set_alpha_mode(TARGET_ALPHA_MODE_FILL);
}

ffmpeg_trgt::~ffmpeg_trgt()
{
	if(pipe)
	{
		pipe->close();
	}
	pipe = nullptr;

	// Remove temporary sound file
	if (FileSystemNative::instance()->is_file(sound_filename.c_str())) {
		if(FileSystemNative::instance()->remove_recursive(sound_filename.c_str())) {
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
ffmpeg_trgt::init(ProgressCallback* cb = nullptr)
{
	bool with_sound = false;
	const std::string extension = etl::filename_extension(filename);
	const std::vector<const char*> image_only_extensions{".gif", ".mng"};
	const bool does_file_format_support_audio = std::find(image_only_extensions.begin(), image_only_extensions.end(), extension) == image_only_extensions.end();

	if (!does_file_format_support_audio) {
		with_sound = false;
	} else if(!SoundProcessor::subsys_init()) {
		if (cb) cb->error(_("Unable to initialize Sound subsystem"));
		with_sound = false;
	} else {
		auto& fs = FileSystemNative::instance();
		synfig::SoundProcessor soundProcessor;
		soundProcessor.set_infinite(false);
		get_canvas()->fill_sound_processor(soundProcessor);
		// Generate random filename here
		do {
			synfig::GUID guid;
			sound_filename = String(filename)+"."+guid.get_string().substr(0,8)+".wav";
		} while (fs->is_exists(sound_filename));

		soundProcessor.do_export(sound_filename);

		if (!fs->is_exists(sound_filename)) {
			with_sound = false;
		}
	}

	String ffmpeg_binary_path;
#ifdef _WIN32
	// Windows always have ffmpeg
	ffmpeg_binary_path = etl::dirname(synfig::get_binary_path(".")) + "/ffmpeg.exe";
	if (!FileSystemNative::instance()->is_file(ffmpeg_binary_path)) {
		synfig::error("Expected FFmpeg binary not found: %s", ffmpeg_binary_path.c_str());
		ffmpeg_binary_path.clear();
	}
#else
	// Some Linux OS may have `avconv` instead of `ffmpeg`, so let's check both
	const std::vector<std::string> binary_choices = {"ffmpeg", "avconv"};
	for (const auto& bin_name : binary_choices) {
		OS::RunArgs args;
		args.push_back(bin_name);
		OS::RunPipe::Handle pipe = OS::run_async("which", args, OS::RUN_MODE_READ);
		if (!pipe) {
			synfig::error(_("%s: Internal error: couldn't run 'which' async"), "trgt_ffmpeg");
			continue;
		}
		std::string result = pipe->read_contents();
		int status = pipe->close();
		synfig::info("which %s -> [%i] %s", args.get_string().c_str(), status, result.c_str());
		if (status == 0) {
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

	const bool use_alpha = get_alpha_mode() == TARGET_ALPHA_MODE_KEEP;

	std::string video_codec_real = (video_codec == "libx264-lossless" ? "libx264" : video_codec);

	OS::RunArgs vargs;
	if (with_sound) {
		vargs.push_back("-i");
		vargs.push_back(filesystem::Path(sound_filename));
	}
	vargs.push_back("-f");
	vargs.push_back("image2pipe");
	vargs.push_back("-vcodec");
	vargs.push_back(use_alpha ? "pam" : "ppm");
	vargs.push_back("-r");
	{
		// this should avoid conflicts with locale settings
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		vargs.push_back(strprintf("%f", desc.get_frame_rate()));
	}
	vargs.push_back("-i");
	vargs.push_back("pipe:");
	vargs.push_back("-metadata");
	vargs.push_back(strprintf("title=\"%s\"", get_canvas()->get_name().c_str()));
	vargs.push_back("-vcodec");
	vargs.push_back(video_codec_real);
	vargs.push_back("-b:v");
	vargs.push_back(strprintf("%ik", bitrate));
	if (video_codec == "libx264-lossless") {
		vargs.push_back("-tune");
		vargs.push_back("fastdecode");
		vargs.push_back("-pix_fmt");
		vargs.push_back(use_alpha ? "yuva420p" : "yuv420p");
		vargs.push_back("-qp");
		vargs.push_back("0");
	} else if (use_alpha){
		if (video_codec == "hap") {
			vargs.push_back("-format");
			vargs.push_back("hap_alpha");
		}
		vargs.push_back("-pix_fmt");
		vargs.push_back("yuva420p");
	}
	if (with_sound) {
		vargs.push_back("-acodec");
		// MPEG-1 cannot work with 'le' audio, it requires 'be'
		vargs.push_back(video_codec == "mpeg1video" ? "pcm_s16be" : "pcm_s16le");
	}
	vargs.push_back("-y");
	vargs.push_back("-t");
	{
		// this should avoid conflicts with locale settings
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		vargs.push_back(desc.get_duration().get_string(Time::Format::FORMAT_VIDEO));
	}
	// We need "--" to separate filename from arguments (for the case when filename starts with "-")
	if ( filename.substr(0,1) == "-" )
		vargs.push_back("--");

	vargs.push_back(filesystem::Path(filename));

	pipe = OS::run_async(ffmpeg_binary_path, vargs, OS::RUN_MODE_WRITE);

	if(!pipe || !pipe->is_writable())
	{
		synfig::error(_("Unable to open pipe to ffmpeg (no file)"));
		if (pipe) {
			pipe->close();
			pipe = nullptr;
		}
		return false;
	}

	synfig::info(_("Running async command: %s"), pipe->get_command().c_str());

	return true;
}

void
ffmpeg_trgt::end_frame()
{
	//pipe->print(" ");
	pipe->flush();
	imagecount++;
}

bool
ffmpeg_trgt::start_frame(synfig::ProgressCallback */*callback*/)
{
	std::size_t w=desc.get_w(),h=desc.get_h();

	if(!pipe || !pipe->is_writable())
		return false;

	const bool use_alpha = get_alpha_mode() == TARGET_ALPHA_MODE_KEEP;

	if (!use_alpha) {
		pipe->printf("P6\n");
		pipe->printf("%zu %zu\n", w, h);
		pipe->printf("%d\n", 255);
	} else {
		pipe->printf("P7\n");
		pipe->printf("WIDTH %zu\n", w);
		pipe->printf("HEIGHT %zu\n", h);
		pipe->printf("DEPTH 4\n");
		pipe->printf("MAXVAL %d\n", 255);
		pipe->printf("TUPLTYPE RGB_ALPHA\n");
		pipe->printf("ENDHDR\n");
	}

	buffer.resize(w * (use_alpha ? 4 : 3));
	color_buffer.resize(w);

	return true;
}

Color *
ffmpeg_trgt::start_scanline(int /*scanline*/)
{
	return color_buffer.data();
}

bool
ffmpeg_trgt::end_scanline()
{
	if(!pipe)
		return false;

	PixelFormat format = PF_RGB;
	if(get_alpha_mode() == TARGET_ALPHA_MODE_KEEP)
		format |= PF_A;

	color_to_pixelformat(buffer.data(), color_buffer.data(), format, 0, desc.get_w());

	if(!pipe->write(buffer.data(),1,buffer.size()))
		return false;

	return true;
}
