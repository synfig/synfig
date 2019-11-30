/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/packedsurface.h
**	\brief PackedSurface Header
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_SOFTWARE_PACKEDSURFACE_H
#define __SYNFIG_RENDERING_SOFTWARE_PACKEDSURFACE_H

/* === H E A D E R S ======================================================= */

#include <set>

#include <synfig/real.h>
#include <synfig/color.h>
#include <synfig/surface.h>

#include "../../primitive/contour.h"
#include "../../primitive/polyspan.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace software
{

class PackedSurface
{
public:
	enum ChannelType {
		ChannelUInt8,
		ChannelFloat32
	};

	enum {
		ChunkSize = 32,
		CacheRows = 2
	};

	class Reader
	{
	private:
		struct CacheEntry {
			int chunk_index;
			CacheEntry *prev;
			CacheEntry *next;
			void *data(int offset = 0) { return (char*)this + sizeof(*this) + offset; }
		};

		const PackedSurface *surface;
		mutable CacheEntry* first;
		mutable CacheEntry* last;
		mutable std::vector<CacheEntry*> chunks;
		char* cache;

	public:

		Reader();
		explicit Reader(const PackedSurface &surface);
		~Reader();

		void open(const PackedSurface &surface);
		void close();
		bool is_opened() const { return surface != NULL; }

		Color get_pixel(int x, int y) const;

		template< etl::clamping::func clamp_x = etl::clamping::clamp,
				  etl::clamping::func clamp_y = etl::clamping::clamp >
		inline static Color reader(const void *surf, int x, int y)
		{
			const Reader &r = *(const Reader*)surf;
			return clamp_x(x, r.surface->width) && clamp_y(y, r.surface->height)
			     ? r.get_pixel(x, y) : Color();
		}

		template< etl::clamping::func clamp_x = etl::clamping::clamp,
				  etl::clamping::func clamp_y = etl::clamping::clamp >
		inline static ColorAccumulator reader_cook(const void *surf, int x, int y)
		{
			const Reader &r = *(const Reader*)surf;
			return clamp_x(x, r.surface->width) && clamp_y(y, r.surface->height)
				 ? ColorPrep::cook_static(r.get_pixel(x, y)) : Color();
		}
	};

	struct DiscreteHelper {
		Color::value_type value;
		Color::value_type min;
		Color::value_type max;
		int count;
		DiscreteHelper(): value(), min(), max(), count() { }
		DiscreteHelper(Color::value_type c):
			value(c),
			min(c - real_low_precision<Color::value_type>()),
			max(c + real_low_precision<Color::value_type>()),
			count(1)
		{ }
		bool in_range(Color::value_type c) const
			{ return min <= c && c <= max; }
	};

	typedef etl::sampler<ColorAccumulator, float, ColorAccumulator, Reader::reader_cook> Sampler;

private:
	mutable std::mutex mutex_;
	mutable std::set<Reader*> readers;

	int width;
	int height;

	ChannelType channel_type;
	int channels[4];
	Color::value_type discrete_to_float[256];
	Color constant;

	int pixel_size;
	int row_size;

	int chunk_size;
	int chunk_row_size;
	int chunks_width;
	int chunks_height;

	std::vector<char> data;

	static Color::value_type get_channel(const void *pixel, int offset, ChannelType type, Color::value_type constant, const Color::value_type *discrete_to_float);
	static void set_channel(void *pixel, int offset, ChannelType type, Color::value_type color, const Color::value_type *discrete_to_float);

	Color get_pixel(const void *pixel) const;
	void set_pixel(void *pixel, const Color &color);

	void get_compressed_chunk(int index, const void *&data, int &size, bool &compressed) const;

public:
	PackedSurface();
	~PackedSurface();

	void clear();
	void set_pixels(const Color *pixels, int width, int height, int pitch = 0);
	int get_width() const { return width; }
	int get_height() const { return height; }
	void get_pixels(Color *target) const;
};

} /* end namespace software */
} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
