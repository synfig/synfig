/*! ========================================================================
** Sinfg
** ppm Target Module
** $Id: mptr_png.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
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
#include <sinfg/importer.h>
#include <sinfg/time.h>
#include <sinfg/general.h>


#include <cstdio>
#include <algorithm>
#include <functional>
#endif

/* === M A C R O S ========================================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

#define PNG_CHECK_BYTES 	8

/* === G L O B A L S ======================================================= */

SINFG_IMPORTER_INIT(png_mptr);
SINFG_IMPORTER_SET_NAME(png_mptr,"png_mptr");
SINFG_IMPORTER_SET_EXT(png_mptr,"png");
SINFG_IMPORTER_SET_VERSION(png_mptr,"0.1");
SINFG_IMPORTER_SET_CVS_ID(png_mptr,"$Id: mptr_png.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $");

/* === M E T H O D S ======================================================= */

void
png_mptr::png_out_error(png_struct *png_data,const char *msg)
{
	//png_mptr *me=(png_mptr*)png_data->error_ptr;
	sinfg::error(strprintf("png_mptr: error: %s",msg));
	//me->ready=false;
}

void
png_mptr::png_out_warning(png_struct *png_data,const char *msg)
{
	//png_mptr *me=(png_mptr*)png_data->error_ptr;
	sinfg::warning(strprintf("png_mptr: warning: %s",msg));
	//me->ready=false;
}

int
png_mptr::read_chunk_callback(png_struct *png_data, png_unknown_chunkp chunk)
{
	/* The unknown chunk structure contains your
	  chunk data: */
	//png_byte name[5];
	//png_byte *data;
	//png_size_t size;
	/* Note that libpng has already taken care of
	  the CRC handling */
	
	/* put your code here.  Return one of the
	  following: */
	
	//return (-n); /* chunk had an error */
	return (0); /* did not recognize */
	//return (n); /* success */
}

png_mptr::png_mptr(const char *file_name)
{
	filename=file_name;
	
	/* Open the file pointer */
    FILE *file = fopen(file_name, "rb");
    if (!file)
    {
        //! \todo THROW SOMETHING
		throw strprintf("Unable to physically open %s",file_name);
		return;
    }
    
	
	/* Make sure we are dealing with a PNG format file */
	png_byte header[PNG_CHECK_BYTES];
	fread(header, 1, PNG_CHECK_BYTES, file);
    bool is_png = !png_sig_cmp(header, 0, PNG_CHECK_BYTES);
    if (!is_png)
    {
        //! \todo THROW SOMETHING
		throw strprintf("This (\"%s\") doesn't appear to be a PNG file",file_name);
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

	
	
	png_init_io(png_ptr, file);
	png_set_sig_bytes(png_ptr,PNG_CHECK_BYTES);

	double fgamma;
	if (png_get_gAMA(png_ptr, info_ptr, &fgamma))
	{
		sinfg::info("PNG: Image gamma is %f",fgamma);
		png_set_gamma(png_ptr, gamma().get_gamma(), fgamma);
	}
	
	
	/*
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		sinfg::error("Unable to setup longjump");
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(file);
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*5");
		return;
	}
	*/
	
	png_set_read_user_chunk_fn(png_ptr, this, &png_mptr::read_chunk_callback);
	
	
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_PACKING|PNG_TRANSFORM_STRIP_16, NULL);

	int bit_depth,color_type,interlace_type, compression_type,filter_method;
	png_uint_32 width,height;
	
    png_get_IHDR(png_ptr, info_ptr, &width, &height,
       &bit_depth, &color_type, &interlace_type,
       &compression_type, &filter_method);
	
	png_bytep *row_pointers=new png_bytep[height];
	row_pointers = png_get_rows(png_ptr, info_ptr);
	int x;
	int y;
	surface_buffer.set_wh(width,height);

	switch(color_type)
	{
	case PNG_COLOR_TYPE_RGB:
		DEBUGPOINT();
		for(y=0;y<surface_buffer.get_h();y++)
			for(x=0;x<surface_buffer.get_w();x++)
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
		DEBUGPOINT();
		for(y=0;y<surface_buffer.get_h();y++)
			for(x=0;x<surface_buffer.get_w();x++)
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
		for(y=0;y<surface_buffer.get_h();y++)
			for(x=0;x<surface_buffer.get_w();x++)
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
		for(y=0;y<surface_buffer.get_h();y++)
			for(x=0;x<surface_buffer.get_w();x++)
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
		sinfg::warning("png_mptr: Paletted PNGs aren't yet fully supported.");
		for(y=0;y<surface_buffer.get_h();y++)
			for(x=0;x<surface_buffer.get_w();x++)
			{
				float r=gamma().r_U8_to_F32((unsigned char)png_ptr->palette[row_pointers[y][x]].red);
				float g=gamma().g_U8_to_F32((unsigned char)png_ptr->palette[row_pointers[y][x]].green);
				float b=gamma().b_U8_to_F32((unsigned char)png_ptr->palette[row_pointers[y][x]].blue);
				surface_buffer[y][x]=Color(
					r,
					g,
					b,
					1.0
				);
			}
		break;
	default:
		sinfg::error("png_mptr: error: Unsupported color type");
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*6");
		return;
	}

	DEBUGPOINT();
	
	// \fixme These shouldn't be uncommented, but for some
	// reason, they crash the program. I will have to look into this
	// later. This is a memory leak, but it shouldn't be too bad.
	
	/*
	png_read_end(png_ptr, end_info);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	fclose(file);
	*/

	delete [] row_pointers;
}

png_mptr::~png_mptr()
{
	DEBUGPOINT();
}

bool
png_mptr::get_frame(sinfg::Surface &surface,Time, sinfg::ProgressCallback *cb)
{
	surface.mirror(surface_buffer);
//	surface=surface_buffer;
	return true;
}
