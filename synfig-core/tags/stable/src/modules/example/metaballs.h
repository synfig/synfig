/* === S I N F G =========================================================== */
/*!	\file metaballs.h
**	\brief Declares information for defining Metaballs.
**
**	$Id: metaballs.h,v 1.1.1.1 2005/01/04 01:23:09 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_METABALLS_H
#define __SINFG_METABALLS_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer_composite.h>
#include <sinfg/color.h>
#include <sinfg/vector.h>
#include <sinfg/value.h>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
class Metaballs : public sinfg::Layer_Composite
{
	SINFG_LAYER_MODULE_EXT
	
private:

	sinfg::Color color;

	std::vector<sinfg::Point>	centers;
	std::vector<sinfg::Real>	radii;
	std::vector<sinfg::Real>	weights;

	sinfg::Real	threshold;
	//Real	threshold2;
	
	sinfg::Real totaldensity(const sinfg::Point &pos)const;

public:
	
	Metaballs();
	
	virtual bool set_param(const sinfg::String & param, const sinfg::ValueBase &value);

	virtual sinfg::ValueBase get_param(const sinfg::String & param)const;

	virtual sinfg::Color get_color(sinfg::Context context, const sinfg::Point &pos)const;
	
	virtual bool accelerated_render(sinfg::Context context,sinfg::Surface *surface,int quality, const sinfg::RendDesc &renddesc, sinfg::ProgressCallback *cb)const;
	
	virtual Vocab get_param_vocab()const;
}; // END of class Metaballs

/* === E N D =============================================================== */

#endif
