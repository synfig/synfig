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

#include <cstring>

#include "blur.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>
#include <synfig/cairo_renddesc.h>

#include <synfig/rendering/common/task/taskblend.h>
#include <synfig/rendering/common/task/taskblur.h>
#include <synfig/rendering/software/function/blur.h>

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
SYNFIG_LAYER_SET_VERSION(Blur_Layer,"0.3");
SYNFIG_LAYER_SET_CVS_ID(Blur_Layer,"$Id$");

/* -- F U N C T I O N S ----------------------------------------------------- */

inline void clamp(synfig::Vector &v)
{
	if(v[0]<0.0)v[0]=0.0;
	if(v[1]<0.0)v[1]=0.0;
}

Blur_Layer::Blur_Layer():
	Layer_CompositeFork(1.0,Color::BLEND_STRAIGHT),
	param_size(ValueBase(Point(0.1,0.1))),
	param_type(ValueBase(int(Blur::FASTGAUSSIAN)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
Blur_Layer::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_size,
		{
			synfig::Point size=param_size.get(Point());
			clamp(size);
			param_size.set(size);
		});
		
	IMPORT_VALUE(param_type);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Blur_Layer::get_param(const String &param)const
{
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_type);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Color
Blur_Layer::get_color(Context context, const Point &pos)const
{
	synfig::Point size=param_size.get(Point());
	int type=param_type.get(int());
  	size *= rendering::software::Blur::get_size_amplifier((rendering::Blur::Type)type)
  	      * ::Blur::get_size_amplifier(type);
	
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
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	synfig::Point size=param_size.get(Point());
	int type=param_type.get(int());
  	size *= rendering::software::Blur::get_size_amplifier((rendering::Blur::Type)type)
  	      * ::Blur::get_size_amplifier(type);

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

	// check size
	for(int i = surface->get_w()-2; i; --i)
	{
		Real a0 = (*surface)[0][i].get_a();
		Real a1 = (*surface)[0][i+1].get_a();
		if (type == 2) a0*=2.0, a1*=2.0;
		if (a0 > 0.25) {
			Real d = (a1 - a0);
			Real p = fabs(d) > 1e-10 ? (a0 - 0.25)/d : 0.0;
			p += i - 200;
			Real pw = (Real)workdesc.get_w()/(workdesc.get_br()[0]-workdesc.get_tl()[0]);
			info("legacy type %d size %f actual size %f", type, size[0]*pw, p);
			break;
		}
	}

	return true;
}

/////
bool
Blur_Layer::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	synfig::Point size=param_size.get(Point());
	int type=param_type.get(int());
  	size *= rendering::software::Blur::get_size_amplifier((rendering::Blur::Type)type)
  	      * ::Blur::get_size_amplifier(type);

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

	RendDesc	workdesc(renddesc);
	cairo_surface_t	*worksurface, *blurred;
	
	if(!cairo_renddesc_untransform(cr, workdesc))
		return false;
	
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
	cairo_save(cr);
	cairo_translate(cr, wtlx, wtly);
	cairo_scale(cr, wpw, wph);
	// then set the source surface the worksurface
	cairo_set_source_surface(cr, worksurface, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);
	cairo_restore(cr);
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
	synfig::Point size=param_size.get(Point());
	int type=param_type.get(int());
  	size *= rendering::software::Blur::get_size_amplifier((rendering::Blur::Type)type)
  	      * ::Blur::get_size_amplifier(type);

	if(is_disabled() || Color::is_onto(get_blend_method()))
		return context.get_full_bounding_rect();

	Rect bounds(context.get_full_bounding_rect().expand_x(size[0]).expand_y(size[1]));

	return bounds;
}

rendering::Task::Handle
Blur_Layer::build_composite_fork_task_vfunc(ContextParams /* context_params */, rendering::Task::Handle sub_task)const
{
	Vector size = param_size.get(Point());
	rendering::Blur::Type type = (rendering::Blur::Type)param_type.get(int());

	rendering::TaskBlur::Handle task_blur(new rendering::TaskBlur());
	task_blur->blur.size = size;
	task_blur->blur.type = type;
	task_blur->sub_task() = sub_task ? sub_task->clone_recursive() : rendering::Task::Handle();

	return task_blur;
}
