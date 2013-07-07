/* === S Y N F I G ========================================================= */
/*!	\file plant.h
**	\brief Header file for implementation of the "Plant" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#ifndef __SYNFIG_PLANT_H
#define __SYNFIG_PLANT_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <vector>
#include <synfig/layer_composite.h>
#include <synfig/segment.h>
#include <synfig/blinepoint.h>
#include <synfig/value.h>
#include <synfig/gradient.h>
#include <synfig/angle.h>
#include "random.h"
#include <synfig/rect.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace std;
using namespace etl;

class Plant : public Layer_Composite, public Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT
private:

	std::vector<BLinePoint> bline;
	bool bline_loop;

	Point origin;

	Gradient gradient;

	struct Particle
	{
		Point point;
		Color color;

		Particle(const Point &point,const Color& color):
			point(point),color(color) { }
	};

	mutable std::vector<Particle> particle_list;
	mutable Rect	bounding_rect;
	Angle split_angle;
	Vector gravity;
	Real velocity;
	Real perp_velocity;
	Real step;
	Real mass;
	Real drag;
	Real size;
	int splits;
	int sprouts;
	Real random_factor;
	Random random;

	bool size_as_alpha;
	bool reverse;
	mutable bool needs_sync_;
	mutable Mutex mutex;

	void branch(int n, int depth,float t, float stunt_growth, Point position,Vector velocity)const;
	void sync()const;
	String version;
	bool use_width;
	void draw_particles(Surface *surface, const RendDesc &renddesc)const;
	void draw_particles(cairo_surface_t *surface, const RendDesc &renddesc)const;
	void draw_particles(cairo_t *cr)const;

public:

	Plant();

	void calc_bounding_rect()const;

	virtual bool set_param(const String & param, const ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual bool set_version(const String &ver);

	virtual Vocab get_param_vocab()const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context, cairo_surface_t *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual Rect get_bounding_rect(Context context)const;
};

/* === E N D =============================================================== */

#endif
