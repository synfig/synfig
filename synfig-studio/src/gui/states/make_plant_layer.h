/* === S Y N F I G ========================================================= */
/*!	\file make_plant_layer.h
**	\brief Make plant layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2016 caryoscelus
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

#ifndef __SYNFIG_STUDIO_MAKE_PLANT_LAYER_H
#define __SYNFIG_STUDIO_MAKE_PLANT_LAYER_H

/* === H E A D E R S ======================================================= */

#include "make_layer.h"
#include "state_shape.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
namespace studio {

class MakePlantLayer : public MakeLayer {
public:
	MakePlantLayer(StateShape_Context* context_) :
		MakeLayer(context_)
	{}

	virtual void make_layer(synfig::Canvas::Handle canvas,
		int depth,
		synfigapp::Action::PassiveGrouper& group,
		synfigapp::SelectionManager::LayerList& layer_selection,
		synfig::ValueNode_BLine::Handle value_node_bline,
		synfig::Vector& origin,
		synfig::ValueNode::Handle value_node_origin
	);
};

}; // END of namespace studio

/* === E N D =============================================================== */
#endif
