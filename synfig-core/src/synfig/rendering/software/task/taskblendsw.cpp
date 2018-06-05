/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskblendsw.cpp
**	\brief TaskBlendSW
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <synfig/general.h>

#include "../../common/task/taskblend.h"
#include "tasksw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskBlendSW: public TaskBlend, public TaskSW
{
public:
	typedef etl::handle<TaskBlendSW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }


	virtual bool run(RunParams&) const {
		if (!is_valid()) return true;

		LockWrite lc(this);
		if (!lc) return false;
		synfig::Surface &c = lc->get_surface();
		RectInt r = target_rect;

		// blit surface a
		RectInt ra = RectInt::zero();
		if (sub_task_a() && sub_task_a()->is_valid())
		{
			RectInt ra = sub_task_a()->target_rect + r.get_min() + get_offset_a();
			if (ra.is_valid())
			{
				etl::set_intersect(ra, ra, r);
				if (ra.is_valid() && sub_task_a()->target_surface != target_surface)
				{
					LockRead la(sub_task_a());
					if (!la) return false;
					synfig::Surface &a = la.cast_handle()->get_surface(); // TODO: make blit_to constant

					synfig::Surface::pen p = c.get_pen(ra.minx, ra.maxx);
					a.blit_to(
						p,
						ra.minx - r.minx - get_offset_a()[0],
						ra.miny - r.miny - get_offset_a()[1],
						ra.maxx - ra.minx,
						ra.maxy - ra.miny );
				}
			}
		}

		// blit surface b
		RectInt fill[] = { ra, RectInt::zero(), RectInt::zero(), RectInt::zero() };
		if (sub_task_b() && sub_task_b()->is_valid())
		{
			RectInt rb = sub_task_b()->target_rect + r.get_min() + get_offset_b();
			if (rb.is_valid())
			{
				etl::set_intersect(rb, rb, r);
				if (rb.is_valid())
				{
					LockRead lb(sub_task_b());
					if (!lb) return false;
					synfig::Surface &b = lb.cast_handle()->get_surface(); // TODO: make blit_to constant

					synfig::Surface::alpha_pen ap(c.get_pen(rb.minx, rb.miny));
					ap.set_blend_method(blend_method);
					ap.set_alpha(amount);
					b.blit_to(
						ap,
						rb.minx - r.minx - get_offset_b()[0],
						rb.miny - r.miny - get_offset_b()[1],
						rb.maxx - rb.minx,
						rb.maxy - rb.miny );

					if (ra.is_valid())
					{
						// mark unfilled regions
						fill[0] = fill[1] = fill[2] = fill[3] = ra;
						fill[0].maxx = fill[2].minx = fill[3].minx = std::max(ra.minx, std::min(ra.maxx, rb.minx));
						fill[1].minx = fill[2].maxx = fill[3].maxx = std::max(ra.minx, std::min(ra.maxx, rb.maxx));
						fill[2].maxy = std::max(ra.miny, std::min(ra.maxy, rb.miny));
						fill[3].miny = std::max(ra.miny, std::min(ra.maxy, rb.maxy));
					}
				}
			}
		}

		// process unfilled region
		if (Color::is_straight(blend_method))
		{
			for(int i = 0; i < 4; ++i)
			{
				if (fill[i].valid())
				{
					synfig::Surface::alpha_pen ap(
						c.get_pen(fill[i].minx, fill[i].miny) );
					ap.set_blend_method(blend_method);
					ap.set_alpha(amount);
					c.fill( Color(0, 0, 0, 0), ap,
							fill[i].maxx - fill[i].minx,
							fill[i].maxy - fill[i].miny );
				}
			}
		}

		return true;
	}
};


Task::Token TaskBlendSW::token(
	DescReal<TaskBlendSW, TaskBlend>("BlendSW") );

} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
