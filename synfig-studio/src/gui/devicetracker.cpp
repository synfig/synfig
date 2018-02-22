/* === S Y N F I G ========================================================= */
/*!	\file devicetracker.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2009 Gerco Ballintijn
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

// FIXME: The code here doesn't use the GTKmm layer but uses GTK+ directly
// since the GTKmm wrapper for Gdk::Device is incomplete. When the wrapper
// gets fixed, this code should be updated accoordingly.

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/general.h>

#include "devicetracker.h"
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <synfigapp/main.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static
GList* get_auxiliary_devices(GdkSeat*& seat) {
	seat = gdk_display_get_default_seat(
		gdk_display_manager_get_default_display(
			gdk_display_manager_get()));
	return gdk_seat_get_slaves(
		seat,
		GDK_SEAT_CAPABILITY_ALL_POINTING );
}
			
static
GList* get_devices(GdkDevice*& pointer) {
	GdkSeat* seat = NULL;
	GList* device_list = get_auxiliary_devices(seat);

	pointer = gdk_seat_get_pointer(seat);

	device_list = g_list_prepend(device_list, pointer);
	return device_list;
}

static
GList* get_devices() {
	GdkSeat* seat = NULL;
	GList* device_list = get_auxiliary_devices(seat);

	GdkDevice* pointer = gdk_seat_get_pointer(seat);

	device_list = g_list_prepend(device_list, pointer);
	return device_list;
}

DeviceTracker::DeviceTracker()
{
	GdkDevice* pointer = NULL;
	GList* device_list = get_devices(pointer);
		for(GList *iter=device_list; iter; iter=g_list_next(iter))
		{
			GdkDevice* device=reinterpret_cast<GdkDevice*>(iter->data);

			synfigapp::InputDevice::Handle input_device;
			input_device=synfigapp::Main::add_input_device(
				gdk_device_get_name(device),
				synfigapp::InputDevice::Type(gdk_device_get_source(device)) );
			//Disable all extended devices by default. This tries to fix several
			// bugs reported in track and forums
			if(	input_device->get_type()==synfigapp::InputDevice::TYPE_MOUSE  ||
				input_device->get_type()==synfigapp::InputDevice::TYPE_PEN    ||
				input_device->get_type()==synfigapp::InputDevice::TYPE_ERASER ||
				input_device->get_type()==synfigapp::InputDevice::TYPE_CURSOR  )
			{
				input_device->set_mode(synfigapp::InputDevice::MODE_DISABLED);
				//synfigapp::Main::select_input_device(input_device);
			}
		}

	// Once all devices are disabled be sure that the core pointer is the
	// one selected. The user should decide later whether enable and save the
	// rest of input devices found.
	synfigapp::Main::select_input_device(
		gdk_device_get_name(pointer));

	g_list_free(device_list);

}

DeviceTracker::~DeviceTracker()
{
}

void
DeviceTracker::save_preferences()
{
		GList* device_list = get_devices();
	
		for(GList *itr=device_list; itr; itr=g_list_next(itr))
		{
			GdkDevice * gdk_device = reinterpret_cast<GdkDevice*>(itr->data);

			InputDevice::Handle synfig_device =
				synfigapp::Main::find_input_device(
					gdk_device_get_name(gdk_device) );
			if (!synfig_device)
				continue;

			synfig_device->set_mode(InputDevice::Mode(gdk_device_get_mode(gdk_device)));
			int n_axes = gdk_device_get_n_axes(gdk_device);
			if (n_axes > 0) {
				vector<synfigapp::InputDevice::AxisUse> axes(n_axes);
				for(int j = 0; j < n_axes; ++j)
					axes[j] = InputDevice::AxisUse(gdk_device_get_axis_use(gdk_device, j));
				synfig_device->set_axes(axes);
			}

			int n_keys = gdk_device_get_n_keys(gdk_device);
			if (n_keys > 0) {
				vector<synfigapp::InputDevice::DeviceKey> keys(n_keys);
				for(int j = 0; j < n_keys; ++j) {
					guint gdk_keyval = 0;
					GdkModifierType gdk_modifiers = GdkModifierType();
					gdk_device_get_key(gdk_device, j, &gdk_keyval, &gdk_modifiers);
					keys[j].keyval = gdk_keyval;
					keys[j].modifiers = gdk_modifiers;
				}
				synfig_device->set_keys(keys);
			}
		}

		g_list_free(device_list);
}

void
DeviceTracker::load_preferences()
{
	GList* device_list = get_devices();

		for(GList *itr=device_list; itr; itr=g_list_next(itr))
		{
			GdkDevice * gdk_device = reinterpret_cast<GdkDevice*>(itr->data);

			InputDevice::Handle synfig_device =
				synfigapp::Main::find_input_device(
					gdk_device_get_name(gdk_device) );
			if (!synfig_device)
				continue;

			gdk_device_set_mode(gdk_device, GdkInputMode(synfig_device->get_mode()));

			const std::vector<synfigapp::InputDevice::AxisUse> axes = synfig_device->get_axes();
			for (int axis = 0; axis < (int) axes.size(); axis++)
				gdk_device_set_axis_use(gdk_device, axis, GdkAxisUse(axes[axis]));

			const std::vector<synfigapp::InputDevice::DeviceKey> keys = synfig_device->get_keys();
			for (int key = 0; key < (int) keys.size(); key++)
				gdk_device_set_key(gdk_device, key, keys[key].keyval,
						GdkModifierType(keys[key].modifiers));
		}

		g_list_free(device_list);
}
