/* === S Y N F I G ========================================================= */
/*!	\file layer_group.h
**	\brief Header file for implementation of the "Group" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
**	......... ... 2014 Ivan Mahonin
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

#ifndef __SYNFIG_LAYER_GROUP_H
#define __SYNFIG_LAYER_GROUP_H

/* === H E A D E R S ======================================================= */

#include "layer_pastecanvas.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
/*!	\class Layer_Group
**	\brief Class of the Group layer.
*/
class Layer_Group : public Layer_PasteCanvas
{
	//! Layer module: defines the needed members to belong to a layer's factory.
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter: (bool) Z_Depth Range is active
	ValueBase param_z_range;
	//! Parameter: (Real) Z_Depth Range position
	ValueBase param_z_range_position;
	//! Parameter: (Real) Z_Depth Range depth
	ValueBase param_z_range_depth;
	//! Parameter: (Real) Z_Depth Range transition
	ValueBase param_z_range_blur;

public:
	//! Default constructor
	Layer_Group();
	//! Destructor
	virtual ~Layer_Group();
	//! Returns a string with the localized name of this layer
	virtual String get_local_name()const;

	//!	Sets the parameter described by \a param to \a value. \see Layer::set_param
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	//! Get the value of the specified parameter. \see Layer::get_param
	virtual ValueBase get_param(const String & param)const;
	//! Gets the parameter vocabulary
	virtual Vocab get_param_vocab()const;

	//! Sets z_range* fields of specified ContextParams \a cp
	virtual void apply_z_range_to_params(ContextParams &cp)const;
}; // END of class Layer_Group

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
