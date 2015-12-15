/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/blur.h
**	\brief Blur Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_SOFTWARE_BLUR_H
#define __SYNFIG_RENDERING_SOFTWARE_BLUR_H

/* === H E A D E R S ======================================================= */

#include <vector>

#include "../../primitive/blur.h"

#include <synfig/vector.h>
#include <synfig/complex.h>
#include <synfig/surface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace software
{

class Blur
{
public:
	static Real gauss(Real x, Real radius);

	static Real get_extra_size(rendering::Blur::Type type);
	static VectorInt get_extra_size(rendering::Blur::Type type, const Vector &size);

	static void fill_pattern_2d(
		Complex *pattern,
		int rows,
		int row_stride,
		int cols,
		int col_stride,
		Real value );

	static void fill_pattern_box(
		Complex *pattern,
		int count,
		int stride,
		Real size );

	static void fill_pattern_gauss(
		Complex *pattern,
		int count,
		int stride,
		Real size );

	static void fill_pattern_2d_disk(
		Complex *pattern,
		int rows,
		int row_stride,
		int cols,
		int col_stride,
		const Vector &size );

	static void multiply(
		Complex *target,
		int count,
		int stride,
		const Complex *amplfier,
		int amplfier_stride );

	static void multiply_2d(
		Complex *target,
		int rows,
		int row_stride,
		int cols,
		int col_stride,
		const Complex *amplfier,
		int amplfier_row_stride,
		int amplfier_col_stride );

	static void add(
		Complex *target,
		int count,
		int stride,
		const Complex *amplfier,
		int amplfier_stride );

	static void add_2d(
		Complex *target,
		int rows,
		int row_stride,
		int cols,
		int col_stride,
		const Complex *amplfier,
		int amplfier_row_stride,
		int amplfier_col_stride );

	//! Full-size blur using Furier transform
	static void blur_fft(
		synfig::Surface &dest,
		const RectInt &dest_rect,
		const synfig::Surface &src,
		const VectorInt src_offset,
		rendering::Blur::Type type,
		const Vector &size,
		bool blend,
		Color::BlendMethod blend_method,
		Color::value_type amount );

	//! Generic blur function
	static void blur(
		synfig::Surface &dest,
		const RectInt &dest_rect,
		const synfig::Surface &src,
		const VectorInt src_offset,
		rendering::Blur::Type type,
		const Vector &size,
		bool blend,
		Color::BlendMethod blend_method,
		Color::value_type amount );
};

} /* end namespace software */
} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
