/* === S Y N F I G ========================================================= */
/*!	\file simplecircle.h
**	\brief Header file for implementation of the "Simple Circle" layer
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

#ifndef __SYNFIG_SIMPLECIRCLE_H
#define __SYNFIG_SIMPLECIRCLE_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_shape.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace etl;

class SimpleCircle : public Layer_Shape
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter: (Real)
	ValueBase param_radius;

protected:
	virtual void sync_vfunc();

public:
	SimpleCircle();

	virtual bool set_shape_param(const synfig::String & param, const synfig::ValueBase &value);
	virtual bool set_param(const String & param, const ValueBase &value);
	virtual ValueBase get_param(const String & param)const;
	virtual Vocab get_param_vocab()const;
}; // END of class SimpleCircle

/* === E N D =============================================================== */

#endif
