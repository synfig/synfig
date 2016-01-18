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

extern "C"
{

/*
	ffmpeg library headers have historically had multiple locations.
	We should check all of the locations to be more portable.
*/

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

}

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

#ifdef WIN32
#define snprintf	_snprintf
#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace std;
using namespace etl;

/* === I N F O ============================================================= */

SYNFIG_TARGET_INIT(Target_LibAVCodec);
SYNFIG_TARGET_SET_NAME(Target_LibAVCodec,"libav");
SYNFIG_TARGET_SET_EXT(Target_LibAVCodec,"avi");
SYNFIG_TARGET_SET_VERSION(Target_LibAVCodec,"0.1");
SYNFIG_TARGET_SET_CVS_ID(Target_LibAVCodec,"$Id$");

/* === C L A S S E S & S T R U C T S ======================================= */

bool Target_LibAVCodec::registered = false;

//for now compilation
//float STREAM_DURATION = 5.0f;

struct VideoInfo
{
	int	w,h;
	int	fps;

	int bitrate;
};

struct AudioInfo
{
	int	samplerate;	//in HZ
	int	samplesize;	//in bytes
};

AVFrame *alloc_picture(int pix_fmt, int width, int height)
{
    AVFrame *picture;
    uint8_t *picture_buf;
    int size;

    picture = avcodec_alloc_frame();
    if (!picture)
        return NULL;
    size = avpicture_get_size(pix_fmt, width, height);
    picture_buf = (uint8_t *)malloc(size);
    if (!picture_buf) {
        av_free(picture);
        return NULL;
    }
    avpicture_fill((AVPicture *)picture, picture_buf,
                   pix_fmt, width, height);
    return picture;
}

void free_picture(AVFrame *pic)
{
	av_free(pic->data[0]);
	av_free(pic);
}

//the frame must be RGB24
static void convert_surface_frame(AVFrame *pic, const Surface &s, const Gamma &gamma)
{
	unsigned int j;
	Surface::const_pen p = s.begin();
	unsigned int w,h;
	Color c;

	uint8_t *ptr;
	int stride;

	w = s.size().x;
	h = s.size().y;

	ptr = pic->data[0];
	stride = pic->linesize[0];

	for(j = 0; j < h; j++, p.inc_y(), ptr += stride)
	{
		uint8_t *tptr = ptr;

		//use convert_color_format instead...
		#if 0
		const int channels = 3;

		for(int i = 0; i < w; i++, p.inc_x(), tptr += channels)
		{
			c = p.get_value();

			Color::value_type r = c.get_r();
			Color::value_type g = c.get_g();
			Color::value_type b = c.get_b();
			Color::value_type a = c.get_a();

			//premultiply alpha
			r *= a;
			g *= a;
			b *= a;

			//essentially treats it as if it has a background color of black

			//we must also clamp the rgb values [0,1]
			r = min(1.0f,r);
			g = min(1.0f,g);
			b = min(1.0f,b);

			r = max(0.0f,r);
			g = max(0.0f,g);
			b = max(0.0f,b);

			//now scale to range of char [0,255]
			tptr[0] = (int)(r*255);
			tptr[1] = (int)(g*255);
			tptr[2] = (int)(b*255);
		}

		p.dec_x(w);
		#else

		convert_color_format((unsigned char *)tptr,&p.get_value(),w,PF_RGB,gamma);

		#endif
	}
}

//Audio Streamer (abstracts the open, write and close operations for audio streams)
#if 0
class AudioEncoder
{
public:

	void *samples;

	vector<unsigned char>audiobuffer;

	int audio_input_frame_size;

	bool open(AVFormatContext *formatc, AVStream *stream)
	{
		AVCodecContext *context;
		AVCodec *codec;

		context = &stream->codec;

		//find audio encoder
		codec = avcodec_find_encoder(context->codec_id);
		if(!codec)
		{
			synfig::warning("audio-open: could not find codec");
			return 0;
		}

		//open the codec
		if(avcodec_open(context, codec) < 0)
		{
			synfig::warning("audio-open: could not open codec");
			return 0;
		}

		/* hardcoded example for generating samples array*/
		/*
		t = 0;
		tincr = 2 * M_PI * 110.0 / c->sample_rate;
		tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;*/

		audiobuffer.resize(10000);

		/* ugly hack for PCM codecs (will be removed ASAP with new PCM
		   support to compute the input frame size in samples */
		if (context->frame_size <= 1) {
			audio_input_frame_size = audiobuffer.size() / context->channels;
			switch(stream->codec.codec_id) {
			case CODEC_ID_PCM_S16LE:
			case CODEC_ID_PCM_S16BE:
			case CODEC_ID_PCM_U16LE:
			case CODEC_ID_PCM_U16BE:
				audio_input_frame_size >>= 1;
				break;
			default:
				break;
			}
		} else {
			audio_input_frame_size = context->frame_size;
		}

		//hardcoded array
		//samples = (int16_t *)malloc(audio_input_frame_size * 2 * c->channels);

		return true;
	}

	bool write_frame(AVFormatContext *formatc, AVStream *stream, void *samples)
	{
		int size;
		AVCodecContext *context;

		context = &stream->codec;

		//hardcoded in example
		//must read in from somewhere...
		//get_audio_frame(samples, audio_input_frame_size, c->channels);

		//encode the audio
		const short int*samps=(const short int *)samples; 	//assuming it's set from somewhere right now

		size = avcodec_encode_audio(context, &audiobuffer[0], audiobuffer.size(), samps);

		//write the compressed audio to a file
		if(av_write_frame(formatc, stream->index, &audiobuffer[0], size) != 0)
		{
			synfig::warning("audio-write_frame: unable to write the entire audio frame");
			return 0;
		}

		return true;
	}

	void close(AVFormatContext *formatc, AVStream *stream)
	{
		//we may also want to catch delayed frames from here (don't for now)

		if(stream)
		{
			avcodec_close(&stream->codec);
		}

		//if(samples)av_free(samples);
		audiobuffer.resize(0);
	}
};

#endif

class VideoEncoder
{
public:
	AVFrame *encodable;	//for compression and output to a file (in compatible pixel format)

	vector<unsigned char>	videobuffer;

	bool 	startedencoding;

	//int		stream_nb_frames;

	bool open(AVFormatContext *formatc, AVStream *stream)
	{
		if(!formatc || !stream)
		{
			synfig::warning("Attempt to open a video codec with a bad format or stream");
			return false;
		}

		//codec and context
		AVCodec *codec;
		AVCodecContext *context;

		//get from inside stream
		context = stream->codec;

		//search for desired codec (contained in the stream)
		codec = avcodec_find_encoder(context->codec_id);
		if(!codec)
		{
			synfig::warning("Open_video: could not find desired codec");
			return 0;
		}

		//try to open the codec
		if(avcodec_open(context, codec) < 0)
		{
			synfig::warning("open_video: could not open desired codec");
			return 0;
		}

		videobuffer.resize(0);
		if(!(formatc->oformat->flags & AVFMT_RAWPICTURE))
		{
			//resize buffer to desired buffersize
			videobuffer.resize(200000); //TODO: need to figure out a good size
		}

		//allocate the base picture which will be used to encode
		/*picture = alloc_picture(PIX_FMT_RGBA32, context->width, context->height);
		if(!picture)
		{
			synfig::warning("open_video: could not allocate the picture to be encoded");
			return 0;
		}*/

		//if our output (rgb) needs to be translated to a different coordinate system, need a temporary picture in that color space

		/*	Should use defaults of RGB
			Possible formats:
				PIX_FMT_RGB24
				PIX_FMT_BGR24
				PIX_FMT_RGBA32 //stored in cpu endianness (!!!!)

			(possibly translate directly to required coordinate systems later on... less error)
		*/
		encodable = NULL;
		if(context->pix_fmt != PIX_FMT_RGB24)
		{
			encodable = alloc_picture(context->pix_fmt, context->width, context->height);
			if(!encodable)
			{
				synfig::warning("open_video: could not allocate encodable picture");
				return 0;
			}
		}

		return true;
	}

	//write a frame with the frame passed in
	bool write_frame(AVFormatContext *formatc, AVStream *stream, AVFrame *pict)
	{
		if(!formatc || !stream)
		{
			synfig::warning("Attempt to open a video codec with a bad format or stream");
			return false;
		}

		int 			size,
						ret = 0;
		AVCodecContext 	*context = stream->codec;

		/*
		If pict is invalid (NULL), then we are done compressing frames and we are trying to get
		the buffer cleared out (or if it's already in the right format) so no transform necessary
		*/
		if ( pict )
		{
			startedencoding = true;
		}


		if ( pict && context->pix_fmt != PIX_FMT_RGB24 )
		{
			//We're using RGBA at the moment, write custom conversion code later (get less accuracy errors)
#ifdef WITH_LIBSWSCALE
			struct SwsContext* img_convert_ctx =
				sws_getContext(context->width, context->height, PIX_FMT_RGB24,
					context->width, context->height, context->pix_fmt,
					SWS_BICUBIC, NULL, NULL, NULL);

			sws_scale(img_convert_ctx, pict->data, pict->linesize,

				0, context->height, encodable->data,
				encodable->linesize);

			sws_freeContext (img_convert_ctx);
#else
			img_convert((AVPicture *)encodable, context->pix_fmt,
						(AVPicture *)pict, PIX_FMT_RGB24,
						context->width, context->height);
#endif

			pict = encodable;
		}

		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.stream_index = stream->index;
		pkt.data = (uint8_t *)pict;
		pkt.size = sizeof(AVPicture);
		if( context->coded_frame )
			pkt.pts = context->coded_frame->pts;
		if( context->coded_frame && context->coded_frame->key_frame)
			pkt.flags |= PKT_FLAG_KEY;

		//kluge for raw picture format (they said they'd fix)
		if (formatc->oformat->flags & AVFMT_RAWPICTURE)
		{
			ret = av_write_frame(formatc, &pkt);
		}
		else
		{
			//encode our given image
			size = avcodec_encode_video(context, &videobuffer[0], videobuffer.size(), pict);

			//if greater than zero we've got stuff to write
			if (size > 0)
			{
				av_init_packet(&pkt);
				pkt.stream_index = stream->index;
				pkt.data = &videobuffer[0];
				pkt.size = size;
				if( context->coded_frame )
					pkt.pts = context->coded_frame->pts;
				if( context->coded_frame && context->coded_frame->key_frame)
					pkt.flags |= PKT_FLAG_KEY;

				ret = av_write_frame(formatc, &pkt);

				//error detect - possibly throw later...
				if(ret < 0)
				{
					synfig::warning("write_frame: error while writing video frame");
					return false;
				}
			}
			//if 0, it was buffered (if invalid picture we don't have ANY data left)
			else
			{
				//if we're clearing the buffers and there was no stuff to be written, we're done (like in codec example)
				if(pict == NULL)
				{
					return false;
					startedencoding = false;
				}

				//buffered picture
			}
		}

		return true;
	}

	void close(AVFormatContext */*formatc*/, AVStream *stream)
	{
		if(stream)
			avcodec_close(stream->codec);

		if (encodable)
		{
			free_picture(encodable);
			encodable = 0;
		}

		videobuffer.resize(0);
	}
};

class Target_LibAVCodec::LibAVEncoder
{
public:

	bool initialized;

	AVOutputFormat *format;	//reference to global, do not delete

    AVFormatContext *formatc;

	AVStream *video_st;
	//AVStream *audio_st;

	double video_pts;
	//double audio_pts;

	VideoEncoder	vid;
	VideoInfo		vInfo;

	/*AudioEncoder	aud;
	AudioInfo		aInfo;*/

	AVFrame			*picture;	//for encoding to RGB24 (perhaps RGBA later)

	int				frame_count;
	int				num_frames;

	LibAVEncoder()
	{
		format = 0;
		formatc = 0;

		//video settings
		video_st = 0;
		video_pts = 0;

		vid.encodable = 0;
		//vid.stream_nb_frames = 2;	//reasonable default

		initialized = false;
		picture = 0;

		frame_count = 0;
		num_frames = 0;

		//aud.samples = 0;
		//audio_st = 0;
		//audio_pts = 0;
	}

	~LibAVEncoder()
	{
		CleanUp();
	}

	bool Initialize(const char *filename, const char *typestring)
	{
		//guess if we have a type string, otherwise use filename
		if (typestring)
		{
			//formatptr guess_format(type, filename, MIME type)
			format = guess_format(typestring,NULL,NULL);
		}
		else
		{
			format = guess_format(NULL, filename, NULL);
		}

		if(!format)
		{
			synfig::warning("Unable to Guess the output, defaulting to mpeg");
			format = guess_format("mpeg", NULL, NULL);
		}

		if(!format)
		{
			synfig::warning("Unable to find output format");
			return 0;
		}

		//allocate the output context
		formatc = (AVFormatContext *)av_mallocz(sizeof(AVFormatContext));
		if(!formatc)
		{
			synfig::warning("Memory error\n");
			return 0;
		}
		//set the output format to the one we found
		formatc->oformat = format;

		//print the output filename
		snprintf(formatc->filename, sizeof(formatc->filename), "%s", filename);

		//initial
		video_st = NULL;
		//audio_st = NULL;

		//video stream
		if(format->video_codec != CODEC_ID_NONE)
		{
			video_st = add_video_stream(format->video_codec,vInfo);
			if(!video_st)
			{
				av_free(formatc);
			}
		}

		//audio stream
		/*if(format->audio_codec != CODEC_ID_NONE)
		{
			audio_st = add_audio_stream(format->audio_codec,aInfo);
		}*/

		//set output parameters: required in ALL cases

		video_st->codec->time_base= (AVRational){1,vInfo.fps};
		video_st->codec->width = vInfo.w;
		video_st->codec->height = vInfo.h;
		video_st->codec->pix_fmt = PIX_FMT_YUV420P;

		//dump the formatting information as the file header
		dump_format(formatc, 0, filename, 1);

		//open codecs and allocate buffers
		if(video_st)
		{
			if(!vid.open(formatc, video_st))
			{
				synfig::warning("Could not open video encoder");
				return 0;
			}
		}
		/*if(audio_st)
		{
			if(!aud.open(formatc, audio_st))
			{
				synfig::warning("Could not open audio encoder");
				return 0;
			}
		}*/

		//open output file
		if(!(format->flags & AVFMT_NOFILE))
		{
			//use libav's file open function (what does it do differently????)
			if(url_fopen(&formatc->pb, filename, URL_WRONLY) < 0)
			{
				synfig::warning("Unable to open file: %s", filename);
				return 0;
			}
		}

		//allocate the picture to render to
		//may have to retrieve the width, height from the codec... for resizing...
		picture = alloc_picture(PIX_FMT_RGB24,vInfo.w,vInfo.h);//video_st->codec.width, video_st->codec.height);
		if(!picture)
		{
			synfig::warning("Unable to allocate the temporary AVFrame surface");
			return 0;
		}

		initialized = true;
		//vInfo.w = video_st->codec.width;
		//vInfo.h = video_st->codec.height;

		//write the stream header
		av_write_header(formatc);

		return true;
	}

	void CleanUp()
	{
		unsigned int i;

		if(picture) free_picture(picture);

		//do all the clean up file rendering
		if(formatc && video_st)
		{
			//want to scan in delayed frames until no longer needed (TODO)
			if(vid.startedencoding) while( vid.write_frame(formatc, video_st, 0) );

			//may want to move this... probably to the end of the last frame...
			av_write_trailer(formatc);
		}

		//close codecs
		if (video_st)
			vid.close(formatc,video_st);
		/*if (audio_st)
			aud.close(formatc,audio_st);*/

		/* write the trailer, if any */
		if(formatc)
		{
			/* free the streams */
			for(i = 0; i < formatc->nb_streams; i++)
			{
				av_freep(&formatc->streams[i]);
			}

			if(!(format->flags & AVFMT_NOFILE))
			{
				/* close the output file */
#if LIBAVFORMAT_VERSION_INT >= (52<<16)
				url_fclose(formatc->pb);
#else
				url_fclose(&formatc->pb);
#endif
			}

			/* free the stream */
			av_free(formatc);
		}

		initialized = false;

		format = 0;
		formatc = 0;

		//video settings
		video_st = 0;
		video_pts = 0;

		vid.encodable = 0;
		//vid.stream_nb_frames = 2;	//reasonable default

		initialized = false;
		picture = 0;
	}

	//create a video output stream
	AVStream *add_video_stream(int codec_id, const VideoInfo &info)
	{
		AVCodecContext *context;
		AVStream *st;

		st = av_new_stream(formatc, 0);
		if(!st)
		{
			synfig::warning("video-add_stream: Unable to allocate stream");
			return 0;
		}

		context = st->codec;
		context->codec_id = (CodecID)codec_id;
		context->codec_type = CODEC_TYPE_VIDEO;

		//PARAMETERS MUST BE PASSED IN SOMEHOW (ANOTHER FUNCTION PARAMETER???)

		/* resolution must be a multiple of two */
		context->width = info.w;
		context->height = info.h;

		//have another way to input these
		context->bit_rate = info.bitrate; //TODO: Make dependant on the quality

		/* frames per second */
		// FIXME: Port next two lines to recent libavcodec versions
		//context->frame_rate = info.fps;
		//context->frame_rate_base = 1;

		/* "High Quality" */
		context->mb_decision=FF_MB_DECISION_BITS;

		context->gop_size = info.fps/4; /* emit one intra frame every twelve frames at most */

		//HACK: MPEG requires b frames be set... any better way to do this?
		if (context->codec_id == CODEC_ID_MPEG1VIDEO ||
			context->codec_id == CODEC_ID_MPEG2VIDEO)
		{
			/* just for testing, we also add B frames */
			context->max_b_frames = 2;
		}

		return st;
	}

	// add an audio output stream
	AVStream *add_audio_stream(int codec_id,const AudioInfo &/*aInfo*/)
	{
		AVCodecContext *context;
		AVStream *stream;

		stream = av_new_stream(formatc, 1);
		if(!stream)
		{
			synfig::warning("could not alloc stream");
			return 0;
		}

		context = stream->codec;
		context->codec_id = (CodecID)codec_id;
		context->codec_type = CODEC_TYPE_AUDIO;

		/* put sample parameters */
		context->bit_rate = 64000;
		context->sample_rate = 44100;
		context->channels = 2;

		return stream;
	}
};

/* === M E T H O D S ======================================================= */

Target_LibAVCodec::Target_LibAVCodec(const char *Filename,
									 const synfig::TargetParam& /* params */):
	filename(Filename)
{
	if(!registered)
	{
		registered = true;
		av_register_all();
	}
	set_remove_alpha();

	data = new LibAVEncoder;
}

Target_LibAVCodec::~Target_LibAVCodec()
{
	data->CleanUp();
}

bool
Target_LibAVCodec::set_rend_desc(RendDesc *given_desc)
{
	// This is where you can determine how you want stuff
	// to be rendered! given_desc is the suggestion, and
	// you need to modify it to suit the needs of the codec.
	// ie: Making the pixel dimensions divisible by 8, etc...

	desc=*given_desc;

	//resize surface (round even)
	int w = desc.get_w();
	int h = desc.get_h();
	Point tl = desc.get_tl();
	Point br = desc.get_br();
	Real pw = desc.get_pw();
	Real ph = desc.get_ph();

	//resize to the size it should be...
	//desc.set_subwindow(-offx/2,-offy/2, desc.get_w() - offx?(offx + 8):0, desc.get_h() - offy?(offy + 8):0);

	//if resolution is broken, change the size... or something
	//budge to nearest pixel values
	if(w&1)
	{
		w += 1;
		tl[0] -= pw/2;
		br[0] += pw/2;
	}

	if(h&1)
	{
		h += 1;
		tl[1] -= ph/2;
		br[1] += ph/2;
	}

	desc.set_w(w);
	desc.set_h(h);
	desc.set_tl(tl);
	desc.set_br(br);

	data->vInfo.w = w;
	data->vInfo.h = h;

	//may want to round frame rate
	data->vInfo.fps = (int)floor(desc.get_frame_rate()+0.5);
#define MEGABYTES_PER_HOUR(x)		(((x)*1024/3600*1024*8)/*/640*w/480*h*/)
	data->vInfo.bitrate = MEGABYTES_PER_HOUR(400);
	//data->vInfo.bitrate = 800000;	//make customizable somehow

	desc.set_frame_rate(data->vInfo.fps);

	data->frame_count = desc.get_frame_start();
	data->num_frames = desc.get_frame_end()+1; //number of frames should be 1 greater than the last one

	surface.set_wh(data->vInfo.w,data->vInfo.h);

	return true;
}

void
Target_LibAVCodec::end_frame()
{
	//AVStream *audio_st = data->audio_st;
	AVStream *video_st = data->video_st;

	AVFormatContext *formatc = data->formatc;

	//double &audio_pts = data->audio_pts;
	//double &video_pts = data->video_pts;

	//ignore audio for now
	/*if (audio_st)
		audio_pts = (double)audio_st->pts.val * formatc->pts_num / formatc->pts_den;
	else
		audio_pts = 0.0;

	if (video_st)
		video_pts = (double)video_st->pts.val * formatc->pts_num / formatc->pts_den;
	else
		video_pts = 0.0;*/

	//hardcoded crappiness
	/*if ((!audio_st || audio_pts >= STREAM_DURATION) &&
		(!video_st || video_pts >= STREAM_DURATION))
		break;*/

	if(data->frame_count >= data->num_frames) return;

	//copy the current surface to the buffer
	if(data->picture)convert_surface_frame(data->picture,surface,gamma());

	//encode the frame and write it to the file
	if(!data->vid.write_frame(formatc,video_st,data->picture))
	{
		synfig::warning("Unable to write a frame");
	}

	data->frame_count++;

	if(data->frame_count >= data->num_frames)
	{
		data->CleanUp();
	}

	/* write interleaved audio and video frames */
	/*if (!video_st || (video_st && audio_st && audio_pts < video_pts)) {
		data->aud.write_frame(formatc,audio_st);
	} else {
		data->vid.write_frame(formatc,video_st);
    }*/
}

bool
Target_LibAVCodec::start_frame(synfig::ProgressCallback */*callback*/)
{
	//prepare all the color buffer stuff, etc.

	return true;
}

Color *
Target_LibAVCodec::start_scanline(int scanline)
{
	return surface[scanline];

	return 0;	// This should kill the render process
}

bool
Target_LibAVCodec::end_scanline()
{
	//don't need to do anything until the whole frame is done
	return true;
}

bool Target_LibAVCodec::init(synfig::ProgressCallback *cb)
{
	//hardcoded test for mpeg
	if(!data->Initialize(filename.c_str(),NULL))
	{
		synfig::warning("Unable to Initialize the audio video encoders");
		return 0;
	}

	return true;
}

#endif
