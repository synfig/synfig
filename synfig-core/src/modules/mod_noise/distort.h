/* === S Y N F I G ========================================================= */
/*!	\file distort.h
**	\brief Header file for implementation of the "Noise Distort" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef SYNFIG_NOISE_DISTORT_H
#define SYNFIG_NOISE_DISTORT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_composite_fork.h>
#include <synfig/rendering/common/task/taskdistort.h>
#include <synfig/rendering/common/task/tasktransformation.h>
#include "random_noise.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class NoiseDistort : public synfig::Layer_CompositeFork
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//!Parameter: (synfig::Vector)
	synfig::ValueBase param_displacement;
	//!Parameter: (synfig::Vector)
	synfig::ValueBase param_size;
	//!Parameter: (RandomNoise)
	synfig::ValueBase param_random;
	//!Parameter: (RandomNoise::SmoothType)
	synfig::ValueBase param_smooth;
	//!Parameter: (int)
	synfig::ValueBase param_detail;
	//!Parameter: (synfig::Real)
	synfig::ValueBase param_speed;
	//!Parameter: (bool)
	synfig::ValueBase param_turbulent;

	synfig::Color color_func(const synfig::Point &x,synfig::Context context)const;
	synfig::Point point_func(const synfig::Point &point)const;

public:
	NoiseDistort();

	bool set_param(const synfig::String &param, const synfig::ValueBase &value) override;
	synfig::ValueBase get_param(const synfig::String &param) const override;
	synfig::Color get_color(synfig::Context context, const synfig::Point &pos) const override;
	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point) const override;
	using Layer::get_bounding_rect;
	virtual synfig::Rect get_bounding_rect(synfig::Context context)const;
	Vocab get_param_vocab() const override;
	bool reads_context() const override { return true; }

protected:
	synfig::rendering::Task::Handle build_composite_fork_task_vfunc(synfig::ContextParams /* context_params */, synfig::rendering::Task::Handle sub_task) const override;

public:
	struct Internal
	{
		synfig::Vector displacement;
		synfig::Vector size;
		RandomNoise random;
		RandomNoise::SmoothType smooth;
		int detail;
		synfig::Real speed;
		bool turbulent;
		synfig::Time time_mark;

		synfig::Point transform(const synfig::Point& point) const;
	};
}; // EOF of class NoiseDistort

class TaskNoiseDistort
	: public synfig::rendering::TaskDistort, public synfig::rendering::TaskInterfaceTransformationGetAndPass
{
public:
	typedef etl::handle<TaskNoiseDistort> Handle;
	static Token token;
	Token::Handle get_token() const override;

	NoiseDistort::Internal internal;

	synfig::Rect compute_required_source_rect(const synfig::Rect& source_rect, const synfig::Matrix& /*vector_to_raster*/) const override;

	synfig::rendering::Transformation::Handle get_transformation() const override {
		return transformation.handle();
	}

protected:
	synfig::rendering::Holder<synfig::rendering::TransformationAffine> transformation;
};
/* === E N D =============================================================== */

#endif
