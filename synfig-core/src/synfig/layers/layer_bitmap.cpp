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
using namespace etl;

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::Layer_Bitmap::Layer_Bitmap():
	Layer_Composite	(1.0,Color::BLEND_COMPOSITE),
	surface_modification_id (GUID::zero()),
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
		.set_is_distance()
	);

	ret.push_back(ParamDesc("br")
		.set_local_name(_("Bottom-Right"))
		.set_description(_("Lower right-hand Corner of image"))
		.set_is_distance()
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
			std::lock_guard<std::mutex> lock(mutex);

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
						int x(std::min(w-1,std::max(0,round_to_int(surface_pos[0]))));
						int y(std::min(h-1,std::max(0,round_to_int(surface_pos[1]))));
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
	task_gamma->gamma = get_canvas()->get_root()->rend_desc().get_gamma() / gamma;
	task_gamma->sub_task() = task;
	if (!task_gamma->is_transparent())
		task = task_gamma;

	return task;
}
