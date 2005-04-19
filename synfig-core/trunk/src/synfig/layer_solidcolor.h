/* === S Y N F I G ========================================================= */
/*!	\file layer_solidcolor.h
**	\brief Template Header
**
**	$Id: layer_solidcolor.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SYNFIG_LAYER_SOLIDCOLOR_H
#define __SYNFIG_LAYER_SOLIDCOLOR_H

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include "color.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Layer_SolidColor : public Layer_Composite, public Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT
	
private:

	Color color;

public:
	
	Layer_SolidColor();
	
	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;
	
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	
	virtual Vocab get_param_vocab()const;

	virtual synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;

}; // END of class Layer_SolidColor

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
