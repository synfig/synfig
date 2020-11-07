/* === S Y N F I G ========================================================= */
/*!	\file trgt_jpeg.cpp
**	\brief jpeg_trgt Target Module
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <glib/gstdio.h>
#include "trgt_jpeg.h"
#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(jpeg_trgt);
SYNFIG_TARGET_SET_NAME(jpeg_trgt,"jpeg");
SYNFIG_TARGET_SET_EXT(jpeg_trgt,"jpg");
SYNFIG_TARGET_SET_VERSION(jpeg_trgt,"0.1");

/* === M E T H O D S ======================================================= */

jpeg_trgt::jpeg_trgt(const char *Filename, const synfig::TargetParam &params):
	file(nullptr),
	quality(95),
	cinfo(),
	jerr(),
	multi_image(),
	ready(false),
	imagecount(),
	filename(Filename),
	buffer(nullptr),
	color_buffer(nullptr),
	sequence_separator(params.sequence_separator)
{
	set_alpha_mode(TARGET_ALPHA_MODE_FILL);
}

jpeg_trgt::~jpeg_trgt()
{
	if(ready)
	{
		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		ready=false;
	}
	if(file)
		fclose(file);
	file=nullptr;
	delete [] buffer;
	delete [] color_buffer;
}

bool
jpeg_trgt::set_rend_desc(RendDesc *given_desc)
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
jpeg_trgt::start_frame(synfig::ProgressCallback *callback)
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
	buffer=new unsigned char[3*w];

	delete [] color_buffer;
	color_buffer=new Color[w];


	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, file);

	cinfo.image_width = w; 	/* image width and height, in pixels */
	cinfo.image_height = h;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	/* Now use the library's routine to set default compression parameters.
	* (You must set at least cinfo.in_color_space before calling this,
	* since the defaults depend on the source color space.)
	*/
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	* Here we just illustrate the use of quality (quantization table) scaling:
	*/
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */

	/* TRUE ensures that we will write a complete interchange-JPEG file.
	* Pass TRUE unless you are very sure of what you're doing.
	*/
	jpeg_start_compress(&cinfo, TRUE);

	ready=true;
	return true;
}

void
jpeg_trgt::end_frame()
{
	if(ready)
	{
		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		ready=false;
	}

	if(file && file!=stdout)
		fclose(file);
	file=nullptr;
	imagecount++;
}

Color *
jpeg_trgt::start_scanline(int /*scanline*/)
{
	return color_buffer;
}

bool
jpeg_trgt::end_scanline()
{
	if(!file || !ready)
		return false;

	color_to_pixelformat(buffer, color_buffer, PF_RGB, nullptr, desc.get_w());

	JSAMPROW *row_pointer(&buffer);
	jpeg_write_scanlines(&cinfo, row_pointer, 1);

	return true;
}
