/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/task/tasksurfaceresamplesw.h
**	\brief TaskSurfaceResampleSW Header
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

#ifndef __SYNFIG_RENDERING_TASKSURFACERESAMPLESW_H
#define __SYNFIG_RENDERING_TASKSURFACERESAMPLESW_H

/* === H E A D E R S ======================================================= */

#include "tasksw.h"

#include "../surfaceswpacked.h"
#include "../../common/task/tasksurfaceresample.h"
#include "../../common/task/taskcomposite.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
	class Surface;
}

namespace synfig
{
namespace rendering
{

namespace software
{
	class PackedSurface;
}

class TaskSurfaceResampleSW: public TaskSurfaceResample, public TaskComposite, public TaskSW
{
public:
	typedef etl::handle<TaskSurfaceResampleSW> Handle;
	Task::Handle clone() const { return clone_pointer(this); }
	virtual bool run(RunParams &params) const;
	virtual bool is_supported_source(const Surface::Handle &surface)
		{ return TaskSW::is_supported_source(surface) || surface.type_is<SurfaceSWPacked>(); }

	static void resample(
		synfig::Surface &dest,
		const RectInt &dest_bounds,
		const synfig::Surface &src,
		const RectInt &src_bounds,
		const Matrix &transformation,
		ColorReal gamma,
		Color::Interpolation interpolation,
		bool antialiasing,
		bool blend,
		ColorReal blend_amount,
		Color::BlendMethod blend_method );

	static void resample(
		synfig::Surface &dest,
		const RectInt &dest_bounds,
		const software::PackedSurface &src,
		const RectInt &src_bounds,
		const Matrix &transformation,
		ColorReal gamma,
		Color::Interpolation interpolation,
		bool antialiasing,
		bool blend,
		ColorReal blend_amount,
		Color::BlendMethod blend_method );
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
