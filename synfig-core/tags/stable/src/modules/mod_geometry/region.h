/*! ========================================================================
** Sinfg
** Template Header File
** $Id: region.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SINFG_REGION_H
#define __SINFG_REGION_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer_polygon.h>
#include <list>
#include <vector>
#include <sinfg/value.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg { class Segment; }

using namespace sinfg;
using namespace std;
using namespace etl;

class Region : protected sinfg::Layer_Polygon//Shape
{
	SINFG_LAYER_MODULE_EXT
private:
	sinfg::ValueBase bline;
	std::vector<sinfg::Segment> segment_list;
public:
	Region();

	//! Updates the polygon data to match the parameters.
	void sync();

	virtual bool set_param(const String & param, const sinfg::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Vocab get_param_vocab()const;
	virtual void set_time(Context context, Time time)const;
	virtual void set_time(Context context, Time time, Vector pos)const;

};

/* === E N D =============================================================== */

#endif
