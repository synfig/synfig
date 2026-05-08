/* === S Y N F I G ========================================================= */
/*!	\file dialogs/vectorizersettings.h
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

/* === S T A R T =========================================================== */
#ifndef __SYNFIG_GTKMM_DIALOG_VECTORIZERSETTINGS_H
#define __SYNFIG_GTKMM_DIALOG_VECTORIZERSETTINGS_H

/* === H E A D E R S ======================================================= */

#include <unordered_map>

#include <gtkmm/adjustment.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/dialog.h>
#include <gtkmm/grid.h>
#include <gtkmm/separator.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/switch.h>
#include <gtkmm/builder.h>

#include <synfig/handle.h>
#include <synfig/layers/layer_bitmap.h>

#include <gui/instance.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{
class VectorizerSettings : public Gtk::Dialog
{
	Glib::RefPtr<Gtk::Adjustment> adjustment_accuracy;
	Glib::RefPtr<Gtk::Adjustment> adjustment_accuracy2;
	Glib::RefPtr<Gtk::Adjustment> adjustment_threshold;
	Glib::RefPtr<Gtk::Adjustment> adjustment_despeckling;
	Glib::RefPtr<Gtk::Adjustment> adjustment_despeckling2;
	Glib::RefPtr<Gtk::Adjustment> adjustment_maxthickness;
	
	Glib::RefPtr<Gtk::Adjustment> adjustment_radius;
	Glib::RefPtr<Gtk::Adjustment> adjustment_adherence;
	Glib::RefPtr<Gtk::Adjustment> adjustment_angle;

	Gtk::Switch toggle_pparea;
	Gtk::Switch *toggle_pparea2;
	Gtk::Switch toggle_add_border;

	Gtk::Grid *Outline_setting_grid;
	Gtk::Grid *Centerline_setting_grid;

  	Gtk::Separator Separator;

	const Glib::RefPtr<Gtk::Builder>& builder;

	Gtk::ComboBoxText comboboxtext_mode;
	synfig::Layer_Bitmap::Handle layer_bitmap_;
	synfig::Layer::Handle reference_layer_;
	etl::handle<Instance> instance;
	std::unordered_map <std::string,int>* config_map;

public:

	bool isOutline;
	VectorizerSettings(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	static VectorizerSettings * create(Gtk::Window& parent,synfig::Layer_Bitmap::Handle my_layer_bitmap,
			etl::handle<studio::Instance> selected_instance,std::unordered_map <std::string,int>& configmap, synfig::Layer::Handle reference_layer);
	~VectorizerSettings();

	// CenterlineConfiguration getCenterlineConfiguration() const;
  	// NewOutlineConfiguration getOutlineConfiguration(double weight) const;

	// void doVectorize(const VectorizerConfiguration &conf); 

  	// VectorizerConfiguration *getCurrentConfiguration(double weight) const {
    // 	return isOutline ? (VectorizerConfiguration *)new NewOutlineConfiguration(getOutlineConfiguration(weight))
    //                    : (VectorizerConfiguration *)new CenterlineConfiguration(getCenterlineConfiguration());
  	// }
	
private:	
	void initialize_parameters(synfig::Layer_Bitmap::Handle& my_layer_bitmap,
		etl::handle<studio::Instance>& selected_instance,std::unordered_map <std::string,int>& configmap, etl::handle<synfig::Layer>& reference_layer);

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
}; // END of class VectorizerSettings

}; // END of namespace studio


/* === E N D =============================================================== */

#endif

