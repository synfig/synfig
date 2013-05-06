/* === S Y N F I G ========================================================= */
/*!	\file region.cpp
**	\brief Implementation of the "Region" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "region.h"
#include <ETL/stringf>
#include <ETL/bezier>
#include <ETL/hermite>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>
#include <synfig/valuenode_bline.h>

#endif

using namespace etl;

/* === M A C R O S ========================================================= */

#define SAMPLES		75

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Region);
SYNFIG_LAYER_SET_NAME(Region,"region");
SYNFIG_LAYER_SET_LOCAL_NAME(Region,N_("Region"));
SYNFIG_LAYER_SET_CATEGORY(Region,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Region,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Region,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Region::Region()
{
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
	bline=bline_point_list;
}

void
Region::sync()
{
	if(bline.get_contained_type()==ValueBase::TYPE_BLINEPOINT)
		segment_list=convert_bline_to_segment_list(bline);
	else if(bline.get_contained_type()==ValueBase::TYPE_SEGMENT)
		segment_list=vector<synfig::Segment>(bline.get_list().begin(), bline.get_list().end());
	else
	{
		synfig::warning("Region: incorrect type on bline, layer disabled");
		clear();
		return;
	}

	if(segment_list.empty())
	{
		synfig::warning("Region: segment_list is empty, layer disabled");
		clear();
		return;
	}

	bool looped = bline.get_loop();

	Vector::value_type n;
	etl::hermite<Vector> curve;
	vector<Point> vector_list;

	vector<Segment>::const_iterator iter=segment_list.begin();
	//Vector							last = iter->p1;

	//make sure the shape has a clean slate for writing
	//clear();

	//and start off at the first point
	//move_to(last[0],last[1]);

	for(;iter!=segment_list.end();++iter)
	{
		//connect them with a line if they aren't already joined
		/*if(iter->p1 != last)
		{
			line_to(iter->p1[0],iter->p1[1]);
		}

		//curve to the next end point
		curve_to(iter->p1[0] + iter->t1[0]/3.0,iter->p1[1] + iter->t1[1]/3.0,
				 iter->p2[0] - iter->t2[0]/3.0,iter->p2[1] - iter->t2[1]/3.0,
				 iter->p2[0],iter->p2[1]);

		last = iter->p2;*/

		if(iter->t1.is_equal_to(Vector(0,0)) && iter->t2.is_equal_to(Vector(0,0)))
		{
			vector_list.push_back(iter->p2);
		}
		else
		{
			curve.p1()=iter->p1;
			curve.t1()=iter->t1;
			curve.p2()=iter->p2;
			curve.t2()=iter->t2;
			curve.sync();

			for(n=0.0;n<1.0;n+=1.0/SAMPLES)
				vector_list.push_back(curve(n));
		}
	}

	//add the starting point onto the end so it actually fits the shape, so we can be extra awesome...
	if(!looped)
		vector_list.push_back(segment_list[0].p1);

	clear();
	add_polygon(vector_list);

	/*close();
	endpath();*/
}

bool
Region::set_param(const String & param, const ValueBase &value)
{
	if(param=="segment_list")
	{
		if(dynamic_param_list().count("segment_list"))
		{
			connect_dynamic_param("bline",dynamic_param_list().find("segment_list")->second);
			disconnect_dynamic_param("segment_list");
			synfig::warning("Region::set_param(): Updated valuenode connection to use the new \"bline\" parameter.");
		}
		else
			synfig::warning("Region::set_param(): The parameter \"segment_list\" is deprecated. Use \"bline\" instead.");
	}

	if(	(param=="segment_list" || param=="bline") && value.get_type()==ValueBase::TYPE_LIST)
	{
		//if(value.get_contained_type()!=ValueBase::TYPE_BLINEPOINT)
		//	return false;

		bline=value;

		return true;
	}

/*	if(	param=="segment_list" && value.get_type()==ValueBase::TYPE_LIST)
	{
		if(value.get_contained_type()==ValueBase::TYPE_BLINEPOINT)
			segment_list=convert_bline_to_segment_list(value);
		else
		if(value.get_contained_type()==ValueBase::TYPE_SEGMENT)
			segment_list=value;
		else
		if(value.empty())
			segment_list.clear();
		else
			return false;
		sync();
		return true;
	}
	*/
	return Layer_Shape::set_param(param,value);
}

ValueBase
Region::get_param(const String& param)const
{
	EXPORT(bline);
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Shape::get_param(param);
}

Layer::Vocab
Region::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Shape::get_param_vocab());

	ret.push_back(ParamDesc("bline")
		.set_local_name(_("Vertices"))
		.set_origin("origin")
		.set_description(_("A list of spline points"))
	);

	return ret;
}

void
Region::set_time(Context context, Time time)const
{
	const_cast<Region*>(this)->sync();
	context.set_time(time);
}

void
Region::set_time(Context context, Time time, Vector pos)const
{
	const_cast<Region*>(this)->sync();
	context.set_time(time,pos);
}

/////////
bool
Region::accelerated_cairorender(Context context,cairo_surface_t *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
//	synfig::info("rendering Cairo Region");
	std::vector<synfig::Segment> segments;

	// Grab the rgba values
	const float r(color.get_r());
	const float g(color.get_g());
	const float b(color.get_b());
	const float a(color.get_a());
	
	// Window Boundaries
	const Point	tl(renddesc.get_tl());
	const Point br(renddesc.get_br());
	const int	w(renddesc.get_w());
	const int	h(renddesc.get_h());
	
	// Width and Height of a pixel
	const Real pw = (br[0] - tl[0]) / w;
	const Real ph = (br[1] - tl[1]) / h;
	
	// These are the scale and translation values
	const double sx(1/pw);
	const double sy(1/ph);
	const double tx((-tl[0]+origin[0])*sx);
	const double ty((-tl[1]+origin[1])*sy);
	
	if(bline.get_contained_type()==ValueBase::TYPE_BLINEPOINT)
		segments=convert_bline_to_segment_list(bline);
	else if(bline.get_contained_type()==ValueBase::TYPE_SEGMENT)
		segments=vector<synfig::Segment>(bline.get_list().begin(), bline.get_list().end());
	else
	{
		synfig::warning("Region: incorrect type on bline, layer disabled");
		return false;
	}
	
	if(segments.empty())
	{
		synfig::warning("Region: segment_list is empty, layer disabled");
		return false;
	}
		
	cairo_t* cr=cairo_create(surface);
	// Let's render the region in other surface
	// Initially I'll fill it completely with the alpha color
	cairo_surface_t* subimage;
	// Let's calculate the subimage dimensions based on the feather value
	//so make a separate surface
	RendDesc	workdesc(renddesc);
	int halfsizex(0), halfsizey(0);
	if(feather && quality != 10)
	{
		//the expanded size = 1/2 the size in each direction rounded up
		halfsizex = (int) (abs(feather*.5/pw) + 3),
		halfsizey = (int) (abs(feather*.5/ph) + 3);
		
		//expand by 1/2 size in each direction on either side
		switch(blurtype)
		{
			case Blur::DISC:
			case Blur::BOX:
			case Blur::CROSS:
			{
				workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),w+2*max(1,halfsizex),h+2*max(1,halfsizey));
				break;
			}
			case Blur::FASTGAUSSIAN:
			{
				if(quality < 4)
				{
					halfsizex*=2;
					halfsizey*=2;
				}
				workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),w+2*max(1,halfsizex),h+2*max(1,halfsizey));
				break;
			}
			case Blur::GAUSSIAN:
			{
#define GAUSSIAN_ADJUSTMENT		(0.05)
				Real	pw = (Real)workdesc.get_w()/(workdesc.get_br()[0]-workdesc.get_tl()[0]);
				Real 	ph = (Real)workdesc.get_h()/(workdesc.get_br()[1]-workdesc.get_tl()[1]);
				
				pw=pw*pw;
				ph=ph*ph;
				
				halfsizex = (int)(abs(pw)*feather*GAUSSIAN_ADJUSTMENT+0.5);
				halfsizey = (int)(abs(ph)*feather*GAUSSIAN_ADJUSTMENT+0.5);
				
				halfsizex = (halfsizex + 1)/2;
				halfsizey = (halfsizey + 1)/2;
				workdesc.set_subwindow( -halfsizex, -halfsizey, w+2*halfsizex, h+2*halfsizey );
				break;
#undef GAUSSIAN_ADJUSTMENT
			}
		}
	}
	subimage=cairo_surface_create_similar(surface, CAIRO_CONTENT_COLOR_ALPHA, workdesc.get_w(), workdesc.get_h());
	cairo_t* subcr=cairo_create(subimage);
	cairo_save(subcr);
	cairo_set_source_rgba(subcr, r, g, b, a);
	// Now let's check if it is inverted
	if(invert)
	{
		cairo_paint(subcr);
	}
	// Draw the region	
	// Calculate new translations after expand the tile
	const double extx((-workdesc.get_tl()[0]+origin[0])*sx);
	const double exty((-workdesc.get_tl()[1]+origin[1])*sy);

	cairo_translate(subcr, extx , exty);
	cairo_scale(subcr, sx, sy);

	vector<Segment>::const_iterator iter=segments.begin();
	double t1x;
	double t1y;
	double t2x;
	double t2y;
	double p2x;
	double p2y;
	double p1x=iter->p1[0];
	double p1y=iter->p1[1];
	cairo_move_to(subcr, p1x, p1y);
	for(;iter!=segments.end();++iter)
	{
		t1x=iter->t1[0];
		t1y=iter->t1[1];
		t2x=iter->t2[0];
		t2y=iter->t2[1];
		p1x=iter->p1[0];
		p1y=iter->p1[1];
		p2x=iter->p2[0];
		p2y=iter->p2[1];
		cairo_curve_to(subcr, p1x+t1x/3, p1y+t1y/3, p2x-t2x/3, p2y-t2y/3, p2x, p2y);
	}
	
	cairo_close_path(subcr);
	if(invert)
		cairo_set_operator(subcr, CAIRO_OPERATOR_CLEAR);
	else
		cairo_set_operator(subcr, CAIRO_OPERATOR_OVER);
	switch(winding_style)
	{
		case WINDING_NON_ZERO:
			cairo_set_fill_rule(subcr, CAIRO_FILL_RULE_WINDING);
			break;
		default:
			cairo_set_fill_rule(subcr, CAIRO_FILL_RULE_EVEN_ODD);
			break;
	}
	if(!antialias)
		cairo_set_antialias(subcr, CAIRO_ANTIALIAS_NONE);

	cairo_fill(subcr);
	cairo_restore(subcr);
	if(feather && quality!=10)
	{
		etl::surface<float>	shapesurface;
		shapesurface.set_wh(workdesc.get_w(),workdesc.get_h());
		shapesurface.clear();
		
		CairoSurface cairosubimage(subimage);
		if(!cairosubimage.map_cairo_image())
		{
			synfig::info("map cairo image failed");
			return false;
		}
		// Extract the alpha values:
		int x, y;
		int wh(workdesc.get_h()), ww(workdesc.get_w());
		float div=1.0/((float)(CairoColor::ceil));
		for(y=0; y<wh; y++)
			for(x=0;x<ww;x++)
				shapesurface[y][x]=cairosubimage[y][x].get_a()*div;
		// Blue the alpha values
		Blur(feather,feather,blurtype,cb)(shapesurface,workdesc.get_br()-workdesc.get_tl(),shapesurface);
		// repaint the cairosubimage with the result
		Color ccolor(color);
		for(y=0; y<wh; y++)
			for(x=0;x<ww;x++)
			{
				float a=shapesurface[y][x];
				ccolor.set_a(a);
				ccolor.clamped();
				cairosubimage[y][x]=CairoColor(ccolor).premult_alpha();
			}
		
		cairosubimage.unmap_cairo_image();
	}
	
	// Put the (feathered) region on the surface
	if(!is_solid_color()) // we need to render the context before
		if(!context.accelerated_cairorender(surface,quality,renddesc,cb))
		{
			if(cb)
				cb->error(strprintf(__FILE__"%d: Accelerated Cairo Renderer Failure",__LINE__));
			cairo_destroy(cr);
			cairo_destroy(subcr);
			cairo_surface_destroy(subimage);
			return false;
		}
	cairo_save(cr);
	const double px(tl[0]-workdesc.get_tl()[0]);
	const double py(tl[1]-workdesc.get_tl()[1]);
	cairo_set_source_surface(cr, subimage, -px*sx, -py*sy);
	cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	cairo_restore(cr);
	cairo_surface_destroy(subimage);
	cairo_destroy(subcr);
	cairo_destroy(cr);
	
	return true;
}

/////////
