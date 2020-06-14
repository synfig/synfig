/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_soundwave.h
**	\brief Widget for display a sound wave time-graph
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2019 Rodolfo Ribeiro Gomes
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "widget_soundwave.h"

#include <synfig/general.h>
#include <gui/canvasview.h>
#include <gui/timeplotdata.h>
#include <gui/helpers.h>
#include <gui/localization.h>

#include <Mlt.h>

#include <cairomm/cairomm.h>
#include <gdkmm.h>
#include <glibmm/convert.h>
#include <cstring>
#endif

using namespace studio;

const int default_frequency = 48000;
const int default_n_channels = 2;

Widget_SoundWave::MouseHandler::~MouseHandler() {}

Widget_SoundWave::Widget_SoundWave()
    : Widget_TimeGraphBase(),
	  frequency(default_frequency),
	  n_channels(default_n_channels),
	  n_samples(0),
	  channel_idx(0),
	  loading_error(false)
{
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::SCROLL_MASK | Gdk::POINTER_MOTION_MASK | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
	setup_mouse_handler();

	set_default_page_size(255);
	set_zoom(1.0);

	int range_max = 255;
	int range_min = 0;
	int h = get_allocated_height();

	ConfigureAdjustment(range_adjustment)
		.set_lower(-range_max /*- 0.05*range_adjustment->get_page_size()*/)
		.set_upper(-range_min /*+ 0.05*range_adjustment->get_page_size()*/)
		.set_step_increment(range_adjustment->get_page_size()*20.0/h) // 20 pixels
		.set_value((range_max-range_min)/2)
		.finish();
}

Widget_SoundWave::~Widget_SoundWave()
{
	clear();
}

bool Widget_SoundWave::load(const std::string& filename)
{
	clear();

	std::lock_guard<std::mutex> lock(mutex);
	if (!do_load(filename)) {
		loading_error = true;
		queue_draw();
		return false;
	}
	this->filename = filename;
	signal_file_loaded().emit(filename);
	queue_draw();
	return true;
}

void Widget_SoundWave::clear()
{
	std::lock_guard<std::mutex> lock(mutex);
	buffer.clear();
	this->filename.clear();
	loading_error = false;
	sound_delay = 0.0;
	channel_idx = 0;
	n_samples = 0;
	queue_draw();
}

void Widget_SoundWave::set_channel_idx(int new_channel_idx)
{
	if (channel_idx != new_channel_idx && new_channel_idx >= 0 && new_channel_idx < n_channels) {
		channel_idx = new_channel_idx;
		queue_draw();
	}
}

int Widget_SoundWave::get_channel_idx() const
{
	return channel_idx;
}

int Widget_SoundWave::get_channel_number() const
{
	return n_channels;
}

void Widget_SoundWave::set_delay(synfig::Time delay)
{
	if (delay == sound_delay)
		return;

	std::lock_guard<std::mutex> lock(mutex);
	sound_delay = delay;
	buffer.clear();
	n_samples = 0;
	do_load(filename);
	queue_draw();
}

const synfig::Time& Widget_SoundWave::get_delay() const
{
	return sound_delay;
}

bool Widget_SoundWave::on_event(GdkEvent* event)
{
	if (mouse_handler.process_event(event))
		return true;
	return Widget_TimeGraphBase::on_event(event);
}

bool Widget_SoundWave::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
	if (Widget_TimeGraphBase::on_draw(cr))
		return true;

	if (loading_error) {
		Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_pango_context()));
		layout->set_text(_("Audio file not supported"));

		cr->save();
		cr->set_source_rgb(0.8, 0.3, 0.3);
		cr->move_to(5, get_height()/2);
		layout->show_in_cairo_context(cr);
		cr->restore();
	}

	if (filename.empty())
		return true;

	if (buffer.empty())
		return true;

	cr->save();

	std::lock_guard<std::mutex> lock(mutex);

	const int bytes_per_sample = 1;

	Gdk::RGBA color = get_style_context()->get_color();
	cr->set_source_rgb(color.get_red(), color.get_green(), color.get_blue());

	const int middle_y = 127;
	cr->move_to(0, middle_y);

	const int stride = frequency * bytes_per_sample * n_channels;

	for (double x = 0; x < get_width(); x+=0.1) {
		synfig::Time t = time_plot_data->get_t_from_pixel_coord(x);
		if (synfig::approximate_greater(synfig::Real(sound_delay), 0.0))
			t = t - sound_delay;
		synfig::Time dt = t - time_plot_data->time_model->get_lower();
		int index = int(dt * stride) + channel_idx;
		int value = middle_y;
		if (index >= 0 && index < buffer.size())
			std::copy(buffer.begin() + index, buffer.begin() + index + bytes_per_sample, &value);
		int y = time_plot_data->get_pixel_y_coord(value);
		cr->line_to(x, y);
	}
	cr->set_line_width(0.3);
	cr->stroke();

	draw_current_time(cr);

	int h = get_height();

	ConfigureAdjustment(range_adjustment)
		.set_step_increment(range_adjustment->get_page_size()*20.0/h) // 20 pixels
		.finish();

	cr->restore();
	return true;
}

void Widget_SoundWave::set_time_model(const etl::handle<TimeModel>& x)
{
	if (x) {
		previous_lower_time = x->get_lower();
		previous_upper_time = x->get_upper();
	}
	Widget_TimeGraphBase::set_time_model(x);
}

void Widget_SoundWave::on_time_model_changed()
{
	if (filename.empty())
		return;
	if (previous_lower_time == time_plot_data->time_model->get_lower()
		&& previous_upper_time == time_plot_data->time_model->get_upper())
		return;

	previous_lower_time = time_plot_data->time_model->get_lower();
	previous_upper_time = time_plot_data->time_model->get_upper();

	std::lock_guard<std::mutex> lock(mutex);
	buffer.clear();
	n_samples = 0;
	do_load(filename);
	queue_draw();
}

void Widget_SoundWave::setup_mouse_handler()
{
	mouse_handler.set_pan_enabled(true);
	mouse_handler.set_zoom_enabled(true);
	mouse_handler.set_scroll_enabled(true);
	mouse_handler.set_canvas_interface(canvas_interface);
	mouse_handler.signal_redraw_needed().connect(sigc::mem_fun(*this, &Gtk::Widget::queue_draw));
	mouse_handler.signal_focus_requested().connect(sigc::mem_fun(*this, &Gtk::Widget::grab_focus));
	mouse_handler.signal_zoom_in_requested().connect(sigc::mem_fun(*this, &Widget_SoundWave::zoom_in));
	mouse_handler.signal_zoom_out_requested().connect(sigc::mem_fun(*this, &Widget_SoundWave::zoom_out));
	mouse_handler.signal_scroll_up_requested().connect(sigc::mem_fun(*this, &Widget_SoundWave::scroll_up));
	mouse_handler.signal_scroll_down_requested().connect(sigc::mem_fun(*this, &Widget_SoundWave::scroll_down));
	mouse_handler.signal_panning_requested().connect(sigc::mem_fun(*this, &Widget_SoundWave::pan));
}

bool Widget_SoundWave::do_load(const std::string& filename)
{
	std::string real_filename = Glib::filename_from_uri(filename);
	Mlt::Profile profile;
	Mlt::Producer *track = new Mlt::Producer(profile, (std::string("avformat:") + real_filename).c_str());
	if (!track->get_producer() || track->get_length() <= 0) {
		delete track;
		track = new Mlt::Producer(profile, (std::string("vorbis:") + real_filename).c_str());
		if (!track->get_producer() || track->get_length() <= 0) {
			delete track;
			return false;
		}
	}

	const int length = track->get_length();
	double fps = track->get_fps();
	int start_frame = (time_plot_data->time_model->get_lower() - sound_delay) * fps;
	int end_frame = (time_plot_data->time_model->get_upper() - sound_delay) * fps;
	start_frame = synfig::clamp(start_frame, 0, length);
	end_frame = synfig::clamp(end_frame, 0, length);
	track->seek(start_frame);
	// check if audio is seekable
	if (track->position() != start_frame) {
		// Not seekable!
		synfig::error("Audio file not seekable, but a delay (%s) was set: %s", sound_delay.get_string(time_plot_data->time_model->get_frame_rate()).c_str(), filename.c_str());
	}
	unsigned char *outbuffer = nullptr;
	int bytes_written = 0;

	for (int i = start_frame; i < end_frame; ++i) {
		Mlt::Frame *frame = track->get_frame(0);
		if (!frame)
			break;
		mlt_audio_format format = mlt_audio_u8;
		int bytes_per_sample = 1;
		int _frequency = frequency? frequency : default_frequency;
		int _channels = n_channels ? n_channels : default_n_channels;
		int _n_samples = 0;
		void * _buffer = frame->get_audio(format, _frequency, _channels, _n_samples);
		if (_buffer == nullptr) {
			delete frame;
			break;
		}
		if (buffer.empty()) {
			int buffer_length = (end_frame - start_frame + 1) * _channels * bytes_per_sample * std::round(_frequency/fps);
			buffer.resize(buffer_length);
		}
		int _n_bytes = _n_samples * _channels * bytes_per_sample;
		std::copy(static_cast<unsigned char*>(_buffer), static_cast<unsigned char*>(_buffer) + _n_bytes, buffer.begin()+bytes_written);
		bytes_written += _n_bytes;
		outbuffer += _n_bytes;
		frequency = _frequency;
		n_channels = _channels;
		n_samples += _n_samples;
		delete frame;
	}
	if (channel_idx > n_channels)
		channel_idx = 0;
	queue_draw();
	delete track;
	return true;
}
