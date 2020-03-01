#ifndef __SYNFIG_TEST_VISULIZATION_WINDOW_H
#define __SYNFIG_TEST_VISULIZATION_WINDOW_H


#include <gtkmm/window.h>

#include <synfig/canvas.h>
#include <synfig/renddesc.h>
#include <synfig/rendering/renderer.h>
#include <synfig/rendering/surface.h>


class VisualizationWindow: public Gtk::Window {
private:
	synfig::Canvas::Handle canvas;
	synfig::rendering::Renderer::Handle renderer;
	
	synfig::RendDesc rend_desc;
	bool transform;
	synfig::Matrix matrix;
	int frame;
	int frames_count;
	synfig::Time frame_duration;
	
	synfig::PixelFormat pixel_format;
	Cairo::RefPtr<Cairo::ImageSurface> cairo_surface;
	synfig::rendering::SurfaceResource::Handle surface_resource;
	
public:
	VisualizationWindow(
		const synfig::Canvas::Handle &canvas,
		const synfig::rendering::Renderer::Handle &renderer );
	
	bool convert(const synfig::rendering::SurfaceResource::Handle &surface);
	
	bool on_content_draw(const Cairo::RefPtr<Cairo::Context> &context);

	bool on_idle();
};

#endif


