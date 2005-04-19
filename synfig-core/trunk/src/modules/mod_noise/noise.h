/* === S Y N F I G ========================================================= */
/*!	\file noise.h
**	\brief Template Header
**
**	$Id: noise.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_NOISE_H
#define __SYNFIG_NOISE_H

/* === H E A D E R S ======================================================= */

#include <synfig/vector.h>
#include <synfig/valuenode.h>

#include <synfig/layer_composite.h>
#include <synfig/gradient.h>
#include <synfig/time.h>
#include "random.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Noise : public synfig::Layer_Composite, public synfig::Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT

private:

	synfig::Vector size;

	Random random;
	int smooth;
	int detail;
	bool do_alpha;
	synfig::Gradient gradient;
	synfig::Real speed;
	bool turbulent;
	bool do_displacement;
	synfig::Vector displacement;
	
	//void sync();
	mutable synfig::Time curr_time;
	
	bool super_sample;

	synfig::Color color_func(const synfig::Point &x, float supersample,synfig::Context context)const;

	float calc_supersample(const synfig::Point &x, float pw,float ph)const;

public:
	Noise();
	
	virtual bool set_param(const synfig::String &param, const synfig::ValueBase &value);
	virtual synfig::ValueBase get_param(const synfig::String &param)const;
	virtual synfig::Color get_color(synfig::Context context, const synfig::Point &pos)const;
	virtual bool accelerated_render(synfig::Context context,synfig::Surface *surface,int quality, const synfig::RendDesc &renddesc, synfig::ProgressCallback *cb)const;
	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;	
	virtual void set_time(synfig::Context context, synfig::Time time)const;
	virtual void set_time(synfig::Context context, synfig::Time time, const synfig::Point &point)const;

	virtual Vocab get_param_vocab()const;
};

/* === E N D =============================================================== */

#endif
