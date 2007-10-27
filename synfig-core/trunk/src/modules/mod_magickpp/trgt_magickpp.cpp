/* === S Y N F I G ========================================================= */
/*!	\file trgt_magickpp.cpp
**	\brief Magick++ Target Module
**
**	$Id$
**
**	\legal
**	Copyright 2007 Chris Moore
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

#define SYNFIG_TARGET

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <ETL/misc>
#include "trgt_magickpp.h"

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(magickpp_trgt);
SYNFIG_TARGET_SET_NAME(magickpp_trgt,"magick++");
SYNFIG_TARGET_SET_EXT(magickpp_trgt,"gif");
SYNFIG_TARGET_SET_VERSION(magickpp_trgt,"0.1");
SYNFIG_TARGET_SET_CVS_ID(magickpp_trgt,"$Id$");

/* === M E T H O D S ======================================================= */

magickpp_trgt::~magickpp_trgt()
{
	MagickLib::ExceptionInfo exceptionInfo;
	MagickLib::GetExceptionInfo(&exceptionInfo);

	// check whether this file format supports multiple-image files
	Magick::Image image(*(images.begin()));
	image.fileName(filename);
	SetImageInfo(image.imageInfo(),Magick::MagickTrue,&exceptionInfo);

	// the file type is now in image.imageInfo()->magick and
	// image.adjoin() tells us whether we can write to a single file
	if (image.adjoin())
	{
		unsigned int delay = round_to_int(100.0 / desc.get_frame_rate());
		for_each(images.begin(), images.end(), Magick::animationDelayImage(delay));

		// if we can write multiple images to a single file, optimize
		// the images (only write the pixels that change from frame to
		// frame
#ifdef HAVE_MAGICK_OPTIMIZE
		linkImages(images.begin(), images.end());
		OptimizeImageTransparency(images.begin()->image(),&exceptionInfo);
		unlinkImages(images.begin(), images.end());
#else
		// linkImages(images.begin(), images.end());
		// MagickLib::Image* new_images = DeconstructImages(images.begin()->image(),&exceptionInfo);
		// unlinkImages(images.begin(), images.end());
		// images.clear();
		// insertImages(&images, new_images);
#endif
	}
	else
		// if we can't write multiple images to a file of this type,
		// include '%04d' in the filename, so the files will be numbered
		// with a fixed width, '0'-padded number
		filename = (filename_sans_extension(filename) + ".%04d" + filename_extension(filename));

	Magick::writeImages(images.begin(), images.end(), filename);

	if (buffer != NULL) delete [] buffer;
	if (color_buffer != NULL) delete [] color_buffer;
}

bool
magickpp_trgt::set_rend_desc(RendDesc *given_desc)
{
	desc = *given_desc;
	return true;
}

bool
magickpp_trgt::init()
{
	width = desc.get_w();
	height = desc.get_h();

	buffer = new unsigned char[4*width*height];
	if (buffer == NULL)
		return false;

	color_buffer = new Color[width];
	if (color_buffer == NULL)
	{
		delete [] buffer;
		return false;
	}

	return true;
}

void
magickpp_trgt::end_frame()
{
	Magick::Image image(width, height, "RGBA", Magick::CharPixel, buffer);
	images.push_back(image);
}

bool
magickpp_trgt::start_frame(synfig::ProgressCallback *callback)
{
	row = 0;
	return true;
}

Color*
magickpp_trgt::start_scanline(int scanline)
{
	return color_buffer;
}

bool
magickpp_trgt::end_scanline()
{
	convert_color_format(buffer + row++ * width*4, color_buffer, width, PF_RGB|PF_A, gamma());
	return true;
}
