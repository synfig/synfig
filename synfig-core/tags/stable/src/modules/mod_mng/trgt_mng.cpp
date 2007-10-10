/* === S Y N F I G ========================================================= */
/*!	\file trgt_mng.cpp
**	\brief MNG Target Module
**
**	$Id$
**
**	\legal
**	Copyright 2007 Paul Wise
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
** FIXME: THIS DOES NOT ACTUALLY WORK YET
**
** You will need to read the PNG and MNG specs to understand this code
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#define SYNFIG_TARGET

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

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

static mng_ptr MNG_DECL mng_alloc_proc(mng_size_t size){
	return (mng_ptr)calloc(1,size);
}

static void MNG_DECL mng_free_proc(mng_ptr ptr, mng_size_t size)
{
  free(ptr); return;
}

static mng_bool MNG_DECL mng_null_proc(mng_handle mng)
{
	synfig::error("mng_trgt: error: mng_null_proc");
	return MNG_TRUE;
}

static mng_bool MNG_DECL mng_write_proc(mng_handle mng, mng_ptr buf, mng_uint32 size, mng_uint32* written)
{
	synfig::error("mng_trgt: error: mng_write_proc");
	FILE* file = (FILE*)mng_get_userdata (mng);
	*written = fwrite(buf, 1, size, file);
	return MNG_TRUE;
}

static mng_bool MNG_DECL mng_error_proc(
	mng_handle mng, mng_int32 error, mng_int8 severity, mng_chunkid chunkname,
	mng_uint32 chunkseq, mng_int32 extra1, mng_int32 extra2, mng_pchar errortext){
	mng_trgt* me = (mng_trgt*)mng_get_userdata (mng);
	fprintf(stderr,"mng_trgt: error: %s",errortext);
	// me->ready=false;
	return MNG_TRUE;
}

mng_trgt::mng_trgt(const char *Filename):
	filename(Filename)
{
	file=NULL;
	buffer=NULL;
	color_buffer=NULL;	
	zbuffer=NULL;	
	zbuffer_len=0;
	ready=false;
}

mng_trgt::~mng_trgt()
{
	synfig::error("mng_trgt: error: ~mng_trgt");
	if( mng != MNG_NULL){
		mng_putchunk_mend(mng);
		if( mng_write(mng) != 0 ){
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
	if( file != NULL ) fclose(file); file=NULL;
	if( buffer != NULL ){ delete [] buffer; buffer = NULL; }
	if( color_buffer != NULL ){ delete [] color_buffer; color_buffer = NULL; }
	if( zbuffer != NULL ){ free(zbuffer); zbuffer = NULL; zbuffer_len = 0; }
}

bool
mng_trgt::set_rend_desc(RendDesc *given_desc)
{
	desc=*given_desc;
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;
	else
		multi_image=false;
	return true;
}


bool
mng_trgt::init(){
	time_t t = time (NULL);
	struct tm* gmt = gmtime(&t);
	w=desc.get_w(); h=desc.get_h();
	//synfig::error("mng_trgt: init %d %d",w,h); 
	file = fopen(filename.c_str(), "wb");
	if( file == NULL ) goto cleanup_on_error;
	mng = mng_initialize((mng_ptr)file, mng_alloc_proc, mng_free_proc, MNG_NULL);
	if(mng == MNG_NULL) goto cleanup_on_error;
	if( mng_setcb_errorproc(mng, mng_error_proc) != 0 ) goto cleanup_on_error;
	if( mng_setcb_writedata(mng, mng_write_proc) != 0 ) goto cleanup_on_error;
	if( mng_setcb_openstream(mng, mng_null_proc) != 0 ) goto cleanup_on_error;
	if( mng_setcb_closestream(mng, mng_null_proc) != 0 ) goto cleanup_on_error;
	if( mng_create(mng) != 0 ) goto cleanup_on_error;
	if( mng_putchunk_mhdr(mng, w, h, multi_image?1000:0, 1, desc.get_frame_end()-desc.get_frame_start(), 0, 0) != 0 ) goto cleanup_on_error;
	if( mng_putchunk_term(mng, MNG_TERMACTION_REPEAT, MNG_ITERACTION_LASTFRAME, 0, 0x7fffffff) != 0 ) goto cleanup_on_error;
	if( mng_putchunk_text(mng, sizeof(MNG_TEXT_TITLE), MNG_TEXT_TITLE, get_canvas()->get_name().length(), const_cast<char *>(get_canvas()->get_name().c_str()) ) != 0 ) goto cleanup_on_error;
	if( mng_putchunk_text(mng, sizeof(MNG_TEXT_DESCRIPTION), MNG_TEXT_DESCRIPTION, get_canvas()->get_description().length(), const_cast<char *>(get_canvas()->get_description().c_str()) ) != 0 ) goto cleanup_on_error;
	if( mng_putchunk_text(mng, sizeof(MNG_TEXT_SOFTWARE), MNG_TEXT_SOFTWARE, sizeof("SYNFIG"), "SYNFIG" ) != 0 ) goto cleanup_on_error;
	/* FIXME: not sure if this is correct */
	if( mng_putchunk_gama(mng, MNG_FALSE, (int)(1.0/gamma().get_gamma()*100000)) != 0 ) goto cleanup_on_error;
	if( mng_putchunk_phyg(mng, MNG_FALSE,round_to_int(desc.get_x_res()),round_to_int(desc.get_y_res()),MNG_UNIT_METER) != 0 ) goto cleanup_on_error;
	if( mng_putchunk_time(mng, gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec) != 0 ) goto cleanup_on_error;
	buffer=new unsigned char[4*w];
	if( buffer == NULL ) goto cleanup_on_error;
	color_buffer=new Color[w];
	if( color_buffer == NULL ) goto cleanup_on_error;
	return true;
	
cleanup_on_error:
	ready=false;
	if( mng != MNG_NULL){
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
	if( file && file!=stdout ) fclose(file); file=NULL;
	if( buffer != NULL ){ delete [] buffer; buffer = NULL; }
	if( color_buffer != NULL ){ delete [] color_buffer; color_buffer = NULL; }
	return false;	
}

void
mng_trgt::end_frame()
{
	//synfig::error("mng_trgt: endf %d %d",w,h); 
	//synfig::error("mng_trgt: error: endframe");
	deflate(&zstream,Z_FINISH);
	deflateEnd(&zstream);
	if( mng != MNG_NULL ){
		mng_int8 severity;
		mng_chunkid chunkname;
		mng_uint32 chunkseq;
		mng_int32 extra1;
		mng_int32 extra2;
		mng_pchar errortext;
		mng_putchunk_idat(mng, zbuffer_len, zbuffer);
		//mng_getlasterror (mng, &severity, &chunkname, &chunkseq, &extra1,&extra2, &errortext);
		//synfig::error("mng_trgt: libmng: %s %d",errortext,zbuffer_len); 
		mng_putchunk_iend(mng);
		//mng_getlasterror (mng, &severity, &chunkname, &chunkseq, &extra1,&extra2, &errortext);
		//synfig::error("mng_trgt: libmng: %s %d",errortext); 
	}
	imagecount++;
	ready=false;
}

bool
mng_trgt::start_frame(synfig::ProgressCallback *callback)
{
	//synfig::error("mng_trgt: startf %d %d",w,h); 
	//synfig::error("mng_trgt: error: startframe");
	/*
	FIXME: figure out if we need this
	mng_putchunk_fram(mng,
		MNG_FALSE,
		MNG_FRAMINGMODE_NOCHANGE,
		0,
		NULL,
		MNG_CHANGEDELAY_NO,
		MNG_CHANGETIMOUT_NO,
		MNG_CHANGECLIPPING_NO,
		MNG_CHANGESYNCID_NO,
		0,
		0,
		0,
		layer_offset_x,
		layer_offset_x + layer_cols,
		layer_offset_y,
		layer_offset_y + layer_rows,
		0,
		0)
	mng_getchunk_fram(mng_handle       hHandle,
		mng_handle       hChunk,
		mng_bool         *bEmpty,
		mng_uint8        *iMode,
		mng_uint32       *iNamesize,
		mng_pchar        *zName,
		mng_uint8        *iChangedelay,
		mng_uint8        *iChangetimeout,
		mng_uint8        *iChangeclipping,
		mng_uint8        *iChangesyncid,
		mng_uint32       *iDelay,
		mng_uint32       *iTimeout,
		mng_uint8        *iBoundarytype,
		mng_int32        *iBoundaryl,
		mng_int32        *iBoundaryr,
		mng_int32        *iBoundaryt,
		mng_int32        *iBoundaryb,
		mng_uint32       *iCount,
		mng_uint32p      *pSyncids);
	*/
	if( mng == MNG_NULL ) return false;
	if( mng_putchunk_defi(mng,0,MNG_DONOTSHOW_VISIBLE,MNG_ABSTRACT,MNG_FALSE,0,0,MNG_FALSE,0,0,0,0) != 0) return false;
	if( mng_putchunk_ihdr(mng,w,h,MNG_BITDEPTH_8,MNG_COLORTYPE_RGBA,MNG_COMPRESSION_DEFLATE, 0/*MNG_FILTER_NO_DIFFERING*/,MNG_INTERLACE_NONE) != 0) return false;
	zstream.zalloc = Z_NULL;
	zstream.zfree = Z_NULL;
	zstream.opaque = Z_NULL;
	zstream.data_type = Z_BINARY; 
	if( deflateInit(&zstream, Z_DEFAULT_COMPRESSION) != Z_OK ) return false;
	//zbuffer_len = deflateBound(&zstream,4*w*h);
	if(zbuffer == NULL){
		zbuffer_len = 4*w*h;
		zbuffer = (unsigned char*)realloc(zbuffer, zbuffer_len);
		zstream.next_out = zbuffer;
		zstream.avail_out = zbuffer_len;
	}
	ready=true;
	return true;
}

Color *
mng_trgt::start_scanline(int scanline)
{
	//synfig::error("mng_trgt: starts %d %d",w,h); 
	//synfig::error("mng_trgt: error: startscanline");
	return color_buffer;
}

bool
mng_trgt::end_scanline()
{
	//synfig::error("mng_trgt: ends %d %d",w,h); 
	//synfig::error("mng_trgt: error: endscanline");
	if(!file || !ready)
		return false;
	convert_color_format(buffer, color_buffer, desc.get_w(), PF_RGB|PF_A, gamma());
	/* FIXME: Implement buffer management */
	zstream.next_in = buffer;
	zstream.avail_in = 4*w;
	deflate(&zstream,Z_NO_FLUSH);
	return true;
}
