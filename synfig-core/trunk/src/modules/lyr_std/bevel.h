/* === S Y N F I G ========================================================= */
/*!	\file bevel.h
**	\brief Template Header
**
**	$Id: bevel.h,v 1.2 2005/01/24 03:08:17 darco Exp $
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

/* === H E A D E R S ======================================================= */

#ifndef __SYNFIG_LAYER_BEVEL_H__
#define __SYNFIG_LAYER_BEVEL_H__

/* -- H E A D E R S --------------------------------------------------------- */

#include <synfig/layer_composite.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/blur.h>
#include <synfig/angle.h>

using namespace synfig;
using namespace std;
using namespace etl;

class Layer_Bevel : public synfig::Layer_Composite
{
	SYNFIG_LAYER_MODULE_EXT
private:
	synfig::Real 	softness;
	int				type;

	synfig::Color	color1;
	synfig::Color	color2;

	synfig::Angle	angle;
	synfig::Real		depth;

	synfig::Vector	offset;
	synfig::Vector	offset45;

	bool use_luma;
	bool solid;

	void calc_offset();
public:
	Layer_Bevel();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual synfig::Rect get_full_bounding_rect(Context context)const;
	virtual Vocab get_param_vocab()const;
}; // END of class Blur

/* -- E X T E R N S --------------------------------------------------------- */

/* -- E N D ----------------------------------------------------------------- */

#endif
