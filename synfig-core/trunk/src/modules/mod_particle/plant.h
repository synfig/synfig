/* === S Y N F I G ========================================================= */
/*!	\file plant.h
**	\brief Template Header
**
**	$Id: plant.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

class Plant : public synfig::Layer_Composite
{
	SYNFIG_LAYER_MODULE_EXT
private:

	std::vector<synfig::BLinePoint> bline;
	bool bline_loop;

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
	synfig::Real step;
	synfig::Real mass;
	synfig::Real drag;
	synfig::Real size;
	int splits;
	int sprouts;
	synfig::Real random_factor;
	Random random;
	
	bool size_as_alpha;
	mutable bool needs_sync_;
	

	void branch(int n, int depth,float t, float stunt_growth, synfig::Point position,synfig::Vector velocity)const;

public:

	Plant();

	void sync()const;

	void calc_bounding_rect()const;

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Vocab get_param_vocab()const;

	virtual bool accelerated_render(synfig::Context context,synfig::Surface *surface,int quality, const synfig::RendDesc &renddesc, synfig::ProgressCallback *cb)const;\

	virtual synfig::Rect get_bounding_rect(synfig::Context context)const;
};

/* === E N D =============================================================== */

#endif
