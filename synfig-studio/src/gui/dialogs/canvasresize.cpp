/* === S Y N F I G ========================================================= */
/*!	\file dialogs/canvasresize.cpp
**	\brief Template File
**
**	$Id$
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "canvasresize.h"

#include <gui/dialogs/canvasproperties.h>
#include <gui/localization.h>
#include <gui/resourcehelper.h>
#include <gui/widgets/widget_link.h>

#include <gdkmm/pixbuf.h>

#endif

using namespace studio;
using namespace synfig;

CanvasResize::CanvasResize(Gtk::Window &parent, etl::handle<synfigapp::CanvasInterface> &ci, CanvasProperties &cp)
	: Gtk::Dialog       ("", parent)
	, canvas_interface  (ci)
	, rend_desc         (ci->get_canvas()->rend_desc())
	, canvas_properties (cp)
	, builder           (Gtk::Builder::create_from_file(ResourceHelper::get_ui_path("canvas_resize.glade")))
	, grid_content      (nullptr)
	, grid_canvas       (nullptr)
	, width             (nullptr)
	, height            (nullptr)
	, canvas_label      (nullptr)
	, canvas_only       (nullptr)
	, toggle_ratio_wh   (nullptr)
	, canvas_center     (canvas_buttons[4])
	, was_canvas_checked(false)
	, is_canvas_checked (false)
	, old_width         (0)
	, old_height        (0)
	, new_width         (0)
	, new_height        (0)
	, update_lock       (0)
{
	set_up_builder_widgets();
	set_up_toggle_tooltips();

	// Add builder grid content to this dialog window content
	get_content_area()->add(*grid_content);

	set_up_action_widgets();

	set_resizable(false);

	refresh_title();
	refresh_wh_toggle_widgets();

	set_up_signals();
}

void CanvasResize::set_up_builder_widgets()
{
	builder->get_widget("grid_content", grid_content);
	builder->get_widget("grid_canvas" , grid_canvas);
	builder->get_widget("width"       , width);
	builder->get_widget("height"      , height);
	builder->get_widget("canvas_only" , canvas_only);
	builder->get_widget("canvas_label", canvas_label);

	builder->get_widget_derived("toggle_ratio_wh", toggle_ratio_wh);

	int index = 0;
	for (std::size_t left = 0; left < 3; left++) {
		for (std::size_t top = 0; top < 3; top++) {
			auto widget = grid_canvas->get_child_at(top, left);
			canvas_buttons[index++] = static_cast<Gtk::Button*>(widget);
		}
	}
}

void CanvasResize::set_up_toggle_tooltips()
{
	toggle_ratio_wh->property_tooltip_active()   = _("Unlink width and height");
	toggle_ratio_wh->property_tooltip_inactive() = _("Link width and height");

	// Force refresh new tooltip messages
	toggle_ratio_wh->toggled();
}

void CanvasResize::set_up_action_widgets()
{
	add_button("_Advanced...", ADVANCED)->set_use_underline();
	add_button("_Cancel"     , CANCEL  )->set_use_underline();
	add_button("_OK"         , OKAY    )->set_use_underline();
}

void CanvasResize::set_up_signals()
{
	// Keep refreshing dialog when synfigapp::CanvasInterface object is updated
	canvas_interface->signal_id_changed().connect(
			sigc::mem_fun(*this, &CanvasResize::refresh_title));
	canvas_interface->signal_rend_desc_changed().connect(
			sigc::mem_fun(*this, &CanvasResize::on_rend_desc_changed));

	// Widgets
	width->signal_changed().connect(sigc::bind<Gtk::SpinButton*>(
			sigc::mem_fun(*this, &CanvasResize::on_spinbutton_updated), width));
	width->signal_value_changed().connect(
			sigc::mem_fun(*this, &CanvasResize::on_width_changed));
	height->signal_changed().connect(sigc::bind<Gtk::SpinButton*>(
			sigc::mem_fun(*this, &CanvasResize::on_spinbutton_updated), height));
	height->signal_value_changed().connect(
			sigc::mem_fun(*this, &CanvasResize::on_height_changed));
	canvas_only->signal_toggled().connect(
			sigc::mem_fun(*this, &CanvasResize::on_canvas_only_checked));
	canvas_label->add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
	canvas_label->signal_enter_notify_event().connect(
			sigc::mem_fun(*this, &CanvasResize::on_canvas_label_enter_event));
	canvas_label->signal_leave_notify_event().connect(
			sigc::mem_fun(*this, &CanvasResize::on_canvas_label_leave_event));
	canvas_label->signal_button_press_event().connect(
			sigc::mem_fun(*this, &CanvasResize::on_canvas_label_button_event));
	toggle_ratio_wh->signal_toggled().connect(
			sigc::mem_fun(*this, &CanvasResize::on_wh_ratio_toggled));
	for (auto button : canvas_buttons) {
		button->signal_clicked().connect(sigc::bind<Gtk::Button*>(
			sigc::mem_fun(*this, &CanvasResize::on_canvas_button_clicked), button));
	}

	// Action buttons
	signal_response().connect(
			sigc::mem_fun(*this, &CanvasResize::on_action_signal_response));
}

void CanvasResize::on_rend_desc_changed()
{
	rend_desc = canvas_interface->get_canvas()->rend_desc();

	refresh_content();
}

void CanvasResize::on_action_signal_response(int response_id)
{
	refresh_old_new_wh_values();

	auto rend_desc_old = canvas_interface->get_canvas()->rend_desc();
	auto was_toggled   = rend_desc_old.get_flags() & RendDesc::LINK_IM_ASPECT;
	auto is_toggled    = rend_desc    .get_flags() & RendDesc::LINK_IM_ASPECT;

	switch (response_id) {
	case ADVANCED:
		canvas_properties.present();
	case CANCEL:
	case CLOSE:
		is_canvas_checked = was_canvas_checked;
		canvas_only->set_active(was_canvas_checked);
		rend_desc = rend_desc_old;
		refresh_content();
		break;
	case OKAY:
		was_canvas_checked = is_canvas_checked;
		/*
		 * Set canvas resize direction
		 *
		 * Hack by going back to the previous dialog state and replicate new
		 * inputs to correctly resize as the user intended
		 */
		rend_desc.set_wh(old_width, old_height);
		set_canvas_center_point();
		rend_desc.set_wh(new_width, new_height);
		if (
			new_width  != old_width  ||
			new_height != old_height ||
			is_toggled != was_toggled
		) {
			rend_desc.set_pixel_ratio(new_width, new_height);
			canvas_interface->set_rend_desc(rend_desc);
		}
	}

	hide();
}

void CanvasResize::on_width_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);

	rend_desc.set_w(width->get_value_as_int());

	refresh_wh_toggle_widgets();
}

void CanvasResize::on_height_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);

	rend_desc.set_h(height->get_value_as_int());

	refresh_wh_toggle_widgets();
}

void CanvasResize::on_canvas_button_clicked(Gtk::Button *button)
{
	auto image = static_cast<Gtk::Image*>(button->get_image());

	// Return if the button clicked is already the center point
	if (image->get_name().compare("center") == 0) return;

	reset_canvas_buttons();

	image->set_name("center");

	canvas_center = button;

	refresh_old_new_wh_values();

	if (new_width < old_width && new_height < old_height)
		flip_canvas_button_images();
	else
		set_canvas_button_images();
}

bool CanvasResize::on_canvas_label_enter_event(GdkEventCrossing *event)
{
	if (event->mode != GDK_CROSSING_NORMAL) return false;

	canvas_only->set_state_flags(canvas_only->get_state_flags() | Gtk::STATE_FLAG_PRELIGHT);

	return true;
}

bool CanvasResize::on_canvas_label_leave_event(GdkEventCrossing *event)
{
	if (event->mode != GDK_CROSSING_NORMAL) return false;

	canvas_only->unset_state_flags(Gtk::STATE_FLAG_PRELIGHT);

	return true;
}

bool CanvasResize::on_canvas_label_button_event(GdkEventButton *event)
{
	if (event->button == 1 && event->type == GDK_BUTTON_PRESS) {
		canvas_only->set_active(!is_canvas_checked);
		return true;
	}

	return false;
}

void CanvasResize::on_canvas_only_checked()
{
	was_canvas_checked = is_canvas_checked;
	is_canvas_checked  = canvas_only->get_active();

	auto is_toggled    = toggle_ratio_wh->get_active();

	grid_canvas->get_parent()->set_sensitive(is_canvas_checked);

	reset_canvas_buttons();

	refresh_old_new_wh_values();

	if (is_canvas_checked) {
		new_width < old_width && new_height < old_height ? flip_canvas_button_images() : set_canvas_button_images();
		set_canvas_flags(is_toggled);
	} else {
		set_canvas_disabled_button_image();
		set_image_flags(is_toggled);
	}
}

void CanvasResize::on_wh_ratio_toggled()
{
	if (toggle_ratio_wh->get_active()) {
		rend_desc.set_pixel_ratio(width->get_value_as_int(), height->get_value_as_int());

		is_canvas_checked ? set_canvas_flags(true) : set_image_flags(true);
	} else
		is_canvas_checked ? set_canvas_flags(false) : set_image_flags(false);
}

void CanvasResize::on_spinbutton_updated(Gtk::SpinButton *widget)
{
	// Prevent widget from adding default value 1 if user fully overwrites w or h
	if (widget->get_text_length() == 0) return;

	width ->update();
	height->update();

	if (!is_canvas_checked) return;

	refresh_old_new_wh_values();

	new_width < old_width && new_height < old_height ? flip_canvas_button_images() : set_canvas_button_images();
}

void CanvasResize::refresh_title()
{
	set_title(_("Resize - ") + canvas_interface->get_canvas()->get_name());
}

void CanvasResize::refresh_content()
{
	// Canvas button grid
	int  index;
	auto coordinates = rend_desc.get_focus();
	auto x           = coordinates[0];
	auto y           = coordinates[1];

	if (x < 0 && y > 0)
		index = 0;
	else if (x == 0 && y == rend_desc.get_tl()[1])
		index = 1;
	else if (x > 0 && y > 0)
		index = 2;
	else if (x == rend_desc.get_tl()[0] && y == 0)
		index = 3;
	else if (x == rend_desc.get_br()[0] && y == 0)
		index = 5;
	else if (x < 0 && y < 0)
		index = 6;
	else if (x == 0 && y == rend_desc.get_br()[1])
		index = 7;
	else if (x > 0 && y < 0)
		index = 8;
	else
		index = 4;

	reset_canvas_buttons();

	canvas_buttons[index]->get_image()->set_name("center");

	canvas_center = canvas_buttons[index];

	is_canvas_checked ? set_canvas_button_images() : set_canvas_disabled_button_image();

	refresh_wh_toggle_widgets();
}

void CanvasResize::refresh_wh_toggle_widgets()
{
	UpdateLock lock(update_lock);

	width ->set_value(rend_desc.get_w());
	height->set_value(rend_desc.get_h());
	toggle_ratio_wh->set_active(rend_desc.get_flags() & RendDesc::LINK_IM_ASPECT);
}

void CanvasResize::refresh_old_new_wh_values()
{
	old_width  = canvas_interface->get_canvas()->rend_desc().get_w();
	old_height = canvas_interface->get_canvas()->rend_desc().get_h();
	new_width  = rend_desc.get_w();
	new_height = rend_desc.get_h();
}

void CanvasResize::reset_canvas_buttons()
{
	for (auto button : canvas_buttons) {
		auto image = static_cast<Gtk::Image*>(button->get_image());
		set_button_image(image, "resize_transparent_icon.png");
		if (!image->get_name().empty()) image->unset_name();
	}
}

void CanvasResize::set_canvas_center_point()
{
	if (canvas_center == canvas_buttons[0])
		rend_desc.set_focus(rend_desc.get_tl());
	else if (canvas_center == canvas_buttons[1])
		rend_desc.set_focus(Point(0, rend_desc.get_tl()[1]));
	else if (canvas_center == canvas_buttons[2])
		rend_desc.set_focus(Point(rend_desc.get_br()[0], rend_desc.get_tl()[1]));
	else if (canvas_center == canvas_buttons[3])
		rend_desc.set_focus(Point(rend_desc.get_tl()[0], 0));
	else if (canvas_center == canvas_buttons[5])
		rend_desc.set_focus(Point(rend_desc.get_br()[0], 0));
	else if (canvas_center == canvas_buttons[6])
		rend_desc.set_focus(Point(rend_desc.get_tl()[0], rend_desc.get_br()[1]));
	else if (canvas_center == canvas_buttons[7])
		rend_desc.set_focus(Point(0, rend_desc.get_br()[1]));
	else if (canvas_center == canvas_buttons[8])
		rend_desc.set_focus(rend_desc.get_br());
	else
		rend_desc.set_focus(Point(0, 0));
}

void CanvasResize::set_canvas_disabled_button_image()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_disabled_icon.png");
}

void CanvasResize::set_canvas_button_images()
{
	if (canvas_center == canvas_buttons[0])
		set_button_image_top_left();
	else if (canvas_center == canvas_buttons[1])
		set_button_image_top();
	else if (canvas_center == canvas_buttons[2])
		set_button_image_top_right();
	else if (canvas_center == canvas_buttons[3])
		set_button_image_left();
	else if (canvas_center == canvas_buttons[5])
		set_button_image_right();
	else if (canvas_center == canvas_buttons[6])
		set_button_image_bottom_left();
	else if (canvas_center == canvas_buttons[7])
		set_button_image_bottom();
	else if (canvas_center == canvas_buttons[8])
		set_button_image_bottom_right();
	else
		set_default_button_images();
}

void CanvasResize::set_button_image_top_left()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[1]->get_image()), "resize_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[3]->get_image()), "resize_down_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_down_right_icon.png");
}

void CanvasResize::set_button_image_top()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[0]->get_image()), "resize_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[2]->get_image()), "resize_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[3]->get_image()), "resize_down_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_down_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[5]->get_image()), "resize_down_right_icon.png");
}

void CanvasResize::set_button_image_top_right()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[1]->get_image()), "resize_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_down_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[5]->get_image()), "resize_down_icon.png");
}

void CanvasResize::set_button_image_left()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[0]->get_image()), "resize_up_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[1]->get_image()), "resize_up_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[6]->get_image()), "resize_down_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[7]->get_image()), "resize_down_right_icon.png");
}

void CanvasResize::set_button_image_right()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[1]->get_image()), "resize_up_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[2]->get_image()), "resize_up_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[7]->get_image()), "resize_down_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[8]->get_image()), "resize_down_icon.png");
}

void CanvasResize::set_button_image_bottom_left()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[3]->get_image()), "resize_up_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_up_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[7]->get_image()), "resize_right_icon.png");
}

void CanvasResize::set_button_image_bottom()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[3]->get_image()), "resize_up_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_up_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[5]->get_image()), "resize_up_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[6]->get_image()), "resize_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[8]->get_image()), "resize_right_icon.png");
}

void CanvasResize::set_button_image_bottom_right()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_up_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[5]->get_image()), "resize_up_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[7]->get_image()), "resize_left_icon.png");
}

void CanvasResize::set_default_button_images()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[0]->get_image()), "resize_up_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[1]->get_image()), "resize_up_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[2]->get_image()), "resize_up_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[3]->get_image()), "resize_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[5]->get_image()), "resize_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[6]->get_image()), "resize_down_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[7]->get_image()), "resize_down_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[8]->get_image()), "resize_down_right_icon.png");
}

void CanvasResize::flip_canvas_button_images()
{
	if (canvas_center == canvas_buttons[0])
		flip_button_image_top_left();
	else if (canvas_center == canvas_buttons[1])
		flip_button_image_top();
	else if (canvas_center == canvas_buttons[2])
		flip_button_image_top_right();
	else if (canvas_center == canvas_buttons[3])
		flip_button_image_left();
	else if (canvas_center == canvas_buttons[5])
		flip_button_image_right();
	else if (canvas_center == canvas_buttons[6])
		flip_button_image_bottom_left();
	else if (canvas_center == canvas_buttons[7])
		flip_button_image_bottom();
	else if (canvas_center == canvas_buttons[8])
		flip_button_image_bottom_right();
	else
		flip_default_button_images();
}

void CanvasResize::flip_button_image_top_left()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[1]->get_image()), "resize_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[3]->get_image()), "resize_up_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_up_left_icon.png");
}

void CanvasResize::flip_button_image_top()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[0]->get_image()), "resize_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[2]->get_image()), "resize_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[3]->get_image()), "resize_up_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_up_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[5]->get_image()), "resize_up_left_icon.png");
}

void CanvasResize::flip_button_image_top_right()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[1]->get_image()), "resize_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_up_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[5]->get_image()), "resize_up_icon.png");
}

void CanvasResize::flip_button_image_left()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[0]->get_image()), "resize_down_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[1]->get_image()), "resize_down_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[6]->get_image()), "resize_up_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[7]->get_image()), "resize_up_left_icon.png");
}

void CanvasResize::flip_button_image_right()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[1]->get_image()), "resize_down_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[2]->get_image()), "resize_down_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[7]->get_image()), "resize_up_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[8]->get_image()), "resize_up_icon.png");
}

void CanvasResize::flip_button_image_bottom_left()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[3]->get_image()), "resize_down_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_down_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[7]->get_image()), "resize_left_icon.png");
}

void CanvasResize::flip_button_image_bottom()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[3]->get_image()), "resize_down_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_down_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[5]->get_image()), "resize_down_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[6]->get_image()), "resize_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[8]->get_image()), "resize_left_icon.png");
}

void CanvasResize::flip_button_image_bottom_right()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[4]->get_image()), "resize_down_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[5]->get_image()), "resize_down_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[7]->get_image()), "resize_right_icon.png");
}

void CanvasResize::flip_default_button_images()
{
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[0]->get_image()), "resize_down_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[1]->get_image()), "resize_down_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[2]->get_image()), "resize_down_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[3]->get_image()), "resize_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[5]->get_image()), "resize_left_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[6]->get_image()), "resize_up_right_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[7]->get_image()), "resize_up_icon.png");
	set_button_image(static_cast<Gtk::Image*>(canvas_buttons[8]->get_image()), "resize_up_left_icon.png");
}

void CanvasResize::set_button_image(Gtk::Image *image, const std::string icon_file)
{
	auto pixbuf = Gdk::Pixbuf::create_from_file(ResourceHelper::get_icon_path(icon_file), 32, 32);

	image->set(pixbuf);
	image->show();
}

void CanvasResize::set_image_flags(bool is_wh_linked)
{
	rend_desc.set_flags(RendDesc::IM_SPAN);

	if (is_wh_linked)
		rend_desc.set_flags(rend_desc.get_flags() |  RendDesc::LINK_IM_ASPECT);
	else {
		rend_desc.set_flags(rend_desc.get_flags() & ~RendDesc::LINK_IM_ASPECT);
		rend_desc.set_flags(rend_desc.get_flags() |  RendDesc::PX_ASPECT);
	}
}

void CanvasResize::set_canvas_flags(bool is_wh_linked)
{
	rend_desc.set_flags(RendDesc::PX_ASPECT);

	if (is_wh_linked)
		rend_desc.set_flags(rend_desc.get_flags() |  RendDesc::LINK_IM_ASPECT);
	else
		rend_desc.set_flags(rend_desc.get_flags() & ~RendDesc::LINK_IM_ASPECT);
}
