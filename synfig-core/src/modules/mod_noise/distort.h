/* === S Y N F I G ========================================================= */
/*!	\file distort.h
**	\brief Header file for implementation of the "Noise Distort" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#ifndef __SYNFIG_NOISE_DISTORT_H
#define __SYNFIG_NOISE_DISTORT_H

/* === H E A D E R S ======================================================= */

#include <synfig/vector.h>
#include <synfig/valuenode.h>
#include <synfig/layers/layer_composite.h>
#include <synfig/gradient.h>
#include <synfig/time.h>
#include "random_noise.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class NoiseDistort : public synfig::Layer_Composite
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

	//void sync();
	mutable synfig::Time curr_time;

	synfig::Color color_func(const synfig::Point &x, float supersample,synfig::Context context)const;
	synfig::CairoColor cairocolor_func(const synfig::Point &x, float supersample,synfig::Context context)const;
	synfig::Point point_func(const synfig::Point &point)const;

	float calc_supersample(const synfig::Point &x, float pw,float ph)const;

public:
	NoiseDistort();

	virtual bool set_param(const synfig::String &param, const synfig::ValueBase &value);
	virtual synfig::ValueBase get_param(const synfig::String &param)const;
	virtual synfig::Color get_color(synfig::Context context, const synfig::Point &pos)const;
	virtual synfig::CairoColor get_cairocolor(synfig::Context context, const synfig::Point &pos)const;
	//virtual bool accelerated_render(synfig::Context context,synfig::Surface *surface,int quality, const synfig::RendDesc &renddesc, synfig::ProgressCallback *cb)const;
	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
	virtual void set_time(synfig::IndependentContext context, synfig::Time time)const;
	virtual void set_time(synfig::IndependentContext context, synfig::Time time, const synfig::Point &point)const;
	using Layer::get_bounding_rect;
	virtual synfig::Rect get_bounding_rect(synfig::Context context)const;
	virtual Vocab get_param_vocab()const;
	virtual bool reads_context()const { return true; }
}; // EOF of class NoiseDistort

/* === E N D =============================================================== */

#endif
