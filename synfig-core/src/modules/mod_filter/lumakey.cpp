/* === S Y N F I G ========================================================= */
/*!	\file lumakey.cpp
**	\brief Implementation of the "Luma Key" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "lumakey.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/segment.h>

#endif

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(LumaKey);
SYNFIG_LAYER_SET_NAME(LumaKey,"lumakey");
SYNFIG_LAYER_SET_LOCAL_NAME(LumaKey,N_("Luma Key"));
SYNFIG_LAYER_SET_CATEGORY(LumaKey,N_("Filters"));
SYNFIG_LAYER_SET_VERSION(LumaKey,"0.2");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

rendering::Task::Token TaskLumaKey::token(
	DescAbstract<TaskLumaKey>("LumaKey") );
rendering::Task::Token TaskLumaKeySW::token(
	DescReal<TaskLumaKeySW, TaskLumaKey>("LumaKeySW") );

TaskLumaKey::TaskLumaKey()
{
	// Sadly, LumaKey is not a linear transformation
	// we use a matrix just to save some intermadiate computations

	// final color must be: [ r' g' b' a*y 1 ]
	// matrix makes [ r g b a 1 ] -> [ r' g' b' a y ]
	// so we need to force later a final operation to make alpha = a*y

	matrix.set_encode_yuv();
	// makes W = Y and then Y = 1
	matrix *= ColorMatrix(
				0.,0.,0.,0.,1.,
				0.,1.,0.,0.,0.,
				0.,0.,1.,0.,0.,
				0.,0.,0.,1.,0.,
				1.,0.,0.,0.,0.
				);
	matrix *= ColorMatrix().set_decode_yuv();
}

bool
TaskLumaKeySW::run(RunParams&) const
{
	RectInt r = target_rect;
	if (r.valid())
	{
		VectorInt offset = get_offset();
		RectInt ra = sub_task()->target_rect + r.get_min() + get_offset();
		if (ra.valid())
		{
			rect_set_intersect(ra, ra, r);
			if (ra.valid())
			{
				LockWrite ldst(this);
				if (!ldst) return false;
				LockRead lsrc(sub_task());
				if (!lsrc) return false;

				const synfig::Surface &a = lsrc->get_surface();
				synfig::Surface &c = ldst->get_surface();
				for(int y = ra.miny; y < ra.maxy; ++y)
				{
					const Color *ca = &a[y - r.miny + offset[1]][ra.minx - r.minx + offset[0]];
					Color *cc = &c[y][ra.minx];
					for(int x = ra.minx; x < ra.maxx; ++x, ++ca, ++cc) {
						// new pixel value = matrix * pixel
						//    (pixel alpha does not affect and is not affected by the matrix)
						// matrix layout:
						//    [ a b c 0 d]
						//    [ e f g 0 h]
						//    [ i j k 0 l]
						//    [ 0 0 0 1 0]
						//    [ m n p 0 0]
						cc->set_r( ca->get_r()*matrix.m00 + ca->get_g()*matrix.m10 + ca->get_b()*matrix.m20 /*+ ca->get_a()*matrix.m30*/ + matrix.m40 );
						cc->set_g( ca->get_r()*matrix.m01 + ca->get_g()*matrix.m11 + ca->get_b()*matrix.m21 /*+ ca->get_a()*matrix.m31*/ + matrix.m41 );
						cc->set_b( ca->get_r()*matrix.m02 + ca->get_g()*matrix.m12 + ca->get_b()*matrix.m22 /*+ ca->get_a()*matrix.m32*/ + matrix.m42 );

						// bogus pixel w component is actually Luma (see Task_LumaKey)
						// set alpha := original alpha * original luma
						Real w   = ca->get_r()*matrix.m04 + ca->get_g()*matrix.m14 + ca->get_b()*matrix.m24 /*+ ca->get_a()*matrix.m34 + matrix.m44*/;
						cc->set_a( ca->get_a()/**matrix.m33*/ * w );
					}
				}
			}
		}
	}

	return true;
}

LumaKey::LumaKey()
{
}

ValueBase
LumaKey::get_param(const String &param)const
{
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer::get_param(param);
}

Color
LumaKey::get_color(Context context, const Point &getpos)const
{
	Color color(context.get_color(getpos));

	if (active()) {
		color.set_a(color.get_y()*color.get_a());
		color.set_y(1);
	}

	return color;
}

Rect
LumaKey::get_bounding_rect(Context context)const
{
	if(!active())
		return Rect::zero();

	return context.get_full_bounding_rect();
}

rendering::Task::Handle
LumaKey::build_rendering_task_vfunc(Context context) const
{
	rendering::Task::Handle task = context.build_rendering_task();

	TaskLumaKey::Handle task_lumakey(new TaskLumaKey());
	task_lumakey->sub_task() = task;

	task = task_lumakey;

	return task;
}
