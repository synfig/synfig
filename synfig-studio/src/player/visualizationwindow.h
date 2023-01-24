/*!	\file player/visualizationwindow.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2020 Ivan Mahonin
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

#ifndef __SYNFIG_TEST_VISUALIZATION_WINDOW_H
#define __SYNFIG_TEST_VISUALIZATION_WINDOW_H

#include <gtkmm/window.h>

#include <synfig/canvas.h>
#include <synfig/renddesc.h>
#include <synfig/rendering/renderer.h>
#include <synfig/rendering/surface.h>

#include <vector>

class VisualizationWindow: public Gtk::Window {
public:
	struct Measure {
		synfig::String name;
		long long last;
		long long sum;
		explicit Measure(const synfig::String &name = synfig::String()):
			name(name), last(), sum() { }

		static void print(const synfig::String &name, long long sum, long long rendered_frames);

		void print(long long rendered_frames) const
			{ print(name, sum, rendered_frames); }
	};
	typedef std::vector<Measure> MeasureList;
	
private:
	synfig::Canvas::Handle canvas;
	synfig::rendering::Renderer::Handle renderer;
	
	synfig::RendDesc rend_desc;
	bool transform;
	synfig::Matrix matrix;
	int frame;
	int frames_count;
	synfig::Time frame_duration;

	bool real_time;
	synfig::PixelFormat pixel_format;
	Cairo::RefPtr<Cairo::ImageSurface> cairo_surface;
	synfig::rendering::SurfaceResource::Handle surface_resource;

	long long bef_render_time;
	long long last_frame_time;
	long long last_report_time;
	long long last_report_id;
	long long rendered_frames;
	long long report_seconds;
	MeasureList measures;
	
	bool closed;
	
public:
	VisualizationWindow(
		const synfig::Canvas::Handle &canvas,
		const synfig::rendering::Renderer::Handle &renderer,
		bool r_time =true);
	
	bool convert(const synfig::rendering::SurfaceResource::Handle &surface);
	
	bool on_content_draw(const Cairo::RefPtr<Cairo::Context> &context);

	bool on_idle();

	virtual void on_hide();
};

#endif
