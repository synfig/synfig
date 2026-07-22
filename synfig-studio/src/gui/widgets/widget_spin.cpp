/* === S Y N F I G ========================================================= */
/*!	\file widget_spin.cpp
**	\brief Template File
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

#include <gui/widgets/widget_spin.h>

#include <gui/exception_guard.h>
#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Spin::Widget_Spin()
{
	signal_event_after().connect(sigc::mem_fun(*this, &Widget_Spin::after_event));
}

Widget_Spin::Widget_Spin(Glib::RefPtr<Gtk::Adjustment >& adjustment,//this ref should prob be const
												  double climb_rate,
													   guint digits)
{
	SpinButton::configure(adjustment,climb_rate,digits);
	signal_event_after().connect(sigc::mem_fun(*this, &Widget_Spin::after_event));
}


Widget_Spin::~Widget_Spin()
{
}

void
Widget_Spin::after_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	if ((event->type == GDK_BUTTON_RELEASE) && first_selection) {
		select_region(0, -1);
		first_selection = false;
	}
	SYNFIG_EXCEPTION_GUARD_END()
}

bool
Widget_Spin::on_event(GdkEvent* event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	if (event->type == GDK_FOCUS_CHANGE && event->focus_change.in)
			first_selection=true;
	return SpinButton::on_event(event);
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}
