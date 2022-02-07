/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/blur.cpp
**	\brief Blur
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <cassert>

#include <algorithm>
#include <functional>

#include "blur.h"

#include "blurtemplates.h"
#include "fft.h"
//#include "blur_iir_coefficients.cpp"
#endif

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

	amplified_size = size*get_size_amplifier(type);
	amplified_size[0] = fabs(amplified_size[0]);
	amplified_size[1] = fabs(amplified_size[1]);

	extra_size = get_extra_size(type, size);
	rect_set_intersect(dest_rect, dest_rect, RectInt(0, 0, dest->get_w(), dest->get_h()));
	if (!dest_rect.valid()) return false;

	VectorInt offset = src_offset - dest_rect.get_min();
	src_rect = dest_rect + offset;
	src_rect.minx -= extra_size[0];
	src_rect.miny -= extra_size[1];
	src_rect.maxx += extra_size[0];
	src_rect.maxy += extra_size[1];
	if (!src_rect.valid()) return false;
	rect_set_intersect(src_rect, src_rect, RectInt(0, 0, src->get_w(), src->get_h()));
	if (!src_rect.valid()) return false;

	dest_rect = src_rect - offset;
	dest_rect.minx += extra_size[0];
	dest_rect.miny += extra_size[1];
	dest_rect.maxx -= extra_size[0];
	dest_rect.maxy -= extra_size[1];
	if (!dest_rect.valid()) return false;
	if (!rect_contains(RectInt(0, 0, dest->get_w(), dest->get_h()), dest_rect)) return false;

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

	static const Real fast_min_value = 1.0 / 128.0;
	static const Real fast_gauss_size = BlurTemplates::ungauss(fast_min_value)
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
			return fast_gauss_size;
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
software::Blur::blur_pattern(const Params &params)
{
	// init
	const int channels = 4;
	int rows = params.src_rect.get_size()[1];
	int cols = params.src_rect.get_size()[0];
	int pattern_rows = params.extra_size[1] + 1;
	int pattern_cols = params.extra_size[0] + 1;
	std::vector<ColorReal> src_surface(rows*cols*channels);
	std::vector<ColorReal> dst_surface(rows*cols*channels);
	std::vector<ColorReal> full_pattern;
	std::vector<ColorReal> row_pattern;
	std::vector<ColorReal> col_pattern;
	bool full = false;
	bool cross = false;

	Array<ColorReal, 3> arr_src_surface(&src_surface.front());
	Array<ColorReal, 3> arr_dst_surface(&dst_surface.front());

	arr_src_surface
		.set_dim(rows, cols*channels)
		.set_dim(cols, channels)
		.set_dim(channels, 1);
	arr_dst_surface
		.set_dim(rows, cols*channels)
		.set_dim(cols, channels)
		.set_dim(channels, 1);
	Array<ColorReal, 2> arr_full_pattern;
	arr_full_pattern
		.set_dim(pattern_rows, pattern_cols)
		.set_dim(pattern_cols, 1);
	Array<ColorReal, 1> arr_row_pattern;
	arr_row_pattern
		.set_dim(pattern_cols, 1);
	Array<ColorReal, 1> arr_col_pattern;
	arr_col_pattern
		.set_dim(pattern_rows, 1);

	// prepare surface (apply alpha)
	BlurTemplates::surface_read(arr_src_surface, *params.src, VectorInt(0, 0), params.src_rect);

	// alloc memory
	switch(params.type)
	{
	case rendering::Blur::BOX:
	case rendering::Blur::CROSS:
	case rendering::Blur::GAUSSIAN:
	case rendering::Blur::FASTGAUSSIAN:
		row_pattern.resize(pattern_cols);
		col_pattern.resize(pattern_rows);
		arr_row_pattern.pointer = &row_pattern.front();
		arr_col_pattern.pointer = &col_pattern.front();
		break;
	case rendering::Blur::DISC:
		full_pattern.resize(pattern_rows*pattern_cols);
		arr_full_pattern.pointer =&full_pattern.front();
		break;
	default:
		assert(false);
		return;
	}

	// create patterns
	switch(params.type)
	{
	case rendering::Blur::BOX:
		BlurTemplates::fill_pattern_box(arr_row_pattern, (ColorReal)params.amplified_size[0]);
		BlurTemplates::fill_pattern_box(arr_col_pattern, (ColorReal)params.amplified_size[1]);
		break;
	case rendering::Blur::CROSS:
		BlurTemplates::fill_pattern_box(arr_row_pattern, (ColorReal)params.amplified_size[0]);
		BlurTemplates::fill_pattern_box(arr_col_pattern, (ColorReal)params.amplified_size[1]);
		cross = true;
		break;
	case rendering::Blur::GAUSSIAN:
	case rendering::Blur::FASTGAUSSIAN:
		BlurTemplates::fill_pattern_gauss(arr_row_pattern, (ColorReal)params.amplified_size[0]);
		BlurTemplates::fill_pattern_gauss(arr_col_pattern, (ColorReal)params.amplified_size[1]);
		break;
	case rendering::Blur::DISC:
		BlurTemplates::fill_pattern_2d_disk(
			arr_full_pattern,
			(ColorReal)params.amplified_size[0],
			(ColorReal)params.amplified_size[1] );
		full = true;
		break;
	default:
		assert(false);
		return;
	}

	// process
	if (full)
	{
		BlurTemplates::normalize_half_pattern_2d( arr_full_pattern );
		for(Array<ColorReal, 3>::Iterator dst(arr_dst_surface.reorder(2, 0, 1)), src(arr_src_surface.reorder(2, 0, 1)); dst; ++dst, ++src)
			BlurTemplates::blur_2d_pattern(*dst, *src, arr_full_pattern);
	}
	else
	{
		BlurTemplates::normalize_half_pattern( arr_row_pattern );
		BlurTemplates::normalize_half_pattern( arr_col_pattern );

		Array<ColorReal, 3> arr_src_surface_rows(arr_src_surface.reorder(2, 0, 1));
		Array<ColorReal, 3> arr_src_surface_cols(arr_src_surface_rows.reorder(0, 2, 1));

		Array<ColorReal, 3> arr_dst_surface_rows(arr_dst_surface.reorder(2, 0, 1));
		Array<ColorReal, 3> arr_dst_surface_cols(arr_dst_surface_rows.reorder(0, 2, 1));

		if (cross)
		{
			arr_row_pattern.process< std::multiplies<ColorReal> >(0.5);
			arr_col_pattern.process< std::multiplies<ColorReal> >(0.5);
		}

		for(Array<ColorReal, 3>::Iterator src_channel(arr_src_surface_rows), dst_channel(arr_dst_surface_rows); dst_channel; ++src_channel, ++dst_channel)
			for(Array<ColorReal, 2>::Iterator sr(*src_channel), dr(*dst_channel); dr; ++sr, ++dr)
				BlurTemplates::blur_pattern(*dr, *sr, arr_row_pattern);

		if (!cross)
		{
			std::swap(arr_src_surface_cols.pointer, arr_dst_surface_cols.pointer);
			std::swap(arr_src_surface.pointer, arr_dst_surface.pointer);
			memset(&src_surface.front(), 0, sizeof(src_surface.front())*src_surface.size());
		}

		for(Array<ColorReal, 3>::Iterator src_channel(arr_src_surface_cols), dst_channel(arr_dst_surface_cols); dst_channel; ++src_channel, ++dst_channel)
			for(Array<ColorReal, 2>::Iterator sr(*src_channel), dr(*dst_channel); dr; ++sr, ++dr)
				BlurTemplates::blur_pattern(*dr, *sr, arr_row_pattern);
	}

	// copy result surface and restore alpha
	BlurTemplates::surface_write(
		*params.dest,
		arr_dst_surface,
		params.dest_rect,
		params.src_offset - params.src_rect.get_min(),
		params.blend,
		params.blend_method,
		params.amount );
}

void
software::Blur::blur_fft(const Params &params)
{
	// init
	const int channels = 4;
	int rows = FFT::get_valid_count(params.src_rect.get_size()[1]);
	int cols = FFT::get_valid_count(params.src_rect.get_size()[0]);
	std::vector<Complex> surface(rows*cols*channels);
	std::vector<Complex> full_pattern;
	std::vector<Complex> row_pattern;
	std::vector<Complex> col_pattern;
	bool full = false;
	bool cross = false;

	Array<Real, 4> arr_surface((Real*)&surface.front());

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
	BlurTemplates::surface_read(arr_surface.reorder(0, 1, 2), *params.src, VectorInt(0, 0), params.src_rect);

	// alloc memory
	switch(params.type)
	{
	case rendering::Blur::BOX:
	case rendering::Blur::CROSS:
	case rendering::Blur::GAUSSIAN:
	case rendering::Blur::FASTGAUSSIAN:
		row_pattern.resize(cols);
		col_pattern.resize(rows);
		arr_row_pattern.pointer = (Real*)&row_pattern.front();
		arr_col_pattern.pointer = (Real*)&col_pattern.front();
		break;
	case rendering::Blur::DISC:
		full_pattern.resize(rows*cols);
		arr_full_pattern.pointer = (Real*)&full_pattern.front();
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
		BlurTemplates::mirror_pattern_2d( arr_full_pattern.reorder(0, 1) );
		BlurTemplates::normalize_full_pattern_2d( arr_full_pattern.reorder(0, 1) );

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
		BlurTemplates::mirror_pattern( arr_row_pattern.reorder(0) );
		BlurTemplates::mirror_pattern( arr_col_pattern.reorder(0) );
		BlurTemplates::normalize_full_pattern( arr_row_pattern.reorder(0) );
		BlurTemplates::normalize_full_pattern( arr_col_pattern.reorder(0) );

		std::vector<Complex> surface_copy;
		Array<Complex, 3> arr_surface_rows(arr_surface.group_items<Complex>().reorder(2, 0, 1));
		Array<Complex, 3> arr_surface_cols(arr_surface_rows.reorder(0, 2, 1));

		if (cross)
		{
			arr_row_pattern.reorder(0).process< std::multiplies<Real> >(0.5);
			arr_col_pattern.reorder(0).process< std::multiplies<Real> >(0.5);
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
	BlurTemplates::surface_write(
		*params.dest,
		arr_surface.reorder(0, 1, 2),
		params.dest_rect,
		params.src_offset - params.src_rect.get_min(),
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

	std::vector<ColorReal> surface(rows*cols*channels);
	Array<ColorReal, 3> arr_surface(&surface.front());
	arr_surface
		.set_dim(rows, cols*channels)
		.set_dim(cols, channels)
		.set_dim(channels, 1);
	BlurTemplates::surface_read(arr_surface, *params.src, VectorInt(0, 0), params.src_rect);

	Vector size = params.amplified_size;
	bool cross = false;
	bool count = 1;
	switch(params.type)
	{
	case rendering::Blur::BOX:
		break;
	case rendering::Blur::GAUSSIAN:
	case rendering::Blur::FASTGAUSSIAN:
		size *= 1.662155813;
		count = 2;
		break;
	case rendering::Blur::CROSS:
		cross = true;
		break;
	default:
		assert(false);
		return;
	}

	std::deque<ColorReal> q;
	std::vector<ColorReal> surface_copy;
	Array<ColorReal, 3> arr_surface_rows(arr_surface.reorder(2, 0, 1));
	Array<ColorReal, 3> arr_surface_cols(arr_surface_rows.reorder(0, 2, 1));

	if (cross)
	{
		arr_surface.process< std::multiplies<ColorReal> >(0.5);
		surface_copy = surface;
		arr_surface_cols.pointer = &surface_copy.front();
	}

	if (true || fabs(size[0] - round(size[0])) < precision)
		for(Array<ColorReal, 3>::Iterator channel(arr_surface_rows); channel; ++channel)
			for(Array<ColorReal, 2>::Iterator r(*channel); r; ++r)
				for(int i = 0; i < count; ++i)
					BlurTemplates::blur_box_discrete(*r, q, (int)round(size[0]));
	else
		for(Array<ColorReal, 3>::Iterator channel(arr_surface_rows); channel; ++channel)
			for(Array<ColorReal, 2>::Iterator r(*channel); r; ++r)
				for(int i = 0; i < count; ++i)
					BlurTemplates::blur_box_aa(*r, q, (ColorReal)size[0]);

	if (true || fabs(size[1] - round(size[1])) < precision)
		for(Array<ColorReal, 3>::Iterator channel(arr_surface_cols); channel; ++channel)
			for(Array<ColorReal, 2>::Iterator c(*channel); c; ++c)
				for(int i = 0; i < count; ++i)
					BlurTemplates::blur_box_discrete(*c, q, (int)round(size[1]));
	else
		for(Array<ColorReal, 3>::Iterator channel(arr_surface_cols); channel; ++channel)
			for(Array<ColorReal, 2>::Iterator c(*channel); c; ++c)
				for(int i = 0; i < count; ++i)
					BlurTemplates::blur_box_aa(*c, q, (ColorReal)size[1]);

	if (cross)
		arr_surface_rows
			.process< std::plus<ColorReal> >(
				arr_surface_cols.reorder(0, 2, 1) );

	BlurTemplates::surface_write(
		*params.dest,
		arr_surface,
		params.dest_rect,
		params.src_offset - params.src_rect.get_min(),
		params.blend,
		params.blend_method,
		params.amount );
}
/*
software::Blur::IIRCoefficients
software::Blur::get_iir_coefficients(Real radius)
{
	const Real precision(1e-8);
	radius = max(iir_min_radius + precision, min(iir_max_radius - precision, fabs(radius)));

	Real x = (radius - iir_min_radius)/iir_radius_step;
	//int index = floor(x);
	//x -= index;
	int index = round(x);
	x = 0.0;

	const Real *prev = iir_coefficients_unprepared[index];
	const Real *next = iir_coefficients_unprepared[index+1];

	Real k[3];
	for(int i = 0; i < 3; ++i)
		k[i] = prev[i]*(1.0 - x) + next[i]*x;

	Real a = 1.0/k[0];
	Real b = a*cos(PI*k[1]);
	Real c = 1.0/k[2];

	IIRCoefficients coef;
	coef.k1 = (a*a + 2.0*c*b)/(c*a*a);
	coef.k2 = -(c + 2.0*b)/(c*a*a);
	coef.k3 = 1.0/(c*a*a);

	coef.k0 = 1.0 - coef.k1 - coef.k2 - coef.k3;

	return coef;
}

void
software::Blur::blur_iir(const Params &params)
{
	const Real precision(1e-5);
	const int channels = 4;
	int rows = params.src_rect.get_size()[1];
	int cols = params.src_rect.get_size()[0];
	int pattern_rows = params.extra_size[1] + 1;
	int pattern_cols = params.extra_size[0] + 1;

	vector<ColorReal> surface(rows*cols*channels);
	vector<ColorReal> tmp_surface;
	vector<ColorReal> row_pattern;
	vector<ColorReal> col_pattern;

	Array<ColorReal, 3> arr_surface(&surface.front());
	arr_surface
		.set_dim(rows, cols*channels)
		.set_dim(cols, channels)
		.set_dim(channels, 1);
	Array<ColorReal, 1> arr_row_pattern;
	arr_row_pattern
		.set_dim(pattern_cols, 1);
	Array<ColorReal, 1> arr_col_pattern;
	arr_col_pattern
		.set_dim(pattern_rows, 1);
	bool use_row_pattern = (params.extra_size[0] < 4 && params.amplified_size[0] < 2.0);
	bool use_col_pattern = (params.extra_size[1] < 4 && params.amplified_size[1] < 2.0);
	bool use_pattern = use_row_pattern || use_col_pattern;
	if (use_pattern)
		tmp_surface.resize(rows*cols*channels);
	Array<ColorReal, 3> arr_tmp_surface(
		use_pattern ? &tmp_surface.front() : &surface.front(),
		arr_surface );
	
	
	BlurTemplates::surface_read(arr_surface, *params.src, VectorInt(0, 0), params.src_rect);

	switch(params.type)
	{
	case rendering::Blur::GAUSSIAN:
	case rendering::Blur::FASTGAUSSIAN:
		if (use_row_pattern)
		{
			row_pattern.resize(pattern_cols);
			arr_row_pattern.pointer = &row_pattern.front();
			BlurTemplates::fill_pattern_gauss(arr_row_pattern, (ColorReal)params.amplified_size[0]);
			BlurTemplates::normalize_half_pattern( arr_row_pattern.reorder(0) );
		}
		if (use_col_pattern)
		{
			col_pattern.resize(pattern_rows);
			arr_col_pattern.pointer = &col_pattern.front();
			BlurTemplates::fill_pattern_gauss(arr_col_pattern, (ColorReal)params.amplified_size[1]);
			BlurTemplates::normalize_half_pattern( arr_col_pattern.reorder(0) );
		}
		break;
	default:
		assert(false);
		return;
	}

	Array<ColorReal, 3> arr_surface_rows(arr_surface.reorder(2, 0, 1));
	Array<ColorReal, 3> arr_surface_cols(arr_surface_rows.reorder(0, 2, 1));

	IIRCoefficients cr = get_iir_coefficients(params.amplified_size[0]);
	IIRCoefficients cc = get_iir_coefficients(params.amplified_size[1]);

	ColorReal cr0 = (ColorReal)cr.k0;
	ColorReal cr1 = (ColorReal)cr.k1;
	ColorReal cr2 = (ColorReal)cr.k2;
	ColorReal cr3 = (ColorReal)cr.k3;

	ColorReal cc0 = (ColorReal)cc.k0;
	ColorReal cc1 = (ColorReal)cc.k1;
	ColorReal cc2 = (ColorReal)cc.k2;
	ColorReal cc3 = (ColorReal)cc.k3;

	Array<ColorReal, 3> arr_src_surface_rows(arr_surface.reorder(2, 0, 1));
	Array<ColorReal, 3> arr_src_surface_cols(arr_surface_rows.reorder(0, 2, 1));

	Array<ColorReal, 3> arr_dst_surface_rows(arr_tmp_surface.reorder(2, 0, 1));
	Array<ColorReal, 3> arr_dst_surface_cols(arr_dst_surface_rows.reorder(0, 2, 1));
	
	if (fabs(params.amplified_size[0]) > precision)
	{
		if (use_row_pattern)
		{
			for(Array<ColorReal, 3>::Iterator src_channel(arr_src_surface_rows), dst_channel(arr_dst_surface_rows); dst_channel; ++src_channel, ++dst_channel)
				for(Array<ColorReal, 2>::Iterator sr(*src_channel), dr(*dst_channel); dr; ++sr, ++dr)
					BlurTemplates::blur_pattern(*dr, *sr, arr_row_pattern);
			swap(arr_src_surface_cols.pointer, arr_dst_surface_cols.pointer);
			swap(arr_surface.pointer, arr_tmp_surface.pointer);
			arr_surface_cols.pointer = arr_surface.pointer;
			memset(&surface.front(), 0, sizeof(surface.front())*surface.size());
		}
		else
		{
			for(Array<ColorReal, 3>::Iterator channel(arr_surface_rows); channel; ++channel)
				for(Array<ColorReal, 2>::Iterator r(*channel); r; ++r)
					BlurTemplates::blur_iir(*r, cr0, cr1, cr2, cr3);
		}
	}

	if (fabs(params.amplified_size[1]) > precision)
	{
		if (use_col_pattern)
		{
			for(Array<ColorReal, 3>::Iterator src_channel(arr_src_surface_cols), dst_channel(arr_dst_surface_cols); dst_channel; ++src_channel, ++dst_channel)
				for(Array<ColorReal, 2>::Iterator sr(*src_channel), dr(*dst_channel); dr; ++sr, ++dr)
					BlurTemplates::blur_pattern(*dr, *sr, arr_col_pattern);
			swap(arr_surface.pointer, arr_tmp_surface.pointer);
		}
		else
		{
			for(Array<ColorReal, 3>::Iterator channel(arr_surface_cols); channel; ++channel)
				for(Array<ColorReal, 2>::Iterator c(*channel); c; ++c)
					BlurTemplates::blur_iir(*c, cc0, cc1, cc2, cc3);
		}
	}

	BlurTemplates::surface_write(
		*params.dest,
		arr_surface,
		params.dest_rect,
		params.src_offset - params.src_rect.get_min(),
		params.blend,
		params.blend_method,
		params.amount );
}*/

void
software::Blur::blur(Params params)
{
	if (!params.validate()) return;

	if ( params.type == rendering::Blur::BOX
	  || params.type == rendering::Blur::CROSS )
		{ blur_box(params); return; }

	if ( params.type == rendering::Blur::DISC
      && fabs(params.extra_size[0]) < 8
      && fabs(params.extra_size[1]) < 8 )
		{ blur_pattern(params); return; }

	if ( params.type == rendering::Blur::DISC
      && (params.extra_size[0] + 1)*(params.extra_size[1] + 1) < 64.0 )
		{ blur_pattern(params); return; }

	if ( params.type == rendering::Blur::GAUSSIAN
	  && params.extra_size[0] < 32
	  && params.extra_size[1] < 32 )
		{ blur_pattern(params); return; }

	if ( params.type == rendering::Blur::FASTGAUSSIAN )
		{ blur_box(params); return; }

	blur_fft(params);
}

/* === E N T R Y P O I N T ================================================= */
