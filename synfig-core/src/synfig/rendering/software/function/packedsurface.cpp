/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/packedsurface.cpp
**	\brief PackedSurface
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <cstdlib>
#include <cstring>

#include <vector>
#include <map>

#include "packedsurface.h"

#include <synfig/real.h>
#include <synfig/zstreambuf.h>

#endif

using namespace synfig;
using namespace rendering;
using namespace software;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


PackedSurface::Reader::Reader():
	surface(NULL),
	first(NULL),
	last(NULL),
	cache(NULL)
{ }

PackedSurface::Reader::Reader(const PackedSurface &surface):
	surface(NULL),
	first(NULL),
	last(NULL),
	cache(NULL)
{
	open(surface);
}

PackedSurface::Reader::~Reader()
{
	close();
}

void
PackedSurface::Reader::open(const PackedSurface &surface)
{
	if (this->surface == &surface)
		return;

	close();

	if (surface.width <= 0 && surface.height <= 0)
		return;

	{
		this->surface = &surface;
		std::lock_guard<std::mutex> lock(surface.mutex);
		surface.readers.insert(this);
	}

	if (surface.chunk_size)
	{
		chunks.resize(surface.chunks_width*surface.chunks_height, NULL);

		int cacheCount = std::max(surface.chunks_width, surface.chunks_height)*CacheRows;
		assert(cacheCount > 1);
		int cacheEntrySize = sizeof(CacheEntry) + surface.chunk_size;
		cache = new char[cacheCount*cacheEntrySize];
		first = (CacheEntry*)cache;
		for(int i = 0; i < cacheCount; ++i)
		{
			CacheEntry *entry = (CacheEntry*)(void*)(cache + i*cacheEntrySize);
			entry->chunk_index = -1;
			entry->next = NULL;
			entry->prev = last;
			if (last) last->next = entry;
			last = entry;
		}
	}
}

void
PackedSurface::Reader::close()
{
	if (is_opened())
	{
		{
			std::lock_guard<std::mutex> lock(surface->mutex);
			surface->readers.erase(this);
		}
		if (cache) delete[] cache;
		first = NULL;
		last = NULL;
		cache = NULL;
		surface = NULL;
	}
}

Color
PackedSurface::Reader::get_pixel(int x, int y) const
{
	if (!is_opened())
		return Color();

	if (x < 0)
		x = 0;
	if (x >= surface->width)
		x = surface->width-1;
	if (y < 0)
		y = 0;
	if (y >= surface->height)
		y = surface->height-1;

	if (cache)
	{
		int chunk_index = x/ChunkSize + y/ChunkSize*surface->chunks_width;
		x %= ChunkSize;
		y %= ChunkSize;
		CacheEntry *entry = chunks[chunk_index];
		if (!entry)
		{
			const void *data;
			int size;
			bool compressed;
			surface->get_compressed_chunk(chunk_index, data, size, compressed);
			if (!compressed)
				return surface->get_pixel(&((const char*)data)[x*surface->pixel_size + y*surface->chunk_row_size]);

			entry = last;
			if (entry->chunk_index >= 0)
				chunks[entry->chunk_index] = NULL;
			entry->chunk_index = chunk_index;
			chunks[chunk_index] = entry;
			zstreambuf::unpack(entry->data(), surface->chunk_size, data, size);
		}
		if (first != entry)
		{
			entry->prev->next = entry->next;
			(entry->next ? entry->next->prev : last) = entry->prev;

			first->prev = entry;
			entry->prev = NULL;
			entry->next = first;
			first = entry;
		}
		return surface->get_pixel(entry->data(x*surface->pixel_size + y*surface->chunk_row_size));
	}
	else
	if (surface->pixel_size)
	{
		return surface->get_pixel(&surface->data[x*surface->pixel_size + y*surface->row_size]);
	}
	return surface->constant;
}


PackedSurface::PackedSurface():
	width(0),
	height(0),
	channel_type(),
	pixel_size(0),
	row_size(0),
	chunk_size(0),
	chunk_row_size(0),
	chunks_width(0),
	chunks_height(0)
{
	memset(channels, 0, sizeof(channels));
	memset(discrete_to_float, 0, sizeof(discrete_to_float));
}

PackedSurface::~PackedSurface()
{
	clear();
}


void
PackedSurface::clear() {
	while(!readers.empty())
		(*readers.begin())->close();
	width = 0;
	height = 0;
	channel_type = ChannelUInt8;
	memset(channels, 0, sizeof(channels));
	memset(discrete_to_float, 0, sizeof(discrete_to_float));
	constant = Color();
	pixel_size = 0;
	row_size = 0;
	chunk_size = 0;
	chunk_row_size = 0;
	chunks_width = 0;
	chunks_height = 0;
	data.clear();
}

Color::value_type
PackedSurface::get_channel(const void *pixel, int offset, ChannelType type, Color::value_type constant, const Color::value_type *discrete_to_float)
{
	if (offset < 0)
		return constant;
	if (type == ChannelUInt8)
		return discrete_to_float[((const unsigned char*)pixel)[offset]];
	return *(const Color::value_type*)((const char*)pixel + offset);
}

void
PackedSurface::set_channel(void *pixel, int offset, ChannelType type, Color::value_type color, const Color::value_type *discrete_to_float)
{
	if (offset < 0)
		return;
	if (type == ChannelUInt8) {
		int i = 0;
		int j = 255;
		while(true)
		{
			int k = (i+j)/2;
			if (k == i) break;
			if (color < discrete_to_float[k]) j = k; else i = k;
		}
		int best_key = fabs(discrete_to_float[i] - color) < fabs(discrete_to_float[j] - color) ? i : j;
		((unsigned char*)pixel)[offset] = (unsigned char)best_key;
		return;
	}
	*(Color::value_type*)((char*)pixel + offset) = color;
}

Color
PackedSurface::get_pixel(const void *pixel) const
{
	return Color(
		get_channel(pixel, channels[0], channel_type, constant.get_r(), discrete_to_float),
		get_channel(pixel, channels[1], channel_type, constant.get_g(), discrete_to_float),
		get_channel(pixel, channels[2], channel_type, constant.get_b(), discrete_to_float),
		get_channel(pixel, channels[3], channel_type, constant.get_a(), discrete_to_float) );
}

void
PackedSurface::set_pixel(void *pixel, const Color &color)
{
	set_channel(pixel, channels[0], channel_type, color.get_r(), discrete_to_float);
	set_channel(pixel, channels[1], channel_type, color.get_g(), discrete_to_float);
	set_channel(pixel, channels[2], channel_type, color.get_b(), discrete_to_float);
	set_channel(pixel, channels[3], channel_type, color.get_a(), discrete_to_float);
}

void
PackedSurface::get_compressed_chunk(int index, const void *&data, int &size, bool &compressed) const
{
	assert(chunk_size);
	const int *chunks = (const int*)(const void*)&this->data.front();
	int begin = chunks[index];
	int end = chunks[index+1];
	data = &this->data[begin];
	size = end - begin;
	compressed = size != chunk_size;
}

void
PackedSurface::set_pixels(const Color *pixels, int width, int height, int pitch) {
	clear();
	if (pixels == NULL || width <= 0 || height <= 0)
		return;

	if (pitch == 0) pitch = sizeof(Color)*width;

	// check format
	Color constant = *pixels;
	Color::value_type *constant_channels = (Color::value_type*)(void*)&constant;
	bool discrete = true;
	std::vector<DiscreteHelper> discrete_values;
	bool channels_equality[4][4];
	bool constant_equality[4];
	for(int i = 0; i < 4; ++i)
	{
		for(int j = 0; j < 4; ++j)
			channels_equality[i][j] = true;
		constant_equality[i] = true;
	}

	for(int row = 0; row < height; ++row) {
		for(const Color *color = (const Color*)((const char*)pixels + row*pitch), *end = color + width; color < end; ++color)
		{
			const Color::value_type *color_channels = (const Color::value_type*)(const void*)color;
			for(int i = 0; i < 4; ++i)
			{
				// compare channels
				for(int j = 0; j < i; ++j)
					if (channels_equality[i][j] && !approximate_equal_lp(color_channels[i], color_channels[j]))
						channels_equality[i][j] = false;

				// compare with constant
				if (constant_equality[i] && !approximate_equal_lp(color_channels[i], constant_channels[i]))
					constant_equality[i] = false;

				// check discrete
				if (discrete)
				{
					Color::value_type c = color_channels[i];
					if (discrete_values.empty()) {
						discrete_values.push_back(DiscreteHelper(c));
					} else {
						int i = 0;
						int j = discrete_values.size() - 1;
						while(true) {
							int k = (i+j)/2;
							if (k == i) break;
							if (c < discrete_values[k].min) j = k; else i = k;
						}

						if (discrete_values[i].in_range(c))
							++discrete_values[i].count;
						else
						if (discrete_values[j].in_range(c))
							++discrete_values[j].count;
						else
						{
							if (c < discrete_values[j].value)
								discrete_values.insert(discrete_values.begin() + j, DiscreteHelper(c));
							else
								discrete_values.push_back(DiscreteHelper(c));
							if (discrete_values.size() > 260) discrete = false;
						}
					}
				}
			}
		}
	}

	this->channel_type = discrete ? ChannelUInt8 : ChannelFloat32;
	int channel_size = this->channel_type == ChannelUInt8 ? sizeof(unsigned char) : sizeof(ColorReal);

	if (discrete) {
		while(discrete_values.size() > 256) {
			std::vector<DiscreteHelper>::iterator min_i = discrete_values.begin();
			for(std::vector<DiscreteHelper>::iterator i = discrete_values.begin(); i != discrete_values.end(); ++i)
				if (min_i->count > i->count)
					min_i = i;
			discrete_values.erase(min_i);
		}
		int index = 0;
		for(std::vector<DiscreteHelper>::const_iterator i = discrete_values.begin(); i != discrete_values.end(); ++i, ++index)
			discrete_to_float[index] = i->value;
		if (index > 0)
			for(; index < 256; ++index)
				discrete_to_float[index] = discrete_to_float[index - 1];
	}

	pixel_size = 0;
	for(int i = 0; i < 4; ++i) {
		channels[i] = i*channel_size;
		for(int j = 0; j < i; ++j)
			if (channels_equality[i][j])
				{ channels[i] = channels[j]; break; }
		if (constant_equality[i])
			channels[i] = -1;
		else
			constant_channels[i] = 0;
		if (channels[i] >= 0 && channels[i] + channel_size > pixel_size)
			pixel_size = channels[i] + channel_size;
	}
	this->constant = constant;
	this->width = width;
	this->height = height;
	row_size = width * pixel_size;

	const char *s;
	bool gzip = (s = getenv("SYNFIG_PACK_IMAGES_GZIP")) && atoi(s) != 0;
	bool split = (s = getenv("SYNFIG_PACK_IMAGES_SPLIT")) && atoi(s) != 0;

	if (pixel_size == 0) {
		// do nothing
	}
	else
	if ((!gzip && !split) || std::max((width-1)/ChunkSize + 1, (height-1)/ChunkSize + 1)*CacheRows*ChunkSize*ChunkSize*16 > width*height)
	{
		// no compression
		data.resize(row_size*height);
		char *pixel = &data.front();
		for(int row = 0; row < height; ++row)
			for(const Color *color = (const Color*)((const char*)pixels + row*pitch), *end = color + width; color < end; ++color, pixel += pixel_size)
				set_pixel(pixel, *color);
	}
	else
	{
		// make chunks
		chunk_row_size = pixel_size*ChunkSize;
		chunk_size = chunk_row_size*ChunkSize;
		chunks_width = (width-1)/ChunkSize + 1;
		chunks_height = (height-1)/ChunkSize + 1;

		int count = chunks_width*chunks_height;
		std::vector<char> data((count + 1)*sizeof(int), 0);
		std::vector<char> chunk(chunk_size);
		std::vector<char> compressed_chunk(2*chunk.size());
		for(int i = 0; i < count; ++i) {
			char *pixel = &chunk.front();
			for(int r = 0; r < ChunkSize; ++r) {
				int x0 = i%chunks_width*ChunkSize;
				int y0 = i/chunks_width*ChunkSize;
				const Color *color = (const Color*)((const char*)pixels + (y0 + r)*pitch) + x0;
				for(int c = 0; c < ChunkSize; ++c, pixel += pixel_size, ++color)
					if (x0+c < width && y0+r < height)
						set_pixel(pixel, *color);
					else
						set_pixel(pixel, Color());
			}

			const void* current_data = &chunk.front();
			int size = (int)chunk.size();

			if (gzip) {
				int gzip_size = (int)zstreambuf::pack(&compressed_chunk.front(), compressed_chunk.size(), &chunk.front(), chunk.size(), true);
				if (gzip_size <= (int)chunk.size()/4)
				{
					current_data = &compressed_chunk.front();
					size = gzip_size;
				}
			}

			((int*)(void*)&data.front())[i] = data.size();
			data.resize(data.size() + size);
			memcpy(&data[data.size() - size], current_data, size);
		}
		((int*)(void*)&data.front())[count] = data.size();

		this->data = data;
	}
}

void
PackedSurface::get_pixels(Color *target) const {
	if (target == NULL || width <= 0 || height <= 0)
		return;
	Reader reader(*this);
	Color *color = target;
	for(int y = 0; y < height; ++y)
		for(int x = 0; x < width; ++x, ++color)
			*color = reader.get_pixel(x, y);
}



/* === E N T R Y P O I N T ================================================= */
