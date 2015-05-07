/* === S Y N F I G ========================================================= */
/*!	\file twirl.h
**	\brief Header file for implementation of the "Twirl" layer
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TWIRL_H
#define __SYNFIG_TWIRL_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_composite.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/value.h>
#include <synfig/gradient.h>
#include <synfig/angle.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
class Twirl_Trans;

class Twirl : public synfig::Layer_Composite
{
	SYNFIG_LAYER_MODULE_EXT
	friend class Twirl_Trans;

private:
	//! Parameter: (synfig::Point)
	synfig::ValueBase param_center;
	//! Parameter: (synfig::Real)
	synfig::ValueBase param_radius;
	//! Parameter: (synfig::Angle)
	synfig::ValueBase param_rotations;
	//! Parameter: (bool)
	synfig::ValueBase param_distort_inside;
	//! Parameter: (bool)
	synfig::ValueBase param_distort_outside;

	synfig::Point distort(const synfig::Point &pos, bool reverse=false)const;
public:

	Twirl();

	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);

	virtual synfig::ValueBase get_param(const synfig::String & param)const;

	virtual synfig::Color get_color(synfig::Context context, const synfig::Point &pos)const;
	virtual synfig::CairoColor get_cairocolor(synfig::Context context, const synfig::Point &pos)const;

	//virtual bool accelerated_render(synfig::Context context,synfig::Surface *surface,int quality, const synfig::RendDesc &renddesc, synfig::ProgressCallback *cb)const;

	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;

	virtual Vocab get_param_vocab()const;
	virtual etl::handle<synfig::Transform> get_transform()const;
	virtual bool reads_context()const { return true; }
}; // END of class Twirl

/* === E N D =============================================================== */

#endif
