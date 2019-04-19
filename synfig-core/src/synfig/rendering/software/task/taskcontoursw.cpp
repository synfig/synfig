/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskcontoursw.cpp
**	\brief TaskContourSW
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#include <synfig/debug/debugsurface.h>

#include "../../primitive/polyspan.h"
#include "../../common/task/taskcontour.h"
#include "../../common/task/taskblend.h"
#include "tasksw.h"
#include "../function/contour.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskContourSW: public TaskContour, public TaskSW,
	public TaskInterfaceBlendToTarget,
	public TaskInterfaceSplit
{
public:
	typedef etl::handle<TaskContourSW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual void on_target_set_as_source() {
		Task::Handle &subtask = sub_task(0);
		if ( subtask
		  && subtask->target_surface == target_surface
		  && !Color::is_straight(blend_method) )
		{
			trunc_by_bounds();
			subtask->source_rect = source_rect;
			subtask->target_rect = target_rect;
		}
	}

	virtual Color::BlendMethodFlags get_supported_blend_methods() const
		{ return Color::BLEND_METHODS_ALL & ~Color::BLEND_METHODS_STRAIGHT; }

	virtual bool run(RunParams&) const {
		if (!is_valid())
			return true;
		if (!contour)
			return false;

		Vector ppu = get_pixels_per_unit();

		Matrix bounds_transfromation;
		bounds_transfromation.m00 = ppu[0];
		bounds_transfromation.m11 = ppu[1];
		bounds_transfromation.m20 = target_rect.minx - ppu[0]*source_rect.minx;
		bounds_transfromation.m21 = target_rect.miny - ppu[1]*source_rect.miny;

		Matrix matrix = bounds_transfromation * transformation->matrix;

		Polyspan polyspan;
		polyspan.init(target_rect);
		software::Contour::build_polyspan(contour->get_chunks(), matrix, polyspan, detail);
		polyspan.close();
		polyspan.sort_marks();

		LockWrite la(this);
		if (!la)
			return false;

		software::Contour::render_polyspan(
			la->get_surface(),
			polyspan,
			contour->invert,
			allow_antialias && contour->antialias,
			contour->winding_style,
			contour->color,
			blend ? amount : 1.0,
			blend ? blend_method : Color::BLEND_COMPOSITE );

		return true;
	}
};


Task::Token TaskContourSW::token(
	DescReal<TaskContourSW, TaskContour>("ContourSW") );

} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
