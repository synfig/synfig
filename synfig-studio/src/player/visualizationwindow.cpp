/*!	\file player/visualizationwindow.cpp
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

#include <cassert>
#include <iomanip>
#include <iostream>

#include <cairomm/cairomm.h>

#include <ETL/misc>

#include <glibmm.h>

#include <gtkmm/drawingarea.h>

#include <synfig/context.h>
#include <synfig/general.h>
#include <synfig/rendering/task.h>
#include <synfig/rendering/common/task/tasktransformation.h>

#include "visualizationwindow.h"


using namespace synfig;


void
VisualizationWindow::Measure::print(const String &name, long long sum, long long rendered_frames) {
	Real time = Real(sum)/Real(rendered_frames)*1e-6;
	std::cout << std::setw(30) << name << std::setw(0) << " : "
			  << std::setw(15) << std::setprecision(6) << std::fixed << time << std::endl;
}



VisualizationWindow::VisualizationWindow(
	const Canvas::Handle &canvas,
	const rendering::Renderer::Handle &renderer,
	bool r_time
):
	canvas(canvas),
	renderer(renderer),
	transform(false),
	frame(0),
	frames_count(1),
	real_time(r_time),
	frame_duration(0),
	pixel_format(0),
	last_frame_time(0),
	last_report_time(0),
	last_report_id(0),
	rendered_frames(0),
	report_seconds(10),
	closed(false)
{
	// prepare rend desc
	rend_desc = canvas->rend_desc();
	rend_desc.set_wh(
		std::max(1, rend_desc.get_w()),
		std::max(1, rend_desc.get_h()) );
	Vector p0 = rend_desc.get_tl();
	Vector p1 = rend_desc.get_br();
	if (p0[0] > p1[0] || p0[1] > p1[1]) {
		if (p0[0] > p1[0]) { matrix.m00 = -1.0; matrix.m20 = p0[0] + p1[0]; std::swap(p0[0], p1[0]); }
		if (p0[1] > p1[1]) { matrix.m11 = -1.0; matrix.m21 = p0[1] + p1[1]; std::swap(p0[1], p1[1]); }
		rend_desc.set_tl_br(p0, p1);
		transform = true;
	}

	// prepare frames counter
	frames_count = std::max(
		frames_count,
		rend_desc.get_frame_end() - rend_desc.get_frame_start() + 1 );
	Real fps = rend_desc.get_frame_rate();
	frame_duration = fps > real_precision<Real>() ? 1/fps : 0;

	// prepare cairo surface
	cairo_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, rend_desc.get_w(), rend_desc.get_h());

	// check endianness and prepare pixel format
	union { int i; char c[4]; } checker = {0x01020304};
	bool big_endian = checker.c[0] == 1;
	pixel_format = big_endian
				 ? (PF_A_START | PF_RGB | PF_A_PREMULT)
				 : (PF_BGR | PF_A | PF_A_PREMULT);
	
	// prepare surface resource
	surface_resource = new rendering::SurfaceResource();
	surface_resource->create(rend_desc.get_w(), rend_desc.get_h());
	
	// prepare measures
	measures.push_back(Measure("other gtk processing"));
	measures.push_back(Measure("set time"));
	measures.push_back(Measure("load resources"));
	measures.push_back(Measure("outline grow"));
	measures.push_back(Measure("create task"));
	measures.push_back(Measure("optimize and render"));
	measures.push_back(Measure("convert to cairo"));
	measures.push_back(Measure("queue paint with cairo"));
	
	// create widgets
	Gtk::DrawingArea *drawing_area = manage(new Gtk::DrawingArea());
	drawing_area->signal_draw().connect(sigc::mem_fun(*this, &VisualizationWindow::on_content_draw));
	drawing_area->set_size_request(
		rend_desc.get_w(),
		rend_desc.get_h() );
	drawing_area->show();
	add(*drawing_area);
	
	// redraw forever
	Glib::signal_idle().connect(sigc::mem_fun(*this, &VisualizationWindow::on_idle));
}

void
VisualizationWindow::on_hide() {
	closed = true;
}

bool
VisualizationWindow::on_idle() {
	if (closed) return false;
	queue_draw();
	return true;
}


bool
VisualizationWindow::convert(const rendering::SurfaceResource::Handle &surface)
{
	rendering::SurfaceResource::LockReadBase surface_lock(surface);
	if (surface_lock.get_resource() && surface_lock.get_resource()->is_blank())
		return false;
	
	if (!surface_lock.convert(rendering::Surface::Token::Handle(), false, true)) {
		synfig::error("convert: surface does not exists");
		return false;
	}
	
	int w = cairo_surface->get_width();
	int h = cairo_surface->get_height();
	const rendering::Surface &s = *surface_lock.get_surface();
	if (w != s.get_width() || h != s.get_height()) {
		synfig::error("convert: surface with wrong size");
		return false;
	}
	
	const Color *pixels = s.get_pixels_pointer();
	std::vector<Color> pixels_copy;
	if (!pixels) {
		pixels_copy.resize(w*h);
		if (s.get_pixels(&pixels_copy.front()))
			pixels = &pixels_copy.front();
	}
	if (!pixels) {
		synfig::error("convert: cannot access surface pixels - that really strange");
		return false;
	}
	
	// do conversion
	color_to_pixelformat(
		cairo_surface->get_data(),
		pixels,
		pixel_format,
		0,
		cairo_surface->get_width(),
		cairo_surface->get_height(),
		cairo_surface->get_stride() );
	cairo_surface->mark_dirty();

	return true;
}


bool
VisualizationWindow::on_content_draw(const Cairo::RefPtr<Cairo::Context> &context) {
	if (!last_frame_time && !last_report_time) {
		last_frame_time = last_report_time = g_get_monotonic_time();
		std::cout << std::endl
				<< "gathering statistics, please wait reports every "
				<< report_seconds << " seconds"
				<< std::endl
				<< std::endl;
	}
	Measure *measure = &measures.front();
	
	(measure++)->last = g_get_monotonic_time();

	Time time = frame_duration*(frame + rend_desc.get_frame_start());
	canvas->set_time(time);

	bef_render_time = g_get_monotonic_time();

	(measure++)->last = g_get_monotonic_time();

	canvas->load_resources(time);

	(measure++)->last = g_get_monotonic_time();
	
	canvas->set_outline_grow(rend_desc.get_outline_grow());

	(measure++)->last = g_get_monotonic_time();
	
	bool surface_exists = false;
	ContextParams context_params(rend_desc.get_render_excluded_contexts());
	rendering::Task::Handle task = canvas->build_rendering_task(context_params);
	if (task) {
		if (transform) {
			rendering::TaskTransformationAffine::Handle t = new rendering::TaskTransformationAffine();
			t->transformation->matrix = matrix;
			t->sub_task() = task;
			task = t;
		}
		surface_resource->clear();
		task->target_surface = surface_resource;
		task->target_rect = RectInt( VectorInt(), task->target_surface->get_size() );
		task->source_rect = Rect(rend_desc.get_tl(), rend_desc.get_br());
	}

	(measure++)->last = g_get_monotonic_time();
	
	if (task)
		renderer->run(task);
	
	(measure++)->last = g_get_monotonic_time();

	if (task)
		surface_exists = convert(task->target_surface);
	
	(measure++)->last = g_get_monotonic_time();

	if (surface_exists) {
		context->set_source(cairo_surface, 0, 0);
		context->paint();
	}
		
	(measure++)->last = g_get_monotonic_time();
	
	assert(measure - 1 == &measures.back());
	for(Measure *m = &measures.front(); m < measure; ++m) {
		m->sum += m->last - last_frame_time;
		last_frame_time = m->last;
	}
	
	if (rendered_frames > 1 && last_frame_time - last_report_time >= report_seconds*1000000ll) {
		std::cout << "report #" << (++last_report_id) << std::endl;
		Measure *mb = &measures.front();
		for(Measure *m = mb+1; m < measure; ++m) {
			m->print(rendered_frames);
			m->sum = 0;
		}
		mb->print(rendered_frames); // print 'other gtk' at last row
		mb->sum = 0;
		Measure::print("whole frame", last_frame_time - last_report_time, rendered_frames);
		std::cout << std::endl;
		last_report_time = last_frame_time;
		rendered_frames = 0;
	}
	
	++rendered_frames;
	if(real_time) {
		frame = (frame + etl::round_to_int((g_get_monotonic_time() - bef_render_time) / (frame_duration * 1000000ll))) %
				frames_count;
	}else{
		frame = (frame + 1) % frames_count;
	}
	queue_draw();
	return true;
}
