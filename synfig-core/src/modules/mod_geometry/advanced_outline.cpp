/* === S Y N F I G ========================================================= */
/*!	\file outline.cpp
**	\brief Implementation of the "Advanced Outline" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
**	......... ... 2019 Ivan Mahonin
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

#include <map>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/curve.h>

#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/valuenodes/valuenode_wplist.h>
#include <synfig/valuenodes/valuenode_dilist.h>

#include <synfig/rendering/primitive/bend.h>
#include <synfig/rendering/primitive/contour.h>

#include "advanced_outline.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#endif


using namespace synfig;


/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Advanced_Outline);
SYNFIG_LAYER_SET_NAME(Advanced_Outline,"advanced_outline");
SYNFIG_LAYER_SET_LOCAL_NAME(Advanced_Outline,N_("Advanced Outline"));
SYNFIG_LAYER_SET_CATEGORY(Advanced_Outline,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Advanced_Outline,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Advanced_Outline,"$Id$");

/* === P R O C E D U R E S ================================================= */

namespace {
	Real calc_position(Real p, const rendering::Bend &bend, bool homogeneous) {
		return homogeneous
			 ? p*bend.length1()
			 : bend.length_by_l( p*bend.l1() );
	}
	
	class AdvancedPoint {
	public:
		Real w;
		Vector pp0, pp1;
		WidthPoint::SideType side0, side1;
		AdvancedPoint(): w(), side0(WidthPoint::TYPE_INTERPOLATE), side1(side0) { }
		
		Real y0() const { return side0 == WidthPoint::TYPE_INTERPOLATE ? w : 0; }
		Real y1() const { return side1 == WidthPoint::TYPE_INTERPOLATE ? w : 0; }
	};
	
	typedef std::map<Real, AdvancedPoint> AdvancedMap;
	
	class AdvancedLine: public AdvancedMap {
	public:
		void add(Real p, Real w, WidthPoint::SideType side0, WidthPoint::SideType side1) {
			w = fabs(w);
			AdvancedPoint &ap = (*this)[p];
			if (approximate_zero(w)) {
				ap.w = 0;
				ap.side0 = ap.side1 = WidthPoint::TYPE_FLAT;
			} else {
				ap.w = w;
				ap.side0 = side0;
				ap.side1 = side1;
			}
		}
		
		void calc_tangents(Real smoothness) {
			if (empty()) return;
			
			const Real kx = Real(1)/3;
			iterator i2 = begin(), i0 = i2++, i1;
			if (i2 != end()) {
				Real s0 = clamp(smoothness, Real(0), Real(1));
				Real s1 = 1 - s0;
				
				for(i1 = i2++; i2 != end(); i0 = i1, i1 = i2++) {
					Vector p0(i0->first, i0->second.y1());
					Vector p1(i1->first, i1->second.y0());
					Vector p2(i2->first, i2->second.y0());
					
					Vector d0 = p1 - p0;
					Vector d1 = p2 - p1;
					Vector dd0 = d0*kx;
					Vector dd1 = d1*kx;
					i1->second.pp0[0] = p1[0] - dd0[0];
					i1->second.pp1[0] = p1[0] + dd1[0];
					
					if (p1[1] == i1->second.y1()) {
						Real ky0 = d0[1]/d0[0];
						Real ky1 = d1[1]/d1[0];
						Real ky = (ky0 + ky1)*0.5;
						ky = ky0 > 0 && ky1 > 0 ? std::min(ky, std::min(ky0, ky1)*3)
						   : ky0 < 0 && ky1 < 0 ? std::max(ky, std::max(ky0, ky1)*3)
						   : 0;
						ky *= s0;
						i1->second.pp0[1] = p1[1] - (dd0[0]*ky + s1*dd0[1]);
						i1->second.pp1[1] = p1[1] + (dd1[0]*ky + s1*dd1[1]);
					} else {
						i1->second.pp0[1] = p1[1] - s1*dd0[1];
						Real y = i1->second.y1();
						i1->second.pp1[1] = y + s1*kx*(p2[1] - y);
					}
				}
				
				Real ky = kx*s1;
				i1 = begin(), i0 = i1++;
				i0->second.pp1[0] = i0->first + (i1->first - i0->first)*kx;
				i0->second.pp1[1] = i0->second.y1() + (i1->second.y0() - i0->second.y1())*ky;

				i0 = end(), i1 = (--i0)--;
				i1->second.pp0[0] = i1->first - (i1->first - i0->first)*kx;
				i1->second.pp0[1] = i1->second.y1() - (i1->second.y0() - i0->second.y1())*ky;
			}
			
			i0 = begin(), i1 = end(), --i1;
			i0->second.pp0 = Vector(i0->first, i0->second.y0());
			i1->second.pp1 = Vector(i1->first, i1->second.y1());
		}
		
		void trunc_left(Real p, WidthPoint::SideType side) {
			iterator i1 = upper_bound(p);
			if (i1 == end())
				{ clear(); return; }
				
			Bezier b;
			if (i1 == begin()) {
				b = Bezier(
					Vector(p, i1->second.y0()),
					Vector(i1->first, i1->second.y0()) );
			} else {
				iterator i0 = i1; --i0;
				b.p0 = Vector(i0->first, i0->second.y1());
				b.p1 = Vector(i1->first, i1->second.y0());
				b.pp0 = i0->second.pp1;
				b.pp1 = i1->second.pp0;
			}
			
			erase(begin(), i1);
			if (!b.p0[1] && !b.p1[1]) return;
			
			Real k = (b.p1[0] - b.p0[0]);
			k = approximate_zero_hp(k) ? 0 : 1/k;
			b.split( (p - b.p0[0])*k, nullptr, &b );
			
			AdvancedPoint &ap = (*this)[p];
			ap.w = b.p0[1];
			ap.pp0 = Vector(p, 0);
			ap.pp1 = b.pp0;
			ap.side0 = side;
			ap.side1 = WidthPoint::TYPE_INTERPOLATE;
			
			i1->second.pp0 = b.pp1;
		}

		void trunc_right(Real p, WidthPoint::SideType side) {
			iterator i1 = upper_bound(p);
			if (i1 == begin())
				{ clear(); return; }
			iterator i0 = i1; --i0;
				
			Bezier b;
			if (i1 == end()) {
				b = Bezier(
					Vector(i0->first, i0->second.y1()),
					Vector(p, i0->second.y1()) );
			} else {
				iterator i0 = i1; --i0;
				b.p0 = Vector(i0->first, i0->second.y1());
				b.p1 = Vector(i1->first, i1->second.y0());
				b.pp0 = i0->second.pp1;
				b.pp1 = i1->second.pp0;
			}
			
			erase(i1, end());
			if (!b.p0[1] && !b.p1[1]) return;
			
			Real k = (b.p1[0] - b.p0[0]);
			k = approximate_zero_hp(k) ? 0 : 1/k;
			b.split( (p - b.p0[0])*k, &b, nullptr );
			
			AdvancedPoint &ap = (*this)[p];
			ap.w = b.p1[1];
			ap.pp0 = b.pp1;
			ap.pp1 = Vector(p, 0);
			ap.side0 = WidthPoint::TYPE_INTERPOLATE;
			ap.side1 = side;
			
			i0->second.pp1 = b.pp0;
		}

		void cut(Real p0, Real p1, WidthPoint::SideType side0, WidthPoint::SideType side1) {
			if (!approximate_less(p0, p1)) return;

			iterator i0 = lower_bound(p0);
			iterator i1 = upper_bound(p1);
			if (i0 == begin())
				{ trunc_left(p1, side1); return; }
			if (i1 == end())
				{ trunc_right(p0, side0); return; }
			
			iterator i00 = i0, i01 = i00--;
			Bezier b0(
				Vector(i00->first, i00->second.y1()),
				Vector(i01->first, i01->second.y0()),
				i00->second.pp1,
				i01->second.pp0 );
			
			iterator i10 = i1, i11 = i10--;
			Bezier b1(
				Vector(i10->first, i10->second.y1()),
				Vector(i11->first, i11->second.y0()),
				i10->second.pp1,
				i11->second.pp0 );
			
			erase(i0, i1);
			
			bool add0 = b0.p0[1] || b0.p1[1];
			bool add1 = b1.p0[1] || b1.p1[1];
			const Real kx = Real(1)/3;
			
			if (add0) {
				Real l = b0.p1[0] - b0.p0[0];
				l = approximate_zero_hp(l) ? 0 : 1/l;
				l = (p0 - b0.p0[0])*l;
				b0.split( l, &b0, nullptr );
				
				// fix x-coord of spline to avoid accumulaeted error
				// x-coord should be linear
				b0.p1[0] = p0;
				Real dx = (b0.p1[0] - b0.p0[0])*kx;
				b0.pp0[0] = b0.p0[0] + dx;
				b0.pp1[0] = b0.p1[0] - dx;
				Real pp = p0 + ((add1 ? p1 : i11->first) - p0)*kx;
				
				AdvancedPoint &ap = (*this)[p0];
				ap.w = b0.p1[1];
				ap.pp0 = b0.pp1;
				ap.pp1 = Vector(pp, 0);
				ap.side0 = WidthPoint::TYPE_INTERPOLATE;
				ap.side1 = side0;

				i00->second.pp1 = b0.pp0;
			}
			
			if (add1) {
				Real l = b1.p1[0] - b1.p0[0];
				l = approximate_zero_hp(l) ? 0 : 1/l;
				l = (p1 - b1.p0[0])*l;
				b1.split( l, nullptr, &b1 );
				
				// fix x-coord of spline to avoid accumulaeted error
				// x-coord should be linear
				b1.p0[0] = p1;
				Real dx = (b1.p1[0] - b1.p0[0])*kx;
				b1.pp0[0] = b1.p0[0] + dx;
				b1.pp1[0] = b1.p1[0] - dx;
				Real pp = p1 - (p1 - (add0 ? p0 : i00->first))*kx;

				AdvancedPoint &ap = (*this)[p1];
				ap.w = b1.p0[1];
				ap.pp0 = Vector(pp, 0);
				ap.pp1 = b1.pp0;
				ap.side0 = side1;
				ap.side1 = WidthPoint::TYPE_INTERPOLATE;

				i11->second.pp0 = b1.pp1;
			}
		}
		
		void build_contour(rendering::Contour &dst) const {
			const Real round_k0 = 0.5*sqrt(2);
			const Real round_k1 = sqrt(2) - 1;
			dst.close();
			for(const_iterator i = begin(); i != end(); ++i) {
				if (i->second.side0 != WidthPoint::TYPE_INTERPOLATE && !dst.closed())
					dst.close_mirrored_vert();
				Real s = 1;
				switch(i->second.side0) {
					case WidthPoint::TYPE_INTERPOLATE:
						if (i == begin()) {
							dst.move_to( Vector(i->first, 0) );
							dst.line_to( Vector(i->first, i->second.w) );
						} else {
							const_iterator i0 = i; --i0;
							if (dst.closed()) {
								dst.move_to( Vector(i0->first, i0->second.y1()) );
								dst.cubic_to(
									Vector(i->first, i->second.y0()),
									i0->second.pp1,
									i->second.pp0 );
							}
						}
						break;
					case WidthPoint::TYPE_SQUARED:
						dst.move_to( Vector(i->first - i->second.w, 0) );
						dst.line_to( Vector(i->first - i->second.w, i->second.w) ),
						dst.line_to( Vector(i->first, i->second.w) );
						break;
					case WidthPoint::TYPE_INNER_PEAK:
						s = -1;
					case WidthPoint::TYPE_PEAK:
						dst.move_to( Vector(i->first - s*i->second.w, 0) );
						dst.line_to( Vector(i->first, i->second.w ) );
						break;
					case WidthPoint::TYPE_INNER_ROUNDED:
						s = -1;
					case WidthPoint::TYPE_ROUNDED:
						dst.move_to( Vector(i->first - s*i->second.w, 0) );
						dst.conic_to(
							Vector(i->first - s*i->second.w*round_k0, i->second.w*round_k0),
							Vector(i->first - s*i->second.w, i->second.w*round_k1) );
						dst.conic_to(
							Vector(i->first, i->second.w),
							Vector(i->first - s*i->second.w*round_k1, i->second.w) );
						break;
					default: // flat
						dst.move_to( Vector(i->first, 0) );
						dst.line_to( Vector(i->first, i->second.w) );
						break;
				}
				
				s = 1;
				switch(i->second.side1) {
					case WidthPoint::TYPE_INTERPOLATE:
						{
							const_iterator i1 = i; ++i1;
							if (i1 != end()) {
								dst.line_to( Vector(i->first, i->second.y1()) );
								dst.cubic_to(
									Vector(i1->first, i1->second.y0()),
									i->second.pp1,
									i1->second.pp0 );
							} else {
								dst.line_to( Vector(i->first, 0) );
								dst.close_mirrored_vert();
							}
						}
						break;
					case WidthPoint::TYPE_SQUARED:
						dst.line_to( Vector(i->first + i->second.w, i->second.w) ),
						dst.line_to( Vector(i->first + i->second.w, 0) );
						dst.close_mirrored_vert();
						break;
					case WidthPoint::TYPE_INNER_PEAK:
						s = -1;
					case WidthPoint::TYPE_PEAK:
						dst.line_to( Vector(i->first + s*i->second.w, 0) );
						dst.close_mirrored_vert();
						break;
					case WidthPoint::TYPE_INNER_ROUNDED:
						s = -1;
					case WidthPoint::TYPE_ROUNDED:
						dst.conic_to(
							Vector(i->first + s*i->second.w*round_k0, i->second.w*round_k0),
							Vector(i->first + s*i->second.w*round_k1, i->second.w) );
						dst.conic_to(
							Vector(i->first + s*i->second.w, 0),
							Vector(i->first + s*i->second.w, i->second.w*round_k1) );
						dst.close_mirrored_vert();
						break;
					default: // flat
						dst.line_to( Vector(i->first, 0) );
						dst.close_mirrored_vert();
						break;
				}
			}
		}
	};
}

/* === M E T H O D S ======================================================= */

Advanced_Outline::Advanced_Outline():
	param_bline(ValueBase(std::vector<synfig::BLinePoint>())),
	param_wplist(ValueBase(std::vector<synfig::WidthPoint>())),
	param_dilist(ValueBase(std::vector<synfig::DashItem>()))
{
	param_cusp_type = ValueBase(int(TYPE_SHARP));
	param_start_tip = param_end_tip = ValueBase(int(WidthPoint::TYPE_ROUNDED));
	param_width = ValueBase(Real(1.0f));
	param_expand = ValueBase(Real(0));
	param_smoothness = ValueBase(Real(1));
	param_dash_offset = ValueBase(Real(0));
	param_homogeneous = ValueBase(false);
	param_dash_enabled = ValueBase(false);
	
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
					(WidthPoint::SideType)point.get_side_type_after() );
			}
		}
		
		if (loop) {
			AdvancedLine::iterator i0 = aline.begin();
			AdvancedLine::iterator i1 = aline.end(); --i1;
			if (approximate_less(Real(0), i0->first))
				aline.add(i1->first - kl, i1->second.w, WidthPoint::TYPE_FLAT, i1->second.side1);
			if (approximate_less(i1->first, kl))
				aline.add(i0->first + kl, i0->second.w, i0->second.side1, WidthPoint::TYPE_FLAT);
			aline.calc_tangents(smoothness);
		} else {
			// make tails longer for proper trunc
			AdvancedLine::const_iterator i = aline.begin();
			if (i->second.side0 == WidthPoint::TYPE_INTERPOLATE)
				aline.add(-1, i->second.w, WidthPoint::TYPE_FLAT, WidthPoint::TYPE_INTERPOLATE);
			i = aline.end(); --i;
			if (i->second.side1 == WidthPoint::TYPE_INTERPOLATE)
				aline.add(kl + 1, i->second.w, WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_FLAT);
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
				while(p0 < 1) {
					for(ValueBase::List::const_iterator i = dilist.begin(); i != dilist.end(); ++i) {
						const DashItem &dash = i->get(di_blank);
						Real p1 = p0 + dash.get_offset();
						aline.cut(
							calc_position( p0, bend, homogeneous ),
							calc_position( p1, bend, homogeneous ),
							DashItem::to_wp_side_type( type0 ),
							DashItem::to_wp_side_type( (DashItem::SideType)dash.get_side_type_before() ) );
						p0 = p1 + dash.get_length();
						if (p0 >= 1) break;
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
	ret.push_back(ParamDesc(ValueBase(),"start_tip")
		.set_local_name(_("Tip Type at Start"))
		.set_description(_("Defines the Tip type of the first spline point when spline is unlooped"))
		.set_hint("enum")
		.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
		.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
		.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
		.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
		.add_enum_value(WidthPoint::TYPE_INNER_ROUNDED,"inner_rounded", _("Inner Rounded Stop"))
		.add_enum_value(WidthPoint::TYPE_INNER_PEAK,"inner_peak", _("Off-Peak Stop"))
		);
	ret.push_back(ParamDesc(ValueBase(),"end_tip")
		.set_local_name(_("Tip Type at End"))
		.set_description(_("Defines the Tip type of the last spline point when spline is unlooped"))
		.set_hint("enum")
		.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
		.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
		.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
		.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
		.add_enum_value(WidthPoint::TYPE_INNER_ROUNDED,"inner_rounded", _("Inner Rounded Stop"))
		.add_enum_value(WidthPoint::TYPE_INNER_PEAK,"inner_peak", _("Off-Peak Stop"))
		);
	ret.push_back(ParamDesc("cusp_type")
		.set_local_name(_("Cusps Type"))
		.set_description(_("Determines cusp type"))
		.set_hint("enum")
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
		.set_description(_("When true, widthpoints positions are spline length based"))
	);
	ret.push_back(ParamDesc("wplist")
		.set_local_name(_("Width Point List"))
		.set_hint("width")
		.set_origin("origin")
		.set_description(_("List of width Points that defines the variable width"))
	);
	ret.push_back(ParamDesc("dash_enabled")
		.set_local_name(_("Dashed Outline"))
		.set_hint("dash")
		.set_description(_("When checked outline is dashed"))
	);
	ret.push_back(ParamDesc("dilist")
		.set_local_name(_("Dash Item List"))
		.set_hint("dash")
		.set_origin("origin")
		.set_description(_("List of dash items that defines the dashed outline"))
	);
	ret.push_back(ParamDesc("dash_offset")
		.set_local_name(_("Dash Items Offset"))
		.set_is_distance()
		.set_hint("dash")
		.set_description(_("Distance to Offset the Dash Items"))
	);
	return ret;
}

bool
Advanced_Outline::connect_dynamic_param(const String& param, etl::loose_handle<ValueNode> x)
{
	if(param=="bline")
	{
		connect_bline_to_wplist(x);
		connect_bline_to_dilist(x);
		return Layer::connect_dynamic_param(param, x);
	}
	if(param=="wplist")
	{
		if(Layer::connect_dynamic_param(param, x))
		{
			DynamicParamList::const_iterator iter(dynamic_param_list().find("bline"));
			if(iter==dynamic_param_list().end())
				return false;
			else if(!connect_bline_to_wplist(iter->second))
				return false;
			return true;
		}
		else
			return false;
	}
	if(param=="dilist")
	{
		if(Layer::connect_dynamic_param(param, x))
		{
			DynamicParamList::const_iterator iter(dynamic_param_list().find("bline"));
			if(iter==dynamic_param_list().end())
				return false;
			else if(!connect_bline_to_dilist(iter->second))
				return false;
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
	if(x->get_type() != type_list)
		return false;
	if((*x)(Time(0)).empty())
		return false;
	if((*x)(Time(0)).get_list().front().get_type() != type_bline_point)
		return false;
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

bool
Advanced_Outline::connect_bline_to_dilist(etl::loose_handle<ValueNode> x)
{
	if(x->get_type() != type_list)
		return false;
	if((*x)(Time(0)).empty())
		return false;
	if((*x)(Time(0)).get_list().front().get_type() != type_bline_point)
		return false;
	ValueNode::LooseHandle vnode;
	DynamicParamList::const_iterator iter(dynamic_param_list().find("dilist"));
	if(iter==dynamic_param_list().end())
		return false;
	ValueNode_DIList::Handle dilist(ValueNode_DIList::Handle::cast_dynamic(iter->second));
	if(!dilist)
		return false;
	dilist->set_bline(ValueNode::Handle(x));
	return true;
}

