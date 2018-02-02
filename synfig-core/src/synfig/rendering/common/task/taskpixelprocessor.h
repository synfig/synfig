/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskpixelprocessor.h
**	\brief TaskPixelProcessor Header
**
**	$Id$
**
**	\legal
**	......... ... 2016-2018 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_TASKPIXELPROCESSOR_H
#define __SYNFIG_RENDERING_TASKPIXELPROCESSOR_H

/* === H E A D E R S ======================================================= */

#include <synfig/color/colormatrix.h>

#include "../../task.h"
#include "tasktransformation.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{


class TaskPixelProcessor: public Task,
	public TaskInterfaceTransformationPass,
	public TaskInterfaceConstant,
	public TaskInterfaceSplit
{
public:
	typedef etl::handle<TaskPixelProcessor> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token; }

	const Task::Handle& sub_task() const { return Task::sub_task(0); }
	Task::Handle& sub_task() { return Task::sub_task(0); }

	VectorInt get_offset() const;

	virtual Rect calc_bounds() const;

	virtual bool is_transparent() const
		{ return false; }
	virtual bool is_constant() const
		{ return false; }
	virtual bool is_affects_transparent() const
		{ return false; }
};


class TaskPixelGamma: public TaskPixelProcessor
{
public:
	typedef etl::handle<TaskPixelGamma> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token; }

	union {
		ColorReal gamma[4];
		struct { ColorReal gamma_r, gamma_g, gamma_b, gamma_a; };
	};
	TaskPixelGamma(): gamma_r(1.0), gamma_g(1.0), gamma_b(1.0), gamma_a(1.0) { }
};


class TaskPixelColorMatrix: public TaskPixelProcessor
{
public:
	typedef etl::handle<TaskPixelColorMatrix> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token; }

	ColorMatrix matrix;

	virtual bool is_transparent() const
		{ return matrix.is_transparent(); }
	virtual bool is_constant() const
		{ return matrix.is_constant(); }
	virtual bool is_affects_transparent() const
		{ return matrix.is_affects_transparent(); }
};


} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
