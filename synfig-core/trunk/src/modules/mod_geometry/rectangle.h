/* === S Y N F I G ========================================================= */
/*!	\file rectangle.h
**	\brief Template Header
**
**	$Id: rectangle.h,v 1.2 2005/01/24 03:08:17 darco Exp $
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

#ifndef __SYNFIG_FILLEDRECT_H
#define __SYNFIG_FILLEDRECT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer_composite.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/value.h>
#include <list>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Rectangle : public synfig::Layer_Composite, public synfig::Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT
	
private:

	synfig::Color color;

	synfig::Point point1;
	synfig::Point point2;

	synfig::Real expand;

	bool invert;

public:
	
	Rectangle();
	
	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);

	virtual synfig::ValueBase get_param(const synfig::String & param)const;

	virtual synfig::Color get_color(synfig::Context context, const synfig::Point &pos)const;
	
	virtual bool accelerated_render(synfig::Context context,synfig::Surface *surface,int quality, const synfig::RendDesc &renddesc, synfig::ProgressCallback *cb)const;

	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;	

	virtual synfig::Rect get_bounding_rect()const;
	virtual synfig::Rect get_full_bounding_rect(synfig::Context context)const;

	virtual Vocab get_param_vocab()const;
}; // END of class FilledRect

/* === E N D =============================================================== */

#endif
