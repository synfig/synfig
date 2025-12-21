/* === S Y N F I G ========================================================= */
/*!	\file advanced_outline.cpp
**	\brief Implementation of the "Advanced Outline" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
**	......... ... 2019 Ivan Mahonin
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

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/dashitem.h>

#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/valuenodes/valuenode_wplist.h>
#include <synfig/valuenodes/valuenode_dilist.h>

#include <synfig/rendering/primitive/bend.h>

#include "advanced_outline.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include "advancedline.h"

#endif


using namespace synfig;


/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Advanced_Outline);
SYNFIG_LAYER_SET_NAME(Advanced_Outline,"advanced_outline");
SYNFIG_LAYER_SET_LOCAL_NAME(Advanced_Outline,N_("Advanced Outline"));
SYNFIG_LAYER_SET_CATEGORY(Advanced_Outline,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Advanced_Outline,"0.3");

/* === P R O C E D U R E S ================================================= */

namespace {
	Real calc_position(Real p, const rendering::Bend &bend, bool homogeneous) {
		return homogeneous
			 ? p*bend.length1()
			 : bend.length_by_index( p*bend.l1() );
	}
}

/* === M E T H O D S ======================================================= */

Advanced_Outline::Advanced_Outline():
	param_bline(ValueBase(std::vector<synfig::BLinePoint>())),
	param_wplist(ValueBase(std::vector<synfig::WidthPoint>())),
	param_dilist(ValueBase(std::vector<synfig::DashItem>())),
	param_start_tip(ValueBase(int(WidthPoint::TYPE_ROUNDED))),
	param_end_tip(ValueBase(int(WidthPoint::TYPE_ROUNDED))),
	param_cusp_type(ValueBase(int(TYPE_SHARP))),
	param_width(ValueBase(Real(1.0f))),
	param_expand(ValueBase(Real(0))),
	param_smoothness(ValueBase(Real(1))),
	param_homogeneous(ValueBase(false)),
	param_dash_offset(ValueBase(Real(0))),
	param_dash_enabled(ValueBase(false))
{
	clear();

	std::vector<BLinePoint> bline_point_list;
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
	param_bline.set_list_of(bline_point_list);
	
	std::vector<WidthPoint> wpoint_list;
	wpoint_list.push_back(WidthPoint());
	wpoint_list.push_back(WidthPoint());
	wpoint_list[0].set_position(0.1);
	wpoint_list[1].set_position(0.9);
	wpoint_list[0].set_width(1.0);
	wpoint_list[1].set_width(1.0);
	wpoint_list[0].set_side_type_before(WidthPoint::TYPE_INTERPOLATE);
	wpoint_list[1].set_side_type_after(WidthPoint::TYPE_INTERPOLATE);
	param_wplist.set_list_of(wpoint_list);
	
	std::vector<DashItem> ditem_list;
	ditem_list.push_back(DashItem());
	param_dilist.set_list_of(ditem_list);
	
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Advanced_Outline::~Advanced_Outline()
	{ }

void
Advanced_Outline::sync_vfunc()
{
	clear();
	
	const int wire_segments = 16;
	const int contour_segments = 8;
	const BLinePoint bp_blank;
	const WidthPoint wp_blank;
	const DashItem di_blank;

	const ValueBase::List &bline = param_bline.get_list();    // outline curve
	const ValueBase::List &wplist = param_wplist.get_list();  // width points
	const ValueBase::List &dilist = param_dilist.get_list();  // dash shapes for dashed outline

	const bool loop         = param_bline.get_loop();         // outline is looped
	const int start_tip     = param_start_tip.get(int());     // shape of tails:
	const int end_tip       = param_end_tip.get(int());       //   flat, peak, squared, rounded
	const int cusp_type     = param_cusp_type.get(int());     // shape of corners: sharp, rounded
	const Real width        = param_width.get(Real());        // width multiply
	const Real expand       = param_expand.get(Real());       // width add
	const Real smoothness   = param_smoothness.get(Real());   // lenear interpolation between bezier and line (for width)
	const bool homogeneous  = param_homogeneous.get(bool());  // use real length of outline
	const bool dash_enabled = param_dash_enabled.get(bool()); // enable dash
	const Real dash_offset  = param_dash_offset.get(Real());  // offset of dashes
	
	if (bline.empty())
		return;
	
	
	try
	{
		// retrieve the parent canvas grow value
		const Real gv = exp(get_outline_grow_mark());
		const Real wk = 0.5*gv*width;
		const Real we = gv*expand;
		const bool use_bline_width = wplist.empty();
		
		// build bend
		rendering::Bend bend;
		AdvancedLine aline;
		for(ValueBase::List::const_iterator i = bline.begin(); i != bline.end(); ++i) {
			const BLinePoint &point = i->get(bp_blank);
			bend.add(
				point.get_vertex(),
				point.get_tangent1(),
				point.get_tangent2(),
				cusp_type == TYPE_SHARP   ? rendering::Bend::CORNER :
				cusp_type == TYPE_ROUNDED ? rendering::Bend::ROUND  : rendering::Bend::FLAT,
				true,
				wire_segments );
			if (use_bline_width)
				aline.add(
					bend.length1(),
					point.get_width()*wk + we,
					WidthPoint::TYPE_INTERPOLATE,
					WidthPoint::TYPE_INTERPOLATE );
		}
		if (loop) {
			bend.loop(true, wire_segments);
			if (use_bline_width)
				aline.add(
					bend.length1(),
					bline.front().get(bp_blank).get_width()*wk + we,
					WidthPoint::TYPE_INTERPOLATE,
					WidthPoint::TYPE_INTERPOLATE );
		} else {
			bend.tails();
		}
		const Real kl = bend.length1();
		
		// apply wplist
		if (!use_bline_width) {
			for(ValueBase::List::const_iterator i = wplist.begin(); i != wplist.end(); ++i) {
				const WidthPoint &point = i->get(wp_blank);
				aline.add(
					calc_position( clamp(point.get_position(), Real(0), Real(1)), bend, homogeneous ),
					point.get_width()*wk + we,
					(WidthPoint::SideType)point.get_side_type_before(),
					(WidthPoint::SideType)point.get_side_type_after(),
					AdvancedLine::AddAction::APPEND);
			}
		}
		
		if (loop) {
			AdvancedLine::iterator b0 = aline.begin(), b1 = b0;
			AdvancedLine::iterator e0 = aline.end(), e1 = --e0;
			Real kl2 = 2*kl;
			if (aline.size() > 1) { ++b1; --e1; kl2 = kl; }
			
			// add two points from end to begin (to simulate loopped width points)
			aline.add(e0->first - kl , e0->second.w, e0->second.side0, e0->second.side1, AdvancedLine::AddAction::PREPEND);
			aline.add(e1->first - kl2, e1->second.w, WidthPoint::TYPE_FLAT, e1->second.side1, AdvancedLine::AddAction::PREPEND);
			// add two points from begin to end
			aline.add(b0->first + kl , b0->second.w, b0->second.side0, b0->second.side1, AdvancedLine::AddAction::APPEND);
			aline.add(b1->first + kl2, b1->second.w, b1->second.side0, WidthPoint::TYPE_FLAT, AdvancedLine::AddAction::APPEND);

			aline.calc_tangents(smoothness);
		} else {
			// make tails longer for proper trunc
			AdvancedLine::const_iterator i = aline.begin();
			AdvancedLine::const_iterator j = aline.end(); --j;
			if (i->second.side0 == WidthPoint::TYPE_INTERPOLATE) {
				if (approximate_greater(i->first, 0.0) && i == j) {
					// Somehow with only one width point and with its left side as Interpolate,
					// calc_tangents and trunc_left make it with wrong width at start.
					// So here is a mini hack/workaround
					aline.add(0, i->second.w, WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_INTERPOLATE);
				}
				aline.add(-2, i->second.w, WidthPoint::TYPE_FLAT, WidthPoint::TYPE_INTERPOLATE);
				aline.add(-1, i->second.w, WidthPoint::TYPE_FLAT, WidthPoint::TYPE_INTERPOLATE);
			}
			if (j->second.side1 == WidthPoint::TYPE_INTERPOLATE) {
				aline.add(kl + 1, j->second.w, WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_FLAT);
				aline.add(kl + 2, j->second.w, WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_FLAT);
			}
			aline.calc_tangents(smoothness);
			aline.trunc_left(0, (WidthPoint::SideType)start_tip);
			aline.trunc_right(kl, (WidthPoint::SideType)end_tip);
		}
		
		// add dashes
		if (dash_enabled && !dilist.empty()) {
			Real dashes_length = 0;
			for(ValueBase::List::const_iterator i = dilist.begin(); i != dilist.end(); ++i) {
				const DashItem &dash = i->get(di_blank);
				dashes_length += dash.get_offset() + dash.get_length();
			}
			if (!approximate_zero_lp(dashes_length)) {
				Real p0 = dash_offset/dashes_length;
				p0 = (p0 - ceil(p0))*dashes_length;
				DashItem::SideType type0 = (DashItem::SideType)dilist.back().get(di_blank).get_side_type_after();
				while(p0 < kl) {
					for(ValueBase::List::const_iterator i = dilist.begin(); i != dilist.end(); ++i) {
						const DashItem &dash = i->get(di_blank);
						Real p1 = p0 + dash.get_offset();
						aline.cut(
							p0,
							p1,
							DashItem::to_wp_side_type( type0 ),
							DashItem::to_wp_side_type( (DashItem::SideType)dash.get_side_type_before() ) );
						p0 = p1 + dash.get_length();
						if (p0 >= kl) break;
						type0 = (DashItem::SideType)dash.get_side_type_after();
					}
				}
			}
		}
		
		// create contour
		rendering::Contour contour;
		aline.build_contour(contour);
		
		// bend contour
		bend.bend(shape_contour(), contour, Matrix(), contour_segments);
	}
	catch (...) { synfig::error("Advanced Outline::sync(): Exception thrown"); throw; }
}

bool
Advanced_Outline::set_shape_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_bline);
	IMPORT_VALUE(param_wplist);
	IMPORT_VALUE(param_dilist);
	IMPORT_VALUE(param_start_tip);
	IMPORT_VALUE(param_end_tip);
	IMPORT_VALUE(param_cusp_type);
	IMPORT_VALUE(param_width);
	IMPORT_VALUE(param_expand);
	IMPORT_VALUE_PLUS(param_smoothness,
		{
			if(value.get(Real()) > 1.0) param_smoothness.set(Real(1.0));
			else if(value.get(Real()) < 0.0) param_smoothness.set(Real(0.0));
		}
	);
	IMPORT_VALUE(param_homogeneous);
	IMPORT_VALUE(param_dash_offset);
	IMPORT_VALUE(param_dash_enabled);

	// Skip polygon parameters
	return Layer_Shape::set_shape_param(param,value);
}

ValueBase
Advanced_Outline::get_param(const String& param)const
{
	EXPORT_VALUE(param_bline);
	EXPORT_VALUE(param_wplist);
	EXPORT_VALUE(param_dilist);
	EXPORT_VALUE(param_start_tip);
	EXPORT_VALUE(param_end_tip);
	EXPORT_VALUE(param_cusp_type);
	EXPORT_VALUE(param_width);
	EXPORT_VALUE(param_expand);
	EXPORT_VALUE(param_smoothness);
	EXPORT_VALUE(param_homogeneous);
	EXPORT_VALUE(param_dash_offset);
	EXPORT_VALUE(param_dash_enabled);

	EXPORT_NAME();
	EXPORT_VERSION();

	// Skip polygon parameters
	return Layer_Shape::get_param(param);
}

Layer::Vocab
Advanced_Outline::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Shape::get_param_vocab());

	ret.push_back(ParamDesc("bline")
		.set_local_name(_("Vertices"))
		.set_origin("origin")
		.set_hint(param_wplist.get_list().empty() ? "width" : "")
		.set_description(_("A list of spline points"))
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
	ret.push_back(ParamDesc("start_tip")
		.set_local_name(_("Tip Type at Start"))
		.set_description(_("Defines the Tip type of the first spline point when spline is unlooped"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
		.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
		.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
		.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
		.add_enum_value(WidthPoint::TYPE_INNER_ROUNDED,"inner_rounded", _("Inner Rounded Stop"))
		.add_enum_value(WidthPoint::TYPE_INNER_PEAK,"inner_peak", _("Off-Peak Stop"))
		);
	ret.push_back(ParamDesc("end_tip")
		.set_local_name(_("Tip Type at End"))
		.set_description(_("Defines the Tip type of the last spline point when spline is unlooped"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
		.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
		.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
		.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
		.add_enum_value(WidthPoint::TYPE_INNER_ROUNDED,"inner_rounded", _("Inner Rounded Stop"))
		.add_enum_value(WidthPoint::TYPE_INNER_PEAK,"inner_peak", _("Off-Peak Stop"))
		);
	ret.push_back(ParamDesc("cusp_type")
		.set_local_name(_("Cusps Type"))
		.set_description(_("Determines cusps type"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(TYPE_SHARP,"sharp", _("Sharp"))
		.add_enum_value(TYPE_ROUNDED,"rounded", _("Rounded"))
		.add_enum_value(TYPE_BEVEL,"bevel", _("Bevel"))
	);
	ret.push_back(ParamDesc("smoothness")
		.set_local_name(_("Smoothness"))
		.set_description(_("Determines the interpolation between widthpoints. (0) Linear (1) Smooth"))
	);
	ret.push_back(ParamDesc("homogeneous")
		.set_local_name(_("Homogeneous"))
		.set_description(_("When checked, widthpoints positions are spline length based"))
		.set_static(true)
	);
	ret.push_back(ParamDesc("wplist")
		.set_local_name(_("Width Point List"))
		.set_hint("width")
		.set_origin("origin")
		.set_description(_("List of width Points that defines the variable width"))
	);
	ret.push_back(ParamDesc("dash_enabled")
		.set_local_name(_("Dashed Outline"))
		.set_description(_("When checked, outline is dashed"))
	);
	ret.push_back(ParamDesc("dilist")
		.set_local_name(_("Dash Item List"))
		.set_origin("origin")
		.set_description(_("List of dash items that defines the dashed outline"))
	);
	ret.push_back(ParamDesc("dash_offset")
		.set_local_name(_("Dash Items Offset"))
		.set_is_distance()
		.set_description(_("Distance to Offset the Dash Items"))
	);
	return ret;
}

bool
Advanced_Outline::connect_dynamic_param(const String& param, ValueNode::LooseHandle x)
{
	if(param=="bline")
	{
		// only accept a valuenode that gives us a list of blinepoints, or null to disconnect
		bool is_bline_vn = false;
		if (x && x->get_type() == type_list) {
			ValueBase v = (*x)(Time(0));
			if (v.get_contained_type() == type_bline_point) {
				is_bline_vn = true;
			}
		}
		if (!x || is_bline_vn) {
			connect_bline_to_wplist(x);
		}
		return Layer::connect_dynamic_param(param, x);
	}
	if(param=="wplist")
	{
		if(Layer::connect_dynamic_param(param, x))
		{
			DynamicParamList::const_iterator iter(dynamic_param_list().find("bline"));
			if(iter==dynamic_param_list().end())
				return false;
			else if(iter->second && !connect_bline_to_wplist(iter->second))
				return false;
			return true;
		}
		else
			return false;
	}
	// no special treatment for param == "dilist"

	return Layer::connect_dynamic_param(param, x);
}

bool
Advanced_Outline::connect_bline_to_wplist(ValueNode::LooseHandle x)
{
	// connect_dynamic_param() makes sure x is a list of blinepoints.
	ValueNode::LooseHandle vnode;
	DynamicParamList::const_iterator iter(dynamic_param_list().find("wplist"));
	if(iter==dynamic_param_list().end())
		return false;
	ValueNode_WPList::Handle wplist(ValueNode_WPList::Handle::cast_dynamic(iter->second));
	if(!wplist)
		return false;
	wplist->set_bline(ValueNode::Handle(x));
	return true;
}
