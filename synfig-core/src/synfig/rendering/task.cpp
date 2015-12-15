/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/task.cpp
**	\brief Task
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "task.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

const etl::handle<Task> Task::blank;

Task::~Task() { }

bool
Task::run(RunParams & /* params */) const
	{ return true; }

Vector
Task::get_pixels_per_unit() const
{
	if (!target_rect.valid())
		return Vector();
	return Vector(
		fabs(source_rect_rb[0] - source_rect_lt[0]) < 1e-10 ? 0.0 :
			(Real)(target_rect.maxx - target_rect.minx)/(source_rect_rb[0] - source_rect_lt[0]),
		fabs(source_rect_rb[1] - source_rect_lt[1]) < 1e-10 ? 0.0 :
			(Real)(target_rect.maxy - target_rect.miny)/(source_rect_rb[1] - source_rect_lt[1]) );
}

Vector
Task::get_units_per_pixel() const
{
	if ( !target_rect.valid()
	  || fabs(source_rect_rb[0] - source_rect_lt[0]) < 1e-10
	  || fabs(source_rect_rb[1] - source_rect_lt[1]) < 1e-10 )
		return Vector();
	return Vector(
		target_rect.maxx == target_rect.minx ? 0.0 :
			(source_rect_rb[0] - source_rect_lt[0])/(Real)(target_rect.maxx - target_rect.minx),
		target_rect.maxy == target_rect.miny ? 0.0 :
			(source_rect_rb[1] - source_rect_lt[1])/(Real)(target_rect.maxy - target_rect.miny) );
}


void
Task::clear_target_rect()
{
	source_rect_lt = source_rect_rb = Vector::zero();
	target_rect = RectInt::zero();
}

void
Task::init_target_rect(const RectInt &target_rect, const Point &source_rect_lt, const Point &source_rect_rb)
{
	this->target_rect = target_rect;
	this->source_rect_lt = source_rect_lt;
	this->source_rect_rb = source_rect_rb;
	if (!valid_target_rect())
		clear_target_rect();
}

void
Task::trunc_target_rect(const RectInt &rect)
{
	if (!valid_target_rect()) return;

	const RectInt tr = get_target_rect();
	if (target_rect.valid())
	{
		RectInt ntr = rect;
		etl::set_intersect(ntr, ntr, tr);
		if (ntr.valid())
		{
			const Vector lt = get_source_rect_lt();
			const Vector rb = get_source_rect_rb();
			Vector k( (rb[0] - lt[0])/(Real)(tr.maxx - tr.minx),
					  (rb[1] - lt[1])/(Real)(tr.maxy - tr.miny) );
			source_rect_lt[0] = (Real)(ntr.minx - tr.minx)*k[0] + lt[0];
			source_rect_lt[1] = (Real)(ntr.miny - tr.miny)*k[1] + lt[1];
			source_rect_rb[0] = (Real)(ntr.maxx - tr.minx)*k[0] + lt[0];
			source_rect_rb[1] = (Real)(ntr.maxy - tr.miny)*k[1] + lt[1];
			this->target_rect = ntr;
			return;
		}
	}
	source_rect_lt = source_rect_rb = Vector::zero();
	this->target_rect = RectInt::zero();
}

void
Task::trunc_source_rect(const Rect &rect)
{
	if (!valid_target_rect()) return;

	const RectInt tr = get_target_rect();
	Rect nsb = rect;
	if (nsb.valid())
	{
		const Vector lt = get_source_rect_lt();
		const Vector rb = get_source_rect_rb();
		Vector nlt( std::min(std::max(lt[0], nsb.minx), nsb.maxx),
					std::min(std::max(lt[1], nsb.miny), nsb.maxy) );
		Vector nrb( std::min(std::max(rb[0], nsb.minx), nsb.maxx),
					std::min(std::max(rb[1], nsb.miny), nsb.maxy) );
		if (nlt[0] != nrb[0] && nlt[1] != nrb[1])
		{
			Vector k(  (Real)(tr.maxx - tr.minx)/(rb[0] - lt[0]),
					   (Real)(tr.maxy - tr.miny)/(rb[1] - lt[1]) );
			Vector t0( (nlt[0] - lt[0])*k[0] + tr.minx,
					   (nlt[1] - lt[1])*k[1] + tr.miny );
			Vector t1( (nrb[0] - lt[0])*k[0] + tr.minx,
					   (nrb[1] - lt[1])*k[1] + tr.miny );
			if (t1[0] < t0[0]) std::swap(t1[0], t0[0]);
			if (t1[1] < t0[1]) std::swap(t1[1], t0[1]);

			const Real e = 1e-6;
			RectInt ntr( (int)floor(t0[0] + e),
						 (int)floor(t0[1] + e),
						 (int)ceil (t1[0] - e),
						 (int)ceil (t1[1] - e) );
			trunc_target_rect(ntr);
			return;
		}
	}
	source_rect_lt = source_rect_rb = Vector::zero();
	target_rect = RectInt::zero();
}

void
Task::move_target_rect(const VectorInt &offset)
{
	if (!valid_target_rect()) return;
	target_rect += offset;
}

void
Task::set_target_origin(const VectorInt &origin)
{
	if (!valid_target_rect()) return;
	move_target_rect(origin - target_rect.get_min());
}

void
Task::trunc_source_rect(const Point &lt, const Point &rb)
{
	if (!valid_target_rect()) return;

	const Vector &slt = get_source_rect_lt();
	const Vector &srb = get_source_rect_rb();
	if ( !lt.is_nan_or_inf()
	  && !rb.is_nan_or_inf()
	  && (slt[0] < srb[0]) == (lt[0] < rb[0])
	  && (slt[1] < srb[1]) == (lt[1] < rb[1]) )
	{
		trunc_source_rect(Rect(lt, rb));
		return;
	}
	source_rect_lt = source_rect_rb = Vector::zero();
	target_rect = RectInt::zero();
}

void
Task::trunc_target_by_bounds()
{
	trunc_source_rect(get_bounds());
}

Task::Handle
Task::clone_recursive() const {
	Task::Handle task = clone();
	for(List::iterator i = task->sub_tasks.begin(); i != task->sub_tasks.end(); ++i)
		if (*i) *i = (*i)->clone_recursive();
	return task;
}



/* === E N T R Y P O I N T ================================================= */
