/* === S I N F G =========================================================== */
/*!	\file shade.h
**	\brief Template Header
**
**	$Id: bevel.h,v 1.2 2005/01/24 03:08:17 darco Exp $
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

#ifndef __SINFG_LAYER_BEVEL_H__
#define __SINFG_LAYER_BEVEL_H__

/* -- H E A D E R S --------------------------------------------------------- */

#include <sinfg/layer_composite.h>
#include <sinfg/color.h>
#include <sinfg/vector.h>
#include <sinfg/blur.h>
#include <sinfg/angle.h>

using namespace sinfg;
using namespace std;
using namespace etl;

class Layer_Bevel : public sinfg::Layer_Composite
{
	SINFG_LAYER_MODULE_EXT
private:
	sinfg::Real 	softness;
	int				type;

	sinfg::Color	color1;
	sinfg::Color	color2;
	
	sinfg::Angle	angle;
	sinfg::Real		depth;

	sinfg::Vector	offset;
	sinfg::Vector	offset45;
	
	bool use_luma;
	bool solid;

	void calc_offset();
public:
	Layer_Bevel();
	
	virtual bool set_param(const String & param, const sinfg::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual sinfg::Rect get_full_bounding_rect(Context context)const;
	virtual Vocab get_param_vocab()const;
}; // END of class Blur

/* -- E X T E R N S --------------------------------------------------------- */

/* -- E N D ----------------------------------------------------------------- */

#endif
