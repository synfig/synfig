/* === S Y N F I G ========================================================= */
/*!	\file trgt_png.cpp
**	\brief png_trgt Target Module
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <glib/gstdio.h>
#include "trgt_png.h"
#include <png.h>
#include <cstdio>
#include <ETL/misc>
#include <string.h>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(png_trgt);
SYNFIG_TARGET_SET_NAME(png_trgt,"png");
SYNFIG_TARGET_SET_EXT(png_trgt,"png");
SYNFIG_TARGET_SET_VERSION(png_trgt,"0.1");

/* === M E T H O D S ======================================================= */

void
png_trgt::png_out_error(png_struct *png_data,const char *msg)
{
	png_trgt *me=(png_trgt*)png_get_error_ptr(png_data);
	synfig::error(strprintf("png_trgt: error: %s",msg));
	me->ready=false;
}

void
png_trgt::png_out_warning(png_struct *png_data,const char *msg)
{
	png_trgt *me=(png_trgt*)png_get_error_ptr(png_data);
	synfig::warning(strprintf("png_trgt: warning: %s",msg));
	me->ready=false;
}


//Target *png_trgt::New(const char *filename){	return new png_trgt(filename);}

png_trgt::png_trgt(const char *Filename, const synfig::TargetParam &params):
	file(NULL),
	png_ptr(NULL),
	info_ptr(NULL),
	multi_image(),
	ready(false),
	imagecount(),
	filename(Filename),
	buffer(NULL),
	color_buffer(NULL),
	sequence_separator(params.sequence_separator)
{ }

png_trgt::~png_trgt()
{
	if(file)
		fclose(file);
	file=NULL;
	delete [] buffer;
	delete [] color_buffer;
}

bool
png_trgt::set_rend_desc(RendDesc *given_desc)
{
	//given_desc->set_pixel_format(PixelFormat((int)PF_RGB|(int)PF_A));
	desc=*given_desc;
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;
	else
		multi_image=false;
	return true;
}

void
png_trgt::end_frame()
{
	if(ready && file)
	{
		png_write_end(png_ptr,info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
	}

	if(file && file!=stdout)
		fclose(file);
	file=NULL;
	imagecount++;
	ready=false;
}

bool
png_trgt::start_frame(synfig::ProgressCallback *callback)
{
	int w=desc.get_w(),h=desc.get_h();

	if(file && file!=stdout)
		fclose(file);
	if(filename=="-")
	{
		if(callback)callback->task(strprintf("(stdout) %d",imagecount).c_str());
		file=stdout;
	}
	else if(multi_image)
	{
		String newfilename(filename_sans_extension(filename) +
						   sequence_separator +
						   etl::strprintf("%04d",imagecount) +
						   filename_extension(filename));
		file=g_fopen(newfilename.c_str(),POPEN_BINARY_WRITE_TYPE);
		if(callback)callback->task(newfilename);
	}
	else
	{
		file=g_fopen(filename.c_str(),POPEN_BINARY_WRITE_TYPE);
		if(callback)callback->task(filename);
	}

	if(!file)
		return false;

	delete [] buffer;
	buffer=new unsigned char[4*w];

	delete [] color_buffer;
	color_buffer=new Color[w];

	png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)this,png_out_error, png_out_warning);
	if (!png_ptr)
	{
		synfig::error("Unable to setup PNG struct");
		fclose(file);
		return false;
	}

	info_ptr= png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		synfig::error("Unable to setup PNG info struct");
		fclose(file);
		png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		synfig::error("Unable to setup longjump");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(file);
		return false;
	}
	png_init_io(png_ptr,file);
	png_set_filter(png_ptr,0,PNG_FILTER_NONE);

	setjmp(png_jmpbuf(png_ptr));
	if (get_alpha_mode()==TARGET_ALPHA_MODE_KEEP)
		png_set_IHDR(png_ptr,info_ptr,w,h,8,PNG_COLOR_TYPE_RGBA,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
	else
		png_set_IHDR(png_ptr,info_ptr,w,h,8,PNG_COLOR_TYPE_RGB,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);

	// Write the physical size
	png_set_pHYs(png_ptr,info_ptr,round_to_int(desc.get_x_res()),round_to_int(desc.get_y_res()),PNG_RESOLUTION_METER);
	
	// Explicit set gamma value to 2.2 (it's a default value)
	png_set_gAMA(png_ptr,info_ptr,1/2.2);

	char title      [] = "Title";
	char description[] = "Description";
	char software   [] = "Software";
	char synfig     [] = "SYNFIG";

	// Output any text info along with the file
	png_text comments[3];
	memset(comments, 0, sizeof(comments));

	comments[0].compression = PNG_TEXT_COMPRESSION_NONE;
	comments[0].key         = title;
	comments[0].text        = const_cast<char *>(get_canvas()->get_name().c_str());
	comments[0].text_length = strlen(comments[0].text);

	comments[1].compression = PNG_TEXT_COMPRESSION_NONE;
	comments[1].key         = description;
	comments[1].text        = const_cast<char *>(get_canvas()->get_description().c_str());
	comments[1].text_length = strlen(comments[1].text);

	comments[2].compression = PNG_TEXT_COMPRESSION_NONE;
	comments[2].key         = software;
	comments[2].text        = synfig;
	comments[2].text_length = strlen(comments[2].text);

	png_set_text(png_ptr, info_ptr, comments, sizeof(comments)/sizeof(png_text));

	png_write_info_before_PLTE(png_ptr, info_ptr);
	png_write_info(png_ptr, info_ptr);
	ready=true;
	return true;
}

Color *
png_trgt::start_scanline(int /*scanline*/)
{
	return color_buffer;
}

bool
png_trgt::end_scanline()
{
	if(!file || !ready)
		return false;

	PixelFormat pf = get_alpha_mode()==TARGET_ALPHA_MODE_KEEP ? PF_RGB|PF_A : PF_RGB;
	color_to_pixelformat(buffer, color_buffer, pf, 0, desc.get_w());

	setjmp(png_jmpbuf(png_ptr));
	png_write_row(png_ptr,buffer);

	return true;
}
