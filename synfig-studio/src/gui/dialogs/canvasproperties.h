/* === S Y N F I G ========================================================= */
/*!	\file dialogs/canvasproperties.h
**	\brief Template Header
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_CANVASPROPERTIES_H
#define __SYNFIG_GTKMM_CANVASPROPERTIES_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/builder.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <gtkmm/spinbutton.h>

#include <ETL/handle>

#include <synfig/renddesc.h>
#include <synfigapp/canvasinterface.h>

#include "widgets/widget_gammapattern.h"
#include "widgets/widget_link.h"
#include "widgets/widget_vector.h"
#include "widgets/widget_time.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class TreeView; };

namespace studio
{

class CanvasProperties  :  public Gtk::Dialog
{
	Glib::RefPtr<Gtk::Builder> canvas_properties;

	etl::handle<synfigapp::CanvasInterface> canvas_interface_;
	synfig::RendDesc rend_desc_;
	sigc::signal<void> signal_changed_;

	Gtk::Box *outer_box;
	Gtk::Box *inner_box;

	// Canvas Info
	Gtk::Entry *entry_id;
	Gtk::Entry *entry_name;
	Gtk::Entry *entry_desc;

	// Image
	Gtk::Label *ratio_label;

	Gtk::SpinButton *entry_width;
	Gtk::SpinButton *entry_height;
	Gtk::SpinButton *entry_xres;
	Gtk::SpinButton *entry_yres;
	Gtk::SpinButton *entry_phy_w;
	Gtk::SpinButton *entry_phy_h;
	Gtk::SpinButton *entry_span;

	studio::Widget_Link *toggle_wh_r;
	studio::Widget_Link *toggle_res_r;

	studio::Widget_Vector *entry_tl;
	studio::Widget_Vector *entry_br;

	// Time
	Gtk::SpinButton *entry_fps;

	studio::Widget_Time *entry_start;
	studio::Widget_Time *entry_end;
	studio::Widget_Time *entry_dur;

	// Gamma Correction
	Gtk::SpinButton *entry_gamma_r;
	Gtk::SpinButton *entry_gamma_g;
	Gtk::SpinButton *entry_gamma_b;

	Gtk::Scale *scale_gamma_r;
	Gtk::Scale *scale_gamma_g;
	Gtk::Scale *scale_gamma_b;

	studio::Widget_GammaPattern *gamma_pattern;

	// Other
	Gtk::CheckButton *toggle_pxl_w;
	Gtk::CheckButton *toggle_pxl_h;
	Gtk::CheckButton *toggle_pxl_a;
	Gtk::CheckButton *toggle_img_w;
	Gtk::CheckButton *toggle_img_h;
	Gtk::CheckButton *toggle_img_a;
	Gtk::CheckButton *toggle_img_s;

	studio::Widget_Vector *entry_focus;

	bool dirty_rend_desc;

	mutable int update_lock;

	struct UpdateLock
	{
		int &locked;
		UpdateLock(int &locked):locked(locked){locked++;}
		~UpdateLock(){locked--;}
	};

	//Gtk::TreeView* meta_data_tree_view;
	//void on_button_meta_data_add();
	//void on_button_meta_data_delete();

public:
	CanvasProperties(Gtk::Window &parent, etl::handle<synfigapp::CanvasInterface> canvas_interface);
	~CanvasProperties();

	void set_rend_desc(const synfig::RendDesc &rend_desc);
	const synfig::RendDesc &get_rend_desc();
	sigc::signal<void> &signal_changed() { return signal_changed_; }

	void refresh();
	void refresh_dialog();

private:
	//Gtk::Widget& create_meta_data_view();

	void on_action_signal_response(int response_id);
	void on_rend_desc_changed();

	void on_width_changed();
	void on_height_changed();
	void on_xres_changed();
	void on_yres_changed();
	void on_phy_w_changed();
	void on_phy_h_changed();
	void on_tl_changed();
	void on_br_changed();
	void on_fps_changed();
	void on_start_time_changed();
	void on_end_time_changed();
	void on_duration_changed();
	void on_lock_changed();
	void on_focus_changed();
	void on_span_changed();
	void on_gamma_changed();
	void on_ratio_wh_toggled();
	void on_ratio_res_toggled();
	
	void set_up_info_widgets();
	void set_up_image_widgets();
	void set_up_time_widgets();
	void set_up_gamma_widgets();
	void set_up_other_widgets();
	void set_up_action_widgets();

	void connect_signals();
}; // END of class CanvasProperties

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
