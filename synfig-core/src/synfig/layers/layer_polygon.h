/* === S Y N F I G ========================================================= */
/*!	\file layer_polygon.h
**	\brief Header file for implementation of the "Polygon" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_LAYER_POLYGON_H
#define __SYNFIG_LAYER_POLYGON_H

/* === H E A D E R S ======================================================= */

#include "layer_shape.h"
#include <synfig/color.h>
#include <synfig/vector.h>
#include <list>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Layer_Polygon
**	\brief writeme
**	\todo This layer needs to support multiple polygons */
class Layer_Polygon : public Layer_Shape
{
	SYNFIG_LAYER_MODULE_EXT

private:

	//! Parameter: (std::vector<Point>)
	ValueBase param_vector_list;

protected:

	Layer_Polygon();

public:

	~Layer_Polygon();

protected:
	//! Adds a polygon to the layer
	/*!	The edge data is automatically added to the
	**	EdgeTable, so there is no need to call sync()
	**	after adding a polygon using this function.
	**	\param point_list A list containing the
	**		points that define the polygon's parameter.
	*/
	void add_polygon(const std::vector<Point> &point_list);
	
	// Places the point_list on the vector_list, for later render as polygon.
	void set_stored_polygon(const std::vector<Point> &point_list);

	//! Clears out any polygon data
	void clear_stored_polygon();

public:
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	virtual ValueBase get_param(const String & param)const;
	virtual Vocab get_param_vocab()const;
}; // END of Layer_Polygon

}; // END of namespace synfig
/* === E N D =============================================================== */

#endif
