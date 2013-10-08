/* === S Y N F I G ========================================================= */
/*!	\file mptr_png.cpp
**	\brief ppm Target Module
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

/*!
**  \todo Support 16 bit PNG files
**	\todo Support GAMMA correction
**	\todo Fix memory leaks
*/

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mptr_png.h"
#include <synfig/importer.h>
#include <synfig/time.h>
#include <synfig/general.h>


#include <cstdio>
#include <algorithm>
#include <functional>
#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

#define PNG_CHECK_BYTES 	8

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(png_mptr);
SYNFIG_IMPORTER_SET_NAME(png_mptr,"png");
SYNFIG_IMPORTER_SET_EXT(png_mptr,"png");
SYNFIG_IMPORTER_SET_VERSION(png_mptr,"0.1");
SYNFIG_IMPORTER_SET_CVS_ID(png_mptr,"$Id$");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(png_mptr, true);

/* === M E T H O D S ======================================================= */

void
png_mptr::png_out_error(png_struct */*png_data*/,const char *msg)
{
	//png_mptr *me=(png_mptr*)png_data->error_ptr;
	synfig::error(strprintf("png_mptr: error: %s",msg));
	//me->ready=false;
}

void
png_mptr::png_out_warning(png_struct */*png_data*/,const char *msg)
{
	//png_mptr *me=(png_mptr*)png_data->error_ptr;
	synfig::warning(strprintf("png_mptr: warning: %s",msg));
	//me->ready=false;
}

void
png_mptr::read_callback(png_structp png_ptr, png_bytep out_bytes, png_size_t bytes_count_to_read)
{
	FileSystem::ReadStream *stream = (FileSystem::ReadStream*)png_get_io_ptr(png_ptr);
	png_size_t s = stream == NULL
				 ? 0
				 : stream->read_block(out_bytes, bytes_count_to_read);
	if (s < bytes_count_to_read)
		memset(out_bytes + s, 0, bytes_count_to_read - s);
}

png_mptr::png_mptr(const synfig::FileSystem::Identifier &identifier):
	Importer(identifier)
{
	/* Open the file pointer */
	FileSystem::ReadStreamHandle stream = identifier.get_read_stream();
    if (!stream)
    {
        //! \todo THROW SOMETHING
		throw strprintf("Unable to physically open %s",identifier.filename.c_str());
		return;
    }

	/* Make sure we are dealing with a PNG format file */
	png_byte header[PNG_CHECK_BYTES];
	if (!stream->read_variable(header))
	{
        //! \todo THROW SOMETHING
		throw strprintf("Cannot read header from \"%s\"",identifier.filename.c_str());
		return;
	}

    if (0 != png_sig_cmp(header, 0, PNG_CHECK_BYTES))
    {
        //! \todo THROW SOMETHING
		throw strprintf("This (\"%s\") doesn't appear to be a PNG file",identifier.filename.c_str());
		return;
    }

	png_structp png_ptr = png_create_read_struct
       (PNG_LIBPNG_VER_STRING, (png_voidp)this,
        &png_mptr::png_out_error, &png_mptr::png_out_warning);
	if (!png_ptr)
    {
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*3");
		return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr,
           (png_infopp)NULL, (png_infopp)NULL);
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*4");
		return;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr,
          (png_infopp)NULL);
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*4");
		return;
    }

    png_set_read_fn(png_ptr, stream.get(), read_callback);
	png_set_sig_bytes(png_ptr,PNG_CHECK_BYTES);

	png_read_info(png_ptr, info_ptr);

	int bit_depth,color_type,interlace_type, compression_type,filter_method;
	png_uint_32 width,height;

	png_get_IHDR(png_ptr, info_ptr, &width, &height,
				 &bit_depth, &color_type, &interlace_type,
				 &compression_type, &filter_method);

	if (bit_depth == 16)
		png_set_strip_16(png_ptr);

	if (bit_depth < 8)
		png_set_packing(png_ptr);

	double fgamma;
	if (png_get_gAMA(png_ptr, info_ptr, &fgamma))
	{
		synfig::info("PNG: Image gamma is %f",fgamma);
		png_set_gamma(png_ptr, gamma().get_gamma(), fgamma);
	}


	/*
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		synfig::error("Unable to setup longjump");
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(file);
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*5");
		return;
	}
	*/

	// man libpng tells me:
	//   You must use png_transforms and not call any
	//   png_set_transform() functions when you use png_read_png().
	// but we used png_set_gamma(), which may be why we were seeing a crash at the end
	//   png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_PACKING|PNG_TRANSFORM_STRIP_16, NULL);

	png_read_update_info(png_ptr, info_ptr);
	png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	// allocate buffer to read image data into
	png_bytep *row_pointers=new png_bytep[height];
	png_byte *data = new png_byte[rowbytes*height];
	for (png_uint_32 i = 0; i < height; i++)
		row_pointers[i] = &(data[rowbytes*i]);

	png_read_image(png_ptr, row_pointers);

	png_uint_32 x, y;
	surface_buffer.set_wh(width,height);

	switch(color_type)
	{
	case PNG_COLOR_TYPE_RGB:
		for(y=0;y<height;y++)
			for(x=0;x<width;x++)
			{
				float r=gamma().r_U8_to_F32((unsigned char)row_pointers[y][x*3+0]);
				float g=gamma().g_U8_to_F32((unsigned char)row_pointers[y][x*3+1]);
				float b=gamma().b_U8_to_F32((unsigned char)row_pointers[y][x*3+2]);
				surface_buffer[y][x]=Color(
					r,
					g,
					b,
					1.0
				);
/*
				surface_buffer[y][x]=Color(
					(float)(unsigned char)row_pointers[y][x*3+0]*(1.0/255.0),
					(float)(unsigned char)row_pointers[y][x*3+1]*(1.0/255.0),
					(float)(unsigned char)row_pointers[y][x*3+2]*(1.0/255.0),
					1.0
				);
*/
			}
		break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
		for(y=0;y<height;y++)
			for(x=0;x<width;x++)
			{
				float r=gamma().r_U8_to_F32((unsigned char)row_pointers[y][x*4+0]);
				float g=gamma().g_U8_to_F32((unsigned char)row_pointers[y][x*4+1]);
				float b=gamma().b_U8_to_F32((unsigned char)row_pointers[y][x*4+2]);
				surface_buffer[y][x]=Color(
					r,
					g,
					b,
					(float)(unsigned char)row_pointers[y][x*4+3]*(1.0/255.0)
				);
				/*
				surface_buffer[y][x]=Color(
					(float)(unsigned char)row_pointers[y][x*4+0]*(1.0/255.0),
					(float)(unsigned char)row_pointers[y][x*4+1]*(1.0/255.0),
					(float)(unsigned char)row_pointers[y][x*4+2]*(1.0/255.0),
					(float)(unsigned char)row_pointers[y][x*4+3]*(1.0/255.0)
				);
				*/
			}
		break;

	case PNG_COLOR_TYPE_GRAY:
		for(y=0;y<height;y++)
			for(x=0;x<width;x++)
			{
				float gray=gamma().g_U8_to_F32((unsigned char)row_pointers[y][x]);
				//float gray=(float)(unsigned char)row_pointers[y][x]*(1.0/255.0);
				surface_buffer[y][x]=Color(
					gray,
					gray,
					gray,
					1.0
				);
			}
		break;

	case PNG_COLOR_TYPE_GRAY_ALPHA:
		for(y=0;y<height;y++)
			for(x=0;x<width;x++)
			{
				float gray=gamma().g_U8_to_F32((unsigned char)row_pointers[y][x*2]);
//				float gray=(float)(unsigned char)row_pointers[y][x*2]*(1.0/255.0);
				surface_buffer[y][x]=Color(
					gray,
					gray,
					gray,
					(float)(unsigned char)row_pointers[y][x*2+1]*(1.0/255.0)
				);
			}
		break;

	case PNG_COLOR_TYPE_PALETTE:
		for(y=0;y<height;y++)
			for(x=0;x<width;x++)
			{
				png_colorp palette;
				int num_palette;
				png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
				float r=gamma().r_U8_to_F32((unsigned char)palette[row_pointers[y][x]].red);
				float g=gamma().g_U8_to_F32((unsigned char)palette[row_pointers[y][x]].green);
				float b=gamma().b_U8_to_F32((unsigned char)palette[row_pointers[y][x]].blue);
				float a=1.0;
				if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
				{
					png_bytep trans_alpha;
					int num_trans;
					png_color_16p trans_color;
					png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color);
					a = (float)(unsigned char)trans_alpha[row_pointers[y][x]]*(1.0/255.0);
				}
				surface_buffer[y][x]=Color(
					r,
					g,
					b,
					a
				);
			}
		break;
	default:
		png_read_end(png_ptr, end_info);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		synfig::error("png_mptr: error: Unsupported color type");
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*6");
		return;
	}

	png_read_end(png_ptr, end_info);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	stream.reset();

	delete [] row_pointers;
	delete [] data;

	trim = false;

	//if (getenv("SYNFIG_DISABLE_CROP_IMPORTED_IMAGES"))
		return;

	switch(color_type)
	{
	case PNG_COLOR_TYPE_RGB_ALPHA:
	case PNG_COLOR_TYPE_GRAY_ALPHA:
	case PNG_COLOR_TYPE_PALETTE:
		for(y=0;y<height;y++)
		{
			for(x=0;x<width;x++)
				if (surface_buffer[y][x].get_a()) break;
			if (x != width) break;
		}

		if (y != height)
		{
#define BORDER 1				// don't chop off all the transparent space - leave a border this many pixels wide for the interpolation

			png_uint_32 min_x, min_y, max_x, max_y;
			if (y>BORDER) min_y = y-BORDER; else min_y = 0;

			for(y=height-1;y>0;y--)
			{
				for(x=0;x<width;x++)
					if (surface_buffer[y][x].get_a()) break;
				if (x != width) break;
			}
			max_y = std::min(y+BORDER,height-1);

			for(x=0;x<width;x++)
			{
				for(y=0;y<height;y++)
					if (surface_buffer[y][x].get_a()) break;
				if (y != height) break;
			}
			if (x>BORDER) min_x = x-BORDER; else min_x = 0;

			for(x=width-1;x>0;x--)
			{
				for(y=0;y<height;y++)
					if (surface_buffer[y][x].get_a()) break;
				if (y != height) break;
			}
			max_x = std::min(x+BORDER,width-1);

			if (min_x != 0 || max_x != width-1 ||
				min_y != 0 || max_y != height-1)
			{
				trim = true;
				orig_width = width;
				orig_height = height;
				trimmed_x = min_x;
				trimmed_y = min_y;

				width=max_x-min_x+1;
				height=max_y-min_y+1;
				synfig::Surface tmp_buffer;
				tmp_buffer.set_wh(width,height);
				for(y=0;y<height;y++)
					for(x=0;x<width;x++)
						tmp_buffer[y][x] = surface_buffer[y+min_y][x+min_x];
				surface_buffer = tmp_buffer;
			}
		}
	}
}

png_mptr::~png_mptr()
{
}

bool
png_mptr::get_frame(synfig::Surface &surface, const synfig::RendDesc &/*renddesc*/, Time, synfig::ProgressCallback */*cb*/)
{
	//assert(0);					// shouldn't be called?
	surface=surface_buffer;
	return true;
}

bool
png_mptr::get_frame(synfig::Surface &surface, const synfig::RendDesc &/*renddesc*/, Time,
					bool &trimmed, unsigned int &width, unsigned int &height, unsigned int &top, unsigned int &left,
					synfig::ProgressCallback */*cb*/)
{
	surface=surface_buffer;
	if ((trimmed = trim))
	{
		width = orig_width;
		height = orig_height;
		top = trimmed_y;
		left = trimmed_x;
	}
	return true;
}
