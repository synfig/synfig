/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_soundwave.cpp
**	\brief Dock for display a user-configurable Widget_SoundWave
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  ......... ... 2019 Rodolfo Ribeiro Gomes
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

#include "dock_soundwave.h"

#include <widgets/widget_soundwave.h>
#include <gui/canvasview.h>
#include <gui/localization.h>

#include <synfig/general.h>

#endif

using namespace studio;

Dock_SoundWave::Dock_SoundWave()
	: Dock_CanvasSpecific("soundwave", _("Sound Wave"), Gtk::StockID("synfig-layer_other_sound")),
	  current_widget_sound(nullptr)
{
	widget_kf_list.set_hexpand();
	widget_kf_list.show();
	widget_timeslider.set_hexpand();
	widget_timeslider.show();

	file_button.show();
	file_button.signal_file_set().connect(sigc::mem_fun(*this, &Dock_SoundWave::on_file_button_clicked));
	Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
	filter->add_mime_type("audio/*");
	file_button.set_filter(filter);

	clear_button.set_label(_("Clear"));
//	clear_button.get_style_context()->add_class("button");
	clear_button.show();
	clear_button.signal_clicked().connect(sigc::mem_fun(*this, &Dock_SoundWave::on_clear_button_clicked));

	vscrollbar.set_vexpand();
	vscrollbar.set_hexpand(false);
	vscrollbar.show();
	hscrollbar.set_hexpand();
	hscrollbar.show();

	grid.set_column_homogeneous(false);
	grid.set_row_homogeneous(false);
	add(grid);

	file_settings_box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
	file_settings_box.set_homogeneous(false);
	file_settings_box.set_spacing(2);
	file_settings_box.pack_start(clear_button, false, false);
	file_settings_box.hide();

	file_box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
	file_box.set_homogeneous(false);
	file_box.set_spacing(2);
	file_box.pack_start(file_button, false, false);
	file_box.pack_start(file_settings_box, false, false);

	drag_dest_set(Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_COPY);
	signal_drag_data_received().connect(sigc::mem_fun(*this,
				&Dock_SoundWave::on_drop_drag_data_received) );
}

void Dock_SoundWave::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	Widget_SoundWave *widget_sound = new Widget_SoundWave();
	widget_sound->set_time_model(canvas_view->time_model());
	widget_sound->show();
	widget_sound->set_hexpand(true);
	widget_sound->set_vexpand(true);
	widget_sound->signal_file_loaded().connect([=](const std::string &filename) {
		if (!filename.empty()) {
			file_button.set_uri(filename);
		} else {
			file_button.unselect_all();
		}
		file_settings_box.set_visible(!filename.empty());
	});
	canvas_view->set_ext_widget(get_name(), widget_sound);
}

void Dock_SoundWave::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	std::lock_guard<std::mutex> lock(mutex);

	const std::vector<Gtk::Widget*> children = grid.get_children();
	for (Gtk::Widget * widget : children) {
		// CanvasView and Dock_SoundWave will delete widgets when needed
		grid.remove(*widget);
	}

	if( !canvas_view ) {
		widget_kf_list.set_time_model( etl::handle<TimeModel>() );
		widget_kf_list.set_canvas_interface( etl::loose_handle<synfigapp::CanvasInterface>() );

		widget_timeslider.set_canvas_view( CanvasView::Handle() );

		current_widget_sound = nullptr; // deleted by its studio::CanvasView::~CanvasView()

		file_box.hide();

		hscrollbar.unset_adjustment();
		vscrollbar.unset_adjustment();

		std::vector< Gtk::TargetEntry > targets;
		drag_dest_set(targets);
	} else {
		widget_kf_list.set_time_model(canvas_view->time_model());
		widget_kf_list.set_canvas_interface(canvas_view->canvas_interface());

		widget_timeslider.set_canvas_view(canvas_view);

		current_widget_sound = dynamic_cast<Widget_SoundWave*>( canvas_view->get_ext_widget(get_name()) );
		current_widget_sound->set_size_request(100, 100);
		current_widget_sound->set_hexpand(true);
		current_widget_sound->set_vexpand(true);

		std::string filename = current_widget_sound->get_filename();
		if (filename.empty())
			file_button.unselect_all();
		else
			file_button.set_uri(filename);
		file_settings_box.set_visible(!filename.empty());
		file_box.show();

		vscrollbar.set_adjustment(current_widget_sound->get_range_adjustment());
		hscrollbar.set_adjustment(canvas_view->time_model()->scroll_time_adjustment());

		grid.attach(widget_kf_list,        0, 0, 1, 1);
		grid.attach(widget_timeslider,     0, 1, 1, 1);
		grid.attach(*current_widget_sound, 0, 2, 1, 1);
		grid.attach(file_box,              0, 3, 1, 1);
		grid.attach(hscrollbar,            0, 4, 2, 1);
		grid.attach(vscrollbar,            1, 0, 1, 4);
		grid.show();

		std::vector< Gtk::TargetEntry > targets;
		drag_dest_set(targets);
		drag_dest_add_uri_targets();
	}

}

void Dock_SoundWave::on_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time)
{
	bool success = false;
	const int length = selection_data.get_length();
	if((length >= 0) && (selection_data.get_format() == 8)) {
		std::string uri_list_string = selection_data.get_data_as_string();
		std::vector<std::string> uris;
		std::istringstream f(uri_list_string);
		std::string s;
		while (getline(f, s)) {
			// r-trim
			s.erase(std::find_if(s.rbegin(), s.rend(),
									[](int chr) { return !std::isspace(chr);}).base(),
			        s.end()
			       );
			uris.push_back(s);
		}

		success = load_sound_file(uris.front());
	}
	context->drag_finish(success, false, time);
}

void Dock_SoundWave::on_file_button_clicked()
{
	load_sound_file(file_button.get_uri());
}

void Dock_SoundWave::on_clear_button_clicked()
{
	std::lock_guard<std::mutex> lock(mutex);
	file_button.unselect_all();
	current_widget_sound->clear();
	file_settings_box.hide();
}

bool Dock_SoundWave::load_sound_file(const std::string& filename)
{
	file_button.set_sensitive(false);
	std::lock_guard<std::mutex> lock(mutex);
	bool ok = current_widget_sound->load(filename);
	if (ok) {
		file_settings_box.show();
	} else {
		synfig::warning("Audio file not supported");
	}
	file_button.set_sensitive(true);
	return ok;
}
