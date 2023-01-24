/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/blur.h
**	\brief Blur Header file
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_RENDERING_BLUR_H
#define __SYNFIG_RENDERING_BLUR_H

/* === H E A D E R S ======================================================= */

#include <synfig/vector.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

struct Blur
{
public:
	enum Type
	{
		BOX				= 0,
		FASTGAUSSIAN	= 1,
		CROSS			= 2,
		GAUSSIAN		= 3,
		DISC			= 4
	};

	Type type;
	synfig::Vector size;

	Blur(): type(BOX) { }
	Blur(Type type, const synfig::Vector& size): type(type), size(size) { }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* === E N D =============================================================== */

#endif
