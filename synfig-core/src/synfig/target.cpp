/* === S Y N F I G ========================================================= */
/*!	\file target.cpp
**	\brief Target Class Implementation
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2010 Diego Barrios Romero
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

/* === H E A D E R S ======================================================= */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "target.h"
#include "string.h"
#include "canvas.h"
#include "target_null.h"
#include "target_null_tile.h"
#include "targetparam.h"

using namespace synfig;
using namespace etl;

synfig::Target::Book* synfig::Target::book_;
synfig::Target::ExtBook* synfig::Target::ext_book_;

/* === P R O C E D U R E S ================================================= */

bool
Target::subsys_init()
{
	book_=new synfig::Target::Book();
	ext_book_=new synfig::Target::ExtBook();

	// At least one target must be available.
	book()["null"].factory =
		reinterpret_cast<synfig::Target::Factory>(&Target_Null::create);
	book()["null"].filename = "null";
	book()["null"].target_param = TargetParam();
	ext_book()["null"]="null";

	book()["null-tile"].factory =
		reinterpret_cast<synfig::Target::Factory>(&Target_Null_Tile::create);
	book()["null-tile"].filename = "null-tile";
	book()["null-tile"].target_param = TargetParam();
	ext_book()["null-tile"]="null-tile";

	return true;
}

bool
Target::subsys_stop()
{
	delete book_;
	delete ext_book_;
	return true;
}

Target::Book&
Target::book()
{
	return *book_;
}

Target::ExtBook&
Target::ext_book()
{
	return *ext_book_;
}


/* === M E T H O D S ======================================================= */

Target::Target():
	quality_(4),
	alpha_mode(TARGET_ALPHA_MODE_KEEP),
	avoid_time_sync_(false),
	curr_frame_(0)
{
}

void
synfig::Target::set_canvas(etl::handle<Canvas> c)
{
	canvas=c;
	RendDesc desc=canvas->rend_desc();
	set_rend_desc(&desc);
}


Target::Handle
Target::create(const String &name, const String &filename,
			   const synfig::TargetParam& params)
{
	if(!book().count(name))
		return handle<Target>();

	return Target::Handle(book()[name].factory(filename.c_str(), params));
}

int
Target::next_frame(Time& time)
{
	int
	total_frames(1),
	frame_start(0),
	frame_end(0);
	Time
	time_start(0),
	time_end(0);
		
	frame_start=desc.get_frame_start();
	frame_end=desc.get_frame_end();
	time_start=desc.get_time_start();
	time_end=desc.get_time_end();
	// TODO: Add option to exclude last frame
	// If user wants to recover the last buggy behavior then 
	// expose this option to the interface using the target params.
	// At the moment it is set to false.
	bool exclude_last_frame(false);
	// Calculate the number of frames
	total_frames=frame_end-frame_start+(exclude_last_frame?0:1);
	if(total_frames<=0)total_frames=1;

	if(total_frames == 1)
	{
		time=time_start;
	}
	else
	{
		time=(time_end-time_start)*curr_frame_/(total_frames-(exclude_last_frame?0:1))+time_start;
	}

//	synfig::info("before curr_frame_: %d",curr_frame_);
	curr_frame_++;
	
//	synfig::info("before curr_frame_: %d",curr_frame_);
//	synfig::info("total_frames: %d",total_frames);
//	synfig::info("time_end: %s",time_end.get_string().c_str());
//	synfig::info("time_start: %s",time_start.get_string().c_str());
//	synfig::info("time: %s",time.get_string().c_str());
//	synfig::info("remaining frames %d", total_frames-curr_frame_);

	return total_frames- curr_frame_;
}

