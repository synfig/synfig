/* === S Y N F I G ========================================================= */
/*!	\file warp.h
**	\brief Header file for implementation of the "Warp" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_WARP_H
#define __SYNFIG_WARP_H

/* === H E A D E R S ======================================================= */

#include <synfig/vector.h>
#include <synfig/angle.h>
#include <synfig/layer.h>
#include <synfig/rect.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace std;
using namespace etl;
class Warp_Trans;

class Warp : public Layer
{
	SYNFIG_LAYER_MODULE_EXT
	friend class Warp_Trans;
private:
	//! Parameters: (Point)
	ValueBase param_src_tl;
	ValueBase param_src_br;
	ValueBase param_dest_tl;
	ValueBase param_dest_tr;
	ValueBase param_dest_bl;
	ValueBase param_dest_br;
	//! Parameter: (Real)
	ValueBase param_horizon;
	//! Parameter: (bool)
	ValueBase param_clip;

	//Real cache_a,cache_b,cache_c,cache_d,cache_e,cache_f,cache_i,cache_j;

	Real matrix[3][3];
	Real inv_matrix[3][3];

	Point transform_forward(const Point& p)const;
	Point transform_backward(const Point& p)const;

	Real transform_forward_z(const Point& p)const;
	Real transform_backward_z(const Point& p)const;


public:
	void sync();

	Warp();
	~Warp();

	virtual synfig::Rect get_full_bounding_rect(synfig::Context context)const;
	virtual synfig::Rect get_bounding_rect()const;

	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);
	virtual ValueBase get_param(const synfig::String & param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;
	virtual Vocab get_param_vocab()const;
	virtual etl::handle<synfig::Transform> get_transform()const;
};

/* === E N D =============================================================== */

#endif
