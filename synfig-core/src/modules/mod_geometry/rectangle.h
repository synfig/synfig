/* === S Y N F I G ========================================================= */
/*!	\file rectangle.h
**	\brief Header file for implementation of the "Rectangle" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_RECTANGLE_H
#define __SYNFIG_RECTANGLE_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_polygon.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Rectangle : public synfig::Layer_Polygon
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//! Parameter: (Point)
	synfig::ValueBase param_point1;
	//! Parameter: (Point)
	synfig::ValueBase param_point2;
	//! Parameter: (Real)
	synfig::ValueBase param_expand;
	//! Parameter: (Real)
	synfig::ValueBase param_feather_x;
	//! Parameter: (Real)
	synfig::ValueBase param_feather_y;
	//! Parameter: (Real)
	synfig::ValueBase param_bevel;
	//! Parameter: (bool)
	synfig::ValueBase param_bevCircle;

protected:
	virtual void sync_vfunc();

public:
	Rectangle();

	virtual bool set_shape_param(const synfig::String & param, const synfig::ValueBase &value);
	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);
	virtual synfig::ValueBase get_param(const synfig::String & param)const;
	virtual Vocab get_param_vocab()const;
}; // END of class Rectangle

/* === E N D =============================================================== */

#endif
