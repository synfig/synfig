/* === S Y N F I G ========================================================= */
/*!	\file renderer_ducks.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2008 Gerald Young
**  Copyright (c) 2011 Nikita Kitaev
**  Copyright (c) 2011 Carlos López
**  Copyright (c) 2015 Jérôme Blanchi
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

#include "renderer_ducks.h"

#include <ETL/bezier>
#include <synfig/distance.h>
#include <synfig/valuenodes/valuenode_wplist.h>
#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/transformation.h>
#include <gui/app.h>
#include <gui/duckmatic.h>
#include <gui/workarea.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/** DUCK_COLOR_NOT_EDITABLE : light grey - for parameter (handle) like converted (linked also?) for example*/
#define DUCK_COLOR_NOT_EDITABLE	Gdk::Color("#cfcfcf")
/** DUCK_COLOR_ORIGIN : green */
#define DUCK_COLOR_ORIGIN        Gdk::Color("#00ff00") // green
/** DUCK_COLOR_ANGLE : blue */
#define DUCK_COLOR_ANGLE		Gdk::Color("#0000ff") // blue
/** DUCK_COLOR_RADIUS : cyan */
#define DUCK_COLOR_RADIUS		Gdk::Color("#00ffff") // cyan
/** DUCK_COLOR_LINEAR : cyan for linear radius ducks */
#define DUCK_COLOR_LINEAR		Gdk::Color("#00ffff") // cyan // for linear radius ducks
/** DUCK_COLOR_TANGENT_1 : yellow */
#define DUCK_COLOR_TANGENT_1	Gdk::Color("#ffff00") // yellow
/** DUCK_COLOR_TANGENT_2 : red (not used) */
#define DUCK_COLOR_TANGENT_2	Gdk::Color("#ff0000") // red
/** DUCK_COLOR_SKEW : red */
#define DUCK_COLOR_SKEW         Gdk::Color("#ff0000") // red
/** DUCK_COLOR_VERTEX : orange */
#define DUCK_COLOR_VERTEX		Gdk::Color("#ff7f00") // orange
/** DUCK_COLOR_WIDTH : magenta */
#define DUCK_COLOR_WIDTH		Gdk::Color("#ff00ff") // magenta
/** DUCK_COLOR_WIDTHPOINT_POSITION : purple */
#define DUCK_COLOR_WIDTHPOINT_POSITION	Gdk::Color("#d3afff") // purple
/** DUCK_COLOR_OTHER : green */
#define DUCK_COLOR_OTHER		Gdk::Color("#00ff00") // green
/** DUCK_COLOR_OUTLINE : black , the outline around each duck*/
#define DUCK_COLOR_OUTLINE		Gdk::Color("#000000") // the outline around each duck
/** DUCK_COLOR_BEZIER_1 : black, the 2 colors used to draw bezier curves */
#define DUCK_COLOR_BEZIER_1		Gdk::Color("#000000") // black // the 2 colors used to draw bezier curves
/** DUCK_COLOR_BEZIER_2 : grey , the second colors used to draw bezier curves*/
#define DUCK_COLOR_BEZIER_2		Gdk::Color("#afafaf") // grey
/** DUCK_COLOR_COLOR_BOX_1 : white , the first color used to draw boxes*/
#define DUCK_COLOR_BOX_1		Gdk::Color("#ffffff") // white // the 2 colors used to draw boxes
/** DUCK_COLOR_BOX_2 : black , the second color used to draw boxes*/
#define DUCK_COLOR_BOX_2		Gdk::Color("#000000") // black
/** DUCK_COLOR_SELECTED : red , the color of the box drawn when a valuenode is selected*/
#define DUCK_COLOR_SELECTED		Gdk::Color("#ff0000") // red // the color of the box drawn when a valuenode is selected
/** DUCK_COLOR_CONNECT_INSIDE : the color of the inside of the line connecting a vertex duck to the tangent ducks */
#define DUCK_COLOR_CONNECT_INSIDE	Gdk::Color("#9fefef") // the color of the inside of the line connecting a vertex duck to the tangent ducks
/** DUCK_COLOR_CONNECT_OUTSIDE : black, the color of the outside of the line connecting a vertex duck to the tangent ducks*/
#define DUCK_COLOR_CONNECT_OUTSIDE	Gdk::Color("#000000") // the color of the outside of the line connecting a vertex duck to the tangent ducks
/** DUCK_COLOR_WIDTH_TEXT_1 : black, the color of the text's shadow when hovering over a width duck*/
#define DUCK_COLOR_WIDTH_TEXT_1	Gdk::Color("#000000") // the color of the text's shadow when hovering over a width duck
/** DUCK_COLOR_WIDTH_TEXT_2 : magenta, the color of the text when hovering over a width duck*/
#define DUCK_COLOR_WIDTH_TEXT_2	Gdk::Color("#ff00ff") // the color of the text when hovering over a width duck
/** DUCK_COLOR_TRANSFO_TEXT_1 : black , the color of the text's shadow when hovering over any duck of a transformation widget*/
#define DUCK_COLOR_TRANSFO_TEXT_1 Gdk::Color("#000000") // the color of the text's shadow when hovering over any duck of a transformation widget
/** ACTIVE_BONE : Color of active bone rectangle */
#define ACTIVE_BONE Gdk::Color("#fff700") // the color of the text's shadow when hovering over any duck of a transformation widget

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_Ducks::~Renderer_Ducks()
{
}

/*
bool
Renderer_Ducks::get_enabled_vfunc()const
{
	return get_work_area()->grid_status();
}
*/

struct ScreenDuck
{
	synfig::Point pos;
	Gdk::Color color;
	Real width;
	bool selected;
	bool hover;
	bool has_alternative;
	bool has_move_origin;

	ScreenDuck():
	    width(0),
		selected(),
		hover(),
		has_alternative(false),
		has_move_origin(false)
	{ }
};

// TODO immense function !! break into parts and clean
void
Renderer_Ducks::render_vfunc(
	const Glib::RefPtr<Gdk::Window>& drawable,
	const Gdk::Rectangle& /*expose_area*/)
{
	assert(get_work_area());
	if(!get_work_area())
		return;

	const synfig::Point window_start(get_work_area()->get_window_tl());
	const float pw(get_pw()),ph(get_ph());

	const bool solid_lines(get_work_area()->solid_lines);
	bool alternative = get_work_area()->get_alternative_mode();

	const std::list<etl::handle<Duckmatic::Bezier> >& bezier_list(get_work_area()->bezier_list());
	const std::list<handle<Duckmatic::Stroke> >& stroke_list(get_work_area()->stroke_list());
	Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_work_area()->get_pango_context()));

	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();

	cr->save();
	cr->set_line_cap(Cairo::LINE_CAP_BUTT);
	cr->set_line_join(Cairo::LINE_JOIN_MITER);

	// Render the strokes
	Gamma gamma = App::get_selected_canvas_gamma().get_inverted();
	for(std::list<handle<Duckmatic::Stroke> >::const_iterator iter=stroke_list.begin();iter!=stroke_list.end();++iter)
	{
		cr->save();

		std::list<synfig::Point>::iterator iter2;
		for(iter2=(*iter)->stroke_data->begin();iter2!=(*iter)->stroke_data->end();++iter2)
			if (!iter2->is_nan_or_inf())
				cr->line_to(
					((*iter2)[0]-window_start[0])/pw,
					((*iter2)[1]-window_start[1])/ph );

		cr->set_line_width(1.0);
		synfig::Color c = gamma.apply((*iter)->color);
		cr->set_source_rgb(c.get_r(), c.get_g(), c.get_b());
		cr->stroke();

		cr->restore();
	}



	// Render the beziers
	for(std::list<handle<Duckmatic::Bezier> >::const_iterator iter=bezier_list.begin();iter!=bezier_list.end();++iter)
	{
		Point p1((*iter)->p1->get_trans_point()-window_start);
		Point p2((*iter)->p2->get_trans_point()-window_start);
		Point c1((*iter)->c1->get_trans_point()-window_start);
		Point c2((*iter)->c2->get_trans_point()-window_start);
		if (p1.is_nan_or_inf()
		 || p2.is_nan_or_inf()
		 || c1.is_nan_or_inf()
		 || c2.is_nan_or_inf() )
			continue;

		p1[0]/=pw;p1[1]/=ph;
		p2[0]/=pw;p2[1]/=ph;
		c1[0]/=pw;c1[1]/=ph;
		c2[0]/=pw;c2[1]/=ph;

		cr->save();

		cr->move_to(p1[0], p1[1]);
		cr->curve_to(c1[0], c1[1], c2[0], c2[1], p2[0], p2[1]);

/*
		if (solid_lines)
		{
			cr->set_source_rgb(0,0,0); // DUCK_COLOR_BEZIER_1
			cr->set_line_width(3.0);
			cr->stroke_preserve();

			cr->set_source_rgb(175.0/255.0,175.0/255.0,175.0/255.0); //DUCK_COLOR_BEZIER_2
			cr->set_line_width(1.0);
			cr->stroke();
		}
		else
*/
		{
			//Solid line background
			cr->set_line_width(1.0);
			cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_BEZIER_1)); // DUCK_COLOR_BEZIER_1
			cr->stroke_preserve();

			//Dashes
			cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_BEZIER_2)); //DUCK_COLOR_BEZIER_2 : 175.0/255.0,175.0/255.0,175.0/255.0
			std::valarray<double> dashes(2);
			dashes[0]=5.0;
			dashes[1]=5.0;
			cr->set_dash(dashes, 0);
			cr->stroke();
		}
		cr->restore();
	}


	const DuckList duck_list(get_work_area()->get_duck_list());

	std::list<ScreenDuck> screen_duck_list;
	const float radius((abs(pw)+abs(ph))*4);

	etl::handle<Duck> hover_duck(get_work_area()->find_duck(get_work_area()->get_cursor_pos(),radius, get_work_area()->get_type_mask()));

	// Render the ducks
	for(std::list<handle<Duck> >::const_iterator iter=duck_list.begin();iter!=duck_list.end();++iter)
	{

		// If this type of duck has been masked, then skip it
		if(!(*iter)->get_type() || (!(get_work_area()->get_type_mask() & (*iter)->get_type())))
			continue;

		Point sub_trans_point((*iter)->get_sub_trans_point());
		Point sub_trans_origin((*iter)->get_sub_trans_origin());
		if (sub_trans_point.is_nan_or_inf() || sub_trans_origin.is_nan_or_inf())
			continue;

		if (App::restrict_radius_ducks &&
			(*iter)->is_radius())
		{
			if (sub_trans_point[0] < sub_trans_origin[0])
				sub_trans_point[0] = sub_trans_origin[0];
			if (sub_trans_point[1] < sub_trans_origin[1])
				sub_trans_point[1] = sub_trans_origin[1];
		}

		Point point((*iter)->get_transform_stack().perform(sub_trans_point));
		Point origin((*iter)->get_transform_stack().perform(sub_trans_origin));

		point[0]=(point[0]-window_start[0])/pw;
		point[1]=(point[1]-window_start[1])/ph;

		bool has_connect = (*iter)->get_tangent()
		                || ((*iter)->get_type()&( Duck::TYPE_ANGLE
		                					   | Duck::TYPE_SKEW
		        		                       | Duck::TYPE_SCALE_X
		        		                       | Duck::TYPE_SCALE_Y ));
		if((*iter)->get_connect_duck())
		{
			has_connect=true;
			origin=(*iter)->get_connect_duck()->get_trans_point();
		}

		origin[0]=(origin[0]-window_start[0])/pw;
		origin[1]=(origin[1]-window_start[1])/ph;

		if (point.is_nan_or_inf() || origin.is_nan_or_inf())
			continue;

		bool selected(get_work_area()->duck_is_selected(*iter));
		bool hover(*iter==hover_duck || (*iter)->get_hover());

		if(get_work_area()->get_selected_value_node())
		{
			synfigapp::ValueDesc value_desc((*iter)->get_value_desc());
			if (value_desc.is_valid() &&
				((value_desc.is_value_node()		&& get_work_area()->get_selected_value_node() == value_desc.get_value_node()) ||
				 (value_desc.parent_is_value_node()	&& get_work_area()->get_selected_value_node() == value_desc.get_parent_value_node())))
			{
				cr->save();

				cr->rectangle(
					round_to_int(point[0]-5),
					round_to_int(point[1]-5),
					10,
					10
					);

				cr->set_line_width(2.0);
				cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_SELECTED)); //DUCK_COLOR_SELECTED
				cr->stroke();

				cr->restore();
			}

		}
		if(get_work_area()->get_active_bone_value_node() && get_work_area()->get_active_bone_display())
		{
			synfigapp::ValueDesc value_desc((*iter)->get_value_desc());
			if (value_desc.is_valid() &&
				((value_desc.is_value_node()		&& get_work_area()->get_active_bone_value_node() == value_desc.get_value_node()) ||
				 (value_desc.parent_is_value_node()	&& get_work_area()->get_active_bone_value_node() == value_desc.get_parent_value_node())))
			{
				cr->save();

				cr->rectangle(
					round_to_int(point[0]-5),
					round_to_int(point[1]-5),
					10,
					10
					);

				cr->set_line_width(2.0);
				cr->set_source_rgb(GDK_COLOR_TO_RGB(ACTIVE_BONE)); //DUCK_COLOR_SELECTED
				cr->stroke();

				cr->restore();
			}

		}

		if((*iter)->get_box_duck())
		{
			Point boxpoint((*iter)->get_box_duck()->get_trans_point());
			boxpoint[0]=(boxpoint[0]-window_start[0])/pw;
			boxpoint[1]=(boxpoint[1]-window_start[1])/ph;
			Point tl(min(point[0],boxpoint[0]),min(point[1],boxpoint[1]));

			if (!boxpoint.is_nan_or_inf() && !tl.is_nan_or_inf())
			{
				cr->save();

				cr->rectangle(
					round_to_int(tl[0]),
					round_to_int(tl[1]),
					round_to_int(abs(boxpoint[0]-point[0])),
					round_to_int(abs(boxpoint[1]-point[1]))
					);

				// Solid white box
				cr->set_line_width(1.0);
				cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_BOX_1)); //DUCK_COLOR_BOX_1
				cr->stroke_preserve();

				// Dashes
				cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_BOX_2)); //DUCK_COLOR_BOX_2
				std::valarray<double> dashes(2);
				dashes[0]=5.0;
				dashes[1]=5.0;
				cr->set_dash(dashes, 0);
				cr->stroke();

				cr->restore();
			}
		}

		if((*iter)->is_axes_tracks())
		{
			Point pos((*iter)->get_point());
			Point points[] = {
				(*iter)->get_sub_trans_origin(),
				(*iter)->get_sub_trans_point(Point(pos[0],0)),
				(*iter)->get_sub_trans_point(),
				(*iter)->get_sub_trans_point(Point(0,pos[1])),
				(*iter)->get_sub_trans_origin()
			};

			cr->save();

			bool first = true;
			for(int i = 0; i < 5; i++) {
				Point p((*iter)->get_transform_stack().perform(points[i]));
				p[0] = (p[0]-window_start[0])/pw;
				p[1] = (p[1]-window_start[1])/ph;
				if (!p.is_nan_or_inf()) {
					if (first)
						{ first = false; cr->move_to(p[0], p[1]); }
					else
						cr->line_to(p[0], p[1]);
				}
			}

			// Solid white box
			cr->set_line_width(1.0);
			cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_BOX_1)); //DUCK_COLOR_BOX_1
			cr->stroke_preserve();

			// Dashes
			cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_BOX_2)); //DUCK_COLOR_BOX_2
			std::valarray<double> dashes(2);
			dashes[0]=5.0;
			dashes[1]=5.0;
			cr->set_dash(dashes, 0);
			cr->stroke();

			cr->restore();
		}

		ScreenDuck screen_duck;
		screen_duck.pos=point;
		screen_duck.selected=selected;
		screen_duck.hover=hover;
		screen_duck.has_alternative=(*iter)->get_alternative_value_desc().is_valid();
		screen_duck.has_move_origin=(*iter)->get_move_origin();

		bool splited_angle=false;
		bool splited_radius=false;
		bool is_bline_point=false;

		if(!(*iter)->get_editable(alternative))
			screen_duck.color=(DUCK_COLOR_NOT_EDITABLE);
		else if((*iter)->get_tangent() && (*iter)->get_value_desc().get_value_type() != type_transformation)
		{
		    is_bline_point=true;
		    // Check if we can reach the canvas and set the time to
		    // evaluate the split value accordingly
		    synfig::Canvas::Handle canvas_h(get_work_area()->get_canvas());
		    synfig::Time time(canvas_h?canvas_h->get_time():synfig::Time(0));
		    // Retrieve the split value of the bline point.
		    const synfigapp::ValueDesc& v_d((*iter)->get_value_desc());

		    synfig::ValueNode_Composite::Handle value_node;
		    if(v_d.is_value_node() && v_d.get_value_node())
            {
                if (synfig::ValueNode_Composite::Handle value_node = synfig::ValueNode_Composite::Handle::cast_dynamic( v_d.get_value_node() ))
                {
					try
					{
						synfig::ValueNode::Handle child(value_node->get_link("split_angle"));
						if(synfig::ValueNode_Animated::Handle::cast_dynamic(child))
						{
							synfig::ValueNode_Animated::Handle animated_child(
									synfig::ValueNode_Animated::Handle::cast_dynamic(child));
							splited_angle=animated_child->new_waypoint_at_time(time).get_value(time).get(bool());
						}
						else if(synfig::ValueNode_Const::Handle::cast_dynamic(child))
						{
							synfig::ValueNode_Const::Handle const_child(
									synfig::ValueNode_Const::Handle::cast_dynamic(child));
							splited_angle=(const_child->get_value()).get(bool());
						}
					}
					catch(Exception::BadLinkName&)
					{

					}

					try
					{
						synfig::ValueNode::Handle child(value_node->get_link("split_radius"));
						if(synfig::ValueNode_Animated::Handle::cast_dynamic(child))
						{
							synfig::ValueNode_Animated::Handle animated_child(
									synfig::ValueNode_Animated::Handle::cast_dynamic(child));
							splited_radius=animated_child->new_waypoint_at_time(time).get_value(time).get(bool());
						}
						else if(synfig::ValueNode_Const::Handle::cast_dynamic(child))
						{
							synfig::ValueNode_Const::Handle const_child(
									synfig::ValueNode_Const::Handle::cast_dynamic(child));
							splited_radius=(const_child->get_value()).get(bool());
						}
					}
					catch(Exception::BadLinkName&)
					{

					}
                }
            }

			if(false){
				// Tangents ducks have different color depending on the split state (disabled for now)
			    screen_duck.color=(splited_angle? DUCK_COLOR_TANGENT_2 : DUCK_COLOR_TANGENT_1);

			} else {
				// All tangents are the same color
				screen_duck.color=((*iter)->get_scalar()<0 ? DUCK_COLOR_TANGENT_1 : DUCK_COLOR_TANGENT_1);
			}
		}
		else if((*iter)->get_type()&Duck::TYPE_SKEW)
			screen_duck.color=DUCK_COLOR_SKEW;
		else if((*iter)->get_type()&Duck::TYPE_VERTEX)
			screen_duck.color=DUCK_COLOR_VERTEX;
		else if((*iter)->get_type()&Duck::TYPE_SCALE)
			screen_duck.color=DUCK_COLOR_VERTEX;
		else if((*iter)->get_type()&Duck::TYPE_RADIUS)
			screen_duck.color=((*iter)->is_linear() ? DUCK_COLOR_LINEAR : DUCK_COLOR_RADIUS);
		else if(((*iter)->get_type()&Duck::TYPE_SCALE_X) || ((*iter)->get_type()&Duck::TYPE_SCALE_Y))
			screen_duck.color=DUCK_COLOR_TANGENT_1;
		else if((*iter)->get_type()&Duck::TYPE_WIDTH)
			screen_duck.color=DUCK_COLOR_WIDTH;
		else if((*iter)->get_type()&Duck::TYPE_ANGLE)
			screen_duck.color=(DUCK_COLOR_ANGLE);
		else if((*iter)->get_type()&Duck::TYPE_WIDTHPOINT_POSITION)
			screen_duck.color=(DUCK_COLOR_WIDTHPOINT_POSITION);
		else if ((*iter)->get_move_origin())
			screen_duck.color=(DUCK_COLOR_WIDTHPOINT_POSITION);
		else
			screen_duck.color=DUCK_COLOR_OTHER;

		screen_duck_list.push_front(screen_duck);

		if(has_connect)
		{
			cr->save();

			cr->move_to(origin[0], origin[1]);
			cr->line_to(point[0], point[1]);

//			if(solid_lines) // Disabled (solid_lines metadata entry)
			{
				// Outside
				cr->set_line_width(3.0);
				cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_CONNECT_OUTSIDE)); //DUCK_COLOR_CONNECT_OUTSIDE
				cr->stroke_preserve();

				// Both tangents are collinear and have same length, dash the line
				if(is_bline_point && !splited_radius && !splited_angle)
				{
				    std::valarray<double> dashes(2);
				    dashes[0]=5.0;
				    dashes[1]=5.0;
				    cr->set_dash(dashes, 0);
				}

				// Inside
				cr->set_line_width(1.0);
				cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_CONNECT_INSIDE)); //DUCK_COLOR_CONNECT_INSIDE : 159.0/255,239.0/255,239.0/255
				cr->stroke();
			}
//			else
//			{
//				// White background
//				cr->set_line_width(1.0);
//				cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_CONNECT_OUTSIDE)); //DUCK_COLOR_CONNECT_OUTSIDE
//				cr->stroke_preserve();
//
//				// Dashes on top of the background
//				cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_CONNECT_INSIDE)); //DUCK_COLOR_CONNECT_INSIDE : 159.0/255,239.0/255,239.0/255
//				std::valarray<double> dashes(2);
//				dashes[0]=5.0;
//				dashes[1]=5.0;
//				cr->set_dash(dashes, 0);
//				cr->stroke();
//			}

			cr->restore();
		}

		if((*iter)->is_radius())
		{
			if (!(*iter)->is_linear())
			{
				const Real mag((point-origin).mag());

				cr->save();

				cr->arc(
					origin[0],
					origin[1],
					mag,
					0,
					M_PI*2
					);

				if(solid_lines)
				{
					cr->set_line_width(3.0);
					cr->set_source_rgb(0,0,0);
					cr->stroke_preserve();

					cr->set_source_rgb(175.0/255.0,175.0/255.0,175.0/255.0);
				}
				else
				{
					cr->set_source_rgb(1.0,1.0,1.0);

					// Operator difference was added in Cairo 1.9.4
					// It currently isn't supported by Cairomm
	#if CAIRO_VERSION >= 10904
					cairo_set_operator(cr->cobj(), CAIRO_OPERATOR_DIFFERENCE);
	#else
					// Fallback: set color to black
					cr->set_source_rgb(0,0,0);
	#endif

				}

				cr->set_line_width(1.0);
				cr->stroke();

				cr->restore();
			}

		}

		//! Tooltip time
		if(hover)
		{
			//! Tooltip time : radius
			if( (App::ui_handle_tooltip_flag&Duck::STRUCT_RADIUS) &&
					(*iter)->is_radius())
			{
				Real mag;
				if ((*iter)->get_exponential()){
					mag = log((*iter)->get_point().mag());
				}
				else if (App::restrict_radius_ducks)
				{
					Point sub_trans_point((*iter)->get_sub_trans_point());
					Point sub_trans_origin((*iter)->get_sub_trans_origin());

					if (sub_trans_point[0] < sub_trans_origin[0])
						sub_trans_point[0] = sub_trans_origin[0];
					if (sub_trans_point[1] < sub_trans_origin[1])
						sub_trans_point[1] = sub_trans_origin[1];

					Point point((*iter)->get_transform_stack().perform(sub_trans_point));
					Point origin((*iter)->get_transform_stack().perform(sub_trans_origin));

					mag = (point-origin).mag();
				}
				else
					mag = ((*iter)->get_trans_point()-(*iter)->get_trans_origin()).mag();

				Distance real_mag(mag, Distance::SYSTEM_UNITS);
				if (!(*iter)->get_exponential()) {
					Canvas::Handle canvas = get_work_area()->get_canvas();
					real_mag.convert(App::distance_system, canvas ? canvas->rend_desc() : RendDesc());
				}

				cr->save();

				layout->set_text(real_mag.get_string());

				cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_WIDTH_TEXT_1)); // DUCK_COLOR_WIDTH_TEXT_1
				cr->move_to(
					point[0]+1+6,
					point[1]+1-8
					);
				layout->show_in_cairo_context(cr);
				cr->stroke();


				cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_WIDTH_TEXT_2)); // DUCK_COLOR_WIDTH_TEXT_2
				cr->move_to(
					point[0]+6,
					point[1]-8
					);
				layout->show_in_cairo_context(cr);
				cr->stroke();

				cr->restore();
			}
			//! Tooltip time : width point position
			if( (App::ui_handle_tooltip_flag&Duck::STRUCT_WIDTHPOINT) &&
					(*iter)->get_type() & Duck::TYPE_WIDTHPOINT_POSITION)
			{
				synfig::Canvas::Handle canvas_h(get_work_area()->get_canvas());
				synfig::Time time(canvas_h?canvas_h->get_time():synfig::Time(0));
				synfigapp::ValueDesc value_desc((*iter)->get_value_desc());
				synfig::ValueNode_WPList::Handle wplist;
				ValueNode_Composite::Handle wpoint_composite;
				Real radius=0.0;
				Real new_value;
				Point p(sub_trans_point-sub_trans_origin);
				if(value_desc.parent_is_value_node())
					wplist=synfig::ValueNode_WPList::Handle::cast_dynamic(value_desc.get_parent_value_node());
				if(wplist)
				{
					bool wplistloop(wplist->get_loop());
					synfig::ValueNode_BLine::Handle bline(synfig::ValueNode_BLine::Handle::cast_dynamic(wplist->get_bline()));
					wpoint_composite=ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node());
					if(bline && wpoint_composite)
					{
						bool blineloop(bline->get_loop());
						bool homogeneous=false;
						// Retrieve the homogeneous layer parameter
						std::set<Node*>::iterator iter;
						for(iter=wplist->parent_set.begin();iter!=wplist->parent_set.end();++iter)
							{
								Layer::Handle layer;
								layer=Layer::Handle::cast_dynamic(*iter);
								if(layer && layer->get_name() == "advanced_outline")
								{
									homogeneous=layer->get_param("homogeneous").get(bool());
									break;
								}
							}
						WidthPoint wp((*wpoint_composite)(time).get(WidthPoint()));
						if(wplistloop)
						{
							// The wplist is looped. This may require a position parameter
							// outside the range of 0-1, so make sure that the position doesn't
							// change drastically.
							// First normalise the current position
							Real value_old(wp.get_norm_position(wplistloop));
							Real value_old_b(wp.get_bound_position(wplistloop));
							// If it is homogeneous then convert it to standard
							value_old=homogeneous?hom_to_std((*bline)(time), value_old, wplistloop, blineloop):value_old;
							// grab a new position given by duck's position on the bline
							Real value_new = synfig::find_closest_point((*bline)(time), p , radius, blineloop);
							// calculate the difference between old and new positions
							Real difference = fmod( fmod(value_new - value_old, 1.0) + 1.0 , 1.0);
							//fmod is called twice to avoid negative values
							if (difference > 0.5)
								difference=difference-1.0;
							// calculate a new value for the position
							new_value=value_old+difference;
							// restore the homogeneous value if needed
							new_value = homogeneous?std_to_hom((*bline)(time), new_value, wplistloop, blineloop):new_value;
							// this is the difference between the new value and the old value inside the boundaries
							Real bound_diff((wp.get_lower_bound() + new_value*(wp.get_upper_bound()-wp.get_lower_bound()))-value_old_b);
							// add the new diff to the current value
							new_value = wp.get_position() + bound_diff;
						}
						else
						{
							// grab a new position given by duck's position on the bline
							new_value = synfig::find_closest_point((*bline)(time), p , radius, blineloop);
							// if it is homogeneous then convert to it
							new_value=homogeneous?std_to_hom((*bline)(time), new_value, wplistloop, blineloop):new_value;
							// convert the value inside the boundaries
							new_value = wp.get_lower_bound()+new_value*(wp.get_upper_bound()-wp.get_lower_bound());
						}
						cr->save();
						layout->set_text(strprintf("%2.3f", new_value));

						cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_WIDTH_TEXT_1)); // DUCK_COLOR_WIDTH_TEXT_1
						cr->move_to(
							point[0]+1+6,
							point[1]+1-18
							);
						layout->show_in_cairo_context(cr);
						cr->stroke();


						cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_WIDTH_TEXT_2)); // DUCK_COLOR_WIDTH_TEXT_2
						cr->move_to(
							point[0]+6,
							point[1]-18
							);
						layout->show_in_cairo_context(cr);
						cr->stroke();

						cr->restore();
					}
				}
			}
			//! Tooltip time : transformation widget, display layer name &| value
			if( ((App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFORMATION) ||
				 (App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFO_BY_VALUE)) &&
					((*iter)->get_value_desc().is_value_node()) &&
					((*iter)->get_value_desc().get_value_type() == type_transformation)
					)
			{
				cr->save();
				String tooltiptext("");
				if(App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFORMATION)
					tooltiptext = (*iter)->get_value_desc().get_layer()->get_non_empty_description();

				if(App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFO_BY_VALUE)
				{
					//TODO translation of subname
					String value = (*iter)->get_value_desc().get_sub_name();

					synfig::Canvas::Handle canvas_h(get_work_area()->get_canvas());
					synfig::Time time(canvas_h?canvas_h->get_time():synfig::Time(0));
					Transformation transformation = (*iter)->get_value_desc().get_value(time).get(Transformation());

					const RendDesc rend_desc = canvas_h ? canvas_h->rend_desc() : RendDesc();
					switch((*iter)->get_type()) {
					case Duck::TYPE_POSITION:
					{
						Distance posx(sub_trans_point[0],Distance::SYSTEM_UNITS);
						posx.convert(App::distance_system,rend_desc);
						Distance posy(sub_trans_point[1],Distance::SYSTEM_UNITS);
						posy.convert(App::distance_system,rend_desc);

						value += " " + posx.get_string(3) + " " + posy.get_string(3);
					}
					break;
					case Duck::TYPE_ANGLE:
						value+=strprintf(" %2.2g°",
								synfig::Angle::deg(transformation.angle + ((*iter)->get_rotations())).get());
						break;
					case Duck::TYPE_SKEW:
						value+=strprintf(" %2.2g°",
								synfig::Angle::deg(transformation.skew_angle + ((*iter)->get_rotations())).get());
						break;
					case Duck::TYPE_SCALE:
					{
						Distance scalex(transformation.scale.multiply_coords((*iter)->get_point())[0],
								Distance::SYSTEM_UNITS);
						scalex.convert(App::distance_system,rend_desc);
						Distance scaley(transformation.scale.multiply_coords((*iter)->get_point())[1],
								Distance::SYSTEM_UNITS);
						scaley.convert(App::distance_system,rend_desc);

						value += " " + scalex.get_string(3) + " " +	scaley.get_string(3);
					}
					break;
					case Duck::TYPE_SCALE_X:
					{
						Distance scalex(transformation.scale[0] * ((*iter)->get_point())[0],
								Distance::SYSTEM_UNITS);
						scalex.convert(App::distance_system,rend_desc);
						value += "x" + scalex.get_string(3);
					}
					break;
					case Duck::TYPE_SCALE_Y:
					{
						Distance scaley(transformation.scale[1] * ((*iter)->get_point())[0],
								Distance::SYSTEM_UNITS);
						scaley.convert(App::distance_system,rend_desc);
						value += "y" + scaley.get_string(3);
					}
//						value += strprintf("y %2.3g",
//								Distance(transformation.scale[1] * ((*iter)->get_point())[0],
//										Distance::SYSTEM_UNITS).get(App::distance_system,rend_desc));
						break;
					default:
						break;
					}

					tooltiptext = App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFORMATION? tooltiptext + "\n" + value : value;
				}

				layout->set_text(tooltiptext);

				cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_TRANSFO_TEXT_1));
				cr->move_to(
					point[0]+1+14,
					point[1]+1-14
					);
				layout->show_in_cairo_context(cr);
				cr->stroke();

				cr->set_source_rgb(GDK_COLOR_TO_RGB(DUCK_COLOR_ORIGIN));
				cr->move_to(
					point[0]+14,
					point[1]-14
					);
				layout->show_in_cairo_context(cr);
				cr->stroke();

				cr->restore();
			}
		}//! end if hover

	}

	for(;!screen_duck_list.empty();screen_duck_list.pop_front())
	{
		Gdk::Color color(screen_duck_list.front().color);
		double radius = 4;
		double outline = 1;
		bool duck_alternative = alternative && screen_duck_list.front().has_alternative;
		bool duck_move_origin = screen_duck_list.front().has_move_origin;

		// Draw the hovered duck last (on top of everything)
		if(screen_duck_list.front().hover && !screen_duck_list.back().hover && screen_duck_list.size()>1)
		{
			screen_duck_list.push_back(screen_duck_list.front());
			continue;
		}

		cr->save();

		if(!screen_duck_list.front().selected)
		{
		    color.set_rgb_p(color.get_red_p()*2/3,
		                    color.get_green_p()*2/3,
		                    color.get_blue_p()*2/3);
		}

		if(screen_duck_list.front().hover)
		{
			radius += 1;
			outline += 1;
		}

		cr->arc(
			screen_duck_list.front().pos[0],
			screen_duck_list.front().pos[1],
			radius,
			0,
			M_PI*2
			);

		cr->set_source_rgba(
			color.get_red_p(),
			color.get_green_p(),
			color.get_blue_p(),
			duck_alternative || duck_move_origin ? 0.5 : 1.0
			);
		cr->fill_preserve();

		cr->set_line_width(outline);
		cr->set_source_rgba(GDK_COLOR_TO_RGB(DUCK_COLOR_OUTLINE),1); //DUCK_COLOR_OUTLINE
		cr->stroke();

		cr->restore();
	}
}
