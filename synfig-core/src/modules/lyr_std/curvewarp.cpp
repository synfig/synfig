/* === S Y N F I G ========================================================= */
/*!	\file curvewarp.cpp
**	\brief Implementation of the "Curve Warp" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007-2008 Chris Moore
**	Copyright (c) 2011-2013 Carlos López
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
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "curvewarp.h"

#include <synfig/blinepoint.h>
#include <synfig/context.h>
#include <synfig/localization.h>

#include <synfig/rendering/common/task/taskdistort.h>
#include <synfig/rendering/software/task/taskdistortsw.h>

#endif

using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(CurveWarp);
SYNFIG_LAYER_SET_NAME(CurveWarp,"curve_warp");
SYNFIG_LAYER_SET_LOCAL_NAME(CurveWarp,N_("Curve Warp"));
SYNFIG_LAYER_SET_CATEGORY(CurveWarp,N_("Distortions"));
SYNFIG_LAYER_SET_VERSION(CurveWarp,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === I N T E R N A L S =================================================== */

struct CurveWarp::Internal
{
	std::vector<BLinePoint> bline;
	Point start_point;
	Point end_point;
	Point origin;
	bool fast;
	Real perp_width;

	static constexpr double FAKE_TANGENT_STEP = 0.000001;
	static constexpr double TOO_THIN = 0.01;

	Point transform(const Point& point) const;
	void sync();

private:
	Vector perp_;
	Real curve_length_;

	float calculate_distance() const;
	std::vector<BLinePoint>::const_iterator find_closest_to_bline(const Point& p, float& t, float& len, bool& extreme) const;
};

float
CurveWarp::Internal::calculate_distance() const
{
	std::vector<BLinePoint>::const_iterator iter, next;
	std::vector<BLinePoint>::const_iterator end(bline.end());

	float dist(0);

	if (bline.empty()) return dist;

	next=bline.begin();
	iter=next++;

	for(;next!=end;iter=next++)
	{
		// Setup the curve
		hermite<Vector> curve(iter->get_vertex(), next->get_vertex(), iter->get_tangent2(), next->get_tangent1());
		dist+=curve.length();
	}

	return dist;
}

std::vector<BLinePoint>::const_iterator
CurveWarp::Internal::find_closest_to_bline(const Point& p,float& t, float& len, bool& extreme) const
{
	std::vector<BLinePoint>::const_iterator iter,next,ret;
	std::vector<BLinePoint>::const_iterator end(bline.end());

	ret=bline.end();
	float dist(100000000000.0);
	next=bline.begin();
	float best_pos(0), best_len(0);
	hermite<Vector> best_curve;
	iter=next++;
	Point bp;
	float total_len(0);
	bool first = true, last = false;
	extreme = false;

	for(;next!=end;iter=next++)
	{
		// Setup the curve
		hermite<Vector> curve(iter->get_vertex(), next->get_vertex(), iter->get_tangent2(), next->get_tangent1());
		float thisdist(0);
		last = false;

		if (fast)
		{
#define POINT_CHECK(x) bp=curve(x);	thisdist=(bp-p).mag_squared(); if(thisdist<dist) { extreme = (first&&x<0.01); ret=iter; best_len = total_len; dist=thisdist; best_curve=curve; last=true; best_pos=x;}
			POINT_CHECK(0.0001);  POINT_CHECK((1.0/6)); POINT_CHECK((2.0/6)); POINT_CHECK((3.0/6));
			POINT_CHECK((4.0/6)); POINT_CHECK((5.0/6)); POINT_CHECK(0.9999);
		}
		else
		{
			float pos = curve.find_closest(fast, p);
			thisdist=(curve(pos)-p).mag_squared();
			if(thisdist<dist)
			{
				extreme = (first && pos == 0);
				ret=iter;
				dist=thisdist;
				best_pos = pos;
				best_curve = curve;
				best_len = total_len;
				last = true;
			}
		}
		total_len += curve.length();
		first = false;
	}

	t = best_pos;
	if (fast)
	{
		len = best_len + best_curve.find_distance(0,best_curve.find_closest(fast, p));
		if (last && t > .99) extreme = true;
	}
	else
	{
		len = best_len + best_curve.find_distance(0,best_pos);
		if (last && t == 1) extreme = true;
	}
	return ret;
}

void
CurveWarp::Internal::sync()
{
	curve_length_ = calculate_distance();
	perp_ = (end_point - start_point).perp().norm();
}

Point
CurveWarp::Internal::transform(const Point &point_) const
{
	Vector tangent;
	Vector diff;
	Point p1;
	Real thickness;
	bool edge_case = false;
	float len(0);
	bool extreme;
	float t;

	if(bline.size()==0)
		return Point();
	else if(bline.size()==1)
	{
		tangent=bline.front().get_tangent1();
		p1=bline.front().get_vertex();
		thickness=bline.front().get_width();
		t = 0.5;
		extreme = false;
	}
	else
	{
		Point point(point_-origin);

		std::vector<BLinePoint>::const_iterator iter,next;

		// Figure out the BLinePoint we will be using,
		next=find_closest_to_bline(point,t,len,extreme);

		iter=next++;
		if(next==bline.end()) next=bline.begin();

		// Setup the curve
		hermite<Vector> curve(iter->get_vertex(), next->get_vertex(), iter->get_tangent2(), next->get_tangent1());

		// Figure out the closest point on the curve
		if (fast) t = curve.find_closest(fast, point, 7);

		// Calculate our values
		p1=curve(t);			     // the closest point on the curve
		tangent=curve.derivative(t); // the tangent at that point

		// if the point we're nearest to is at either end of the
		// bline, our distance from the curve is the distance from the
		// point on the curve.  we need to know which side of the
		// curve we're on, so find the average of the two tangents at
		// this point
		if (t<0.00001 || t>0.99999)
		{
			bool zero_tangent = (tangent[0] == 0 && tangent[1] == 0);

			if (t<0.5)
			{
				if (iter->get_split_tangent_angle() || iter->get_split_tangent_radius() || zero_tangent)
				{
					// fake the current tangent if we need to
					if (zero_tangent) tangent = curve(FAKE_TANGENT_STEP) - curve(0);

					// calculate the other tangent
					Vector other_tangent(iter->get_tangent1());
					if (other_tangent[0] == 0 && other_tangent[1] == 0)
					{
						// find the previous blinepoint
						std::vector<BLinePoint>::const_iterator prev;
						if (iter != bline.begin()) (prev = iter)--;
						else prev = iter;

						hermite<Vector> other_curve(prev->get_vertex(), iter->get_vertex(), prev->get_tangent2(), iter->get_tangent1());
						other_tangent = other_curve(1) - other_curve(1-FAKE_TANGENT_STEP);
					}

					// normalise and sum the two tangents
					tangent=(other_tangent.norm()+tangent.norm());
					edge_case=true;
				}
			}
			else
			{
				if (next->get_split_tangent_angle() || next->get_split_tangent_radius() || zero_tangent)
				{
					// fake the current tangent if we need to
					if (zero_tangent) tangent = curve(1) - curve(1-FAKE_TANGENT_STEP);

					// calculate the other tangent
					Vector other_tangent(next->get_tangent2());
					if (other_tangent[0] == 0 && other_tangent[1] == 0)
					{
						// find the next blinepoint
						std::vector<BLinePoint>::const_iterator next2(next);
						if (++next2 == bline.end())
							next2 = next;

						hermite<Vector> other_curve(next->get_vertex(), next2->get_vertex(), next->get_tangent2(), next2->get_tangent1());
						other_tangent = other_curve(FAKE_TANGENT_STEP) - other_curve(0);
					}

					// normalise and sum the two tangents
					tangent=(other_tangent.norm()+tangent.norm());
					edge_case=true;
				}
			}
		}
		tangent = tangent.norm();

		// the width of the bline at the closest point on the curve
		thickness=(next->get_width()-iter->get_width())*t+iter->get_width();
	}

	if (thickness < TOO_THIN && thickness > -TOO_THIN)
	{
		if (thickness > 0) thickness = TOO_THIN;
		else thickness = -TOO_THIN;
	}

	if (extreme)
	{
		Vector tangent;

		if (t < 0.5)
		{
			std::vector<BLinePoint>::const_iterator iter(bline.begin());
			tangent = iter->get_tangent1().norm();
			len = 0;
		}
		else
		{
			std::vector<BLinePoint>::const_iterator iter(--bline.end());
			tangent = iter->get_tangent2().norm();
			len = curve_length_;
		}
		len += (point_-origin - p1)*tangent;
		diff = tangent.perp();
	}
	else if (edge_case)
	{
		diff=(p1-(point_-origin));
		if(diff*tangent.perp()<0) diff=-diff;
		diff=diff.norm();
	}
	else
		diff=tangent.perp();

	// diff is a unit vector perpendicular to the bline
	const Real unscaled_distance((point_-origin - p1)*diff);
	return ((start_point + (end_point - start_point) * len / curve_length_) +
			perp_ * unscaled_distance/(thickness*perp_width));
}

class TaskCurveWarp
	: public rendering::TaskDistort
{
public:
	typedef etl::handle<TaskCurveWarp> Handle;
	static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	CurveWarp::Internal internal;
	//	virtual bool get_allow_multithreading() const {
	//		if (const Mode *mode = dynamic_cast<const Mode*>(this))
	//			return mode->get_mode_allow_multithreading();
	//		return true;
	//	}
	//	virtual bool get_mode_allow_source_as_target() const {
	//		if (const Mode *mode = dynamic_cast<const Mode*>(this))
	//			return mode->get_mode_allow_source_as_target();
	//		return false;
	//	}
//		virtual bool get_mode_allow_simultaneous_write() const { //!< allow simultaneous write to the same target
//	//		if (const Mode *mode = dynamic_cast<const Mode*>(this))
//	//			return mode->get_mode_allow_simultaneous_write();
//	//		return true;
//		return false;
//		}

	Rect
	compute_required_source_rect(const Rect& source_rect, const Matrix& inv_matrix) const override
	{
		const int tw = target_rect.get_width();
		const int th = target_rect.get_height();
		Vector dx = inv_matrix.axis_x();
		Vector dy = inv_matrix.axis_y() - dx*(Real)tw;
		Vector p = inv_matrix.get_transformed( Vector((Real)target_rect.minx, (Real)target_rect.miny) );

		Rect sub_source_rect = source_rect;

		// Check from where the boundary pixels come in source context (before transform)
		// vertical borders
		for (int iy = target_rect.miny; iy < target_rect.maxy; ++iy, p[1] += dy[1]) {
			Point tmp = internal.transform(p);
			sub_source_rect.expand(tmp);
			tmp = internal.transform(Point(p[0] + dx[0]*(Real)tw, p[1]));
			sub_source_rect.expand(tmp);
		}

		// horizontal borders
		for (int ix = target_rect.minx; ix < target_rect.maxx; ++ix, p[0] += dx[0]) {
			Point tmp = internal.transform(p);
			sub_source_rect.expand(tmp);
			tmp = internal.transform(Point(p[0], p[1] - dy[1]*(Real)th));
			sub_source_rect.expand(tmp);
		}

		return sub_source_rect;
	}

};

class TaskCurveWarpSW
	: public TaskCurveWarp, public rendering::TaskDistortSW
{
public:
	typedef etl::handle<TaskCurveWarp> Handle;
	static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Point
	point_vfunc(const Point &point) const override
	{
		return internal.transform(point);
	}

	bool run(Task::RunParams& /*params*/) const override
	{
		return run_task(*this);
	}
};

rendering::Task::Token TaskCurveWarp::token(
	DescAbstract<TaskCurveWarp>("CurveWarp") );
rendering::Task::Token TaskCurveWarpSW::token(
	DescReal<TaskCurveWarpSW, TaskCurveWarp>("CurveWarpSW") );


/* === M E T H O D S ======================================================= */

void
CurveWarp::sync()
{
	internal->bline = param_bline.get_list_of(BLinePoint());
	internal->start_point = param_start_point.get(Point());
	internal->end_point = param_end_point.get(Point());

	internal->sync();
}

CurveWarp::CurveWarp():
	param_origin(ValueBase(Point(0,0))),
	param_perp_width(ValueBase(Real(1))),
	param_start_point(ValueBase(Point(-2.5,-0.5))),
	param_end_point(ValueBase(Point(2.5,-0.3))),
	param_bline(ValueBase(std::vector<BLinePoint>())),
	param_fast(ValueBase(true))
{
	std::vector<BLinePoint> bline;
	bline.push_back(BLinePoint());
	bline.push_back(BLinePoint());
	bline[0].set_vertex(Point(-2.5,0));
	bline[1].set_vertex(Point( 2.5,0));
	bline[0].set_tangent(Point(1,  0.1));
	bline[1].set_tangent(Point(1, -0.1));
	bline[0].set_width(1.0f);
	bline[1].set_width(1.0f);
	param_bline.set_list_of(bline);

	internal = new Internal();
	internal->origin = {0,0};
	internal->perp_width = 1.;
	internal->start_point = {-2.5, -0.5};
	internal->end_point = {2.5, -0.3};
	internal->bline = bline;
	internal->fast = true;

	sync();

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

CurveWarp::~CurveWarp()
{
	delete internal;
}

Layer::Handle
CurveWarp::hit_check(Context context, const Point &point)const
{
	return context.hit_check(internal->transform(point));
}

bool
CurveWarp::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_origin, internal->origin = value.get(Point()));
	IMPORT_VALUE_PLUS(param_start_point, sync());
	IMPORT_VALUE_PLUS(param_end_point, sync());
	IMPORT_VALUE_PLUS(param_fast, internal->fast = value.get(bool()));
	IMPORT_VALUE_PLUS(param_perp_width, internal->perp_width = value.get(Real()));
	IMPORT_VALUE_PLUS(param_bline, sync());

	if(param=="offset")
		return set_param("origin", value);

	return false;
}

ValueBase
CurveWarp::get_param(const String & param)const
{
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_start_point);
	EXPORT_VALUE(param_end_point);
	EXPORT_VALUE(param_bline);
	EXPORT_VALUE(param_fast);
	EXPORT_VALUE(param_perp_width);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Vocab
CurveWarp::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("origin")
				  .set_local_name(_("Origin"))
				  .set_description(_("Position of the destiny Spline line"))
				  .set_is_distance()
	);
	ret.push_back(ParamDesc("perp_width")
				  .set_local_name(_("Width"))
				  .set_origin("start_point")
				  .set_description(_("How much is expanded the result perpendicular to the source line"))
	);
	ret.push_back(ParamDesc("start_point")
				  .set_local_name(_("Start Point"))
				  .set_connect("end_point")
				  .set_description(_("First point of the source line"))
				  .set_is_distance()
	);
	ret.push_back(ParamDesc("end_point")
				  .set_local_name(_("End Point"))
				  .set_description(_("Final point of the source line"))
				  .set_is_distance()
	);
	ret.push_back(ParamDesc("bline")
				  .set_local_name(_("Vertices"))
				  .set_origin("origin")
				  .set_hint("width")
				  .set_description(_("List of Spline Points where the source line is curved to"))
	);
	ret.push_back(ParamDesc("fast")
				  .set_local_name(_("Fast"))
				  .set_description(_("When checked, renders quickly but with artifacts"))
	);
	return ret;
}

Color
CurveWarp::get_color(Context context, const Point &point)const
{
	return context.get_color(internal->transform(point));
}


rendering::Task::Handle
CurveWarp::build_rendering_task_vfunc(Context context) const
{
	rendering::Task::Handle task = context.build_rendering_task();

	TaskCurveWarp::Handle task_curvewarp(new TaskCurveWarp());
	task_curvewarp->internal = *internal;

	task_curvewarp->sub_task() = task;

	task = task_curvewarp;

	return task;
}
