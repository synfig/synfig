/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/blur.cpp
**	\brief Blur
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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <cassert>

#include <algorithm>
#include <functional>

#include "blur.h"

#include "blurtemplates.h"
#include "fft.h"
#include <synfig/angle.h>
#include <synfig/general.h>

#endif

using namespace std;
using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
software::Blur::Params::validate()
{
	const Real precision = 1e-10;

	if (blend && fabs(amount) < precision)
		return false;

	if ( !dest->is_valid()
	  || !dest_rect.valid()
	  || !src->is_valid() ) return false;

	offset = src_offset - dest_rect.get_min();

	amplified_size = size*get_size_amplifier(type);

	VectorInt extra_size = get_extra_size(type, size);
	etl::set_intersect(dest_rect, dest_rect, RectInt(0, 0, dest->get_w(), dest->get_h()));
	if (!dest_rect.valid()) return false;

	src_rect = dest_rect + offset;
	src_rect.minx -= extra_size[0];
	src_rect.miny -= extra_size[1];
	src_rect.maxx += extra_size[0];
	src_rect.maxy += extra_size[1];
	if (!src_rect.valid()) return false;
	etl::set_intersect(src_rect, src_rect, RectInt(0, 0, src->get_w(), src->get_h()));
	if (!src_rect.valid()) return false;
	etl::set_intersect(dest_rect, dest_rect, src_rect - offset);
	if (!dest_rect.valid()) return false;
	offset += src_rect.get_min();

	return true;
}

Real
software::Blur::get_size_amplifier(rendering::Blur::Type type)
{
	switch(type)
	{
		case rendering::Blur::BOX:
			return BlurTemplates::amplifier_box<Real>();
		case rendering::Blur::CROSS:
			return BlurTemplates::amplifier_cross<Real>();
		case rendering::Blur::DISC:
			return BlurTemplates::amplifier_disk<Real>();
		case rendering::Blur::FASTGAUSSIAN:
			return BlurTemplates::amplifier_fastgauss<Real>();
		case rendering::Blur::GAUSSIAN:
			return BlurTemplates::amplifier_gauss<Real>();
		default:
			break;
	}
	return 0.0;
}

Real
software::Blur::get_extra_size(rendering::Blur::Type type)
{
	static const Real min_value = 1.0 / 4096.0;
	static const Real gauss_size = BlurTemplates::ungauss(min_value)
	                             * BlurTemplates::amplifier_gauss<Real>();

	switch(type)
	{
		case rendering::Blur::BOX:
			return BlurTemplates::amplifier_box<Real>();
		case rendering::Blur::CROSS:
			return BlurTemplates::amplifier_cross<Real>();
		case rendering::Blur::DISC:
			return BlurTemplates::amplifier_disk<Real>();
		case rendering::Blur::FASTGAUSSIAN:
		case rendering::Blur::GAUSSIAN:
			return gauss_size;
		default:
			break;
	}
	return 0.0;
}

VectorInt
software::Blur::get_extra_size(
	rendering::Blur::Type type,
	const Vector &size )
{
	const Real precision = 1e-10;
	Vector s = size*get_extra_size(type);
	return VectorInt( (int)ceil(fabs(s[0]) + 1.0 - precision), (int)ceil(fabs(s[1]) + 1.0 - precision) );
}

void
software::Blur::blur_fft(const Params &params)
{
	// init
	const int channels = 4;
	int rows = FFT::get_valid_count(params.src_rect.get_size()[1]);
	int cols = FFT::get_valid_count(params.src_rect.get_size()[0]);
	vector<Complex> surface(rows*cols*channels);
	vector<Complex> full_pattern;
	vector<Complex> row_pattern;
	vector<Complex> col_pattern;
	bool full = false;
	bool cross = false;

	Array<Real, 4> arr_surface(&surface.front().real());
	arr_surface
		.set_dim(rows, cols*channels*2)
		.set_dim(cols, channels*2)
		.set_dim(channels, 2)
		.set_dim(2, 1);
	Array<Real, 3> arr_full_pattern;
	arr_full_pattern
		.set_dim(rows, 2*cols)
		.set_dim(cols, 2)
		.set_dim(2, 1);
	Array<Real, 2> arr_row_pattern;
	arr_row_pattern
		.set_dim(cols, 2)
		.set_dim(2, 1);
	Array<Real, 2> arr_col_pattern;
	arr_col_pattern
		.set_dim(rows, 2)
		.set_dim(2, 1);

	// convert surface to complex
	BlurTemplates::read_surface(arr_surface.reorder(0, 1, 2), *params.src, VectorInt(0, 0), params.src_rect);

	// alloc memory
	switch(params.type)
	{
	case rendering::Blur::BOX:
	case rendering::Blur::CROSS:
	case rendering::Blur::GAUSSIAN:
	case rendering::Blur::FASTGAUSSIAN:
		row_pattern.resize(cols);
		col_pattern.resize(rows);
		arr_row_pattern.pointer = &row_pattern.front().real();
		arr_col_pattern.pointer = &col_pattern.front().real();
		break;
	case rendering::Blur::DISC:
		full_pattern.resize(rows*cols);
		arr_full_pattern.pointer = &full_pattern.front().real();
		break;
	default:
		assert(false);
		return;
	}

	// create patterns
	switch(params.type)
	{
	case rendering::Blur::BOX:
		BlurTemplates::fill_pattern_box(arr_row_pattern.reorder(0), params.amplified_size[0]);
		BlurTemplates::fill_pattern_box(arr_col_pattern.reorder(0), params.amplified_size[1]);
		break;
	case rendering::Blur::CROSS:
		BlurTemplates::fill_pattern_box(arr_row_pattern.reorder(0), params.amplified_size[0]);
		BlurTemplates::fill_pattern_box(arr_col_pattern.reorder(0), params.amplified_size[1]);
		cross = true;
		break;
	case rendering::Blur::GAUSSIAN:
	case rendering::Blur::FASTGAUSSIAN:
		BlurTemplates::fill_pattern_gauss(arr_row_pattern.reorder(0), params.amplified_size[0]);
		BlurTemplates::fill_pattern_gauss(arr_col_pattern.reorder(0), params.amplified_size[1]);
		break;
	case rendering::Blur::DISC:
		BlurTemplates::fill_pattern_2d_disk(
			arr_full_pattern.reorder(0, 1),
			params.amplified_size[0],
			params.amplified_size[1] );
		full = true;
		break;
	default:
		assert(false);
		return;
	}

	// process
	if (full)
	{
		FFT::fft2d(arr_full_pattern.group_items<Complex>(), false);
		for(Array<Complex, 3>::Iterator channel(arr_surface.group_items<Complex>().reorder(2, 0, 1)); channel; ++channel)
		{
			FFT::fft2d(*channel, false);
			channel->process< std::multiplies<Complex> >(arr_full_pattern.group_items<Complex>());
			FFT::fft2d(*channel, true);
		}
	}
	else
	{
		vector<Complex> surface_copy;
		Array<Complex, 3> arr_surface_rows(arr_surface.group_items<Complex>().reorder(2, 0, 1));
		Array<Complex, 3> arr_surface_cols(arr_surface_rows.reorder(0, 2, 1));

		if (cross)
		{
			arr_surface.reorder(0, 1, 2).process< std::multiplies<Real> >(0.5);
			surface_copy = surface;
			arr_surface_cols.pointer = &surface_copy.front();
		}

		FFT::fft(arr_row_pattern.group_items<Complex>(), false);
		for(Array<Complex, 3>::Iterator channel(arr_surface_rows); channel; ++channel)
		{
			FFT::fft2d(*channel, false, true, false);
			for(Array<Complex, 2>::Iterator r(*channel); r; ++r)
				r->process< std::multiplies<Complex> >(arr_row_pattern.group_items<Complex>());
			FFT::fft2d(*channel, true, true, false);
		}

		FFT::fft(arr_col_pattern.group_items<Complex>(), false);
		for(Array<Complex, 3>::Iterator channel(arr_surface_cols); channel; ++channel)
		{
			FFT::fft2d(*channel, false, true, false);
			for(Array<Complex, 2>::Iterator c(*channel); c; ++c)
				c->process< std::multiplies<Complex> >(arr_col_pattern.group_items<Complex>());
			FFT::fft2d(*channel, true, true, false);
		}

		arr_surface_rows.process< BlurTemplates::Abs<Complex> >();
		if (cross)
		{
			arr_surface_cols.process< BlurTemplates::Abs<Complex> >();
			arr_surface_rows.split_items<Real>().reorder(0, 1, 2)
				.process< std::plus<Real> >(
					arr_surface_cols.split_items<Real>().reorder(0, 2, 1) );
		}
	}

	// convert surface from complex to color
	BlurTemplates::write_surface(
		*params.dest,
		arr_surface.reorder(0, 1, 2),
		params.dest_rect,
		params.offset,
		params.blend,
		params.blend_method,
		params.amount );
}

void
software::Blur::blur_box(const Params &params)
{
	const Real precision(1e-5);
	const int channels = 4;
	int rows = params.src_rect.get_size()[1];
	int cols = params.src_rect.get_size()[0];

	vector<ColorReal> surface(rows*cols*channels);
	Array<ColorReal, 3> arr_surface(&surface.front());
	arr_surface
		.set_dim(rows, cols*channels)
		.set_dim(cols, channels)
		.set_dim(channels, 1);
	BlurTemplates::read_surface(arr_surface, *params.src, VectorInt(0, 0), params.src_rect);

	bool cross = false;
	switch(params.type)
	{
	case rendering::Blur::BOX:
		break;
	case rendering::Blur::CROSS:
		cross = true;
		break;
	default:
		assert(false);
		return;
	}

	deque<ColorReal> q;
	vector<ColorReal> surface_copy;
	Array<ColorReal, 3> arr_surface_rows(arr_surface.reorder(2, 0, 1));
	Array<ColorReal, 3> arr_surface_cols(arr_surface_rows.reorder(0, 2, 1));

	if (cross)
	{
		arr_surface.process< std::multiplies<ColorReal> >(0.5);
		surface_copy = surface;
		arr_surface_cols.pointer = &surface_copy.front();
	}

	if (fabs(params.amplified_size[0] - round(params.amplified_size[0])) < precision)
		for(Array<ColorReal, 3>::Iterator channel(arr_surface_rows); channel; ++channel)
			for(Array<ColorReal, 2>::Iterator r(*channel); r; ++r)
				BlurTemplates::box_blur_discrete(*r, q, (int)round(params.amplified_size[0]));
	else
		for(Array<ColorReal, 3>::Iterator channel(arr_surface_rows); channel; ++channel)
			for(Array<ColorReal, 2>::Iterator r(*channel); r; ++r)
				BlurTemplates::box_blur_discrete(*r, q, (ColorReal)params.amplified_size[0]);

	if (fabs(params.amplified_size[1] - round(params.amplified_size[1])) < precision)
		for(Array<ColorReal, 3>::Iterator channel(arr_surface_cols); channel; ++channel)
			for(Array<ColorReal, 2>::Iterator c(*channel); c; ++c)
				BlurTemplates::box_blur_discrete(*c, q, (int)round(params.amplified_size[1]));
	else
		for(Array<ColorReal, 3>::Iterator channel(arr_surface_cols); channel; ++channel)
			for(Array<ColorReal, 2>::Iterator c(*channel); c; ++c)
				BlurTemplates::box_blur_aa(*c, q, (ColorReal)params.amplified_size[1]);

	if (cross)
		arr_surface_rows
			.process< std::plus<ColorReal> >(
				arr_surface_cols.reorder(0, 2, 1) );
	//if (cross)
	//	arr_surface_rows
	//		.assign(
	//			arr_surface_cols.reorder(0, 2, 1) );

	BlurTemplates::write_surface(
		*params.dest,
		arr_surface,
		params.dest_rect,
		params.offset,
		params.blend,
		params.blend_method,
		params.amount );
}

void
software::Blur::blur(Params params)
{
	if (!params.validate()) return;

	if ( params.type == rendering::Blur::BOX
	  || params.type == rendering::Blur::CROSS )
	{
		blur_box(params);
		return;
	}

	// TODO: special functions for small sizes and for fast-gaussian blur
	blur_fft(params);
}

/* === E N T R Y P O I N T ================================================= */
