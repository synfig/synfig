/*! ========================================================================
** Sinfg
** Template Header File
** $Id: radialblur.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_RADIALBLUR_H
#define __SINFG_RADIALBLUR_H

/* === H E A D E R S ======================================================= */

#include <sinfg/vector.h>
#include <sinfg/angle.h>
#include <sinfg/layer_composite.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

class RadialBlur : public Layer_Composite
{
	SINFG_LAYER_MODULE_EXT
	friend class RadialBlur_Trans;
private:
	Vector origin;
	Real size;
	bool fade_out;

public:
	RadialBlur();
	~RadialBlur();
	
	virtual bool set_param(const sinfg::String & param, const sinfg::ValueBase &value);
	virtual ValueBase get_param(const sinfg::String & param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual Vocab get_param_vocab()const;	
};

/* === E N D =============================================================== */

#endif
