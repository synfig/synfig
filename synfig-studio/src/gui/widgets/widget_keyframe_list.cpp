/* === S Y N F I G ========================================================= */
/*!	\file widget_keyframe_list.cpp
**	\brief A custom widget to manage keyframes in the timeline.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2009 Carlos López
**	Copyright (c) 2012-2013 Konstantin Dmitriev
**	......... ... 2018 Ivan Mahonin
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

#include <gui/widgets/widget_keyframe_list.h>

#include <gtkmm/menu.h>
#include <gtkmm/object.h>

#include <gui/app.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
//#include <synfig/general.h>
#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace synfigapp;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Keyframe*
Widget_Keyframe_List::get_keyframe_around(Time t, bool ignore_disabled)
{
	KeyframeList::iterator ret(kf_list->end());
	Keyframe* kf = nullptr;

	Time p_t, n_t;
	kf_list->find_prev_next(t, p_t, n_t, ignore_disabled);

	const Time keyframe_mark_time_length = time_plot_data.get_delta_t_from_delta_pixel_coord(keyframe_width);

	Time delta_to_previous = t - p_t;
	Time delta_to_next = n_t - t;

	// near to previous keyframe?
	if (delta_to_previous < keyframe_mark_time_length/2) {
		// is it nearest to previous one than to next one?
		if (delta_to_previous < delta_to_next) {
			if (kf_list->find(p_t, ret))
				kf = &*ret;
		}
	}

	if (!kf && delta_to_next < keyframe_mark_time_length/2) {
		if (kf_list->find(n_t, ret))
			kf = &*ret;
	}

	return kf;
}

Widget_Keyframe_List::Widget_Keyframe_List():
	kf_list(),
	editable(true),
	dragging(),
	changed(),
	selected(),
	moving_tooltip(Gtk::WINDOW_POPUP),
	time_plot_data(this),
	keyframe_width(4)
	//moving_tooltip_y()
{
	set_size_request(-1, 10);
	add_events( Gdk::BUTTON_PRESS_MASK
			  | Gdk::BUTTON_RELEASE_MASK
			  | Gdk::BUTTON1_MOTION_MASK
			  | Gdk::POINTER_MOTION_MASK );

	// Window of the moving tooltip

	moving_tooltip_label.set_halign(Gtk::ALIGN_CENTER);
	moving_tooltip_label.set_valign(Gtk::ALIGN_CENTER);
	moving_tooltip_label.show();

	moving_tooltip.set_resizable(false);
	moving_tooltip.set_name("gtk-tooltips");
	moving_tooltip.set_border_width(4);
	moving_tooltip.set_default_size(10, 10);
	moving_tooltip.set_type_hint(Gdk::WINDOW_TYPE_HINT_TOOLTIP);
	moving_tooltip.set_transient_for(*App::main_window);
	moving_tooltip.add(moving_tooltip_label);
}

Widget_Keyframe_List::~Widget_Keyframe_List()
{
	set_kf_list(nullptr);
	set_canvas_interface(etl::loose_handle<CanvasInterface>());
}

void
Widget_Keyframe_List::draw_arrow(
	const Cairo::RefPtr<Cairo::Context> &cr,
	double x, double y,
	double width, double height,
	bool fill,
	const Color &color )
{
	cr->save();
	cr->set_source_rgba(color.get_r(), color.get_g(), color.get_b(), color.get_a());
	cr->set_line_width(1.0);
	cr->move_to(x, y);
	cr->line_to(x - 0.5*width, y - height);
	cr->line_to(x + 0.5*width, y - height);
	cr->close_path();
	if (fill) cr->fill(); else cr->stroke();
	cr->restore();
}

bool
Widget_Keyframe_List::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	// Check if the window we want draw is ready
	if (!time_plot_data.time_model)
		return false;

	// TODO: hardcoded colors
	// Colors
	Color background(0.46, 0.55, 0.70, 1.0);
	Color normal(0.0, 0.0, 0.0, 1.0);
	Color selected(1.0, 1.0, 1.0, 1.0);
	Color drag_old_position(1.0, 1.0, 1.0, 0.6);
	Color drag_new_position(1.0, 1.0, 1.0, 1.0);

	if (!editable) {
		normal.set_a(0.5);
		selected.set_a(0.5);
		drag_old_position.set_a(0.3);
		drag_new_position.set_a(0.5);
	}

	const double h(get_height());
	const double w(get_width());
	const double y(h - 2);
	const double ah(h - 4);
	const double aw(2*ah);

	keyframe_width = aw;

	if (w <= 0 || h <= 0)
		return false;

	// Draw a background
	cr->save();
	cr->set_source_rgba(background.get_r(), background.get_g(), background.get_b(), background.get_a());
	cr->rectangle(0.0, 0.0, w, h);
	cr->fill();
	cr->restore();

	// Returns if there are not keyframes to draw.
	if (!kf_list || kf_list->empty()) return true;

	// draw all keyframes
	Time selected_time = Time::end();
	for(KeyframeList::const_iterator i = kf_list->begin(); i != kf_list->end(); ++i)
		if (time_plot_data.is_time_visible_extra(i->get_time())) {
			if (*i == selected_kf) {
				selected_time = i->get_time();
			} else {
				const double x = time_plot_data.get_pixel_t_coord(i->get_time());
				draw_arrow(cr, x, y, aw, ah, i->active(), normal);
			}
		}

	// we do this so that we can be sure that
	// the selected keyframe is shown on top
	if (selected_time != Time::end()) {
		const double x = time_plot_data.get_pixel_t_coord(selected_time);
		if (dragging) {
			const double new_x = time_plot_data.get_pixel_t_coord(dragging_kf_time);
			draw_arrow(cr, x, y, aw, ah, selected_kf.active(), drag_old_position);
			draw_arrow(cr, new_x, y, aw, ah, selected_kf.active(), drag_new_position);
		} else {
			draw_arrow(cr, x, y, aw, ah, selected_kf.active(), selected);
		}
	}

	return true;
}

void
Widget_Keyframe_List::set_kf_list(KeyframeList *x)
{
	if (kf_list == x) return;
	kf_list = x;
	reset_selected_keyframe();
}

void
Widget_Keyframe_List::reset_selected_keyframe()
{
	selected = false;
	dragging = false;
}

void
Widget_Keyframe_List::set_selected_keyframe(const Keyframe &x)
{
	if (selected && x == selected_kf) {
		// Keyframe::operator== only on uniqueid::operator==
		// \see UniqueID::operator==
		// In all case, refresh keyframe description, time and activation status to do not loose it
		selected_kf = x;
		return;
	}

	selected_kf = x;
	selected = true;
	dragging = false;
	dragging_kf_time = selected_kf.get_time();

	if (canvas_interface)
		canvas_interface->signal_keyframe_selected()(selected_kf);

	queue_draw();
}

void
Widget_Keyframe_List::on_keyframe_selected(Keyframe keyframe)
	{ set_selected_keyframe(keyframe); }

bool
Widget_Keyframe_List::perform_move_kf(bool delta)
{
	if (!kf_list)
		return false;
	if (!selected)
		return false;
	if (dragging_kf_time == selected_kf.get_time())
		return false; // change this checking if not sticked to integer frames

	Time selected_kf_time(selected_kf.get_time());
	Time prev, next;
	kf_list->find_prev_next(selected_kf_time, prev, next);

	// Not possible to set delta to the first keyframe
	// perform normal movement
	// As suggested by Zelgadis it is better to not perform anything.
	if (prev == Time::begin() && delta) {
		info(_("Not possible to ALT-drag the first keyframe"));
		return false;
	}

	if (!delta) {
		Action::Handle action(Action::create("KeyframeSet"));
		if (!action) return false;
		selected_kf.set_time(dragging_kf_time);
		action->set_param("canvas", canvas_interface->get_canvas());
		action->set_param("canvas_interface", canvas_interface);
		action->set_param("keyframe", selected_kf);
		try {
			canvas_interface->get_instance()->perform_action(action);
			canvas_interface->signal_keyframe_selected()(selected_kf);
		} catch(...) { return false; }
	} else {
		// find prev from selected kf time including deactivated kf
		KeyframeList::iterator iter;
		if (kf_list->find_prev(selected_kf_time, iter, false)) {
			//Keyframe prev_kf(*kf_list->find_prev(selected_kf_time, false));
			Keyframe prev_kf(*iter);
			Time prev_kf_time(prev_kf.get_time());
			if (prev_kf_time >= dragging_kf_time) { // Not allowed
				warning(_("Delta set not allowed"));
				info("Widget_Keyframe_List::perform_move_kf(%i)::prev_kf_time=%s", delta, prev_kf_time.get_string().c_str());
				info("Widget_Keyframe_List::perform_move_kf(%i)::dragging_kf_time=%s", delta, dragging_kf_time.get_string().c_str());
				return false;
			} else {
				Time old_delta_time(selected_kf_time-prev_kf_time);
				Time new_delta_time(dragging_kf_time-prev_kf_time);
				Time change_delta(new_delta_time-old_delta_time);
				Action::Handle action(Action::create("KeyframeSetDelta"));
				if (!action)
					return false;
				action->set_param("canvas", canvas_interface->get_canvas());
				action->set_param("canvas_interface", canvas_interface);
				action->set_param("keyframe", prev_kf);
				action->set_param("delta", change_delta);
				canvas_interface->get_instance()->perform_action(action);
			}

		}
	}

	queue_draw();
	return true;
}

bool
Widget_Keyframe_List::on_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	if (!time_plot_data.time_model || get_width() <= 0 || !kf_list || !editable)
		return false;

	etl::handle<TimeModel> time_model = time_plot_data.time_model;

	const int x = (int)event->button.x;

	// Boundaries of the drawing area in time units.
	Time lower(time_model->get_visible_lower());
	Time upper(time_model->get_visible_upper());

	// The time where the event x is
	Time t = time_plot_data.get_t_from_pixel_coord(x);
	t = synfig::clamp(t, lower, upper);

	// here the guts of the event
	switch(event->type) {
	case GDK_MOTION_NOTIFY: {
		if (event->motion.state & GDK_BUTTON1_MASK) {
			// here is captured mouse motion
			// AND left or right mouse button pressed

			if (!selected)
				return true;

			// stick to integer frames
			t = time_model->round_time(t);

			dragging_kf_time = t;
			dragging = true;

			// Moving tooltip displaying the dragging time
			int x_root = static_cast<int>(event->button.x_root);
			int x_origin; int y_origin;
			get_window()->get_origin (x_origin, y_origin);

			moving_tooltip_label.set_text(
				String(_("Time: "))
			  + dragging_kf_time.get_string(time_model->get_frame_rate(), App::get_time_format())
			  + "\n"
			  + _("Old Time: ")
			  + selected_kf.get_time().get_string(time_model->get_frame_rate(), App::get_time_format()) );

			// Show the tooltip and move to a nice position
			if (!moving_tooltip.get_visible()) moving_tooltip.show();
			moving_tooltip.move(x_root, y_origin - moving_tooltip.get_height());
		} else {
			// here is captured mouse motion
			// AND NOT left or right mouse button pressed
			String ttip;
			Keyframe* kf = get_keyframe_around(t);
			if (!kf)
				ttip = _("Click and drag keyframes");
			else
				ttip = kf->get_description().empty() ? String(_("No name")) : kf->get_description();
			set_tooltip_text(ttip);
			dragging = false;
		}
		queue_draw();
		return true;
	}
	case GDK_BUTTON_PRESS: {
		changed = false;
		dragging = false;

		const Keyframe *kf = get_keyframe_around(t, false);

		switch(event->button.button) {
		case 1:
			if (kf) set_selected_keyframe(*kf); else reset_selected_keyframe();
			break;
		case 3:
			if (kf) set_selected_keyframe(*kf);
			if (Gtk::Menu* menu = Gtk::manage(new Gtk::Menu(App::menu_keyframe))) {
				menu->attach_to_widget(*this);
				menu->popup(event->button.button, gtk_get_current_event_time());
			} else {
				synfig::error(_("Internal error: couldn't instantiate menu Add Layer to pop it up."));
			}
			break;
		default:
			return false;
		}

		queue_draw();
		return true;
	}
	case GDK_2BUTTON_PRESS: {
		if (event->button.button == 1) {
			if (selected && canvas_interface)
				canvas_interface->signal_keyframe_properties()();
			return true;
		}
		break;
	}
	case GDK_BUTTON_RELEASE: {
		if (event->button.button == 1 && dragging) {
			moving_tooltip.hide();
			perform_move_kf((bool)(event->button.state & GDK_MOD1_MASK));
			dragging = false;
		}
		if (event->button.button == 1 || event->button.button == 3)
			return true; // we captured press of buttons 1 and 3, so capture release too
		break;
	}
	default:
		break;
	}

	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}


void Widget_Keyframe_List::set_time_model(const etl::handle<TimeModel> &x)
{
	time_plot_data.set_time_model(x);
//		time_model_change = x->signal_visible_changed().connect(
//			sigc::mem_fun(*this, &Widget_Keyframe_List::queue_draw) );
}

void
Widget_Keyframe_List::set_canvas_interface(const etl::loose_handle<CanvasInterface> &x)
{
	if (canvas_interface == x) return;

	keyframe_added.disconnect();
	keyframe_changed.disconnect();
	keyframe_removed.disconnect();
	keyframe_selected.disconnect();

	canvas_interface = x;
	set_kf_list(canvas_interface ? &canvas_interface->get_canvas()->keyframe_list() : nullptr);

	if (canvas_interface) {
		keyframe_added = canvas_interface->signal_keyframe_added().connect(
			sigc::hide_return(
				sigc::hide(
					sigc::mem_fun(*this,&Widget_Keyframe_List::queue_draw) )));
		keyframe_changed = canvas_interface->signal_keyframe_changed().connect(
			sigc::hide_return(
				sigc::hide(
					sigc::mem_fun(*this,&Widget_Keyframe_List::queue_draw) )));
		keyframe_removed = canvas_interface->signal_keyframe_removed().connect(
			sigc::hide_return(
				sigc::hide(
					sigc::mem_fun(*this,&Widget_Keyframe_List::queue_draw) )));
		keyframe_selected = canvas_interface->signal_keyframe_selected().connect(
				sigc::mem_fun(*this,&Widget_Keyframe_List::on_keyframe_selected) );
	}
}
