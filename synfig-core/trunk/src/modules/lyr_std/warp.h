/*! ========================================================================
** Synfig
** Template Header File
** $Id: warp.h,v 1.2 2005/01/24 05:00:18 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
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

	Point src_tl,src_br,dest_tl,dest_tr,dest_bl,dest_br;

	Real horizon;

	Real cache_a,cache_b,cache_c,cache_d,cache_e,cache_f,cache_i,cache_j;

	Real matrix[3][3];
	Real inv_matrix[3][3];

	Point transform_forward(const Point& p)const;
	Point transform_backward(const Point& p)const;

	Real transform_forward_z(const Point& p)const;
	Real transform_backward_z(const Point& p)const;

	bool clip;

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
	synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;	
	virtual Vocab get_param_vocab()const;	
	virtual etl::handle<synfig::Transform> get_transform()const;
};

/* === E N D =============================================================== */

#endif
