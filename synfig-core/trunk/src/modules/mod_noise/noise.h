/* === S I N F G =========================================================== */
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

#ifndef __SINFG_NOISE_H
#define __SINFG_NOISE_H

/* === H E A D E R S ======================================================= */

#include <sinfg/vector.h>
#include <sinfg/valuenode.h>

#include <sinfg/layer_composite.h>
#include <sinfg/gradient.h>
#include <sinfg/time.h>
#include "random.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Noise : public sinfg::Layer_Composite, public sinfg::Layer_NoDeform
{
	SINFG_LAYER_MODULE_EXT

private:

	sinfg::Vector size;

	Random random;
	int smooth;
	int detail;
	bool do_alpha;
	sinfg::Gradient gradient;
	sinfg::Real speed;
	bool turbulent;
	bool do_displacement;
	sinfg::Vector displacement;
	
	//void sync();
	mutable sinfg::Time curr_time;
	
	bool super_sample;

	sinfg::Color color_func(const sinfg::Point &x, float supersample,sinfg::Context context)const;

	float calc_supersample(const sinfg::Point &x, float pw,float ph)const;

public:
	Noise();
	
	virtual bool set_param(const sinfg::String &param, const sinfg::ValueBase &value);
	virtual sinfg::ValueBase get_param(const sinfg::String &param)const;
	virtual sinfg::Color get_color(sinfg::Context context, const sinfg::Point &pos)const;
	virtual bool accelerated_render(sinfg::Context context,sinfg::Surface *surface,int quality, const sinfg::RendDesc &renddesc, sinfg::ProgressCallback *cb)const;
	sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;	
	virtual void set_time(sinfg::Context context, sinfg::Time time)const;
	virtual void set_time(sinfg::Context context, sinfg::Time time, const sinfg::Point &point)const;

	virtual Vocab get_param_vocab()const;
};

/* === E N D =============================================================== */

#endif
