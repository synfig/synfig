/* === S Y N F I G ========================================================= */
/*!	\file selectdraghelper.h
**	\brief Helper to allow to select and drag items in a widget, eg. DrawingArea
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2019 Rodolfo R Gomes
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

#ifndef SYNFIG_STUDIO_SELECTDRAGHELPER_H
#define SYNFIG_STUDIO_SELECTDRAGHELPER_H

#include <vector>
#include <cairomm/context.h>
#include <gdkmm/rectangle.h>
#include <gdkmm/event.h>

#include <ETL/handle>
#include <synfigapp/canvasinterface.h>
#include "app.h"

namespace synfigapp {
namespace Action {
class PassiveGrouper;
}
}

namespace studio {

template <class T>
class SelectDragHelper
{
public:
	enum State {POINTER_NONE, POINTER_DRAGGING, POINTER_SELECTING};

private:
	etl::handle<synfigapp::CanvasInterface> canvas_interface;
	const std::string drag_action_name;
	synfigapp::Action::PassiveGrouper *action_group_drag;

	bool made_dragging_move;
	bool dragging_started_by_key;

	T hovered_item;
	std::vector<T> selected_items;
	const T* active_item;

	State pointer_state;
	int pointer_tracking_start_x, pointer_tracking_start_y;

	void start_dragging(const T *pointed_item);
	void finish_dragging();
	void cancel_dragging();

	bool process_key_press_event(GdkEventKey *event);
	bool process_key_release_event(GdkEventKey *event);
	bool process_button_press_event(GdkEventButton *event);
	bool process_button_release_event(GdkEventButton *event);
	bool process_motion_event(GdkEventMotion *event);
	bool process_scroll_event(GdkEventScroll *event);

	sigc::signal<void> signal_selection_changed_;

	sigc::signal<void> signal_zoom_in_requested_;
	sigc::signal<void> signal_zoom_out_requested_;

	sigc::signal<void> signal_scroll_up_requested_;
	sigc::signal<void> signal_scroll_down_requested_;

	sigc::signal<void> signal_redraw_needed_;

	sigc::signal<void> signal_focus_requested_;

	sigc::signal<void> signal_drag_started_;
	sigc::signal<void> signal_drag_canceled_;
	sigc::signal<void> signal_drag_finished_;

	sigc::signal<void, const T&, unsigned int, Gdk::Point> signal_item_clicked_;

public:
	SelectDragHelper(const char *drag_action_name);
	virtual ~SelectDragHelper() {delete action_group_drag;}

	void set_canvas_interface(etl::handle<synfigapp::CanvasInterface> canvas_interface);
	etl::handle<synfigapp::CanvasInterface>& get_canvas_interface();

	const T& get_hovered_item() const;
	std::vector<T*> get_selected_items();
	bool is_selected(const T& item) const;
	const T& get_active_item() const;

	State get_state() const;
	void get_initial_tracking_point(int &px, int &py) const;

	virtual bool find_item_at_position(int pos_x, int pos_y, T & cp) = 0;
	virtual bool find_items_in_rect(Gdk::Rectangle rect, std::vector<T> & list) = 0;
	virtual void get_all_items(std::vector<T> & items) = 0;

	void drag_to(int pointer_x, int pointer_y);
	virtual void delta_drag(int dx, int dy, bool by_keys) = 0;

	bool process_event(GdkEvent *event);

	void refresh();
	void clear();

	void select_all_items();

	sigc::signal<void>& signal_selection_changed() { return signal_selection_changed_; }

	sigc::signal<void>& signal_zoom_in_requested() { return signal_zoom_in_requested_; }
	sigc::signal<void>& signal_zoom_out_requested() { return signal_zoom_out_requested_; }

	sigc::signal<void>& signal_scroll_up_requested() { return signal_scroll_up_requested_; }
	sigc::signal<void>& signal_scroll_down_requested() { return signal_scroll_down_requested_; }

	sigc::signal<void>& signal_redraw_needed() { return signal_redraw_needed_; }

	sigc::signal<void>& signal_focus_requested() { return signal_focus_requested_; }

	sigc::signal<void>& signal_drag_started() { return signal_drag_started_; }
	sigc::signal<void>& signal_drag_canceled() { return signal_drag_canceled_; }
	sigc::signal<void>& signal_drag_finished() { return signal_drag_finished_; }

	sigc::signal<void, const T&, unsigned int, Gdk::Point>& signal_item_clicked() { return signal_item_clicked_; }
};


template <class T>
SelectDragHelper<T>::SelectDragHelper(const char* drag_action_name)
	: drag_action_name(drag_action_name), action_group_drag(nullptr), active_item(nullptr), pointer_state(POINTER_NONE)
{
}

template<class T>
void SelectDragHelper<T>::set_canvas_interface(etl::handle<synfigapp::CanvasInterface> canvas_interface)
{
	if (pointer_state == POINTER_DRAGGING) {
		cancel_dragging();
	}
	this->canvas_interface = canvas_interface;
}

template<class T>
etl::handle<synfigapp::CanvasInterface>& SelectDragHelper<T>::get_canvas_interface()
{
	return canvas_interface;
}

template <class T>
const T& SelectDragHelper<T>::get_hovered_item() const
{
	return hovered_item;
}

template<class T>
std::vector<T*> SelectDragHelper<T>::get_selected_items()
{
	const size_t nselection = selected_items.size();
	std::vector<T*> r;
	r.reserve(nselection);
	for (size_t n = 0; n < nselection; n++)
		r.push_back(&selected_items[n]);
	return r;
}

template<class T>
bool SelectDragHelper<T>::is_selected(const T& item) const {
	return std::find(selected_items.begin(), selected_items.end(), item) != selected_items.end();
}

template<class T>
const T& SelectDragHelper<T>::get_active_item() const {
	return *active_item;
}

template<class T>
typename SelectDragHelper<T>::State SelectDragHelper<T>::get_state() const
{
	return pointer_state;
}

template<class T>
void SelectDragHelper<T>::get_initial_tracking_point(int& px, int& py) const
{
	px = pointer_tracking_start_x;
	py = pointer_tracking_start_y;
}


template <class T>
bool
SelectDragHelper<T>::process_event(GdkEvent *event)
{
	switch(event->type) {
	case GDK_SCROLL:
		return process_scroll_event(&event->scroll);
	case GDK_MOTION_NOTIFY:
		return process_motion_event(&event->motion);
	case GDK_BUTTON_PRESS:
		return process_button_press_event(&event->button);
	case GDK_BUTTON_RELEASE:
		return process_button_release_event(&event->button);
	case GDK_KEY_RELEASE:
		return process_key_release_event(&event->key);
	case GDK_KEY_PRESS:
		return process_key_press_event(&event->key);
	default:
		break;
	}

	return false;
}

template<class T>
bool SelectDragHelper<T>::process_key_press_event(GdkEventKey* event)
{
	switch (event->keyval) {
	case GDK_KEY_Up:
	case GDK_KEY_Down: {
		if (selected_items.size() == 0)
			break;
		if (pointer_state != POINTER_DRAGGING) {
			start_dragging(&selected_items.front());
			dragging_started_by_key = true;
		}
		int delta = 1;
		if (event->state & GDK_SHIFT_MASK)
			delta = 10;
		if (event->keyval == GDK_KEY_Up)
			delta = -delta;
		delta_drag(0, delta, true);
		return true;
	}
	case GDK_KEY_Left:
	case GDK_KEY_Right: {
		if (selected_items.size() == 0)
			break;
		if (pointer_state != POINTER_DRAGGING) {
			start_dragging(&selected_items.front());
			dragging_started_by_key = true;
		}
		int delta = 1;
		if (event->state & GDK_SHIFT_MASK)
			delta *= 10;
		if (event->keyval == GDK_KEY_Left)
			delta = -delta;
		delta_drag(delta, 0, true);
		return true;
	}
	case GDK_KEY_a: {
		if ((event->state & Gdk::CONTROL_MASK) == Gdk::CONTROL_MASK) {
			// ctrl a
			cancel_dragging();
			select_all_items();
			return true;
		}
		break;
	}
	case GDK_KEY_d: {
		if ((event->state & Gdk::CONTROL_MASK) == Gdk::CONTROL_MASK) {
			// ctrl d
			cancel_dragging();
			selected_items.clear();
			signal_redraw_needed().emit();
			signal_selection_changed().emit();
			return true;
		}
		break;
	}
	}
	return false;
}

template<class T>
bool SelectDragHelper<T>::process_key_release_event(GdkEventKey* event)
{
	switch (event->keyval) {
	case GDK_KEY_Escape: {
		// cancel/undo current action
		if (pointer_state != POINTER_NONE) {
			cancel_dragging();
			pointer_state = POINTER_NONE;
			signal_redraw_needed().emit();
			return true;
		}
	}
	case GDK_KEY_Up:
	case GDK_KEY_Down:
	case GDK_KEY_Left:
	case GDK_KEY_Right: {
		if (pointer_state == POINTER_DRAGGING) {
			if (dragging_started_by_key)
				finish_dragging();
			dragging_started_by_key = false;
			return true;
		}
	}
	}
	return false;
}

template<class T>
bool SelectDragHelper<T>::process_button_press_event(GdkEventButton* event)
{
	bool some_action_done = false;

	signal_focus_requested().emit();
	if (event->button == 3) {
		// cancel/undo current action
		if (pointer_state != POINTER_NONE) {
			cancel_dragging();
			pointer_state = POINTER_NONE;
			signal_redraw_needed().emit();
			some_action_done = true;
		}
	} else if (event->button == 1) {
		if (pointer_state == POINTER_NONE) {
			pointer_tracking_start_x = std::trunc(event->x);
			pointer_tracking_start_y = std::trunc(event->y);
			T pointed_item;
			find_item_at_position(pointer_tracking_start_x, pointer_tracking_start_y, pointed_item);
			if (pointed_item.is_valid()) {
				auto already_selection_it = std::find(selected_items.begin(), selected_items.end(), pointed_item);
				bool is_already_selected = already_selection_it != selected_items.end();
				bool using_key_modifiers = (event->state & (GDK_CONTROL_MASK|GDK_SHIFT_MASK)) != 0;
				if (using_key_modifiers) {
					pointer_state = POINTER_SELECTING;
				} else {
					T* pointed_item_ptr = nullptr;
					if (is_already_selected) {
						pointed_item_ptr = &*already_selection_it;
					} else {
						selected_items.clear();
						selected_items.push_back(pointed_item);
						pointed_item_ptr = &selected_items.front();
						signal_selection_changed().emit();
					}
					start_dragging(pointed_item_ptr);
					dragging_started_by_key = false;
					pointer_state = POINTER_DRAGGING;
				}
			} else {
				pointer_state = POINTER_SELECTING;
			}
			some_action_done = true;
		}
	}

	{
		T pointed_item;
		find_item_at_position(event->x, event->y, pointed_item);
		if (pointed_item.is_valid()) {
			int pointer_x = std::trunc(event->x);
			int pointer_y = std::trunc(event->y);
			signal_item_clicked().emit(pointed_item, event->button, Gdk::Point(pointer_x, pointer_y));
		}
	}
	return some_action_done;
}

template<class T>
bool SelectDragHelper<T>::process_button_release_event(GdkEventButton* event)
{
	int pointer_x = std::trunc(event->x);
	int pointer_y = std::trunc(event->y);

	if (event->button == 1) {
		bool selection_changed = false;

		if (pointer_state == POINTER_SELECTING) {
			std::vector<T> circled_items;
			int x0 = std::min(pointer_tracking_start_x, pointer_x);
			int width = std::abs(pointer_tracking_start_x - pointer_x);
			int y0 = std::min(pointer_tracking_start_y, pointer_y);
			int height = std::abs(pointer_tracking_start_y - pointer_y);
			if (width < 1 && height < 1) {
				width = 1;
				height = 1;
			}
			Gdk::Rectangle rect(x0, y0, width, height);
			bool found = find_items_in_rect(rect, circled_items);
			if (!found) {
				if (selected_items.size() > 0 && (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) == 0) {
					selection_changed = true;
					selected_items.clear();
				}
			} else {
				if ((event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK) {
					// toggle selection status of each point in rectangle
					for (const T& cp : circled_items) {
						auto already_selection_it = std::find(selected_items.begin(), selected_items.end(), cp);
						bool already_selected = already_selection_it != selected_items.end();
						if (already_selected) {
							selected_items.erase(already_selection_it);
							selection_changed = true;
						} else {
							selected_items.push_back(cp);
							selection_changed = true;
						}
					}
				} else if ((event->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK) {
					// add to selection, if it aren't yet
					for (const T& cp : circled_items) {
						auto already_selection_it = std::find(selected_items.begin(), selected_items.end(), cp);
						bool already_selected = already_selection_it != selected_items.end();
						if (!already_selected) {
							selected_items.push_back(cp);
							selection_changed = true;
						}
					}
				} else {
					selected_items.clear();
					selected_items = circled_items;
					selection_changed = true;
				}
			}
		}
		else if (pointer_state == POINTER_DRAGGING) {
			if (event->button == 1) {
				if (made_dragging_move)
					finish_dragging();
				else {
					T saved_active_point = *active_item;
					selected_items.clear();
					selected_items.push_back(saved_active_point);
					selection_changed = true;
					cancel_dragging();
				}
			}
		}

		if (selection_changed) {
			signal_selection_changed().emit();
			signal_redraw_needed().emit();
		}
	}

	if (event->button == 1 || event->button == 3) {
		if (pointer_state != POINTER_NONE) {
			pointer_state = POINTER_NONE;
			signal_redraw_needed().emit();
		}
	}

	return false;
}

template<class T>
bool SelectDragHelper<T>::process_motion_event(GdkEventMotion* event)
{
	auto previous_hovered_point = hovered_item;
	hovered_item.invalidate();

	int pointer_x = std::trunc(event->x);
	int pointer_y = std::trunc(event->y);
	if (pointer_state != POINTER_DRAGGING)
		find_item_at_position(pointer_x, pointer_y, hovered_item);

	if (previous_hovered_point != hovered_item)
		signal_redraw_needed().emit();

	if (pointer_state == POINTER_DRAGGING && !dragging_started_by_key) {
		guint any_pointer_button = Gdk::BUTTON1_MASK |Gdk::BUTTON2_MASK | Gdk::BUTTON3_MASK;
		if ((event->state & any_pointer_button) == 0) {
			// If some modal window is called, we lose the button-release event...
			cancel_dragging();
		} else {
			bool axis_lock = event->state & Gdk::SHIFT_MASK;
			if (axis_lock) {
				int dx = pointer_x - pointer_tracking_start_x;
				int dy = pointer_y - pointer_tracking_start_y;
				if (std::abs(dy) > std::abs(dx))
					pointer_x = pointer_tracking_start_x;
				else
					pointer_y = pointer_tracking_start_y;
			}
			drag_to(pointer_x, pointer_y);
		}
	}
	if (pointer_state != POINTER_NONE) {
		signal_redraw_needed().emit();
	}
	return true;
}

template<class T>
bool SelectDragHelper<T>::process_scroll_event(GdkEventScroll* event)
{
	switch(event->direction) {
		case GDK_SCROLL_UP:
		case GDK_SCROLL_RIGHT: {
			if (event->state & GDK_CONTROL_MASK) {
				// Ctrl+scroll , perform zoom in
				signal_zoom_in_requested().emit();
			} else {
				// Scroll up
				signal_scroll_up_requested().emit();
			}
			return true;
		}
		case GDK_SCROLL_DOWN:
		case GDK_SCROLL_LEFT: {
			if (event->state & GDK_CONTROL_MASK) {
				// Ctrl+scroll , perform zoom out
				signal_zoom_out_requested().emit();
			} else {
				// Scroll down
				signal_scroll_down_requested().emit();
			}
			return true;
		}
		default:
			break;
	}
	return false;
}

template <class T>
void SelectDragHelper<T>::refresh() {
	hovered_item.invalidate();
}

template <class T>
void SelectDragHelper<T>::clear() {
	if (pointer_state == POINTER_DRAGGING) {
		cancel_dragging();
	}
	hovered_item.invalidate();
	if (!selected_items.empty()) {
		selected_items.clear();
		signal_selection_changed().emit();
	}
}

template <class T>
void SelectDragHelper<T>::start_dragging(const T* pointed_item)
{
	made_dragging_move = false;
	active_item = pointed_item;

	if (canvas_interface) {
		action_group_drag = new synfigapp::Action::PassiveGrouper(canvas_interface->get_instance().get(), drag_action_name);
	}

	pointer_state = POINTER_DRAGGING;

	signal_drag_started().emit();
}

template <class T>
void SelectDragHelper<T>::drag_to(int pointer_x, int pointer_y)
{
	made_dragging_move = true;

	int pointer_dx = pointer_x - pointer_tracking_start_x;
	int pointer_dy = pointer_y - pointer_tracking_start_y;
	delta_drag(pointer_dx, pointer_dy, false);
}

template <class T>
void SelectDragHelper<T>::finish_dragging()
{
	delete action_group_drag;
	action_group_drag = nullptr;

	pointer_state = POINTER_NONE;

	signal_drag_finished().emit();
}

template <class T>
void SelectDragHelper<T>::cancel_dragging()
{
	if (pointer_state != POINTER_DRAGGING)
		return;

	// Sadly group->cancel() just remove PassiverGroup indicator, not its actions, from stack

	bool has_any_content =  0 < action_group_drag->get_depth();
	delete action_group_drag;
	action_group_drag = nullptr;
	if (has_any_content && canvas_interface) {
		canvas_interface->get_instance()->undo();
		canvas_interface->get_instance()->clear_redo_stack();
	}

	pointer_state = POINTER_NONE;
	signal_drag_canceled().emit();
	signal_redraw_needed().emit();
}

template <class T>
void SelectDragHelper<T>::select_all_items()
{
	cancel_dragging();
	selected_items.clear();
	std::vector<T> all_items;
	get_all_items(all_items);
	selected_items = std::move(all_items);
	signal_selection_changed().emit();
}


};

#endif // SYNFIG_STUDIO_SELECTDRAGHELPER_H
