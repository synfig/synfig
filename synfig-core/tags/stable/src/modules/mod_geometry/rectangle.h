/* === S I N F G =========================================================== */
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

#ifndef __SINFG_FILLEDRECT_H
#define __SINFG_FILLEDRECT_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer_composite.h>
#include <sinfg/color.h>
#include <sinfg/vector.h>
#include <sinfg/value.h>
#include <list>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Rectangle : public sinfg::Layer_Composite, public sinfg::Layer_NoDeform
{
	SINFG_LAYER_MODULE_EXT
	
private:

	sinfg::Color color;

	sinfg::Point point1;
	sinfg::Point point2;

	sinfg::Real expand;

	bool invert;

public:
	
	Rectangle();
	
	virtual bool set_param(const sinfg::String & param, const sinfg::ValueBase &value);

	virtual sinfg::ValueBase get_param(const sinfg::String & param)const;

	virtual sinfg::Color get_color(sinfg::Context context, const sinfg::Point &pos)const;
	
	virtual bool accelerated_render(sinfg::Context context,sinfg::Surface *surface,int quality, const sinfg::RendDesc &renddesc, sinfg::ProgressCallback *cb)const;

	sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;	

	virtual sinfg::Rect get_bounding_rect()const;
	virtual sinfg::Rect get_full_bounding_rect(sinfg::Context context)const;

	virtual Vocab get_param_vocab()const;
}; // END of class FilledRect

/* === E N D =============================================================== */

#endif
