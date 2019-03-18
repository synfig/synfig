/* === S Y N F I G ========================================================= */
/*!	\file trgt_magickpp.cpp
**	\brief Magick++ Target Module
**
**	$Id$
**
**	\legal
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <synfig/general.h>

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
MagickCore::Image* copy_image_list(Container& container)
{
	typedef typename Container::iterator Iter;
	MagickCore::Image* previous = 0;
	MagickCore::Image* first = NULL;
	MagickCore::ExceptionInfo* exceptionInfo = MagickCore::AcquireExceptionInfo();
	for (Iter iter = container.begin(); iter != container.end(); ++iter)
	{
		MagickCore::Image* current;

		try
		{
			current = CloneImage(iter->image(), 0, 0, Magick::MagickTrue, exceptionInfo);

			if (!first) first = current;

			current->previous = previous;
			current->next	  = 0;

			if ( previous != 0) previous->next = current;
			previous = current;
		}
		catch(Magick::Warning &warning) {
			synfig::warning("exception '%s'", warning.what());
		}
	}

	exceptionInfo = MagickCore::DestroyExceptionInfo(exceptionInfo);
	return first;
}

magickpp_trgt::~magickpp_trgt()
{
	MagickCore::ExceptionInfo* exceptionInfo = MagickCore::AcquireExceptionInfo();

	try
	{
		bool multiple_images = images.size() > 1;
		bool can_adjoin = false;

		if (multiple_images)
		{
			// check whether this file format supports multiple-image files
			Magick::Image image(*(images.begin()));
			image.fileName(filename);
			try
			{
				SetImageInfo(image.imageInfo(),Magick::MagickTrue,exceptionInfo);
				can_adjoin = image.adjoin();
			}
			catch(Magick::Warning &warning) {
				synfig::warning("exception '%s'", warning.what());
			}
		}

		// the file type is now in image.imageInfo()->magick and
		// image.adjoin() tells us whether we can write to a single file
		if (can_adjoin)
		{
			synfig::info("joining images");
			unsigned int delay = round_to_int(100.0 / desc.get_frame_rate());
			for_each(images.begin(), images.end(), Magick::animationDelayImage(delay));

			// optimize the images (only write the pixels that change from frame to frame
			// make a completely new image list
			// this is required because:
			//   RemoveDuplicateLayers wants a linked list of images, and removes some of them
			//   when it removes an image, it invalidates it using DeleteImageFromList, but we still have it in our container
			//   when we destroy our container, the image is re-freed, failing an assertion

			synfig::info("copying image list");
			MagickCore::Image *image_list = copy_image_list(images);

			synfig::info("clearing old image list");
			images.clear();

			if (!getenv("SYNFIG_DISABLE_REMOVE_DUPS"))
			{
				synfig::info("removing duplicate frames");
				try
				{
					RemoveDuplicateLayers(&image_list, exceptionInfo);
				}
				catch(Magick::Warning &warning) {
					synfig::warning("exception '%s'", warning.what());
				}
			}

			if (!getenv("SYNFIG_DISABLE_OPTIMIZE"))
			{
				synfig::info("optimizing layers");
				try
				{
					image_list = OptimizeImageLayers(image_list,exceptionInfo);
				}
				catch(Magick::Warning &warning) {
					synfig::warning("exception '%s'", warning.what());
				}
			}

			if (!getenv("SYNFIG_DISABLE_OPTIMIZE_TRANS"))
			{
				synfig::info("optimizing layer transparency");
				try
				{
					OptimizeImageTransparency(image_list,exceptionInfo);
				}
				catch(Magick::Warning &warning) {
					synfig::warning("exception '%s'", warning.what());
				}
			}

			synfig::info("recreating image list");
			insertImages(&images, image_list);
		}
		else if (multiple_images)
		{
			// if we can't write multiple images to a file of this type,
			// include '%04d' in the filename, so the files will be numbered
			// with a fixed width, '0'-padded number
			synfig::info("can't join images of this type - numbering instead");
			filename = (filename_sans_extension(filename) + sequence_separator + "%04d" + filename_extension(filename));
		}

		synfig::info("writing %d image%s to %s", images.size(), images.size() == 1 ? "" : "s", filename.c_str());
		try
		{
			Magick::writeImages(images.begin(), images.end(), filename);
			synfig::info("done");
		}
		catch(Magick::Warning &warning) {
			synfig::warning("exception '%s'", warning.what());
		}
	}
	catch(Magick::Warning &warning) {
		synfig::warning("exception '%s'", warning.what());
	}
	catch(Magick::Error &error) {
		synfig::error("exception '%s'", error.what());
	}
	catch(...) {
		synfig::error("unknown exception");
	}

	if (buffer1 != NULL) delete [] buffer1;
	if (buffer2 != NULL) delete [] buffer2;
	if (color_buffer != NULL) delete [] color_buffer;
	//exceptionInfo = MagickCore::DestroyExceptionInfo(exceptionInfo);
	MagickCore::DestroyExceptionInfo(exceptionInfo);
}

bool
magickpp_trgt::set_rend_desc(RendDesc *given_desc)
{
	desc = *given_desc;
	return true;
}

bool
magickpp_trgt::init(synfig::ProgressCallback*)
{
	width = desc.get_w();
	height = desc.get_h();

	start_pointer = NULL;

	buffer1 = new unsigned char[4*width*height];
	if (buffer1 == NULL)
		return false;

	buffer2 = new unsigned char[4*width*height];
	if (buffer2 == NULL)
	{
		delete [] buffer1;
		return false;
	}

	color_buffer = new Color[width];
	if (color_buffer == NULL)
	{
		delete [] buffer1;
		delete [] buffer2;
		return false;
	}

	return true;
}

void
magickpp_trgt::end_frame()
{
	Magick::Image image(width, height, "RGBA", Magick::CharPixel, start_pointer);
	if (transparent && images.begin() != images.end())
		(images.end()-1)->gifDisposeMethod(Magick::BackgroundDispose);
	images.push_back(image);
}

bool
magickpp_trgt::start_frame(synfig::ProgressCallback *callback __attribute__ ((unused)))
{
	if (start_pointer == buffer1)
		start_pointer = buffer_pointer = buffer2;
	else
		start_pointer = buffer_pointer = buffer1;

	previous_buffer_pointer = start_pointer;

	transparent = false;
	return true;
}

Color*
magickpp_trgt::start_scanline(int scanline __attribute__ ((unused)))
{
	return color_buffer;
}

bool
magickpp_trgt::end_scanline()
{
	if (previous_buffer_pointer)
		color_to_pixelformat(previous_buffer_pointer, color_buffer, PF_RGB|PF_A, &gamma(), width);

	if (!transparent)
		for (int i = 0; i < width; i++)
			if (previous_buffer_pointer &&					// this isn't the first frame
				buffer_pointer[i*4 + 3] < 128 &&			// our pixel is transparent
				!(previous_buffer_pointer[i*4 + 3] < 128))	// the previous frame's pixel wasn't
			{
				transparent = true;
				break;
			}

	buffer_pointer += 4 * width;

	if (previous_buffer_pointer)
		previous_buffer_pointer += 4 * width;

	return true;
}
