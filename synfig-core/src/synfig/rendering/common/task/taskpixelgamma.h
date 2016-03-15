/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskpixelgamma.h
**	\brief TaskPixelGamma Header
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

#ifndef __SYNFIG_RENDERING_TASKPIXELGAMMA_H
#define __SYNFIG_RENDERING_TASKPIXELGAMMA_H

/* === H E A D E R S ======================================================= */

#include "taskpixelprocessor.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskPixelGamma: public TaskPixelProcessor
{
public:
	typedef etl::handle<TaskPixelGamma> Handle;
	union {
		ColorReal gamma[4];
		struct {
			ColorReal gamma_r, gamma_g, gamma_b, gamma_a;
		};
	};
	TaskPixelGamma(): gamma_r(1.0), gamma_g(1.0), gamma_b(1.0), gamma_a(1.0) { }
	Task::Handle clone() const { return clone_pointer(this); }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
