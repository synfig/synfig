/* === S I N F G =========================================================== */
/*!	\file colorcorrect.h
**	\brief Template Header
**
**	$Id: stretch.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SINFG_LAYER_STRETCH_H
#define __SINFG_LAYER_STRETCH_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer.h>
#include <sinfg/vector.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Stretch_Trans;
namespace sinfg {

class Layer_Stretch : public Layer
{
	SINFG_LAYER_MODULE_EXT
	friend class Stretch_Trans;
	
private:

	Vector amount;
	Point center;

public:
	
	Layer_Stretch();
		
	virtual bool set_param(const String & param, const sinfg::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;
	
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;	
	
	virtual Vocab get_param_vocab()const;
	virtual etl::handle<sinfg::Transform> get_transform()const;
}; // END of class Layer_Stretch

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
