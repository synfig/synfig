/* === S Y N F I G ========================================================= */
/*!	\file canvasoptions.cpp
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

#include "canvasoptions.h"

#include <gui/canvasview.h>
#include <gui/localization.h>
#include <gui/resourcehelper.h>
#include <gui/workarea.h>

#include <synfig/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void CanvasOptions::set_canvas_view(CanvasView* canvas_view)
{
	this->canvas_view_ = canvas_view;
	vector_grid_size->set_canvas(canvas_view->get_canvas());
	update_title();
}

CanvasOptions::CanvasOptions(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade) :
	Gtk::Dialog(cobject),
	builder(refGlade)//,
//	canvas_view_(canvas_view),
{
	refGlade->get_widget("toggle_grid_show", toggle_grid_show);
	refGlade->get_widget("toggle_grid_snap", toggle_grid_snap);
	refGlade->get_widget("toggle_time_snap", toggle_time_snap);
	refGlade->get_widget("vector_grid_size", vector_grid_size);
	//vector_grid_size.set_canvas(canvas_view->get_canvas());

	Gtk::Button *button = nullptr;

	refGlade->get_widget("ok_button", button);
	if (button)
		button->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasOptions::on_ok_pressed));

	refGlade->get_widget("apply_button", button);
	if (button)
		button->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasOptions::on_apply_pressed));

	refGlade->get_widget("cancel_button", button);
	if (button)
		button->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasOptions::on_cancel_pressed));

	if (toggle_grid_snap)
		toggle_grid_snap->signal_toggled().connect(sigc::mem_fun(*this, &studio::CanvasOptions::on_grid_snap_toggle));
	if (toggle_grid_show)
		toggle_grid_show->signal_toggled().connect(sigc::mem_fun(*this, &studio::CanvasOptions::on_grid_show_toggle));

	signal_show().connect(sigc::mem_fun(*this, &studio::CanvasOptions::refresh));

	vector_grid_size->set_digits(5);

	update_title();
}

CanvasOptions* CanvasOptions::create(Gtk::Window& parent, CanvasView* canvas_view)
{
	auto refBuilder = ResourceHelper::load_interface("canvas_options.glade");
	if (!refBuilder)
		return nullptr;
	CanvasOptions * dialog = nullptr;
	refBuilder->get_widget_derived("canvas_options", dialog);
	if (dialog) {
		dialog->set_transient_for(parent);
		dialog->set_canvas_view(canvas_view);
	}
	return dialog;
}

CanvasOptions::~CanvasOptions()
{
}

void
CanvasOptions::update_title()
{
	if (canvas_view_)
		set_title(_("Options")+String(" - ")+canvas_view_->get_canvas()->get_name());
}

void
CanvasOptions::refresh()
{
	if(canvas_view_->get_work_area()->grid_status())
		toggle_grid_show->set_active(true);
	else
		toggle_grid_show->set_active(false);

	if(canvas_view_->get_work_area()->get_grid_snap())
		toggle_grid_snap->set_active(true);
	else
		toggle_grid_snap->set_active(false);

	vector_grid_size->set_value(canvas_view_->get_work_area()->get_grid_size());

	toggle_time_snap->set_tooltip_text(_("Not yet implemented"));
	toggle_time_snap->set_sensitive(false);

	update_title();
}

void
CanvasOptions::on_grid_snap_toggle()
{
}

void
CanvasOptions::on_grid_show_toggle()
{
}

void
CanvasOptions::on_apply_pressed()
{
	canvas_view_->set_grid_snap_toggle(toggle_grid_snap->get_active());
	if(toggle_grid_snap->get_active())
		canvas_view_->get_work_area()->enable_grid_snap();
	else
		canvas_view_->get_work_area()->disable_grid_snap();

	canvas_view_->set_grid_show(toggle_grid_show->get_active());
	if(toggle_grid_show->get_active())
		canvas_view_->get_work_area()->enable_grid();
	else
		canvas_view_->get_work_area()->disable_grid();

	canvas_view_->get_work_area()->set_grid_size(vector_grid_size->get_value());
}

void
CanvasOptions::on_ok_pressed()
{
	on_apply_pressed();
	hide();
}

void
CanvasOptions::on_cancel_pressed()
{
	refresh();
	hide();
}
