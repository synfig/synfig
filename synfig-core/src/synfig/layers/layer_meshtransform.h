/* === S Y N F I G ========================================================= */
/*!	\file layer_meshtransform.h
**	\brief Header file for implementation of the "MeshTransform" layer
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#ifndef __SYNFIG_LAYER_MESHTRANSFORM_H
#define __SYNFIG_LAYER_MESHTRANSFORM_H

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include <synfig/mesh.h>
#include <synfig/polygon.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
class Mesh_Trans;
class Layer_MeshTransform : public Layer_Composite
{
protected:
	friend class Mesh_Trans;
	Mesh mesh;
	Polygon mask;

	int max_texture_size;
	Real max_texture_scale;

private:
	Vector texture_scale_dependency_from_x;
	Vector texture_scale_dependency_from_y;
	Rect world_bounds;
	Rect texture_bounds;

protected:
	void update_mesh_and_mask();

public:
	//! Default constructor
	Layer_MeshTransform();
	//! Destructor
	virtual ~Layer_MeshTransform();

	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual Rect get_full_bounding_rect(Context context)const;
	virtual etl::handle<synfig::Transform> get_transform()const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
}; // END of class Layer_MeshTransform

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
