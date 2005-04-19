/* === S Y N F I G ========================================================= */
/*!	\file simplecircle.h
**	\brief Template Header
**
**	$Id: simplecircle.h,v 1.1.1.1 2005/01/04 01:23:09 darco Exp $
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

#ifndef __SYNFIG_SIMPLECIRCLE_H
#define __SYNFIG_SIMPLECIRCLE_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer_composite.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/layer_composite.h>
#include <synfig/value.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class SimpleCircle : public synfig::Layer_Composite
{
	SYNFIG_LAYER_MODULE_EXT
	
private:

	synfig::Color color;

	synfig::Point center;

	synfig::Real radius;

public:
	
	SimpleCircle();
	
	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);

	virtual synfig::ValueBase get_param(const synfig::String & param)const;

	virtual synfig::Color get_color(synfig::Context context, const synfig::Point &pos)const;
	
	//virtual bool accelerated_render(synfig::Context context,synfig::Surface *surface,int quality, const synfig::RendDesc &renddesc, synfig::ProgressCallback *cb)const;
	
	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;	

	virtual Vocab get_param_vocab()const;
}; // END of class SimpleCircle

/* === E N D =============================================================== */

#endif
