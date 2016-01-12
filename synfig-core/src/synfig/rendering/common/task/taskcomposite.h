/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskcomposite.h
**	\brief TaskComposite Header
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

#ifndef __SYNFIG_RENDERING_TASKCOMPOSITE_H
#define __SYNFIG_RENDERING_TASKCOMPOSITE_H

/* === H E A D E R S ======================================================= */

#include <synfig/color.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskComposite
{
public:
	bool blend;
	Color::BlendMethod blend_method;
	Color::value_type amount;

	TaskComposite():
		blend(),
		blend_method(Color::BLEND_COMPOSITE),
		amount() { }

	virtual ~TaskComposite() { }

	virtual Color::BlendMethodFlags get_supported_blend_methods() const { return 0; }

	bool is_blend_method_supported(Color::BlendMethod blend_method)
		{ return get_supported_blend_methods() & (1 << blend_method); }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
