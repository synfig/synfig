/* === S Y N F I G ========================================================= */
/*!	\file dialog_input.cpp
**	\brief Input dialog implementation
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <vector>

#include <gtk/gtk.h>

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/separator.h>

#include "dialog_input.h"
#include <synfigapp/main.h>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

struct Dialog_Input::DeviceOptions {
	struct Axis
	{
		GdkAxisUse use;
		Axis(): use() { }
	};

	typedef std::vector<Axis> AxisList;

	struct Key
	{
		guint val;
		GdkModifierType modifiers;
		Key(): val(), modifiers() { }
	};

	typedef std::vector<Key> KeyList;

	struct Device
	{
		GdkDevice *handle;
		std::string name;
		GdkInputMode mode;
		AxisList axes;
		KeyList keys;
		Device(): handle(), mode() { }
	};

	typedef std::vector<Device> DeviceList;

	DeviceList devices;

	void on_mode_comboboxtext_changed(Gtk::ComboBoxText *comboboxtext, Device *device)
	{
		int i = comboboxtext->get_active_row_number();
		switch(i) {
		case GDK_MODE_SCREEN:
		case GDK_MODE_WINDOW:
			device->mode = (GdkInputMode)i;
			break;
		default:
			device->mode = GDK_MODE_DISABLED;
			break;
		}
	}
};


Dialog_Input::Dialog_Input(Gtk::Window& parent):
	Gtk::Dialog(_("Input Dialog"), parent),
	dialog_settings(this, "input"),
	options(new DeviceOptions()),
	scrolled_window(NULL)
{
	set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);
	add_button(_("OK"), Gtk::RESPONSE_OK);
	add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
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

	static const GdkDeviceType device_types[] =
	{
		GDK_DEVICE_TYPE_MASTER,
		GDK_DEVICE_TYPE_SLAVE,
		GDK_DEVICE_TYPE_FLOATING
	};

	for(int i = 0; i < (int)(sizeof(device_types)/sizeof(device_types[0])); ++i)
	{
		GList *device_list = gdk_device_manager_list_devices(
			gdk_display_get_device_manager(
				gdk_display_manager_get_default_display(
					gdk_display_manager_get() )),
			device_types[i] );

		for(GList *itr=device_list; itr; itr=g_list_next(itr))
		{
			GdkDevice * gdk_device = reinterpret_cast<GdkDevice*>(itr->data);

			if (!synfigapp::Main::find_input_device(gdk_device_get_name(gdk_device)))
				continue;

			options->devices.push_back(DeviceOptions::Device());
			DeviceOptions::Device &device_options = options->devices.back();

			device_options.handle = gdk_device;
			device_options.name = gdk_device_get_name(gdk_device);

			// allow to select device mode
			device_options.mode = gdk_device_get_mode(gdk_device);

			// allow to select device axis usage
			device_options.axes.resize( gdk_device_get_n_axes(gdk_device) );
			for(int j = 0; j < (int)device_options.axes.size(); ++j)
				device_options.axes[j].use = gdk_device_get_axis_use(gdk_device, j);

			// allow to select device keys
			device_options.keys.resize( gdk_device_get_n_keys(gdk_device) );
			for(int j = 0; j < (int)device_options.keys.size(); ++j)
				gdk_device_get_key(gdk_device, j, &device_options.keys[j].val, &device_options.keys[j].modifiers);
		}

		g_list_free(device_list);
	}
}

void Dialog_Input::create_widgets()
{
	if (scrolled_window != NULL)
	{
		get_content_area()->remove(*scrolled_window);
		scrolled_window = NULL;
	}

	// Devices
	if (!options->devices.empty())
	{
		scrolled_window = Gtk::manage(new Gtk::ScrolledWindow());
		Gtk::Table *table = Gtk::manage(new Gtk::Table((int)options->devices.size() + 1, 2));

		for(DeviceOptions::DeviceList::iterator i = options->devices.begin(); i != options->devices.end(); ++i)
		{
			int row = i - options->devices.begin();

			Gtk::Label *label = Gtk::manage(new Gtk::Label(i->name));
			label->show();
			table->attach(*label, 0, 1, row, row+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK | Gtk::FILL);

			Gtk::ComboBoxText *comboboxtext = Gtk::manage(new Gtk::ComboBoxText());
			comboboxtext->append(_("Disabled"));
			comboboxtext->append(_("Screen"));
			comboboxtext->append(_("Window"));
			comboboxtext->set_active(i->mode);
			comboboxtext->signal_changed().connect(
				sigc::bind(
					sigc::mem_fun(options, &DeviceOptions::on_mode_comboboxtext_changed),
					comboboxtext, &*i ));
			comboboxtext->show();
			table->attach(*comboboxtext, 1, 2, row, row+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK | Gtk::FILL);
		}

		table->attach( *manage(new class Gtk::HSeparator()),
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
	for(DeviceOptions::DeviceList::const_iterator i = options->devices.begin(); i != options->devices.end(); ++i)
	{
		gdk_device_set_mode(i->handle, i->mode);
		//for(DeviceOptions::AxisList::const_iterator j = i->axes.begin(); j != i->axes.end(); ++j)
		//	gdk_device_set_axis_use(i->handle, j - i->axes.begin(), j->use);
		//for(DeviceOptions::KeyList::const_iterator j = i->keys.begin(); j != i->keys.end(); ++j)
		//	gdk_device_set_key(i->handle, j - i->keys.begin(), j->val, j->modifiers);
	}
	signal_apply()();
}

void Dialog_Input::apply_and_hide()
{
	apply();
	hide();
}
