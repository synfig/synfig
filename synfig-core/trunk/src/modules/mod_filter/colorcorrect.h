/* === S Y N F I G ========================================================= */
/*!	\file colorcorrect.h
**	\brief Template Header
**
**	$Id: colorcorrect.h,v 1.2 2005/01/24 05:00:18 darco Exp $
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

#ifndef __SYNFIG_LAYER_COLORCORRECT_H
#define __SYNFIG_LAYER_COLORCORRECT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/angle.h>
#include <synfig/gamma.h>
#include <synfig/rect.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Layer_ColorCorrect : public Layer
{
	SYNFIG_LAYER_MODULE_EXT
	
private:

	Angle hue_adjust;
	Real brightness;
	Real contrast;
	Real exposure;

	Gamma gamma;

	Color correct_color(const Color &in)const;

public:
	
	Layer_ColorCorrect();
		
	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual Rect get_full_bounding_rect(Context context)const;
	
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	
	virtual Vocab get_param_vocab()const;
}; // END of class Layer_ColorCorrect

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
