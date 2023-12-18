/* === S Y N F I G ========================================================= */
/*!	\file bevel.cpp
**	\brief Implementation of the "Bevel" layer
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

#include "bevel.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/misc.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include <synfig/rendering/primitive/transformationaffine.h>

#include <synfig/rendering/common/task/taskblur.h>
#include <synfig/rendering/common/task/tasktransformation.h>
#include <synfig/rendering/common/task/taskpixelprocessor.h>
#include <synfig/rendering/common/task/taskblend.h>

#include <cstring>

#endif

using namespace synfig;
using namespace modules;
using namespace lyr_std;

/*#define TYPE_BOX			0
#define TYPE_FASTGUASSIAN	1
#define TYPE_FASTGAUSSIAN	1
#define TYPE_CROSS			2
#define TYPE_GUASSIAN		3
#define TYPE_GAUSSIAN		3
#define TYPE_DISC			4
*/

/* -- G L O B A L S --------------------------------------------------------- */

SYNFIG_LAYER_INIT(Layer_Bevel);
SYNFIG_LAYER_SET_NAME(Layer_Bevel,"bevel");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Bevel,N_("Bevel"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Bevel,N_("Stylize"));
SYNFIG_LAYER_SET_VERSION(Layer_Bevel,"0.2");

/* -- F U N C T I O N S ----------------------------------------------------- */

inline void clamp(Vector &v)
{
	if(v[0]<0.0)v[0]=0.0;
	if(v[1]<0.0)v[1]=0.0;
}

Layer_Bevel::Layer_Bevel():
	Layer_CompositeFork(0.75,Color::BLEND_ONTO),
	param_type(ValueBase(int(Blur::FASTGAUSSIAN))),
	param_softness (ValueBase(Real(0.1))),
	param_color1(ValueBase(Color::white())),
	param_color2(ValueBase(Color::black())),
	param_depth(ValueBase(Real(0.2)))
{
	param_angle=ValueBase(Angle::deg(135));
	calc_offset();
	param_use_luma=ValueBase(false);
	param_solid=ValueBase(false);

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

void
Layer_Bevel::calc_offset()
{
	/*
    Angle angle=param_angle.get(Angle());
	Real depth=param_depth.get(Real());
	
	offset[0]=Angle::cos(angle).get()*depth;
	offset[1]=Angle::sin(angle).get()*depth;

	offset45[0]=Angle::cos(angle-Angle::deg(45)).get()*depth*0.707106781;
	offset45[1]=Angle::sin(angle-Angle::deg(45)).get()*depth*0.707106781;
	 */
}

bool
Layer_Bevel::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_softness,
		{
			Real softness=param_softness.get(Real());
			softness=softness>0?softness:0;
			param_softness.set(softness);
		}
		);
	IMPORT_VALUE(param_color1);
	IMPORT_VALUE(param_color2);
	IMPORT_VALUE_PLUS(param_depth,calc_offset());
	IMPORT_VALUE_PLUS(param_angle,calc_offset());
	IMPORT_VALUE(param_type);
	IMPORT_VALUE(param_use_luma);
	IMPORT_VALUE(param_solid);
	if (param == "fake_origin")
		return true;

	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_Bevel::get_param(const String &param)const
{
	EXPORT_VALUE(param_type);
	EXPORT_VALUE(param_softness);
	EXPORT_VALUE(param_color1);
	EXPORT_VALUE(param_color2);
	EXPORT_VALUE(param_depth);
	EXPORT_VALUE(param_angle);
	EXPORT_VALUE(param_use_luma);
	EXPORT_VALUE(param_solid);
	if (param == "fake_origin")
	{
		return Vector();
	}

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Color
Layer_Bevel::get_color(Context context, const Point &pos)const
{
	/*
    Real softness=param_softness.get(Real());
	int type=param_type.get(int());
	Color color1=param_color1.get(Color());
	Color color2=param_color2.get(Color());
	
	const Vector size(softness,softness);
	Point blurpos = Blur(size,type)(pos);

	if(get_amount()==0.0)
		return context.get_color(pos);

	Color shade;

	Real hi_alpha(1.0f-context.get_color(blurpos+offset).get_a());
	Real lo_alpha(1.0f-context.get_color(blurpos-offset).get_a());

	Real shade_alpha(hi_alpha-lo_alpha);
	if(shade_alpha>0)
		shade=color1,shade.set_a(shade_alpha);
	else
		shade=color2,shade.set_a(-shade_alpha);

	return Color::blend(shade,context.get_color(pos),get_amount(),get_blend_method());
	 */
    return Color();
}

/*
RendDesc
Layer_Bevel::get_sub_renddesc_vfunc(const RendDesc &renddesc) const
{
	Real softness=param_softness.get(Real());
	int type=param_type.get(int());

	const int	w = renddesc.get_w(),
				h = renddesc.get_h();
	const Real	pw = renddesc.get_pw(),
				ph = renddesc.get_ph();
	const Vector size(softness,softness);

	RendDesc workdesc(renddesc);

	//expand the working surface to accommodate the blur

	//the expanded size = 1/2 the size in each direction rounded up
	int	halfsizex = (int) (std::fabs(size[0]*.5/pw) + 3),
		halfsizey = (int) (std::fabs(size[1]*.5/ph) + 3);

	int offset_u(round_to_int(offset[0]/pw)),offset_v(round_to_int(offset[1]/ph));
	int offset_w(w+std::abs(offset_u)*2),offset_h(h+std::abs(offset_v)*2);

	workdesc.set_subwindow(
		-std::abs(offset_u),
		-std::abs(offset_v),
		w+std::abs(offset_u),
		h+std::abs(offset_v)
	);

	//expand by 1/2 size in each direction on either side
	switch(type)
	{
		case Blur::DISC:
		case Blur::BOX:
		case Blur::CROSS:
		{
			workdesc.set_subwindow(-std::max(1,halfsizex),-std::max(1,halfsizey),offset_w+2*std::max(1,halfsizex),offset_h+2*std::max(1,halfsizey));
			break;
		}
		case Blur::FASTGAUSSIAN:
		{
			workdesc.set_subwindow(-std::max(1,halfsizex),-std::max(1,halfsizey),offset_w+2*std::max(1,halfsizex),offset_h+2*std::max(1,halfsizey));
			break;
		}
		case Blur::GAUSSIAN:
		{
		#define GAUSSIAN_ADJUSTMENT		(0.05)
			Real	pw = (Real)workdesc.get_w()/(workdesc.get_br()[0]-workdesc.get_tl()[0]);
			Real 	ph = (Real)workdesc.get_h()/(workdesc.get_br()[1]-workdesc.get_tl()[1]);

			pw=pw*pw;
			ph=ph*ph;

			halfsizex = (int)(std::fabs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			halfsizey = (int)(std::fabs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);

			halfsizex = (halfsizex + 1)/2;
			halfsizey = (halfsizey + 1)/2;
			workdesc.set_subwindow( -halfsizex, -halfsizey, offset_w+2*halfsizex, offset_h+2*halfsizey );

			break;
		}
	}

	return workdesc;
}
 */

bool
Layer_Bevel::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	/*
    RENDER_TRANSFORMED_IF_NEED(__FILE__, __LINE__)

	Real softness=param_softness.get(Real());
	int type=param_type.get(int());
	Color color1=param_color1.get(Color());
	Color color2=param_color2.get(Color());
	bool use_luma=param_use_luma.get(bool());
	bool solid=param_solid.get(bool());

	int x,y;
	SuperCallback stageone(cb,0,5000,10000);
	SuperCallback stagetwo(cb,5000,10000,10000);

	RendDesc	workdesc = get_sub_renddesc(renddesc);
	Surface		worksurface;
	synfig::surface<float> blurred;

	const Real	pw = renddesc.get_pw(),
				ph = renddesc.get_ph();
	const Vector size(softness,softness);

	int	halfsizex = (int) (std::fabs(size[0]*.5/pw) + 3),
		halfsizey = (int) (std::fabs(size[1]*.5/ph) + 3);

	int offset_u(round_to_int(offset[0]/pw)),offset_v(round_to_int(offset[1]/ph));

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

    #define GAUSSIAN_ADJUSTMENT		(0.05)
	switch(type)
	{
		case Blur::GAUSSIAN:
		{
			Real pw = (Real)workdesc.get_w()/(workdesc.get_br()[0]-workdesc.get_tl()[0]);
			Real ph = (Real)workdesc.get_h()/(workdesc.get_br()[1]-workdesc.get_tl()[1]);

			pw=pw*pw;
			ph=ph*ph;

			halfsizex = (int)(std::fabs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			halfsizey = (int)(std::fabs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);

			halfsizex = (halfsizex + 1)/2;
			halfsizey = (halfsizey + 1)/2;

			break;
		}
	}

	//render the background onto the expanded surface
	if(!context.accelerated_render(&worksurface,quality,workdesc,&stageone))
		return false;

	// Copy over the alpha
	blurred.set_wh(worksurface.get_w(),worksurface.get_h());
	if(!use_luma)
	{
		for(int j=0;j<worksurface.get_h();j++)
			for(int i=0;i<worksurface.get_w();i++)
			{
				blurred[j][i]=worksurface[j][i].get_a();
			}
	}
	else
	{
		for(int j=0;j<worksurface.get_h();j++)
			for(int i=0;i<worksurface.get_w();i++)
			{
				blurred[j][i]=worksurface[j][i].get_a()*worksurface[j][i].get_y();
			}
	}

	//blur the image
	Blur(size,type,&stagetwo)(blurred,workdesc.get_br()-workdesc.get_tl(),blurred);

	//be sure the surface is of the correct size
	//surface->set_wh(renddesc.get_w(),renddesc.get_h());

	int v = halfsizey+std::abs(offset_v);
	for(y=0;y<renddesc.get_h();y++,v++)
	{
		int u = halfsizex+std::abs(offset_u);
		for(x=0;x<renddesc.get_w();x++,u++)
		{
			Real alpha(0);
			Color shade;

			{
				const float u2(offset[0]/pw),v2(offset[1]/ph);
				alpha+=1.0f-blurred.linear_sample(u2+u,v2+v);
			}
			{
				const float u2(-offset[0]/pw),v2(-offset[1]/ph);
				alpha-=1.0f-blurred.linear_sample(u2+u,v2+v);
			}
			{
				const float u2(offset45[0]/pw),v2(offset45[1]/ph);
				alpha+=1.0f-blurred.linear_sample(u2+u,v2+v)*0.5f;
			}
			{
				const float u2(offset45[1]/ph),v2(-offset45[0]/pw);
				alpha+=1.0f-blurred.linear_sample(u2+u,v2+v)*0.5f;
			}
			{
				const float u2(-offset45[0]/pw),v2(-offset45[1]/ph);
				alpha-=1.0f-blurred.linear_sample(u2+u,v2+v)*0.5f;
			}
			{
				const float u2(-offset45[1]/ph),v2(offset45[0]/pw);
				alpha-=1.0f-blurred.linear_sample(u2+u,v2+v)*0.5f;
			}

			if(solid)
			{
				alpha/=4.0f;
				alpha+=0.5f;
				shade=Color::blend(color1,color2,alpha,Color::BLEND_STRAIGHT);
			}
			else
			{
				alpha/=2;
				if(alpha>0)
					shade=color1,shade.set_a(shade.get_a()*alpha);
				else
					shade=color2,shade.set_a(shade.get_a()*-alpha);
			}



			if(shade.get_a())
			{
				(*surface)[y][x]=Color::blend(shade,worksurface[v][u],get_amount(),get_blend_method());
			}
			else (*surface)[y][x] = worksurface[v][u];
		}
	}

	if(cb && !cb->amount_complete(10000,10000))
	{
		//if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}
	 */

	return true;
}


////

Layer::Vocab
Layer_Bevel::get_param_vocab(void)const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("type")
		.set_local_name(_("Type"))
		.set_description(_("Type of blur to use"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(Blur::BOX,"box",_("Box Blur"))
		.add_enum_value(Blur::FASTGAUSSIAN,"fastgaussian",_("Fast Gaussian Blur"))
		.add_enum_value(Blur::CROSS,"cross",_("Cross-Hatch Blur"))
		.add_enum_value(Blur::GAUSSIAN,"gaussian",_("Gaussian Blur"))
		.add_enum_value(Blur::DISC,"disc",_("Disc Blur"))
	);

	ret.push_back(ParamDesc("color1")
		.set_local_name(_("Hi-Color"))
	);
	ret.push_back(ParamDesc("color2")
		.set_local_name(_("Lo-Color"))
	);
	ret.push_back(ParamDesc("angle")
		.set_local_name(_("Light Angle"))
		.set_origin("fake_origin")
	);
	ret.push_back(ParamDesc("depth")
		.set_is_distance()
		.set_local_name(_("Depth of Bevel"))
		.set_origin("fake_origin")
	);
	ret.push_back(ParamDesc("softness")
		.set_is_distance()
		.set_local_name(_("Softness"))
		.set_origin("fake_origin")
	);
	ret.push_back(ParamDesc("use_luma")
		.set_local_name(_("Use Luma"))
	);
	ret.push_back(ParamDesc("solid")
		.set_local_name(_("Solid"))
	);

	ret.push_back(ParamDesc("fake_origin")
		.hidden()
	);

	return ret;
}

Rect
Layer_Bevel::get_full_bounding_rect(Context context)const
{
	Real softness=param_softness.get(Real());
	Real depth=param_depth.get(Real());

	if(is_disabled())
		return context.get_full_bounding_rect();

	Rect under(context.get_full_bounding_rect());

	if(Color::is_onto(get_blend_method()))
		return under;

	Rect bounds(under.expand(softness));
	bounds.expand_x(std::fabs(depth));
	bounds.expand_y(std::fabs(depth));

	return bounds;
}

rendering::TaskBlend::Handle
Layer_Bevel::draw_sample(rendering::Task::Handle sub_task, Vector offset, Color color )const {

    /*
    rendering::TaskBlur::Handle task_blur1(new rendering::TaskBlur());
    task_blur1->blur.size = Vector(0.0, 0.0);
    task_blur1->blur.type = (rendering::Blur::Type)param_type.get(int());
    //task_blur1->sub_task() = task_blend23;
    task_blur1->sub_task() = sub_task->clone_recursive();
     */

    ColorMatrix matrix1;
    matrix1 *= ColorMatrix().set_replace_color(color);
    matrix1 *= ColorMatrix().set_invert_alpha();

    rendering::TaskPixelColorMatrix::Handle task_colormatrix1(new rendering::TaskPixelColorMatrix());
    task_colormatrix1->matrix = matrix1;
    //task_colormatrix1->sub_task() = sub_task->clone_recursive();
    task_colormatrix1->sub_task() = sub_task->clone_recursive();

    rendering::TaskTransformationAffine::Handle task_transformation1(new rendering::TaskTransformationAffine());
    task_transformation1->transformation->matrix.set_translate(-offset);
    task_transformation1->sub_task() = task_colormatrix1;

    rendering::TaskBlend::Handle task_alpha_over2(new rendering::TaskBlend());
    task_alpha_over2->amount = 1;
    task_alpha_over2->blend_method = Color::BLEND_COMPOSITE;
    task_alpha_over2->sub_task_a() = task_transformation1;
    task_alpha_over2->sub_task_b() = task_transformation1;

    return task_alpha_over2;

    ColorMatrix matrix2;
    matrix2 *= ColorMatrix().set_replace_color(color);
    matrix2 *= ColorMatrix().set_invert_alpha();
    rendering::TaskPixelColorMatrix::Handle task_colormatrix2(new rendering::TaskPixelColorMatrix());
    task_colormatrix2->matrix = matrix2;
    task_colormatrix2->sub_task() = sub_task->clone_recursive();

    rendering::TaskTransformationAffine::Handle task_transformation2(new rendering::TaskTransformationAffine());
    task_transformation2->transformation->matrix.set_translate(offset);
    task_transformation2->sub_task() = task_colormatrix2;

    rendering::TaskBlend::Handle task_alpha_over(new rendering::TaskBlend());
    task_alpha_over->amount = 1;
    task_alpha_over->blend_method = Color::BLEND_ALPHA_OVER;
    task_alpha_over->sub_task_a() = task_transformation2;
    task_alpha_over->sub_task_b() = task_transformation1;

    return task_alpha_over;

}

rendering::Task::Handle
Layer_Bevel::build_composite_fork_task_vfunc(ContextParams /* context_params */, rendering::Task::Handle sub_task)const
{

    Real softness=param_softness.get(Real());
    //int type=param_type.get(int());
    rendering::Blur::Type type = (rendering::Blur::Type)param_type.get(int());
    Color color1=param_color1.get(Color());
    Color color2=param_color2.get(Color());
    bool use_luma=param_use_luma.get(bool());
    bool solid=param_solid.get(bool());

    Vector size = Vector(softness,softness);

    //Color color1 = param_color1.get(Color());
    Angle angle=param_angle.get(Angle());
    Real depth=param_depth.get(Real());




    //offset45[0]=Angle::cos(angle-Angle::deg(45)).get()*depth*0.707106781;
    //offset45[1]=Angle::sin(angle-Angle::deg(45)).get()*depth*0.707106781;
    bool invert = true;

    if (!sub_task)
        return sub_task;

    Vector offset = Vector();
    offset[0]=Angle::cos(angle+Angle::deg(180)).get()*depth;
    offset[1]=Angle::sin(angle+Angle::deg(180)).get()*depth;
    rendering::TaskBlend::Handle task_sample1 = draw_sample(sub_task, offset, color1);

    return task_sample1;
    /*
    offset[0]=Angle::cos(angle+Angle::deg(180)-Angle::deg(45)).get()*depth*0.707106781;
    offset[1]=Angle::sin(angle+Angle::deg(180)-Angle::deg(45)).get()*depth*0.707106781;
    rendering::TaskBlend::Handle task_sample2 = draw_sample(sub_task, offset, color1);
    offset[1]=Angle::cos(angle+Angle::deg(180)-Angle::deg(45)).get()*depth*0.707106781;
    offset[0]=Angle::sin(angle+Angle::deg(180)-Angle::deg(45)).get()*depth*0.707106781;
    rendering::TaskBlend::Handle task_sample3 = draw_sample(sub_task, offset, color1);

    rendering::TaskBlend::Handle task_blend12(new rendering::TaskBlend());
    task_blend12->amount = 0.5;
    task_blend12->blend_method = Color::BLEND_COMPOSITE;
    task_blend12->sub_task_a() = task_sample1;
    task_blend12->sub_task_b() = task_sample2;

    rendering::TaskBlend::Handle task_blend23(new rendering::TaskBlend());
    task_blend23->amount = 0.5;
    task_blend23->blend_method = Color::BLEND_COMPOSITE;
    task_blend23->sub_task_a() = task_blend12;
    task_blend23->sub_task_b() = task_sample3;
     */



    rendering::TaskBlur::Handle task_blur1(new rendering::TaskBlur());
    task_blur1->blur.size = Vector(softness, softness);
    task_blur1->blur.type = type;
    //task_blur1->sub_task() = task_blend23;
    task_blur1->sub_task() = task_sample1;

    return task_blur1;
}
