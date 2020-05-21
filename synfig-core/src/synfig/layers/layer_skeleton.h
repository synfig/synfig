/* === S Y N F I G ========================================================= */
/*!	\file layer_skeleton.h
**	\brief Header file for implementation of the "Layer_Skeleton" layer
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

#ifndef __SYNFIG_LAYER_SKELETON_H
#define __SYNFIG_LAYER_SKELETON_H

/* === H E A D E R S ======================================================= */

#include <synfig/bone.h>
#include "layer_polygon.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Layer_Skeleton : public Layer_Polygon
{
	SYNFIG_LAYER_MODULE_EXT
private:

	//!Parameter: (std::vector<synfig::Bone>) Bones list of the skeleton
	ValueBase param_bones;
	//!Parameter: (synfig::String) Name of the skeleton
	ValueBase param_name;

public:

	Layer_Skeleton();

#ifdef _DEBUG
	~Layer_Skeleton();
#endif

	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);
	virtual synfig::ValueBase get_param(const synfig::String & param)const;

	virtual Vocab get_param_vocab()const;

protected:
	virtual void sync_vfunc();
}; // END of class Layer_Skeleton

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
