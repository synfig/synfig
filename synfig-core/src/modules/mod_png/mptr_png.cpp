/* === S Y N F I G ========================================================= */
/*!	\file mptr_png.cpp
**	\brief PNG Importer Module
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

/*!
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

#include <synfig/filecontainerzip.h>
#include <synfig/general.h>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;

#define PNG_CHECK_BYTES 	8

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(png_mptr);
SYNFIG_IMPORTER_SET_NAME(png_mptr,"png");
SYNFIG_IMPORTER_SET_EXT(png_mptr,"png");
SYNFIG_IMPORTER_SET_VERSION(png_mptr,"0.2");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(png_mptr, true);

/* === M E T H O D S ======================================================= */

namespace {
	inline ColorReal get_channel(png_bytep *rows, int bit_depth, int row, int col) {
		int x = bit_depth > 8
			  ? GUINT16_FROM_BE((png_uint_16p(rows[row]))[col])
			  : rows[row][col];
		int max = (1 << bit_depth) - 1;
		return x/ColorReal(max);
	}
}

void
png_mptr::png_out_error(png_struct */*png_data*/,const char *msg)
{
	//png_mptr *me=(png_mptr*)png_data->error_ptr;
	synfig::error(strprintf("png_mptr: error: %s",msg));
	//me->ready=false;
}

void
png_mptr::png_out_warning(png_struct */*png_data*/,const char *msg)
{
	//png_mptr *me=(png_mptr*)png_data->error_ptr;
	synfig::warning(strprintf("png_mptr: warning: %s",msg));
	//me->ready=false;
}

void
png_mptr::read_callback(png_structp png_ptr, png_bytep out_bytes, png_size_t bytes_count_to_read)
{
	FileSystem::ReadStream *stream = (FileSystem::ReadStream*)png_get_io_ptr(png_ptr);
	png_size_t s = !stream
				 ? 0
				 : stream->read_block(out_bytes, bytes_count_to_read);
	if (s < bytes_count_to_read)
		memset(out_bytes + s, 0, bytes_count_to_read - s);
}

png_mptr::png_mptr(const synfig::FileSystem::Identifier &identifier):
	Importer(identifier)
{
	std::string file_ext = identifier.filename.extension().u8string();
	if (file_ext == ".kra" || file_ext == ".ora") {
		zip_fs = new FileContainerZip();
		if (!zip_fs->open(identifier.filename.u8string())) {
			synfig::error("Can't find the file %s", identifier.filename.u8_str());
			return;
		}
		zipped_file = FileSystem::Identifier(zip_fs, "mergedimage.png");
	}
}

png_mptr::~png_mptr()
{
}

bool
png_mptr::get_frame(synfig::Surface &surface, const synfig::RendDesc &/*renddesc*/, Time, synfig::ProgressCallback */*cb*/)
{
	if (zip_fs && zipped_file.empty()) {
		//! \todo THROW SOMETHING
		throw strprintf("Unable to physically open %s: missing internal 'mergedimage.png'",identifier.filename.c_str());
		return false;
	}
	/* Open the file pointer */
	FileSystem::ReadStream::Handle stream = zip_fs? zipped_file.get_read_stream() : identifier.get_read_stream();
    if (!stream)
    {
        //! \todo THROW SOMETHING
		throw strprintf("Unable to physically open %s",identifier.filename.c_str());
		return false;
    }

	/* Make sure we are dealing with a PNG format file */
	png_byte header[PNG_CHECK_BYTES];
	if (!stream->read_variable(header))
	{
        //! \todo THROW SOMETHING
		throw strprintf("Cannot read header from \"%s\"",identifier.filename.c_str());
		return false;
	}

    if (0 != png_sig_cmp(header, 0, PNG_CHECK_BYTES))
    {
        //! \todo THROW SOMETHING
		throw strprintf("This (\"%s\") doesn't appear to be a PNG file",identifier.filename.c_str());
		return false;
    }

	png_structp png_ptr = png_create_read_struct
       (PNG_LIBPNG_VER_STRING, (png_voidp)this,
        &png_mptr::png_out_error, &png_mptr::png_out_warning);
	if (!png_ptr)
    {
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*3");
		return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*4");
		return false;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*4");
		return false;
    }

    png_set_read_fn(png_ptr, stream.get(), read_callback);
	png_set_sig_bytes(png_ptr,PNG_CHECK_BYTES);

	png_read_info(png_ptr, info_ptr);

	int bit_depth,color_type,interlace_type, compression_type,filter_method;
	png_uint_32 width,height;

	png_get_IHDR(png_ptr, info_ptr, &width, &height,
				 &bit_depth, &color_type, &interlace_type,
				 &compression_type, &filter_method);

	if (bit_depth > 16) {
		synfig::error("png_mptr: error: bit depth not supported: %d", bit_depth);
		throw strprintf("png_mptr: error: bit depth not supported: %d", bit_depth);
		return false;
	}

	if (bit_depth < 8)
		png_set_packing(png_ptr);

	double png_gamma;
	if (!png_get_gAMA(png_ptr, info_ptr, &png_gamma))
		png_gamma = 1/2.2;
	Gamma gamma(2.2*png_gamma);

	/*
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		synfig::error("Unable to setup longjump");
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(file);
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*5");
		return;
	}
	*/

	// man libpng tells me:
	//   You must use png_transforms and not call any
	//   png_set_transform() functions when you use png_read_png().
	// but we used png_set_gamma(), which may be why we were seeing a crash at the end
	//   png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_PACKING|PNG_TRANSFORM_STRIP_16, nullptr);

	png_read_update_info(png_ptr, info_ptr);
	png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	// allocate buffer to read image data into
	std::vector<png_bytep> row_pointers(height);
	std::vector<png_byte> data(rowbytes * height);
	for (png_uint_32 i = 0; i < height; i++)
		row_pointers[i] = &(data[rowbytes*i]);

	png_read_image(png_ptr, row_pointers.data());

	surface.set_wh(width, height);
	switch(color_type)
	{
	case PNG_COLOR_TYPE_RGB:
		for(int y = 0; y < surface.get_h(); ++y)
			for(int x = 0; x < surface.get_w(); ++x)
				surface[y][x]=gamma.apply(Color(
					get_channel(row_pointers.data(), bit_depth, y, x*3+0),
					get_channel(row_pointers.data(), bit_depth, y, x*3+1),
					get_channel(row_pointers.data(), bit_depth, y, x*3+2) ));
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		for(int y = 0; y < surface.get_h(); ++y)
			for(int x = 0; x < surface.get_w(); ++x)
				surface[y][x]=gamma.apply(Color(
					get_channel(row_pointers.data(), bit_depth, y, x*4+0),
					get_channel(row_pointers.data(), bit_depth, y, x*4+1),
					get_channel(row_pointers.data(), bit_depth, y, x*4+2),
					get_channel(row_pointers.data(), bit_depth, y, x*4+3) ));
		break;
	case PNG_COLOR_TYPE_GRAY:
		for(int y = 0; y < surface.get_h(); ++y)
			for(int x = 0; x < surface.get_w(); ++x)
			{
				ColorReal gray = get_channel(row_pointers.data(), bit_depth, y, x);
				surface[y][x] = gamma.apply(Color(gray, gray, gray));
			}
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		for(int y = 0; y < surface.get_h(); ++y)
			for(int x = 0; x < surface.get_w(); ++x)
			{
				ColorReal gray = get_channel(row_pointers.data(), bit_depth, y, x*2+0);
				ColorReal a    = get_channel(row_pointers.data(), bit_depth, y, x*2+1);
				surface[y][x] = gamma.apply(Color(gray, gray, gray, a));
			}
		break;

	case PNG_COLOR_TYPE_PALETTE:
	{
		if (bit_depth > 8) {
			synfig::error("png_mptr: error: bit depth with palette not supported: %d", bit_depth);
			throw strprintf("png_mptr: error: bit depth with palette not supported: %d", bit_depth);
			return false;
		}
		png_colorp palette;
		int num_palette;
		png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
		png_bytep trans_alpha = nullptr;
		int num_trans = 0;
		bool has_alpha = png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &num_trans, nullptr)
		               & PNG_INFO_tRNS;
		const ColorReal k = 1/255.0;
		for(int y = 0; y < surface.get_h(); ++y)
			for(int x = 0; x < surface.get_w(); ++x)
			{
				ColorReal r = k*(unsigned char)palette[row_pointers[y][x]].red;
				ColorReal g = k*(unsigned char)palette[row_pointers[y][x]].green;
				ColorReal b = k*(unsigned char)palette[row_pointers[y][x]].blue;
				ColorReal a = 1;
				if (has_alpha && num_trans > 0 && trans_alpha && row_pointers[y][x] < num_trans)
                    a = k*(unsigned char)trans_alpha[row_pointers[y][x]];
				surface[y][x] = gamma.apply(Color(r, g, b, a));
			}
		break;
	}
	default:
		png_read_end(png_ptr, end_info);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		synfig::error("png_mptr: error: Unsupported color type");
        //! \todo THROW SOMETHING
		throw String("error on importer construction, *WRITEME*6");
		return false;
	}

	png_read_end(png_ptr, end_info);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	//debug::DebugSurface::save_to_file(surface, "pngimport");
	
	return true;
}
