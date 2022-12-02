/* === S Y N F I G ========================================================= */
/*!	\file trgt_dv.cpp
**	\brief dv Target Module
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <ETL/stringf>

#include <synfig/localization.h>
#include <synfig/general.h>

#include <glib/gstdio.h>
#include "trgt_dv.h"
#include <algorithm>
#include <thread>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(dv_trgt);
SYNFIG_TARGET_SET_NAME(dv_trgt,"dv");
SYNFIG_TARGET_SET_EXT(dv_trgt,"dv");
SYNFIG_TARGET_SET_VERSION(dv_trgt,"0.1");

/* === M E T H O D S ======================================================= */


dv_trgt::dv_trgt(const char *Filename, const synfig::TargetParam & /* params */):
	imagecount(0),
	wide_aspect(false),
	pipe(nullptr),
	filename(Filename),
	buffer(nullptr),
	color_buffer(nullptr)
{
	set_alpha_mode(TARGET_ALPHA_MODE_FILL);
}

dv_trgt::~dv_trgt()
{
	pipe = nullptr;
	delete [] buffer;
	delete [] color_buffer;
}

bool
dv_trgt::set_rend_desc(RendDesc *given_desc)
{
	// Set the aspect ratio
	if(wide_aspect)
	{
		// 16:9 Aspect
		given_desc->set_wh(160,90);

		// Widescreen should be progressive scan
		given_desc->set_interlaced(false);
	}
	else
	{
		// 4:3 Aspect
		given_desc->set_wh(400,300);

		// We should be interlaced
		given_desc->set_interlaced(true);
	}

	// but the pixel res should be 720x480
	given_desc->clear_flags(),given_desc->set_wh(720,480);

	// NTSC Frame rate is 29.97
	given_desc->set_frame_rate(29.97);

	// The pipe to encodedv is PPM, which needs RGB data
	//given_desc->set_pixel_format(PF_RGB);

	// Set the description
	desc=*given_desc;

	return true;
}

bool
dv_trgt::init(synfig::ProgressCallback * /* cb */)
{
	imagecount=desc.get_frame_start();

	OS::RunArgs args;

	if (wide_aspect)
		args.push_back({"-w", "1"});
	args.push_back("-");

	pipe = OS::run_async("encodedv", args, OS::RUN_MODE_WRITE, {"", filename, ""});
	if (!pipe || !pipe->is_writable()) {
		synfig::error(_("Unable to open pipe to encodedv"));
		return false;
	}

	// Sleep for a moment to let the pipe catch up
	std::this_thread::sleep_for(std::chrono::milliseconds(25));

	return true;
}

void
dv_trgt::end_frame()
{
	pipe->printf(" ");
	pipe->flush();
	imagecount++;
}

bool
dv_trgt::start_frame(synfig::ProgressCallback */*callback*/)
{
	int w=desc.get_w(),h=desc.get_h();

	if (!pipe)
		return false;

	pipe->printf("P6\n");
	pipe->printf("%d %d\n", w, h);
	pipe->printf("%d\n", 255);

	delete [] buffer;
	buffer=new unsigned char[3*w];

	delete [] color_buffer;
	color_buffer=new Color[w];

	return true;
}

Color *
dv_trgt::start_scanline(int /*scanline*/)
{
	return color_buffer;
}

bool
dv_trgt::end_scanline()
{
	if (!pipe)
		return false;

	color_to_pixelformat(buffer, color_buffer, PF_RGB, 0, desc.get_w());

	if (!pipe->write(buffer, 1, desc.get_w() * 3))
		return false;

	return true;
}
