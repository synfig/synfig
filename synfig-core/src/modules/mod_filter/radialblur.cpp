/* === S Y N F I G ========================================================= */
/*!	\file radialblur.cpp
**	\brief Implementation of the "Radial Blur" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/localization.h>
#include <synfig/general.h>

#include "radialblur.h"
#include <synfig/string.h>
#include <synfig/context.h>
#include <synfig/misc.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/transform.h>
#include <synfig/rendering/software/task/tasksw.h>

#endif

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(RadialBlur);
SYNFIG_LAYER_SET_NAME(RadialBlur,"radial_blur");
SYNFIG_LAYER_SET_LOCAL_NAME(RadialBlur,N_("Radial Blur"));
SYNFIG_LAYER_SET_CATEGORY(RadialBlur,N_("Blurs"));
SYNFIG_LAYER_SET_VERSION(RadialBlur,"0.2");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class TaskRadialBlur
	: public rendering::Task
{
public:
	typedef etl::handle<TaskRadialBlur> Handle;
	static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	Vector origin;
	Real size;
	bool fade_out;

	void
	set_coords_sub_tasks() override
	{
		const Point tl(source_rect.get_min()), br(source_rect.get_max());
		const int w(target_rect.get_width()), h(target_rect.get_height());
		const Real pw(get_units_per_pixel()[0]),ph(get_units_per_pixel()[1]);

		Rect rect(tl, br);
		Point pos;
		// find how far towards the origin of the blur we are going to
		// wander for each of the 4 corners of our tile, expanding the
		// render description for each of them if necessary
		int x, y;
		for(y=0,pos[1]=tl[1];y<h;y+=(h-1),pos[1]+=ph*(h-1))
			for(x=0,pos[0]=tl[0];x<w;x+=(w-1),pos[0]+=pw*(w-1))
				rect.expand((pos-origin)*(1.0f-size) + origin);

		Vector stl = rect.get_min();
		Vector sbr = rect.get_max();
		if (br[0] < tl[0]) std::swap(stl[0], sbr[0]);
		if (br[1] < tl[1]) std::swap(stl[1], sbr[1]);

		// round out to the nearest pixel
		Point tmp_surface_tl = Point(tl[0] - pw*(int((tl[0]-stl[0])/pw+1-1e-6)),
									 tl[1] - ph*(int((tl[1]-stl[1])/ph+1-1e-6)));
		Point tmp_surface_br = Point(br[0] + pw*(int((sbr[0]-br[0])/pw+2-1e-6)),
									 br[1] + ph*(int((sbr[1]-br[1])/ph+2-1e-6)));

		// round to nearest integer width and height (should be very
		// nearly whole numbers already, but don't want to round 5.99999
		// down to 5)
		int tmp_surface_width = int((tmp_surface_br[0]-tmp_surface_tl[0])/pw + 0.5);
		int tmp_surface_height = int((tmp_surface_br[1]-tmp_surface_tl[1])/ph + 0.5);
		sub_tasks[0]->set_coords(Rect(tmp_surface_tl, tmp_surface_br), VectorInt(tmp_surface_width, tmp_surface_height));
	}

	/**
	 * Compute how large is the source_rect given a sub_task source_rect
	 */
	Rect
	compute_expanded_rect(const Rect& original_rect, const Vector& upp) const
	{
		const Point tl(original_rect.get_min()), br(original_rect.get_max());

		Rect rect(tl, br);
		{
			Point pos = tl;
			// find how far towards the origin of the blur we are going to
			// wander for each of the 4 corners of our tile, expanding the
			// render description for each of them if necessary
			for (int j = 0; j < 2; ++j, pos[1] = br[1] - upp[1]) {
				pos[0]=tl[0];
				for (int i = 0; i < 2; ++i, pos[0] = br[0] - upp[0])
					rect.expand((pos-origin)/(1.0f-size) + origin);
			}
		}

		Vector stl = rect.get_min();
		Vector sbr = rect.get_max();
		if (br[0] < tl[0]) std::swap(stl[0], sbr[0]);
		if (br[1] < tl[1]) std::swap(stl[1], sbr[1]);

		return Rect(stl, sbr);
	}
};

class TaskRadialBlurSW
	: public TaskRadialBlur, public rendering::TaskSW
{
public:
	typedef etl::handle<TaskRadialBlurSW> Handle;
	static Token token;
	Token::Handle get_token() const override { return token.handle(); }

	bool run(Task::RunParams& /*params*/) const override
	{
		if (!sub_task(0))
			return true;
		if (!is_valid())
			return true;

		Surface::value_prep_type cooker;

		Rect expanded_subtask_source_rect = compute_expanded_rect(sub_task(0)->source_rect, sub_task(0)->get_units_per_pixel());
		Rect common_source_rect;
		rect_set_intersect(common_source_rect, source_rect, expanded_subtask_source_rect);
		if (!common_source_rect.is_valid())
			return true;

		const Point tl(common_source_rect.get_min()), br(common_source_rect.get_max());

		RectInt ra;
		{
			const Vector ppu = get_pixels_per_unit();
			Matrix world_to_raster_transformation;
			world_to_raster_transformation.m00 = ppu[0];
			world_to_raster_transformation.m11 = ppu[1];
			world_to_raster_transformation.m20 = target_rect.minx - ppu[0]*source_rect.minx;
			world_to_raster_transformation.m21 = target_rect.miny - ppu[1]*source_rect.miny;

			Rect raf{world_to_raster_transformation.get_transformed(tl), world_to_raster_transformation.get_transformed(br)};
			ra = RectInt{VectorInt{int(raf.get_min()[0]), int(raf.get_min()[1])}, VectorInt{int(raf.get_max()[0]), int(raf.get_max()[1])}};
		}

		const int w(ra.get_width()), h(ra.get_height());
		const Vector upp = get_units_per_pixel();
		const Vector sub_ppu = sub_task(0)->get_pixels_per_unit();

		rendering::TaskSW::LockWrite la(this);
		if (!la)
			return false;

		rendering::TaskSW::LockRead lb(sub_task(0));
		if (!lb)
			return false;
		const synfig::Surface& sub_surface = lb->get_surface();


		Matrix sub_world_to_raster_transformation;
		sub_world_to_raster_transformation.m00 = sub_ppu[0];
		sub_world_to_raster_transformation.m11 = sub_ppu[1];
		sub_world_to_raster_transformation.m20 = sub_task(0)->target_rect.minx - sub_ppu[0]*sub_task(0)->source_rect.minx;
		sub_world_to_raster_transformation.m21 = sub_task(0)->target_rect.miny - sub_ppu[1]*sub_task(0)->source_rect.miny;

		Point pos;
		Surface::pen pen(la->get_surface().get_pen(ra.get_min()[0], ra.get_min()[1]));
		pos[1] = tl[1];
		for (int y=ra.get_min()[1]; y<ra.get_max()[1]; y++, pen.inc_y(), pen.dec_x(w), pos[1]+=upp[1]) {
			pos[0] = tl[0];
			for (int x=ra.get_min()[0]; x<ra.get_max()[0]; x++, pen.inc_x(), pos[0]+=upp[0]) {
				const Point begin = sub_world_to_raster_transformation.get_transformed(pos);//Point(pos - sub_tasks[0]->source_rect.get_min()).divide_coords(upp);
				const Point end = sub_world_to_raster_transformation.get_transformed((pos-origin)*(1.0f-size) + origin);//Point((pos-origin)*(-size)).divide_coords(upp) + begin;

				int x0(round_to_int(begin[0])),
					y0(round_to_int(begin[1])),
					x1(round_to_int(end[0])),
					y1(round_to_int(end[1]));

				int steep = 1;
				int sx, sy;  /* step positive or negative (1 or -1) */
				int dx, dy;  /* delta (difference in X and Y between points) */
				int e;
				int sub_w(sub_surface.get_w());
				int sub_h(sub_surface.get_h());

				dx = std::abs(x1 - x0);
				sx = ((x1 - x0) > 0) ? 1 : -1;
				dy = std::abs(y1 - y0);
				sy = ((y1 - y0) > 0) ? 1 : -1;
				if (dy > dx) {
					steep = 0;
					std::swap(x0, y0);
					std::swap(dx, dy);
					std::swap(sx, sy);
					std::swap(sub_w, sub_h);
				}
				e = (dy << 1) - dx;
				Color pool(Color::alpha());
				int poolsize(0);
				for (int i = 0; i < dx; i++) {
					if(y0>=0 && x0>=0 && y0 < sub_h && x0 < sub_w) {
						if (fade_out) {
							if (steep)
								pool += cooker.cook(sub_surface[y0][x0])*(i-dx);
							else
								pool += cooker.cook(sub_surface[x0][y0])*(i-dx);
							poolsize+=(i-dx);
						} else {
							if (steep)
								pool += cooker.cook(sub_surface[y0][x0]);
							else
								pool += cooker.cook(sub_surface[x0][y0]);
							poolsize+=1;
						}
					} else
						synfig::error("%s:%d unexpected %d >= %d or %d >= %d?\n", __FILE__, __LINE__, x0, w, y0, h);

					while (e >= 0) {
						y0 += sy;
						e -= (dx << 1);
					}
					x0 += sx;
					e += (dy << 1);
				}
				if (poolsize) {
					pool /= poolsize;
					pen.put_value(cooker.uncook(pool));
				}
			}
		}
		return true;
	}
};

rendering::Task::Token TaskRadialBlur::token(
	DescAbstract<TaskRadialBlur>("RadialBlur") );
rendering::Task::Token TaskRadialBlurSW::token(
	DescReal<TaskRadialBlurSW, TaskRadialBlur>("RadialBlurSW") );

RadialBlur::RadialBlur():
	Layer_CompositeFork(1.0,Color::BLEND_STRAIGHT),
	param_origin (ValueBase(Vector(0,0))),
	param_size(ValueBase(Real(0.2))),
	param_fade_out(ValueBase(false))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

RadialBlur::~RadialBlur()
{
}

bool
RadialBlur::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_origin);
	IMPORT_VALUE(param_size);
	IMPORT_VALUE(param_fade_out);

	return Layer_Composite::set_param(param,value);
}

ValueBase
RadialBlur::get_param(const String &param)const
{
	EXPORT_VALUE(param_origin);
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_fade_out);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
RadialBlur::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("origin")
		.set_local_name(_("Origin"))
		.set_description(_("Origin of the blur"))
		.set_is_distance()
	);

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of the blur"))
		.set_origin("origin")
		.set_is_distance()
	);

	ret.push_back(ParamDesc("fade_out")
		.set_local_name(_("Fade Out"))
	);

	return ret;
}

Color
RadialBlur::get_color(Context context, const Point &p)const
{
	//! \writeme
	return context.get_color(p);
}

rendering::Task::Handle
RadialBlur::build_composite_fork_task_vfunc(synfig::ContextParams /*context_params*/, synfig::rendering::Task::Handle sub_task) const
{
	TaskRadialBlur::Handle task_radialblur(new TaskRadialBlur());
	task_radialblur->origin = param_origin.get(Vector());
	task_radialblur->size = param_size.get(Real());
	task_radialblur->fade_out = param_fade_out.get(bool());

	task_radialblur->sub_task(0) = sub_task;

	return task_radialblur;
}

/* === E N T R Y P O I N T ================================================= */
