/* === S Y N F I G ========================================================= */
/*!	\file dialog_input.cpp
**	\brief Input dialog implementation
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#include <gui/dialogs/dialog_input.h>

#include <gdkmm/device.h>

#include <gtkmm/comboboxtext.h>
#include <gtkmm/label.h>
#include <gtkmm/separator.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>

#include <gui/devicetracker.h>
#include <gui/localization.h>

#include <synfigapp/main.h>

#include <vector>
#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace synfigapp;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

struct Dialog_Input::DeviceOptions {
	struct Axis {
		Gdk::AxisUse use;
		Axis(): use(Gdk::AXIS_IGNORE) { }
	};
	typedef std::vector<Axis> AxisList;

	struct Key {
		guint val;
		Gdk::ModifierType modifiers;
		Key(): val(), modifiers() { }
	};
	typedef std::vector<Key> KeyList;

	struct Device {
		Glib::RefPtr<Gdk::Device> device;
		std::string name;
		Gdk::InputMode mode;
		AxisList axes;
		KeyList keys;
		Device(): mode(Gdk::MODE_DISABLED) { }
	};
	typedef std::vector<Device> DeviceList;

	DeviceList devices;

	void on_mode_comboboxtext_changed(Gtk::ComboBoxText *comboboxtext, int device_index) {
		if (device_index < 0 || device_index >= (int)devices.size()) return;
		Device &device = devices[device_index];
		std::string id = comboboxtext->get_active_id();
		device.mode = id == "screen" ? Gdk::MODE_SCREEN
				    : id == "window" ? Gdk::MODE_WINDOW
				    :                  Gdk::MODE_DISABLED;
	}
};


Dialog_Input::Dialog_Input(Gtk::Window& parent):
	Gtk::Dialog(_("Input Dialog"), parent),
	dialog_settings(this, "input"),
	options(new DeviceOptions()),
	scrolled_window(NULL)
{
	set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);
	add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
	add_button(_("OK"), Gtk::RESPONSE_OK);
	reset();
}

Dialog_Input::~Dialog_Input()
{
	delete options;
}

void Dialog_Input::on_response(int id)
{
	if (id == Gtk::RESPONSE_OK) apply_and_hide(); else
	if (id == Gtk::RESPONSE_CANCEL) hide();
}

void Dialog_Input::take_options()
{
	options->devices.clear();

	DeviceList devices;
	DeviceTracker::list_devices(devices);
	for(DeviceList::const_iterator i = devices.begin(); i != devices.end(); ++i) {
		const Glib::RefPtr<Gdk::Device> &device = *i;

		if (!synfigapp::Main::find_input_device(device->get_name()))
			continue;

		options->devices.push_back(DeviceOptions::Device());
		DeviceOptions::Device &device_options = options->devices.back();

		device_options.device = device;
		device_options.name = device->get_name();

		// allow to select device mode
		device_options.mode = device->get_mode();

		// allow to select device axis usage (ignore axes of keyboard devices, why?)
		device_options.axes.resize(device->get_source() == Gdk::SOURCE_KEYBOARD ? 0 : device->get_n_axes());
		for(int j = 0; j < (int)device_options.axes.size(); ++j)
			device_options.axes[j].use = device->get_axis_use(j);

		// allow to select device keys
		device_options.keys.resize( device->get_n_keys() );
		for(int j = 0; j < (int)device_options.keys.size(); ++j)
			device->get_key(j, device_options.keys[j].val, device_options.keys[j].modifiers);
	}
}

void Dialog_Input::create_widgets()
{
	if (scrolled_window != NULL) {
		get_content_area()->remove(*scrolled_window);
		scrolled_window = NULL;
	}

	// Devices
	if (!options->devices.empty()) {
		scrolled_window = Gtk::manage(new Gtk::ScrolledWindow());
		Gtk::Table *table = Gtk::manage(new Gtk::Table((int)options->devices.size() + 1, 2));

		for(DeviceOptions::DeviceList::iterator i = options->devices.begin(); i != options->devices.end(); ++i)
		{
			int row = i - options->devices.begin();

			Gtk::Label *label = Gtk::manage(new Gtk::Label(i->name));
			label->show();
			table->attach(*label, 0, 1, row, row+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK | Gtk::FILL);

			Gtk::ComboBoxText *comboboxtext = Gtk::manage(new Gtk::ComboBoxText());
			comboboxtext->append("disabled", _("Disabled"));
			comboboxtext->append("screen", _("Screen"));
			comboboxtext->append("window", _("Window"));
			switch(i->mode) {
			case Gdk::MODE_SCREEN: comboboxtext->set_active(1); break;
			case Gdk::MODE_WINDOW: comboboxtext->set_active(2); break;
			default:               comboboxtext->set_active(0); break;
			}
			comboboxtext->signal_changed().connect(
				sigc::bind(
					sigc::mem_fun(options, &DeviceOptions::on_mode_comboboxtext_changed),
					comboboxtext, options->devices.begin() - i ));
			comboboxtext->show();
			table->attach(*comboboxtext, 1, 2, row, row+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK | Gtk::FILL);
		}

		table->attach( *manage(new class Gtk::Separator()),
					   0,
					   2,
					   (int)options->devices.size(),
					   (int)options->devices.size()+1,
					   Gtk::EXPAND | Gtk::FILL,
					   Gtk::EXPAND | Gtk::FILL );
		table->show();

		scrolled_window->add(*table);
		scrolled_window->show();
		get_content_area()->pack_end(*scrolled_window);
	}
}

void Dialog_Input::reset()
{
	take_options();
	create_widgets();
}

void Dialog_Input::apply()
{
	for(DeviceOptions::DeviceList::const_iterator i = options->devices.begin(); i != options->devices.end(); ++i) {
		i->device->set_mode(i->mode);
		//for(DeviceOptions::AxisList::const_iterator j = i->axes.begin(); j != i->axes.end(); ++j)
		//	i->device->set_axis_use(j - i->axes.begin(), j->use);
		//for(DeviceOptions::KeyList::const_iterator j = i->keys.begin(); j != i->keys.end(); ++j)
		//	i->device->set_key(j - i->keys.begin(), j->val, j->modifiers);
	}
	signal_apply()();
}

void Dialog_Input::apply_and_hide()
{
	apply();
	hide();
}
