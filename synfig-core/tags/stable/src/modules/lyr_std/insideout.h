/*! ========================================================================
** Sinfg
** Template Header File
** $Id: insideout.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SINFG_INSIDEOUT_H
#define __SINFG_INSIDEOUT_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer.h>
#include <sinfg/color.h>
#include <sinfg/context.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace sinfg;
using namespace std;
using namespace etl;
class InsideOut_Trans;

class InsideOut : public Layer
{
	SINFG_LAYER_MODULE_EXT
	friend class InsideOut_Trans;

private:

	Point origin;

public:
	InsideOut();
	
	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;	
	virtual Vocab get_param_vocab()const;	
	virtual etl::handle<sinfg::Transform> get_transform()const;
};

/* === E N D =============================================================== */

#endif
