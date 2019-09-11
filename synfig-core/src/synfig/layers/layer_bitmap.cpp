/* === S Y N F I G ========================================================= */
/*!	\file layer_bitmap.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <ETL/misc>

#include "layer_bitmap.h"

#include <synfig/time.h>
#include <synfig/string.h>
#include <synfig/vector.h>

#include <synfig/context.h>
#include <synfig/time.h>
#include <synfig/color.h>
#include <synfig/surface.h>
#include <synfig/renddesc.h>
#include <synfig/target.h>

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/paramdesc.h>

#include <synfig/rendering/software/surfacesw.h>
#include <synfig/rendering/software/surfaceswpacked.h>
#include <synfig/rendering/common/task/tasktransformation.h>
#include <synfig/rendering/common/task/taskpixelprocessor.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace std;
using namespace etl;

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::Layer_Bitmap::Layer_Bitmap():
	Layer_Composite	(1.0,Color::BLEND_COMPOSITE),
	method                  (SOFTWARE),
	surface_modification_id (0),
	param_tl                (Point(-0.5,0.5)),
	param_br                (Point(0.5,-0.5)),
	param_c                 (int(1)),
	param_gamma_adjust      (Real(1.0)),
	trimmed                 (false),
	left                    (0),
	top                     (0),
	width                   (0),
	height                  (0)
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

/*
synfig::Surface&
Layer_Bitmap::get_surface() const
{
	// TODO: not thread safe, return SurfaceResource instead
	if (!rendering_surface || !rendering_surface->is_exists())
		rendering_surface->create(128, 128);
	rendering::SurfaceResource::LockWrite<rendering::SurfaceSW> lock(rendering_surface);
	return lock->get_surface();
}
*/

bool
synfig::Layer_Bitmap::set_param(const String & param, const ValueBase & value)
{
	IMPORT_VALUE(param_tl);
	IMPORT_VALUE(param_br);
	IMPORT_VALUE(param_c);
	IMPORT_VALUE_PLUS(param_gamma_adjust,
		if(param=="gamma_adjust"&& value.get_type()==type_real)
		{
			param_gamma_adjust.set(Real(1.0/value.get(Real())));
			return true;
		}
		);

	return Layer_Composite::set_param(param,value);
}

ValueBase
synfig::Layer_Bitmap::get_param(const String & param)const
{
	EXPORT_VALUE(param_tl);
	EXPORT_VALUE(param_br);
	EXPORT_VALUE(param_c);
	if(param=="gamma_adjust")
	{
		ValueBase ret=param_gamma_adjust;
		ret.set(1.0/param_gamma_adjust.get(Real()));
		return ret;
	}

	if(param=="_width")
	{
		ValueBase ret1(type_integer);
		ret1=int(width);
		ValueBase ret2(type_integer);
		ret2=int(rendering_surface ? rendering_surface->get_width() : 0);
		if (trimmed) return ret1;
		return ret2;
	}
	if(param=="_height")
	{
		ValueBase ret1(type_integer);
		ret1=int(height);
		ValueBase ret2(type_integer);
		ret2=int(rendering_surface ? rendering_surface->get_height() : 0);
		if (trimmed) return ret1;
		return ret2;
	}

	return Layer_Composite::get_param(param);
}


Layer::Vocab
Layer_Bitmap::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("tl")
		.set_local_name(_("Top-Left"))
		.set_description(_("Upper left-hand Corner of image"))
	);

	ret.push_back(ParamDesc("br")
		.set_local_name(_("Bottom-Right"))
		.set_description(_("Lower right-hand Corner of image"))
	);

	ret.push_back(ParamDesc("c")
		.set_local_name(_("Interpolation"))
		.set_description(_("What type of interpolation to use"))
		.set_hint("enum")
		.add_enum_value(0,"nearest",_("Nearest Neighbor"))
		.add_enum_value(1,"linear",_("Linear"))
		.add_enum_value(2,"cosine",_("Cosine"))
		.add_enum_value(3,"cubic",_("Cubic"))
		.set_static(true)
	);

	ret.push_back(ParamDesc("gamma_adjust")
		.set_local_name(_("Gamma Adjustment"))
	);

	return ret;
}

synfig::Layer::Handle
Layer_Bitmap::hit_check(synfig::Context context, const synfig::Point &pos)const
{
	Point tl(param_tl.get(Point()));
	Point br(param_br.get(Point()));
	Point surface_pos;
	surface_pos=pos-tl;

	surface_pos[0]/=br[0]-tl[0];
	if(surface_pos[0]<=1.0 && surface_pos[0]>=0.0)
	{
		surface_pos[1]/=br[1]-tl[1];
		if(surface_pos[1]<=1.0 && surface_pos[1]>=0.0)
		{
			return const_cast<Layer_Bitmap*>(this);
		}
	}

	return context.hit_check(pos);
}

void
synfig::Layer_Bitmap::set_render_method(Context context, RenderMethod x)
{
	set_method(x);
	context.set_render_method(x);
}

inline
const Color&
synfig::Layer_Bitmap::filter(Color& x)const
{
	Real gamma_adjust(param_gamma_adjust.get(Real()));
	if(gamma_adjust!=1.0)
	{
		x.set_r(powf((float)x.get_r(),gamma_adjust));
		x.set_g(powf((float)x.get_g(),gamma_adjust));
		x.set_b(powf((float)x.get_b(),gamma_adjust));
		x.set_a(powf((float)x.get_a(),gamma_adjust));
	}
	return x;
}

inline
const CairoColor&
synfig::Layer_Bitmap::filter(CairoColor& x)const
{
	Real gamma_adjust(param_gamma_adjust.get(Real()));
	if(gamma_adjust!=1.0)
	{
		x.set_r(powf((float)(x.get_r()/CairoColor::range),gamma_adjust)*CairoColor::range);
		x.set_g(powf((float)(x.get_g()/CairoColor::range),gamma_adjust)*CairoColor::range);
		x.set_b(powf((float)(x.get_b()/CairoColor::range),gamma_adjust)*CairoColor::range);
		x.set_a(powf((float)(x.get_a()/CairoColor::range),gamma_adjust)*CairoColor::range);
	}
	return x;
}


Color
synfig::Layer_Bitmap::get_color(Context context, const Point &pos)const
{
	Point tl(param_tl.get(Point()));
	Point br(param_br.get(Point()));
	int c(param_c.get(int()));

	Point surface_pos;

	if(!get_amount() || !rendering_surface || !rendering_surface->is_exists())
		return context.get_color(pos);

	surface_pos=pos-tl;
	int w = rendering_surface->get_width();
	int h = rendering_surface->get_height();

	surface_pos[0]/=br[0]-tl[0];
	if(surface_pos[0]<=1.0 && surface_pos[0]>=0.0)
	{
		surface_pos[1]/=br[1]-tl[1];
		if(surface_pos[1]<=1.0 && surface_pos[1]>=0.0)
		{
			Mutex::Lock lock(mutex);

			if (trimmed)
			{
				surface_pos[0]*=width;
				surface_pos[1]*=height;

				if (surface_pos[0] > left+w || surface_pos[0] < left || surface_pos[1] > top+h || surface_pos[1] < top)
					return context.get_color(pos);

				surface_pos[0] -= left;
				surface_pos[1] -= top;
			}
			else
			{
				surface_pos[0]*=w;
				surface_pos[1]*=h;
			}

			Color ret(Color::alpha());

			rendering::SurfaceResource::LockReadBase lsurf(rendering_surface);
			if (lsurf.convert<rendering::SurfaceSWPacked>(false))
			{
				typedef rendering::software::PackedSurface PackedSurface;
				typedef PackedSurface::Sampler Sampler;

				assert(lsurf.get_handle().type_is<rendering::SurfaceSWPacked>());
				reader.open( lsurf.cast<rendering::SurfaceSWPacked>()->get_surface() );
				switch(c)
				{
				case 6:	// Undefined
				case 5:	// Undefined
				case 4:	// Undefined
				case 3:	// Cubic
					ret = ColorPrep::uncook_static(Sampler::cubic_sample(&reader, surface_pos[0], surface_pos[1]));
					break;
				case 2:	// Cosine
					ret = ColorPrep::uncook_static(Sampler::cosine_sample(&reader, surface_pos[0], surface_pos[1]));
					break;
				case 1:	// Linear
					ret = ColorPrep::uncook_static(Sampler::linear_sample(&reader, surface_pos[0], surface_pos[1]));
					break;
				case 0:	// Nearest Neighbor
				default:
					ret = ColorPrep::uncook_static(Sampler::nearest_sample(&reader, surface_pos[0], surface_pos[1]));
					break;
				break;
				}
			}
			else
			if (lsurf.convert<rendering::SurfaceSW>())
			{
				assert(lsurf.get_handle().type_is<rendering::SurfaceSW>());
				const Surface &surface = lsurf.cast<rendering::SurfaceSW>()->get_surface();
				switch(c)
				{
				case 6:	// Undefined
				case 5:	// Undefined
				case 4:	// Undefined
				case 3:	// Cubic
					ret=surface.cubic_sample(surface_pos[0],surface_pos[1]);
					break;

				case 2:	// Cosine
					ret=surface.cosine_sample(surface_pos[0],surface_pos[1]);
					break;
				case 1:	// Linear
					ret=surface.linear_sample(surface_pos[0],surface_pos[1]);
					break;
				case 0:	// Nearest Neighbor
				default:
					{
						int x(min(w-1,max(0,round_to_int(surface_pos[0]))));
						int y(min(h-1,max(0,round_to_int(surface_pos[1]))));
						ret= surface[y][x];
					}
				break;
				}
			}

			ret=filter(ret);

			if(get_amount()==1 && get_blend_method()==Color::BLEND_STRAIGHT)
				return ret;
			else
				return Color::blend(ret,context.get_color(pos),get_amount(),get_blend_method());
		}
	}

	return context.get_color(pos);
}


CairoColor
synfig::Layer_Bitmap::get_cairocolor(Context context, const Point &pos)const
{
	// no cairo implementation
	return context.get_cairocolor(pos);
}


bool
Layer_Bitmap::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)  const
{
	RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	Mutex::Lock lock(mutex);

	Point tl(param_tl.get(Point()));
	Point br(param_br.get(Point()));
	int c(param_c.get(int()));
	Real gamma_adjust(param_gamma_adjust.get(Real()));

	rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lsurf(rendering_surface);
	if (!lsurf)
		return false;
	Surface &layer_surface = lsurf.cast<rendering::SurfaceSW>()->get_surface(); // const cast

	int interp=c;
	if(quality>=10)
		interp=0;
	else if(quality>=5 && interp>1)
		interp=1;

	// We can only handle NN and Linear at the moment
	//if(interp>1)
	//	return Layer_Composite::accelerated_render(context,surface,quality,renddesc,cb);

	//if we don't actually have a valid surface just skip us
	if(!layer_surface.is_valid())
	{
		// Render what is behind us
		return context.accelerated_render(surface,quality,renddesc,cb);
	}

	SuperCallback subcb(cb,1,10000,10001+renddesc.get_h());

	if(	get_amount()==1 &&
		get_blend_method()==Color::BLEND_STRAIGHT &&
		!trimmed &&
		renddesc.get_tl()==tl &&
		renddesc.get_br()==br)
	{
		// Check for the trivial case
		if(layer_surface.get_w()==renddesc.get_w() && layer_surface.get_h()==renddesc.get_h() && gamma_adjust==1.0f)
		{
			if(cb && !cb->amount_complete(0,100)) return false;
			*surface=layer_surface;
			if(cb && !cb->amount_complete(100,100)) return false;
			return true;
		}
		surface->set_wh(renddesc.get_w(),renddesc.get_h());
	}
	else
	{
		// Render what is behind us
		if(!context.accelerated_render(surface,quality,renddesc,&subcb))
			return false;
	}

	if(cb && !cb->amount_complete(10000,10001+renddesc.get_h())) return false;

	Point	obr	= renddesc.get_br(),
			otl = renddesc.get_tl();

	//Vector::value_type pw=renddesc.get_w()/(renddesc.get_br()[0]-renddesc.get_tl()[0]);
	//Vector::value_type ph=renddesc.get_h()/(renddesc.get_br()[1]-renddesc.get_tl()[1]);

	//A = representation of input (just tl,br) 	//just a scaling right now
	//B = representation of output (just tl,br)	//just a scaling right now
	//sa = scaling for input (0,1) -> (0,w/h)
	//sb = scaling for output (0,1) -> (0,w/h)

	float	outwf = obr[0] - otl[0];
	float	outhf = obr[1] - otl[1];

	int		inw = layer_surface.get_w();
	int		inh = layer_surface.get_h();

	int		outw = renddesc.get_w();
	int		outh = renddesc.get_h();

	float	inwf, inhf;
	Point	itl, ibr;

	if (trimmed)
	{
		inwf = (br[0] - tl[0])*layer_surface.get_w()/width;
		inhf = (br[1] - tl[1])*layer_surface.get_h()/height;
		itl = Point(tl[0] + (br[0]-tl[0])*left/width,
					tl[1] + (br[1]-tl[1])*top/height);
		ibr = Point(tl[0] + (br[0]-tl[0])*(left+inw)/width,
					tl[1] + (br[1]-tl[1])*(top+inh)/height);
	}
	else
	{
		inwf = br[0] - tl[0];
		inhf = br[1] - tl[1];
		itl = tl;
		ibr = br;
	}

	//need to get the input coords in output space, so we can clip

	//get the desired corners of the bitmap (in increasing order) in integers
	//floating point corners
	float x1f = (itl[0] - otl[0])*outw/outwf;
	float x2f = (ibr[0] - otl[0])*outw/outwf;
	float y1f = (itl[1] - otl[1])*outh/outhf;
	float y2f = (ibr[1] - otl[1])*outh/outhf;

	if(x1f > x2f) swap(x1f,x2f);
	if(y1f > y2f) swap(y1f,y2f);

	int x_start = max(0,(int)floor(x1f));	//probably floor
	int x_end 	= min(outw,(int)ceil(x2f));	//probably ceil
	int y_start = max(0,(int)floor(y1f));	//probably floor
	int y_end	= min(outh,(int)ceil(y2f));	//probably ceil

	//need to get the x,y,dx,dy values from output space to input, so we can do fast interpolation

	//get the starting position in input space... for interpolating

	// in int -> out float:
	// Sb(B^-1)A(Sa^-1) x
	float inx_start = (((x_start/*+0.5f*/)*outwf/outw + otl[0]) - itl[0])*inw/inwf; //may want to bias this (center of pixel)???
	float iny_start = (((y_start/*+0.5f*/)*outhf/outh + otl[1]) - itl[1])*inh/inhf; //may want to bias this (center of pixel)???

	//calculate the delta values in input space for one pixel movement in output space
	//same matrix but with a vector instead of a point...
	float indx = outwf*(inw)/((outw)*inwf);		//translations died
	float indy = outhf*(inh)/((outh)*inhf);		//translations died

	//perhaps use a DDA algorithm... if faster...
	//   will still want pixel fractions to be floating point since colors are

	//synfig::info("xstart:%d ystart:%d xend:%d yend:%d",x_start,y_start,x_end,y_end);

	//start drawing at the start of the bitmap (either origin or corner of input...)
	//and get other info
	Surface::alpha_pen pen(surface->get_pen(x_start,y_start));
	pen.set_alpha(get_amount());
	pen.set_blend_method(get_blend_method());

	//perform normal interpolation
	if(interp==0)
	{
		//synfig::info("Decided to do nearest neighbor");
		float iny, inx;
		int x,y;

		//Point sample - truncate
		iny = iny_start;//+0.5f;
		for(y = y_start; y < y_end; y++, pen.inc_y(), iny += indy)
		{
			inx = inx_start;//+0.5f;
			int yclamp = min(inh-1, max(0, round_to_int(iny)));
			for(x = x_start; x < x_end; x++, pen.inc_x(), inx += indx)
			{
				int xclamp = min(inw-1, max(0, round_to_int(inx)));
				Color c = filter(layer_surface[yclamp][xclamp]);
				pen.put_value(c); //must get rid of the clip
			}
			pen.dec_x(x_end-x_start);
		}
	}
	else
	if(interp==1)
	{
		//bilinear filtering

		//float 	xmf,xpf,ymf,ypf;
		//int		xm,xp,ym,yp;
		float 	inx,iny;
		int		x,y;

		//can probably buffer for x values...

		//loop and based on inx,iny sample input image
		iny = iny_start;
		for(y = y_start; y < y_end; y++, pen.inc_y(), iny += indy)
		{
			inx = inx_start;
			for(x = x_start; x < x_end; x++, pen.inc_x(), inx += indx)
			{
				Color col(layer_surface.linear_sample(inx,iny));
				pen.put_value(filter(col));
			}
			pen.dec_x(x_end-x_start);

		}
	}
	else
	if(interp==2)
	{
		//cosine filtering

		//float 	xmf,xpf,ymf,ypf;
		//int		xm,xp,ym,yp;
		float 	inx,iny;
		int		x,y;

		//can probably buffer for x values...

		//loop and based on inx,iny sample input image
		iny = iny_start;
		for(y = y_start; y < y_end; y++, pen.inc_y(), iny += indy)
		{
			inx = inx_start;
			for(x = x_start; x < x_end; x++, pen.inc_x(), inx += indx)
			{
				Color col(layer_surface.cosine_sample(inx,iny));
				pen.put_value(filter(col));
			}
			pen.dec_x(x_end-x_start);

		}
	}
	else
	{
		//cubic filtering

		//float 	xmf,xpf,ymf,ypf;
		//int		xm,xp,ym,yp;
		float 	inx,iny;
		int		x,y;

		//can probably buffer for x values...

		//loop and based on inx,iny sample input image
		iny = iny_start;
		for(y = y_start; y < y_end; y++, pen.inc_y(), iny += indy)
		{
			inx = inx_start;
			for(x = x_start; x < x_end; x++, pen.inc_x(), inx += indx)
			{
				Color col(layer_surface.cubic_sample(inx,iny));
				pen.put_value(filter(col));
			}
			pen.dec_x(x_end-x_start);

		}
	}

	return true;
}

/////
/////

bool
Layer_Bitmap::accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)  const
{
	// no cairo implementation
	return context.accelerated_cairorender(cr,quality,renddesc,cb);
}

/////


Rect
Layer_Bitmap::get_bounding_rect()const
{
	Point tl(param_tl.get(Point()));
	Point br(param_br.get(Point()));

	return Rect(tl,br);
}


rendering::Task::Handle
Layer_Bitmap::build_composite_task_vfunc(ContextParams /* context_params */) const
{
	if ( !rendering_surface
	  || !rendering_surface->is_exists() )
		return rendering::Task::Handle();

	ColorReal gamma = (Color::value_type)param_gamma_adjust.get(Real());
	Point tl(param_tl.get(Point()));
	Point br(param_br.get(Point()));
	Matrix m;
	m.m00 = (br[0] - tl[0]); m.m20 = tl[0];
	m.m11 = (br[1] - tl[1]); m.m21 = tl[1];

	rendering::Task::Handle task;

	rendering::TaskSurface::Handle task_surface(new rendering::TaskSurface());
	task_surface->target_surface = rendering_surface;
	task_surface->target_rect = RectInt(VectorInt(), rendering_surface->get_size());
	task_surface->source_rect = Rect(0.0, 0.0, 1.0, 1.0);
	task = task_surface;

	rendering::TaskTransformationAffine::Handle task_transform = new rendering::TaskTransformationAffine();
	task_transform->interpolation = (Color::Interpolation)param_c.get(int());
	task_transform->transformation->matrix = m;
	task_transform->sub_task() = task;
	task = task_transform;

	rendering::TaskPixelGamma::Handle task_gamma = new rendering::TaskPixelGamma();
	if (!approximate_equal_lp(gamma, 1.f)) {
		task_gamma->gamma[0] = gamma;
		task_gamma->gamma[1] = gamma;
		task_gamma->gamma[2] = gamma;
		task_gamma->gamma[3] = gamma;
		task_gamma->sub_task() = task;
		task = task_gamma;
	}

	return task;
}
