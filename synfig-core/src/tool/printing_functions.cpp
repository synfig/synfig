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
		std::cout << std::endl << "# " << _("Start Time") << std::endl;
		std::cout << "time_start" << "="
			 << rend_desc.get_time_start().get_string().c_str() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_time_end)
	{
		std::cout << std::endl << "# " << _("End Time") << std::endl;
		std::cout << "time_end" << "="
			 << rend_desc.get_time_end().get_string().c_str() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_frame_rate)
	{
		std::cout << std::endl << "# " << _("Frame Rate") << std::endl;
		std::cout << "frame_rate" << "="
			 << rend_desc.get_frame_rate() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_frame_start)
	{
		std::cout << std::endl << "# " << _("Start Frame") << std::endl;
		std::cout << "frame_start" << "="
			 << rend_desc.get_frame_start() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_frame_end)
	{
		std::cout << std::endl << "# " << _("End Frame") << std::endl;
		std::cout << "frame_end"	<< "="
			 << rend_desc.get_frame_end() << std::endl;
	}

	if (job.canvas_info_all)
		std::cout << std::endl;

	if (job.canvas_info_all || job.canvas_info_w)
	{
		std::cout << std::endl << "# " << _("Width") << std::endl;
		std::cout << "w"	<< "=" << rend_desc.get_w() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_h)
	{
		std::cout << std::endl << "# " << _("Height") << std::endl;
		std::cout << "h"	<< "=" << rend_desc.get_h() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_image_aspect)
	{
		std::cout << std::endl << "# " << _("Image Aspect Ratio") << std::endl;
		std::cout << "image_aspect" << "="
			 << rend_desc.get_image_aspect() << std::endl;
	}

	if (job.canvas_info_all)
		std::cout << std::endl;

	if (job.canvas_info_all || job.canvas_info_pw)
	{
		std::cout << std::endl << "# " << _("Pixel Width") << std::endl;
		std::cout << "pw" << "="
			 << rend_desc.get_pw() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_ph)
	{
		std::cout << std::endl << "# " << _("Pixel Height") << std::endl;
		std::cout << "ph" << "="
			 << rend_desc.get_ph() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_pixel_aspect)
	{
		std::cout << std::endl << "# " << _("Pixel Aspect Ratio") << std::endl;
		std::cout << "pixel_aspect" << "="
			 << rend_desc.get_pixel_aspect() << std::endl;
	}

	if (job.canvas_info_all)
		std::cout << std::endl;

	if (job.canvas_info_all || job.canvas_info_tl)
	{
		std::cout << std::endl << "# " << _("Top Left") << std::endl;
		std::cout << "tl" << "=" << rend_desc.get_tl()[0]
			 << " " << rend_desc.get_tl()[1] << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_br)
	{
		std::cout << std::endl << "# " << _("Bottom Right") << std::endl;
		std::cout << "br" << "=" << rend_desc.get_br()[0]
			 << " " << rend_desc.get_br()[1] << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_physical_w)
	{
		std::cout << std::endl << "# " << _("Physical Width") << std::endl;
		std::cout << "physical_w" << "="
			 << rend_desc.get_physical_w() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_physical_h)
	{
		std::cout << std::endl << "# " << _("Physical Height") << std::endl;
		std::cout << "physical_h" << "="
			 << rend_desc.get_physical_h() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_x_res)
	{
		std::cout << std::endl << "# " << _("X Resolution") << std::endl;
		std::cout << "x_res"	<< "=" << rend_desc.get_x_res() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_y_res)
	{
		std::cout << std::endl << "# " << _("Y Resolution") << std::endl;
		std::cout << "y_res"	<< "=" << rend_desc.get_y_res() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_span)
	{
		std::cout << std::endl << "# " << _("Diagonal Image Span") << std::endl;
		std::cout << "span" << "=" << rend_desc.get_span() << std::endl;
	}

	if (job.canvas_info_all)
		std::cout << std::endl;

	if (job.canvas_info_all || job.canvas_info_interlaced)
	{
		std::cout << std::endl << "# " << _("Interlaced") << std::endl;
		std::cout << "interlaced" << "="
			 << rend_desc.get_interlaced() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_antialias)
	{
		std::cout << std::endl << "# " << _("Antialias") << std::endl;
		std::cout << "antialias"	<< "="
			 << rend_desc.get_antialias() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_clamp)
	{
		std::cout << std::endl << "# " << _("Clamp") << std::endl;
		std::cout << "clamp"
			 << "=" << rend_desc.get_clamp() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_flags)
	{
		std::cout << std::endl << "# " << _("Flags") << std::endl;
		std::cout << "flags"
			 << "=" << rend_desc.get_flags() << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_focus)
	{
		std::cout << std::endl << "# " << _("Focus") << std::endl;
		std::cout << "focus"	<< "=" << rend_desc.get_focus()[0]
			 << " " << rend_desc.get_focus()[1] << std::endl;
	}

	if (job.canvas_info_all || job.canvas_info_bg_color)
	{
		std::cout << std::endl << "# " << _("Background Color") << std::endl;
		std::cout << "bg_color" << "="
			 << rend_desc.get_bg_color().get_string().c_str() << std::endl;
	}

	if (job.canvas_info_all)
		std::cout << std::endl;

	if (job.canvas_info_all || job.canvas_info_metadata)
	{
		std::list<String> keys(canvas->get_meta_data_keys());
		std::cout << std::endl << "# " << _("Metadata") << std::endl;

		for (std::list<String>::iterator key = keys.begin();
			 key != keys.end(); key++)
			std::cout << (*key).c_str() << "=" << canvas->get_meta_data(*key).c_str()<< std::endl;
	}
}
