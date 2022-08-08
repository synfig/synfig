/* === S Y N F I G ========================================================= */
/*!	\file Dialog_Guide.cpp
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/dialogs/dialog_guide.h>

#include <gui/localization.h>

#include <gui/widgets/widget_waypoint.h>


#endif

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dialog_Guide::Dialog_Guide(Gtk::Window& parent,etl::handle<synfig::Canvas> canvas):
	Dialog(_("Guide Editor"),parent),
	canvas(canvas)
{
	this->set_resizable(false);
	assert(canvas);
////	waypointwidget=manage(new class Widget_Waypoint(canvas));
//	get_content_area()->pack_start(*waypointwidget);// to get the box an then pack in the beginning the box

	Gtk::Button *ok_button(manage(new Gtk::Button(_("_OK"), true)));
	ok_button->show();
	add_action_widget(*ok_button,2);
	ok_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Guide::on_ok_pressed));

//	waypointwidget->show_all();
}

Dialog_Guide::~Dialog_Guide()
{
}

void
Dialog_Guide::on_ok_pressed()
{
	hide();
//	signal_changed_();
}
