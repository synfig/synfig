/* === S Y N F I G ========================================================= */
/*!	\file mod_filter/blur.cpp
**	\brief Implementation of the "Blur" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

#include "blur.h"

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>

#include <cstring>
#include <ETL/pen>

#endif

using namespace synfig;
using namespace etl;
using namespace std;

/*#define TYPE_BOX			0
#define TYPE_FASTGUASSIAN	1
#define TYPE_FASTGAUSSIAN	1
#define TYPE_CROSS			2
#define TYPE_GUASSIAN		3
#define TYPE_GAUSSIAN		3
#define TYPE_DISC			4
*/

/* -- G L O B A L S --------------------------------------------------------- */

SYNFIG_LAYER_INIT(Blur_Layer);
SYNFIG_LAYER_SET_NAME(Blur_Layer,"blur");
SYNFIG_LAYER_SET_LOCAL_NAME(Blur_Layer,N_("Blur"));
SYNFIG_LAYER_SET_CATEGORY(Blur_Layer,N_("Blurs"));
SYNFIG_LAYER_SET_VERSION(Blur_Layer,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Blur_Layer,"$Id$");

/* -- F U N C T I O N S ----------------------------------------------------- */

inline void clamp(synfig::Vector &v)
{
	if(v[0]<0.0)v[0]=0.0;
	if(v[1]<0.0)v[1]=0.0;
}

Blur_Layer::Blur_Layer():
	Layer_Composite(1.0,Color::BLEND_STRAIGHT),
	size(0.1,0.1),
	type(Blur::FASTGAUSSIAN)
{
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
	set_param_static("blend_method", true);
}

bool
Blur_Layer::set_param(const String &param, const ValueBase &value)
{
	IMPORT_PLUS(size,clamp(size));
	IMPORT(type);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Blur_Layer::get_param(const String &param)const
{
	EXPORT(size);
	EXPORT(type);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Color
Blur_Layer::get_color(Context context, const Point &pos)const
{
	Point blurpos = Blur(size,type)(pos);

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return context.get_color(blurpos);

	if(get_amount()==0.0)
		return context.get_color(pos);

	return Color::blend(context.get_color(blurpos),context.get_color(pos),get_amount(),get_blend_method());
}

bool
Blur_Layer::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	// don't do anything at quality 10
	if (quality == 10)
		return context.accelerated_render(surface,quality,renddesc,cb);

	// int x,y;
	SuperCallback stageone(cb,0,5000,10000);
	SuperCallback stagetwo(cb,5000,10000,10000);

	const int	w = renddesc.get_w(),
				h = renddesc.get_h();
	const Real	pw = renddesc.get_pw(),
				ph = renddesc.get_ph();

	RendDesc	workdesc(renddesc);
	Surface		worksurface,blurred;

	//callbacks depend on how long the blur takes
	if(size[0] || size[1])
	{
		if(type == Blur::DISC)
		{
			stageone = SuperCallback(cb,0,5000,10000);
			stagetwo = SuperCallback(cb,5000,10000,10000);
		}
		else
		{
			stageone = SuperCallback(cb,0,9000,10000);
			stagetwo = SuperCallback(cb,9000,10000,10000);
		}
	}
	else
	{
		stageone = SuperCallback(cb,0,9999,10000);
		stagetwo = SuperCallback(cb,9999,10000,10000);
	}

	//expand the working surface to accommodate the blur

	//the expanded size = 1/2 the size in each direction rounded up
	int	halfsizex = (int) (abs(size[0]*.5/pw) + 3),
		halfsizey = (int) (abs(size[1]*.5/ph) + 3);

	//expand by 1/2 size in each direction on either side
	switch(type)
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

			halfsizex = (int)(abs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			halfsizey = (int)(abs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);

			halfsizex = (halfsizex + 1)/2;
			halfsizey = (halfsizey + 1)/2;
			workdesc.set_subwindow( -halfsizex, -halfsizey, w+2*halfsizex, h+2*halfsizey );

			break;
		}
	}

	//render the background onto the expanded surface
	if(!context.accelerated_render(&worksurface,quality,workdesc,&stageone))
		return false;

	//blur the image
	Blur(size,type,&stagetwo)(worksurface,workdesc.get_br()-workdesc.get_tl(),blurred);

	//be sure the surface is of the correct size
	surface->set_wh(renddesc.get_w(),renddesc.get_h());

	{
		Surface::pen pen(surface->begin());
		worksurface.blit_to(pen,halfsizex,halfsizey,renddesc.get_w(),renddesc.get_h());
	}
	{
		Surface::alpha_pen pen(surface->begin());
		pen.set_alpha(get_amount());
		pen.set_blend_method(get_blend_method());
		blurred.blit_to(pen,halfsizex,halfsizey,renddesc.get_w(),renddesc.get_h());
	}
	if(cb && !cb->amount_complete(10000,10000))
	{
		//if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}

	return true;
}

/////
bool
Blur_Layer::accelerated_cairorender(Context context,cairo_surface_t *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	// don't do anything at quality 10
	if (quality == 10)
		return context.accelerated_cairorender(surface,quality,renddesc,cb);
	
	// int x,y;
	SuperCallback stageone(cb,0,5000,10000);
	SuperCallback stagetwo(cb,5000,10000,10000);
	
	const int	w = renddesc.get_w(), h = renddesc.get_h();
	const Real	pw = renddesc.get_pw(),	ph = renddesc.get_ph();
	
	const Point	tl(renddesc.get_tl());
	const Point br(renddesc.get_br());
	
	RendDesc	workdesc(renddesc);
	cairo_surface_t	*worksurface, *blurred;

	//callbacks depend on how long the blur takes
	if(size[0] || size[1])
	{
		if(type == Blur::DISC)
		{
			stageone = SuperCallback(cb,0,5000,10000);
			stagetwo = SuperCallback(cb,5000,10000,10000);
		}
		else
		{
			stageone = SuperCallback(cb,0,9000,10000);
			stagetwo = SuperCallback(cb,9000,10000,10000);
		}
	}
	else
	{
		stageone = SuperCallback(cb,0,9999,10000);
		stagetwo = SuperCallback(cb,9999,10000,10000);
	}
	
	//expand the working surface to accommodate the blur
	
	//the expanded size = 1/2 the size in each direction rounded up
	int	halfsizex = (int) (abs(size[0]*.5/pw) + 3),
	halfsizey = (int) (abs(size[1]*.5/ph) + 3);
	
	//expand by 1/2 size in each direction on either side
	switch(type)
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
			
			halfsizex = (int)(abs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			halfsizey = (int)(abs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);
			
			halfsizex = (halfsizex + 1)/2;
			halfsizey = (halfsizey + 1)/2;
			workdesc.set_subwindow( -halfsizex, -halfsizey, w+2*halfsizex, h+2*halfsizey );
#undef GAUSSIAN_ADJUSTMENT
			break;
		}
	}
	
	worksurface=cairo_surface_create_similar(surface, CAIRO_CONTENT_COLOR_ALPHA, workdesc.get_w(), workdesc.get_h());
	//render the background onto the expanded surface
	if(!context.accelerated_cairorender(worksurface,quality,workdesc,&stageone))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Cairo Renderer Failure",__LINE__));
		cairo_surface_destroy(worksurface);
		return false;
	}
	blurred=cairo_surface_create_similar(surface, CAIRO_CONTENT_COLOR_ALPHA, workdesc.get_w(), workdesc.get_h());
	
	//blur the image
	Blur(size,type,&stagetwo)(worksurface,workdesc.get_br()-workdesc.get_tl(),blurred);

	cairo_t *cr=cairo_create(surface);
	cairo_save(cr);
	cairo_set_source_surface(cr, worksurface, -halfsizex, -halfsizey);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);
	cairo_restore(cr);
	
	cairo_save(cr);
	cairo_set_source_surface(cr, blurred, -halfsizex, -halfsizey);
	cairo_paint_with_alpha_operator(cr, get_amount(), get_blend_method());
	cairo_restore(cr);

	if(cb && !cb->amount_complete(10000,10000))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}
	cairo_surface_destroy(blurred);
	cairo_surface_destroy(worksurface);
	cairo_destroy(cr);
	return true;
	
}

/////
/////
bool
Blur_Layer::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	// don't do anything at quality 10
	if (quality == 10)
		return context.accelerated_cairorender(cr,quality,renddesc,cb);
	
	SuperCallback stageone(cb,0,5000,10000);
	SuperCallback stagetwo(cb,5000,10000,10000);
	// Calculate the callbacks sizes
	// callbacks depend on how long the blur takes
	if(size[0] || size[1])
	{
		if(type == Blur::DISC)
		{
			stageone = SuperCallback(cb,0,5000,10000);
			stagetwo = SuperCallback(cb,5000,10000,10000);
		}
		else
		{
			stageone = SuperCallback(cb,0,9000,10000);
			stagetwo = SuperCallback(cb,9000,10000,10000);
		}
	}
	else
	{
		stageone = SuperCallback(cb,0,9999,10000);
		stagetwo = SuperCallback(cb,9999,10000,10000);
	}
	
	const Real	pw = renddesc.get_pw(),	ph = renddesc.get_ph();
	
	const Point	tl(renddesc.get_tl());
	const Point br(renddesc.get_br());
	
	double tl_x, tl_y, tr_x, tr_y, bl_x, bl_y, br_x, br_y;
	double mtlx, mtly, mbrx, mbry;
	double pminx, pminy, pmaxx, pmaxy;
	tl_x=tl[0];
	tl_y=tl[1];
	br_x=br[0];
	br_y=br[1];
	tr_x=br_x;
	tr_y=tl_y;
	bl_x=tl_x;
	bl_y=br_y;

	RendDesc	workdesc(renddesc);
	cairo_surface_t	*worksurface, *blurred;
	
	// In this block we are going to calculate the inversed transform of the
	// workdesc but not applying the transformation to convert the surface to
	// device space (See the cairo translate and scale on Target_Cairo::render and
	// Target_Cairo_Tile::render)
	
	// Extract the matrix from the current context
	cairo_matrix_t cr_matrix, cr_result;
	cairo_get_matrix(cr, &cr_matrix);
	
	// Now create three matrixes with the following values:
	// resulting matrix result=i_translate*i_scale
	// inverse translation i_translate = inverse translation from -renddesc_tl
	// inverse scale i_scale = inverse scale of 1/pw and 1/ph
	
	cairo_matrix_t i_scale, i_translate, result;
	cairo_matrix_init_translate(&i_translate, tl[0], tl[1]);
	cairo_matrix_init_scale(&i_scale, pw, ph);
	
	// Now multiply the two matrixes, the order is important!
	// first apply scale and then rotate, the inverse than done in Target_Cairo::render
	
	cairo_matrix_multiply(&result, &i_scale, &i_translate);
	
	// Now let's multiply the cr matrix retrieved and the result matrix
	
	cairo_matrix_multiply(&cr_result, &cr_matrix, &result);
	
	// Explanation:
	// Current cairo context matrix is this of this form:
	// [T][S][DRAW] where the [T][S] parts corresponds to convert the cairo operations
	// in DRAW part into the device space (usually the image surface of size w, h)
	// DRAW matrix is the result of the layer transformations stack (rotate, zoom, etc.)
	// But we want to transformm the render desc with the inverse of the DRAW part only,
	// not the inverse of the T and S part because we are transforming user coordinates
	// the renddesc and not pixels.
	// So we retrieve the cairo context matrix: [CR]=[T][S][DRAW] and remove the [T] and
	// [S] matrixes by applying its inverses: (the notation ' denotes inverse)
	// [S'][T'][CR]=[S'][T'][T][S][DRAW]=[S'][I][S][DRAW]=[I][DRAW]=[DRAW] as we wanted.
	// [M'][M]=[I] where I is the identity matrix.
	
	
	// Now let's invert the result matrix, that is calculate [DRAW']
	cairo_status_t status;
	status=cairo_matrix_invert(&cr_result);
	if(status) // doh! the matrix can't be inverted! I can't render the surface!
	{
		synfig::error("Can't invert current Cairo matrix!");
		return false;
	}
	
	// Now let's tranform the renddesc corners with the calculated matrix
	cairo_matrix_transform_point(&cr_result, &tl_x, &tl_y);
	cairo_matrix_transform_point(&cr_result, &tr_x, &tr_y);
	cairo_matrix_transform_point(&cr_result, &bl_x, &bl_y);
	cairo_matrix_transform_point(&cr_result, &br_x, &br_y);

	// Now let's figure out the rounding box of the transformed renddesc
	pminx=min(min(min(tl_x, tr_x), bl_x), br_x);
	pminy=min(min(min(tl_y, tr_y), bl_y), br_y);
	pmaxx=max(max(max(tl_x, tr_x), bl_x), br_x);
	pmaxy=max(max(max(tl_y, tr_y), bl_y), br_y);
	// let's assign the right values to the meaningfull variables :)
	mtlx=pminx;
	mtly=pmaxy;
	mbrx=pmaxx;
	mbry=pminy;
	
	// Now apply the new tl and br values to the workdesc
	// Before we will keep the pixel aspect and so when we modify the tl and br
	// the width and height will be modified as well.
	workdesc.set_flags(RendDesc::PX_ASPECT);
	// finally apply the new desc values!
	workdesc.set_tl_br(Point(mtlx, mtly), Point(mbrx, mbry));
	
	// Expand the working surface to accommodate the blur
	//the expanded size = 1/2 the size in each direction rounded up
	int w=workdesc.get_w(), h=workdesc.get_h();
	const double wpw=(workdesc.get_br()[0]-workdesc.get_tl()[0])/w;
	const double wph=(workdesc.get_br()[1]-workdesc.get_tl()[1])/h;
	int	halfsizex = (int) (abs(size[0]*.5/wpw) + 3),
	halfsizey = (int) (abs(size[1]*.5/wph) + 3);
	
	//expand by 1/2 size in each direction on either side
	switch(type)
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
			
			halfsizex = (int)(abs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			halfsizey = (int)(abs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);
			
			halfsizex = (halfsizex + 1)/2;
			halfsizey = (halfsizey + 1)/2;
			workdesc.set_subwindow( -halfsizex, -halfsizey, w+2*halfsizex, h+2*halfsizey );
#undef GAUSSIAN_ADJUSTMENT
			break;
		}
	}
	
	// New expanded workdesc values
	const int ww=workdesc.get_w();
	const int wh=workdesc.get_h();
	const double wtlx=workdesc.get_tl()[0];
	const double wtly=workdesc.get_tl()[1];
	
	// Create a surface to work on
	worksurface=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, ww, wh);
	// Create one new cairo_t* to render the layer's context on worksurface
	cairo_t* cr2=cairo_create(worksurface);
	// We need to scale up the surface to the pixel size to be able to make the blur
	cairo_scale(cr2, 1/wpw, 1/wph);
	cairo_translate(cr2, -wtlx, -wtly);
	// Lets render the background onto the expanded surface
	if(!context.accelerated_cairorender(cr2,quality,workdesc,&stageone))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Cairo Renderer Failure",__LINE__));
		cairo_surface_destroy(worksurface);
		return false;
	}
	// If the context was rendered, then blur it.
	blurred=cairo_surface_create_similar(worksurface, CAIRO_CONTENT_COLOR_ALPHA, ww, wh);
	//blur the image
	Blur(size,type,&stagetwo)(worksurface,workdesc.get_br()-workdesc.get_tl(),blurred);
	// We are done with cr2, destroy it
	cairo_destroy(cr2);
	// Let's composite the two surfaces: blurred over worksurface
	cr2=cairo_create(worksurface);
	cairo_set_source_surface(cr2, blurred, 0, 0);
	cairo_paint_with_alpha_operator(cr2, get_amount(), get_blend_method());
	cairo_destroy(cr2);
	// Now let's paint the blurred result on the cairo context.
	// But first we need to scale it down to the user coordinate space so when the
	// blurred surface is treated as the rest of layers it scales right to pixel size.
	// WE need to scale down the same that we scaled up to access the pixels.
	// This iwhat we would obtain:
	// [T][S][DRAW1][T1'][S1'][Blur][T1][S1][DRAW2]
	//                              ------CR2-----
	//                        -----image surface--
	// --------------CR---------------------------
	// Where:
	// [T][S] are the user to device transformation from renddesc
	// [DRAW1] are potential cairo operations before the blur layer
	// [T1][S1] are the user to device transformations from workdesc
	// [T1'][S1'] are the inverse of above
	// [DRAW2]is the cairo drawing operations below the blur
	cairo_translate(cr, wtlx, wtly);
	cairo_scale(cr, wpw, wph);
	// then set the source surface the worksurface
	cairo_set_source_surface(cr, worksurface, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);	
	if(cb && !cb->amount_complete(10000,10000))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}
	cairo_surface_destroy(blurred);
	cairo_surface_destroy(worksurface);
	return true;	
}




Layer::Vocab
Blur_Layer::get_param_vocab(void)const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of Blur"))
	);
	ret.push_back(ParamDesc("type")
		.set_local_name(_("Type"))
		.set_description(_("Type of blur to use"))
		.set_hint("enum")
		.add_enum_value(Blur::BOX,"box",_("Box Blur"))
		.add_enum_value(Blur::FASTGAUSSIAN,"fastgaussian",_("Fast Gaussian Blur"))
		.add_enum_value(Blur::CROSS,"cross",_("Cross-Hatch Blur"))
		.add_enum_value(Blur::GAUSSIAN,"gaussian",_("Gaussian Blur"))
		.add_enum_value(Blur::DISC,"disc",_("Disc Blur"))
	);

	return ret;
}

Rect
Blur_Layer::get_full_bounding_rect(Context context)const
{
	if(is_disabled() || Color::is_onto(get_blend_method()))
		return context.get_full_bounding_rect();

	Rect bounds(context.get_full_bounding_rect().expand_x(size[0]).expand_y(size[1]));

	return bounds;
}
