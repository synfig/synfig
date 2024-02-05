/* === S Y N F I G ========================================================= */
/*!	\file dialogs/canvasresize.h
**	\brief Template Header
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

#ifndef __SYNFIG_GTKMM_CANVASRESIZE_H
#define __SYNFIG_GTKMM_CANVASRESIZE_H

#include <array>

#include <ETL/handle>
#include <synfig/renddesc.h>
#include <synfigapp/canvasinterface.h>

#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/dialog.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/comboboxtext.h>

namespace studio
{

class CanvasProperties;
class Widget_Link;

class CanvasResize : public Gtk::Dialog
{
	etl::handle<synfigapp::CanvasInterface> canvas_interface;
	synfig::RendDesc rend_desc;
	CanvasProperties &canvas_properties;

	Glib::RefPtr<Gtk::Builder> builder;

	Gtk::Grid        *grid_content;
	Gtk::Grid        *grid_canvas;
	Gtk::SpinButton  *width;
	Gtk::SpinButton  *height;
	Gtk::EventBox    *rsz_im_label;
	Gtk::CheckButton *rsz_im_chbox;

	Widget_Link *toggle_ratio_wh;
	Gtk::ComboBoxText *combo_box;

	//    0  1  2
	//    3  4  5    3x3 grid button indexes
	//    6  7  8
	std::array<Gtk::Button*, 9> canvas_buttons;
	Gtk::Button *canvas_center;

	bool was_image_checked;
	bool is_image_checked;

	int old_width;
	int old_height;
	int new_width;
	int new_height;

	// Dialog response ids
	enum Response {
		ADVANCED,
		CANCEL = Gtk::RESPONSE_CANCEL,
		CLOSE  = Gtk::RESPONSE_DELETE_EVENT,
		OKAY   = Gtk::RESPONSE_OK
	};

	// Update width & height widgets only once
	int update_lock;
	struct UpdateLock
	{
		int &locked;
		UpdateLock(int &locked) : locked(locked) { locked++; }
		~UpdateLock() { locked--; }
	};

	// Initial set ups to keep our constructor body compact & readable
	void set_up_builder_widgets();
	void set_up_toggle_tooltips();
	void set_up_action_widgets();
	void set_up_signals();

	// Our dialog signal slots
	void on_dialog_shown();
	void on_rend_desc_changed();
	void on_action_signal_response(int response_id);
	void on_width_changed();
	void on_height_changed();
	void on_size_template_changed();
	void on_canvas_button_clicked(Gtk::Button *button);

	bool on_image_label_enter_event(GdkEventCrossing *event);
	bool on_image_label_leave_event(GdkEventCrossing *event);
	bool on_image_label_button_event(GdkEventButton *event);
	void on_resize_image_checked();
	void on_wh_ratio_toggled();
	void on_spinbutton_updated(Gtk::SpinButton *widget);

	// Dialog update helper functions
	// The first one is also used as a signal slot for synfigapp::CanvasInterface
	void refresh_title();
	void refresh_content();
	void refresh_wh_toggle_widgets();
	void refresh_old_new_wh_values();
	void reset_canvas_buttons();
	void set_canvas_center_point();
	void set_canvas_disabled_button_image();
	void set_canvas_button_images();
	void set_button_image_top_left();
	void set_button_image_top();
	void set_button_image_top_right();
	void set_button_image_left();
	void set_button_image_right();
	void set_button_image_bottom_left();
	void set_button_image_bottom();
	void set_button_image_bottom_right();
	void set_default_button_images();
	void flip_canvas_button_images();
	void flip_button_image_top_left();
	void flip_button_image_top();
	void flip_button_image_top_right();
	void flip_button_image_left();
	void flip_button_image_right();
	void flip_button_image_bottom_left();
	void flip_button_image_bottom();
	void flip_button_image_bottom_right();
	void flip_default_button_images();
	void set_button_image(Gtk::Image *image, const std::string icon_file);
	void set_image_flags(bool is_wh_linked);
	void set_canvas_flags(bool is_wh_linked);

public:
	CanvasResize(Gtk::Window &parent, etl::handle<synfigapp::CanvasInterface> &ci, CanvasProperties &cp);
}; // End of class CanvasResize

}; // End of namespace studio

#endif
