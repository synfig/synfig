/* === S Y N F I G ========================================================= */
/*!	\file colorcorrect.h
**	\brief Header file for implementation of the "Color Correct" layer
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

#ifndef __SYNFIG_LAYER_COLORCORRECT_H
#define __SYNFIG_LAYER_COLORCORRECT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/angle.h>
#include <synfig/gamma.h>
#include <synfig/rect.h>

#include <synfig/rendering/optimizer.h>
#include <synfig/rendering/common/task/taskpixelprocessor.h>
#include <synfig/rendering/common/task/tasksplittable.h>
#include <synfig/rendering/software/task/tasksw.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
namespace modules {
namespace mod_filter {

class TaskColorCorrect: public rendering::TaskPixelProcessor
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


class TaskColorCorrectSW: public TaskColorCorrect, public rendering::TaskSW, public rendering::TaskSplittable
{
private:
	void correct_pixel(Color &dst, const Color &src, const Angle &hue_djust, ColorReal shift, ColorReal amplifier, const Gamma &gamma) const;

public:
	typedef etl::handle<TaskColorCorrectSW> Handle;
	Task::Handle clone() const { return clone_pointer(this); }
	virtual void split(const RectInt &sub_target_rect);
	virtual bool run(RunParams &params) const;
};


class OptimizerColorCorrectSW: public rendering::Optimizer
{
public:
	OptimizerColorCorrectSW()
	{
		category_id = CATEGORY_ID_SPECIALIZE;
		depends_from = CATEGORY_COMMON & CATEGORY_PRE_SPECIALIZE;
		for_task = true;
	}

	virtual void run(const RunParams &params) const;
};


class Layer_ColorCorrect : public Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//! Parameter: (Angle)
	ValueBase param_hue_adjust;
	//! Parameter: (Real)
	ValueBase param_brightness;
	//! Parameter: (Real)
	ValueBase param_contrast;
	//! Parameter: (Real)
	ValueBase param_exposure;
	//! Parameter: (Real)
	ValueBase param_gamma;
	// This gamma member is kept to avoid need to recalculate the gamma table
	// on each pixel
	Gamma gamma;

	Color correct_color(const Color &in)const;

public:

	Layer_ColorCorrect();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual Rect get_full_bounding_rect(Context context)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context,cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual Vocab get_param_vocab()const;

	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context)const;
}; // END of class Layer_ColorCorrect

}; // END of namespace mod_filter
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
