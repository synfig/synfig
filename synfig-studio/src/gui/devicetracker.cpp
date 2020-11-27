/* === S Y N F I G ========================================================= */
/*!	\file devicetracker.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2009 Gerco Ballintijn
**  ......... ... 2018 Ivan Mahonin
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

#include <gdkmm/device.h>
#include <gdkmm/displaymanager.h>

#if GDKMM_MAJOR_VERSION < 3 || (GDKMM_MAJOR_VERSION == 3 && GDKMM_MINOR_VERSION < 20)
	#define OLD_GDKMM_DEVICE_FUNCTIONALITY
	#include <gdkmm/devicemanager.h>
#else
	#include <gdkmm/seat.h>
#endif

#include <synfigapp/main.h>

#include "devicetracker.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfigapp;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
DeviceTracker::list_devices(DeviceList &out_devices)
{
	out_devices.clear();

#ifdef OLD_GDKMM_DEVICE_FUNCTIONALITY
	Gdk::DeviceType types[] = {
		Gdk::DEVICE_TYPE_MASTER,
		Gdk::DEVICE_TYPE_SLAVE,
		Gdk::DEVICE_TYPE_FLOATING };
	int count = (int)(sizeof(types)/sizeof(types[0]));
	for(int i = 0; i < count; ++i) {
		DeviceList list =
			Gdk::DisplayManager::get()->
				get_default_display()->
					get_device_manager()->
						list_devices(types[i]);
		out_devices.insert(out_devices.end(), list.begin(), list.end());
	}
#else
	Glib::RefPtr<Gdk::Seat> seat = Gdk::DisplayManager::get()->get_default_display()->get_default_seat();
	if (seat->get_keyboard())
		out_devices.push_back(seat->get_keyboard());
	if (seat->get_pointer())
		out_devices.push_back(seat->get_pointer());
	DeviceList slaves = seat->get_slaves(Gdk::SEAT_CAPABILITY_ALL);
	out_devices.reserve(out_devices.size() + slaves.size());
	out_devices.insert(out_devices.end(), slaves.begin(), slaves.end());
#endif
}

DeviceTracker::DeviceTracker()
{
	DeviceList devices;
	list_devices(devices);
	for(DeviceList::const_iterator i = devices.begin(); i != devices.end(); ++i) {
		const Glib::RefPtr<Gdk::Device> &device = *i;

		bool unknown_type = false;
		InputDevice::Type type = InputDevice::TYPE_MOUSE;
		switch(device->get_source()) {
		case Gdk::SOURCE_MOUSE:  type = InputDevice::TYPE_MOUSE;  break;
		case Gdk::SOURCE_PEN:    type = InputDevice::TYPE_PEN;    break;
		case Gdk::SOURCE_ERASER: type = InputDevice::TYPE_ERASER; break;
		case Gdk::SOURCE_CURSOR: type = InputDevice::TYPE_CURSOR; break;
		default: unknown_type = true; break;
		}

		InputDevice::Handle input_device;
		input_device = synfigapp::Main::add_input_device(device->get_name(), type);

		// Disable all extended devices by default.
		// This tries to fix several bugs reported in track and forums
		if (!unknown_type) {
			input_device->set_mode(InputDevice::MODE_DISABLED);
			//synfigapp::Main::select_input_device(input_device);
		}
	}

	// Once all devices are disabled be sure that the core pointer is the
	// one selected. The user should decide later whether enable and save the
	// rest of input devices found.
#ifdef OLD_GDKMM_DEVICE_FUNCTIONALITY
	Glib::RefPtr<Gdk::DeviceManager> manager = Gdk::DisplayManager::get()->get_default_display()->get_device_manager();
	if (manager && manager->get_client_pointer())
		synfigapp::Main::select_input_device(manager->get_client_pointer()->get_name());
#else
	Glib::RefPtr<Gdk::Seat> seat = Gdk::DisplayManager::get()->get_default_display()->get_default_seat();
	if (seat->get_pointer())
		synfigapp::Main::select_input_device(seat->get_pointer()->get_name());
#endif
}

DeviceTracker::~DeviceTracker()
	{ }

void
DeviceTracker::save_preferences()
{
	DeviceList devices;
	list_devices(devices);
	for(DeviceList::const_iterator i = devices.begin(); i != devices.end(); ++i) {
		const Glib::RefPtr<Gdk::Device> &device = *i;

		InputDevice::Handle synfig_device = synfigapp::Main::find_input_device(device->get_name());
		if (!synfig_device)
			continue;

		InputDevice::Mode mode = InputDevice::MODE_DISABLED;
		switch(device->get_mode()) {
		case Gdk::MODE_SCREEN: mode = InputDevice::MODE_SCREEN;  break;
		case Gdk::MODE_WINDOW: mode = InputDevice::MODE_WINDOW;  break;
		default: break;
		}
		synfig_device->set_mode(mode);

		int n_axes = device->get_source() == Gdk::SOURCE_KEYBOARD ? 0 : device->get_n_axes();
		if (n_axes > 0) {
			std::vector<InputDevice::AxisUse> axes(n_axes);
			for(int j = 0; j < n_axes; ++j)
				switch(device->get_axis_use(j)) {
				case Gdk::AXIS_X:        axes[j] = InputDevice::AXIS_X; break;
				case Gdk::AXIS_Y:        axes[j] = InputDevice::AXIS_Y; break;
				case Gdk::AXIS_PRESSURE: axes[j] = InputDevice::AXIS_PRESSURE; break;
				case Gdk::AXIS_XTILT:    axes[j] = InputDevice::AXIS_XTILT; break;
				case Gdk::AXIS_YTILT:    axes[j] = InputDevice::AXIS_YTILT; break;
				case Gdk::AXIS_WHEEL:    axes[j] = InputDevice::AXIS_WHEEL; break;
				case Gdk::AXIS_LAST:     axes[j] = InputDevice::AXIS_LAST; break;
				default:                 axes[j] = InputDevice::AXIS_IGNORE; break;
				}
			synfig_device->set_axes(axes);
		}

		int n_keys = device->get_n_keys();
		if (n_keys > 0) {
			std::vector<InputDevice::DeviceKey> keys(n_keys);
			for(int j = 0; j < n_keys; ++j) {
				guint gdk_keyval = 0;
				Gdk::ModifierType gdk_modifiers = Gdk::ModifierType();
				device->get_key(j, gdk_keyval, gdk_modifiers);
				keys[j].keyval = gdk_keyval;
				keys[j].modifiers = gdk_modifiers;
			}
			synfig_device->set_keys(keys);
		}
	}
}

void
DeviceTracker::load_preferences()
{
	DeviceList devices;
	list_devices(devices);
	for(DeviceList::const_iterator i = devices.begin(); i != devices.end(); ++i) {
		const Glib::RefPtr<Gdk::Device> &device = *i;

		InputDevice::Handle synfig_device = synfigapp::Main::find_input_device(device->get_name());
		if (!synfig_device)
			continue;

		Gdk::InputMode gdk_mode = Gdk::MODE_DISABLED;
		switch(synfig_device->get_mode()) {
		case InputDevice::MODE_SCREEN: gdk_mode = Gdk::MODE_SCREEN;  break;
		case InputDevice::MODE_WINDOW: gdk_mode = Gdk::MODE_WINDOW;  break;
		default: break;
		}
		device->set_mode(gdk_mode);

		const std::vector<InputDevice::AxisUse> axes = synfig_device->get_axes();
		for(int axis = 0; axis < (int)axes.size(); ++axis)
			switch(axes[axis]) {
			case InputDevice::AXIS_X:        device->set_axis_use(axis, Gdk::AXIS_X);        break;
			case InputDevice::AXIS_Y:        device->set_axis_use(axis, Gdk::AXIS_Y);        break;
			case InputDevice::AXIS_PRESSURE: device->set_axis_use(axis, Gdk::AXIS_PRESSURE); break;
			case InputDevice::AXIS_XTILT:    device->set_axis_use(axis, Gdk::AXIS_XTILT);    break;
			case InputDevice::AXIS_YTILT:    device->set_axis_use(axis, Gdk::AXIS_YTILT);    break;
			case InputDevice::AXIS_WHEEL:    device->set_axis_use(axis, Gdk::AXIS_WHEEL);    break;
			case InputDevice::AXIS_LAST:     device->set_axis_use(axis, Gdk::AXIS_LAST);     break;
			default:                         device->set_axis_use(axis, Gdk::AXIS_IGNORE);   break;
			}

		const std::vector<InputDevice::DeviceKey> keys = synfig_device->get_keys();
		for (int key = 0; key < (int) keys.size(); key++)
			device->set_key(key, keys[key].keyval, Gdk::ModifierType(keys[key].modifiers));
	}
}
