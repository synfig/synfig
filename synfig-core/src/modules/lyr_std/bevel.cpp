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

#include <synfig/blur.h>
#include <synfig/context.h>
#include <synfig/rendering/software/task/tasksw.h>
#include <synfig/rendering/software/function/blur.h>


#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Bevel);
SYNFIG_LAYER_SET_NAME(Layer_Bevel,"bevel");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Bevel,N_("Bevel"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Bevel,N_("Stylize"));
SYNFIG_LAYER_SET_VERSION(Layer_Bevel,"0.3");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

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
	Angle angle=param_angle.get(Angle());
	Real depth=param_depth.get(Real());
	
	offset[0]=Angle::cos(angle).get()*depth;
	offset[1]=Angle::sin(angle).get()*depth;

	offset45[0]=Angle::cos(angle-Angle::deg(45)).get()*depth*0.707106781;
	offset45[1]=Angle::sin(angle-Angle::deg(45)).get()*depth*0.707106781;
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
}

void
TaskBevel::set_coords_sub_tasks()
{
	if (!sub_task(0)) {
		trunc_to_zero();
		return;
	}
	if (!is_valid_coords()) {
		sub_task(0)->set_coords_zero();
		return;
	}

	const int w = target_rect.get_width();
	const int h = target_rect.get_height();
	const Real pw = get_units_per_pixel()[0];
	const Real ph = get_units_per_pixel()[1];

	const Vector size(softness,softness);

	//expand the working surface to accommodate the blur

	//the expanded size = 1/2 the size in each direction rounded up
	int	halfsizex = (int) (std::fabs(size[0]*.5/pw) + 3),
		halfsizey = (int) (std::fabs(size[1]*.5/(ph)) + 3);

	const int offset_u(round_to_int(offset[0]/pw));
	const int offset_v(round_to_int(offset[1]/(ph)));
	const int offset_w(w+std::abs(offset_u)*2);
	const int offset_h(h+std::abs(offset_v)*2);

	//expand by 1/2 size in each direction on either side
	switch(type)
	{
		case Blur::DISC:
		case Blur::BOX:
		case Blur::CROSS:
		case Blur::FASTGAUSSIAN:
		{
			halfsizex = std::max(1, halfsizex);
			halfsizey = std::max(1, halfsizey);
			break;
		}
		case Blur::GAUSSIAN:
		{
		#define GAUSSIAN_ADJUSTMENT		(0.05)

			Real pw2 = pw * pw;
			Real ph2 = ph * ph;

			halfsizex = (int)(size[0]*GAUSSIAN_ADJUSTMENT/std::fabs(pw2) + 0.5);
			halfsizey = (int)(size[1]*GAUSSIAN_ADJUSTMENT/std::fabs(ph2) + 0.5);

			halfsizex = (halfsizex + 1)/2;
			halfsizey = (halfsizey + 1)/2;
			break;
		}
	}

	Real delta_x = -std::abs(offset_u) - halfsizex;
	Real delta_y = -std::abs(offset_v) - halfsizey;
	Real new_w = offset_w + 2*halfsizex;
	Real new_h = offset_h + 2*halfsizey;

	Rect new_sub_source_rect(source_rect.minx + pw*delta_x, source_rect.miny + ph*delta_y, source_rect.minx + pw*delta_x + pw*new_w, source_rect.miny + ph*delta_y + ph*new_h);


	sub_task(0)->set_coords(new_sub_source_rect, VectorInt(new_w, new_h));
}

Rect
TaskBevel::calc_bounds() const
{
	if (!sub_task(0))
		return Rect::zero();

	Rect bounds = sub_task(0)->get_bounds();
	Vector size = Vector(softness,softness) * rendering::software::Blur::get_extra_size(rendering::Blur::Type(type));
	size[0] = fabs(size[0]) + 1.0;
	size[1] = fabs(size[1]) + 1.0;
	bounds.minx -= size[0];
	bounds.miny -= size[1];
	bounds.maxx += size[0];
	bounds.maxy += size[1];

	return bounds;
}

SYNFIG_EXPORT rendering::Task::Token TaskBevel::token(
	DescAbstract<TaskBevel>("Bevel") );

class TaskBevelSW : public TaskBevel, public synfig::rendering::TaskSW
{
public:
	typedef etl::handle<TaskBevel> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	bool run(RunParams&) const override {
		if (!is_valid())
			return true;
		if (!sub_task(0))
			return false;

		Rect common_source_rect;
		rect_set_intersect(common_source_rect, source_rect, sub_task(0)->source_rect);
		if (!common_source_rect.is_valid())
			return false;

		LockWrite la(this);
		if (!la)
			return false;

		synfig::surface<float> blurred;

		const Vector size(softness,softness);

		{
			synfig::surface<float> alpha_surface;
			get_alpha_surface(alpha_surface);
			//blur the image
			Blur(size, type)(alpha_surface, sub_task(0)->source_rect.get_size(), blurred);
		}

		const Vector ppu = get_pixels_per_unit();
		const Real pw = 1/ppu[0];
		const Real ph = 1/ppu[1];
		const int halfsizex = (int) (std::fabs(size[0]*.5/pw) + 3);
		const int halfsizey = (int) (std::fabs(size[1]*.5/ph) + 3);

		const int offset_u(round_to_int(offset[0]/pw)),offset_v(round_to_int(offset[1]/ph));

		const float u0(offset[0]/pw),   v0(offset[1]/ph);
		const float u1(offset45[0]/pw), v1(offset45[1]/ph);

		// convert vector-coordinates to raster-coordinates (i.e. source rect to target rect)
		Matrix transformation_matrix;
		transformation_matrix.m00 = ppu[0];
		transformation_matrix.m11 = ppu[1];
		transformation_matrix.m20 = target_rect.minx - source_rect.minx*ppu[0];
		transformation_matrix.m21 = target_rect.miny - source_rect.miny*ppu[1];

		const Point ftarget_min = transformation_matrix.get_transformed(source_rect.get_min());
		const Point ftarget_max = transformation_matrix.get_transformed(source_rect.get_max());
		const PointInt target_min = {round_to_int(ftarget_min[0]), round_to_int(ftarget_min[1])};
		const PointInt target_max = {round_to_int(ftarget_max[0]), round_to_int(ftarget_max[1])};
		int v = halfsizey+std::abs(offset_v) + target_min[1];
		for (int iy = target_min[1]; iy < target_max[1]; ++iy, ++v) {
			int u = halfsizex+std::abs(offset_u) + target_min[0];
			for (int ix = target_min[0]; ix < target_max[0]; ++ix, ++u) {

				Real alpha(0);
				Color shade;

				alpha += -blurred.linear_sample(u+u0, v+v0);
				alpha -= -blurred.linear_sample(u-u0, v-v0);
				alpha += -blurred.linear_sample(u+u1, v+v1)*0.5f;
				alpha += -blurred.linear_sample(u+v1, v-u1)*0.5f;
				alpha -= -blurred.linear_sample(u-u1, v-v1)*0.5f;
				alpha -= -blurred.linear_sample(u-v1, v+u1)*0.5f;

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

				if (shade.get_a())
					la->get_surface()[iy][ix] = shade;
				else
					la->get_surface()[iy][ix] = Color::alpha();
			}
		}
		return true;
	}

private:
	bool get_alpha_surface(synfig::surface<float>& output) const
	{
		LockRead lb(sub_tasks[0]);
		if (!lb)
			return false;

		const Surface& context = lb->get_surface();
		synfig::surface<float>& alpha_surface = output;
		alpha_surface.set_wh(context.get_w(), context.get_h());
		if (!use_luma) {
			for (int y = 0; y < context.get_h(); ++y) {
				for (int x = 0; x < context.get_w(); ++x) {
					alpha_surface[y][x] = context[y][x].get_a();
				}
			}
		} else {
			for (int y = 0; y < context.get_h(); ++y) {
				for (int x = 0; x < context.get_w(); ++x) {
					const auto& value = context[y][x];
					alpha_surface[y][x] = value.get_a() * value.get_y();
				}
			}
		}

		return true;
	}
};

SYNFIG_EXPORT rendering::Task::Token TaskBevelSW::token(
	DescReal<TaskBevelSW, TaskBevel>("BevelSW") );

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

rendering::Task::Handle
Layer_Bevel::build_composite_fork_task_vfunc(ContextParams /*context_params*/, rendering::Task::Handle sub_task) const
{
	TaskBevel::Handle task_bevel(new TaskBevel());
	task_bevel->softness = param_softness.get(Real());
	task_bevel->type = param_type.get(int());
	task_bevel->color1 = param_color1.get(Color());
	task_bevel->color2 = param_color2.get(Color());
	task_bevel->use_luma = param_use_luma.get(bool());
	task_bevel->solid = param_solid.get(bool());

	task_bevel->offset = offset;
	task_bevel->offset45 = offset45;

	task_bevel->sub_task(0) = sub_task->clone_recursive();

	return task_bevel;
}
