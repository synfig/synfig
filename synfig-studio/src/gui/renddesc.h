/* === S Y N F I G ========================================================= */
/*!	\file gtkmm/renddesc.h
**	\brief Header File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_RENDDESC_H
#define __SYNFIG_GTKMM_RENDDESC_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/table.h>
#include <gtkmm/frame.h>
#include <synfig/renddesc.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/notebook.h>
#include "widgets/widget_vector.h"
#include "widgets/widget_time.h"
#include "widgets/widget_link.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_RendDesc : public Gtk::Notebook
{
	synfig::RendDesc rend_desc_;
	sigc::signal<void> signal_changed_;

	Glib::RefPtr<Gtk::Adjustment> adjustment_width;
	Glib::RefPtr<Gtk::Adjustment> adjustment_height;
	Glib::RefPtr<Gtk::Adjustment> adjustment_xres;
	Glib::RefPtr<Gtk::Adjustment> adjustment_yres;
	Glib::RefPtr<Gtk::Adjustment> adjustment_phy_width;
	Glib::RefPtr<Gtk::Adjustment> adjustment_phy_height;
	Glib::RefPtr<Gtk::Adjustment> adjustment_fps;
	Glib::RefPtr<Gtk::Adjustment> adjustment_span;

	Gtk::SpinButton *entry_width;
	Gtk::SpinButton *entry_height;
	Gtk::SpinButton *entry_xres;
	Gtk::SpinButton *entry_yres;
	Gtk::SpinButton *entry_phy_width;
	Gtk::SpinButton *entry_phy_height;
	Gtk::SpinButton *entry_fps;
	Gtk::SpinButton *entry_span;

	Widget_Link *toggle_wh_ratio;
	Widget_Link *toggle_res_ratio;

	Gtk::Label *pixel_ratio_label;

	Gtk::CheckButton *toggle_px_aspect;
	Gtk::CheckButton *toggle_px_width;
	Gtk::CheckButton *toggle_px_height;

	Gtk::CheckButton *toggle_im_aspect;
	Gtk::CheckButton *toggle_im_width;
	Gtk::CheckButton *toggle_im_height;
	Gtk::CheckButton *toggle_im_span;

	Gtk::Frame *time_frame;

	Widget_Vector *entry_tl;
	Widget_Vector *entry_br;

	Widget_Vector *entry_focus;

	Widget_Time *entry_start_time;
	Widget_Time *entry_end_time;
	Widget_Time *entry_duration;

	mutable int update_lock;

	struct UpdateLock
	{
		int &locked;
		UpdateLock(int &locked):locked(locked){locked++;}
		~UpdateLock(){locked--;}
	};

public:

	sigc::signal<void> &signal_changed() { return signal_changed_; }

	Widget_RendDesc();
	~Widget_RendDesc();

	//! Sets the RendDesc
	void set_rend_desc(const synfig::RendDesc &rend_desc);

	//! Applies the given RendDesc to the current RendDesc
	void apply_rend_desc(const synfig::RendDesc &rend_desc);

	//! Retrieves the current RendDesc
	const synfig::RendDesc &get_rend_desc();

	void disable_time_section();

	void enable_time_section();

	void refresh();
	
private:

	void on_width_changed();
	void on_height_changed();
	void on_xres_changed();
	void on_yres_changed();
	void on_phy_width_changed();
	void on_phy_height_changed();
	void on_tl_changed();
	void on_br_changed();
	void on_fps_changed();
	void on_start_time_changed();
	void on_end_time_changed();
	void on_duration_changed();
	void on_lock_changed();
	void on_focus_changed();
	void on_span_changed();

	void on_ratio_wh_toggled();
	void on_ratio_res_toggled();

	void create_widgets();
	void connect_signals();
	Gtk::Widget *create_image_tab();
	Gtk::Widget *create_time_tab();
	Gtk::Widget *create_other_tab();
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
