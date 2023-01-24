/* === S Y N F I G ========================================================= */
/*!	\file cellrenderer_gradient.cpp
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

#include <cassert>
#include "cellrenderer_gradient.h"
#include "widgets/widget_gradient.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

CellRenderer_Gradient::CellRenderer_Gradient():
	Glib::ObjectBase	(typeid(CellRenderer_Gradient)),
	Gtk::CellRendererText	(),
	property_gradient_(*this,"gradient",synfig::Gradient())
{
	assert(0); //CHECK: This class does not appear to be used.
	//CellRendererText::signal_edited().connect(sigc::mem_fun(*this,&studio::CellRenderer_Gradient::string_edited_));
}

CellRenderer_Gradient::~CellRenderer_Gradient()
{
}


void
CellRenderer_Gradient::render_vfunc(
		const ::Cairo::RefPtr< ::Cairo::Context>& cr,
		Gtk::Widget& /* widget */,
		const Gdk::Rectangle& /* background_area */,
		const Gdk::Rectangle& cell_area,
		Gtk::CellRendererState /* flags */)
{
	if(!cr)
		return;
	render_gradient_to_window(cr,cell_area,property_gradient_.get_value());
}


Gtk::CellEditable*
CellRenderer_Gradient::start_editing_vfunc(
	GdkEvent* /*event*/,
	Gtk::Widget& /*widget*/,
	const Glib::ustring& /*path*/,
	const Gdk::Rectangle& /*background_area*/,
	const Gdk::Rectangle& /*cell_area*/,
	Gtk::CellRendererState /*flags*/)
{
	return 0;
}
