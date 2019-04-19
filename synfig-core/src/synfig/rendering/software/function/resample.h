/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/resample.h
**	\brief Resample Header
**
**	$Id$
**
**	\legal
**	......... ... 2015-2019 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_SOFTWARE_RESAMPLE_H
#define __SYNFIG_RENDERING_SOFTWARE_RESAMPLE_H

/* === H E A D E R S ======================================================= */

#include <synfig/rect.h>
#include <synfig/surface.h>

#include "../surfaceswpacked.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace software
{

class Resample
{
public:
	static void downscale(
		synfig::Surface &dest,
		const RectInt &dest_bounds,
		const synfig::Surface &src,
		const RectInt &src_bounds,
		bool keep_cooked = false );

	static void downscale(
		synfig::Surface &dest,
		const RectInt &dest_bounds,
		const software::PackedSurface &src,
		const RectInt &src_bounds,
		bool keep_cooked = false );

	static void resample(
		synfig::Surface &dest,
		const RectInt &dest_bounds,
		const synfig::Surface &src,
		const RectInt &src_bounds,
		const Matrix &transformation,
		Color::Interpolation interpolation,
		bool blend,
		ColorReal blend_amount,
		Color::BlendMethod blend_method );

	static void resample(
		synfig::Surface &dest,
		const RectInt &dest_bounds,
		const software::PackedSurface &src,
		const RectInt &src_bounds,
		const Matrix &transformation,
		Color::Interpolation interpolation,
		bool blend,
		ColorReal blend_amount,
		Color::BlendMethod blend_method );
};

} /* end namespace software */
} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
