/* === S Y N F I G ========================================================= */
/*!	\file dialog/vectorizersettings.cpp
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

/* === H E A D E R S ======================================================= */

#include <gui/dialogs/vectorizersettings.h>

#include <glibmm/fileutils.h>
#include <glibmm/markup.h>

#include <gtkmm/alignment.h>

#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/resourcehelper.h>

#include <synfig/debug/log.h>

#include <synfigapp/action.h>
#include <synfigapp/canvasinterface.h>

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

VectorizerSettings::VectorizerSettings(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade) :
	Gtk::Dialog(cobject),
	builder(refGlade)
{
	Gtk::ComboBoxText * comboboxtext_mode;
	refGlade->get_widget("comboboxtext_mode", comboboxtext_mode);
	// comboboxtext_mode.signal_changed().connect(
	// 	sigc::mem_fun(this, &VectorizerSettings::on_comboboxtext_mode_changed));

	Gtk::Grid *mode_grid;
	refGlade->get_widget("mode_grid", mode_grid);

	refGlade->get_widget("Outline_setting_grid", Outline_setting_grid);
	refGlade->get_widget("Centerline_setting_grid", Centerline_setting_grid);

	//---------------------------------Centerline--------------------------------------//
	Gtk::SpinButton * thresholdSpinner;
	refGlade->get_widget("threshold_spinner", thresholdSpinner);
	if (thresholdSpinner){
		adjustment_threshold = thresholdSpinner->get_adjustment();
	}

	Gtk::SpinButton * accuracySpinner;
	refGlade->get_widget("accuracy_spinner", accuracySpinner);
	if (accuracySpinner){
		adjustment_accuracy = accuracySpinner->get_adjustment();
	}

	Gtk::SpinButton * despecklingSpinner;
	refGlade->get_widget("despeckling_spinner", despecklingSpinner);
	if (despecklingSpinner){
		adjustment_despeckling = despecklingSpinner->get_adjustment();
	}

	Gtk::SpinButton * maxthicknessSpinner;
	refGlade->get_widget("maxthickness_spinner", maxthicknessSpinner);
	if (maxthicknessSpinner){
		adjustment_maxthickness = maxthicknessSpinner->get_adjustment();
	}

	//-----------------------------------Outline--------------------------------------//
	///Gtk::Label *lab = manage(new Gtk::Label(_("_Under Development"), true));
	///Outline_setting_grid->attach(*lab, 0, 0, 2, 1);
	// --> The below lines are for Outline params but outline vectorization is not yet implemented

	Gtk::SpinButton * accuracy2Spinner;
	refGlade->get_widget("accuracy2_spinner", accuracy2Spinner);
	if (accuracy2Spinner){
		adjustment_accuracy2 = accuracy2Spinner->get_adjustment();
	}

	Gtk::SpinButton * despeckling2Spinner;
	refGlade->get_widget("despeckling2_spinner", despeckling2Spinner);
	if (despeckling2Spinner){
		adjustment_despeckling2 = despeckling2Spinner->get_adjustment();
	}

	refGlade->get_widget("toggle_pparea2", toggle_pparea2);

	Gtk::SpinButton * adherenceSpinner;
	refGlade->get_widget("adherence_spinner", adherenceSpinner);
	if (adherenceSpinner){
		adjustment_adherence = adherenceSpinner->get_adjustment();
	}

	Gtk::SpinButton * angleSpinner;
	refGlade->get_widget("angle_spinner", angleSpinner);
	if (angleSpinner){
		adjustment_angle = angleSpinner->get_adjustment();
	}

	Gtk::SpinButton * radiusSpinner;
	refGlade->get_widget("radius_spinner", radiusSpinner);
	if (radiusSpinner){
		adjustment_radius = radiusSpinner->get_adjustment();
	}

	//---------------------------------------------------------------------------------//
	Gtk::Button *button = nullptr;

	refGlade->get_widget("cancel_button", button);
	if (button)
		button->signal_clicked().connect(sigc::mem_fun(*this, &VectorizerSettings::on_cancel_pressed));

	refGlade->get_widget("convert_button", button);
	if (button)
		button->signal_clicked().connect(sigc::mem_fun(*this, &VectorizerSettings::on_convert_pressed));

	Outline_setting_grid->hide();
	mode_grid->hide();
	///on_comboboxtext_mode_changed();
}

static Glib::RefPtr<Gtk::Builder> load_interface(const char *filename) {
	auto refBuilder = Gtk::Builder::create();
	try
	{
		refBuilder->add_from_file(ResourceHelper::get_ui_path(filename));
	}
	catch(const Glib::FileError& ex)
	{
		synfig::error("FileError: " + ex.what());
		return Glib::RefPtr<Gtk::Builder>();
	}
	catch(const Glib::MarkupError& ex)
	{
		synfig::error("MarkupError: " + ex.what());
		return Glib::RefPtr<Gtk::Builder>();
	}
	catch(const Gtk::BuilderError& ex)
	{
		synfig::error("BuilderError: " + ex.what());
		return Glib::RefPtr<Gtk::Builder>();
	}
	return refBuilder;
}

void VectorizerSettings::initialize_parameters(etl::handle<synfig::Layer_Bitmap>& my_layer_bitmap,
	etl::handle<studio::Instance>& selected_instance,std::unordered_map <std::string,int>& configmap, etl::handle<synfig::Layer>& reference_layer)
{
	layer_bitmap_ = my_layer_bitmap;
	set_title(_("Vectorizer Settings - ")+ layer_bitmap_->get_description());

	reference_layer_ = reference_layer;
	instance = selected_instance;

	config_map = &configmap;
	adjustment_threshold->set_value(configmap["threshold"]);
	adjustment_accuracy->set_value(configmap["accuracy"]);
	adjustment_despeckling->set_value(configmap["despeckling"]);
	adjustment_maxthickness->set_value(configmap["maxthickness"]);
}

VectorizerSettings * VectorizerSettings::create(Gtk::Window& parent, etl::handle<synfig::Layer_Bitmap> my_layer_bitmap,
	etl::handle<studio::Instance> selected_instance,std::unordered_map <std::string,int>& configmap, etl::handle<synfig::Layer> reference_layer)
{
	auto refBuilder = load_interface("vectorizer_settings.glade");
	if (!refBuilder)
		return nullptr;
	VectorizerSettings * dialog = nullptr;
	refBuilder->get_widget_derived("vectorizer_settings", dialog);
	if (dialog) {
		dialog->initialize_parameters(my_layer_bitmap, selected_instance, configmap, reference_layer);
		dialog->set_transient_for(parent);
	}
	return dialog;
}

VectorizerSettings::~VectorizerSettings()
{
}

// only in use when comboboxtext_mode
void
VectorizerSettings::on_comboboxtext_mode_changed()
{
	isOutline = false;
	//isOutline = comboboxtext_mode.get_active_row_number();
	if(!isOutline)
	{
		//Centerline is active
		Outline_setting_grid->hide();
		Centerline_setting_grid->show_all();
	}	
	else
	{
		//Outline is active
		Centerline_setting_grid->hide();
		Outline_setting_grid->show_all();
	}
}


void
VectorizerSettings::savecurrconfig()
{
	(*config_map)["threshold"] = (int)adjustment_threshold->get_value();
	(*config_map)["accuracy"] = (int)adjustment_accuracy->get_value();
	(*config_map)["despeckling"] = (int)adjustment_despeckling->get_value();
	(*config_map)["maxthickness"] = (int)adjustment_maxthickness->get_value();
		
}

void
VectorizerSettings::on_convert_pressed()
{
	hide();
	synfigapp::Action::Handle action(synfigapp::Action::create("Vectorization"));
	synfig::debug::Log::info("","Action Created ");
	assert(action);
	if(!action)
		return;
	savecurrconfig();
	synfig::debug::Log::info("","Action Asserted ");
	// Add an if else to pass param according to outline /centerline
	action->set_param("image",synfig::Layer::Handle::cast_dynamic(layer_bitmap_));
	action->set_param("mode","centerline");
	action->set_param("threshold",((int)adjustment_threshold->get_value()) * 25);
	action->set_param("penalty",10 - ((int)adjustment_accuracy->get_value()));
	action->set_param("despeckling",((int)adjustment_despeckling->get_value()) * 2);
	action->set_param("maxthickness",((int)adjustment_maxthickness->get_value()) / 2);
	action->set_param("pparea",toggle_pparea.get_state());
	action->set_param("addborder",toggle_add_border.get_state());
	etl::handle<synfig::Canvas> canvas;

	// in case the "convert to vector" was clicked for layer inside a switch
	// and pass canvas accordingly
	
	if(etl::handle<synfig::Layer_PasteCanvas> paste = etl::handle<Layer_PasteCanvas>::cast_dynamic(reference_layer_))
	{
			canvas = layer_bitmap_->get_canvas()->parent();
			action->set_param("reference_layer",reference_layer_);
	}
	else
	{
		canvas = layer_bitmap_->get_canvas();
	}

	etl::handle<synfigapp::CanvasInterface> canvas_interface = instance->find_canvas_interface( canvas->get_non_inline_ancestor() );
	action->set_param("canvas", canvas); 
	action->set_param("canvas_interface", canvas_interface);

	synfig::debug::Log::info("","Action param passed ");
	if(!action->is_ready())
	{
		return;
	}
	synfig::debug::Log::info("","Action is ready ");
	if(!instance->perform_action(action))
	{
		return;
	}
	synfig::debug::Log::info("","Convert Pressed....");
}

void
VectorizerSettings::on_cancel_pressed()
{
	hide();
}
