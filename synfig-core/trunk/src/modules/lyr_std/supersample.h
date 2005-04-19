/*! ========================================================================
** Synfig
** Template Header File
** $Id: supersample.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SYNFIG_SUPERSAMPLE_H
#define __SYNFIG_SUPERSAMPLE_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace std;
using namespace etl;

class SuperSample : public synfig::Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:
	int width, height;
	bool scanline,alpha_aware;
public:
	SuperSample();
	
	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual synfig::Rect get_bounding_rect(Context context)const;

	virtual Vocab get_param_vocab()const;
};

/* === E N D =============================================================== */

#endif
