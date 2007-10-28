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

template <class Container>
MagickLib::Image* copy_image_list(Container& container)
{
	typedef typename Container::iterator Iter;
	MagickLib::Image* previous = 0;
	MagickLib::Image* first = NULL;
	MagickLib::ExceptionInfo exceptionInfo;
	MagickLib::GetExceptionInfo(&exceptionInfo);
	for (Iter iter = container.begin(); iter != container.end(); ++iter)
	{
		MagickLib::Image* current = CloneImage(iter->image(), 0, 0, Magick::MagickTrue, &exceptionInfo);
		if (!first) first = current;

		current->previous = previous;
		current->next	  = 0;

		if ( previous != 0) previous->next = current;
		previous = current;
	}

	return first;
}

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
		synfig::info("joining images");
		unsigned int delay = round_to_int(100.0 / desc.get_frame_rate());
		for_each(images.begin(), images.end(), Magick::animationDelayImage(delay));

		// optimize the images (only write the pixels that change from frame to frame
#ifdef HAVE_MAGICK_OPTIMIZE
		// make a completely new image list
		// this is required because:
		//   RemoveDuplicateLayers wants a linked list of images, and removes some of them
		//   when it removes an image, it invalidates it using DeleteImageFromList, but we still have it in our container
		//   when we destroy our container, the image is re-freed, failing an assertion

		synfig::info("copying image list");
		MagickLib::Image *image_list = copy_image_list(images);

		synfig::info("clearing old image list");
		images.clear();

		synfig::info("removing duplicate frames");
		RemoveDuplicateLayers(&image_list, &exceptionInfo);

		synfig::info("optimizing frames");
		OptimizeImageTransparency(image_list,&exceptionInfo);

		synfig::info("recreating image list");
		insertImages(&images, image_list);
#else
		synfig::info("not optimizing images");
		// DeconstructImages is available in ImageMagic 6.2.* but it doesn't take
		// the 'dispose' method into account, so for frames with transparency where
		// nothing is moving, we end up with objects disappearing when they shouldn't

		// linkImages(images.begin(), images.end());
		// MagickLib::Image* new_images = DeconstructImages(images.begin()->image(),&exceptionInfo);
		// unlinkImages(images.begin(), images.end());
		// images.clear();
		// insertImages(&images, new_images);
#endif
	}
	else
	{
		// if we can't write multiple images to a file of this type,
		// include '%04d' in the filename, so the files will be numbered
		// with a fixed width, '0'-padded number
		synfig::info("can't join images of this type - numbering instead");
		filename = (filename_sans_extension(filename) + ".%04d" + filename_extension(filename));
	}

	synfig::info("writing %d images to %s", images.size(), filename.c_str());
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
	if (transparent && images.begin() != images.end())
		(images.end()-1)->gifDisposeMethod(Magick::BackgroundDispose);
	images.push_back(image);
}

bool
magickpp_trgt::start_frame(synfig::ProgressCallback *callback)
{
	buffer_pointer = buffer;
	transparent = false;
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
	convert_color_format(buffer_pointer, color_buffer, width, PF_RGB|PF_A, gamma());

	if (!transparent)
		for (int i = 0; i < width; i++)
			if (buffer_pointer[i*4 + 3] < 128)
			{
				transparent = true;
				break;
			}

	buffer_pointer += 4 * width;

	return true;
}
