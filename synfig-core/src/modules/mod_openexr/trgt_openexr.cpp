/* === S Y N F I G ========================================================= */
/*!	\file trgt_openexr.cpp
**	\brief exr_trgt Target Module
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

#include "trgt_openexr.h"
#include <cstdio>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_TARGET_INIT(exr_trgt);
SYNFIG_TARGET_SET_NAME(exr_trgt,"openexr");
SYNFIG_TARGET_SET_EXT(exr_trgt,"exr");
SYNFIG_TARGET_SET_VERSION(exr_trgt,"1.0.4");

/* === M E T H O D S ======================================================= */

bool
exr_trgt::ready()
{
	return (bool)exr_file;
}

exr_trgt::exr_trgt(const synfig::filesystem::Path& Filename, const synfig::TargetParam& params):
	multi_image(false),
	imagecount(0),
	scanline(),
	filename(Filename),
	exr_file(nullptr),
	sequence_separator(params.sequence_separator)
{
	// OpenEXR uses linear gamma
}

exr_trgt::~exr_trgt()
{
	if(exr_file) delete exr_file;
}

bool
exr_trgt::set_rend_desc(RendDesc *given_desc)
{
	//given_desc->set_pixel_format(PixelFormat((int)PF_RAW_COLOR));
	desc=*given_desc;
	imagecount=desc.get_frame_start();
	if(desc.get_frame_end()-desc.get_frame_start()>0)
		multi_image=true;
	else
		multi_image=false;
	return true;
}

bool
exr_trgt::start_frame(synfig::ProgressCallback *cb)
{
	int w=desc.get_w(),h=desc.get_h();

	if (exr_file)
		delete exr_file;

	synfig::filesystem::Path frame_name = filename;

	if (multi_image) {
		frame_name.add_suffix(sequence_separator + strprintf("%04d",imagecount));
	}
	if (cb)
		cb->task(frame_name.u8string());

	// OpenEXR implementation does not support wchar_t, so MS Windows users will have troubles sometimes
	exr_file=new Imf::RgbaOutputFile(frame_name.u8_str(),w,h,Imf::WRITE_RGBA,desc.get_pixel_aspect());
	buffer_color.resize(w);
	//buffer.resize(w);
	out_surface.set_wh(w,h);

	return true;
}

void
exr_trgt::end_frame()
{
	if(exr_file)
	{
		exr_file->setFrameBuffer(out_surface[0],1,desc.get_w());
		exr_file->writePixels(desc.get_h());

		delete exr_file;
	}

	exr_file=0;

	imagecount++;
}

Color *
exr_trgt::start_scanline(int i)
{
	scanline=i;
	return buffer_color.data();
}

bool
exr_trgt::end_scanline()
{
	if(!ready())
		return false;

	int i;
	for(i=0;i<desc.get_w();i++)
	{
//		Imf::Rgba &rgba=buffer[i];
		Imf::Rgba &rgba=out_surface[scanline][i];
		Color &color=buffer_color[i];
		rgba.r=color.get_r();
		rgba.g=color.get_g();
		rgba.b=color.get_b();
		rgba.a=color.get_a();
	}

    //exr_file->setFrameBuffer(buffer,1,desc.get_w());
	//exr_file->writePixels(1);

	return true;
}
