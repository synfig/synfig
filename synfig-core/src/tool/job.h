/* === S Y N F I G ========================================================= */
/*!	\file tool/job.h
**	\brief Job struct
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012 Diego Barrios Romero
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

#ifndef __SYNFIG_JOB_H
#define __SYNFIG_JOB_H
#include "synfig/target.h"

struct Job
{
	std::string filename;
	std::string outfilename;
	std::string target_name;

	synfig::RendDesc desc;
	synfig::TargetAlphaMode alpha_mode;

	synfig::Canvas::Handle root;
	synfig::Canvas::Handle canvas;
	synfig::Target::Handle target;

	int quality;
	bool sifout;
	bool list_canvases;
	bool extract_alpha;

	bool
		canvas_info,
		canvas_info_all,
		canvas_info_time_start,
		canvas_info_time_end,
		canvas_info_frame_rate,
		canvas_info_frame_start,
		canvas_info_frame_end,
		canvas_info_w,
		canvas_info_h,
		canvas_info_image_aspect,
		canvas_info_pw,
		canvas_info_ph,
		canvas_info_pixel_aspect,
		canvas_info_tl,
		canvas_info_br,
		canvas_info_physical_w,
		canvas_info_physical_h,
		canvas_info_x_res,
		canvas_info_y_res,
		canvas_info_span,
		canvas_info_interlaced,
		canvas_info_antialias,
		canvas_info_clamp,
		canvas_info_flags,
		canvas_info_focus,
		canvas_info_bg_color,
		canvas_info_metadata;

    Job():
		alpha_mode(synfig::TARGET_ALPHA_MODE_KEEP),
		quality(DEFAULT_QUALITY),
		sifout(false),
		list_canvases(),
		extract_alpha(false),
		canvas_info(),
		canvas_info_all(),
		canvas_info_time_start(),
		canvas_info_time_end(),
		canvas_info_frame_rate(),
		canvas_info_frame_start(),
		canvas_info_frame_end(),
		canvas_info_w(),
		canvas_info_h(),
		canvas_info_image_aspect(),
		canvas_info_pw(),
		canvas_info_ph(),
		canvas_info_pixel_aspect(),
		canvas_info_tl(),
		canvas_info_br(),
		canvas_info_physical_w(),
		canvas_info_physical_h(),
		canvas_info_x_res(),
		canvas_info_y_res(),
		canvas_info_span(),
		canvas_info_interlaced(),
		canvas_info_antialias(),
		canvas_info_clamp(),
		canvas_info_flags(),
		canvas_info_focus(),
		canvas_info_bg_color(),
		canvas_info_metadata()
    { }

};

#endif
