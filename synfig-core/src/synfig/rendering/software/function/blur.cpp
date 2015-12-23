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

Real
software::Blur::get_extra_size(rendering::Blur::Type type)
{
	static const Real min_value = 1.0 / 4096.0;
	static const Real gauss_size = sqrt(-2.0*log(min_value/BlurTemplates::gauss(0.0, 1.0)));

	switch(type)
	{
		case rendering::Blur::BOX:
		case rendering::Blur::CROSS:
		case rendering::Blur::DISC:
			return 1.0;
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
	return VectorInt( (int)ceil(fabs(s[0]) + 0.5 - precision), (int)ceil(fabs(s[1]) + 0.5 - precision) );
}

void
software::Blur::blur_fft(
	synfig::Surface &dest,
	const RectInt &dest_rect,
	const synfig::Surface &src,
	const VectorInt src_offset,
	rendering::Blur::Type type,
	const Vector &size,
	bool blend,
	Color::BlendMethod blend_method,
	ColorReal amount )
{
	const Real precision = 1e-10;

	if (blend && fabs(amount) < precision)
		return;

	if ( !dest.is_valid()
	  || !dest_rect.valid()
	  || !src.is_valid() ) return;

	Vector s(fabs(size[0]), fabs(size[1]));

	// rects
	VectorInt offset = dest_rect.get_min() - src_offset;

	VectorInt extra_size = get_extra_size(type, size);
	RectInt dr = dest_rect;
	etl::set_intersect(dr, dr, RectInt(0, 0, dest.get_w(), dest.get_h()));
	if (!dr.valid()) return;

	RectInt sr = dr - offset;
	sr.minx -= extra_size[0];
	sr.miny -= extra_size[1];
	sr.maxx += extra_size[0];
	sr.maxy += extra_size[1];
	if (!sr.valid()) return;
	etl::set_intersect(sr, sr, RectInt(0, 0, src.get_w(), src.get_h()));
	if (!sr.valid()) return;
	etl::set_intersect(dr, dr, sr + offset);
	if (!dr.valid()) return;
	offset -= sr.get_min();

	VectorInt sr_size = sr.get_size();
	VectorInt dr_size = dr.get_size();

	// init
	const int channels = 4;
	int rows = FFT::get_valid_count(sr_size[1]);
	int cols = FFT::get_valid_count(sr_size[0]);
	vector<Complex> surface(rows*cols*channels);
	vector<Complex> full_pattern;
	vector<Complex> row_pattern;
	vector<Complex> col_pattern;
	bool full = false;
	bool cross = false;

	Array<const ColorReal, 3> arr_src((const ColorReal*)&src[sr.miny][sr.minx]);
	arr_src
		.set_dim(rows, src.get_pitch()/sizeof(Color)*channels)
		.set_dim(cols, channels)
		.set_dim(channels, 1);
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
	Array<const Color, 2>::Iterator rr(arr_src.group_items<const Color>());
	for(Array<Real, 3>::Iterator r(arr_surface.reorder(0, 1, 2)); r; ++r, ++rr)
	{
		Array<const Color, 1>::Iterator cc(*rr);
		for(Array<Real, 2>::Iterator c(*r); c; ++c, ++cc)
		{
			ColorReal a = cc->get_a();
			(*c)[0] = cc->get_r()*a;
			(*c)[1] = cc->get_g()*a;
			(*c)[2] = cc->get_b()*a;
			(*c)[3] = a;
		}
	}

	// alloc memory
	switch(type)
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
		break;
	}

	// create patterns
	switch(type)
	{
	case rendering::Blur::BOX:
		BlurTemplates::fill_pattern_box(arr_row_pattern.reorder(0), s[0]);
		BlurTemplates::fill_pattern_box(arr_col_pattern.reorder(0), s[1]);
		break;
	case rendering::Blur::CROSS:
		BlurTemplates::fill_pattern_box(arr_row_pattern.reorder(0), s[0]);
		BlurTemplates::fill_pattern_box(arr_col_pattern.reorder(0), s[1]);
		cross = true;
		break;
	case rendering::Blur::GAUSSIAN:
	case rendering::Blur::FASTGAUSSIAN:
		BlurTemplates::fill_pattern_gauss(arr_row_pattern.reorder(0), s[0]);
		BlurTemplates::fill_pattern_gauss(arr_col_pattern.reorder(0), s[1]);
		break;
	case rendering::Blur::DISC:
		BlurTemplates::fill_pattern_2d_disk(arr_full_pattern.reorder(0, 1), s[0], s[1]);
		full = true;
		break;
	default:
		assert(false);
		break;
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
	if (blend)
	{
		Surface::alpha_pen apen(dest.get_pen(0, 0));
		apen.set_blend_method(blend_method);
		for(int r = 0; r < dr_size[1]; ++r)
		{
			apen.move_to(dr.minx, r + dr.miny);
			for( Array<Real, 2>::Iterator cmpl(arr_surface.reorder(0, 1, 2)[r+offset[1]], offset[0], offset[0]+dr_size[0]);
				 cmpl;
				 ++cmpl, apen.inc_x() )
			{
				Real a = (*cmpl)[3];
				Real one_div_a = fabs(a) < precision ? 0.0 : 1.0/a;
				apen.put_value(
					Color( (*cmpl)[0]*one_div_a,
						   (*cmpl)[1]*one_div_a,
						   (*cmpl)[2]*one_div_a,
						   a ),
					amount );
				apen.inc_x();
			}
		}
	}
	else
	{
		for(int r = 0; r < dr_size[1]; ++r)
		{
			Color *color = &dest[r + dr.miny][dr.minx];
			for( Array<Complex, 2>::Iterator cmpl(arr_surface.group_items<Complex>()[r+offset[1]], offset[0], offset[0]+dr_size[0]);
				 cmpl;
				 ++cmpl, ++color )
			{
				Real a = abs((*cmpl)[3]);
				Real one_div_a = fabs(a) < precision ? 0.0 : 1.0/a;
				color->set_r( abs((*cmpl)[0])*one_div_a );
				color->set_g( abs((*cmpl)[1])*one_div_a );
				color->set_b( abs((*cmpl)[2])*one_div_a );
				color->set_a( a );
			}
		}
	}
}

void
software::Blur::blur(
	synfig::Surface &dest,
	const RectInt &dest_rect,
	const synfig::Surface &src,
	const VectorInt src_offset,
	rendering::Blur::Type type,
	const Vector &size,
	bool blend,
	Color::BlendMethod blend_method,
	ColorReal amount )
{
	// TODO: special functions for small sizes and for fast-gaussian blur
	blur_fft(dest, dest_rect, src, src_offset, type, size, blend, blend_method, amount);
}

/* === E N T R Y P O I N T ================================================= */
