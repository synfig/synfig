/* === S Y N F I G ========================================================= */
/*!	\file circle.h
**	\brief Header file for implementation of the "Circle" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifndef __SYNFIG_LAYER_CIRCLE_H__
#define __SYNFIG_LAYER_CIRCLE_H__

/* -- H E A D E R S --------------------------------------------------------- */

#include <synfig/layers/layer_polygon.h>

using namespace synfig;
using namespace etl;

/* -- M A C R O S ----------------------------------------------------------- */

/* -- T Y P E D E F S ------------------------------------------------------- */

/* -- S T R U C T S & C L A S S E S ----------------------------------------- */

class Circle : public synfig::Layer_Shape
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//!Parameter: (Real)
	ValueBase param_radius;

protected:
	virtual void sync_vfunc();

public:
	Circle();

	virtual bool set_shape_param(const synfig::String & param, const synfig::ValueBase &value);
	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param)const;
	virtual Vocab get_param_vocab()const;
	
	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
};

/* -- E X T E R N S --------------------------------------------------------- */


/* -- E N D ----------------------------------------------------------------- */

#endif
