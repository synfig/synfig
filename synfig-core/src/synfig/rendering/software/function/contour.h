/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/contour.h
**	\brief Contour Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_SOFTWARE_CONTOUR_H
#define __SYNFIG_RENDERING_SOFTWARE_CONTOUR_H

/* === H E A D E R S ======================================================= */

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

class Contour
{
public:
	static void render_polyspan(
		synfig::Surface &target_surface,
		const Polyspan &polyspan,
		bool invert,
		bool antialias,
		rendering::Contour::WindingStyle winding_style,
		const Color &color,
		Color::value_type opacity,
		Color::BlendMethod blend_method );

	static void build_polyspan(
		const rendering::Contour::ChunkList &chunks,
		const Matrix &transform_matrix,
		Polyspan &out_polyspan,
		Real detail = 0.25 );

	static void render_contour(
		synfig::Surface &target_surface,
		const rendering::Contour::ChunkList &chunks,
		bool invert,
		bool antialias,
		rendering::Contour::WindingStyle winding_style,
		const Matrix &transform_matrix,
		const Color &color,
		Color::value_type opacity,
		Color::BlendMethod blend_method );
};

} /* end namespace software */
} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
