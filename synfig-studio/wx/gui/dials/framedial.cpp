/* === S Y N F I G ========================================================= */
/*!	\file framedial.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
**  Copyright (c) 2009 Gerco Ballintijn
**	Copyright (c) 2009 Carlos López
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

#include "framedial.h"
#include <gtkmm/image.h>
#include <gtkmm/stock.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

FrameDial::FrameDial(): Gtk::Table(8, 1, false)
{
	seek_begin =  create_icon(Gtk::ICON_SIZE_BUTTON, "synfig-animate_seek_begin",_("Seek to begin"));
	seek_prev_keyframe =  create_icon(Gtk::ICON_SIZE_BUTTON, "synfig-animate_seek_prev_keyframe",_("Seek to previous keyframe"));
	seek_prev_frame =  create_icon(Gtk::ICON_SIZE_BUTTON, "synfig-animate_seek_prev_frame",_("Seek to previous frame"));
	play =  create_icon(Gtk::ICON_SIZE_BUTTON, "synfig-animate_play",_("Play"));
	pause = create_icon(Gtk::ICON_SIZE_BUTTON, "synfig-animate_pause",_("Pause"));
	seek_next_frame =  create_icon(Gtk::ICON_SIZE_BUTTON, "synfig-animate_seek_next_frame",_("Seek to next frame"));
	seek_next_keyframe =  create_icon(Gtk::ICON_SIZE_BUTTON, "synfig-animate_seek_next_keyframe",_("Seek to next keyframe"));
	seek_end =  create_icon(Gtk::ICON_SIZE_BUTTON, "synfig-animate_seek_end",_("Seek to end"));

	attach(*seek_begin,			0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*seek_prev_keyframe, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*seek_prev_frame,	2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*play,				3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*pause,				3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*seek_next_frame,	4, 5, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*seek_next_keyframe,	5, 6, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(*seek_end,			6, 7, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	pause->hide();
}

Gtk::Button *
FrameDial::create_icon(Gtk::IconSize iconsize, const char * stockid, const char * tooltip)
{
	iconsize = Gtk::IconSize::from_name("synfig-small_icon_16x16");
	Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID(stockid), iconsize));
	Gtk::Button *button = manage(new class Gtk::Button());
	button->add(*icon);
	button->set_tooltip_text(tooltip);
	icon->set_padding(0, 0);
	icon->show();
	button->set_relief(Gtk::RELIEF_NONE);
	button->show();

	return button;
}

void
FrameDial::toggle_play_pause_button(bool is_playing)
{
	if(is_playing)
	{
		pause->hide();
		play->show();
	}
	else
	{
		play->hide();
		pause->show();
	}
}
