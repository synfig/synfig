/* === S Y N F I G ========================================================= */
/*!	\file timeloop.h
**	\brief Template Header
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

#ifndef __SYNFIG_TIMELOOP_H
#define __SYNFIG_TIMELOOP_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/color.h>
#include <synfig/time.h>
#include <synfig/context.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Layer_TimeLoop : public synfig::Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:
	synfig::Time	start_time;
	synfig::Time	end_time;

protected:
	Layer_TimeLoop();

public:
	~Layer_TimeLoop();

	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);

	virtual synfig::ValueBase get_param(const synfig::String & param)const;

	virtual Vocab get_param_vocab()const;
	virtual synfig::Color get_color(synfig::Context context, const synfig::Point &pos)const;

	virtual void set_time(synfig::Context context, synfig::Time time)const;
	virtual bool accelerated_render(synfig::Context context,synfig::Surface *surface,int quality, const synfig::RendDesc &renddesc, synfig::ProgressCallback *cb)const;
};

/* === E N D =============================================================== */

#endif
