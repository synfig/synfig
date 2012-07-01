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

class Plant : public synfig::Layer_Composite, public synfig::Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT
private:

	std::vector<synfig::BLinePoint> bline;
	bool bline_loop;

	Point origin;

	synfig::Gradient gradient;

	struct Particle
	{
		synfig::Point point;
		synfig::Color color;

		Particle(const synfig::Point &point,const synfig::Color& color):
			point(point),color(color) { }
	};

	mutable std::vector<Particle> particle_list;
	mutable synfig::Rect	bounding_rect;
	synfig::Angle split_angle;
	synfig::Vector gravity;
	synfig::Real velocity;
	synfig::Real perp_velocity;
	synfig::Real step;
	synfig::Real mass;
	synfig::Real drag;
	synfig::Real size;
	int splits;
	int sprouts;
	synfig::Real random_factor;
	Random random;

	bool size_as_alpha;
	bool reverse;
	mutable bool needs_sync_;
	mutable synfig::Mutex mutex;

	void branch(int n, int depth,float t, float stunt_growth, synfig::Point position,synfig::Vector velocity)const;
	void sync()const;
	String version;
	bool use_width;

public:

	Plant();

	void calc_bounding_rect()const;

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual bool set_version(const synfig::String &ver);

	virtual Vocab get_param_vocab()const;

	virtual bool accelerated_render(synfig::Context context,synfig::Surface *surface,int quality, const synfig::RendDesc &renddesc, synfig::ProgressCallback *cb)const;\

	virtual synfig::Rect get_bounding_rect(synfig::Context context)const;
};

/* === E N D =============================================================== */

#endif
