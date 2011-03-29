/* === S Y N F I G ========================================================= */
/*!	\file outline.cpp
**	\brief Implementation of the "Advanced Outline" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011 Carlos López
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

#include "advanced_outline.h"
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include <ETL/calculus>
#include <ETL/bezier>
#include <ETL/hermite>
#include <vector>

#include <synfig/valuenode_bline.h>
#include <synfig/valuenode_wplist.h>
#include <synfig/valuenode_composite.h>

#endif

using namespace etl;


/* === M A C R O S ========================================================= */
#define SAMPLES		50
#define ROUND_END_FACTOR	(4)
#define CUSP_THRESHOLD		(0.40)
#define SPIKE_AMOUNT		(4)
#define NO_LOOP_COOKIE		synfig::Vector(84951305,7836658)
#define EPSILON				(0.000000001)
#define CUSP_TANGENT_ADJUST	(0.0025)
/* === G L O B A L S ======================================================= */
SYNFIG_LAYER_INIT(Advanced_Outline);
SYNFIG_LAYER_SET_NAME(Advanced_Outline,"advanced_outline");
SYNFIG_LAYER_SET_LOCAL_NAME(Advanced_Outline,N_("Advanced Outline"));
SYNFIG_LAYER_SET_CATEGORY(Advanced_Outline,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Advanced_Outline,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Advanced_Outline,"$Id$");
/* === P R O C E D U R E S ================================================= */
Point line_intersection( const Point& p1, const Vector& t1, const Point& p2, const Vector& t2 );
/* === M E T H O D S ======================================================= */

Advanced_Outline::Advanced_Outline()
{
	sharp_cusps_=true;
	start_tip_= end_tip_= WidthPoint::TYPE_ROUNDED;
	width_=1.0f;
	expand_=0;
	smoothness_=0.5;
	clear();

	vector<BLinePoint> bline_point_list;
	bline_point_list.push_back(BLinePoint());
	bline_point_list.push_back(BLinePoint());
	bline_point_list.push_back(BLinePoint());
	bline_point_list[0].set_vertex(Point(0,1));
	bline_point_list[1].set_vertex(Point(0,-1));
	bline_point_list[2].set_vertex(Point(1,0));
	bline_point_list[0].set_tangent(bline_point_list[1].get_vertex()-bline_point_list[2].get_vertex()*0.5f);
	bline_point_list[1].set_tangent(bline_point_list[2].get_vertex()-bline_point_list[0].get_vertex()*0.5f);
	bline_point_list[2].set_tangent(bline_point_list[0].get_vertex()-bline_point_list[1].get_vertex()*0.5f);
	bline_point_list[0].set_width(1.0f);
	bline_point_list[1].set_width(1.0f);
	bline_point_list[2].set_width(1.0f);
	bline_=bline_point_list;
	vector<WidthPoint> wpoint_list;
	wpoint_list.push_back(WidthPoint());
	wpoint_list.push_back(WidthPoint());
	wpoint_list[0].set_position(0.1);
	wpoint_list[1].set_position(0.9);
	wpoint_list[0].set_width(1.0);
	wpoint_list[1].set_width(1.0);
	wpoint_list[0].set_side_type_before(WidthPoint::TYPE_INTERPOLATE);
	wpoint_list[1].set_side_type_after(WidthPoint::TYPE_INTERPOLATE);
	wplist_=wpoint_list;
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
}


/*! The Sync() function takes the values
**	and creates a polygon to be rendered
**	with the polygon layer.
*/
void
Advanced_Outline::sync()
{
	clear();
	if (!bline_.get_list().size())
	{
		synfig::warning(string("Advanced_Outline::sync():")+N_("No vertices in bline " + string("\"") + get_description() + string("\"")));
		return;
	}
	try
	{
		vector<BLinePoint> bline(bline_.get_list().begin(),bline_.get_list().end());
		vector<WidthPoint> wplist(wplist_.get_list().begin(), wplist_.get_list().end());
		const bool blineloop(bline_.get_loop());
		int bline_size(bline.size());
		int wplist_size(wplist.size());
		vector<BLinePoint>::const_iterator biter,bnext(bline.begin());
		vector<WidthPoint>::iterator witer, wnext;
		Vector first_tangent;
		Vector last_tangent;
		Real bezier_size = 1.0/(blineloop?bline_size:(bline_size==1?1.0:(bline_size-1)));
		Real biter_pos(0.0), bnext_pos(bezier_size);
		const vector<BLinePoint>::const_iterator bend(bline.end());
		vector<Point> side_a, side_b;
		// Sort the wplist. It is needed to calculate the first widthpoint
		sort(wplist.begin(),wplist.end());
		// If we are looped, the first bezier to handle starts form the
		// last blinepoint and ends at the first one
		if(blineloop)
			biter=--bline.end();
		else
			biter=bnext++;
		// 				biter	bnext
		//				----	----
		// looped		nth		1st
		// !looped		1st		2nd
		// First tangent is used only to add the start of the bline tip
		first_tangent=bline.front().get_tangent2();
		// Last tangent is only to draw sharp cusps for the first blinepoint
		last_tangent=biter->get_tangent1();
		// if we are looped and drawing sharp cusps, we'll need a value for the incoming tangent
		if (blineloop && sharp_cusps_ && last_tangent.is_equal_to(Vector::zero()))
		{
			hermite<Vector> curve((biter-1)->get_vertex(), biter->get_vertex(), (biter-1)->get_tangent2(), biter->get_tangent1());
			const derivative< hermite<Vector> > deriv(curve);
			last_tangent=deriv(1.0-CUSP_TANGENT_ADJUST);
		}
		///////////////////////////////////////////// Prepare the wplist
		// if we have some widthpoint in the list
		// If not looped
		if(!blineloop)
		{
			if(wplist_size)
			{
				WidthPoint wpfront(wplist.front());
				WidthPoint wpback(wplist.back());
				// if the first widthpoint interpolation before is INTERPOLATE and it is not exactly at 0.0
				if(wpfront.get_side_type_before() == WidthPoint::TYPE_INTERPOLATE && wpfront.get_norm_position()!=0.0)
					// Add a fake widthpoint at position 0.0
					wplist.push_back(WidthPoint(0.0, wpfront.get_width() , start_tip_, WidthPoint::TYPE_INTERPOLATE));
				// if last widhtpoint interpolation after is INTERPOLATE and it is not exactly at 1.0
				if(wpback.get_side_type_after() == WidthPoint::TYPE_INTERPOLATE && wpback.get_norm_position()!=1.0)
				// Add a fake withpoint at position 1.0
					wplist.push_back(WidthPoint(1.0, wpback.get_width() , WidthPoint::TYPE_INTERPOLATE, end_tip_));
			}
			else
			{
				// If there are not widthpoints in list, just use the global width
				wplist.push_back(WidthPoint(0.0, 1.0 , start_tip_, WidthPoint::TYPE_INTERPOLATE));
				wplist.push_back(WidthPoint(1.0, 1.0 , WidthPoint::TYPE_INTERPOLATE, end_tip_));
			}
		}
		else // looped
		{
			if(wplist_size)
			{
				WidthPoint wpfront(wplist.front());
				WidthPoint wpback(wplist.back());
				bool wpfb_int(wpfront.get_side_type_before() == WidthPoint::TYPE_INTERPOLATE);
				bool wpba_int(wpback.get_side_type_after() == WidthPoint::TYPE_INTERPOLATE);
				// if any of front(back) widthpoint interpolation before(after) is  INTERPOLATE
				if(wpfb_int || wpba_int)
				{
					// if it is not exactly at 0.0
					if(wpfront.get_norm_position()!=0.0)
					// Add a fake widthpoint at position 0.0
						wplist.push_back(WidthPoint(0.0, widthpoint_interpolate(wpback, wpfront, 0.0, smoothness_) , WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_INTERPOLATE));
					// If it is not exactly at 1.0
					if(wpback.get_norm_position()!=1.0)
					// Add a fake widthpoint at position 1.0
						wplist.push_back(WidthPoint(1.0, widthpoint_interpolate(wpback, wpfront, 1.0, smoothness_) , WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_INTERPOLATE));
				}
			}
			else
			{
				// If there are not widthpoints in list, just use the global width
				wplist.push_back(WidthPoint(0.0, 1.0 , WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_INTERPOLATE));
				wplist.push_back(WidthPoint(1.0, 1.0 , WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_INTERPOLATE));
			}
		}

		// Sort the wplist again to place the two new widthpoints on place.
		sort(wplist.begin(),wplist.end());
		////////////////////////////////////////////////////////////////
		//list the wplist
		synfig::info("------");
		for(witer=wplist.begin();witer!=wplist.end();witer++)
			synfig::info("P:%f W:%f B:%d A:%d", witer->get_norm_position(), witer->get_width(), witer->get_side_type_before(), witer->get_side_type_after());
		synfig::info("------");
		////////////////////////////////////////////////////////////////

		Real step(1.0/SAMPLES/bline_size);
		wnext=wplist.begin();
		if(blineloop)
			witer=--wplist.end();
		else
			witer=wnext;
		const vector<WidthPoint>::const_iterator wend(wplist.end());
		Real ipos(blineloop?0.0:witer->get_norm_position());
		do
		{
			Vector iter_t(biter->get_tangent2());
			Vector next_t(bnext->get_tangent1());
			bool split_flag(biter->get_split_tangent_flag());
			// Setup the curve
			hermite<Vector> curve(
				biter->get_vertex(),
				bnext->get_vertex(),
				iter_t,
				next_t
			);
			const derivative< hermite<Vector> > deriv(curve);
			Real wnext_pos(wnext->get_norm_position());
			if(ipos==wnext_pos)
			{
				// Do tips
				Real bezier_ipos(bline_to_bezier(ipos, biter_pos, bezier_size));
				add_tip(side_a, side_b, curve(bezier_ipos), deriv(bezier_ipos).norm(), *wnext);
				// Update wplist iterators
				witer=wnext;
				wnext++;
				// increase ipos so we do attempt to do the next interpolation segment.
				// fix bad render when first widhtpoint is not interpolate after and
				// next widhtpoint is interpolate before. Noticiable for the FLAT case
				// or when the width is smaller than the step on the bezier.
				ipos=ipos+step;
				// If we are at the last widthpoint then end
				if(wnext==wend)
					break;
				else
					continue;
			}
			if(witer->get_side_type_after()!=WidthPoint::TYPE_INTERPOLATE &&
				wnext->get_side_type_before()!=WidthPoint::TYPE_INTERPOLATE)
			{
				ipos=wnext_pos;
				while(ipos > bnext_pos && bnext+1!=bend)
				{
					// keep track of last tangent
					last_tangent=bnext->get_tangent1();
					// Update iterators
					biter=bnext;
					bnext++;
					// Update blinepoints positions
					biter_pos = bnext_pos;
					bnext_pos+=bezier_size;
				}
				continue;
			}
			if((ipos==biter_pos || ipos==bnext_pos))
			{
				// Do cusp at ipos
				if(ipos==biter_pos /*&& ipos!=0.0*/ && sharp_cusps_ && split_flag)
				{
					add_cusp(side_a, side_b, biter->get_vertex(), iter_t, last_tangent, expand_+width_*0.5*widthpoint_interpolate(*witer, *wnext, ipos, smoothness_));
				}
				if(bnext+1==bend && ipos == bnext_pos)
					break;
			}
			do
			{
				if(ipos > wnext_pos)
				{
					ipos=wnext_pos;
					// Add interpolation for the last step
					//synfig::info("ipos=%f", ipos);
					const Vector d(deriv(bline_to_bezier(ipos, biter_pos, bezier_size)).perp().norm());
					const Vector p(curve(bline_to_bezier(ipos, biter_pos, bezier_size)));
					Real ww;
					if(wnext->get_side_type_before()!=WidthPoint::TYPE_INTERPOLATE)
						ww=0.0;
					else
						ww=wnext->get_width();
					const Real w(expand_+width_*0.5*ww);
					side_a.push_back(p+d*w);
					side_b.push_back(p-d*w);
					if(ipos <= bnext_pos)
						break;
				}
				if(ipos > bnext_pos)
				{
					ipos=bnext_pos;
					const Vector d(deriv(bline_to_bezier(ipos, biter_pos, bezier_size)).perp().norm());
					const Vector p(curve(bline_to_bezier(ipos, biter_pos, bezier_size)));
					const Real w(expand_+width_*0.5*widthpoint_interpolate(*witer, *wnext, ipos, smoothness_));
					side_a.push_back(p+d*w);
					side_b.push_back(p-d*w);
					// Update iterators
					biter=bnext;
					bnext++;
					// Update blinepoints positions
					biter_pos = bnext_pos;
					bnext_pos+=bezier_size;
					// remember last tangent value
					last_tangent=next_t;
					break;
				}
				// Add interpolation
				//synfig::info("ipos=%f", ipos);
				const Vector d(deriv(bline_to_bezier(ipos, biter_pos, bezier_size)).perp().norm());
				const Vector p(curve(bline_to_bezier(ipos, biter_pos, bezier_size)));
				const Real w(expand_+width_*0.5*widthpoint_interpolate(*witer, *wnext, ipos, smoothness_));
				side_a.push_back(p+d*w);
				side_b.push_back(p-d*w);
				ipos = ipos + step;
			} while (1);
		} while(1);

		if(blineloop)
		{
			reverse(side_b.begin(),side_b.end());
			add_polygon(side_a);
			add_polygon(side_b);
			return;
		}

		// concatenate sides before add to polygon
		for(;!side_b.empty();side_b.pop_back())
			side_a.push_back(side_b.back());
		add_polygon(side_a);
	}
	catch (...) { synfig::error("Advanced Outline::sync(): Exception thrown"); throw; }
}

bool
Advanced_Outline::set_param(const String & param, const ValueBase &value)
{
	if(param=="bline" && value.get_type()==ValueBase::TYPE_LIST)
	{
		bline_=value;
		return true;
	}
	IMPORT_AS(sharp_cusps_, "sharp_cusps");
	IMPORT_AS(start_tip_, "start_tip");
	IMPORT_AS(end_tip_, "end_tip");
	IMPORT_AS(width_,"width");
	IMPORT_AS(expand_, "expand");
	if(param=="smoothness" && value.get_type()==ValueBase::TYPE_REAL)
	{
		if(value > 1.0) smoothness_=1.0;
		else if(value < 0.0) smoothness_=0.0;
		else smoothness_=value;
		return true;
	}
	if(param=="wplist" && value.get_type()==ValueBase::TYPE_LIST)
	{
		wplist_=value;
		return true;
	}
	if(param=="vector_list")
		return false;
	return Layer_Polygon::set_param(param,value);
}

void
Advanced_Outline::set_time(Context context, Time time)const
{
	const_cast<Advanced_Outline*>(this)->sync();
	context.set_time(time);
}

void
Advanced_Outline::set_time(Context context, Time time, Vector pos)const
{
	const_cast<Advanced_Outline*>(this)->sync();
	context.set_time(time,pos);
}

ValueBase
Advanced_Outline::get_param(const String& param)const
{
	EXPORT_AS(bline_, "bline");
	EXPORT_AS(expand_, "expand");
	EXPORT_AS(smoothness_, "smoothness");
	EXPORT_AS(sharp_cusps_, "sharp_cusps");
	EXPORT_AS(start_tip_,"start_tip");
	EXPORT_AS(end_tip_,"end_tip");
	EXPORT_AS(width_, "width");
	EXPORT_AS(wplist_, "wplist");
	EXPORT_NAME();
	EXPORT_VERSION();
	if(param=="vector_list")
		return ValueBase();
	return Layer_Polygon::get_param(param);
}

Layer::Vocab
Advanced_Outline::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Polygon::get_param_vocab());
	// Pop off the polygon parameter from the polygon vocab
	ret.pop_back();
	ret.push_back(ParamDesc("bline")
		.set_local_name(_("Vertices"))
		.set_origin("origin")
		.set_description(_("A list of BLine Points"))
	);
	ret.push_back(ParamDesc("width")
		.set_is_distance()
		.set_local_name(_("Outline Width"))
		.set_description(_("Global width of the outline"))
	);
	ret.push_back(ParamDesc("expand")
		.set_is_distance()
		.set_local_name(_("Expand"))
		.set_description(_("Value to add to the global width"))
	);
	ret.push_back(ParamDesc(ValueBase(),"start_tip")
		.set_local_name(_("Tip Type at Start"))
		.set_description(_("Defines the Tip type of the first bline point when bline is unlooped"))
		.set_hint("enum")
		.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
		.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
		.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
		.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
		);
	ret.push_back(ParamDesc(ValueBase(),"end_tip")
		.set_local_name(_("Tip Type at End"))
		.set_description(_("Defines the Tip type of the last bline point when bline is unlooped"))
		.set_hint("enum")
		.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
		.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
		.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
		.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
		);
	ret.push_back(ParamDesc("sharp_cusps")
		.set_local_name(_("Sharp Cusps"))
		.set_description(_("Determines cusp type"))
	);
	ret.push_back(ParamDesc("smoothness")
		.set_local_name(_("Smoothness"))
		.set_description(_("Determines the interpolation between withpoints. (0) Linear (1) Smooth"))
	);
	ret.push_back(ParamDesc("wplist")
		.set_local_name(_("Width Point List"))
		.set_hint("width")
		.set_origin("origin")
		.set_description(_("List of width Points that defines the variable width"))
	);
	return ret;
}

bool
Advanced_Outline::connect_dynamic_param(const String& param, etl::loose_handle<ValueNode> x)
{
	synfig::info("attempting to connect %s", param.c_str());
	if(param=="bline")
	{
		if(!connect_bline_to_wplist(x))
			synfig::warning("Advanced Outline: WPList doesn't accept new bline");
	}
	if(param=="wplist")
	{
		if(Layer::connect_dynamic_param(param, x))
		{
			DynamicParamList::const_iterator iter(dynamic_param_list().find("bline"));
			if(iter==dynamic_param_list().end())
				{
					synfig::warning("BLine doesn't exists yet!!");
					return false;
				}
			else if(!connect_bline_to_wplist(iter->second))
			{
				synfig::warning("Advanced Outline: WPList doesn't accept new bline");
				return false;
			}
			else
				return true;
		}
		else
			return false;
	}
	return Layer::connect_dynamic_param(param, x);
}

bool
Advanced_Outline::connect_bline_to_wplist(etl::loose_handle<ValueNode> x)
{
	if(x->get_type() != ValueBase::TYPE_LIST)
	{
		synfig::info("Not a list");
		return false;
	}
	if((*x)(Time(0)).get_list().front().get_type() != ValueBase::TYPE_BLINEPOINT)
	{
		synfig::info("No blinepoints!");
		return false;
	}
	ValueNode::LooseHandle vnode;
	DynamicParamList::const_iterator iter(dynamic_param_list().find("wplist"));
	if(iter==dynamic_param_list().end())
	{
		synfig::warning("WPList doesn't exists yet");
		return false;
	}
	ValueNode_WPList::Handle wplist(ValueNode_WPList::Handle::cast_dynamic(iter->second));
	if(!wplist)
	{
		synfig::info("WPList is not ready: NULL");
		return false;
	}
	if(!wplist->link_count())
		synfig::warning("Advanced_Outline::connect_bline_to_wplist: WPList::link_count()=0");
	wplist->set_bline(ValueNode::Handle(x));
	synfig::info("set bline success");
	return true;
}


Real
Advanced_Outline::bline_to_bezier(Real bline_pos, Real origin, Real bezier_size)
{
	if(bezier_size)
		return (bline_pos-origin)/bezier_size;
	return bline_pos;
}
Real
Advanced_Outline::bezier_to_bline(Real bezier_pos, Real origin, Real bezier_size)
{
	return origin+bezier_pos*bezier_size;
}

void
Advanced_Outline::add_tip(std::vector<Point> &side_a, std::vector<Point> &side_b, const Point vertex, const Vector tangent, const WidthPoint wp)
{
	Real w(expand_+width_*0.5*wp.get_width());
	// Side Before
	switch (wp.get_side_type_before())
	{
		case WidthPoint::TYPE_ROUNDED:
		{
			hermite<Vector> curve(
				vertex-tangent.perp()*w,
				vertex+tangent.perp()*w,
				-tangent*w*ROUND_END_FACTOR,
				tangent*w*ROUND_END_FACTOR
			);
			side_a.push_back(vertex);
			side_b.push_back(vertex);
			for(float n=0.0f;n<0.499999f;n+=2.0f/SAMPLES)
			{
				side_a.push_back(curve(0.5+n));
				side_b.push_back(curve(0.5-n));
			}
			side_a.push_back(curve(1.0));
			side_b.push_back(curve(0.0));
			side_a.push_back(vertex);
			side_b.push_back(vertex);
			break;
		}
		case WidthPoint::TYPE_SQUARED:
		{
			side_a.push_back(vertex);
			side_a.push_back(vertex-tangent*w);
			side_a.push_back(vertex+(tangent.perp()-tangent)*w);
			side_a.push_back(vertex+tangent.perp()*w);
			side_a.push_back(vertex);
			side_b.push_back(vertex);
			side_b.push_back(vertex-tangent*w);
			side_b.push_back(vertex+(-tangent.perp()-tangent)*w);
			side_b.push_back(vertex-tangent.perp()*w);
			side_b.push_back(vertex);
			break;
		}
		case WidthPoint::TYPE_PEAK:
		{
			side_a.push_back(vertex);
			side_a.push_back(vertex-tangent*w);
			side_a.push_back(vertex+tangent.perp()*w);
			side_a.push_back(vertex);
			side_b.push_back(vertex);
			side_b.push_back(vertex-tangent*w);
			side_b.push_back(vertex-tangent.perp()*w);
			side_b.push_back(vertex);
			break;
		}
		case WidthPoint::TYPE_FLAT:
		{
			side_a.push_back(vertex);
			side_b.push_back(vertex);
			break;
		}
		case WidthPoint::TYPE_INTERPOLATE:
		default:
			break;
	}
	// Side After
	switch (wp.get_side_type_after())
	{
		case WidthPoint::TYPE_ROUNDED:
		{
			hermite<Vector> curve(
				vertex-tangent.perp()*w,
				vertex+tangent.perp()*w,
				tangent*w*ROUND_END_FACTOR,
				-tangent*w*ROUND_END_FACTOR
			);
			for(float n=0.0f;n<0.499999f;n+=2.0f/SAMPLES)
			{
				side_a.push_back(curve(1-n));
				side_b.push_back(curve(n));
			}
			side_a.push_back(curve(0.5));
			side_b.push_back(curve(0.5));
			side_a.push_back(vertex);
			side_b.push_back(vertex);
			break;
		}
		case WidthPoint::TYPE_SQUARED:
		{
			side_a.push_back(vertex);
			side_a.push_back(vertex+tangent*w);
			side_a.push_back(vertex+(-tangent.perp()+tangent)*w);
			side_a.push_back(vertex-tangent.perp()*w);
			side_a.push_back(vertex);
			side_b.push_back(vertex);
			side_b.push_back(vertex+tangent*w);
			side_b.push_back(vertex+(tangent.perp()+tangent)*w);
			side_b.push_back(vertex+tangent.perp()*w);
			side_b.push_back(vertex);
			break;
		}
		case WidthPoint::TYPE_PEAK:
		{
			side_a.push_back(vertex);
			side_a.push_back(vertex+tangent*w);
			side_a.push_back(vertex-tangent.perp()*w);
			side_a.push_back(vertex);
			side_b.push_back(vertex);
			side_b.push_back(vertex+tangent*w);
			side_b.push_back(vertex+tangent.perp()*w);
			side_b.push_back(vertex);
			break;
		}
		case WidthPoint::TYPE_FLAT:
		{
			side_a.push_back(vertex);
			side_b.push_back(vertex);
			break;
		}
		case WidthPoint::TYPE_INTERPOLATE:
		default:
			break;
	}
}
void
Advanced_Outline::add_cusp(std::vector<Point> &side_a, std::vector<Point> &side_b, const Point vertex, const Vector curr, const Vector last, Real w)
{
	const Vector t1(last.perp().norm());
	const Vector t2(curr.perp().norm());
	Real cross(t1*t2.perp());
	Real perp((t1-t2).mag());
	if(cross>CUSP_THRESHOLD)
	{
		const Point p1(vertex+t1*w);
		const Point p2(vertex+t2*w);
		side_a.push_back(line_intersection(p1,last,p2,curr));
	}
	else if(cross<-CUSP_THRESHOLD)
	{
		const Point p1(vertex-t1*w);
		const Point p2(vertex-t2*w);
		side_b.push_back(line_intersection(p1,last,p2,curr));
	}
	else if(cross>0 && perp>1)
	{
		float amount(max(0.0f,(float)(cross/CUSP_THRESHOLD))*(SPIKE_AMOUNT-1)+1);
		side_a.push_back(vertex+(t1+t2).norm()*w*amount);
	}
	else if(cross<0 && perp>1)
	{
		float amount(max(0.0f,(float)(-cross/CUSP_THRESHOLD))*(SPIKE_AMOUNT-1)+1);
		side_b.push_back(vertex-(t1+t2).norm()*w*amount);
	}
}
