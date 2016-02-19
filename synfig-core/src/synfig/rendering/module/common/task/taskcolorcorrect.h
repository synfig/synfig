/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/module/common/task/taskcolorcorrect.h
**	\brief TaskColorCorrect Header
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_MODULE_TASKCOLORCORRECT_H
#define __SYNFIG_RENDERING_MODULE_TASKCOLORCORRECT_H

/* === H E A D E R S ======================================================= */

#include <synfig/rendering/common/task/taskpixelprocessor.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskColorCorrect: public TaskPixelProcessor
{
public:
	typedef etl::handle<TaskColorCorrect> Handle;

	Angle hue_adjust;
	Real brightness;
	Real contrast;
	Real exposure;
	Real gamma;

	TaskColorCorrect():
		hue_adjust(Angle::zero()),
		brightness(0.0),
		contrast(1.0),
		exposure(0.0),
		gamma(1.0) { }
	Task::Handle clone() const { return clone_pointer(this); }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
