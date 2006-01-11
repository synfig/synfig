/* === S Y N F I G ========================================================= */
/*!	\file metaballs.h
**	\brief Declares information for defining Metaballs.
**
**	$Id: metaballs.h,v 1.1.1.1 2005/01/04 01:23:09 darco Exp $
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

#ifndef __SYNFIG_METABALLS_H
#define __SYNFIG_METABALLS_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer_composite.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/value.h>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
class Metaballs : public synfig::Layer_Composite
{
	SYNFIG_LAYER_MODULE_EXT
	
private:

	synfig::Color color;

	std::vector<synfig::Point>	centers;
	std::vector<synfig::Real>	radii;
	std::vector<synfig::Real>	weights;

	synfig::Real	threshold;
	//Real	threshold2;
	
	synfig::Real totaldensity(const synfig::Point &pos)const;

public:
	
	Metaballs();
	
	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);

	virtual synfig::ValueBase get_param(const synfig::String & param)const;

	virtual synfig::Color get_color(synfig::Context context, const synfig::Point &pos)const;
	
	virtual bool accelerated_render(synfig::Context context,synfig::Surface *surface,int quality, const synfig::RendDesc &renddesc, synfig::ProgressCallback *cb)const;
	
	virtual Vocab get_param_vocab()const;
}; // END of class Metaballs

/* === E N D =============================================================== */

#endif
