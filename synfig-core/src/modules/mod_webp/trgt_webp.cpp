/* === S Y N F I G ========================================================= */
/*!	\file trgt_webp.cpp
 **	\brief WEBP (via FFMPEG) Target Module
 **
 **	\legal
 **	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
 **	Copyright (c) 2007 Chris Moore
 **	Copyright (c) 2010 Diego Barrios Romero
 **	Copyright (c) 2022 BobSynfig
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

#include "trgt_webp.h"

#include <ETL/stringf>

#include <synfig/filesystemnative.h>
#include <synfig/general.h>
#include <synfig/localization.h>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT       (webp_trgt);
SYNFIG_TARGET_SET_NAME   (webp_trgt, "libwebp");
SYNFIG_TARGET_SET_EXT    (webp_trgt, "webp"   );
SYNFIG_TARGET_SET_VERSION(webp_trgt, "0.1"    );

/* === M E T H O D S ======================================================= */

bool
webp_trgt::does_video_codec_support_alpha_channel( const synfig::String &video_codec ) const
{
/* TO BE CHANGED !*/
	const std::vector<const char*> valid_codecs = {
		"libwebp_anim" //vp8
	};
	return std::find( valid_codecs.begin()
	                , valid_codecs.end()
	                , video_codec
	                ) != valid_codecs.end();

}

webp_trgt::webp_trgt( const char *Filename, const synfig::TargetParam &params ):
	imagecount (0),
	multi_image(false   ),
	pipe       (nullptr ),
	filename   (Filename),
	bitrate    (),
	loop       (params.loop    ), //true
	lossless   (params.lossless), //true
	quality    (params.quality ), //75.0
	comp_lvl   (params.comp_lvl), // 6
	preset     (params.preset  )  //"default"
{
	// Set default video codec if it wasn't given.
	if (    params.video_codec != "libwebp_anim"
	     && params.video_codec != "libwebp"
	   )
		video_codec = "libwebp_anim";
	else
		video_codec = params.video_codec;

	if (does_video_codec_support_alpha_channel( video_codec ))
		set_alpha_mode( TARGET_ALPHA_MODE_KEEP );
	else
		set_alpha_mode( TARGET_ALPHA_MODE_FILL );
}

webp_trgt::~webp_trgt()
{
	if (pipe) pipe->close();
	pipe = nullptr;
}

bool
webp_trgt::set_rend_desc(RendDesc *given_desc)
{
	//given_desc->set_pixel_format(PF_RGB);

	// Make sure that the width and height
	// are multiples of 8
	//given_desc->set_w((given_desc->get_w()+4)/8*8);
	//given_desc->set_h((given_desc->get_h()+4)/8*8);

	// Make sure our width is divisible by two
	//given_desc->set_w(given_desc->get_w()*2/2);
	//given_desc->set_h(given_desc->get_h()*2/2);

	//Max 16383 x 16383

	desc = *given_desc;

	return true;
}

bool
webp_trgt::init( ProgressCallback* cb = nullptr )
{

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
			synfig::error(_("%s: Internal error: couldn't run 'which' async"), "trgt_webp");
			continue;
		}

		std::string result = pipe->read_contents();
		int         status = pipe->close();

		synfig::info("which %s -> [%i] %s", args.get_string().c_str(), status, result.c_str());
		if ( status == 0 ) {
			ffmpeg_binary_path = bin_name; // we can use `result` value here, but in this case we need to remove trailing `\n`
			break;
		}
	}
#endif

	if ( ffmpeg_binary_path.empty() ) {
		if (cb) cb->error(_("Error: No FFmpeg binary found.\n\nPlease install \"ffmpeg\" or \"avconv\" (libav-tools package)."));
		return false;
	}

	imagecount = desc.get_frame_start();
	if ( desc.get_frame_end() - desc.get_frame_start() > 0 )
		multi_image = true;

	const bool use_alpha = get_alpha_mode() == TARGET_ALPHA_MODE_KEEP;

	OS::RunArgs vargs;
/*
	vargs.push_back( "-analyzeduration");    vargs.push_back( "2147483647" );
	vargs.push_back( "-probesize"      );    vargs.push_back( "2147483647" );
*/
	//Overwrite output files
	vargs.push_back( "-y" );

	//Use Pipe as input stream
	vargs.push_back({"-f",                 "image2pipe"                   });
	vargs.push_back({"-vcodec",            use_alpha ? "pam" : "ppm"      });

	vargs.push_back({"-i",                 "pipe:"                        });
	vargs.push_back({"-metadata",
	                 strprintf( "title=\"%s\"",
	                            get_canvas()->get_name().c_str())         });
	vargs.push_back({"-vcodec",            video_codec                    });
	vargs.push_back({"-loop",              loop     ? "0" : "1"           });
	vargs.push_back({"-lossless",          lossless ? "1" : "0"           });
	vargs.push_back({"-compression_level", strprintf( "%i", comp_lvl )    });
	//vargs.push_back({"-vsync",             "0"                          }); Not with rate!
	vargs.push_back({"-preset",            preset                         });
	vargs.push_back({"-pix_fmt",           use_alpha?"yuva420p":"yuv420p" }); //Needed for transparency
	vargs.push_back( "-an" );

	{//Duration + Quality + Frame rate
		// this should avoid conflicts with locale settings
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		//Quality
		vargs.push_back( {"-q:v", strprintf( "%.1f", quality ) });
		//Duration
		vargs.push_back( {"-t",
		                  desc.get_duration().get_string(Time::Format::FORMAT_VIDEO)});
		//Frame rate
		vargs.push_back( {"-r",
		                  strprintf( "%.3f", desc.get_frame_rate() ) });
	}

	// We need "--" to separate filename from arguments
	//(for the case when filename starts with "-")
	if ( filename.substr(0, 1) == "-" ) vargs.push_back("--");

	vargs.push_back( filesystem::Path(filename) );

	pipe = OS::run_async(ffmpeg_binary_path, vargs, OS::RUN_MODE_WRITE);

	if ( !pipe || !pipe->is_writable() )
	{
		synfig::error( _("Unable to open pipe to ffmpeg (no file)") );
		if (pipe) {
			pipe->close();
			pipe = nullptr;
		}
		return false;
	}

	synfig::info( _("Running async command: %s")
	            , pipe->get_command().c_str()  );

	return true;
}

void
webp_trgt::end_frame()
{
	pipe->flush();
	imagecount++;
}

bool
webp_trgt::start_frame(synfig::ProgressCallback */*callback*/)
{
	std::size_t w = desc.get_w();
	std::size_t h = desc.get_h();

	if ( !pipe || !pipe->is_writable() )
		return false;

	const bool use_alpha = get_alpha_mode() == TARGET_ALPHA_MODE_KEEP;

	if (!use_alpha) {
		pipe->printf( "P6\n"                 );
		pipe->printf( "%zu %zu\n",      w, h );
		pipe->printf( "%d\n",            255 );
	} else {
		pipe->printf( "P7\n"                 );
		pipe->printf( "WIDTH %zu\n",       w );
		pipe->printf( "HEIGHT %zu\n",      h );
		pipe->printf( "DEPTH 4\n"            );
		pipe->printf( "MAXVAL %d\n",     255 );
		pipe->printf( "TUPLTYPE RGB_ALPHA\n" );
		pipe->printf( "ENDHDR\n"             );
	}

	buffer.resize(w * (use_alpha ? 4 : 3));
	color_buffer.resize(w);

	return true;
}

Color *
webp_trgt::start_scanline(int /*scanline*/)
{
	return color_buffer.data();
}

bool
webp_trgt::end_scanline()
{
	if (!pipe) return false;

	const bool use_alpha = get_alpha_mode() == TARGET_ALPHA_MODE_KEEP;

	PixelFormat format = use_alpha ? PF_RGB|PF_A : PF_RGB;

	color_to_pixelformat( buffer.data()
	                    , color_buffer.data()
	                    , format
	                    , 0
	                    , desc.get_w()
	                    );

	if ( !pipe->write( buffer.data(), 1, buffer.size() ) )
		return false;

	return true;
}
