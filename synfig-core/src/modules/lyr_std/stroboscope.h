/* === S Y N F I G ========================================================= */
/*!	\file stroboscope.h
**	\brief Header file for implementation of the "Stroboscope" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Ray Frederikson
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

#ifndef __SYNFIG_STROBOSCOPE_H
#define __SYNFIG_STROBOSCOPE_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/color.h>
#include <synfig/time.h>
#include <synfig/context.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
using namespace synfig;
using namespace std;
using namespace etl;

class Layer_Stroboscope : public Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//!Parameter (float)
	ValueBase param_frequency;

protected:
	Layer_Stroboscope();

public:
	~Layer_Stroboscope();

	virtual bool set_param(const String & param, const ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Vocab get_param_vocab()const;
	virtual Color get_color(Context context, const Point &pos)const;

	virtual void set_time(IndependentContext context, Time time)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context,cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
};
/* === E N D =============================================================== */

#endif
