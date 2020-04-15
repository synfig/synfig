#ifndef __SYNFIG_TEST_VISULIZATION_WINDOW_H
#define __SYNFIG_TEST_VISULIZATION_WINDOW_H

#include <vector>

#include <gtkmm/window.h>

#include <synfig/canvas.h>
#include <synfig/renddesc.h>
#include <synfig/rendering/renderer.h>
#include <synfig/rendering/surface.h>


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
	
	synfig::PixelFormat pixel_format;
	Cairo::RefPtr<Cairo::ImageSurface> cairo_surface;
	synfig::rendering::SurfaceResource::Handle surface_resource;

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
		const synfig::rendering::Renderer::Handle &renderer );
	
	bool convert(const synfig::rendering::SurfaceResource::Handle &surface);
	
	bool on_content_draw(const Cairo::RefPtr<Cairo::Context> &context);

	bool on_idle();

	virtual void on_hide();
};

#endif


