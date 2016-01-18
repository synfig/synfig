/* === S Y N F I G ========================================================= */
/*!	\file debugsurface.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <vector>

#include "debugsurface.h"

#include <synfig/filesystemnative.h>
#include <synfig/filecontainertemporary.h>
#include <synfig/rendering/surface.h>
#include <synfig/surface.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace debug;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
DebugSurface::save_to_file(const void *buffer, int width, int height, int pitch, const String &filename, bool overwrite)
{
	// generate filename
	String actual_filename =
		overwrite
		? filename
		: FileContainerTemporary::generate_indexed_temporary_filename(
			FileSystemNative::instance(),
			filename + ".tga" );

	if (buffer == NULL || width <= 0 || height <= 0)
	{
		// save empty file for empty surface
		FileSystemNative::instance()
			->get_write_stream(actual_filename);
	}
	else
	{
		// convert to 32BPP
		if (pitch == 0)
			pitch = width * sizeof(Color);

		PixelFormat pf(PF_BGR|PF_A);
		size_t total_bytes = width * height * synfig::channels(pf);
		unsigned char *byte_buffer = (unsigned char*)malloc(total_bytes);
		if (!byte_buffer) return;

		// write rows in reverse order (for TGA format)
		unsigned char *dest(byte_buffer);
		Gamma gamma;
		for(int y = height-1; y >= 0; --y)
		{
			const Color *src = (const Color *)((const unsigned char*)buffer + pitch*y);
			for(int x = 0; x < width; ++x, ++src)
				dest = Color2PixelFormat( src->clamped(), pf, dest, gamma );
		}

		// create file
		FileSystem::WriteStreamHandle ws =
			FileSystemNative::instance()
				->get_write_stream(actual_filename);

		// write header
		unsigned char targa_header[] = {
			0,    // Length of the image ID field (0 - no ID field)
			0,    // Whether a color map is included (0 - no colormap)
			2,    // Compression and color types (2 - uncompressed true-color image)
			0, 0, 0, 0, 0, // Color map specification (not need for us)
		    0, 0, // X-origin
		    0, 0, // Y-origin
			(unsigned char)(width & 0xff), // Image width
		    (unsigned char)(width >> 8),
			(unsigned char)(height & 0xff), // Image height
		    (unsigned char)(height >> 8),
		    32,   // Bits per pixel
		    0     // Image descriptor (keep zero for capability)
		};
		ws->write_whole_block(targa_header, sizeof(targa_header));

		// write data
		ws->write_whole_block(byte_buffer, total_bytes);

		free(byte_buffer);
	}
}

void
DebugSurface::save_to_file(const Surface &surface, const String &filename, bool overwrite)
{
	if (surface.is_valid())
		save_to_file(&surface[0][0], surface.get_w(), surface.get_h(), surface.get_pitch(), filename, overwrite);
	else
		save_to_file(NULL, 0, 0, 0, filename, overwrite);
}

void
DebugSurface::save_to_file(const rendering::Surface &surface, const String &filename, bool overwrite)
{
	if (surface.is_created())
	{
		std::vector<Color> buffer(surface.get_pixels_count());
		surface.get_pixels(&buffer.front());
		save_to_file(&buffer.front(), surface.get_width(), surface.get_height(), 0, filename, overwrite);
	}
	else
		save_to_file(NULL, 0, 0, 0, filename, overwrite);
}

void
DebugSurface::save_to_file(const rendering::Surface::Handle &surface, const String &filename, bool overwrite)
{
	if (surface)
		save_to_file(*surface, filename, overwrite);
	else
		save_to_file(NULL, 0, 0, 0, filename, overwrite);
}
