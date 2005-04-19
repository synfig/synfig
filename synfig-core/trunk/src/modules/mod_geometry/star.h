/*! ========================================================================
** Synfig
** Template Header File
** $Id: star.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SYNFIG_STAR_H
#define __SYNFIG_STAR_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer_polygon.h>
#include <list>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace std;
using namespace etl;

class Star : protected synfig::Layer_Polygon
{
	SYNFIG_LAYER_MODULE_EXT
private:

	Real radius1;
	Real radius2;
	int points;
	Angle angle;
public:
	Star();

	//! Updates the polygon data to match the parameters.
	void sync();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Vocab get_param_vocab()const;
};

/* === E N D =============================================================== */

#endif
