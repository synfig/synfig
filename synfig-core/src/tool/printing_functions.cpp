/* === S Y N F I G ========================================================= */
/*!	\file tool/printing_functions.cpp
**	\brief Printing functions
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2014 Diego Barrios Romero
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <iostream>
#include <string>
#include <synfig/canvas.h>
#include <synfig/target.h>
#include "definitions.h"
#include "job.h"
#include "printing_functions.h"
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#endif

using namespace std;
using namespace synfig;

void print_usage()
{
	std::cout << "Synfig " << VERSION << std::endl
			  << "Usage: " << SynfigToolGeneralOptions::instance()->get_binary_path()
			  << " [options] ([sif file] [specific options])" << std::endl;
}

void print_child_canvases(const std::string& prefix,
						  const synfig::Canvas::Handle& canvas)
{
	const synfig::Canvas::Children& children(canvas->children());
	for (synfig::Canvas::Children::const_iterator child_canvas = children.begin();
		 child_canvas != children.end(); child_canvas++)
	{
		std::string new_prefix = prefix + ":" + (*child_canvas)->get_id();
		std::cout << new_prefix << std::endl;
		print_child_canvases(new_prefix, *child_canvas);
	}
}

void print_canvas_info(const Job& job)
{
	const Canvas::Handle canvas(job.canvas);
	const RendDesc &rend_desc(canvas->rend_desc());

	if (job.canvas_info_all || job.canvas_info_time_start)
	{
		cout << endl << "# " << _("Start Time") << endl;
		cout << "time_start" << "="
			 << rend_desc.get_time_start().get_string().c_str() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_time_end)
	{
		cout << endl << "# " << _("End Time") << endl;
		cout << "time_end" << "="
			 << rend_desc.get_time_end().get_string().c_str() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_frame_rate)
	{
		cout << endl << "# " << _("Frame Rate") << endl;
		cout << "frame_rate" << "="
			 << rend_desc.get_frame_rate() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_frame_start)
	{
		cout << endl << "# " << _("Start Frame") << endl;
		cout << "frame_start" << "="
			 << rend_desc.get_frame_start() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_frame_end)
	{
		cout << endl << "# " << _("End Frame") << endl;
		cout << "frame_end"	<< "="
			 << rend_desc.get_frame_end() << endl;
	}

	if (job.canvas_info_all)
		cout << endl;

	if (job.canvas_info_all || job.canvas_info_w)
	{
		cout << endl << "# " << _("Width") << endl;
		cout << "w"	<< "=" << rend_desc.get_w() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_h)
	{
		cout << endl << "# " << _("Height") << endl;
		cout << "h"	<< "=" << rend_desc.get_h() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_image_aspect)
	{
		cout << endl << "# " << _("Image Aspect Ratio") << endl;
		cout << "image_aspect" << "="
			 << rend_desc.get_image_aspect() << endl;
	}

	if (job.canvas_info_all)
		cout << endl;

	if (job.canvas_info_all || job.canvas_info_pw)
	{
		cout << endl << "# " << _("Pixel Width") << endl;
		cout << "pw" << "="
			 << rend_desc.get_pw() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_ph)
	{
		cout << endl << "# " << _("Pixel Height") << endl;
		cout << "ph" << "="
			 << rend_desc.get_ph() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_pixel_aspect)
	{
		cout << endl << "# " << _("Pixel Aspect Ratio") << endl;
		cout << "pixel_aspect" << "="
			 << rend_desc.get_pixel_aspect() << endl;
	}

	if (job.canvas_info_all)
		cout << endl;

	if (job.canvas_info_all || job.canvas_info_tl)
	{
		cout << endl << "# " << _("Top Left") << endl;
		cout << "tl" << "=" << rend_desc.get_tl()[0]
			 << " " << rend_desc.get_tl()[1] << endl;
	}

	if (job.canvas_info_all || job.canvas_info_br)
	{
		cout << endl << "# " << _("Bottom Right") << endl;
		cout << "br" << "=" << rend_desc.get_br()[0]
			 << " " << rend_desc.get_br()[1] << endl;
	}

	if (job.canvas_info_all || job.canvas_info_physical_w)
	{
		cout << endl << "# " << _("Physical Width") << endl;
		cout << "physical_w" << "="
			 << rend_desc.get_physical_w() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_physical_h)
	{
		cout << endl << "# " << _("Physical Height") << endl;
		cout << "physical_h" << "="
			 << rend_desc.get_physical_h() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_x_res)
	{
		cout << endl << "# " << _("X Resolution") << endl;
		cout << "x_res"	<< "=" << rend_desc.get_x_res() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_y_res)
	{
		cout << endl << "# " << _("Y Resolution") << endl;
		cout << "y_res"	<< "=" << rend_desc.get_y_res() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_span)
	{
		cout << endl << "# " << _("Diagonal Image Span") << endl;
		cout << "span" << "=" << rend_desc.get_span() << endl;
	}

	if (job.canvas_info_all)
		cout << endl;

	if (job.canvas_info_all || job.canvas_info_interlaced)
	{
		cout << endl << "# " << _("Interlaced") << endl;
		cout << "interlaced" << "="
			 << rend_desc.get_interlaced() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_antialias)
	{
		cout << endl << "# " << _("Antialias") << endl;
		cout << "antialias"	<< "="
			 << rend_desc.get_antialias() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_clamp)
	{
		cout << endl << "# " << _("Clamp") << endl;
		cout << "clamp"
			 << "=" << rend_desc.get_clamp() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_flags)
	{
		cout << endl << "# " << _("Flags") << endl;
		cout << "flags"
			 << "=" << rend_desc.get_flags() << endl;
	}

	if (job.canvas_info_all || job.canvas_info_focus)
	{
		cout << endl << "# " << _("Focus") << endl;
		cout << "focus"	<< "=" << rend_desc.get_focus()[0]
			 << " " << rend_desc.get_focus()[1] << endl;
	}

	if (job.canvas_info_all || job.canvas_info_bg_color)
	{
		cout << endl << "# " << _("Background Color") << endl;
		cout << "bg_color" << "="
			 << rend_desc.get_bg_color().get_string().c_str() << endl;
	}

	if (job.canvas_info_all)
		cout << endl;

	if (job.canvas_info_all || job.canvas_info_metadata)
	{
		std::list<String> keys(canvas->get_meta_data_keys());
		cout << endl << "# " << _("Metadata") << endl;

		for (std::list<String>::iterator key = keys.begin();
			 key != keys.end(); key++)
			cout << (*key).c_str() << "=" << canvas->get_meta_data(*key).c_str()<< endl;
	}
}
