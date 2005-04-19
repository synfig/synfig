/* === S I N F G =========================================================== */
/*!	\file layer_composite.h
**	\brief Composite Layer Class Implementation
**
**	$Id: layer_composite.h,v 1.2 2005/01/24 03:08:18 darco Exp $
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

#ifndef __SINFG_LAYER_COMPOSITE_H
#define __SINFG_LAYER_COMPOSITE_H

/* === H E A D E R S ======================================================= */

#include "layer.h"
#include "color.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

class Layer_NoDeform {};


/*!	\class Layer_Composite
**	\brief Base class for layers that put stuff ontop of lower layers
*/
class Layer_Composite : public Layer
{
private:

	float amount_;

	Color::BlendMethod blend_method_;

protected:

	Layer_Composite(
		float 	amount=1.0,
		Color::BlendMethod 	blend_method=Color::BLEND_COMPOSITE
	):
		amount_				(amount),
		blend_method_		(blend_method)
	{ }

public:

	float get_amount()const { return amount_; }

	Layer_Composite& set_amount(float x) { amount_=x; return *this; }

	Color::BlendMethod get_blend_method()const { return blend_method_; }

	Layer_Composite& set_blend_method(Color::BlendMethod x) { blend_method_=x; return *this; }

	bool is_solid_color()const { return amount_==1.0f && blend_method_==Color::BLEND_STRAIGHT; }
	
	bool is_disabled()const { return amount_==0.0f; }
	
	virtual Vocab get_param_vocab()const;

	virtual bool set_param(const String &param, const ValueBase &value);

	virtual ValueBase get_param(const String &param)const;

	virtual Rect get_full_bounding_rect(Context context)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
}; // END of class Layer_Composite

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
