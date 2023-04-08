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

#include "trgt_jpeg.h"

#include <ETL/stringf>

#include <synfig/general.h>
#include <synfig/localization.h>

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
	quality(95),
	cinfo(),
	jerr(),
	multi_image(),
	ready(false),
	imagecount(),
	filename(Filename),
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

	if (filename == "-") {
		if (callback)
			callback->task(strprintf("(stdout) %d", imagecount));
		file = stdout;
	} else {
		String newfilename(filename);
		if (multi_image) {
			newfilename = (filename_sans_extension(filename) +
							   sequence_separator +
							   strprintf("%04d", imagecount) +
							   filename_extension(filename));
		}
		file = SmartFILE(newfilename, POPEN_BINARY_WRITE_TYPE);
		if (callback)
			callback->task(newfilename);
	}

	if (!file) {
		if (callback)
			callback->error(_("Unable to open file"));
		else
			synfig::error(_("Unable to open file"));
		return false;
	}

	buffer.resize(3*w);
	color_buffer.resize(w);


	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, file.get());

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

	file.reset();
	imagecount++;
}

Color *
jpeg_trgt::start_scanline(int /*scanline*/)
{
	return color_buffer.empty() ? nullptr : color_buffer.data();
}

bool
jpeg_trgt::end_scanline()
{
	if(!file || !ready)
		return false;

	color_to_pixelformat(buffer.data(), color_buffer.data(), PF_RGB, nullptr, desc.get_w());

	JSAMPROW row_pointer(buffer.data());
	jpeg_write_scanlines(&cinfo, &row_pointer, 1);

	return true;
}
