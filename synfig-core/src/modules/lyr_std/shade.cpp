/* === S Y N F I G ========================================================= */
/*!	\file shade.cpp
**	\brief Implementation of the "Shade" layer
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

#include <ETL/pen>
#include <ETL/misc>

#include "shade.h"

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

#include <synfig/rendering/primitive/transformationaffine.h>

#include <synfig/rendering/common/task/taskblur.h>
#include <synfig/rendering/common/task/tasktransformation.h>
#include <synfig/rendering/common/task/taskpixelprocessor.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/*#define TYPE_BOX			0
#define TYPE_FASTGUASSIAN	1
#define TYPE_CROSS			2
#define TYPE_GAUSSIAN		3
#define TYPE_DISC			4
*/

/* -- G L O B A L S --------------------------------------------------------- */

SYNFIG_LAYER_INIT(Layer_Shade);
SYNFIG_LAYER_SET_NAME(Layer_Shade,"shade");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Shade,N_("Shade"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Shade,N_("Stylize"));
SYNFIG_LAYER_SET_VERSION(Layer_Shade,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Layer_Shade,"$Id$");

/* -- F U N C T I O N S ----------------------------------------------------- */

static inline void clamp_size(Vector &v)
{
	if(v[0]<0.0)v[0]=0.0;
	if(v[1]<0.0)v[1]=0.0;
}

Layer_Shade::Layer_Shade():
	Layer_CompositeFork(0.75,Color::BLEND_BEHIND),
	param_size(ValueBase(Vector(0.1,0.1))),
	param_type(ValueBase(int(Blur::FASTGAUSSIAN))),
	param_color(ValueBase(Color::black())),
	param_origin(ValueBase(Vector(0.2,-0.2))),
	param_invert(ValueBase(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
Layer_Shade::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_size,
		{
			Vector size=param_size.get(Vector());
			clamp_size(size);
			param_size.set(size);
		}
		);
	IMPORT_VALUE(param_type);
	IMPORT_VALUE_PLUS(param_color,
		{
			Color color=param_color.get(Color());
			if (color.get_a() == 0)
			{
				if (converted_blend_)
				{
					set_blend_method(Color::BLEND_ALPHA_OVER);
					color.set_a(1);
					param_color.set(color);
				}
				else
					transparent_color_ = true;
			}
		}
		);
	IMPORT_VALUE(param_origin);
	IMPORT_VALUE(param_invert);

	if(param=="offset")
		return set_param("origin", value);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_Shade::get_param(const String &param)const
{
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_type);
	EXPORT_VALUE(param_color);
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_invert);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Color
Layer_Shade::get_color(Context context, const Point &pos)const
{
	Vector size=param_size.get(Vector());
	int type=param_type.get(int());
	Color color=param_color.get(Color());
	Vector origin=param_origin.get(Vector());
	bool invert=param_invert.get(bool());
	
	Point blurpos = Blur(size,type)(pos);

	if(get_amount()==0.0)
		return context.get_color(pos);

	Color shade(color);

	if(!invert)
		shade.set_a(context.get_color(blurpos-origin).get_a());
	else
		shade.set_a(1.0f-context.get_color(blurpos-origin).get_a());

	return Color::blend(shade,context.get_color(pos),get_amount(),get_blend_method());
}

bool
Layer_Shade::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	Vector size=param_size.get(Vector());
	int type=param_type.get(int());
	Color color=param_color.get(Color());
	Vector origin=param_origin.get(Vector());
	bool invert=param_invert.get(bool());

	int x,y;

	const int	w = renddesc.get_w(),
				h = renddesc.get_h();
	const Real	pw = renddesc.get_pw(),
				ph = renddesc.get_ph();

	RendDesc	workdesc(renddesc);
	Surface		worksurface;
	etl::surface<float> blurred;

	//expand the working surface to accommodate the blur

	//the expanded size = 1/2 the size in each direction rounded up
	int	halfsizex = (int) (abs(size[0]*.5/pw) + 3),
		halfsizey = (int) (abs(size[1]*.5/ph) + 3);

	int origin_u(-round_to_int(origin[0]/pw)),origin_v(-round_to_int(origin[1]/ph));

	int origin_w(w+abs(origin_u)),origin_h(h+abs(origin_v));

	workdesc.set_subwindow(
		origin_u<0?origin_u:0,
		origin_v<0?origin_v:0,
		(origin_u>0?origin_u:0)+w,
		(origin_v>0?origin_v:0)+h
	);

	if(quality >= 10)
	{
		halfsizex=1;
		halfsizey=1;
	}
	else if (quality == 9)
	{
		halfsizex/=4;
		halfsizey/=4;
	}

	//expand by 1/2 size in each direction on either side
	switch(type)
	{
		case Blur::DISC:
		case Blur::BOX:
		case Blur::CROSS:
		{
			workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),origin_w+2*max(1,halfsizex),origin_h+2*max(1,halfsizey));
			break;
		}
		case Blur::FASTGAUSSIAN:
		{
			if(quality < 4)
			{
				halfsizex*=2;
				halfsizey*=2;
			}
			workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),origin_w+2*max(1,halfsizex),origin_h+2*max(1,halfsizey));
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
			workdesc.set_subwindow( -halfsizex, -halfsizey, origin_w+2*halfsizex, origin_h+2*halfsizey );

			break;
		}
	}
#define SCALE_FACTOR	(64.0)
	if(/*quality>9 || */size[0]<=pw*SCALE_FACTOR)
	{
		SuperCallback stageone(cb,0,5000,10000);
		SuperCallback stagetwo(cb,5000,10000,10000);

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



		//render the background onto the expanded surface
		if(!context.accelerated_render(&worksurface,quality,workdesc,&stageone))
			return false;

		// Copy over the alpha
		blurred.set_wh(worksurface.get_w(),worksurface.get_h());
		for(int j=0;j<worksurface.get_h();j++)
			for(int i=0;i<worksurface.get_w();i++)
			{
				blurred[j][i]=worksurface[j][i].get_a();
			}

		//blur the image
		Blur(size,type,&stagetwo)(blurred,workdesc.get_br()-workdesc.get_tl(),blurred);

		//be sure the surface is of the correct size
		surface->set_wh(renddesc.get_w(),renddesc.get_h());

		int v = halfsizey-(origin_v<0?origin_v:0);
		for(y=0;y<renddesc.get_h();y++,v++)
		{
			int u = halfsizex-(origin_u<0?origin_u:0);
			for(x=0;x<renddesc.get_w();x++,u++)
			{
				Color a(color);

				if(!invert)
					a.set_a(blurred.linear_sample(origin_u+(float)u,origin_v+(float)v));
				else
					a.set_a(1.0f-blurred.linear_sample(origin_u+(float)u,origin_v+(float)v));

				if(a.get_a() || get_blend_method()==Color::BLEND_STRAIGHT)
				{
					(*surface)[y][x]=Color::blend(a,worksurface[v][u],get_amount(),get_blend_method());
				}
				else (*surface)[y][x] = worksurface[v][u];
			}
		}
	}
	else
	{

		SuperCallback stageone(cb,0,5000,10000);
		SuperCallback stagetwo(cb,5000,10000,10000);

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

		int fw(floor_to_int(abs(size[0]/(pw*SCALE_FACTOR)))+1);
		int fh(floor_to_int(abs(size[1]/(ph*SCALE_FACTOR)))+1);
		int tmpw(round_to_int((float)workdesc.get_w()/fw)),tmph(round_to_int((float)workdesc.get_h()/fh));

		workdesc.clear_flags();
		workdesc.set_wh(tmpw,tmph);
		//info("fw: %d, fh: %d",fw,fh);

		//render the blur fodder
		if(!context.accelerated_render(&worksurface,quality,workdesc,&stageone))
			return false;

		//render the background
		if(!context.accelerated_render(surface,quality,renddesc,&stageone))
			return false;

		// Copy over the alpha
		blurred.set_wh(worksurface.get_w(),worksurface.get_h());
		for(int j=0;j<worksurface.get_h();j++)
			for(int i=0;i<worksurface.get_w();i++)
				blurred[j][i]=worksurface[j][i].get_a();

		//blur the image
		Blur(size,type,&stagetwo)(blurred,workdesc.get_br()-workdesc.get_tl(),blurred);


		int v = halfsizey-(origin_v<0?origin_v:0);
		for(y=0;y<renddesc.get_h();y++,v++)
		{
			int u = halfsizex-(origin_u<0?origin_u:0);
			for(x=0;x<renddesc.get_w();x++,u++)
			{
				Color a(color);

				if(!invert)
					a.set_a(blurred.linear_sample(((float)origin_u+(float)u)/(float)fw,((float)origin_v+(float)v)/(float)fh));
				else
					a.set_a(1.0f-blurred.linear_sample(((float)origin_u+(float)u)/fw,((float)origin_v+(float)v)/(float)fh));

				if(a.get_a() || get_blend_method()==Color::BLEND_STRAIGHT)
				{
					(*surface)[y][x]=Color::blend(a,(*surface)[y][x],get_amount(),get_blend_method());
				}
			}
		}
	}


	if(cb && !cb->amount_complete(10000,10000))
	{
		//if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}

	return true;
}


///
bool
Layer_Shade::accelerated_cairorender(Context context,cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	Vector size=param_size.get(Vector());
	int type=param_type.get(int());
	Color color=param_color.get(Color());
	Vector origin=param_origin.get(Vector());
	bool invert=param_invert.get(bool());

	int x,y;
	SuperCallback stageone(cb,0,5000,10000);
	SuperCallback stagetwo(cb,5000,10000,10000);
	
	RendDesc	workdesc(renddesc);
	cairo_surface_t		*worksurface;
	etl::surface<float> blurred;
	
	// Untransform the render desc
	if(!cairo_renddesc_untransform(cr, workdesc))
		return false;
	
	const int w=workdesc.get_w(), h=workdesc.get_h();
	const double pw=(workdesc.get_br()[0]-workdesc.get_tl()[0])/w;
	const double ph=(workdesc.get_br()[1]-workdesc.get_tl()[1])/h;

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
	
	int origin_u(-round_to_int(origin[0]/pw)),origin_v(-round_to_int(origin[1]/ph));
	
	int origin_w(w+abs(origin_u)),origin_h(h+abs(origin_v));
	
	workdesc.set_subwindow(
						   origin_u<0?origin_u:0,
						   origin_v<0?origin_v:0,
						   (origin_u>0?origin_u:0)+w,
						   (origin_v>0?origin_v:0)+h
						   );
	
	if(quality >= 10)
	{
		halfsizex=1;
		halfsizey=1;
	}
	else if (quality == 9)
	{
		halfsizex/=4;
		halfsizey/=4;
	}
#define SCALE_FACTOR	(64.0)
	//expand by 1/2 size in each direction on either side
	switch(type)
	{
		case Blur::DISC:
		case Blur::BOX:
		case Blur::CROSS:
		{
			// If passed a certain size don't expand more
			halfsizex=halfsizex>SCALE_FACTOR?SCALE_FACTOR:halfsizex;
			halfsizey=halfsizey>SCALE_FACTOR?SCALE_FACTOR:halfsizey;
			workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),origin_w+2*max(1,halfsizex),origin_h+2*max(1,halfsizey));
			break;
		}
		case Blur::FASTGAUSSIAN:
		{
			if(quality < 4)
			{
				halfsizex*=2;
				halfsizey*=2;
			}
			// If passed a certain size don't expand more
			halfsizex=halfsizex>SCALE_FACTOR?SCALE_FACTOR:halfsizex;
			halfsizey=halfsizey>SCALE_FACTOR?SCALE_FACTOR:halfsizey;
			workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),origin_w+2*max(1,halfsizex),origin_h+2*max(1,halfsizey));
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
			// If passed a certain size don't expand more
			halfsizex=halfsizex>SCALE_FACTOR?SCALE_FACTOR:halfsizex;
			halfsizey=halfsizey>SCALE_FACTOR?SCALE_FACTOR:halfsizey;
			workdesc.set_subwindow( -halfsizex, -halfsizey, origin_w+2*halfsizex, origin_h+2*halfsizey );
			
			break;
		}
	}
	
	// New expanded workdesc values
	const int ww=workdesc.get_w();
	const int wh=workdesc.get_h();
	const double wtlx=workdesc.get_tl()[0];
	const double wtly=workdesc.get_tl()[1];
	
	// setup the worksurface
	worksurface=cairo_surface_create_similar(cairo_get_target(cr), CAIRO_CONTENT_COLOR_ALPHA, ww, wh);
	cairo_t* subcr=cairo_create(worksurface);
	cairo_scale(subcr, 1/pw, 1/ph);
	cairo_translate(subcr, -wtlx, -wtly);

	//render the background onto the expanded surface
	if(!context.accelerated_cairorender(subcr,quality,workdesc,&stageone))
		return false;
	// copy the background on the target surface if applies
	if(!is_solid_color())
	{
		//cairo_translate(subcr, origin[0], origin[1]);
		cairo_save(cr);
		cairo_translate(cr, wtlx, wtly);
		cairo_scale(cr, pw, ph);
		cairo_set_source_surface(cr, worksurface, 0, 0);
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		cairo_paint(cr);
		cairo_restore(cr);
	}
	
	// Extract the CairoSurface from the cairo_surface_t
	CairoSurface cairoworksurface(worksurface);
	if(!cairoworksurface.map_cairo_image())
	{
		info("map cairo image failed");
		return false;
	}
	// Extract the alpha
	blurred.set_wh(workdesc.get_w(),workdesc.get_h());
	float div=1.0/((float)(CairoColor::ceil));
	for(int j=0;j<workdesc.get_h();j++)
		for(int i=0;i<workdesc.get_w();i++)
			blurred[j][i]=cairoworksurface[j][i].get_a()*div;
	
	//blur the image
	Blur(size,type,&stagetwo)(blurred,workdesc.get_br()-workdesc.get_tl(),blurred);
	
	// repaint the cairosubimage with the result. Use the layer's amount here (is faster)
	Color ccolor(color);
	float am=get_amount();
	for(y=0; y<workdesc.get_h(); y++)
		for(x=0;x<workdesc.get_w();x++)
		{
			float a=blurred[y][x];
			if(invert)
				a=1.0-a;
			ccolor.set_a(a*am);
			ccolor=ccolor.clamped();
			cairoworksurface[y][x]=CairoColor(ccolor).premult_alpha();
		}
	
	cairoworksurface.unmap_cairo_image();
	
	// Now lets blend the result in the output surface
	cairo_save(cr);
	cairo_translate(cr, origin[0], origin[1]);
	cairo_translate(cr, wtlx, wtly);
	cairo_scale(cr, pw, ph);
	cairo_set_source_surface(cr, worksurface, 0, 0);
	cairo_paint_with_alpha_operator(cr, 1.0, get_blend_method());
	// TODO: add cairo_paint_opertor function when alpha=1.0 (it is quicker)
	cairo_restore(cr);
	cairo_surface_destroy(worksurface);
	
#undef GAUSSIAN_ADJUSTMENT
#undef SCALE_FACTOR
	
	if(cb && !cb->amount_complete(10000,10000))
	{
		//if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}
	
	return true;
}

///

Layer::Vocab
Layer_Shade::get_param_vocab(void)const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
	);
	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of Shade"))
		.set_is_distance()
		.set_origin("origin")
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

	ret.push_back(ParamDesc("invert")
		.set_local_name(_("Invert"))
	);

	return ret;
}

Rect
Layer_Shade::get_full_bounding_rect(Context context)const
{
	Vector size=param_size.get(Vector());
	Vector origin=param_origin.get(Vector());
	bool invert=param_invert.get(bool());

	if(is_disabled())
		return context.get_full_bounding_rect();

	if(invert)
		return Rect::full_plane();

	Rect under(context.get_full_bounding_rect());

	if(Color::is_onto(get_blend_method()))
		return under;

	Rect bounds((under+origin).expand_x(size[0]).expand_y(size[1]));

	if(is_solid_color())
		return bounds;

	return bounds|under;
}

rendering::Task::Handle
Layer_Shade::build_composite_fork_task_vfunc(ContextParams /* context_params */, rendering::Task::Handle sub_task)const
{
	Vector size = param_size.get(Vector());
	rendering::Blur::Type type = (rendering::Blur::Type)param_type.get(int());
	Color color = param_color.get(Color());
	Vector origin = param_origin.get(Vector());
	bool invert = param_invert.get(bool());

	rendering::TaskBlur::Handle task_blur(new rendering::TaskBlur());
	task_blur->blur.size = size;
	task_blur->blur.type = type;
	task_blur->sub_task() = sub_task->clone_recursive();

	ColorMatrix matrix;
	matrix *= ColorMatrix().set_replace_color(color);
	if (invert)
		matrix *= ColorMatrix().set_invert_alpha();

	rendering::TaskPixelColorMatrix::Handle task_colormatrix(new rendering::TaskPixelColorMatrix());
	task_colormatrix->matrix = matrix;
	task_colormatrix->sub_task() = task_blur;

	rendering::TaskTransformationAffine::Handle task_transformation(new rendering::TaskTransformationAffine());
	task_transformation->transformation->matrix.set_translate(origin);
	task_transformation->sub_task() = task_colormatrix;

	return task_transformation;
}
