/* === S Y N F I G ========================================================= */
/*!	\file trgt_av.cpp
**	\brief \writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Paul Wise
**  Copyright (c) 2008 Gerco Ballintijn
**  ......... ... 2018 Ivan Mahonin
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

// ffmpeg library headers have historically had multiple locations.
// We should check all of the locations to be more portable.

extern "C"
{
#ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
#	include <libavformat/avformat.h>
#elif defined(HAVE_AVFORMAT_H)
#	include <avformat.h>
#elif defined(HAVE_FFMPEG_AVFORMAT_H)
#	include <ffmpeg/avformat.h>
#else
#   define DISABLE_MODULE
#endif

#ifdef WITH_LIBSWSCALE
#ifdef HAVE_LIBSWSCALE_SWSCALE_H
#	include <libswscale/swscale.h>
#elif defined(HAVE_SWSCALE_H)
#	include <swscale.h>
#elif defined(HAVE_FFMPEG_SWSCALE_H)
#	include <ffmpeg/swscale.h>
#endif
#endif
} // extern "C"

#ifndef DISABLE_MODULE
#	include "trgt_av.h"
#	include <synfig/general.h>
#	include <synfig/localization.h>
#	include <cstdio>
#	include <algorithm>
#	include <functional>
#endif

#endif

#ifndef DISABLE_MODULE

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace std;
using namespace etl;

/* === I N F O ============================================================= */

SYNFIG_TARGET_INIT(Target_LibAVCodec);
SYNFIG_TARGET_SET_NAME(Target_LibAVCodec,"libav");
SYNFIG_TARGET_SET_EXT(Target_LibAVCodec,"avi");
SYNFIG_TARGET_SET_VERSION(Target_LibAVCodec,"0.2");
SYNFIG_TARGET_SET_CVS_ID(Target_LibAVCodec,"$Id$");

/* === C L A S S E S & S T R U C T S ======================================= */

static bool av_registered = false;

class Target_LibAVCodec::Internal
{
public:
    const AVCodec *codec;
    AVCodecContext *context;
    AVFrame *frame;
    AVPacket *packet;
    FILE *file;

	Internal(): file(), codec(), context(), frame(), packet() { }
	~Internal() { close(); }

	bool open(const String &filename, const RendDesc &desc) {
		close();

		if (!av_registered) {
			av_register_all();
			av_registered = true;
		}

		// guess format
		// for now force mpeg format instead of guess
		AVOutputFormat *format = NULL;
		//AVOutputFormat *format = av_guess_format(NULL, filename.c_str(), NULL);
		if (!format) {
			warning("Target_LibAVCodec: Unable to guess the output format, defaulting to 'mpeg'");
			format = av_guess_format("mpeg", NULL, NULL);
		}
		if (!format) {
			synfig::error("Target_LibAVCodec: Unable to find 'mpeg' output format");
	    	close();
			return false;
		}

	    codec = avcodec_find_encoder(format->video_codec);
	    if (!codec) {
	    	synfig::error("Target_LibAVCodec: codec not found");
	    	close();
	        return false;
	    }

	    frame = av_frame_alloc();
	    assert(frame);
	    frame->format = context->pix_fmt;
	    frame->width  = context->width;
	    frame->height = context->height;
	    if (av_frame_get_buffer(frame, 32) != 0) {
	    	synfig::error("Target_LibAVCodec: could not alloc the frame data");
	    	av_frame_free(&frame);
	    	close();
	    	return false;
	    }

	    packet = av_packet_alloc();
	    assert(packet);

	    context = avcodec_alloc_context3(codec);
	    assert(context);

	    // put parameters
	    int fps = (int)roundf(desc.get_frame_rate());
	    context->bit_rate = 400*1024*1024/3600; // 400Mb per hour
	    context->width = desc.get_w(); // in most cases resolution must be multiple of two
	    context->width = desc.get_h();
	    context->time_base = (AVRational){1, fps};
	    context->framerate = (AVRational){fps, 1};

	    context->gop_size = fps; // emit one intra frame every second
	    context->pix_fmt = AV_PIX_FMT_YUV420P;

	    // open codec
	    if (avcodec_open2(context, codec, NULL) != 0) {
	    	synfig::error("Target_LibAVCodec: could not open codec");
	    	close();
	        return false;
	    }

	    file = fopen(filename.c_str(), "wb");
	    if (!file) {
	    	synfig::error("Target_LibAVCodec: could not open file %s", filename.c_str());
	    	close();
	    	return false;
	    }

		return true;
	}

	bool encode_frame(const Surface &surface, const Gamma *gamma) {
		// convert frame

		if (av_frame_make_writable(frame) != 0) {
	    	synfig::error("Target_LibAVCodec: could not make frame data writable");
			close();
			return false;
		}
		color_to_pixelformat(
			(unsigned char *)frame->data[0],
			surface[0],
			PF_RGB,
			gamma,
			surface.get_w(),
			surface.get_h(),
			frame->linesize[0],
			surface.get_pitch() );

		// encode frame

	    if (avcodec_send_frame(context, frame) != 0) {
	    	synfig::error("Target_LibAVCodec: error sending a frame for encoding");
			close();
			return false;
	    }
	    while(true) {
	        int res = avcodec_receive_packet(context, packet);
	        if (res == AVERROR(EAGAIN) || res == AVERROR_EOF)
	            break;

	        if (res) {
		    	synfig::error("Target_LibAVCodec: error during encoding");
				close();
				return false;
	        }

	        size_t offset = 0;
	        while(offset < packet->size) {
	        	size_t size = fwrite(packet->data + offset, 1, packet->size - offset, file);
	        	if (size == 0) {
			    	synfig::error("Target_LibAVCodec: cannot write to file");
					close();
					return false;
	        	}
	        	offset += size;
	        }
	        av_packet_unref(packet);
	    }

	    // increment frame counter

	    ++frame->pts;
		return true;
	}

	void close() {
    	if (context) avcodec_free_context(&context);
		if (packet) av_packet_free(&packet);
		if (frame) av_frame_free(&frame);
		if (file) {
		    // add sequence end code to have a real MPEG file
		    const unsigned char endcode[] = { 0, 0, 1, 0xb7 };
		    fwrite(endcode, 1, sizeof(endcode), file);
			fclose(file);
			file = NULL;
		}
	}
};

/* === M E T H O D S ======================================================= */

Target_LibAVCodec::Target_LibAVCodec(
	const char *filename,
	const synfig::TargetParam &/*params*/
):
	internal(new Internal()),
	filename(filename)
{ }

Target_LibAVCodec::~Target_LibAVCodec()
	{ delete internal; }

bool
Target_LibAVCodec::set_rend_desc(RendDesc *given_desc)
{
	// This is where you can determine how you want stuff
	// to be rendered! given_desc is the suggestion, and
	// you need to modify it to suit the needs of the codec.
	// ie: Making the pixel dimensions divisible by 8, etc...
	desc = *given_desc;

    // resolution must be a multiple of two for some codecs
	int w = desc.get_w();
	int h = desc.get_h();
	Point tl = desc.get_tl();
	Point br = desc.get_br();
	Real pw = desc.get_pw();
	Real ph = desc.get_ph();
	if (w & 1) {
		w += 1;
		tl[0] -= pw/2;
		br[0] += pw/2;
	}
	if (h & 1) {
		h += 1;
		tl[1] -= ph/2;
		br[1] += ph/2;
	}
	desc.set_w(w);
	desc.set_h(h);
	desc.set_tl(tl);
	desc.set_br(br);

	desc.set_frame_rate(std::max(1, (int)round(desc.get_frame_rate())));

	return true;
}

void
Target_LibAVCodec::end_frame() {
	internal->encode_frame(surface, &gamma());
}

bool
Target_LibAVCodec::start_frame(synfig::ProgressCallback */*callback*/)
	{ return true; }

Color*
Target_LibAVCodec::start_scanline(int scanline)
	{ return surface[scanline]; }

bool
Target_LibAVCodec::end_scanline()
	{ return true; }

bool Target_LibAVCodec::init(synfig::ProgressCallback *cb)
{
	surface.set_wh(desc.get_w(), desc.get_h());
	if(!internal->open(filename, desc)) {
		synfig::warning("Target_LibAVCodec: Unable to initialize encoders");
		return false;
	}
	return true;
}

#endif
