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
	enum State {POINTER_NONE, POINTER_DRAGGING, POINTER_SELECTING, POINTER_PANNING};

private:
	etl::handle<synfigapp::CanvasInterface> canvas_interface;
	const std::string drag_action_name;
	synfigapp::Action::PassiveGrouper *action_group_drag;

	bool made_dragging_move;
	bool dragging_started_by_key;

	T hovered_item;
	bool is_hovered_item_valid;
	std::vector<T> selected_items;
	const T* active_item;

	State pointer_state;
	int pointer_tracking_start_x, pointer_tracking_start_y;
	int last_pointer_x, last_pointer_y;
	Gdk::Point active_item_start_position;

	Gdk::ModifierType modifiers;

	void start_tracking_pointer(int x, int y);
	void start_tracking_pointer(double x, double y);

	void start_dragging(const T *pointed_item);
	void finish_dragging();
	void cancel_dragging();

	bool process_key_press_event(GdkEventKey *event);
	bool process_key_release_event(GdkEventKey *event);
	bool process_button_double_press_event(GdkEventButton *event);
	bool process_button_press_event(GdkEventButton *event);
	bool process_button_release_event(GdkEventButton *event);
	bool process_motion_event(GdkEventMotion *event);
	bool process_scroll_event(GdkEventScroll *event);

	sigc::signal<void> signal_selection_changed_;
	sigc::signal<void> signal_hovered_item_changed_;

	sigc::signal<void> signal_zoom_in_requested_;
	sigc::signal<void> signal_zoom_out_requested_;

	sigc::signal<void> signal_scroll_up_requested_;
	sigc::signal<void> signal_scroll_down_requested_;

	sigc::signal<void, int, int, int, int> signal_panning_requested_;

	sigc::signal<void> signal_redraw_needed_;

	sigc::signal<void> signal_focus_requested_;

	sigc::signal<void> signal_drag_started_;
	sigc::signal<void> signal_drag_canceled_;
	sigc::signal<void, bool> signal_drag_finished_;

	sigc::signal<void, const T&, unsigned int, Gdk::Point> signal_item_clicked_;
	sigc::signal<void, const T&, unsigned int, Gdk::Point> signal_item_double_clicked_;

	sigc::signal<void> signal_modifier_keys_changed_;

	bool box_selection_enabled = true;
	bool multiple_selection_enabled = true;
	bool scroll_enabled = false;
	bool zoom_enabled = false;
	bool pan_enabled = false;
	bool drag_enabled = true;

protected:
	synfigapp::Action::PassiveGrouper *get_action_group_drag() const;
	bool is_dragging_started_by_keys() const;

public:
	SelectDragHelper(const char *drag_action_name);
	virtual ~SelectDragHelper() {delete action_group_drag;}

	void set_canvas_interface(etl::handle<synfigapp::CanvasInterface> canvas_interface);
	etl::handle<synfigapp::CanvasInterface>& get_canvas_interface();

	bool has_hovered_item() const;
	/// The item whose pointer/mouse is hovering over
	const T& get_hovered_item() const;
	/// Provides a list of selected items
	std::vector<T*> get_selected_items();
	/// Check if an item is selected
	bool is_selected(const T& item) const;
	/// Add the provided item to selection
	void select(const T& item);
	/// Remove the provided item from selection: only if user isn't dragging
	void deselect(const T& item);
	/// The selected item user started to drag
	const T* get_active_item() const;

	State get_state() const;
	/// The point where user clicked
	void get_initial_tracking_point(int &px, int &py) const;
	/// The point where active item was initially placed
	void get_active_item_initial_point(int &px, int &py) const;

	/// Retrieve pressed & held modifier keys (Shift, Control, Alt/Mod1)
	Gdk::ModifierType get_modifiers() const;
	/// Check if a given modifier key (or whole set) m is held (Shift, Control, Alt/Mod1)
	bool has_modifier(Gdk::ModifierType m) const;

	//! @brief Retrieve the position of a given item
	//! Implementation only needed if item dragging is enabled.
	//! @param[out] position the location the provided item
	virtual void get_item_position(const T &item, Gdk::Point &position) = 0;

	//! Retrieve the item placed at a point
	//! @param[out] item the item in the given position
	//! @return true if an item was found, false otherwise
	virtual bool find_item_at_position(int pos_x, int pos_y, T & item) = 0;

	//! @brief Retrieve all items inside a rectangular area.
	//! Implementation only needed if multiple selection is enabled.
	//! @param rect the rectangle area
	//! @param[out] the list of items contained inside rect
	//! @return true if any item was found, false otherwise
	virtual bool find_items_in_rect(Gdk::Rectangle rect, std::vector<T> & list) = 0;

	//! Retrieve all items in collection
	//! @param[out] items the complete item list
	virtual void get_all_items(std::vector<T> & items) = 0;

	void drag_to(int pointer_x, int pointer_y);
	//! @brief Do drag selected items
	//! Implementation only needed if dragging is enabled.
	/*!
	 * @param total_dx total pointer/mouse displacement along x-axis since drag start
	 * @param total_dy total pointer/mouse displacement along y-axis since drag start
	 * @param by_keys if the dragging action is being done by key pressing
	*/
	virtual void delta_drag(int total_dx, int total_dy, bool by_keys) = 0;

	//! Call this method to process mouse/pointer/keyboard events to map them
	//! to select/drag/panning/zoom/scrolling behaviours
	bool process_event(GdkEvent *event);

	void refresh();
	void clear();

	void select_all_items();

	bool get_box_selection_enabled() const;
	void set_box_selection_enabled(bool value);

	bool get_multiple_selection_enabled() const;
	void set_multiple_selection_enabled(bool value);

	bool get_scroll_enabled() const;
	void set_scroll_enabled(bool value);

	bool get_zoom_enabled() const;
	void set_zoom_enabled(bool value);

	bool get_pan_enabled() const;
	void set_pan_enabled(bool value);

	bool get_drag_enabled() const;
	void set_drag_enabled(bool value);

	sigc::signal<void>& signal_selection_changed() { return signal_selection_changed_; }
	sigc::signal<void>& signal_hovered_item_changed() { return signal_hovered_item_changed_; }

	sigc::signal<void>& signal_zoom_in_requested() { return signal_zoom_in_requested_; }
	sigc::signal<void>& signal_zoom_out_requested() { return signal_zoom_out_requested_; }

	sigc::signal<void>& signal_scroll_up_requested() { return signal_scroll_up_requested_; }
	sigc::signal<void>& signal_scroll_down_requested() { return signal_scroll_down_requested_; }

	sigc::signal<void, int, int, int, int>& signal_panning_requested() { return signal_panning_requested_; }

	sigc::signal<void>& signal_redraw_needed() { return signal_redraw_needed_; }

	sigc::signal<void>& signal_focus_requested() { return signal_focus_requested_; }

	sigc::signal<void>& signal_drag_started() { return signal_drag_started_; }
	sigc::signal<void>& signal_drag_canceled() { return signal_drag_canceled_; }
	/// emitted when user finishes drag
	/// \param started_by_key
	sigc::signal<void, bool>& signal_drag_finished() { return signal_drag_finished_; }

	sigc::signal<void, const T&, unsigned int, Gdk::Point>& signal_item_clicked() { return signal_item_clicked_; }
	sigc::signal<void, const T&, unsigned int, Gdk::Point>& signal_item_double_clicked() { return signal_item_double_clicked_; }

	sigc::signal<void>& signal_modifier_keys_changed() { return signal_modifier_keys_changed_; }
};


template <class T>
SelectDragHelper<T>::SelectDragHelper(const char* drag_action_name)
	: drag_action_name(drag_action_name), action_group_drag(nullptr), hovered_item(), is_hovered_item_valid(false), active_item(nullptr), pointer_state(POINTER_NONE),
	  modifiers(Gdk::ModifierType())
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
bool SelectDragHelper<T>::has_hovered_item() const
{
	return is_hovered_item_valid;
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
void SelectDragHelper<T>::select(const T& item)
{
	if (is_selected(item))
		return;
	selected_items.push_back(item);
	signal_selection_changed().emit();
}

template<class T>
void SelectDragHelper<T>::deselect(const T& item)
{
	if (pointer_state == POINTER_DRAGGING)
		return;

	auto iter = std::find(selected_items.begin(), selected_items.end(), item);
	if (iter == selected_items.end())
		return;

	selected_items.erase(iter);
	signal_selection_changed().emit();
}

template<class T>
const T* SelectDragHelper<T>::get_active_item() const {
	return active_item;
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

template<class T>
void SelectDragHelper<T>::get_active_item_initial_point(int& px, int& py) const
{
	px = active_item_start_position.get_x();
	py = active_item_start_position.get_y();
}

template<class T>
Gdk::ModifierType SelectDragHelper<T>::get_modifiers() const
{
	return modifiers;
}

template<class T>
bool SelectDragHelper<T>::has_modifier(Gdk::ModifierType m) const
{
	return modifiers & m;
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
	case GDK_2BUTTON_PRESS:
		return process_button_double_press_event(&event->button);
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
		if (!drag_enabled)
			return false;
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
		if (!drag_enabled)
			return false;
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
	case GDK_KEY_Shift_L:   case GDK_KEY_Shift_R: {
		modifiers |= Gdk::ModifierType::SHIFT_MASK;
		signal_modifier_keys_changed().emit();
		break;
	}
	case GDK_KEY_Control_L:	case GDK_KEY_Control_R: {
		modifiers |= Gdk::ModifierType::CONTROL_MASK;
		signal_modifier_keys_changed().emit();
		break;
	}
	case GDK_KEY_Alt_L:     case GDK_KEY_Alt_R: {
		modifiers |= Gdk::ModifierType::MOD1_MASK;
		signal_modifier_keys_changed().emit();
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
	case GDK_KEY_Shift_L:
	case GDK_KEY_Shift_R: {
		modifiers &= ~Gdk::ModifierType::SHIFT_MASK;
		signal_modifier_keys_changed().emit();
		break;
	}
	case GDK_KEY_Control_L:
	case GDK_KEY_Control_R: {
		modifiers &= ~Gdk::ModifierType::CONTROL_MASK;
		signal_modifier_keys_changed().emit();
		break;
	}
	case GDK_KEY_Alt_L:
	case GDK_KEY_Alt_R: {
		modifiers &= ~Gdk::ModifierType::MOD1_MASK;
		signal_modifier_keys_changed().emit();
		break;
	}
	}
	return false;
}

template<class T>
bool SelectDragHelper<T>::process_button_double_press_event(GdkEventButton* event)
{
	T pointed_item;
	bool found = find_item_at_position(event->x, event->y, pointed_item);
	if (found) {
		int pointer_x = std::trunc(event->x);
		int pointer_y = std::trunc(event->y);
		signal_item_double_clicked().emit(pointed_item, event->button, Gdk::Point(pointer_x, pointer_y));
		return true;
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
			start_tracking_pointer(event->x, event->y);
			T pointed_item;
			bool found = find_item_at_position(pointer_tracking_start_x, pointer_tracking_start_y, pointed_item);
			if (found) {
				auto already_selection_it = std::find(selected_items.begin(), selected_items.end(), pointed_item);
				bool is_already_selected = already_selection_it != selected_items.end();
				bool using_key_modifiers = (event->state & (GDK_CONTROL_MASK|GDK_SHIFT_MASK)) != 0;
				if (using_key_modifiers && box_selection_enabled && multiple_selection_enabled) {
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
					if (drag_enabled) {
						start_dragging(pointed_item_ptr);
						dragging_started_by_key = false;
						pointer_state = POINTER_DRAGGING;
					}
				}
				some_action_done = true;
			} else {
				if (multiple_selection_enabled) {
					if (box_selection_enabled) {
						pointer_state = POINTER_SELECTING;
						some_action_done = true;
					}
				} else {
					selected_items.clear();
					signal_selection_changed().emit();
					some_action_done = true;
				}
			}
		}
	} else if (event->button == 2) {
		if (pointer_state == POINTER_DRAGGING)
			finish_dragging();
		start_tracking_pointer(event->x, event->y);
		pointer_state = POINTER_PANNING;
		some_action_done = true;
	}

	{
		T pointed_item;
		bool found = find_item_at_position(event->x, event->y, pointed_item);
		if (found) {
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
	else if (event->button == 2) {
		if (pointer_state == POINTER_PANNING)
			pointer_state = POINTER_NONE;
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
	{
		// update modifiers when mouse/pointer moves:
		//   useful if widget looses focus due to a shortcut
		//   like Ctrl+Shift+S
		Gdk::ModifierType previous_modifiers = modifiers;
		modifiers = Gdk::ModifierType();
		if (event->state & Gdk::SHIFT_MASK)
			modifiers |= Gdk::SHIFT_MASK;
		if (event->state & Gdk::CONTROL_MASK)
			modifiers |= Gdk::CONTROL_MASK;
		if (event->state & Gdk::MOD1_MASK)
			modifiers |= Gdk::MOD1_MASK;
		if (modifiers != previous_modifiers)
			signal_modifier_keys_changed().emit();
	}

	bool processed = false;
	auto previous_hovered_point = hovered_item;
	bool was_hovered_item_valid = is_hovered_item_valid;
	is_hovered_item_valid = false;

	int pointer_x = std::trunc(event->x);
	int pointer_y = std::trunc(event->y);
	if (pointer_state != POINTER_DRAGGING)
		is_hovered_item_valid = find_item_at_position(pointer_x, pointer_y, hovered_item);

	if (was_hovered_item_valid != is_hovered_item_valid || (is_hovered_item_valid && previous_hovered_point != hovered_item)) {
		signal_hovered_item_changed().emit();
		signal_redraw_needed().emit();
	}

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
		processed = true;
	}
	else if (pointer_state == POINTER_PANNING) {
		const int dx = pointer_x - last_pointer_x;
		const int dy = pointer_y - last_pointer_y;
		const int total_dx = pointer_x - pointer_tracking_start_x;
		const int total_dy = pointer_y - pointer_tracking_start_y;

		signal_panning_requested().emit(dx, dy, total_dx, total_dy);
		processed = true;
	}

	if (pointer_state != POINTER_NONE) {
		signal_redraw_needed().emit();
	}

	last_pointer_x = pointer_x;
	last_pointer_y = pointer_y;
	return processed;
}

template<class T>
bool SelectDragHelper<T>::process_scroll_event(GdkEventScroll* event)
{
	switch(event->direction) {
		case GDK_SCROLL_UP:
		case GDK_SCROLL_RIGHT: {
			if ((event->state & GDK_CONTROL_MASK) && zoom_enabled) {
				// Ctrl+scroll , perform zoom in
				signal_zoom_in_requested().emit();
			} else {
				if (!scroll_enabled)
					return false;
				// Scroll up
				signal_scroll_up_requested().emit();
			}
			return true;
		}
		case GDK_SCROLL_DOWN:
		case GDK_SCROLL_LEFT: {
			if ((event->state & GDK_CONTROL_MASK) && zoom_enabled) {
				// Ctrl+scroll , perform zoom out
				signal_zoom_out_requested().emit();
			} else {
				if (!scroll_enabled)
					return false;
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

template<class T>
synfigapp::Action::PassiveGrouper* SelectDragHelper<T>::get_action_group_drag() const
{
	return action_group_drag;
}

template<class T>
bool SelectDragHelper<T>::is_dragging_started_by_keys() const
{
	return dragging_started_by_key;
}

template <class T>
void SelectDragHelper<T>::refresh() {
	is_hovered_item_valid = false;
}

template <class T>
void SelectDragHelper<T>::clear() {
	if (pointer_state == POINTER_DRAGGING) {
		cancel_dragging();
	}
	is_hovered_item_valid = false;
	if (!selected_items.empty()) {
		selected_items.clear();
		signal_selection_changed().emit();
	}
}

template <class T>
void SelectDragHelper<T>::start_tracking_pointer(double x, double y)
{
	pointer_tracking_start_x = std::trunc(x);
	pointer_tracking_start_y = std::trunc(y);
	last_pointer_x = pointer_tracking_start_x;
	last_pointer_y = pointer_tracking_start_y;
}

template <class T>
void SelectDragHelper<T>::start_dragging(const T* pointed_item)
{
	made_dragging_move = false;
	active_item = pointed_item;
	get_item_position(*active_item, active_item_start_position);

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

	signal_drag_finished().emit(dragging_started_by_key);
}

template <class T>
void SelectDragHelper<T>::cancel_dragging()
{
	if (pointer_state != POINTER_DRAGGING)
		return;

	// Sadly group->cancel() just remove PassiverGroup indicator, not its actions, from stack

	if (action_group_drag) {
		bool has_any_content =  0 < action_group_drag->get_depth();
		delete action_group_drag;
		action_group_drag = nullptr;
		if (has_any_content && canvas_interface) {
			canvas_interface->get_instance()->undo();
			canvas_interface->get_instance()->clear_redo_stack();
		}
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

template <class T>
bool SelectDragHelper<T>::get_box_selection_enabled() const
{
	return box_selection_enabled;
}

template <class T>
void SelectDragHelper<T>::set_box_selection_enabled(bool value)
{
	box_selection_enabled = value;
}

template <class T>
bool SelectDragHelper<T>::get_multiple_selection_enabled() const
{
	return multiple_selection_enabled;
}

template <class T>
void SelectDragHelper<T>::set_multiple_selection_enabled(bool value)
{
	multiple_selection_enabled = value;
	if (!multiple_selection_enabled && selected_items.size() > 1) {
		selected_items.resize(1);
		if (active_item && active_item != &selected_items.front()) {
			cancel_dragging();
		}
	}
}

template <class T>
bool SelectDragHelper<T>::get_scroll_enabled() const
{
	return scroll_enabled;
}

template <class T>
void SelectDragHelper<T>::set_scroll_enabled(bool value)
{
	scroll_enabled = value;
}

template <class T>
bool SelectDragHelper<T>::get_zoom_enabled() const
{
	return zoom_enabled;
}

template <class T>
void SelectDragHelper<T>::set_zoom_enabled(bool value)
{
	zoom_enabled = value;
}

template <class T>
bool SelectDragHelper<T>::get_pan_enabled() const
{
	return pan_enabled;
}

template <class T>
void SelectDragHelper<T>::set_pan_enabled(bool value)
{
	pan_enabled = value;
}

template <class T>
bool SelectDragHelper<T>::get_drag_enabled() const
{
	return drag_enabled;
}

template <class T>
void SelectDragHelper<T>::set_drag_enabled(bool value)
{
	drag_enabled = value;
	if (!drag_enabled) {
		cancel_dragging();
	}
}

};

#endif // SYNFIG_STUDIO_SELECTDRAGHELPER_H
