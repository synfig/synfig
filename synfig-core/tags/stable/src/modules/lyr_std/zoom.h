/* === S I N F G =========================================================== */
/*!	\file zoom.h
**	\brief writeme
**
**	$Id: zoom.h,v 1.2 2005/01/24 03:08:17 darco Exp $
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

#ifndef __SINFG_ZOOM_H
#define __SINFG_ZOOM_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer.h>
#include <sinfg/vector.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

class Zoom_Trans;

class Zoom : public Layer
{
	SINFG_LAYER_MODULE_EXT
	friend class Zoom_Trans;
private:
	Vector center;
	Real amount;
public:
	Zoom();
	
	virtual bool set_param(const String & param, const sinfg::ValueBase &value);
	virtual ValueBase get_param(const String & param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;	
	virtual Vocab get_param_vocab()const;
	virtual sinfg::Rect get_full_bounding_rect(sinfg::Context context)const;
	virtual etl::handle<sinfg::Transform> get_transform()const;

};

/* === E N D =============================================================== */

#endif
