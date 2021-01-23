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

#include <glibmm/convert.h> // Glib::filename_from_uri
#include <gtkmm/stylecontext.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/localization.h>
#include <gui/widgets/widget_soundwave.h>
#include <gui/widgets/widget_time.h>

#include <synfig/canvasfilenaming.h>
#include <synfig/general.h>
#include <synfig/layers/layer_sound.h>
#include <synfig/string_helper.h>

#include <synfigapp/value_desc.h>

#endif

using namespace studio;

namespace studio {

class Grid_SoundWave : public Gtk::Grid {
	Widget_SoundWave widget_sound;
	Gtk::Box file_box;
	Gtk::Box file_settings_box;
	Gtk::ComboBoxText file_combo;
	Gtk::ComboBoxText channel_combo;
	Gtk::Label label_delay;
	Widget_Time delay_widget;

	std::mutex mutex;

	static const std::string item_no_audio_id;
	static const std::string item_no_audio_str;
	static const std::string item_audio_file_str;
	static const std::string item_audio_file_id;

	bool open_dialog_disabled;

	// guid -> layer
	std::map<std::string, etl::handle<synfig::Layer_Sound>> layer_map;

	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface;

public:
	Grid_SoundWave(etl::loose_handle<CanvasView> canvas_view)
		: Gtk::Grid(),
		  open_dialog_disabled(false),
		  canvas_interface(canvas_view->canvas_interface())
	{
		setup_accessor_widgets();
		setup_soundwave_widget(canvas_view);
		setup_canvas_layer_signals(canvas_view->canvas_interface());

		attach(widget_sound, 0, 0, 1, 1);
		attach(file_box, 0, 1, 1, 1);
	}

	Widget_SoundWave & get_widget_sound() {
		return widget_sound;
	}

	bool append_and_set_audio(const std::string &filename) {
		if (filename.empty()) {
			file_combo.set_active_id(item_no_audio_id);
			return true;
		} else {
			open_dialog_disabled = true;
			file_combo.set_active_id(item_audio_file_id);
			open_dialog_disabled = false;
			if (import_file(filename))
				return true;
			file_combo.set_active_id(item_no_audio_id);
			return false;
		}
	}

private:
	void setup_accessor_widgets() {
		file_combo.show();
		file_combo.set_wrap_width(1); // based on https://github.com/synfig/synfig/issues/650#issuecomment-450001367
		file_combo.set_tooltip_text(_("Select a sound layer or type an audio file"));
		file_combo.append(item_no_audio_id, item_no_audio_str);
		file_combo.append(item_audio_file_id, item_audio_file_str);
		file_combo.signal_changed().connect(sigc::mem_fun(*this, &Grid_SoundWave::on_file_combo_changed));
		file_combo.set_active_id(item_no_audio_id);

		channel_combo.show();
		channel_combo.set_wrap_width(1); // based on https://github.com/synfig/synfig/issues/650#issuecomment-450001367
		channel_combo.set_tooltip_text(_("What sound channel to display"));
		channel_combo.signal_changed().connect(sigc::mem_fun(*this, &Grid_SoundWave::on_channel_combo_changed));

		label_delay.set_text(_("Delay:"));
		label_delay.show();

		delay_widget.show();
		delay_widget.signal_value_changed().connect(sigc::mem_fun(*this, &Grid_SoundWave::on_delay_changed));

		file_settings_box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
		file_settings_box.set_homogeneous(false);
		file_settings_box.set_spacing(2);
		file_settings_box.pack_start(channel_combo, false, false);
		file_settings_box.pack_start(label_delay, false, false);
		file_settings_box.pack_start(delay_widget, false, false);
		file_settings_box.hide();

		file_box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
		file_box.set_homogeneous(false);
		file_box.set_spacing(2);
		file_box.pack_start(file_combo, false, false);
		file_box.pack_start(file_settings_box, false, false);

		file_box.show();
	}

	void setup_soundwave_widget(etl::loose_handle<CanvasView> canvas_view) {
		widget_sound.set_time_model(canvas_view->time_model());
		widget_sound.show();
		widget_sound.set_size_request(100, 100);
		widget_sound.set_hexpand(true);
		widget_sound.set_vexpand(true);
		widget_sound.signal_file_loaded().connect([=](const std::string &filename) {
			if (!filename.empty()) {
//				file_button.set_uri(filename);
			} else {
				if (file_combo.get_active_id() != item_no_audio_id)
					file_combo.set_active_id(item_no_audio_id);
			}
			file_settings_box.set_visible(!filename.empty());
		});

		fill_sound_layer_combo(canvas_view->get_canvas());

	}

	void setup_canvas_layer_signals(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface) {
		canvas_interface->signal_layer_inserted().connect([&](synfig::Layer::Handle layer, int /*pos*/) {
			etl::handle<synfig::Layer_Sound> layer_sound = etl::handle<synfig::Layer_Sound>::cast_dynamic(layer);
			if (!layer_sound)
				return;

			add_layer_to_combo(layer_sound);

			if (file_combo.get_active_id() == item_audio_file_id) {
				const std::string guid = layer->get_guid().get_string();
				file_combo.set_active_id(guid);
			}
		});

		canvas_interface->signal_layer_removed().connect([&](synfig::Layer::Handle layer) {
			etl::handle<synfig::Layer_Sound> layer_sound = etl::handle<synfig::Layer_Sound>::cast_dynamic(layer);
			if (!layer_sound)
				return;
			std::string guid = layer_sound->get_guid().get_string();
			bool found = false;
			int index = -1;
			file_combo.get_model()->foreach_iter([guid, &found, &index](const Gtk::TreeModel::iterator& iter) -> bool {
				index++;
				Glib::ustring row_guid;
				iter->get_value(1, row_guid);
				if (row_guid == guid) {
					found = true;
					return true;
				}
				return false;
			});
			if (found) {
				const int active_index = file_combo.get_active_row_number();
				if (index == active_index)
					file_combo.set_active_id(item_no_audio_id);

				file_combo.remove_text(index);
				layer_map.erase(layer->get_guid().get_string());
			} else
				synfig::warning(_("Couldn't remove deleted layer sound from Sound Panel list"));
		});

		canvas_interface->signal_layer_param_changed().connect([&](synfig::Layer::Handle layer, std::string param_name) {
			if (param_name != "filename" && param_name != "delay")
				return;
			etl::handle<synfig::Layer_Sound> layer_sound = etl::handle<synfig::Layer_Sound>::cast_dynamic(layer);
			if (!layer_sound)
				return;
			Gtk::TreeModel::iterator found_iter = find_layer_iter(layer_sound);
			if (found_iter) {
				std::string guid = layer_sound->get_guid().get_string();
				if (param_name == "filename") {
					std::string filename = layer_sound->get_param("filename").get(std::string());
					found_iter->set_value(0, create_layer_item_label(layer_sound));
					if (guid == file_combo.get_active_id()) {
						load_sound_file(filename);
					}
				} else if (param_name == "delay") {
					if (guid == file_combo.get_active_id()) {
						synfig::Time delay = layer_sound->get_param("delay").get(synfig::Time::zero());
						delay_widget.set_value(delay);
						delay_widget.activate();
					}
				}
			}
			else
				synfig::warning(_("Couldn't set new layer sound parameter values to Sound Panel list"));
		});

		canvas_interface->signal_layer_new_description().connect([&](synfig::Layer::Handle layer, std::string new_description) {
			etl::handle<synfig::Layer_Sound> layer_sound = etl::handle<synfig::Layer_Sound>::cast_dynamic(layer);
			if (!layer_sound)
				return;
			Gtk::TreeModel::iterator found_iter = find_layer_iter(layer_sound);
			if (found_iter) {
				found_iter->set_value(0, create_layer_item_label(layer_sound));
			}
		});
	}

	void setup_file_setting_data()
	{
		channel_combo.remove_all();
		for (int n = 0; n < widget_sound.get_channel_number(); n++) {
			// let us be a bit user-friendly by starting index from 1 instead of 0
			std::string text = etl::strprintf(_("Channel #%i"), n+1);
			channel_combo.append(std::to_string(n), text);
		}
		channel_combo.set_active_id(std::to_string(widget_sound.get_channel_idx()));

		delay_widget.set_fps(widget_sound.get_time_model()->get_frame_rate());
		delay_widget.set_value(widget_sound.get_delay());
	}

	static std::string create_layer_item_label(etl::loose_handle<synfig::Layer_Sound> layer_sound) {
		const std::string sound_filename = layer_sound->get_param("filename").get(std::string());
		std::string short_filename = synfig::CanvasFileNaming::make_short_filename(layer_sound->get_canvas()->get_file_name(), sound_filename);
		const std::string layer_name = layer_sound->get_description();

		// Ellipsize long "short_filename"...
		const int max_filename_length = 30;
		if (short_filename.length() > max_filename_length) {
			const std::string ellipsis = "...";
			short_filename = ellipsis +
					short_filename.substr(short_filename.length() - (max_filename_length-ellipsis.length()));
		}
		return etl::strprintf("[%s] %s", layer_name.c_str(), short_filename.c_str());
	}

	bool import_file(const std::string &filename) {
		std::string errors, warnings;

		bool ok = canvas_interface->import(filename, errors, warnings);

		if (!errors.empty())
			App::dialog_message_1b(
				"ERROR",
				etl::strprintf("%s:\n\n%s", _("Error"), errors.c_str()),
				"details",
				_("Close"));
		if (!warnings.empty())
			App::dialog_message_1b(
				"WARNING",
				etl::strprintf("%s:\n\n%s", _("Warning"), warnings.c_str()),
				"details",
				_("Close"));
		if (!ok) {
			synfig::warning("Could not load audio: %s", filename.c_str());
		}
		return ok;
	}

	void clear_audio()
	{
		std::lock_guard<std::mutex> lock(mutex);
		file_combo.set_active_text(item_no_audio_str);
		widget_sound.clear();
		file_settings_box.hide();
	}

	void on_file_combo_changed() {
		std::string guid = file_combo.get_active_id();
		if (guid == item_no_audio_id) {
			clear_audio();
		} else if (guid == item_audio_file_id) {
			if (open_dialog_disabled)
				return;
			std::string filename("*.*");
			if (App::dialog_open_file_audio(_("Please choose an audio file"), filename, ANIMATION_DIR_PREFERENCE))
			{
				import_file(filename);
			} else {
				// User canceled Open File dialog
				file_combo.set_active_id(item_no_audio_id);
			}
		} else {
			const auto & layer_sound = layer_map[guid];
			std::string filename = layer_sound->get_param("filename").get(std::string());
			if (canvas_interface) {
				synfig::Canvas::LooseHandle canvas = canvas_interface->get_canvas();
				if (canvas) {
					filename = synfig::CanvasFileNaming::make_full_filename(canvas->get_file_name(), filename);
					filename = canvas->get_file_system()->get_real_uri(filename);
					filename = Glib::filename_from_uri(filename);
				}
			}

			const synfig::Time delay = layer_sound->get_param("delay").get(synfig::Time::zero());
			load_sound_file(filename, delay);
		}
	}

	void on_channel_combo_changed()
	{
		std::string channel_string = channel_combo.get_active_id();
		if (channel_string.empty())
			return;
		int channel_idx = std::stoi(channel_string);
		widget_sound.set_channel_idx(channel_idx);
	}

	void on_delay_changed()
	{
		std::string guid = file_combo.get_active_id();
		if (auto sound_layer = layer_map[guid]) {
			bool ok = canvas_interface->change_value(synfigapp::ValueDesc(synfig::Layer::LooseHandle(sound_layer), std::string("delay")), synfig::ValueBase(delay_widget.get_value()));
			if (ok)
				widget_sound.set_delay(delay_widget.get_value());
		}
	}

	bool load_sound_file(const std::string& filename, const synfig::Time &delay = synfig::Time::zero())
	{
		file_combo.set_sensitive(false);
		std::lock_guard<std::mutex> lock(mutex);
		bool ok = false;
		try {
			ok = widget_sound.load(filename);
			if (ok) {
				widget_sound.set_delay(delay);
				setup_file_setting_data();
				file_settings_box.show();
			} else {
				synfig::warning(_("Audio file not supported: %s"), filename.c_str());
			}
		} catch (const std::string & a) {
			synfig::error(_("Error loading audio file: %s\n\t%s"), filename.c_str(), a.c_str());
		} catch (...) {
			synfig::error(_("Error loading audio file: %s\n\tReason unknown"), filename.c_str());
		}

		file_combo.set_sensitive(true);
		return ok;
	}

	void fill_sound_layer_combo(synfig::Canvas::LooseHandle canvas)
	{
		std::vector<etl::handle<synfig::Layer_Sound>> layers;
		fetch_sound_layers(canvas, layers);
		for (auto &layer : layers) {
			add_layer_to_combo(layer);
		}
	}

	void add_layer_to_combo(etl::loose_handle<synfig::Layer_Sound> layer)
	{
		file_combo.append(layer->get_guid().get_string(), create_layer_item_label(layer));
		layer_map[layer->get_guid().get_string()] = layer;
	}

	static void fetch_sound_layers(synfig::Canvas::LooseHandle canvas, std::vector<etl::handle<synfig::Layer_Sound>> &layers)
	{
		for (auto layer_it = canvas->begin(); layer_it != canvas->end(); ++layer_it ) {
			etl::handle<synfig::Layer_Sound> layer_sound = etl::handle<synfig::Layer_Sound>::cast_dynamic(*layer_it);
			if (layer_sound) {
				layers.push_back(layer_sound);
			}
		}
		if (!canvas->children().empty()) {
			for (auto inner_canvas : canvas->children()) {
				fetch_sound_layers(inner_canvas, layers);
			}
		}
	}

	Gtk::TreeModel::iterator find_layer_iter(synfig::Layer::Handle layer)
	{
		Gtk::TreeModel::iterator found_iter;
		etl::handle<synfig::Layer_Sound> layer_sound = etl::handle<synfig::Layer_Sound>::cast_dynamic(layer);
		if (!layer_sound)
			return found_iter;

		std::string guid = layer_sound->get_guid().get_string();
		file_combo.get_model()->foreach_iter([guid, &found_iter](const Gtk::TreeModel::iterator& iter) -> bool {
			Glib::ustring row_guid;
			iter->get_value(1, row_guid);
			if (row_guid == guid) {
				found_iter = iter;
				return true;
			}
			return false;
		});

		return found_iter;
	}
};

}

const std::string studio::Grid_SoundWave::item_no_audio_id = "-no-audio-";
const std::string studio::Grid_SoundWave::item_no_audio_str = _("No audio");
const std::string studio::Grid_SoundWave::item_audio_file_id = "-audio-file-";
const std::string studio::Grid_SoundWave::item_audio_file_str = _("Select an audio file");


Dock_SoundWave::Dock_SoundWave()
	: Dock_CanvasSpecific("soundwave", _("Sound"), Gtk::StockID("synfig-layer_other_sound")),
	  current_grid_sound(nullptr)
{
	// Make Sound toolbar buttons small for space efficiency
	auto context = get_style_context();
	context->add_class("synfigstudio-efficient-workspace");

	set_use_scrolled(false);

	widget_kf_list.set_hexpand();
	widget_kf_list.show();
	widget_timeslider.set_hexpand();
	widget_timeslider.show();

//	file_button.signal_file_set().connect(sigc::mem_fun(*this, &Dock_SoundWave::on_file_button_clicked));
//	Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
//	filter->add_mime_type("audio/*");
//	file_button.set_filter(filter);

	vscrollbar.set_vexpand();
	vscrollbar.set_hexpand(false);
	vscrollbar.show();
	hscrollbar.set_hexpand();
	hscrollbar.show();

	grid.set_column_homogeneous(false);
	grid.set_row_homogeneous(false);
	add(grid);

	drag_dest_set(Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_COPY);
	signal_drag_data_received().connect(sigc::mem_fun(*this,
				&Dock_SoundWave::on_drop_drag_data_received) );
}

void Dock_SoundWave::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	Grid_SoundWave *grid_sound = new Grid_SoundWave(canvas_view);
	grid_sound->show();
	canvas_view->set_ext_widget(get_name(), grid_sound);
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

		current_grid_sound = nullptr; // deleted by its studio::CanvasView::~CanvasView()

		hscrollbar.unset_adjustment();
		vscrollbar.unset_adjustment();

		std::vector< Gtk::TargetEntry > targets;
		drag_dest_set(targets);
	} else {
		widget_kf_list.set_time_model(canvas_view->time_model());
		widget_kf_list.set_canvas_interface(canvas_view->canvas_interface());

		widget_timeslider.set_canvas_view(canvas_view);

		current_grid_sound = dynamic_cast<Grid_SoundWave*>( canvas_view->get_ext_widget(get_name()) );

		vscrollbar.set_adjustment(current_grid_sound->get_widget_sound().get_range_adjustment());
		hscrollbar.set_adjustment(canvas_view->time_model()->scroll_time_adjustment());

		grid.attach(widget_kf_list,        0, 0, 1, 1);
		grid.attach(widget_timeslider,     0, 1, 1, 1);
		grid.attach(*current_grid_sound,   0, 2, 1, 1);
		grid.attach(hscrollbar,            0, 3, 2, 1);
		grid.attach(vscrollbar,            1, 0, 1, 4);
		grid.show();

		current_grid_sound->grab_focus();

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
			s = synfig::right_trim(s);
			uris.push_back(s);
		}

		if (current_grid_sound) {
			std::string failed_uris;

			for (const std::string &uri : uris) {
				std::string filename = Glib::filename_from_uri(uri);
				if (!current_grid_sound->append_and_set_audio(filename))
					failed_uris += filename + "\n";
			}

			if (failed_uris.empty()) {
				success = true;
			} else {
				App::dialog_message_1b(
							"WARNING",
							etl::strprintf("%s:\n\n%s\n%s",
										   _("Warning"), _("The following files could not be imported:"), failed_uris.c_str()),
							"details",
							_("Close"));
			}
		}
	}
	context->drag_finish(success, false, time);
}

