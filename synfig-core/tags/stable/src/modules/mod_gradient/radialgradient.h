/* === S I N F G =========================================================== */
/*!	\file radialgradient.h
**	\brief Template Header
**
**	$Id: radialgradient.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SINFG_RADIALGRADIENT_H
#define __SINFG_RADIALGRADIENT_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer_composite.h>
#include <sinfg/color.h>
#include <sinfg/vector.h>
#include <sinfg/value.h>
#include <sinfg/gradient.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class RadialGradient : public sinfg::Layer_Composite, public sinfg::Layer_NoDeform
{
	SINFG_LAYER_MODULE_EXT
	
private:

	sinfg::Gradient gradient;

	sinfg::Point center;

	sinfg::Real radius;

	bool loop;
	bool zigzag;

	sinfg::Color color_func(const sinfg::Point &x, float supersample=0)const;

	float calc_supersample(const sinfg::Point &x, float pw,float ph)const;

public:
	
	RadialGradient();
	
	virtual bool set_param(const sinfg::String & param, const sinfg::ValueBase &value);

	virtual sinfg::ValueBase get_param(const sinfg::String & param)const;

	virtual sinfg::Color get_color(sinfg::Context context, const sinfg::Point &pos)const;
	
	virtual bool accelerated_render(sinfg::Context context,sinfg::Surface *surface,int quality, const sinfg::RendDesc &renddesc, sinfg::ProgressCallback *cb)const;
	sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;	
	
	virtual Vocab get_param_vocab()const;
}; // END of class RadialGradient

/* === E N D =============================================================== */

#endif
