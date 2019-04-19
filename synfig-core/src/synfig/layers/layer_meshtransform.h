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

#include "layer_composite_fork.h"

#include <synfig/polygon.h>

#include <synfig/rendering/primitive/mesh.h>
#include <synfig/rendering/primitive/contour.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
class Layer_MeshTransform : public Layer_CompositeFork
{
protected:
	rendering::Mesh::Handle mesh;
	rendering::Contour::Handle mask;

public:
	//! Default constructor
	explicit Layer_MeshTransform(Real amount=1.0, Color::BlendMethod blend_method=Color::BLEND_COMPOSITE);
	//! Destructor
	virtual ~Layer_MeshTransform();

	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual Rect get_bounding_rect()const;
	virtual etl::handle<synfig::Transform> get_transform()const;

protected:
	virtual rendering::Task::Handle build_composite_fork_task_vfunc(ContextParams context_params, rendering::Task::Handle sub_task)const;
}; // END of class Layer_MeshTransform

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
