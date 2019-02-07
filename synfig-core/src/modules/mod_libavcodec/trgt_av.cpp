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
#   ifndef DISABLE_MODULE
#   define DISABLE_MODULE
#   endif
#endif

#ifdef HAVE_LIBSWSCALE_SWSCALE_H
#	include <libswscale/swscale.h>
#elif defined(HAVE_SWSCALE_H)
#	include <swscale.h>
#elif defined(HAVE_FFMPEG_SWSCALE_H)
#	include <ffmpeg/swscale.h>
#else
#   ifndef DISABLE_MODULE
#   define DISABLE_MODULE
#   endif
#endif
} // extern "C"

#ifndef DISABLE_MODULE
#	include <cstring>
#	include <algorithm>
#	include <functional>
#	include <synfig/general.h>
#	include <synfig/localization.h>
#	include "trgt_av.h"
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
//Use some non-existing extension to disable exporting through this module
//SYNFIG_TARGET_SET_EXT(Target_LibAVCodec,"avi");
SYNFIG_TARGET_SET_EXT(Target_LibAVCodec,"NONEXISTING-EXTENSION");
SYNFIG_TARGET_SET_VERSION(Target_LibAVCodec,"0.2");
SYNFIG_TARGET_SET_CVS_ID(Target_LibAVCodec,"$Id$");

/* === C L A S S E S & S T R U C T S ======================================= */

static bool av_registered = false;

class Target_LibAVCodec::Internal
{
private:
	AVFormatContext *context;
	AVPacket *packet;
	bool file_opened;
	bool headers_sent;

	const AVCodec *video_codec;
	AVStream *video_stream;
	AVCodecContext *video_context;
	AVFrame *video_frame;
	AVFrame *video_frame_rgb;
	SwsContext *video_swscale_context;

	bool add_video_stream(enum AVCodecID codec_id, const RendDesc &desc) {
		// find the video encoder
		video_codec = avcodec_find_encoder(codec_id);
		if (!video_codec) {
			synfig::error("Target_LibAVCodec: video codec not found");
			close();
			return false;
		}

		video_stream = avformat_new_stream(context, video_codec);
		if (!video_stream) {
			synfig::error("Target_LibAVCodec: could not allocate video stream");
			close();
			return false;
		}

		video_context = avcodec_alloc_context3(video_codec);
		if (!video_context) {
			synfig::error("Target_LibAVCodec: could not allocate an encoding video context");
			close();
			return false;
		}

		// set parameters
		int fps = (int)roundf(desc.get_frame_rate());
		video_context->bit_rate     = 400*1024*1024/3600; // 400Mb per hour
		video_context->width        = desc.get_w();       // in most cases resolution must be multiple of two
		video_context->height       = desc.get_h();
		video_context->coded_width  = video_context->width;
		video_context->coded_height = video_context->height;
		video_context->pix_fmt      = AV_PIX_FMT_YUV420P;
		video_context->gop_size     = fps;                // emit one intra frame every second
		video_context->mb_decision  = FF_MB_DECISION_RD;  // use best acroblock decision algorithm
		video_context->framerate    = (AVRational){ fps, 1 };
		video_context->time_base    = (AVRational){ 1, fps };
		video_stream->time_base     = video_context->time_base;

		// some formats want stream headers to be separate.
		if (context->oformat->flags & AVFMT_GLOBALHEADER)
			video_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		return true;
    }

	bool open_video_stream() {
		if (avcodec_open2(video_context, NULL, NULL) < 0) {
			synfig::error("Target_LibAVCodec: could not open video codec");
			// seems the calling of avcodec_free_context after error will cause crash
			// so just forget about this context
			video_context = NULL;
			close();
			return false;
        }

		// allocate frame
		video_frame = av_frame_alloc();
		assert(video_frame);
		video_frame->format = video_context->pix_fmt;
		video_frame->width  = video_context->width;
		video_frame->height = video_context->height;
		video_frame->pts    = 0;
		if (av_frame_get_buffer(video_frame, 32) < 0) {
			synfig::error("Target_LibAVCodec: could not allocate the video frame data");
			close();
			return false;
		}

		// if the output format is not RGB24, then a temporary picture is needed too.
		if (video_frame->format != AV_PIX_FMT_RGB24) {
			video_frame_rgb = av_frame_alloc();
			assert(video_frame_rgb);
			video_frame_rgb->format = AV_PIX_FMT_RGB24;
			video_frame_rgb->width  = video_frame->width;
			video_frame_rgb->height = video_frame->height;
			if (av_frame_get_buffer(video_frame_rgb, 32) < 0) {
				synfig::error("Target_LibAVCodec: could not allocate the temporary video frame data");
				close();
				return false;
			}

			video_swscale_context = sws_getContext(
				video_frame_rgb->width,
				video_frame_rgb->height,
				(AVPixelFormat)video_frame_rgb->format,
				video_frame->width,
				video_frame->height,
				(AVPixelFormat)video_frame->format,
				SWS_BICUBIC, NULL, NULL, NULL );
			if (!video_swscale_context) {
				synfig::error("Target_LibAVCodec: cannot initialize the conversion context");
				close();
				return false;
			}
		}

		// copy the stream parameters to the muxer
		if (avcodec_parameters_from_context(video_stream->codecpar, video_context) < 0) {
			synfig::error("Target_LibAVCodec: could not copy the video stream parameters");
			close();
			return false;
		}

		return true;
	}

public:
	Internal():
		context(),
		packet(),
		file_opened(),
		headers_sent(),
		video_codec(),
		video_stream(),
		video_context(),
		video_frame(),
		video_frame_rgb(),
		video_swscale_context()
	{ }
	~Internal() { close(); }

	bool open(const String &filename, const RendDesc &desc) {
		close();

		if (!av_registered) {
			av_register_all();
			av_registered = true;
		}

		// guess format
		AVOutputFormat *format = av_guess_format(NULL, filename.c_str(), NULL);
		if (!format) {
			synfig::warning("Target_LibAVCodec: unable to guess the output format, defaulting to MPEG");
			format = av_guess_format("mpeg", NULL, NULL);
		}
		if (!format) {
			synfig::error("Target_LibAVCodec: unable to find 'mpeg' output format");
			close();
			return false;
		}

		// allocate output media context.
		context = avformat_alloc_context();
		assert(context);
		context->oformat = format;
		if (filename.size() + 1 > sizeof(context->filename)) {
			synfig::error(
				"Target_LibAVCodec: filename too long, max length is %d, filename is '%s'",
				sizeof(context->filename) - 1,
				filename.c_str() );
			close();
			return false;
		}
		memcpy(context->filename, filename.c_str(), filename.size() + 1);

		packet = av_packet_alloc();
		assert(packet);

		// add video stream
		if (format->video_codec == AV_CODEC_ID_NONE) {
			synfig::error("Target_LibAVCodec: selected format (%s) does not support video", format->name);
			close();
			return false;
		}
		if (!add_video_stream(format->video_codec, desc))
			return false;
		if (!open_video_stream())
			return false;

		// just print selected format options
		av_dump_format(context, 0, filename.c_str(), 1);

		// open the output file, if needed
		if (!(format->flags & AVFMT_NOFILE)) {
			if (avio_open(&context->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0) {
				synfig::error("Target_LibAVCodec: could not open file for write: %s", filename.c_str());
				close();
				return false;
			}
			file_opened = true;
		} else {
	    	synfig::warning("Target_LibAVCodec: selected format (%s) does not write data to file.", format->name);
		}

		// write the stream header, if any.
		if (avformat_write_header(context, NULL) < 0) {
			synfig::error("Target_LibAVCodec: could not write header");
			close();
            return false;
		}

		return true;
	}

	bool encode_frame(const Surface &surface, const Gamma *gamma, bool last_frame) {
		assert(context);
		if (!context) return false;

		// convert frame

		AVFrame *frame_rgb = video_swscale_context ? video_frame_rgb : video_frame;
		int w = std::min(frame_rgb->width, surface.get_w());
		int h = std::max(frame_rgb->height, surface.get_h());
		if (w != surface.get_w() || h != surface.get_h())
			synfig::warning(
				"Target_LibAVCodec: frame size (%d, %d) does not match to initial RendDesc (%d, %d)",
				surface.get_w(), surface.get_h(), w, h );

		if (av_frame_make_writable(frame_rgb) < 0) {
	    	synfig::error("Target_LibAVCodec: could not make frame data writable");
			close();
			return false;
		}

		color_to_pixelformat(
			(unsigned char *)frame_rgb->data[0],
			surface[0],
			PF_RGB,
			gamma,
			w,
			h,
			frame_rgb->linesize[0],
			surface.get_pitch() );

		if (video_swscale_context)
			sws_scale(
				video_swscale_context,
				(const uint8_t * const *)video_frame_rgb->data,
				video_frame_rgb->linesize,
				0,
				video_frame->height,
				video_frame->data,
				video_frame->linesize );

		// encode frame

		if (avcodec_send_frame(video_context, video_frame) < 0) {
			synfig::error("Target_LibAVCodec: error sending a frame for encoding");
			close();
			return false;
		}
		while(true) {
			int res = avcodec_receive_packet(video_context, packet);
			if (res == AVERROR(EAGAIN) || res == AVERROR_EOF)
				break;
			if (res) {
				synfig::error("Target_LibAVCodec: error during encoding");
				close();
				return false;
			}

			av_packet_rescale_ts(packet, video_context->time_base, video_stream->time_base);
			packet->stream_index = video_stream->index;

			res = av_interleaved_write_frame(context, packet);
			av_packet_unref(packet);
			if (res < 0) {
				synfig::error("Target_LibAVCodec: error while writing video frame");
				close();
				return false;
			}
	    }

	    // increment frame counter

		if (last_frame)
			close();
		else
			++video_frame->pts;

		return true;
	}

	void close() {
		if (headers_sent) {
			if (av_write_trailer(context) < 0)
				synfig::error("Target_LibAVCodec: could not write format trailer");
			headers_sent = false;
		}

		if (video_context) avcodec_free_context(&video_context);
		if (video_swscale_context) {
			sws_freeContext(video_swscale_context);
			video_swscale_context = NULL;
		}
		if (video_frame) av_frame_free(&video_frame);
		if (video_frame_rgb) av_frame_free(&video_frame_rgb);
		video_stream = NULL;
		video_codec = NULL;

		if (context) {
			if (file_opened) {
				avio_close(context->pb);
				context->pb = NULL;
				file_opened = false;
			}
			avformat_free_context(context);
			context = NULL;
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
Target_LibAVCodec::end_frame()
	{ internal->encode_frame(surface, &gamma(), curr_frame_ > desc.get_frame_end()); }

bool
Target_LibAVCodec::start_frame(synfig::ProgressCallback */*callback*/)
	{ return true; }

Color*
Target_LibAVCodec::start_scanline(int scanline)
	{ return surface[scanline]; }

bool
Target_LibAVCodec::end_scanline()
	{ return true; }

bool Target_LibAVCodec::init(synfig::ProgressCallback */*cb*/)
{
	surface.set_wh(desc.get_w(), desc.get_h());
	if (!internal->open(filename, desc)) {
		synfig::warning("Target_LibAVCodec: unable to initialize encoders");
		return false;
	}
	return true;
}

#endif
