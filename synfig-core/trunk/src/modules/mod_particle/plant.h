/* === S I N F G =========================================================== */
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

#ifndef __SINFG_PLANT_H
#define __SINFG_PLANT_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <vector>
#include <sinfg/layer_composite.h>
#include <sinfg/segment.h>
#include <sinfg/blinepoint.h>
#include <sinfg/value.h>
#include <sinfg/gradient.h>
#include <sinfg/angle.h>
#include "random.h"
#include <sinfg/rect.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

class Plant : public sinfg::Layer_Composite
{
	SINFG_LAYER_MODULE_EXT
private:

	std::vector<sinfg::BLinePoint> bline;
	bool bline_loop;

	sinfg::Gradient gradient;

	struct Particle
	{
		sinfg::Point point;
		sinfg::Color color;
		
		Particle(const sinfg::Point &point,const sinfg::Color& color):
			point(point),color(color) { }
	};

	mutable std::vector<Particle> particle_list;
	mutable sinfg::Rect	bounding_rect;
	sinfg::Angle split_angle;
	sinfg::Vector gravity;
	sinfg::Real velocity;
	sinfg::Real step;
	sinfg::Real mass;
	sinfg::Real drag;
	sinfg::Real size;
	int splits;
	int sprouts;
	sinfg::Real random_factor;
	Random random;
	
	bool size_as_alpha;
	mutable bool needs_sync_;
	

	void branch(int n, int depth,float t, float stunt_growth, sinfg::Point position,sinfg::Vector velocity)const;

public:

	Plant();

	void sync()const;

	void calc_bounding_rect()const;

	virtual bool set_param(const String & param, const sinfg::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Vocab get_param_vocab()const;

	virtual bool accelerated_render(sinfg::Context context,sinfg::Surface *surface,int quality, const sinfg::RendDesc &renddesc, sinfg::ProgressCallback *cb)const;\

	virtual sinfg::Rect get_bounding_rect(sinfg::Context context)const;
};

/* === E N D =============================================================== */

#endif
