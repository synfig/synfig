/* === S I N F G =========================================================== */
/*!	\file layer_bitmap.h
**	\brief Template Header
**
**	$Id: layer_bitmap.h,v 1.2 2005/01/24 03:08:18 darco Exp $
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

#ifndef __SINFG_LAYER_BITMAP_H
#define __SINFG_LAYER_BITMAP_H

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include "surface.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

/*!	\class Layer_Bitmap
**	\todo writeme
*/
class Layer_Bitmap : public Layer_Composite, public Layer_NoDeform
{
	const Color& filter(const Color& c)const;
public:
	typedef etl::handle<Layer_Bitmap> Handle;	

	Point tl;
	Point br;
	int c;
	mutable Surface surface;
	
	Real gamma_adjust;
	
	Layer_Bitmap();
	
	virtual bool set_param(const String & param, ValueBase value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;
	
	virtual Vocab get_param_vocab()const;

	virtual Rect get_bounding_rect()const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &rend_desc, ProgressCallback *callback)const;

	virtual sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;
}; // END of class Layer_Bitmap

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
