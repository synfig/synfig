/* === S Y N F I G ========================================================= */
/*!	\file noise.cpp
**	\brief Implementation of the "Noise Gradient" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2011-2013 Carlos López
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

#include "noise.h"

#include <synfig/localization.h>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/rendering/software/task/taskpaintpixelsw.h>
#include <synfig/value.h>
#include <ctime>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Noise);
SYNFIG_LAYER_SET_NAME(Noise,"noise");
SYNFIG_LAYER_SET_LOCAL_NAME(Noise,N_("Noise Gradient"));
SYNFIG_LAYER_SET_CATEGORY(Noise,N_("Gradients"));
SYNFIG_LAYER_SET_VERSION(Noise,"0.1");

/* === P R O C E D U R E S ================================================= */

class TaskNoise: public rendering::Task, public rendering::TaskInterfaceTransformation
{
public:
	typedef etl::handle<TaskNoise> Handle;
	SYNFIG_EXPORT static Token token;

	Token::Handle get_token() const override { return token.handle(); }

	Vector size;
	RandomNoise random;
	RandomNoise::SmoothType smooth;
	int detail;
	Real speed;
	bool turbulent;
	bool do_alpha;
	bool super_sample;
	CompiledGradient compiled_gradient;
	Time time_mark;

	rendering::Transformation::Handle get_transformation() const override {
		return transformation.handle();
	}

private:
	rendering::Holder<rendering::TransformationAffine> transformation;
};


class TaskNoiseSW: public TaskNoise, public rendering::TaskPaintPixelSW
{
public:
	typedef etl::handle<TaskNoise> Handle;
	SYNFIG_EXPORT static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	void pre_run(const Matrix3& /*matrix*/, const Matrix3& /*inverse_matrix*/) const override
	{
		pixel_size = (std::fabs(get_units_per_pixel()[0]) + std::fabs(get_units_per_pixel()[1])) * 0.5f;
	}

	Color get_color(const Vector& point) const override
	{
		float x(point[0] / size[0] * (1 << detail));
		float y(point[1] / size[1] * (1 << detail));
		float x2(0), y2(0);

		if (super_sample && pixel_size) {
			x2 = (point[0] + pixel_size) / size[0] * (1 << detail);
			y2 = (point[1] + pixel_size) / size[1] * (1 << detail);
		}

		const RandomNoise::SmoothType smooth_type((!speed && smooth == RandomNoise::SMOOTH_SPLINE)
													? RandomNoise::SMOOTH_FAST_SPLINE
													: smooth);

		const float ftime(speed * time_mark);

		float amount = 0.0f;
		float amount2 = 0.0f;
		float amount3 = 0.0f;
		float alpha = 0.0f;
		for (int i = 0; i < detail; i++) {
			amount = random(smooth_type, 0 + (detail - i) * 5, x, y, ftime) + amount * 0.5;
			amount = synfig::clamp(amount, -1.f, 1.f);

			if (super_sample && pixel_size) {
				amount2 = random(smooth_type, 0 + (detail - i) * 5, x2, y, ftime) + amount2 * 0.5;
				amount2 = synfig::clamp(amount2, -1.f, 1.f);

				amount3 = random(smooth_type, 0 + (detail - i) * 5, x, y2, ftime) + amount3 * 0.5;
				amount3 = synfig::clamp(amount3, -1.f, 1.f);

				if (turbulent) {
					amount2 = std::fabs(amount2);
					amount3 = std::fabs(amount3);
				}

				x2 *= 0.5f;
				y2 *= 0.5f;
			}

			if (do_alpha) {
				alpha = random(smooth_type, 3 + (detail - i) * 5, x, y, ftime) + alpha * 0.5;
				alpha = synfig::clamp(alpha, -1.f, 1.f);
			}

			if (turbulent) {
				amount = std::fabs(amount);
				alpha = std::fabs(alpha);
			}

			x *= 0.5f;
			y *= 0.5f;
		}

		if (!turbulent) {
			amount = amount / 2.0f + 0.5f;
			alpha = alpha / 2.0f + 0.5f;

			if (super_sample && pixel_size) {
				amount2 = amount2 / 2.0f + 0.5f;
				amount3 = amount3 / 2.0f + 0.5f;
			}
		}

		Color color;

		if (super_sample && pixel_size) {
			Real da = std::max(amount3, std::max(amount, amount2)) - std::min(amount3, std::min(amount, amount2));
			color = compiled_gradient.average(amount - da, amount + da);
		} else {
			color = compiled_gradient.color(amount);
		}

		if (do_alpha)
			color.set_a(color.get_a() * alpha);

		return color;
	}

	bool run(RunParams&) const override {
		return run_task();
	}

protected:
	mutable float pixel_size = 0.0f;
};

SYNFIG_EXPORT rendering::Task::Token TaskNoise::token(
	DescAbstract<TaskNoise>("Noise") );
SYNFIG_EXPORT rendering::Task::Token TaskNoiseSW::token(
	DescReal<TaskNoiseSW, TaskNoise>("NoiseSW") );


/* === M E T H O D S ======================================================= */

Noise::Noise():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	param_gradient(ValueBase(Gradient(Color::black(), Color::white()))),
	param_random(ValueBase(int(time(nullptr)))),
	param_size(ValueBase(Vector(1,1))),
	param_smooth(ValueBase(int(RandomNoise::SMOOTH_COSINE))),
	param_detail(ValueBase(int(4))),
	param_speed(ValueBase(Real(0))),
	param_turbulent(ValueBase(bool(false))),
	param_do_alpha(ValueBase(bool(false))),
	param_super_sample(ValueBase(bool(false)))
{
	//displacement=Vector(1,1);
	//do_displacement=false;
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}



void
Noise::compile()
	{ compiled_gradient.set(param_gradient.get(Gradient()) ); }

inline Color
Noise::color_func(const Point &point, float pixel_size,Context /*context*/)const
{
	Vector size=param_size.get(Vector());
	RandomNoise random;
	random.set_seed(param_random.get(int()));
	int smooth_=param_smooth.get(int());
	int detail=param_detail.get(int());
	Real speed=param_speed.get(Real());
	bool turbulent=param_turbulent.get(bool());
	bool do_alpha=param_do_alpha.get(bool());
	bool super_sample=param_super_sample.get(bool());
	

	Color ret(0,0,0,0);

	float x(point[0]/size[0]*(1<<detail));
	float y(point[1]/size[1]*(1<<detail));
	float x2(0),y2(0);

	if(super_sample&&pixel_size)
	{
		x2=(point[0]+pixel_size)/size[0]*(1<<detail);
		y2=(point[1]+pixel_size)/size[1]*(1<<detail);
	}

	Time time;
	time=speed*get_time_mark();
	int smooth((!speed && smooth_ == (int)RandomNoise::SMOOTH_SPLINE) ? (int)RandomNoise::SMOOTH_FAST_SPLINE : smooth_);

	float ftime(time);

	{
		float amount=0.0f;
		float amount2=0.0f;
		float amount3=0.0f;
		float alpha=0.0f;
		for (int i = 0; i < detail; i++) {
			amount=random(RandomNoise::SmoothType(smooth),0+(detail-i)*5,x,y,ftime)+amount*0.5;
			if (amount < -1) amount = -1;
			if (amount >  1) amount =  1;

			if(super_sample&&pixel_size)
			{
				amount2=random(RandomNoise::SmoothType(smooth),0+(detail-i)*5,x2,y,ftime)+amount2*0.5;
				if (amount2 < -1) amount2 = -1;
				if (amount2 >  1) amount2 =  1;

				amount3=random(RandomNoise::SmoothType(smooth),0+(detail-i)*5,x,y2,ftime)+amount3*0.5;
				if (amount3 < -1) amount3 = -1;
				if (amount3 >  1) amount3 =  1;

				if(turbulent)
				{
					amount2=std::fabs(amount2);
					amount3=std::fabs(amount3);
				}

				x2*=0.5f;
				y2*=0.5f;
			}

			if(do_alpha)
			{
				alpha=random(RandomNoise::SmoothType(smooth),3+(detail-i)*5,x,y,ftime)+alpha*0.5;
				if (alpha < -1) alpha = -1;
				if (alpha > 1) alpha = 1;
			}

			if(turbulent)
			{
				amount=std::fabs(amount);
				alpha=std::fabs(alpha);
			}

			x*=0.5f;
			y*=0.5f;
			//ftime*=0.5f;
		}

		if(!turbulent)
		{
			amount=amount/2.0f+0.5f;
			alpha=alpha/2.0f+0.5f;

			if(super_sample&&pixel_size)
			{
				amount2=amount2/2.0f+0.5f;
				amount3=amount3/2.0f+0.5f;
			}
		}

		if(super_sample && pixel_size) {
			Real da = std::max(amount3, std::max(amount,amount2)) - std::min(amount3, std::min(amount,amount2));
			ret = compiled_gradient.average(amount - da, amount + da);
		} else {
			ret = compiled_gradient.color(amount);
		}

		if(do_alpha)
			ret.set_a(ret.get_a()*(alpha));
	}
	return ret;
}


synfig::Layer::Handle
Noise::hit_check(synfig::Context context, const synfig::Point &point)const
{
	bool check_myself_first;
	auto layer = basic_hit_check(context, point, check_myself_first);

	if (!check_myself_first)
		return layer;

	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<Noise*>(this);
	if(color_func(point,0,context).get_a()>0.5)
		return const_cast<Noise*>(this);
	return synfig::Layer::Handle();
}

bool
Noise::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_gradient, compile());
	IMPORT_VALUE(param_size);
	IMPORT_VALUE(param_random);
	IMPORT_VALUE(param_detail);
	IMPORT_VALUE(param_smooth);
	IMPORT_VALUE(param_speed);
	IMPORT_VALUE(param_turbulent);
	IMPORT_VALUE(param_do_alpha);
	IMPORT_VALUE(param_super_sample);
	
	if(param=="seed")
		return set_param("random", value);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Noise::get_param(const String & param)const
{
	EXPORT_VALUE(param_gradient);
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_random);
	EXPORT_VALUE(param_detail);
	EXPORT_VALUE(param_smooth);
	EXPORT_VALUE(param_speed);
	EXPORT_VALUE(param_turbulent);
	EXPORT_VALUE(param_do_alpha);
	EXPORT_VALUE(param_super_sample);

	if(param=="seed")
		return get_param("random");

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
Noise::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("gradient")
		.set_local_name(_("Gradient"))
		.set_description(_("Gradient to apply"))
	);
	ret.push_back(ParamDesc("seed")
		.set_local_name(_("RandomNoise Seed"))
		.set_description(_("Change to modify the random seed of the noise"))
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of the noise"))
		.set_is_distance()
	);
	ret.push_back(ParamDesc("smooth")
		.set_local_name(_("Interpolation"))
		.set_description(_("What type of interpolation to use"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(RandomNoise::SMOOTH_DEFAULT,	"nearest",	_("Nearest Neighbor"))
		.add_enum_value(RandomNoise::SMOOTH_LINEAR,	"linear",	_("Linear"))
		.add_enum_value(RandomNoise::SMOOTH_COSINE,	"cosine",	_("Cosine"))
		.add_enum_value(RandomNoise::SMOOTH_SPLINE,	"spline",	_("Spline"))
		.add_enum_value(RandomNoise::SMOOTH_CUBIC,	"cubic",	_("Cubic"))
	);
	ret.push_back(ParamDesc("detail")
		.set_local_name(_("Detail"))
		.set_description(_("Increase to obtain fine details of the noise"))
	);
	ret.push_back(ParamDesc("speed")
		.set_local_name(_("Animation Speed"))
		.set_description(_("In cycles per second"))
	);
	ret.push_back(ParamDesc("turbulent")
		.set_local_name(_("Turbulent"))
		.set_description(_("When checked, produces turbulent noise"))
	);
	ret.push_back(ParamDesc("do_alpha")
		.set_local_name(_("Do Alpha"))
		.set_description(_("Uses transparency"))
	);
	ret.push_back(ParamDesc("super_sample")
		.set_local_name(_("Super Sampling"))
		.set_description(_("When checked, the gradient is supersampled"))
	);

	return ret;
}

Color
Noise::get_color(Context context, const Point &point)const
{
	const Color color(color_func(point,0,context));

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(point),get_amount(),get_blend_method());
}

rendering::Task::Handle
Noise::build_composite_task_vfunc(ContextParams /*context_params*/) const
{
	TaskNoise::Handle task(new TaskNoise());
	task->size = param_size.get(Vector());
	task->random.set_seed(param_random.get(int()));
	task->smooth = RandomNoise::SmoothType(param_smooth.get(int()));
	task->detail = param_detail.get(int());
	task->speed = param_speed.get(Real());
	task->turbulent = param_turbulent.get(bool());
	task->do_alpha = param_do_alpha.get(bool());
	task->super_sample = param_super_sample.get(bool());
	task->compiled_gradient = compiled_gradient;
	task->time_mark = get_time_mark();

	return task;
}

