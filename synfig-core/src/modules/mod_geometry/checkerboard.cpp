/* === S Y N F I G ========================================================= */
/*!	\file checkerboard.cpp
**	\brief Implementation of the "Checkerboard" layer
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <algorithm>

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>

#include <synfig/rendering/common/task/tasktransformation.h>
#include <synfig/rendering/common/task/taskblend.h>
#include <synfig/rendering/software/task/tasksw.h>

#include "checkerboard.h"

#include <synfig/localization.h>

#endif

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(CheckerBoard);
SYNFIG_LAYER_SET_NAME(CheckerBoard,"checker_board");
SYNFIG_LAYER_SET_LOCAL_NAME(CheckerBoard,N_("Checkerboard"));
SYNFIG_LAYER_SET_CATEGORY(CheckerBoard,N_("Geometry"));
SYNFIG_LAYER_SET_VERSION(CheckerBoard,"0.2");

/* === P R O C E D U R E S ================================================= */

namespace {

class TaskCheckerBoard: public rendering::Task, public rendering::TaskInterfaceTransformation
{
public:
	typedef etl::handle<TaskCheckerBoard> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	Color color;
	bool antialias;
	rendering::Holder<rendering::TransformationAffine> transformation;

	TaskCheckerBoard(): antialias(true) { }
	virtual rendering::Transformation::Handle get_transformation() const
		{ return transformation.handle(); }
};


class TaskCheckerBoardSW: public TaskCheckerBoard, public rendering::TaskSW,
	public rendering::TaskInterfaceBlendToTarget,
	public rendering::TaskInterfaceSplit
{
public:
	typedef etl::handle<TaskCheckerBoardSW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual void on_target_set_as_source() {
		Task::Handle &subtask = sub_task(0);
		if ( subtask
		  && subtask->target_surface == target_surface
		  && !Color::is_straight(blend_method) )
		{
			trunc_by_bounds();
			subtask->source_rect = source_rect;
			subtask->target_rect = target_rect;
		}
	}

	virtual Color::BlendMethodFlags get_supported_blend_methods() const
		{ return Color::BLEND_METHODS_ALL; }

	virtual bool run(RunParams&) const {
		if (!is_valid())
			return true;

		Vector ppu = get_pixels_per_unit();

		Matrix bounds_transfromation;
		bounds_transfromation.m00 = ppu[0];
		bounds_transfromation.m11 = ppu[1];
		bounds_transfromation.m20 = target_rect.minx - ppu[0]*source_rect.minx;
		bounds_transfromation.m21 = target_rect.miny - ppu[1]*source_rect.miny;

		Matrix matrix = bounds_transfromation * transformation->matrix;
		Matrix inv_matrix = matrix.get_inverted();

		int tw = target_rect.get_width();
		Vector dx = inv_matrix.axis_x();
		Vector dy = inv_matrix.axis_y() - dx*(Real)tw;
		Vector p = inv_matrix.get_transformed( Vector((Real)target_rect.minx, (Real)target_rect.miny) );

		LockWrite la(this);
		if (!la)
			return false;

		Surface::alpha_pen apen(la->get_surface().get_pen(target_rect.minx, target_rect.miny));
		ColorReal amount = blend ? this->amount : ColorReal(1.0);
		apen.set_blend_method(blend ? blend_method : Color::BLEND_COMPOSITE);
		Color c = color;
		if (antialias) {
			ColorReal kx(matrix.axis_x().mag()*0.5);
			ColorReal ky(matrix.axis_y().mag()*0.5);
			for(int iy = target_rect.miny; iy < target_rect.maxy; ++iy, p += dy, apen.inc_y(), apen.dec_x(tw))
				for(int ix = target_rect.minx; ix < target_rect.maxx; ++ix, p += dx, apen.inc_x()) {
					p[0] -= floor(p[0]);
					p[1] -= floor(p[1]);

					ColorReal px = p[0]*ColorReal(2);
					px -= floor(px);
					px = std::min(px, ColorReal(1) - px)*kx;

					ColorReal py = p[1]*ColorReal(2);
					py -= floor(py);
					py = std::min(py, ColorReal(1) - py)*ky;

					ColorReal a = std::min(px, py);
					if ((p[0] < 0.5) != (p[1] < 0.5)) a = -a;
					a = std::max(ColorReal(0), std::min(ColorReal(1), a + ColorReal(0.5)));

					c.set_a(color.get_a()*a);
					apen.put_value(c, amount);
				}
		} else {
			for(int iy = target_rect.miny; iy < target_rect.maxy; ++iy, p += dy, apen.inc_y(), apen.dec_x(tw))
				for(int ix = target_rect.minx; ix < target_rect.maxx; ++ix, p += dx, apen.inc_x()) {
					p[0] -= floor(p[0]);
					p[1] -= floor(p[1]);
					ColorReal a((p[0] < 0.5) == (p[1] < 0.5) ? 1.0 : 0.0);
					c.set_a(color.get_a()*a);
					apen.put_value(c, amount);
				}
		}

		return true;
	}
};

rendering::Task::Token TaskCheckerBoard::token(
	DescAbstract<TaskCheckerBoard>("CheckerBoard") );
rendering::Task::Token TaskCheckerBoardSW::token(
	DescReal<TaskCheckerBoardSW, TaskCheckerBoard>("CheckerBoardSW") );

} // namespace

/* === M E T H O D S ======================================================= */

CheckerBoard::CheckerBoard():
	Layer_Composite (1.0,Color::BLEND_COMPOSITE),
	param_color     (ValueBase(Color::black())),
	param_origin    (ValueBase(Point(0.125, 0.125))),
	param_size      (ValueBase(Point(0.25, 0.25))),
	param_antialias (ValueBase(true))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

inline bool
CheckerBoard::point_test(const synfig::Point& getpos)const
{
	Point origin=param_origin.get(Point());
	Point size=param_size.get(Point());
	
	int val=((int)((getpos[0]-origin[0])/size[0])+(int)((getpos[1]-origin[1])/size[1]));
	if(getpos[0]-origin[0] < 0.0)
		val++;
	if(getpos[1]-origin[1] < 0.0)
		val++;
	return val&1;
}

bool
CheckerBoard::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_color,
	  {
		  Color color(param_color.get(Color()));
		  if (color.get_a() == 0)
		  {
			  if(converted_blend_)
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
	IMPORT_VALUE(param_size);
	IMPORT_VALUE(param_antialias);
	
	if(param=="pos")
		return set_param("origin", value);

	for(int i=0;i<2;i++)
		if(param==etl::strprintf("pos[%d]",i) && value.get_type()==type_real)
		{
			Point p=param_origin.get(Point());
			p[i]=value.get(Real());
			param_origin.set(p);
			return true;
		}

	return Layer_Composite::set_param(param,value);
}

ValueBase
CheckerBoard::get_param(const String &param)const
{
	EXPORT_VALUE(param_color);
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_antialias);
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
CheckerBoard::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
		.set_description(_("Color of checkers"))
	);
	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Center of the checkers"))
		.set_is_distance()
	);
	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of checkers"))
		.set_origin("origin")
		.set_is_distance()
	);
	ret.push_back(ParamDesc("antialias")
		.set_local_name(_("Antialiasing"))
	);

	return ret;
}

synfig::Layer::Handle
CheckerBoard::hit_check(synfig::Context context, const synfig::Point &getpos)const
{
	if(get_amount()!=0.0 && point_test(getpos))
	{
		synfig::Layer::Handle tmp;
		if(get_blend_method()==Color::BLEND_BEHIND && (tmp=context.hit_check(getpos)))
			return tmp;
		if(Color::is_onto(get_blend_method()) && !(tmp=context.hit_check(getpos)))
			return 0;
		return const_cast<CheckerBoard*>(this);
	}
	else
		return context.hit_check(getpos);
}

Color
CheckerBoard::get_color(Context context, const Point &getpos)const
{
	Color color=param_color.get(Color());

	if(get_amount()!=0.0 && point_test(getpos))
	{
		if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
			return color;
		else
			return Color::blend(color,context.get_color(getpos),get_amount(),get_blend_method());
	}
	else
		return Color::blend(Color::alpha(),context.get_color(getpos),get_amount(),get_blend_method());
}

rendering::Task::Handle
CheckerBoard::build_composite_task_vfunc(ContextParams /*context_params*/)const
{
	Color color = param_color.get(Color());
	Point origin = param_origin.get(Point());
	Point size = param_size.get(Point());
	bool antialias = param_antialias.get(bool());

	origin[0] += size[0]; // make first cell empty (by history)
    size *= 2.0;          // task expects repeat period instead of cell size

	TaskCheckerBoard::Handle task(new TaskCheckerBoard());
	task->color = color;
	task->antialias = antialias;
	task->transformation->matrix = Matrix().set_translate(origin)
			                     * Matrix().set_scale(size);

	return task;
}

