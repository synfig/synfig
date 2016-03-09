/* === S Y N F I G ========================================================= */
/*!	\file filledrect.h
**	\brief Header file for implementation of the "Rectangle" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_FILLEDRECT_H
#define __SYNFIG_FILLEDRECT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_polygon.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace std;
using namespace etl;

class FilledRect : public Layer_Polygon
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//! Parameter: (Point)
	ValueBase param_point1;
	//! Parameter: (Point)
	ValueBase param_point2;
	//! Parameter: Real)
	ValueBase param_feather_x;
	//! Parameter: (Real)
	ValueBase param_feather_y;
	//! Parameter: (Real)
	ValueBase param_bevel;
	//! Parameter: (bool)
	ValueBase param_bevCircle;

protected:
	virtual void sync_vfunc();

public:
	FilledRect();

	virtual bool set_param(const String & param, const ValueBase &value);
	virtual ValueBase get_param(const String & param)const;
	virtual Vocab get_param_vocab()const;
}; // END of class FilledRect

/* === E N D =============================================================== */

#endif
