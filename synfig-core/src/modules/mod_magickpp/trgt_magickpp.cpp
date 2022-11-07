/* === S Y N F I G ========================================================= */
/*!	\file trgt_magickpp.cpp
**	\brief Magick++ Target Module
**
**	\legal
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <synfig/general.h>
#include <synfig/misc.h>

#include <ETL/stringf>
#include "trgt_magickpp.h"

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(magickpp_trgt);
SYNFIG_TARGET_SET_NAME(magickpp_trgt,"magick++");
SYNFIG_TARGET_SET_EXT(magickpp_trgt,"gif");
SYNFIG_TARGET_SET_VERSION(magickpp_trgt,"0.1");

/* === M E T H O D S ======================================================= */

template <class Container>
MagickCore::Image* copy_image_list(Container& container)
{
	typedef typename Container::iterator Iter;
	MagickCore::Image* previous = 0;
	MagickCore::Image* first = nullptr;
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
			Magick::Image image(images.front());
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

	buffer_pointer = nullptr;

	std::string extension = filename_extension(filename);
	std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); });
	is_gif = extension == ".gif";

	std::size_t buffer_size = static_cast<std::size_t>(4) * width * height;
	buffer1.resize(buffer_size);
	if (is_gif)
		buffer2.resize(buffer_size);

	color_buffer.resize(width);

	return true;
}

void
magickpp_trgt::end_frame()
{
	Magick::Image image(width, height, "RGBA", Magick::CharPixel, buffer_pointer);
	if (is_gif && transparent && images.size() > 1)
		images.back().gifDisposeMethod(Magick::BackgroundDispose);
	images.push_back(image);
}

bool
magickpp_trgt::start_frame(synfig::ProgressCallback */*callback*/)
{
	if (is_gif)
		previous_row_buffer_pointer = buffer_pointer;

	if (is_gif && buffer_pointer == buffer1.data())
		buffer_pointer = current_row_buffer_pointer = buffer2.data();
	else
		buffer_pointer = current_row_buffer_pointer = buffer1.data();

	transparent = false;

	return true;
}

Color*
magickpp_trgt::start_scanline(int /*scanline*/)
{
	return color_buffer.data();
}

bool
magickpp_trgt::end_scanline()
{
	color_to_pixelformat(current_row_buffer_pointer, color_buffer.data(), PF_RGB|PF_A, 0, width);

	if (!transparent && previous_row_buffer_pointer) {
		for (int i = 0; i < width; i++) {
			if (current_row_buffer_pointer[i*4 + 3] < 128 &&	// our pixel is transparent
				!(previous_row_buffer_pointer[i*4 + 3] < 128))	// the previous frame's pixel wasn't
			{
				transparent = true;
				break;
			}
		}
	}

	current_row_buffer_pointer += 4 * width;

	if (previous_row_buffer_pointer)
		previous_row_buffer_pointer += 4 * width;

	return true;
}
