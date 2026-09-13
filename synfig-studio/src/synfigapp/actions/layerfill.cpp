/* === S Y N F I G ========================================================= */
/*!	\file layerfillt.cpp
**	\brief Brush stroke action for StateBrush with live preview
**
**	\legal
**	Copyright (c) 2026 Synfig Contributors
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
#   include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include "layerfill.h"

#include <synfig/rendering/software/surfacesw.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/localization.h>

#endif

using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::BitmapLayerFill);
ACTION_SET_NAME(Action::BitmapLayerFill, "BitmapLayerFill");
ACTION_SET_LOCAL_NAME(Action::BitmapLayerFill, N_("Fill"));
ACTION_SET_TASK(Action::BitmapLayerFill, "paint");
ACTION_SET_CATEGORY(Action::BitmapLayerFill, Action::CATEGORY_NONE);
ACTION_SET_PRIORITY(Action::BitmapLayerFill, 0);
ACTION_SET_VERSION(Action::BitmapLayerFill, "0.0");

/* === G L O B A L S ======================================================= */

struct FloodFiller {
private:
	const Color search_color;
	Surface& surface;
	Color::value_type tolerance;
	std::vector<PointInt> border_pixels;

public:
	FloodFiller(const Color c, Surface& s, Color::value_type tolerance)
		: search_color(c), surface(s), tolerance(tolerance)
	{
	}

	void
	fill(int flood_x, int flood_y, const Color& desired_color)
	{
		struct Span {
			int x1;
			int x2;
			int y;
			int dy;
		};

		std::vector<Span> queue;

		queue.push_back({flood_x, flood_x, flood_y, 1});
		queue.push_back({flood_x, flood_x, flood_y - 1, 1});

		while (!queue.empty()) {
			Span s = queue.back();
			queue.pop_back();

			int x = s.x1;
			if (is_inside(x, s.y)) {
				while (is_inside(x - 1, s.y)) {
					paint_pixel(x-1, s.y, desired_color);
					x = x - 1;
				}
				insert_new_border_pixel(x-1, s.y);
				if (x < s.x1)
					queue.push_back({x, s.x1 - 1, s.y - s.dy, -s.dy});
			}
			insert_new_border_pixel(x, s.y);
			while (s.x1 <= s.x2) {
				while (is_inside(s.x1, s.y)) {
					paint_pixel(s.x1, s.y, desired_color);
					s.x1 += 1;
				}
				insert_new_border_pixel(s.x1, s.y);
				if (s.x1 > x)
					queue.push_back({x, s.x1 - 1, s.y + s.dy, s.dy});
				if (s.x1 - 1 > s.x2)
					queue.push_back({s.x2 + 1, s.x1 - 1, s.y - s.dy, -s.dy});
				s.x1 += 1;
				while (s.x1 <= s.x2 && !is_inside(s.x1, s.y)) {
					insert_new_border_pixel(s.x1, s.y);
					s.x1 += 1;
				}
				x = s.x1;
			}
		}
	}

	bool
	is_inside(int x, int y) const
	{
		if (x < 0 || x >= surface.get_w())
			return false;
		if (y < 0 || y >= surface.get_h())
			return false;

		const Color& color = surface[y][x];
		const Color::value_type dR = search_color.get_r() - color.get_r();
		const Color::value_type dG = search_color.get_g() - color.get_g();
		const Color::value_type dB = search_color.get_b() - color.get_b();
		const Color::value_type r = (search_color.get_r() + color.get_r()) / 2. * 255;

		const Color::value_type dist = sqrt(255*((2+r/256)*dR*dR + 4*dG*dG + (2 + (255-r)/256)*dB*dB));
		const Color::value_type alpha_dist = 255 * std::fabs(color.get_alpha() - search_color.get_alpha());
		return dist < tolerance && alpha_dist < tolerance;
	}

	void
	insert_new_border_pixel(int x, int y)
	{
		if (x < 0 || x >= surface.get_w())
			return;
		if (y < 0 || y >= surface.get_h())
			return;
		for (const auto& p : border_pixels) {
			if (p[0] == x && p[1] == y)
				return;
		}
		border_pixels.push_back({x, y});
	};

	void
	paint_pixel(int x, int y, const Color& desired_color)
	{
		surface[y][x] = desired_color;
		// Color& surface_color = surface[y][x];
		// Color::value_type a = surface_color.get_a();
		// surface_color = color;
		// surface_color.set_a(a);
	};

	void
	antialias_fill_border(const Color& desired_color)
	{
		for (const auto& b : border_pixels) {
			const int x = b[0];
			const int y = b[1];
			if (surface[y][x] == desired_color)
				continue;

			Color surrounding_color;
			Real sn = 0;
			if (x - 1 >= 0) {
				surrounding_color += surface[y][x-1];
				++sn;
				if (y - 1 >= 0) {
					surrounding_color += surface[y-1][x-1];
					++sn;
				}
				if (y + 1 < surface.get_h()) {
					surrounding_color += surface[y+1][x-1];
					++sn;
				}
			}
			if (x + 1 <= surface.get_w()) {
				surrounding_color += surface[y][x+1];
				++sn;
				if (y - 1 >= 0) {
					surrounding_color += surface[y-1][x+1];
					++sn;
				}
				if (y + 1 < surface.get_h()) {
					surrounding_color += surface[y+1][x+1];
					++sn;
				}
			}
			if (y - 1 >= 0) {
				surrounding_color += surface[y-1][x];
				++sn;
			}
			if (y + 1 < surface.get_h()) {
				surrounding_color += surface[y+1][x];
				++sn;
			}
			surrounding_color /= 2 * sn;
			surrounding_color += surface[y][x] / 2.;

			surface[b[1]][b[0]] = surrounding_color;
		}
	}

	void
	clear_border()
	{
		border_pixels.clear();
	}

	RectInt
	get_border_bounding_box()
	{
		if (border_pixels.empty())
			return RectInt{-1, -1};

		RectInt rect(border_pixels[0]);
		for (const auto& pixel : border_pixels) {
			rect.expand(pixel);
		}
		return rect;
	}
};

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

RectInt
Action::BitmapLayerFill::fill(synfig::Surface& surface)
{
	const int flood_x = flood_point[0];
	const int flood_y = flood_point[1];

	const Color search_color = surface[flood_y][flood_x];
	if (search_color == color)
		return RectInt{-1, -1};

	FloodFiller flood_filler(search_color, surface, tolerance);

	flood_filler.fill(flood_x, flood_y, color);

	if (antialiasing) {
		flood_filler.antialias_fill_border(color);
	}

	return flood_filler.get_border_bounding_box();
}

Action::BitmapLayerFill::BitmapLayerFill()
	: flood_point(-1, -1),
	  tolerance(5),
	  antialiasing(true)
{
}

Action::ParamVocab
Action::BitmapLayerFill::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer", Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
		.set_desc(_("Layer to paint on"))
	);

	ret.push_back(ParamDesc("point", Param::TYPE_VALUE)
		.set_local_name(_("Point"))
		.set_desc(_("Point from where start the fill flooding"))
	);

	ret.push_back(ParamDesc("color", Param::TYPE_VALUE)
		.set_local_name(_("Color"))
		.set_desc(_("Fill color to flood"))
	);

	ret.push_back(ParamDesc("tolerance", Param::TYPE_REAL)
		.set_local_name(_("Tolerance"))
		.set_desc(_("How much color difference to accept (from 0 to 255)"))
	);

	ret.push_back(ParamDesc("antialiasing", Param::TYPE_BOOL)
		.set_local_name(_("Antialiasing"))
		.set_desc(_("Try to smooth the border pixel colors"))
	);

	return ret;
}

bool
Action::BitmapLayerFill::is_candidate(const ParamList& x)
{
	// Check if we have a layer parameter and it's a bitmap layer
	for (const auto& i : x) {
		if (i.first == "layer" && i.second.get_type() == Param::TYPE_LAYER) {
			Layer::Handle layer = i.second.get_layer();
			if (Layer_Bitmap::Handle::cast_dynamic(layer)) {
				return true;
			}
		}
	}
	return false;
}

bool
Action::BitmapLayerFill::set_param(const synfig::String& name, const Action::Param& param)
{
	if (name == "layer" && param.get_type() == Param::TYPE_LAYER) {
		Layer_Bitmap::Handle bitmap_layer = Layer_Bitmap::Handle::cast_dynamic(param.get_layer());
		if (bitmap_layer) {
			layer = bitmap_layer;
			return true;
		}
		return false;
	}

	if (name == "point" && param.get_type() == Param::TYPE_VALUE) {
		auto value = param.get_value();
		if (value.get_type() == type_vector) {
			auto point = value.get(Point());
			flood_point[0] = point[0];
			flood_point[1] = point[1];
			return true;
		}
		return false;
	}

	if (name == "color" && param.get_type() == Param::TYPE_VALUE) {
		auto value = param.get_value();
		if (value.get_type() == type_color) {
			color = value.get(Color());
			return true;
		}
		return false;
	}

	if (name == "tolerance" && param.get_type() == Param::TYPE_REAL) {
		if (param.get_real() < 0)
			return false;
		tolerance = param.get_real();
		return true;
	}

	if (name == "antialiasing" && param.get_type() == Param::TYPE_BOOL) {
		antialiasing = param.get_bool();
		return true;
	}

	return CanvasSpecific::set_param(name, param);
}

bool
Action::BitmapLayerFill::is_ready() const
{
	return layer && layer->rendering_surface && color.is_valid() && flood_point[0] >= 0 && flood_point[1] >= 0 && CanvasSpecific::is_ready();
}

void
Action::BitmapLayerFill::perform()
{
	if (!layer) {
		return;
	}

	if (!original_surface.is_valid()) {
		if (layer->rendering_surface) {
			rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(layer->rendering_surface);
			if (lock) {
				original_surface = lock->get_surface();
			}
		}
	}

	rendering::SurfaceResource::LockWrite<rendering::SurfaceSW> lock(layer->rendering_surface);
	if (lock && lock->get_surface().is_valid()) {
		synfig::Surface& surface = lock->get_surface();
		fill(surface);
	}

	layer->changed();

	if (get_canvas_interface()) {
		get_canvas_interface()->signal_layer_param_changed()(layer, "rendering_surface");
	}

	layer->add_surface_modification_id(id);
}

void
Action::BitmapLayerFill::undo()
{
	if (!original_surface.is_valid())
		return;

	rendering::SurfaceResource::LockWrite<rendering::SurfaceSW> lock(layer->rendering_surface);
	if (lock && lock->get_surface().is_valid()) {
		synfig::Surface& surface = lock->get_surface();
		surface = original_surface;

		layer->changed();
		if (get_canvas_interface())
			get_canvas_interface()->signal_layer_param_changed()(layer, "rendering_surface");
		layer->add_surface_modification_id(id);
	}
}
