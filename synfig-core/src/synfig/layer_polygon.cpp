/* === S Y N F I G ========================================================= */
/*!	\file layer_polygon.cpp
**	\brief Implementation of the "Polygon" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "layer_polygon.h"
#include "string.h"
#include "time.h"
#include "context.h"
#include "paramdesc.h"
#include "renddesc.h"
#include "surface.h"
#include "value.h"
#include "valuenode.h"
#include "cairo_renddesc.h"
//#include "ETL/bezier"
#include <vector>

#include <deque>
using std::deque;

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Polygon);
SYNFIG_LAYER_SET_NAME(Layer_Polygon,"polygon");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Polygon,N_("Polygon"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Polygon,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(Layer_Polygon,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_Polygon,"$Id$");

/* === C L A S S E S ======================================================= */

/* === M E T H O D S ======================================================= */

Layer_Polygon::Layer_Polygon():
	Layer_Shape(1.0,Color::BLEND_COMPOSITE),
	vector_list(0)
{
	vector_list.push_back(Point(0,0.5));
	vector_list.push_back(Point(-0.333333,0));
	vector_list.push_back(Point(0.333333,0));
	sync();
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
}

Layer_Polygon::~Layer_Polygon()
{
}

void
Layer_Polygon::sync()
{
/*
	int i,pointcount=vector_list.size();

	if(pointcount<3)
		return;

	//Layer_Shape::clear();
	//clear();

	// Build edge table
	move_to(vector_list[0][0],vector_list[0][1]);

	for(i = 1;i < pointcount; i++)
	{
		if(isnan(vector_list[i][0]) || isnan(vector_list[i][1]))
			break;
		line_to(vector_list[i][0],vector_list[i][1]);
	}
	close();
	//endpath();
*/
}

void
Layer_Polygon::add_polygon(const std::vector<Point> &point_list)
{
	int i,pointcount=point_list.size();

	if(pointcount<3)
		return;

	//Layer_Shape::clear();
	//clear();

	// Build edge table
	move_to(point_list[0][0],point_list[0][1]);

	for(i = 1;i < pointcount; i++)
	{
		if(isnan(point_list[i][0]) || isnan(point_list[i][1]))
			break;
		line_to(point_list[i][0],point_list[i][1]);
	}
	close();
	//endpath();
}

void
Layer_Polygon::upload_polygon(const std::vector<Point> &point_list)
{
	vector_list.clear();
	int i,pointcount=point_list.size();
	for(i = 0;i < pointcount; i++)
	{
		vector_list.push_back(point_list[i]);
	}
	
}

void
Layer_Polygon::clear()
{
	Layer_Shape::clear();
	vector_list.clear();
}

bool
Layer_Polygon::set_param(const String & param, const ValueBase &value)
{
	if(	param=="vector_list" && value.same_type_as(vector_list))
	{
		vector_list=value;
		Layer_Shape::clear();
		add_polygon(value);
		sync();
		return true;
	}

	return Layer_Shape::set_param(param,value);
}

ValueBase
Layer_Polygon::get_param(const String &param)const
{
	EXPORT(vector_list);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Shape::get_param(param);
}

Layer::Vocab
Layer_Polygon::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Shape::get_param_vocab());

	ret.push_back(ParamDesc("vector_list")
		.set_local_name(_("Vertices List"))
		.set_description(_("Define the corners of the polygon"))
		.set_origin("origin")
	);

	return ret;
}


/////////
bool
Layer_Polygon::accelerated_cairorender(Context context,cairo_surface_t *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
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

	cairo_t* cr=cairo_create(surface);
	// Let's render the polygon in other surface
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
	// Draw the polygon
	// Calculate new translations after expand the tile
	const double extx((-workdesc.get_tl()[0]+origin[0])*sx);
	const double exty((-workdesc.get_tl()[1]+origin[1])*sy);
	
	cairo_translate(subcr, extx , exty);
	cairo_scale(subcr, sx, sy);
	int i,pointcount=vector_list.size();
	for(i=0;i<pointcount; i++)
	{
		cairo_line_to(subcr, vector_list[i][0], vector_list[i][1]);
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
	if(!feather_cairo_surface(subimage, workdesc, quality))
	   {
		cairo_surface_destroy(subimage);
		cairo_destroy(subcr);
		cairo_destroy(cr);
		return false;
	   }
	
	// Put the (feathered) polygon on the surface
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
	const double px(tl[0]-workdesc.get_tl()[0]);
	const double py(tl[1]-workdesc.get_tl()[1]);
	cairo_save(cr);
	cairo_set_source_surface(cr, subimage, -px*sx, -py*sy);
	cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	cairo_restore(cr);
	cairo_surface_destroy(subimage);
	cairo_destroy(subcr);
	cairo_destroy(cr);

	return true;
}

/////////

/////////
bool
Layer_Polygon::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	// Grab the rgba values
	const float r(color.get_r());
	const float g(color.get_g());
	const float b(color.get_b());
	const float a(color.get_a());

	// First render the context
	if(!is_solid_color())
		if(!context.accelerated_cairorender(cr,quality,renddesc,cb))
		{
			if(cb)
				cb->error(strprintf(__FILE__"%d: Accelerated Cairo Renderer Failure",__LINE__));
			return false;
		}
	// If the polygon has feather then render as image
	if((feather && quality != 10) || invert)
	{
		cairo_surface_t* subimage;
		RendDesc	workdesc(renddesc);

		// Untransform the render desc
		if(!cairo_renddesc_untransform(cr, workdesc))
			return false;
			
		int halfsizex(0), halfsizey(0);

		int w=workdesc.get_w(), h=workdesc.get_h();
		const double wpw=(workdesc.get_br()[0]-workdesc.get_tl()[0])/w;
		const double wph=(workdesc.get_br()[1]-workdesc.get_tl()[1])/h;
		//the expanded size = 1/2 the size in each direction rounded up
		halfsizex = (int) (abs(feather*.5/wpw) + 3),
		halfsizey = (int) (abs(feather*.5/wph) + 3);

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
		// New expanded workdesc values
		const int ww=workdesc.get_w();
		const int wh=workdesc.get_h();
		const double wtlx=workdesc.get_tl()[0];
		const double wtly=workdesc.get_tl()[1];
		subimage=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, ww, wh);
		cairo_t* subcr=cairo_create(subimage);
		cairo_scale(subcr, 1/wpw, 1/wph);
		cairo_translate(subcr, -wtlx, -wtly);
		cairo_translate(subcr, origin[0], origin[1]);

		cairo_set_source_rgba(subcr, r, g, b, a);
		if(invert)
		{
			cairo_paint(subcr);
		}
		
		int i,pointcount=vector_list.size();
		for(i=0;i<pointcount; i++)
		{
			cairo_line_to(subcr, vector_list[i][0], vector_list[i][1]);
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
		if(quality!=10)
		{
			if(!feather_cairo_surface(subimage, workdesc, quality))
			{
				cairo_surface_destroy(subimage);
				cairo_destroy(subcr);
				return false;
			}
		}
		// we are done with subcr
		cairo_destroy(subcr);
		
		// lets put the feathered polygon on the context
		cairo_save(cr);
		
		cairo_translate(cr, wtlx, wtly);
		cairo_scale(cr, wpw, wph);
		cairo_set_source_surface(cr, subimage, 0, 0);
		cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());

		cairo_restore(cr);
		cairo_surface_destroy(subimage);
		return true;
	}
	// Not inverted and not feather
	cairo_save(cr);
	cairo_translate(cr, origin[0], origin[1]);
	cairo_set_source_rgba(cr, r, g, b, a);
	int i,pointcount=vector_list.size();
	for(i=0;i<pointcount; i++)
	{
		cairo_line_to(cr, vector_list[i][0], vector_list[i][1]);
	}
	cairo_close_path(cr);
	switch(winding_style)
	{
		case WINDING_NON_ZERO:
			cairo_set_fill_rule(cr, CAIRO_FILL_RULE_WINDING);
			break;
		default:
			cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
			break;
	}
	if(!antialias)
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_clip(cr);
	cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	cairo_restore(cr);
	
	return true;
}

/////////
