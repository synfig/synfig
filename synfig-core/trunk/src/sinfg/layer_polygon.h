/* === S I N F G =========================================================== */
/*!	\file layer_polygon.h
**	\brief Template Header
**
**	$Id: layer_polygon.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SINFG_LAYER_POLYGON_H
#define __SINFG_LAYER_POLYGON_H

/* === H E A D E R S ======================================================= */

#include "layer_shape.h"
#include "color.h"
#include "vector.h"
#include <list>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {
	
/*!	\class Layer_Polygon
**	\beief writeme
**	\todo This layer needs to support multiple polygons */
class Layer_Polygon : public Layer_Shape
{
	SINFG_LAYER_MODULE_EXT
	
private:
		
	//exported data
	std::vector< Point > 	vector_list;

protected:

	Layer_Polygon();

public:

	~Layer_Polygon();

	//! Adds a polygon to the layer
	/*!	The edge data is automaticly added to the
	**	EdgeTable, so there is no need to call sync()
	**	after adding a polygon using this function.
	**	\param point_list A list containing the
	**		points that define the polygon's parameter.
	*/
	void add_polygon(const std::vector<Point> &point_list);

	//! Clears out any polygon data
	/*!	Also clears out the EdgeTable, so there is no
	**	need to call sync() after using this function.
	*/
	void clear();

	//! Updates EdgeTable so it will reflect the parameter data
	void sync();
	
	virtual bool set_param(const String & param, const sinfg::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;
	
	virtual Vocab get_param_vocab()const;

private:
	class 		PolySpan;
	bool render_polyspan(Surface *surface,PolySpan &polyspan)const;
}; // END of Layer_Polygon

}; // END of namespace sinfg
/* === E N D =============================================================== */

#endif
