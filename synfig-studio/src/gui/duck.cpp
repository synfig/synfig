/* === S Y N F I G ========================================================= */
/*!	\file duck.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
**  Copyright (c) 2011 Carlos López
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

#include <synfig/general.h>

#include "duck.h"
#include <ETL/misc>

#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/valuenodes/valuenode_wplist.h>
#include <synfig/valuenodes/valuenode_blinecalctangent.h>
#include <synfig/valuenodes/valuenode_blinecalcvertex.h>
#include <synfig/valuenodes/valuenode_blinecalcwidth.h>
#include <synfig/valuenodes/valuenode_composite.h>

#include <gui/localization.h>
#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

int studio::Duck::duck_count(0);

struct _DuckCounter
{
	static int counter;
	~_DuckCounter()
	{
		if(counter)
			synfig::error("%d ducks not yet deleted!",counter);
	}
} _duck_counter;

int _DuckCounter::counter(0);


/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Duck::Duck():
	guid_(GUID::zero()),
	type_(TYPE_NONE),
	editable_(false),
	alternative_editable_(false),
	edit_immediatelly_(false),
	radius_(false),
	tangent_(false),
	hover_(false),
	ignore_(false),
	exponential_(false),
	track_axes_(false),
	lock_aspect_(false),
	move_origin_(false),
	scalar_(1),
	origin_(0,0),
	axis_x_angle_(Angle::deg(0)),
	axis_x_mag_(1),
	axis_y_angle_(Angle::deg(90)),
	axis_y_mag_(1),
	rotations_(synfig::Angle::deg(0)),
	aspect_point_(1,1)
{
	duck_count++;
	_DuckCounter::counter++;
}

Duck::Duck(const synfig::Point &point):
	Duck()
{
	type_ = TYPE_POSITION;
	point_ = point;
}

Duck::Duck(const synfig::Point &point, const synfig::Point &origin):
	Duck()
{
	origin_ = origin;
	point_ = point;
}

Duck::~Duck()
{
	duck_count--;
	_DuckCounter::counter--;
}

synfig::GUID
Duck::get_data_guid()const
{
	synfig::GUID type_guid = synfig::GUID::hasher(get_type());
	if(value_desc_.is_value_node())
		return type_guid ^ value_desc_.get_value_node()->get_guid();
	return type_guid ^ synfig::GUID::hasher(get_name());
}

void
Duck::set_name(const synfig::String &x)
{
	name=x;
	if(guid_==synfig::GUID::zero())
	{
		guid_=synfig::GUID::hasher(name);
	}
}


bool
Duck::operator==(const Duck &rhs)const
{
	if(this==&rhs)
		return true;
	return
		name==rhs.name &&
		scalar_==rhs.scalar_ &&
		type_==rhs.type_ &&
		transform_stack_.size()==rhs.transform_stack_.size();
		//true;
		//(origin_duck?*origin_duck==*rhs.origin_duck:origin==rhs.origin) &&
		//(shared_point?*shared_point==*rhs.shared_point:point==rhs.point) ;
}

//! Sets the location of the duck with respect to the origin
void
Duck::set_point(const synfig::Point &x)
{
	if (get_move_origin() && origin_duck_) {
		Point offset = get_trans_point(x) - get_trans_point();
		origin_duck_->set_trans_point(origin_duck_->get_trans_point() + offset);
		return;
	}
	
	if (is_aspect_locked())
		point_ = aspect_point_ * (x * aspect_point_);
	else
		point_ = x;
	if (shared_point_) *shared_point_ = point_;
	if (shared_angle_ && approximate_not_zero(point_.mag())) *shared_angle_ = point_.angle();
	if (shared_mag_)   *shared_mag_ = point_.mag();
}

//! Returns the location of the duck
synfig::Point
Duck::get_point()const
{
	synfig::Point p;
	if (!shared_point_ && !shared_angle_ && !shared_mag_)
		p = point_;
	else
	if (shared_point_)
		p = *shared_point_;
	else
		p = synfig::Point(
				shared_mag_ ? *shared_mag_ : point_.mag(),
				shared_angle_ ? *shared_angle_ : point_.angle() );

	if (is_aspect_locked())
		p = aspect_point_ * (p * aspect_point_);

	return p;
}

synfig::Point
Duck::get_trans_point()const
{
	return transform_stack_.perform(get_sub_trans_point());
}

synfig::Point
Duck::get_trans_point(const synfig::Point &x)const
{
	return transform_stack_.perform(get_sub_trans_point(x));
}

void
Duck::set_trans_point(const synfig::Point &x)
{
	set_sub_trans_point(transform_stack_.unperform(x));
}

void
Duck::set_trans_point(const synfig::Point &x, const synfig::Time &time)
{
	set_sub_trans_point(transform_stack_.unperform(x), time);
}

//! Retrieves the origin location
synfig::Point
Duck::get_trans_origin()const
{
	return transform_stack_.perform(get_sub_trans_origin());
}

synfig::Point
Duck::get_sub_trans_point_without_offset(const synfig::Point &x)const {
	Point p(x*get_scalar());
	return get_axis_x()*p[0]
	     + get_axis_y()*p[1];
}

synfig::Point
Duck::get_sub_trans_point(const synfig::Point &x)const
{
	return get_sub_trans_point_without_offset(x)
	     + get_sub_trans_origin();
}

synfig::Point
Duck::get_sub_trans_point()const
{
	return get_sub_trans_point(get_point());
}

synfig::Point
Duck::get_sub_trans_point_without_offset()const {
	return get_sub_trans_point_without_offset(get_point());
}

void
Duck::set_sub_trans_point(const synfig::Point &x)
{
	Matrix m(get_axis_x(), get_axis_y(), get_sub_trans_origin());
	m.invert();

	Angle old_angle = get_point().angle();
	set_point(m.get_transformed(x)/get_scalar());
	Angle change = get_point().angle() - old_angle;
	while (change < Angle::deg(-180)) change += Angle::deg(360);
	while (change > Angle::deg(180)) change -= Angle::deg(360);
	rotations_ += change;
}

void
Duck::set_sub_trans_point(const synfig::Point &x, const synfig::Time &time)
{
	set_sub_trans_point(x);

	if(get_type() == Duck::TYPE_VERTEX
	|| get_type() == Duck::TYPE_POSITION
	|| get_type() == Duck::TYPE_WIDTHPOINT_POSITION)
	{
		ValueNode_BLineCalcVertex::Handle bline_vertex;
		ValueNode_Composite::Handle composite;

		if ((bline_vertex = ValueNode_BLineCalcVertex::Handle::cast_dynamic(get_value_desc().get_value_node())) ||
			((composite = ValueNode_Composite::Handle::cast_dynamic(get_value_desc().get_value_node())) &&
			 composite->get_type() == type_bline_point &&
			 (bline_vertex = ValueNode_BLineCalcVertex::Handle::cast_dynamic(composite->get_link("point")))))
		{
			synfig::Point closest_point = get_point();
			synfig::Real radius = 0.0;
			ValueNode_BLine::Handle bline = ValueNode_BLine::Handle::cast_dynamic(bline_vertex->get_link("bline"));
			synfig::find_closest_point(
				(*bline)(time),
				get_point(),
				radius,
				bline->get_loop(),
				&closest_point);
			set_point(closest_point);
		}
		ValueNode_Composite::Handle wpoint_composite;
		ValueNode_WPList::Handle wplist;
		wpoint_composite=ValueNode_Composite::Handle::cast_dynamic(get_value_desc().get_value_node());
		if(wpoint_composite && wpoint_composite->get_type() == type_width_point)
			if(get_value_desc().parent_is_value_node())
			{
				wplist=ValueNode_WPList::Handle::cast_dynamic(get_value_desc().get_parent_value_node());
				if(wplist)
				{
					ValueNode_BLine::Handle bline(ValueNode_BLine::Handle::cast_dynamic(wplist->get_bline()));
					synfig::Point closest_point = get_point();
					synfig::Real radius = 0.0;
					synfig::find_closest_point(
						(*bline)(time),
						get_point(),
						radius,
						bline->get_loop(),
						&closest_point);
					set_point(closest_point);
				}
			}
	}
}

synfig::Point
Duck::get_sub_trans_point(const Handle &duck, const Point &def, bool translate)const
{
	// The origin needs to have the same transform stack as this duck
	return !duck ? def
		 : translate ? transform_stack_.unperform(duck->get_trans_point())
	     : transform_stack_.unperform(duck->get_trans_point())
		 - transform_stack_.unperform(duck->get_trans_origin());
}

synfig::Point
Duck::get_sub_trans_origin()const
{
	return get_sub_trans_point(origin_duck_,origin_);
}

#ifdef _DEBUG
synfig::String
Duck::type_name(Type id)
{
	String ret;

	if (id & TYPE_POSITION) { if (!ret.empty()) ret += ", "; ret += "position"; }
	if (id & TYPE_TANGENT ) { if (!ret.empty()) ret += ", "; ret += "tangent" ; }
	if (id & TYPE_RADIUS  ) { if (!ret.empty()) ret += ", "; ret += "radius"  ; }
	if (id & TYPE_WIDTH	  ) { if (!ret.empty()) ret += ", "; ret += "width"   ; }
	if (id & TYPE_ANGLE	  ) { if (!ret.empty()) ret += ", "; ret += "angle"   ; }
	if (id & TYPE_VERTEX  ) { if (!ret.empty()) ret += ", "; ret += "vertex"  ; }
	if (id & TYPE_WIDTHPOINT_POSITION) { if (!ret.empty()) ret += ", "; ret += "widthpoint position"  ; }
	if (id & TYPE_SCALE   ) { if (!ret.empty()) ret += ", "; ret += "scale"   ; }
	if (id & TYPE_SCALE_X ) { if (!ret.empty()) ret += ", "; ret += "scale-x" ; }
	if (id & TYPE_SCALE_Y ) { if (!ret.empty()) ret += ", "; ret += "scale-y" ; }
	if (id & TYPE_SKEW    ) { if (!ret.empty()) ret += ", "; ret += "skew" ; }

	if (ret.empty())
		ret = "none";

	return ret;
}
#endif	// _DEBUG
