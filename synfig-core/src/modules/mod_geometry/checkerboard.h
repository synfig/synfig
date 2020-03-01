/* === S Y N F I G ========================================================= */
/*!	\file checkerboard.h
**	\brief Header file for implementation of the "Checkerboard" layer
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_CHECKERBOARD_H
#define __SYNFIG_CHECKERBOARD_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_composite.h>
#include <synfig/color.h>
#include <synfig/vector.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class CheckerBoard : public synfig::Layer_Composite, public synfig::Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter: (Color)
	synfig::ValueBase param_color;
	//! Parameter: (Point)
	synfig::ValueBase param_origin;
	//! Parameter: (Point)
	synfig::ValueBase param_size;
	//! Parameter: (bool)
	synfig::ValueBase param_antialias;

	bool point_test(const synfig::Point& x)const;

public:
	CheckerBoard();

	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);

	virtual synfig::ValueBase get_param(const synfig::String & param)const;

	virtual synfig::Color get_color(synfig::Context context, const synfig::Point &pos)const;

	virtual Vocab get_param_vocab()const;

	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;

protected:
	virtual synfig::rendering::Task::Handle build_composite_task_vfunc(synfig::ContextParams context_params)const;
};

/* === E N D =============================================================== */

#endif
