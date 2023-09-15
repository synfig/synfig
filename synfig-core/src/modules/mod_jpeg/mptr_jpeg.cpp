/* === S Y N F I G ========================================================= */
/*!	\file mptr_jpeg.cpp
**	\brief JPEG Importer Module
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2015 Jérôme Blanchi
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
#include <synfig/general.h>

#include <cstdio>
#endif

/* === M A C R O S ========================================================= */

using namespace synfig;

#define JPEG_CHECK_BYTES 	8

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(jpeg_mptr);
SYNFIG_IMPORTER_SET_NAME(jpeg_mptr,"jpeg");
SYNFIG_IMPORTER_SET_EXT(jpeg_mptr,"jpg");
SYNFIG_IMPORTER_SET_VERSION(jpeg_mptr,"0.1");
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
  //(*cinfo->err->output_message) (cinfo);

  char jpegLastErrorMsg[JMSG_LENGTH_MAX];
  ( *(cinfo->err->format_message) ) (cinfo, jpegLastErrorMsg);
  error(String("Jpeg error: ") + jpegLastErrorMsg);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}





jpeg_mptr::jpeg_mptr(const synfig::FileSystem::Identifier &identifier):
	Importer(identifier)
{
}

jpeg_mptr::~jpeg_mptr()
{
}

bool
jpeg_mptr::get_frame(synfig::Surface &surface, const synfig::RendDesc &/*renddesc*/, Time, synfig::ProgressCallback */*cb*/)
{
	jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;

	/* Open the file pointer */
	FileSystem::ReadStream::Handle stream = identifier.get_read_stream();
	if (!stream)
	{
		throw String("Error on jpeg importer, unable to physically open " + identifier.filename.u8string());
		return false;
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
		stream.reset();
		throw String("Error on jpeg importer, unable to connect to jpeg library");
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, from memory thru a String) */

	std::ostringstream tmp;
	tmp << stream->rdbuf();
	String streamString = tmp.str();
	stream.reset();

	jpeg_mem_src(&cinfo, (unsigned char*)streamString.c_str(), streamString.size());

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
		synfig::error("Error on jpeg importer, alloc of \"buffer\" failed (bug?)");
		throw String("Error on jpeg importer, alloc of \"buffer\" failed (bug?)");
	}

	surface.set_wh(cinfo.output_width, cinfo.output_height);
	const ColorReal k = 1/255.0;
	switch(cinfo.output_components)
	{
	case 3:
		for(int y = 0; y < surface.get_h(); ++y) {
			jpeg_read_scanlines(&cinfo, buffer, 1);
			for(int x = 0; x < surface.get_w(); ++x)
				surface[y][x]=Color(
					((unsigned char)buffer[0][x*3+0])*k,
					((unsigned char)buffer[0][x*3+1])*k,
					((unsigned char)buffer[0][x*3+2])*k );
		}
		break;
	case 1:
		for(int y = 0; y < surface.get_h(); ++y) {
			jpeg_read_scanlines(&cinfo, buffer, 1);
			for(int x = 0; x < surface.get_w(); ++x) {
				ColorReal gray = ((unsigned char)buffer[0][x])*k;
				surface[y][x]=Color(gray, gray, gray);
			}
		}
		break;
	default:
		synfig::error("Error on jpeg importer, Unsupported color type");
        //! \todo THROW SOMETHING
		throw String("Error on jpeg importer, Unsupported color type");
		return false;
	}

	/* Step 7: Finish decompression */

	(void) jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	return true;
}
