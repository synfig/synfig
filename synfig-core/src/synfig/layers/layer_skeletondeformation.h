/* === S Y N F I G ========================================================= */
/*!	\file layer_skeletondeformation.h
**	\brief SkeletonDeformation layer
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

#ifndef __SYNFIG_LAYER_SKELETONDEFORMATION_H
#define __SYNFIG_LAYER_SKELETONDEFORMATION_H

/* === H E A D E R S ======================================================= */

#include "layer_meshtransform.h"
#include <synfig/pair.h>
#include <synfig/bone.h>
#include <synfig/polygon.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
/*!	\class Layer_SkeletonDeformation
**	\brief Class of the SkeletonDeformation layer.
*/
class Layer_SkeletonDeformation : public Layer_MeshTransform
{
	//! Layer module: defines the needed members to belong to a layer's factory.
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter: (list) Bones
	ValueBase param_bones;
	//! Parameter: (Point)
	synfig::ValueBase param_point1;
	//! Parameter: (Point)
	synfig::ValueBase param_point2;
	//! Parameter: (Integer)
	synfig::ValueBase param_x_subdivisions;
	//! Parameter: (Integer)
	synfig::ValueBase param_y_subdivisions;

	struct GridPoint;
	static Real distance_to_line(const Vector &p0, const Vector &p1, const Vector &x);

public:
	typedef std::pair<Bone, Bone> BonePair;

	//! Default constructor
	Layer_SkeletonDeformation();
	//! Destructor
	virtual ~Layer_SkeletonDeformation();
	//! Returns a string with the localized name of this layer
	virtual String get_local_name()const;

	//!	Sets the parameter described by \a param to \a value. \see Layer::set_param
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	//! Get the value of the specified parameter. \see Layer::get_param
	virtual ValueBase get_param(const String & param)const;
	//! Gets the parameter vocabulary
	virtual Vocab get_param_vocab()const;

	void prepare_mask();
	void prepare_mesh();
}; // END of class SkeletonDeformation

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
