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
	class Params {
	public:
		synfig::Surface *dest;
		RectInt dest_rect;
		const synfig::Surface *src;
		VectorInt src_offset;
		RectInt src_rect;
		rendering::Blur::Type type;
		Vector size;
		Vector amplified_size;
		VectorInt extra_size;
		bool blend;
		Color::BlendMethod blend_method;
		ColorReal amount;

		Params(): dest(), src(), blend(), blend_method(), amount() { }
		Params(
			synfig::Surface &dest,
			const RectInt &dest_rect,
			const synfig::Surface &src,
			const VectorInt src_offset,
			rendering::Blur::Type type,
			const Vector &size,
			bool blend,
			Color::BlendMethod blend_method,
			ColorReal amount
		):
			dest(&dest),
			dest_rect(dest_rect),
			src(&src),
			src_offset(src_offset),
			type(type),
			size(size),
			blend(blend),
			blend_method(blend_method),
			amount(amount)
		{ }

		bool validate();
	};

	class IIRCoefficients
	{
	public:
		union {
			struct { Real k0, k1, k2, k3; };
			struct { Real k[4]; };
		};
		IIRCoefficients(): k0(), k1(), k2(), k3() { }
	};

	static Real get_size_amplifier(rendering::Blur::Type type);
	static Real get_extra_size(rendering::Blur::Type type);
	static VectorInt get_extra_size(rendering::Blur::Type type, const Vector &size);

private:
	static constexpr Real iir_min_radius = 1.0;
	static constexpr Real iir_max_radius = 2048.0;
	static constexpr Real iir_radius_step = 0.1;

	static IIRCoefficients get_iir_coefficients(Real radius);

	//! Simple blur by pattern
	static void blur_pattern(const Params &params);

	//! Full-size blur using Furier transform
	static void blur_fft(const Params &params);

	//! Fast box-blur
	static void blur_box(const Params &params);

	//! Blur using infinite impulse response filter (gaussian only)
	static void blur_iir(const Params &params);

public:
	//! Generic blur function
	static void blur(Params params);
};

} /* end namespace software */
} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
