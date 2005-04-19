/* === S Y N F I G ========================================================= */
/*!	\file blur.h
**	\brief Template Header
**
**	$Id: blur.h,v 1.2 2005/01/24 03:08:17 darco Exp $
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

/* === H E A D E R S ======================================================= */

#ifndef __SYNFIG_LAYER_BLUR_H__
#define __SYNFIG_LAYER_BLUR_H__

/* -- H E A D E R S --------------------------------------------------------- */

#include <synfig/layer_composite.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/blur.h>

using namespace synfig;
using namespace std;
using namespace etl;

class Blur_Layer : public synfig::Layer_Composite
{
	SYNFIG_LAYER_MODULE_EXT
private:
	synfig::Point 	size;
	int				type;

public:
	Blur_Layer();
	
	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual synfig::Rect get_full_bounding_rect(Context context)const;

	virtual Vocab get_param_vocab()const;
}; // END of class Blur

/* -- E X T E R N S --------------------------------------------------------- */

/* -- E N D ----------------------------------------------------------------- */

#endif
