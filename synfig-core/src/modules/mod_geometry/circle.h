/* === S Y N F I G ========================================================= */
/*!	\file circle.h
**	\brief Header file for implementation of the "Circle" layer
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifndef __SYNFIG_LAYER_CIRCLE_H__
#define __SYNFIG_LAYER_CIRCLE_H__

/* -- H E A D E R S --------------------------------------------------------- */

#include <synfig/layers/layer_composite.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/value.h>

using namespace synfig;
using namespace std;
using namespace etl;

/* -- M A C R O S ----------------------------------------------------------- */

/* -- T Y P E D E F S ------------------------------------------------------- */

/* -- S T R U C T S & C L A S S E S ----------------------------------------- */

class Circle : public synfig::Layer_Composite, public synfig::Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//!Parameter: (Color)
	ValueBase param_color;
	//!Parameter: (Point)
	ValueBase param_origin;
	//!Parameter: (Real)
	ValueBase param_radius;
	//!Parameter: (Real)
	ValueBase param_feather;
	//!Parameter: (bool)
	ValueBase param_invert;
	//!Parameter: (int)
	ValueBase param_falloff;

	//Caching system for circle
	struct CircleDataCache
	{
		Real inner_radius;
		Real outer_radius;

		Real inner_radius_sqd;
		Real outer_radius_sqd;

		Real diff_sqd;
		Real double_feather;
	};

	typedef	Real	FALLOFF_FUNC(const CircleDataCache &c, const Real &mag_sqd);

	FALLOFF_FUNC	*falloff_func;
	CircleDataCache	cache;

	void constructcache();

	static	Real	SqdFalloff(const CircleDataCache &c, const Real &mag_sqd);
	static	Real	InvSqdFalloff(const CircleDataCache &c, const Real &mag_sqd);
	static	Real	SqrtFalloff(const CircleDataCache &c, const Real &mag_sqd);
	static	Real	InvSqrtFalloff(const CircleDataCache &c, const Real &mag_sqd);
	static	Real	LinearFalloff(const CircleDataCache &c, const Real &mag_sqd);
	static	Real	InvLinearFalloff(const CircleDataCache &c, const Real &mag_sqd);
	static	Real	SigmondFalloff(const CircleDataCache &c, const Real &mag_sqd);
	static	Real	InvSigmondFalloff(const CircleDataCache &c, const Real &mag_sqd);
	static	Real	CosineFalloff(const CircleDataCache &c, const Real &mag_sqd);
	static	Real	InvCosineFalloff(const CircleDataCache &c, const Real &mag_sqd);

	FALLOFF_FUNC	*GetFalloffFunc()const;
	bool ImportParameters(const String &param, const ValueBase &value);

public:
	enum Falloff
	{
		FALLOFF_SQUARED		=0,
		FALLOFF_INTERPOLATION_LINEAR		=1,
		FALLOFF_SMOOTH		=2,
		FALLOFF_COSINE		=2,
		FALLOFF_SIGMOND		=3,
		FALLOFF_SQRT		=4
	};

	Circle();

	virtual bool set_param(const String &param, const ValueBase &value);

	virtual ValueBase get_param(const String &param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context,cairo_t *cr,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual synfig::Rect get_full_bounding_rect(synfig::Context context)const;
	virtual synfig::Rect get_bounding_rect()const;

	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;

	virtual Vocab get_param_vocab()const;
	void compile_gradient(cairo_pattern_t* gradient, CircleDataCache cache, FALLOFF_FUNC *func)const;
};

/* -- E X T E R N S --------------------------------------------------------- */


/* -- E N D ----------------------------------------------------------------- */

#endif
