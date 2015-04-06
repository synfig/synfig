/* === S Y N F I G ========================================================= */
/*!	\file layer_svg.h
**	\brief Header file for implementation of the Svg Canvas layer
**
**	$Id:$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2009 Carlos A. Sosa Navarro
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

#ifndef __SYNFIG_SVG_LAYER_H
#define __SYNFIG_SVG_LAYER_H

/* === H E A D E R S ======================================================= */

#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/layers/layer_group.h>
#include <synfig/value.h>

#include "svg_parser.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class svg_layer : public synfig::Layer_Group
{
	SYNFIG_LAYER_MODULE_EXT

private:

	synfig::String filename;
	synfig::String errors,warnings;

public:

	svg_layer();

	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);

	virtual synfig::ValueBase get_param(const synfig::String & param)const;

	virtual Vocab get_param_vocab()const;
}; // END of class svg_layer

/* === E N D =============================================================== */

#endif

