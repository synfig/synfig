/* === S Y N F I G ========================================================= */
/*!	\file clamp.h
**	\brief Header file for implementation of the "Clamp" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_LAYER_CLAMP_H
#define __SYNFIG_LAYER_CLAMP_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>

#include <synfig/rendering/optimizer.h>
#include <synfig/rendering/common/task/taskpixelprocessor.h>
#include <synfig/rendering/software/task/tasksw.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace modules
{
namespace lyr_std
{


class TaskClamp: public rendering::TaskPixelProcessor
{
public:
	typedef etl::handle<TaskClamp> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	bool invert_negative;
	bool clamp_floor;
	bool clamp_ceiling;
	Real floor;
	Real ceiling;

	bool is_transparent() const
		{ return !invert_negative && !clamp_floor && !clamp_ceiling; }
	bool is_constant() const
		{ return clamp_floor && clamp_ceiling && !approximate_less(floor, ceiling); }

	TaskClamp():
		invert_negative(false),
		clamp_floor(true),
		clamp_ceiling(true),
		floor(0.0),
		ceiling(1.0) { }
};


class TaskClampSW: public TaskClamp, public rendering::TaskSW
{
public:
	typedef etl::handle<TaskClampSW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual bool run(RunParams &params) const;
private:
	void clamp_pixel(Color &dst, const Color &src) const;
};


class Layer_Clamp : public Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//!Parameter: (bool)
	ValueBase param_invert_negative;
	//!Parameter: (bool)
	ValueBase param_clamp_ceiling;
	//!Parameter: (Real)
	ValueBase param_ceiling;
	//!Parameter: (Real)
	ValueBase param_floor;

	Color clamp_color(const Color &in)const;

public:

	Layer_Clamp();

	virtual bool set_param(const String & param, const ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual Rect get_full_bounding_rect(Context context)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual Vocab get_param_vocab()const;

	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context)const;
}; // END of class Layer_Clamp

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
