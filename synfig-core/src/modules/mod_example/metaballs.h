/* === S Y N F I G ========================================================= */
/*!	\file metaballs.h
**	\brief Header file for implementation of the "Metaballs" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_METABALLS_H
#define __SYNFIG_METABALLS_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_composite.h>
#include <synfig/gradient.h>
#include <synfig/vector.h>
#include <synfig/value.h>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
class Metaballs : public synfig::Layer_Composite, public synfig::Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//! Parameter: (Gradient)
	synfig::ValueBase param_gradient;
	//! Parameter: (std::vector<synfig::Point>)
	synfig::ValueBase param_centers;
	//! Parameter: (std::vector<synfig::Real>)
	synfig::ValueBase param_radii;
	//! Parameter: (std::vector<synfig::Real>)
	synfig::ValueBase param_weights;
	//! Parameter: (synfig::Real)
	synfig::ValueBase param_threshold;
	//! Parameter: (synfig::Real)
	synfig::ValueBase param_threshold2;
	//! Parameter: (bool)
	synfig::ValueBase param_positive;

	synfig::Real densityfunc(const synfig::Point &p, const synfig::Point &c, synfig::Real R)const;

	synfig::Real totaldensity(const synfig::Point &pos)const;

public:

	Metaballs();

	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);

	virtual synfig::ValueBase get_param(const synfig::String & param)const;

	virtual synfig::Color get_color(synfig::Context context, const synfig::Point &pos)const;

	virtual bool accelerated_render(synfig::Context context,synfig::Surface *surface,int quality, const synfig::RendDesc &renddesc, synfig::ProgressCallback *cb)const;

	virtual Vocab get_param_vocab()const;

	virtual synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
}; // END of class Metaballs

/* === E N D =============================================================== */

#endif
