/* === S Y N F I G ========================================================= */
/*!	\file tool/job.cpp
**	\brief Job class methods
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2012 Diego Barrios Romero
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <iostream>
#include <string>
#include <synfig/loadcanvas.h>
#include <synfig/canvas.h>
#include <synfig/target.h>
#include "definitions.h"
#include "job.h"
#endif

using namespace std;
using namespace synfig;

Job::Job()
{
    canvas_info = canvas_info_all = canvas_info_time_start =
      canvas_info_time_end = canvas_info_frame_rate =
      canvas_info_frame_start = canvas_info_frame_end =
      canvas_info_w = canvas_info_h = canvas_info_image_aspect =
      canvas_info_pw = canvas_info_ph = canvas_info_pixel_aspect =
      canvas_info_tl = canvas_info_br = canvas_info_physical_w =
      canvas_info_physical_h = canvas_info_x_res = canvas_info_y_res =
      canvas_info_span = canvas_info_interlaced =
      canvas_info_antialias = canvas_info_clamp =  canvas_info_flags =
      canvas_info_focus = canvas_info_bg_color = canvas_info_metadata
      = false;

    quality = DEFAULT_QUALITY;
    sifout = false;
}

int Job::load_file (string filename)
{
	_filename = filename;
	// Open the composition
	string errors, warnings;
	try
	{
		_root = open_canvas(_filename, errors, warnings);
	}
	catch(runtime_error x)
	{
		_root = 0;
	}

	// By default, the canvas to render is the root canvas
	// This can be changed through --canvas option
	canvas = _root;

	if(!canvas)
	{
		cerr << _("Unable to load '") << filename << "'."
			 << endl;

		return SYNFIGTOOL_FILENOTFOUND;
	}

	return SYNFIGTOOL_OK;
}

string Job::filename () const
{
	return _filename;
}

Canvas::Handle Job::root () const
{
	return _root;
}
