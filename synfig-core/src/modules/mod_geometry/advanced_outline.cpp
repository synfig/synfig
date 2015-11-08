/* === S Y N F I G ========================================================= */
/*!	\file outline.cpp
**	\brief Implementation of the "Advanced Outline" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011-2013 Carlos López
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

#include <synfig/localization.h>
#include <synfig/general.h>

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

#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/valuenodes/valuenode_wplist.h>
#include <synfig/valuenodes/valuenode_dilist.h>
#include <synfig/valuenodes/valuenode_composite.h>

#endif

using namespace etl;


/* === M A C R O S ========================================================= */
#define SAMPLES		50
#define ROUND_END_FACTOR	(4)
#define CUSP_THRESHOLD		(0.40)
#define SPIKE_AMOUNT		(4)
#define NO_LOOP_COOKIE		synfig::Vector(84951305,7836658)
#define EPSILON				(0.000000001)
#define CUSP_TANGENT_ADJUST	(0.025)
/* === G L O B A L S ======================================================= */
SYNFIG_LAYER_INIT(Advanced_Outline);
SYNFIG_LAYER_SET_NAME(Advanced_Outline,"advanced_outline");
SYNFIG_LAYER_SET_LOCAL_NAME(Advanced_Outline,N_("Advanced Outline"));
SYNFIG_LAYER_SET_CATEGORY(Advanced_Outline,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Advanced_Outline,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Advanced_Outline,"$Id$");
/* === P R O C E D U R E S ================================================= */
Point line_intersection( const Point& p1, const Vector& t1, const Point& p2, const Vector& t2 );
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
	param_smoothness = ValueBase(Real(0.5));
	param_dash_offset = ValueBase(Real(0.0));
	param_homogeneous = ValueBase(false);
	param_dash_enabled = ValueBase(false);
	param_fast = ValueBase(false);
	
	old_version=false;
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
	param_bline.set_list_of(bline_point_list);
	
	vector<WidthPoint> wpoint_list;
	wpoint_list.push_back(WidthPoint());
	wpoint_list.push_back(WidthPoint());
	wpoint_list[0].set_position(0.1);
	wpoint_list[1].set_position(0.9);
	wpoint_list[0].set_width(1.0);
	wpoint_list[1].set_width(1.0);
	wpoint_list[0].set_side_type_before(WidthPoint::TYPE_INTERPOLATE);
	wpoint_list[1].set_side_type_after(WidthPoint::TYPE_INTERPOLATE);
	param_wplist.set_list_of(wpoint_list);
	
	vector<DashItem> ditem_list;
	ditem_list.push_back(DashItem());
	param_dilist.set_list_of(ditem_list);
	
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}


/*! The Sync() function takes the values
**	and creates a polygon to be rendered
**	with the polygon layer.
*/
void
Advanced_Outline::sync()
{
	ValueBase bline_=param_bline;
	ValueBase wplist_=param_wplist;
	ValueBase dilist_=param_dilist;
	int start_tip_=param_start_tip.get(int());
	int end_tip_=param_end_tip.get(int());
	int cusp_type_=param_cusp_type.get(int());
	Real width_=param_width.get(Real());
	Real expand_=param_expand.get(Real());
	Real smoothness_=param_smoothness.get(Real());
	bool homogeneous_=param_homogeneous.get(bool());
	Real dash_offset_=param_dash_offset.get(Real());
	bool dash_enabled_=param_dash_enabled.get(bool());
	bool fast_=param_fast.get(bool());
	
	clear();
	if (!bline_.get_list().size())
	{
		synfig::warning(string("Advanced_Outline::sync():")+N_("No vertices in spline " + string("\"") + get_description() + string("\"")));
		return;
	}
	try
	{
		// The list of blinepoints
		vector<BLinePoint> bline(bline_.get_list_of(BLinePoint()));
		// The list of blinepoints standard and homogeneous positions
		vector<Real> bline_pos, hbline_pos;
		// This is the list of widthpoints coming form the WPList
		// Notice that wplist will contain the dash items if applicable
		// and some of the widthpoints are removed when lies on empty space of the
		// dash items.
		vector<WidthPoint> wplist(wplist_.get_list_of(WidthPoint()));
		// This is the same than wplist but with standard positions.
		vector<WidthPoint> swplist;
		// This is a copy of wplist without dash items and with all the original widthpoints
		// standard and homogeneous ones
		vector<WidthPoint> cwplist,scwplist;
		// This is the list of dash items
		vector<DashItem> dilist(dilist_.get_list_of(DashItem()));
		// This is the list of widthpoints created for the dashed outlines
		vector<WidthPoint> dwplist;
		// This is the temporarly filtered (removed unused) list of dash widthpoints
		// it is a partial filtered of the previous dwplist and merged with wplist
		// only allowing visible widthpoints.
		vector<WidthPoint> fdwplist;
		bool homogeneous(homogeneous_);
		bool dash_enabled(dash_enabled_ && dilist.size());
		Real dash_offset(dash_offset_);
		int dstart_tip(WidthPoint::TYPE_FLAT), dend_tip(WidthPoint::TYPE_FLAT);
		const bool blineloop(bline_.get_loop());
		const bool wplistloop(wplist_.get_loop());
		int bline_size(bline.size());
		int wplist_size(wplist.size());
		// biter: first blinepoint of the current bezier
		// bnext: second blinepoint of the current bezier
		vector<BLinePoint>::const_iterator biter,bnext(bline.begin());
		// bpiter/hbpiter: first position of the current bezier
		// bpnext/hbpnext: second position of the current bezier
		vector<Real>::iterator bpiter, bpnext, hbpiter;
		// (s)(c)witer: current widthpoint in cosideration
		// (s)(c)next: next widthpoint in consideration
		vector<WidthPoint>::iterator witer, wnext, switer, swnext, cwiter, cwnext, scwiter, scwnext, dwiter, dwnext;
		// first tangent: used to remember the first tangent of the first bezier
		// used to draw sharp cusp on the last step.
		Vector first_tangent;
		// Used to remember first tangent only once
		bool first(true);
		// Used to remember if in the next loop we should do a middle corner
		bool middle_corner(false);
		// Used to remember if we have just passed a widthpoint with tip
		bool done_tip(false);
		// Used to remember if we are adding a first(last) widthpoint when
		// blinelooped and first(last) normal widthpoint is before(after) side
		// type set to interpolate
		bool inserted_first(false), inserted_last(false);
		// last tangent: used to remember second tangent of the previous bezier
		// when doing the cusp at the first blinepoint of the current bezier
		Vector last_tangent;
		// Bezier size is differnt depending on whether the bline is looped or not.
		// For one single blinepoint, bezier size is always 1.0
		Real bezier_size = 1.0/(blineloop?bline_size:(bline_size==1?1.0:(bline_size-1)));
		const vector<BLinePoint>::const_iterator bend(bline.end());
		// Retrieve the parent canvas grow value
		Real gv(exp(get_parent_canvas_grow_value()));
		// If we have only one blinepoint and it the bline is not looped
		// then there is nothing to render
		if(!blineloop && bline_size==1)
			return;
		// Fill the list of positions of the blinepoints
		// bindex is used to calculate the position
		// of the bilinepoint on the bline properly
		// *multiply by index is better than sum an index of times*
		Real bindex(0.0);
		for(biter=bline.begin(); biter!=bend; biter++)
		{
			bline_pos.push_back(bindex*bezier_size);
			hbline_pos.push_back(fast_?bline_pos.back():std_to_hom(bline, bline_pos.back(), wplistloop, blineloop));
			bindex++;
		}
		// When bline is looped, it is needed one more position
		if(blineloop)
		{
			bline_pos.push_back(1.0);
			hbline_pos.push_back(1.0);
		}
		// To avoid problems when number of blinepoins is huge don't be confident on
		// multiplications. Make it exactly 1.0 anyway.
		else
		{
			bline_pos.pop_back();
			bline_pos.push_back(1.0);
			hbline_pos.pop_back();
			hbline_pos.push_back(1.0);
		}
		// initialize the blinepoints positions iterators
		hbpiter=hbline_pos.begin();
		bpiter=bline_pos.begin();
		// todo: why 'hbiter_pos' never used?
		Real biter_pos(*bpiter) /* , hbiter_pos(*hbpiter) */ ;
		bpiter++;
		hbpiter++;
		Real bnext_pos(*bpiter), hbnext_pos(*hbpiter);
		// side_a and side_b are the sides of the polygon
		vector<Point> side_a, side_b;
		// Normalize the wplist first and then use always get_position()
		for(witer=wplist.begin();witer!=wplist.end();witer++)
			witer->set_position(witer->get_norm_position(wplistloop));
		// Sort the wplist. It is needed to calculate the first widthpoint
		sort(wplist.begin(),wplist.end());
		// If we are looped, the first bezier to handle starts form the
		// last blinepoint and ends at the first one
		// 				biter	bnext
		//				----	----
		// looped		nth		1st
		// !looped		1st		2nd
		if(blineloop)
			biter=--bline.end();
		else
			biter=bnext++;
		// Let's give to last tangent an initial value.
		last_tangent=biter->get_tangent1();
		// if we are looped and drawing sharp cusps and the last tangent is zero,
		// we'll need a value for the incoming tangent
		if (blineloop && cusp_type_==TYPE_SHARP && last_tangent.is_equal_to(Vector::zero()))
		{
			hermite<Vector> curve((biter-1)->get_vertex(), biter->get_vertex(), (biter-1)->get_tangent2(), biter->get_tangent1());
			const derivative< hermite<Vector> > deriv(curve);
			last_tangent=deriv(1.0-CUSP_TANGENT_ADJUST);
		}
		///////////////////////////////////////////// Prepare the wplist
		// If not looped
		if(!blineloop)
		{
		// if we have some widthpoint in the list
			if(wplist_size)
			{
				WidthPoint wpfront(wplist.front());
				WidthPoint wpback(wplist.back());
				// if the first widthpoint interpolation before is INTERPOLATE and it is not exactly at 0.0
				if(wpfront.get_side_type_before() == WidthPoint::TYPE_INTERPOLATE && wpfront.get_position()!=0.0)
					// Add a fake widthpoint at position 0.0
					wplist.push_back(WidthPoint(0.0, wpfront.get_width() , start_tip_, WidthPoint::TYPE_INTERPOLATE));
				// if last widhtpoint interpolation after is INTERPOLATE and it is not exactly at 1.0
				if(wpback.get_side_type_after() == WidthPoint::TYPE_INTERPOLATE && wpback.get_position()!=1.0)
				// Add a fake withpoint at position 1.0
					wplist.push_back(WidthPoint(1.0, wpback.get_width() , WidthPoint::TYPE_INTERPOLATE, end_tip_));
			}
			// don't have any widthpoint in the list
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
					if(wpfront.get_position()!=0.0)
					// Add a fake widthpoint at position 0.0
					{
						WidthPoint i(wpback);
						WidthPoint n(wpfront);
						if(!homogeneous && !fast_)
						{
							i.set_position(std_to_hom(bline, i.get_position(), wplistloop, blineloop));
							n.set_position(std_to_hom(bline, n.get_position(), wplistloop, blineloop));
						}
						wplist.push_back(WidthPoint(0.0, widthpoint_interpolate(i, n, 0.0, smoothness_) , WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_INTERPOLATE));
						inserted_first=true;
					}
					// If it is not exactly at 1.0
					if(wpback.get_position()!=1.0)
					// Add a fake widthpoint at position 1.0
					{
						WidthPoint i(wpback);
						WidthPoint n(wpfront);
						if(!homogeneous && !fast_)
						{
							i.set_position(std_to_hom(bline, i.get_position(), wplistloop, blineloop));
							n.set_position(std_to_hom(bline, n.get_position(), wplistloop, blineloop));
						}
						wplist.push_back(WidthPoint(1.0, widthpoint_interpolate(i, n, 1.0, smoothness_) , WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_INTERPOLATE));
						inserted_last=true;
					}
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
		////////////////////// End preparing the WPlist ////////////////
		//list the wplist
		//synfig::info("---wplist---");
		//for(witer=wplist.begin();witer!=wplist.end();witer++)
			//synfig::info("P:%f W:%f B:%d A:%d", witer->get_position(), witer->get_width(), witer->get_side_type_before(), witer->get_side_type_after());
		//synfig::info("------");
		////////////////////////////////////////////////////////////////
		// TODO: step should be a function of the current situation
		// i.e.: where in the bline, and where in wplist so we could go
		// faster or slower when needed.
		Real step(1.0/SAMPLES/bline_size);
		//////////////////// prepare the widhtpoints from the dash list
		if(dash_enabled)
		{
			Real blinelength(bline_length(bline, blineloop, NULL));
			if(blinelength > EPSILON)
			{
				Real dashes_length(0.0);
				vector<DashItem>::iterator diter(dilist.begin());
				vector<DashItem>::reverse_iterator rditer(dilist.rbegin());
				WidthPoint before, after;
				// Calculate the length of the defined dashes
				for(;diter!=dilist.end(); diter++)
				{
					dashes_length+=diter->get_length()+diter->get_offset();
				}
				if(dashes_length>EPSILON)
				{
					// Put dash_offset in the [-dashes_length,dashes_length] interval
					if (fabs(dash_offset) > dashes_length) dash_offset=fmod(dash_offset, dashes_length);
					// dpos is always >= 0
					Real dpos=dash_offset>=0?dash_offset:(dashes_length+dash_offset);
					diter=dilist.begin();
					// Insert the widthpoints from Dash Offset to blinelength
					int inserted_to_blinelength(0);
					while(dpos < blinelength)
					{
						// dash widthpoints should have the same homogeneous or standard comparable positions.
						Real before_pos=(dpos+diter->get_offset())/blinelength;
						Real after_pos=(dpos+diter->get_offset()+diter->get_length())/blinelength;
						before_pos=homogeneous?before_pos:hom_to_std(bline, before_pos, wplistloop, blineloop);
						after_pos=homogeneous?after_pos:hom_to_std(bline, after_pos, wplistloop, blineloop);
						before=WidthPoint(before_pos, 1.0, diter->get_side_type_before(), WidthPoint::TYPE_INTERPOLATE, true);
						after=WidthPoint(after_pos, 1.0, WidthPoint::TYPE_INTERPOLATE, diter->get_side_type_after(), true);
						dwplist.push_back(before);
						dwplist.push_back(after);
						dpos+=diter->get_offset() + diter->get_length();
						diter++;
						inserted_to_blinelength++;
						if(diter==dilist.end())
							diter=dilist.begin();
					};
					// Correct the two last widthpoints triming its position to be <= 1.0
					if(inserted_to_blinelength)
					{
						after=dwplist.back();
						// if the if the 'after' widthpoint passed 1.0
						if(after.get_position() >= 1.0)
						{
							// trim to 1.0
							after.set_position(1.0);
							dwplist.pop_back();
							before=dwplist.back();
							// then watch the before one and if it passeed 1.0
							if(before.get_position() >= 1.0)
							{
								// discard it (and also the 'after' one)
								dwplist.pop_back();
								// and decrease the number of inserted dash items
								inserted_to_blinelength--;
							}
							else
							// restore the 'after' widthpoint
							{
								dend_tip=after.get_side_type_after();
								dwplist.push_back(after);
							}
						}
					}
					int inserted_to_zero(0);
					//
					// Now insert the widhtpoints from Dash Offset to 0.0
					dpos=dash_offset>=0?dash_offset:dashes_length+dash_offset;;
					while(dpos > 0.0)
					{
						// dash widthpoints should have the same homogeneous or standard comparable positions.
						Real before_pos=(dpos-rditer->get_length())/blinelength;
						Real after_pos=(dpos)/blinelength;
						before_pos=homogeneous?before_pos:hom_to_std(bline, before_pos, wplistloop, blineloop);
						after_pos=homogeneous?after_pos:hom_to_std(bline, after_pos, wplistloop, blineloop);
						before=WidthPoint(before_pos, 1.0, rditer->get_side_type_before(), WidthPoint::TYPE_INTERPOLATE, true);
						after=WidthPoint(after_pos, 1.0, WidthPoint::TYPE_INTERPOLATE, rditer->get_side_type_after(), true);
						dwplist.insert(dwplist.begin(),after);
						dwplist.insert(dwplist.begin(),before);
						dpos-=rditer->get_offset() + rditer->get_length();
						rditer++;
						inserted_to_zero++;
						if(rditer==dilist.rend())
							rditer=dilist.rbegin();
					};
					// Correct the two first widthpoints triming its position to be >= 0.0
					if(inserted_to_zero)
					{
						before=dwplist.front();
						// if the dash is cutted in the middle then trim the 'before' widthpoint
						if(before.get_position() <= 0.0  )
						{
							// trim it to 0.0
							before.set_position(0.0);
							dwplist.erase(dwplist.begin());
							after=dwplist.front();
							// then watch the after one and if it passed 0.0
							if(after.get_position() <= 0.0)
							{
								// discard the 'after' one (and the 'before' one too)
								dwplist.erase(dwplist.begin());
								// and decrease the number of inserted dash items
								inserted_to_zero--;
							}
							else
								// restore the 'before' widthpoint
							{
								dstart_tip=before.get_side_type_before();
								dwplist.insert(dwplist.begin(), before);
							}
						}
					}
					// Let's check that we have one dash widthpoint at last
					// inside the bline interval
					if(inserted_to_blinelength == 0 && inserted_to_zero==0)
					{
						// all the dash items widthpoints were outside the bline
						// so the bline is an empty interval
						// let's insert two dash widthpoints that would
						// 'clean' the bline area
						before=WidthPoint(0.5, 1.0, WidthPoint::TYPE_FLAT, WidthPoint::TYPE_INTERPOLATE, true);
						after=WidthPoint(0.5, 1.0, WidthPoint::TYPE_INTERPOLATE, WidthPoint::TYPE_FLAT, true);
						dwplist.push_back(before);
						dwplist.push_back(after);
					}
					// now let's remove those dash widthpoints that doesn't
					// lie on a drawable place
					// first prepare the widthpoint iterators
					wnext=wplist.begin();
					if(blineloop)
						witer=--wplist.end();
					else
						witer=wnext;
					do
					{
						// grab the position of the next widthpoint
						// and the position of the iter widthpoint
						Real witer_pos(witer->get_position());
						Real wnext_pos(wnext->get_position());
						// if the current widthpoint interval is not empty
						// then keep all the dash widthpoints that are in between
						// or
						// if we aren't in the first non blinelooped widthpoint
						if((witer->get_side_type_after()==WidthPoint::TYPE_INTERPOLATE ||
						wnext->get_side_type_before()==WidthPoint::TYPE_INTERPOLATE))
						{
							dwiter=dwplist.begin();
							// extract the dash widthpoints that are in a non empty interval
							while(dwiter!=dwplist.end())
							{
								Real dwiter_pos=dwiter->get_position();
								if(dwiter_pos > witer_pos && dwiter_pos < wnext_pos)
								{
									fdwplist.push_back(*dwiter);
								}
								dwiter++;
							}
						}
						witer=wnext;
						wnext++;
					}while(wnext!=wplist.end());
					// Now we need to remove the regular widthpoints that
					// lie in a dash empty space.
					// first prepare the dash widthpoint iterators
					dwiter=dwplist.begin();
					dwnext=dwiter+1;
					do
					{
						Real dwiter_pos=dwiter->get_position();
						Real dwnext_pos=dwnext->get_position();
						for(witer=wplist.begin(); witer!=wplist.end();witer++)
						{
							Real witer_pos=witer->get_position();
							if(witer_pos <= dwnext_pos && witer_pos >= dwiter_pos)
							{
								fdwplist.push_back(*witer);
							}
						}
						dwnext++;
						dwiter=dwnext;
						if(dwnext==dwplist.end())
							break;
						dwnext++;
					}while(1);
				} // if dashes_length > EPSILON
			} // if blinelength > EPSILON
		} ////////////////////////////////////////////// if dash_enabled
		//Make a copy of the original wplist
		cwplist.assign(wplist.begin(), wplist.end());
		//Copy it to the standard position list too
		scwplist.assign(wplist.begin(), wplist.end());
		// Make scwplist standard if needed.
		if(homogeneous)
		{
			for(scwiter=scwplist.begin(); scwiter!=scwplist.end();scwiter++)
				scwiter->set_position(hom_to_std(bline, scwiter->get_position(), wplistloop, blineloop));
		}
		//Make cwplist homogeneous if needed.
		else
		{
			for(cwiter=cwplist.begin(); cwiter!=cwplist.end();cwiter++)
				cwiter->set_position(std_to_hom(bline, cwiter->get_position(), wplistloop, blineloop));
		}
		//If using dashes ...
		if(dash_enabled)
		{
			// ... replace the original widthpoint list
			// with the filtered one, inlcuding the visible dash withpoints and
			// the visible regular widthpoints.
			wplist.assign(fdwplist.begin(), fdwplist.end());
			// sort again the wplist
			sort(wplist.begin(),wplist.end());
			//////////////
			witer=wplist.begin();
		}
		// if at this point, the wplist size is zero, all the regular
		//and dash points have been removed, so there is nothing to draw
		// I insert a single widthpoint that renders nothing.
		// If I just return here, it crashes when played on canvas view.
		//
		if(wplist.size() == 0)
		{
			wplist.push_back(WidthPoint(0.5, 1.0, WidthPoint::TYPE_FLAT, WidthPoint::TYPE_FLAT, true));
		}
		// Make a copy of the work widthpoints to the standard list
		swplist.assign(wplist.begin(), wplist.end());
		// Make swplist standard if needed
		if(homogeneous)
		{
			for(switer=swplist.begin(); switer!=swplist.end();switer++)
				switer->set_position(hom_to_std(bline, switer->get_position(), wplistloop, blineloop));
		}
		//Make wplist homogeneous if needed
		else
		{
			for(witer=wplist.begin(); witer!=wplist.end();witer++)
				witer->set_position(std_to_hom(bline, witer->get_position(), wplistloop, blineloop));
		}
		// Prepare the widthpoint iterators
		// we start with the next withpoint being the first on the list.
		wnext=wplist.begin();
		swnext=swplist.begin();
		// then the current widthpoint would be the last one if blinelooped...
		if(blineloop)
		{
			witer=--wplist.end();
			switer=--swplist.end();
		}
		else
			// ...or the same as the first one if not blinelooped.
			// This allows to make the first tip without need to take any decision
			// in the code. Later they are separated and works as expected.
		{
			witer=wnext;
			switer=swnext;
		}
		// now let's prepare the iterators of the copy. They will be the same
		// than the current one if the outline is not dashed
		cwnext=cwplist.begin();
		scwnext=scwplist.begin();
		if(blineloop)
		{
			cwiter=--cwplist.end();
			scwiter=--cwplist.end();
		}
		else
		{
			cwiter=cwnext;
			scwiter=scwnext;
		}
		const vector<WidthPoint>::const_iterator wend(wplist.end());
		const vector<WidthPoint>::const_iterator swend(wplist.end());
		// standard position
		Real ipos(0.0);
		// homogeneous position
		Real hipos(0.0);
		// Fix bug of bad render of start (end) tip when the first
		// (last) widthpoint has side type before (after) set to
		// interpolate and it is at 0.0 (1.0). User expects the tip to
		// have the same type of the layer's start (end) tip.
		if(!blineloop)
		{
			if(wnext->get_position()==0.0)
				wnext->set_side_type_before(dash_enabled?dstart_tip:start_tip_);
			vector<WidthPoint>::iterator last=--wplist.end();
			if(last->get_position()==1.0)
				last->set_side_type_after(dash_enabled?dend_tip:end_tip_);
		}


		// If the first (last) widthpoint is interpolate before (after)
		// and we are doing dashes, then make the first (last) widthpoint to
		// have the start (end) dash item's corresponding tip.
		if(dash_enabled)
		{
			vector<WidthPoint>::iterator first(wplist.begin());
			vector<WidthPoint>::iterator last(--wplist.end());
			if(first->get_side_type_before() == WidthPoint::TYPE_INTERPOLATE)
				first->set_side_type_before(dstart_tip);
			if(last->get_side_type_after() == WidthPoint::TYPE_INTERPOLATE)
				last->set_side_type_after(dend_tip);
		}
		do ///////////////////////// Main loop
		{
			Vector iter_t(biter->get_tangent2());
			Vector next_t(bnext->get_tangent1());
			Real iter_t_mag(iter_t.mag());
			Real next_t_mag(next_t.mag());
			bool split_flag(biter->get_split_tangent_angle() || biter->get_tangent1().mag()==0 || biter->get_tangent2().mag()==0);
			// Setup the bezier curve
			hermite<Vector> curve(
				biter->get_vertex(),
				bnext->get_vertex(),
				iter_t,
				next_t
			);
			const derivative< hermite<Vector> > deriv(curve);
			// if tangents are zero length then use the derivative.
			if(iter_t_mag==0.0)
				iter_t=deriv(CUSP_TANGENT_ADJUST);
			if(next_t_mag==0.0)
				next_t=deriv(1.0-CUSP_TANGENT_ADJUST);
			// Remember the first tangent to use it on the last cusp
			if(blineloop && first)
			{
				first_tangent=iter_t;
				first=false;
			}
			// get the position of the next widhtpoint.
			// Remember that it is the first widthpoint the first time
			// code passes by here.
			Real wnext_pos(wnext->get_position());
			Real swnext_pos(swnext->get_position());
			// if we are exactly on the next widthpoint...
			if(ipos==swnext_pos)
			{
				Vector unitary;
				hipos=wnext_pos;
				// .. do tips. (If withpoint is interpolate it doesn't do anything).
				Real bezier_ipos(bline_to_bezier(ipos, biter_pos, bezier_size));
				Real q(bezier_ipos);
				if(q < EPSILON)
					unitary=iter_t.norm();
				else if(q > (1.0-EPSILON))
					unitary=next_t.norm();
				else
					unitary=deriv(q).norm();
				if(wnext->get_dash())
				{
					vector<WidthPoint>::iterator ci(scwiter);
					vector<WidthPoint>::iterator cn(scwnext);
					if(!fast_)
					{
						ci=cwiter;
						cn=cwnext;
					}
					// if we inserted the widthpoints at start and end, don't consider them for interpolation.
					if(ci->get_position() == 0.0 && cn->get_position()!=1.0 && inserted_first)
					{
						ci=cwplist.end();
						ci--;
						if(inserted_last) ci--;
					}
					if(cn->get_position() == 1.0 && inserted_last)
					{
						cn=cwplist.begin();
						if(inserted_first) cn++;
					}
					WidthPoint i(*ci);
					WidthPoint n(*cn);
					Real p(ipos);
					if(!fast_)
						p=hipos;
					wnext->set_width(widthpoint_interpolate(i, n, p, smoothness_));
				}
				add_tip(side_a, side_b, curve(q), unitary, *wnext, gv);
				// Update wplist iterators
				witer=wnext;
				switer=swnext;
				wnext++;
				swnext++;
				// If we are at the last widthpoint
				if(wnext==wend || swnext==swend)
				{
					// There is always a widthpoint at the end (and start)
					// when it is blinelooped and interpolated on last widthpoint.
					// In other cases the interpolated width is zero so there is not corner
					// rendered.
					// ... let's make the last cusp...
					cwnext=cwplist.begin();
					cwiter=--cwplist.end();
					scwnext=scwplist.begin();
					scwiter=--scwplist.end();
					// if we are doing looped blines and it is tangent split or one of its tangents is zero
					if(blineloop && (bnext->get_split_tangent_angle()|| bnext->get_tangent1().mag()==0.0 || bnext->get_tangent2().mag()==0.0))
					{
						vector<WidthPoint>::iterator first(wplist.begin());
						vector<WidthPoint>::iterator last(--wplist.end());
						// when doing dashed outlines, the above rule is not always true
						if(first->get_side_type_before()==WidthPoint::TYPE_INTERPOLATE || last->get_side_type_after()==WidthPoint::TYPE_INTERPOLATE)
						{
							WidthPoint i(*scwiter);
							WidthPoint n(*scwnext);
							if(!fast_)
							{
								i=*cwiter;
								n=*cwnext;
							}
							Real p(ipos);
							if(!fast_)
								p=hipos;
							add_cusp(side_a, side_b, bnext->get_vertex(), first_tangent, deriv(1.0-CUSP_TANGENT_ADJUST), gv*(expand_+width_*0.5*widthpoint_interpolate(i, n, p, smoothness_)));
						}
					}
					// ... and get out of the main loop.
					break;
				}
				else
				{
					// In this case there are more width points waiting
					// to be rendered. We need to increase ipos so we do
					// attempt to do the next interpolation segment.
					// It is needed to be EPSILON to produce good cusps
					// for the last blinepoint. Bigger step bends the last
					// cusp.
					// This is because over a widthpoint the interpolation
					// gives a width of exactly the width of the width
					// point, but from the point of view of the next
					// widthpoint in the list, if the current one has not
					//  interpolate side after the interpolated width is zero.
					// see synfig::interpolate_width
					// This modification fixes bad render when first widht
					// point is not interpolate after and next widhtpoint is
					//  interpolate before. Noticiable for the FLAT case
					// or when the width is smaller than the step on the bezier.
					ipos=ipos+EPSILON;
					// If we have just done a tip width side tipe after not interpolate
					if(witer->get_side_type_after()!=WidthPoint::TYPE_INTERPOLATE)
						done_tip=true;
					else
						done_tip=false;
					// Keep track of the interpolation withpoints
					if(ipos > scwnext->get_position())
					{
						cwiter=cwnext;
						scwiter=scwnext;
						cwnext++;
						scwnext++;
					}
					// If a widthpoint is over a blinepoint, don't render corners
					middle_corner=false;
					// continue with the main loop
					continue;
				}
			}
			// if we are in the middle of two widthpoints with sides
			// that doesn't produce interpolation, then jump to the
			// next withpoint.
			// or
			// if are doing the first widthpoint of a non blinelooped outline
			// then we need to jump to the first widthpoint
			if(
				(witer->get_side_type_after()!=WidthPoint::TYPE_INTERPOLATE &&
				wnext->get_side_type_before()!=WidthPoint::TYPE_INTERPOLATE)
				||
				(witer==wplist.begin() && wnext==wplist.begin())
				)
			{
				ipos=swnext_pos;
				if(ipos > scwnext->get_position())
					{
						cwiter=cwnext;
						scwiter=scwnext;
						cwnext++;
						scwnext++;
					}
				// we need to consider if we are jumping any bezier too
				while(ipos > bnext_pos && bnext+1!=bend)
				{
					// keep track of last tangent
					// NOTE: keep tangent here is silly, deriv is not updated!
					last_tangent=deriv(1.0-CUSP_TANGENT_ADJUST);
					// Update iterators
					biter=bnext;
					bnext++;
					// Update blinepoints positions
					biter_pos = bnext_pos;
					// todo: why 'hbiter_pos' never used?
					//hbiter_pos = hbnext_pos;
					bpiter++;
					hbpiter++;
					bnext_pos=*bpiter;
					hbnext_pos=*hbpiter;
				}
				// continue with the main loop
				middle_corner=false;
				continue;
			}
			// If we stopped on an intermediate blinepoint (middle corner=true)...
			if(middle_corner==true)
			{
				// ... do cusp at ipos if tangents are splitted
				// Notice that if we are in the second blinepoint
				// for the last bezier, we will be over a widthpoint
				// artificially inserted, so here we only insert cusps
				// for the intermediate blinepoints when looped
				if(split_flag)
				{
					WidthPoint i(*scwiter);
					WidthPoint n(*scwnext);
					if(!fast_)
					{
						i=*cwiter;
						n=*cwnext;
					}
					Real p(ipos);
					if(!fast_)
						p=hipos;
					add_cusp(side_a, side_b, biter->get_vertex(), deriv(CUSP_TANGENT_ADJUST), last_tangent, gv*(expand_+width_*0.5*widthpoint_interpolate(i, n, p, smoothness_)));
				}
				middle_corner=false;
				// This avoid to calculate derivative on q=0
				ipos=ipos+EPSILON;
			}
			do // secondary loop. For interpolation steps.
			{
				// If during the interpolation travel, we passed a widhpoint...
				Real swnext_pos(swnext->get_position());
				if(ipos > swnext_pos && bnext_pos >= swnext_pos)
				{
					// ... just stay on it and ...
					Vector unitary;
					ipos=swnext_pos;
					hipos=wnext_pos;
					// ... add interpolation for the last step
					Real q(bline_to_bezier(ipos, biter_pos, bezier_size));
					if(q<EPSILON)
						unitary=iter_t.norm();
					else if(q>(1.0-EPSILON))
						unitary=next_t.norm();
					else
						unitary=deriv(q).norm();
					const Vector d(unitary.perp());
					const Vector p(curve(q));
					Real ww;
					// last step has width of zero if the widthpoint is not interpolate
					// on the before side.
					if(wnext->get_side_type_before()!=WidthPoint::TYPE_INTERPOLATE)
						ww=0.0;
					else
					{
						if(wnext->get_dash())
						{
							WidthPoint i(*scwiter);
							WidthPoint n(*scwnext);
							if(!fast_)
							{
								i=*cwiter;
								n=*cwnext;
							}
							Real p(ipos);
							if(!fast_)
								p=hipos;
							wnext->set_width(widthpoint_interpolate(i, n, p, smoothness_));
						}
						ww=wnext->get_width();
					}
					const Real w(gv*(expand_+width_*0.5*ww));
					side_a.push_back(p+d*w);
					side_b.push_back(p-d*w);
					// if we haven't passed the position of the second blinepoint
					// we don't want to step back due to the next checking with
					// bnext_pos
					break;
				}
				else if(ipos > bnext_pos && bnext_pos < swnext_pos)
				{
					hipos=hbnext_pos;
					ipos=bnext_pos;
					middle_corner=true;
					//Note: Calculated q should be always equal to 1.0
					// so I commented this code
					Real q(bline_to_bezier(ipos, biter_pos, bezier_size));
					q=q>CUSP_TANGENT_ADJUST?q:CUSP_TANGENT_ADJUST;
					q=q>1.0-CUSP_TANGENT_ADJUST?1.0-CUSP_TANGENT_ADJUST:q;
					const Vector d(deriv(q).perp().norm());
					const Vector p(curve(bline_to_bezier(ipos, biter_pos, bezier_size)));
					WidthPoint i(*scwiter);
					WidthPoint n(*scwnext);
					if(!fast_)
					{
						i=*cwiter;
						n=*cwnext;
					}
					Real po(ipos);
					if(!fast_)
						po=hipos;
					const Real w(gv*(expand_+width_*0.5*widthpoint_interpolate(i, n, po, smoothness_)));
					side_a.push_back(p+d*w);
					side_b.push_back(p-d*w);
					// Update iterators
					biter=bnext;
					bnext++;
					// Update blinepoints positions
					biter_pos = bnext_pos;
					// todo: why 'hbiter_pos' never used?
					//hbiter_pos = hbnext_pos;
					bpiter++;
					hbpiter++;
					bnext_pos=*bpiter;
					hbnext_pos=*hbpiter;
					// remember last tangent value
					last_tangent=deriv(1.0-CUSP_TANGENT_ADJUST);
					break;
				}
				// Add interpolation
				Vector unitary;
				Real q(bline_to_bezier(ipos, biter_pos, bezier_size));
				//if(q==0.0)
					//unitary=iter_t.norm();
				//else if(q==1.0)
					//unitary=next_t.norm();
				//else
					unitary=deriv(q).norm();
				const Vector d(unitary.perp());
				const Vector p(curve(q));
				// if we inserted the widthpoints at start and end, don't consider them for interpolation.
				if(cwiter->get_position() == 0.0 && cwnext->get_position()!=1.0 && inserted_first)
				{
					cwiter=cwplist.end();
					cwiter--;
					if(inserted_last) cwiter--;
				}
				if(cwnext->get_position() == 1.0 && inserted_last)
				{
					cwnext=cwplist.begin();
					if(inserted_first) cwnext++;
				}
				WidthPoint i(*scwiter);
				WidthPoint n(*scwnext);
				if(!fast_)
				{
					i=*cwiter;
					n=*cwnext;
				}
				Real po(ipos);
				if(!fast_)
					po=std_to_hom(bline, ipos, wplistloop, blineloop);
				Real w;
				if(done_tip)
				{
					w=0;
					done_tip=false;
				}
				else
					w=(gv*(expand_+width_*0.5*widthpoint_interpolate(i, n, po, smoothness_)));
				side_a.push_back(p+d*w);
				side_b.push_back(p-d*w);
				ipos = ipos + step;
			} while (1); // secondary loop
		} while(1); // main loop

		// if it is blinelooped, reverse sides and send them to polygon
		if(blineloop)
		{
			reverse(side_b.begin(),side_b.end());
			add_polygon(side_a);
			add_polygon(side_b);
			return;
		}

		// else concatenate sides before add to polygon
		for(;!side_b.empty();side_b.pop_back())
			side_a.push_back(side_b.back());
		add_polygon(side_a);
	}
	catch (...) { synfig::error("Advanced Outline::sync(): Exception thrown"); throw; }
}

bool
Advanced_Outline::set_param(const String & param, const ValueBase &value)
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
	IMPORT_VALUE(param_fast);

	if(param=="vector_list")
		return false;
	return Layer_Polygon::set_param(param,value);
}

void
Advanced_Outline::set_time(IndependentContext context, Time time)const
{
	const_cast<Advanced_Outline*>(this)->sync();
	context.set_time(time);
}

void
Advanced_Outline::set_time(IndependentContext context, Time time, Vector pos)const
{
	const_cast<Advanced_Outline*>(this)->sync();
	context.set_time(time,pos);
}

bool
Advanced_Outline::set_version(const synfig::String &ver)
{
	if (ver=="0.1")
		old_version = true;
	return true;
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
	EXPORT_VALUE(param_fast);

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
		);
	ret.push_back(ParamDesc(ValueBase(),"end_tip")
		.set_local_name(_("Tip Type at End"))
		.set_description(_("Defines the Tip type of the last spline point when spline is unlooped"))
		.set_hint("enum")
		.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
		.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
		.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
		.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
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
	ret.push_back(ParamDesc("fast")
		.set_local_name(_("Fast"))
		.set_description(_("When checked outline renders faster, but less accurate"))
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
Advanced_Outline::add_tip(std::vector<Point> &side_a, std::vector<Point> &side_b, const Point vertex, const Vector tangent, const WidthPoint wp, const Real gv)
{
	Real width_=param_width.get(Real());
	Real expand_=param_expand.get(Real());

	Real w(gv*(expand_+width_*0.5*wp.get_width()));
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
			break;
		}
		case WidthPoint::TYPE_SQUARED:
		{
			side_a.push_back(vertex);
			side_a.push_back(vertex-tangent*w);
			side_a.push_back(vertex+(tangent.perp()-tangent)*w);
			side_a.push_back(vertex+tangent.perp()*w);
			side_b.push_back(vertex);
			side_b.push_back(vertex-tangent*w);
			side_b.push_back(vertex+(-tangent.perp()-tangent)*w);
			side_b.push_back(vertex-tangent.perp()*w);
			break;
		}
		case WidthPoint::TYPE_PEAK:
		{
			side_a.push_back(vertex);
			side_a.push_back(vertex-tangent*w);
			side_a.push_back(vertex+tangent.perp()*w);
			side_b.push_back(vertex);
			side_b.push_back(vertex-tangent*w);
			side_b.push_back(vertex-tangent.perp()*w);
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
	int cusp_type_=param_cusp_type.get(int());

	static int counter=0;
	counter++;
	const Vector t1(last.perp().norm());
	const Vector t2(curr.perp().norm());
	Real cross(t1*t2.perp());
	Real perp((t1-t2).mag());
	switch(cusp_type_)
	{
	case TYPE_SHARP:
		{
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
			break;
		}
	case TYPE_ROUNDED:
		{
			if(cross > 0)
			{
				const Point p1(vertex+t1*w);
				const Point p2(vertex+t2*w);
				Angle::rad offset(t1.angle());
				Angle::rad angle(t2.angle()-offset);
				if(angle < Angle::rad(0) && offset > Angle::rad(0))
				{
					angle+=Angle::deg(360);
					offset+=Angle::deg(360);
				}
				Real tangent(4 * ((2 * Angle::cos(angle/2).get() - Angle::cos(angle).get() - 1) / Angle::sin(angle).get()));
				hermite<Vector> curve(
					p1,
					p2,
					Point(-tangent*w*Angle::sin(angle*0+offset).get(),tangent*w*Angle::cos(angle*0+offset).get()),
					Point(-tangent*w*Angle::sin(angle*1+offset).get(),tangent*w*Angle::cos(angle*1+offset).get())
				);
				for(float n=0.0f;n<0.999999f;n+=4.0f/SAMPLES)
					side_a.push_back(curve(n));
			}
			if(cross < 0)
			{
				const Point p1(vertex-t1*w);
				const Point p2(vertex-t2*w);
				Angle::rad offset(t2.angle());
				Angle::rad angle(t1.angle()-offset);
				if(angle < Angle::rad(0) && offset > Angle::rad(0))
				{
					angle+=Angle::deg(360);
					offset+=Angle::deg(360);
				}
				Real tangent(4 * ((2 * Angle::cos(angle/2).get() - Angle::cos(angle).get() - 1) / Angle::sin(angle).get()));
				hermite<Vector> curve(
					p1,
					p2,
					Point(-tangent*w*Angle::sin(angle*1+offset).get(),tangent*w*Angle::cos(angle*1+offset).get()),
					Point(-tangent*w*Angle::sin(angle*0+offset).get(),tangent*w*Angle::cos(angle*0+offset).get())
				);
				for(float n=0.0f;n<0.999999f;n+=4.0f/SAMPLES)
					side_b.push_back(curve(n));
			}
			break;
		}
	case TYPE_BEVEL:
	default:
		break;
	}
}
