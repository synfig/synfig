/* === S Y N F I G ========================================================= */
/*!	\file perspective.h
**	\brief Header file for implementation of the "Perspective" layer
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

#ifndef __SYNFIG_PERSPECTIVE_H
#define __SYNFIG_PERSPECTIVE_H

/* === H E A D E R S ======================================================= */

#include <synfig/vector.h>
#include <synfig/angle.h>
#include <synfig/layer.h>
#include <synfig/rect.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace modules
{
namespace lyr_std
{

class Perspective_Trans;

class Perspective : public Layer
{
	SYNFIG_LAYER_MODULE_EXT
	friend class Perspective_Trans;

private:
	//! Parameters: (Point)
	ValueBase param_src_tl;
	ValueBase param_src_br;
	ValueBase param_dest_tl;
	ValueBase param_dest_tr;
	ValueBase param_dest_bl;
	ValueBase param_dest_br;
	//! Parameter: (bool)
	ValueBase param_clip;
	//! Parameter: (int)
	ValueBase param_interpolation;

	bool valid;
	bool affine;
	Matrix matrix;
	Matrix back_matrix;
	bool clip;
	Rect clip_rect;

	Point transform(const Point &x) const;
	Point back_transform(const Point &x) const;

	Rect transform(const Rect &x) const;
	Rect back_transform(const Rect &x) const;

public:
	void sync();

	Perspective();
	~Perspective();

	virtual Rect get_bounding_rect() const;
	virtual Rect get_full_bounding_rect(Context context) const;

	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param) const;
	virtual Vocab get_param_vocab() const;

	virtual Color get_color(Context context, const Point &pos) const;
	virtual Layer::Handle hit_check(Context context, const Point &point) const;
	virtual etl::handle<Transform> get_transform() const;

protected:
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context) const;
};

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
