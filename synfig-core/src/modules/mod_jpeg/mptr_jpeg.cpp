/* === S Y N F I G ========================================================= */
/*!	\file mptr_jpeg.cpp
**	\brief ppm Target Module
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/*! \todo Support 16 bit JPEG files
**	\todo Support for paletted JPEG files
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

#include "mptr_jpeg.h"
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

#define JPEG_CHECK_BYTES 	8

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(jpeg_mptr);
SYNFIG_IMPORTER_SET_NAME(jpeg_mptr,"jpeg");
SYNFIG_IMPORTER_SET_EXT(jpeg_mptr,"jpg");
SYNFIG_IMPORTER_SET_VERSION(jpeg_mptr,"0.1");
SYNFIG_IMPORTER_SET_CVS_ID(jpeg_mptr,"$Id$");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(jpeg_mptr, false);

/* === M E T H O D S ======================================================= */

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

void
jpeg_mptr::my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}





jpeg_mptr::jpeg_mptr(const synfig::FileSystem::Identifier &identifier):
	Importer(identifier)
{
  	struct my_error_mgr jerr;

	/* Open the file pointer */
    FILE *file = fopen(identifier.filename.c_str(), "rb");
    if (!file)
    {
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*1");
		return;
    }

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_decompress(&cinfo);
		fclose(file);
		throw String("error on importer construction, *WRITEME*2");
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_stdio_src(&cinfo, file);

	/* Step 3: read file parameters with jpeg_read_header() */

	(void) jpeg_read_header(&cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	*   (a) suspension is not possible with the stdio data source, and
	*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	* See libjpeg.doc for more info.
	*/

	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	* jpeg_read_header(), so we do nothing here.
	*/

	/* Step 5: Start decompressor */

	(void) jpeg_start_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;
	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	if(!buffer)
	{
		synfig::error("jpeg_mptr: error: alloc of \"buffer\" failed (bug?)");
		throw String("alloc of \"buffer\" failed (bug?)");
	}

	int x;
	int y;
	surface_buffer.set_wh(cinfo.output_width,cinfo.output_height);

	switch(cinfo.output_components)
	{
	case 3:
		for(y=0;y<surface_buffer.get_h();y++)
		{
			int x;
			jpeg_read_scanlines(&cinfo, buffer, 1);
			for(x=0;x<surface_buffer.get_w();x++)
			{
				float r=gamma().g_U8_to_F32((unsigned char)buffer[0][x*3+0]);
				float g=gamma().g_U8_to_F32((unsigned char)buffer[0][x*3+1]);
				float b=gamma().g_U8_to_F32((unsigned char)buffer[0][x*3+2]);
				surface_buffer[y][x]=Color(
					r,
					g,
					b,
					1.0
				);
/*
				surface_buffer[y][x]=Color(
					(float)(unsigned char)buffer[0][x*3+0]*(1.0/255.0),
					(float)(unsigned char)buffer[0][x*3+1]*(1.0/255.0),
					(float)(unsigned char)buffer[0][x*3+2]*(1.0/255.0),
					1.0
				);
*/
				}
		}
		break;

	case 1:
		for(y=0;y<surface_buffer.get_h();y++)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);
			for(x=0;x<surface_buffer.get_w();x++)
			{
				float gray=gamma().g_U8_to_F32((unsigned char)buffer[0][x]);
//				float gray=(float)(unsigned char)buffer[0][x]*(1.0/255.0);
				surface_buffer[y][x]=Color(
					gray,
					gray,
					gray,
					1.0
				);
			}
		}
		break;

	default:
		synfig::error("jpeg_mptr: error: Unsupported color type");
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*6");
		return;
	}

	/* Step 7: Finish decompression */

	(void) jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	/* After finish_decompress, we can close the input file.
	* Here we postpone it until after no more JPEG errors are possible,
	* so as to simplify the setjmp error logic above.  (Actually, I don't
	* think that jpeg_destroy can do an error exit, but why assume anything...)
	*/
	fclose(file);
}

jpeg_mptr::~jpeg_mptr()
{
}

bool
jpeg_mptr::get_frame(synfig::Surface &surface, const synfig::RendDesc &/*renddesc*/, Time, synfig::ProgressCallback */*cb*/)
{
	surface=surface_buffer;
	return true;
}
