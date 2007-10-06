/* === S Y N F I G ========================================================= */
/*!	\file layer_composite.h
**	\brief Composite Layer Class Implementation
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_LAYER_COMPOSITE_H
#define __SYNFIG_LAYER_COMPOSITE_H

/* === H E A D E R S ======================================================= */

#include "layer.h"
#include "color.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

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

	virtual bool is_solid_color()const { return amount_==1.0f && blend_method_==Color::BLEND_STRAIGHT; }

	bool is_disabled()const { return amount_==0.0f; }

	virtual Vocab get_param_vocab()const;

	virtual bool set_param(const String &param, const ValueBase &value);

	virtual ValueBase get_param(const String &param)const;

	virtual Rect get_full_bounding_rect(Context context)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
}; // END of class Layer_Composite

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
