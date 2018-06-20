/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/task/tasktransformationaffinesw.h
**	\brief TaskTransformationAffineSW Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_TASKTRANSFORMATIONAFFINESW_H
#define __SYNFIG_RENDERING_TASKTRANSFORMATIONAFFINESW_H

/* === H E A D E R S ======================================================= */

#include "tasksw.h"

#include "../surfaceswpacked.h"
#include "../../common/task/tasktransformation.h"
#include "../../common/task/taskblend.h"
#include "../../common/task/taskpixelprocessor.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig { class Surface; }

namespace synfig
{
namespace rendering
{

namespace software { class PackedSurface; }


class TaskTransformationAffineSW: public TaskTransformationAffine, public TaskSW,
	public TaskInterfaceBlendToTarget
{
private:
	class Helper;
public:
	typedef etl::handle<TaskTransformationAffineSW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual bool run(RunParams &params) const;

	virtual int get_target_subtask_index() const
		{ return 1; }
	virtual Color::BlendMethodFlags get_supported_blend_methods() const
		{ return Color::BLEND_METHODS_ALL; }

	static void resample(
		synfig::Surface &dest,
		const RectInt &dest_bounds,
		const synfig::Surface &src,
		const RectInt &src_bounds,
		const Matrix &transformation,
		ColorReal gamma,
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
		ColorReal gamma,
		Color::Interpolation interpolation,
		bool blend,
		ColorReal blend_amount,
		Color::BlendMethod blend_method );
};


} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
