/* === S Y N F I G ========================================================= */
/*!	\file gui/json_to_dialog_converter.cpp
**	\brief Fetch dialog data as JSON object or fill the dialog control widgets with JSON input
**
**	\legal
**	Copyright (c) 2023-2025 Synfig Contributors
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
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "json_to_dialog_converter.h"

#include <gtkmm/appchooserbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/scalebutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/switch.h>
#include <gtkmm/volumebutton.h>

#endif

/* === U S I N G =========================================================== */

using namespace JSON;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static void
fetch_data_in_widget(const Gtk::Widget* w, std::map<std::string, std::string>& data)
{
	if (!w)
		return;

	if (!w->get_name().empty()) {
		if (GTK_IS_COMBO_BOX_TEXT(w->gobj())) {
			const Gtk::ComboBoxText* combo = static_cast<const Gtk::ComboBoxText*>(w);
			if (combo->get_has_entry())
				data[w->get_name()] = combo->get_entry_text();
			else
				data[w->get_name()] = combo->get_active_id();
		} else if (GTK_IS_COMBO_BOX(w->gobj())) {
			const Gtk::ComboBox* combo = static_cast<const Gtk::ComboBox*>(w);
			if (combo->get_has_entry())
				data[w->get_name()] = combo->get_entry_text();
			else
				data[w->get_name()] = combo->get_active_id();
		} else if (GTK_IS_SWITCH(w->gobj())) {
			data[w->get_name()] = std::to_string(static_cast<const Gtk::Switch*>(w)->get_active());
		// } else if (GTK_IS_RADIO_BUTTON(w->gobj())) {
		// 	data[w->get_name()] = std::to_string(static_cast<const Gtk::RadioButton*>(w)->get_());
		} else if (GTK_IS_CHECK_BUTTON(w->gobj())) {
			data[w->get_name()] = std::to_string(static_cast<const Gtk::CheckButton*>(w)->get_active());
		} else if (GTK_IS_TOGGLE_BUTTON(w->gobj())) {
			data[w->get_name()] = std::to_string(static_cast<const Gtk::ToggleButton*>(w)->get_active());
		} else if (GTK_IS_FILE_CHOOSER_BUTTON(w->gobj())) {
			data[w->get_name()] = static_cast<const Gtk::FileChooserButton*>(w)->get_filename();
		} else if (GTK_IS_COLOR_BUTTON(w->gobj())) {
			data[w->get_name()] = static_cast<const Gtk::ColorButton*>(w)->get_rgba().to_string();
		} else if (GTK_IS_FONT_BUTTON(w->gobj())) {
			// https://docs.gtk.org/Pango/type_func.FontDescription.from_string.html
			const auto* font_button = static_cast<const Gtk::FontButton*>(w);
			data[w->get_name()] = font_button->get_font_name();
		} else if (GTK_IS_SCALE_BUTTON(w->gobj())) {
			data[w->get_name()] = std::to_string(static_cast<const Gtk::ScaleButton*>(w)->get_value());
		} else if (GTK_IS_VOLUME_BUTTON(w->gobj())) {
			data[w->get_name()] = std::to_string(static_cast<const Gtk::VolumeButton*>(w)->get_value());
		} else if (GTK_IS_APP_CHOOSER_BUTTON(w->gobj())) {
			data[w->get_name()] = static_cast<const Gtk::AppChooserButton*>(w)->get_app_info()->get_commandline();
		} else if (GTK_IS_SCALE(w->gobj())) {
			data[w->get_name()] = std::to_string(static_cast<const Gtk::Scale*>(w)->get_value());
		} else if (GTK_IS_SPIN_BUTTON(w->gobj())) {
			data[w->get_name()] = std::to_string(static_cast<const Gtk::SpinButton*>(w)->get_value());
		} else if (GTK_IS_ENTRY(w->gobj())) {
			data[w->get_name()] = static_cast<const Gtk::Entry*>(w)->get_text();
		}
	}
	if (GTK_IS_CONTAINER(w->gobj())) {
		for (const Gtk::Widget* c : static_cast<const Gtk::Container*>(w)->get_children())
			fetch_data_in_widget(c, data);
	}
};

std::map<std::string, std::string>
JSON::parse_dialog(const Gtk::Widget& dialog_contents)
{
	std::map<std::string, std::string> data;
	fetch_data_in_widget(&dialog_contents, data);
	return data;
}

void
JSON::set_widget_value(Gtk::Widget* widget, const std::string& value)
{
	if (!widget)
		return;

	if (GTK_IS_COMBO_BOX_TEXT(widget->gobj())) {
		auto* combo = static_cast<Gtk::ComboBoxText*>(widget);
		if (combo->get_has_entry()) {
			if (auto entry = combo->get_entry()) {
				entry->set_text(value);
			}
		} else {
			combo->set_active_id(value);
		}
	} else if (GTK_IS_COMBO_BOX(widget->gobj())) {
		auto* combo = static_cast<Gtk::ComboBox*>(widget);
		if (combo->get_has_entry()) {
			if (auto entry = combo->get_entry()) {
				entry->set_text(value);
				entry->set_editable(true);
			}
		} else {
			combo->set_active_id(value);
		}
	} else if (GTK_IS_SWITCH(widget->gobj())) {
		static_cast<Gtk::Switch*>(widget)->set_active(value == "1" || value == "true");
	} else if (GTK_IS_CHECK_BUTTON(widget->gobj())) {
		static_cast<Gtk::CheckButton*>(widget)->set_active(value == "1" || value == "true");
	} else if (GTK_IS_TOGGLE_BUTTON(widget->gobj())) {
		static_cast<Gtk::ToggleButton*>(widget)->set_active(value == "1" || value == "true");
	} else if (GTK_IS_FILE_CHOOSER_BUTTON(widget->gobj())) {
		static_cast<Gtk::FileChooserButton*>(widget)->set_filename(value);
	} else if (GTK_IS_COLOR_BUTTON(widget->gobj())) {
		Gdk::RGBA color;
		color.set(value);
		static_cast<Gtk::ColorButton*>(widget)->set_rgba(color);
	} else if (GTK_IS_FONT_BUTTON(widget->gobj())) {
		static_cast<Gtk::FontButton*>(widget)->set_font_name(value);
	} else if (GTK_IS_SCALE_BUTTON(widget->gobj())) {
		static_cast<Gtk::ScaleButton*>(widget)->set_value(std::stod(value));
	} else if (GTK_IS_VOLUME_BUTTON(widget->gobj())) {
		static_cast<Gtk::VolumeButton*>(widget)->set_value(std::stod(value));
	} else if (GTK_IS_SCALE(widget->gobj())) {
		static_cast<Gtk::Scale*>(widget)->set_value(std::stod(value));
	} else if (GTK_IS_SPIN_BUTTON(widget->gobj())) {
		static_cast<Gtk::SpinButton*>(widget)->set_value(std::stod(value));
	} else if (GTK_IS_ENTRY(widget->gobj())) {
		static_cast<Gtk::Entry*>(widget)->set_text(value);
	}
}

void
JSON::hydrate_dialog(Gtk::Container* container, const std::map<std::string, std::string>& values)
{
	if (!container)
		return;

	// Process all children

	std::vector<Gtk::Widget*> children = container->get_children();
	for (Gtk::Widget* child : children) {
		if (!child->get_name().empty()) {
			auto it = values.find(child->get_name());
			if (it != values.end())
				set_widget_value(child, it->second);
		}

		// Recursively process child containers
		if (GTK_IS_CONTAINER(child->gobj()))
			hydrate_dialog(static_cast<Gtk::Container*>(child), values);
	}
}

/* === M E T H O D S ======================================================= */
