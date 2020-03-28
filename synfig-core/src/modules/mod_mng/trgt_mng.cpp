/* === S Y N F I G ========================================================= */
/*!	\file trgt_mng.cpp
**	\brief MNG Target Module
**
**	$Id$
**
**	\legal
**	Copyright (c) 2007 Paul Wise
**	Copyright (c) 2007 Chris Moore
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
** You will need to read the PNG and MNG specs to understand this code
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/listimporter.h>
#include <synfig/general.h>

#include "trgt_mng.h"
#include <libmng.h>
#include <ETL/stringf>
#include <cstdio>
#include <algorithm>
#include <functional>
#include <ETL/misc>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(mng_trgt);
SYNFIG_TARGET_SET_NAME(mng_trgt,"mng");
SYNFIG_TARGET_SET_EXT(mng_trgt,"mng");
SYNFIG_TARGET_SET_VERSION(mng_trgt,"0.1");
SYNFIG_TARGET_SET_CVS_ID(mng_trgt,"$Id$");

/* === M E T H O D S ======================================================= */

static mng_ptr MNG_DECL
mng_alloc_proc(mng_size_t size)
{
	return (mng_ptr)calloc(1,size);
}

static void MNG_DECL
mng_free_proc(mng_ptr ptr, mng_size_t size)
{
	free(ptr); return;
}

static mng_bool MNG_DECL
mng_null_proc(mng_handle mng)
{
	// synfig::info("%s:%d mng_trgt::mng_null_proc was called", __FILE__, __LINE__);
	return MNG_TRUE;
}

static mng_bool MNG_DECL
mng_write_proc(mng_handle mng, mng_ptr buf, mng_uint32 size, mng_uint32* written)
{
	FILE* file = (FILE*)mng_get_userdata (mng);
	*written = fwrite(buf, 1, size, file);
	return MNG_TRUE;
}

static mng_bool MNG_DECL
mng_error_proc(mng_handle mng, mng_int32 error,
			   mng_int8 severity, mng_chunkid chunkname,
			   mng_uint32 chunkseq, mng_int32 extra1,
			   mng_int32 extra2, mng_pchar errortext)
{
	synfig::error("%s:%d mng_trgt: error: %s", __FILE__, __LINE__, errortext);
	return MNG_TRUE;
}

mng_trgt::mng_trgt(const char *Filename, const synfig::TargetParam & /* params */):
	file(NULL),
	w(),
	h(),
	mng(NULL),
	multi_image(),
	ready(false),
	imagecount(),
	filename(Filename),
	buffer(NULL),
	color_buffer(NULL),
	zstream(),
	zbuffer(NULL),
	zbuffer_len(0)
{ }

mng_trgt::~mng_trgt()
{
	synfig::info("mng_trgt: ~mng_trgt");
	if (mng != MNG_NULL)
	{
		mng_putchunk_mend(mng);
		if (mng_write(mng) != 0)
		{
			mng_int8 severity;
			mng_chunkid chunkname;
			mng_uint32 chunkseq;
			mng_int32 extra1;
			mng_int32 extra2;
			mng_pchar errortext;
			mng_getlasterror(mng, &severity, &chunkname, &chunkseq, &extra1,&extra2, &errortext);
			synfig::error("mng_trgt: error: couldn't write mng: %s",errortext);
		}
		mng_cleanup (&mng);
	}
	if (file != NULL) { fclose(file); file=NULL; }
	if (buffer != NULL) { delete [] buffer; buffer = NULL; }
	if (color_buffer != NULL) { delete [] color_buffer; color_buffer = NULL; }
	if (zbuffer != NULL) { free(zbuffer); zbuffer = NULL; zbuffer_len = 0; }
}

bool
mng_trgt::set_rend_desc(RendDesc *given_desc)
{
	desc=*given_desc;
	imagecount=desc.get_frame_start();
	if (desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;
	else
		multi_image=false;
	return true;
}


bool
mng_trgt::init(synfig::ProgressCallback * /* cb */)
{
	// synfig::info("%s:%d mng_trgt::init()", __FILE__, __LINE__);

	int frame_rate, num_frames, play_time;
	int num_layers = 1;

	if (multi_image)
	{
		frame_rate = int(desc.get_frame_rate());
		printf("frame rt %d\n", frame_rate);
		num_frames = desc.get_frame_end()-desc.get_frame_start();
		play_time = num_frames;// / frame_rate;
	}
	else
	{
		frame_rate = 0;
		num_frames = 1;
		play_time = 0;
	}

	time_t t = time (NULL);
	struct tm* gmt = gmtime(&t);
	w=desc.get_w(); h=desc.get_h();
	file = fopen(filename.c_str(), POPEN_BINARY_WRITE_TYPE);
	if (file == NULL) goto cleanup_on_error;
	mng = mng_initialize((mng_ptr)file, mng_alloc_proc, mng_free_proc, MNG_NULL);
	if (mng == MNG_NULL) goto cleanup_on_error;
	if (mng_setcb_errorproc(mng, mng_error_proc) != 0) goto cleanup_on_error;
	if (mng_setcb_writedata(mng, mng_write_proc) != 0) goto cleanup_on_error;
	if (mng_setcb_openstream(mng, mng_null_proc) != 0) goto cleanup_on_error;
	if (mng_setcb_closestream(mng, mng_null_proc) != 0) goto cleanup_on_error;
	if (mng_create(mng) != 0) goto cleanup_on_error;
	if (mng_putchunk_mhdr(mng, w, h, frame_rate, num_layers, num_frames, play_time, MNG_SIMPLICITY_VALID|MNG_SIMPLICITY_SIMPLEFEATURES) != 0) goto cleanup_on_error;
	if (mng_putchunk_term(mng, MNG_TERMACTION_REPEAT, MNG_ITERACTION_LASTFRAME, 0, 0x7fffffff) != 0) goto cleanup_on_error;
	{
		char title[] = MNG_TEXT_TITLE;
		if (mng_putchunk_text(mng, sizeof(title), title,
							  get_canvas()->get_name().length(), const_cast<char *>(get_canvas()->get_name().c_str())) != 0)
			goto cleanup_on_error;

		char description[] = MNG_TEXT_DESCRIPTION;
		if (mng_putchunk_text(mng, sizeof(description), description,
							  get_canvas()->get_description().length(), const_cast<char *>(get_canvas()->get_description().c_str())) != 0)
			goto cleanup_on_error;

		char software[] = MNG_TEXT_SOFTWARE; char synfig[] = "SYNFIG";
		if (mng_putchunk_text(mng, sizeof(software), software,
							  sizeof(synfig), synfig) != 0)
			goto cleanup_on_error;
	}
	if (mng_putchunk_phys(mng, MNG_FALSE, round_to_int(desc.get_x_res()),round_to_int(desc.get_y_res()), MNG_UNIT_METER) != 0) goto cleanup_on_error;
	if (mng_putchunk_time(mng, gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec) != 0) goto cleanup_on_error;
	buffer=new unsigned char[(4*w)+1];
	if (buffer == NULL) goto cleanup_on_error;
	color_buffer=new Color[w];
	if (color_buffer == NULL) goto cleanup_on_error;
	return true;

cleanup_on_error:
	ready=false;
	if (mng != MNG_NULL)
	{
		mng_int8 severity;
		mng_chunkid chunkname;
		mng_uint32 chunkseq;
		mng_int32 extra1;
		mng_int32 extra2;
		mng_pchar errortext;
		mng_getlasterror (mng, &severity, &chunkname, &chunkseq, &extra1,&extra2, &errortext);
		synfig::error("mng_trgt: libmng: %s",errortext);
		mng_cleanup (&mng);
	}

	if (file && file!=stdout)
		fclose(file);
	file=NULL;

	if (buffer != NULL)
	{
		delete [] buffer;
		buffer = NULL;
	}

	if (color_buffer != NULL)
	{
		delete [] color_buffer;
		color_buffer = NULL;
	}

	return false;
}

void
mng_trgt::end_frame()
{
	// synfig::info("%s:%d mng_trgt::end_frame()", __FILE__, __LINE__);

	if (deflate(&zstream,Z_FINISH) != Z_STREAM_END)
	{
		synfig::error("%s:%d deflate()", __FILE__, __LINE__);
		return;
	}
	if (deflateEnd(&zstream) != Z_OK)
	{
		synfig::error("%s:%d deflateEnd()", __FILE__, __LINE__);
		return;
	}
	if (mng != MNG_NULL)
	{
		mng_putchunk_idat(mng, zstream.next_out-zbuffer, zbuffer);
		mng_putchunk_iend(mng);
	}
	imagecount++;
	ready=false;
}

bool
mng_trgt::start_frame(synfig::ProgressCallback *callback)
{
	// synfig::info("%s:%d mng_trgt::start_frame()", __FILE__, __LINE__);

	if (mng == MNG_NULL)
	{
		synfig::error("%s:%d mng == MNG_NULL", __FILE__, __LINE__);
		return false;
	}

	if (mng_putchunk_ihdr(mng, w, h, MNG_BITDEPTH_8, MNG_COLORTYPE_RGBA, MNG_COMPRESSION_DEFLATE, MNG_FILTER_ADAPTIVE, MNG_INTERLACE_NONE) != 0)
	{
		synfig::error("%s:%d mng_putchunk_ihdr()", __FILE__, __LINE__);
		return false;
	}

	zstream.zalloc = Z_NULL;
	zstream.zfree = Z_NULL;
	zstream.opaque = Z_NULL;

	if (deflateInit(&zstream, /* Z_BEST_COMPRESSION */ Z_DEFAULT_COMPRESSION) != Z_OK)
	{
		synfig::error("%s:%d deflateInit()", __FILE__, __LINE__);
		return false;
	}

	if (zbuffer == NULL)
	{
		zbuffer_len = deflateBound(&zstream,((4*w)+1)*h); // don't forget the 'filter' byte - one per scanline
		zbuffer = (unsigned char*)realloc(zbuffer, zbuffer_len);
	}

	zstream.avail_out = zbuffer_len;
	zstream.next_out = zbuffer;

	ready=true;

	return true;
}

Color*
mng_trgt::start_scanline(int scanline)
{
	return color_buffer;
}

bool
mng_trgt::end_scanline()
{
	if (!file || !ready)
	{
		synfig::error("%s:%d !file or !ready", __FILE__, __LINE__);
		return false;
	}

	*buffer = MNG_FILTER_NONE;
	color_to_pixelformat(buffer+1, color_buffer, PF_RGB|PF_A, 0, desc.get_w());

	zstream.next_in = buffer;
	zstream.avail_in = (4*w)+1;

	if (deflate(&zstream,Z_NO_FLUSH) != Z_OK) {
		synfig::error("%s:%d deflate()", __FILE__, __LINE__);
		return false;
	}

	return true;
}
