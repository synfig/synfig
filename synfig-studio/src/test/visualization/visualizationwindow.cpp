
#include <glibmm.h>

#include <cairomm/cairomm.h>

#include <gtkmm/drawingarea.h>

#include <synfig/general.h>
#include <synfig/context.h>
#include <synfig/rendering/task.h>
#include <synfig/rendering/common/task/tasktransformation.h>

#include "visualizationwindow.h"


using namespace synfig;


VisualizationWindow::VisualizationWindow(
	const Canvas::Handle &canvas,
	const rendering::Renderer::Handle &renderer
):
	canvas(canvas),
	renderer(renderer),
	transform(false),
	frame(0),
	frames_count(1),
	frame_duration(0),
	pixel_format(0)
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

bool
VisualizationWindow::on_idle() {
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
	context->move_to(0, 0);
	context->line_to(frame, 100);
	context->stroke();
	
	Time time = frame_duration*(frame + rend_desc.get_frame_start());
	canvas->set_time(time);
	
	canvas->load_resources(time);

	canvas->set_outline_grow(rend_desc.get_outline_grow());

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

	if (task)
		renderer->run(task);
	
	if (task)
		surface_exists = convert(task->target_surface);
	
	if (surface_exists) {
		context->set_source(cairo_surface, 0, 0);
		context->paint();
	}
		
	frame = (frame + 1) % frames_count;
	return true;
}
