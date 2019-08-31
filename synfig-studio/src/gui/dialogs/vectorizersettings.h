/* === S Y N F I G ========================================================= */
/*!	\file dialog/vectorizersettings.h
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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
#ifndef __SYNFIG_GTKMM_DIALOG_VECTORIZERSETTINGS_H
#define __SYNFIG_GTKMM_DIALOG_VECTORIZERSETTINGS_H

/* === H E A D E R S ======================================================= */
#include <ETL/handle>
#include <vector>
#include <unordered_map> 
#include <gtkmm.h>
// #include <gtkmm/grid.h>
// #include <gtkmm/dialog.h>
// #include <gtkmm/switch.h>
// #include <gtkmm/tooltip.h>
// #include <gtkmm/table.h>
// #include <gtkmm/entry.h>
// #include <gtkmm/adjustment.h>
// #include <gtkmm/spinbutton.h>
// #include <gtkmm/checkbutton.h>
// #include <gtkmm/comboboxtext.h>
#include <synfig/string.h>
#include <synfigapp/canvasinterface.h>
#include <synfig/layers/layer_bitmap.h>
#include <synfig/layers/layer_pastecanvas.h>
#include "instance.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{
class VectorizerSettings : public Gtk::Dialog
{
	Glib::RefPtr<Gtk::Adjustment> adjustment_accuracy;
	Gtk::SpinButton entry_accuracy;
	Glib::RefPtr<Gtk::Adjustment> adjustment_accuracy2;
	Gtk::SpinButton entry_accuracy2;
	Glib::RefPtr<Gtk::Adjustment> adjustment_threshold;
	Gtk::SpinButton entry_threshold;
	Glib::RefPtr<Gtk::Adjustment> adjustment_despeckling;
	Gtk::SpinButton entry_despeckling;
	Glib::RefPtr<Gtk::Adjustment> adjustment_despeckling2;
	Gtk::SpinButton entry_despeckling2;
	Glib::RefPtr<Gtk::Adjustment> adjustment_maxthickness;
	Gtk::SpinButton entry_maxthickness;
	
	Glib::RefPtr<Gtk::Adjustment> adjustment_radius;
	Gtk::SpinButton entry_radius;
	Glib::RefPtr<Gtk::Adjustment> adjustment_adherence;
	Gtk::SpinButton entry_adherence;
	Glib::RefPtr<Gtk::Adjustment> adjustment_angle;
	Gtk::SpinButton entry_angle;

	Gtk::Switch toggle_pparea;
	Gtk::Switch toggle_pparea2;
	Gtk::Switch toggle_add_border;

	Gtk::Grid *Outline_setting_grid = manage(new Gtk::Grid());
	Gtk::Grid *Centerline_setting_grid = manage(new Gtk::Grid());

	Gtk::ProgressBar ProgressBar;
  	Gtk::Separator Separator;

	Gtk::ComboBoxText comboboxtext_mode;
	const etl::handle<synfig::Layer_Bitmap> layer_bitmap_;
	etl::handle<synfig::Layer> reference_layer_;
	const etl::handle<Instance> instance;
	std::unordered_map <std::string,int>* config_map;

public:

	bool isOutline;
	VectorizerSettings(Gtk::Window& parent, etl::handle<synfig::Layer_Bitmap> my_layer_bitmap,
			etl::handle<Instance> selected_instance,std::unordered_map <std::string,int>& configmap,etl::handle<synfig::Layer> reference_layer);
	~VectorizerSettings();
	void set_progress(float value);

	// CenterlineConfiguration getCenterlineConfiguration() const;
  	// NewOutlineConfiguration getOutlineConfiguration(double weight) const;

	// void doVectorize(const VectorizerConfiguration &conf); 

  	// VectorizerConfiguration *getCurrentConfiguration(double weight) const {
    // 	return isOutline ? (VectorizerConfiguration *)new NewOutlineConfiguration(getOutlineConfiguration(weight))
    //                    : (VectorizerConfiguration *)new CenterlineConfiguration(getCenterlineConfiguration());
  	// }
	
private:
	void on_comboboxtext_mode_changed();

	void on_threshold_changed();
   	void on_accuracy_changed();
	void on_despeckling_changed();
	void on_maxthickness_changed();
   	void on_tcalibration_start_changed();
   	void on_tcalibration_end_changed();

    void on_pparea_toggle();
	void on_add_border_toggle();
	void on_convert_pressed();
	void on_cancel_pressed();
	void savecurrconfig();
	void on_finished();
}; // END of class VectorizerSettings

}; // END of namespace studio


/* === E N D =============================================================== */

#endif

