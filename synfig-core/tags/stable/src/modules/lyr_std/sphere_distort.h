/* === S I N F G =========================================================== */
/*!	\file sphere_distort.h
**	\brief Sphere Distort Header
**
**	$Id: sphere_distort.h,v 1.2 2005/01/24 05:00:18 darco Exp $
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

#ifndef __SINFG_SPHERE_DISTORT_H
#define __SINFG_SPHERE_DISTORT_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer_composite.h>
#include <sinfg/vector.h>
#include <sinfg/rect.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
class Spherize_Trans;
namespace sinfg 
{

class Layer_SphereDistort : public Layer_Composite
{
	SINFG_LAYER_MODULE_EXT
	friend class Spherize_Trans;
	
private:

	Vector center;
	double radius;

	double percent;

	int		type;

//	static Point sphtrans(const Point &xoff, const Point &center, const Real &radius, const Real &percent, int type);

//	static double spherify(double xoff);
//	static double unspherify(double xoff);

	bool clip;

	sinfg::Rect bounds;

	void sync();

public:
	
	Layer_SphereDistort();
		
	virtual bool set_param(const String & param, const sinfg::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;
	
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;	

	virtual Rect get_bounding_rect()const;
	
	virtual Vocab get_param_vocab()const;
	virtual etl::handle<sinfg::Transform> get_transform()const;
}; // END of class Layer_SphereDistort

}

/* === E N D =============================================================== */

#endif
