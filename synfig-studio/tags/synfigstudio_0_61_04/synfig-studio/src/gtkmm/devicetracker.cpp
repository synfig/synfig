/* === S Y N F I G ========================================================= */
/*!	\file devicetracker.cpp
**	\brief Template File
**
**	$Id: devicetracker.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "devicetracker.h"
#include <gdkmm/device.h>
#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

DeviceTracker::DeviceTracker()
{
	// By default, set the input mode on devices to
	// GDK_MODE_SCREEN.
	{
		GList*	device_list;
		GList*	iter;
		device_list=gdk_devices_list();
		
		for(iter=device_list;iter;iter=g_list_next(iter))
		{
			GdkDevice* device=reinterpret_cast<GdkDevice*>(iter->data);
			gdk_device_set_mode(device,GDK_MODE_SCREEN);
			
			synfigapp::InputDevice::Handle input_device;
			input_device=synfigapp::Main::add_input_device(device->name,synfigapp::InputDevice::Type(device->source));
			if(input_device->get_type()==synfigapp::InputDevice::TYPE_MOUSE)
				synfigapp::Main::select_input_device(input_device);
		}		
	}
}

DeviceTracker::~DeviceTracker()
{
}
