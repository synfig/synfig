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

#include <iostream>

#include <map>
#include <glibmm.h>
#include <gtkmm/grid.h>
#include <gtkmm/frame.h>
#include <gtkmm/alignment.h>
#include <math.h>
#include <ETL/stringf>
#include "vectorizersettings.h"
#include <synfig/rendering/software/surfacesw.h>
#include <gui/localization.h>
#include <synfigapp/action_param.h>
#include "onemoment.h"


/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

VectorizerSettings::VectorizerSettings(Gtk::Window& parent,etl::handle<synfig::Layer_Bitmap> my_layer_bitmap,
 etl::handle<studio::Instance> selected_instance,std::unordered_map <std::string,int>& configmap, etl::handle<synfig::Layer> reference_layer):
	Gtk::Dialog(_("Convert-to-Vector Settings"),parent),
	layer_bitmap_(my_layer_bitmap),
	reference_layer_(reference_layer),
	instance(selected_instance),
	adjustment_threshold(Gtk::Adjustment::create(configmap["threshold"],1,10)),
	entry_threshold(adjustment_threshold,1,0),
	adjustment_accuracy(Gtk::Adjustment::create(configmap["accuracy"],1,10)),
	entry_accuracy(adjustment_accuracy,1,0),
	adjustment_despeckling(Gtk::Adjustment::create(configmap["despeckling"],0,500)),
	entry_despeckling(adjustment_despeckling,1,0),
	adjustment_accuracy2(Gtk::Adjustment::create(5,1,10)),
	entry_accuracy2(adjustment_accuracy2,1,0),
	adjustment_despeckling2(Gtk::Adjustment::create(3,0,500)),
	entry_despeckling2(adjustment_despeckling2,1,0),
	adjustment_maxthickness(Gtk::Adjustment::create(configmap["maxthickness"],0,500)),
	entry_maxthickness(adjustment_maxthickness,1,0),
	adjustment_radius(Gtk::Adjustment::create(100,1,100)),
	entry_radius(adjustment_radius,1,0),
	adjustment_adherence(Gtk::Adjustment::create(100,1,100)),
	entry_adherence(adjustment_adherence,1,0),
	adjustment_angle(Gtk::Adjustment::create(100,1,100)),
	entry_angle(adjustment_angle,1,0)
{
	//Centerline and Outline option in the comboboxtext
	// comboboxtext_mode.append(_("Centerline"));
	// comboboxtext_mode.append("Outline");
	// //set Centerline Method active by default
	// comboboxtext_mode.set_active(0);
	// comboboxtext_mode.signal_changed().connect(
	// 	sigc::mem_fun(this, &VectorizerSettings::on_comboboxtext_mode_changed));
	config_map = &configmap;
	Gtk::Alignment *dialogPadding = manage(new Gtk::Alignment(1, 1, 1, 1));
	get_vbox()->pack_start(*dialogPadding, false, false, 0);
	Gtk::VBox *dialogBox = manage(new Gtk::VBox(false, 12));
	dialogPadding->add(*dialogBox);

	// Gtk::Frame *target_frame=manage(new Gtk::Frame());
	// target_frame->set_shadow_type(Gtk::SHADOW_NONE);
	// dialogBox->pack_start(*target_frame);
	// Gtk::Grid *mode_grid = manage(new Gtk::Grid());
	// Gtk::Label *mode_label = manage(new Gtk::Label(_("_Mode"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	// mode_label->set_mnemonic_widget(comboboxtext_mode);
	// mode_label->set_margin_right(10);
	// mode_grid->attach(*mode_label, 0, 0, 1, 1);
	// mode_grid->attach(comboboxtext_mode, 1, 0, 1, 1);
	// mode_grid->set_column_homogeneous(true);
	// target_frame->add(*mode_grid);


	Gtk::Box *settings_box=manage(new Gtk::Box());
	dialogBox->pack_start(*settings_box);

	//-----------------------------------Centerline--------------------------------------//
	Gtk::Label *threshold_label = manage(new Gtk::Label(_("_Threshold"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	threshold_label->set_mnemonic_widget(entry_threshold);
	threshold_label->set_margin_right(10);
	threshold_label->set_tooltip_text("sets value of the darkest pixels to be taken into account to detect lines to be converted to vector strokes");

	Centerline_setting_grid->attach(*threshold_label, 0, 0, 1, 1);
	Centerline_setting_grid->attach(entry_threshold, 1, 0, 1, 1);
	
	Gtk::Label *accuracy_label = manage(new Gtk::Label(_("_Accuracy"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	accuracy_label->set_mnemonic_widget(entry_accuracy);
	accuracy_label->set_margin_right(10);
	accuracy_label->set_tooltip_text("sets how much the vector stroke will follow the shape of the original drawing lines");

	Centerline_setting_grid->attach(*accuracy_label, 0, 1, 1, 1);
	Centerline_setting_grid->attach(entry_accuracy, 1, 1, 1, 1);

	Gtk::Label *despeckling_label = manage(new Gtk::Label(_("_Despeckling"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	despeckling_label->set_mnemonic_widget(entry_despeckling);
	despeckling_label->set_margin_right(10);
	despeckling_label->set_tooltip_text("sets value to ignore small areas generated by the image noise");

	Centerline_setting_grid->attach(*despeckling_label, 0, 2, 1, 1);
	Centerline_setting_grid->attach(entry_despeckling, 1, 2, 1, 1);

	Gtk::Label *thickness_label = manage(new Gtk::Label(_("_Max Thickness"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	thickness_label->set_mnemonic_widget(entry_maxthickness);
	thickness_label->set_margin_right(10);
	thickness_label->set_tooltip_text("sets the maximum vector stroke thickness");

	Centerline_setting_grid->attach(*thickness_label, 0, 3, 1, 1);
	Centerline_setting_grid->attach(entry_maxthickness, 1, 3, 1, 1);

	// Gtk::Label *ppa_label = manage(new Gtk::Label(_("_Preserve Painted Areas(Not yet working)"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	// ppa_label->set_mnemonic_widget(toggle_pparea);
	// ppa_label->set_margin_right(10);

	// toggle_pparea.set_halign(Gtk::ALIGN_START);
	// Centerline_setting_grid->attach(*ppa_label, 0, 6, 1, 1);
	// Centerline_setting_grid->attach(toggle_pparea, 1, 6, 1, 1);
	
	// Gtk::Label *add_border_label = manage(new Gtk::Label(_("_Add Border(Not yet working)"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	// add_border_label->set_mnemonic_widget(toggle_add_border);
	// add_border_label->set_margin_right(10);

	// toggle_add_border.set_halign(Gtk::ALIGN_START);
	// Centerline_setting_grid->attach(*add_border_label, 0, 7, 1, 1);
	// Centerline_setting_grid->attach(toggle_add_border, 1, 7, 1, 1);
	
	Centerline_setting_grid->set_column_homogeneous(true);	
	Centerline_setting_grid->set_row_homogeneous(true);

	Centerline_setting_grid->set_hexpand(true);
	settings_box->add(*Centerline_setting_grid);


	//-----------------------------------Outline--------------------------------------//
	Gtk::Label *lab = manage(new Gtk::Label(_("_Under Development"), true));
	Outline_setting_grid->attach(*lab, 0, 0, 2, 1);
	// --> The below lines are for Outline params but outline vectorization is not yet implemented
	// Gtk::Label *accuracy_label2 = manage(new Gtk::Label(_("_Accuracy"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	// accuracy_label2->set_mnemonic_widget(entry_accuracy2);
	// accuracy_label2->set_margin_right(10);
	// Outline_setting_grid->attach(*accuracy_label2, 0, 0, 1, 1);
	// Outline_setting_grid->attach(entry_accuracy2, 1, 0, 1, 1);

	// Gtk::Label *despeckling_label2 = manage(new Gtk::Label(_("_Despeckling"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	// despeckling_label2->set_mnemonic_widget(entry_despeckling2);
	// despeckling_label2->set_margin_right(10);
	// Outline_setting_grid->attach(*despeckling_label2, 0, 1, 1, 1);
	// Outline_setting_grid->attach(entry_despeckling2, 1, 1, 1, 1);

	// Gtk::Label *ppa_label2 = manage(new Gtk::Label(_("_Preserve Painted Areas"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	// ppa_label2->set_mnemonic_widget(toggle_pparea2);
	// ppa_label2->set_margin_right(10);
	// toggle_pparea2.set_halign(Gtk::ALIGN_START);
	// Outline_setting_grid->attach(*ppa_label2, 0, 2, 1, 1);
	// Outline_setting_grid->attach(toggle_pparea2, 1, 2, 1, 1);
	
	// Gtk::Label *adherence_label = manage(new Gtk::Label(_("_Corners Adherenece"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	// adherence_label->set_mnemonic_widget(entry_adherence);
	// adherence_label->set_margin_right(10);
	// Outline_setting_grid->attach(*adherence_label, 0, 3, 1, 1);
	// Outline_setting_grid->attach(entry_adherence, 1, 3, 1, 1);

	// Gtk::Label *angle_label = manage(new Gtk::Label(_("_Corners Angle"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	// angle_label->set_mnemonic_widget(entry_angle);
	// angle_label->set_margin_right(10);
	// Outline_setting_grid->attach(*angle_label, 0, 4, 1, 1);
	// Outline_setting_grid->attach(entry_angle, 1, 4, 1, 1);

	// Gtk::Label *radius_label = manage(new Gtk::Label(_("_Corners Curve Radius"), Gtk::ALIGN_END,Gtk::ALIGN_FILL, true));
	// radius_label->set_mnemonic_widget(entry_radius);
	// radius_label->set_margin_right(10);
	// Outline_setting_grid->attach(*radius_label, 0, 5, 1, 1);
	// Outline_setting_grid->attach(entry_radius, 1, 5, 1, 1);

	Outline_setting_grid->set_column_homogeneous(true);
	Outline_setting_grid->set_hexpand(true);

	settings_box->add(*Outline_setting_grid);
	//---------------------------------------------------------------------------------//
	get_vbox()->pack_start(Separator, Gtk::PACK_SHRINK);
	get_vbox()->pack_start(ProgressBar, Gtk::PACK_SHRINK, 5);
	ProgressBar.set_margin_right(10);
	ProgressBar.set_margin_left(10);
	ProgressBar.set_text("Not started");
	ProgressBar.set_show_text(true);

	Gtk::Button *convert_button(manage(new class Gtk::Button("_Convert",true)));
	convert_button->set_tooltip_text("Perform vectorization");
	convert_button->show();
	add_action_widget(*convert_button,1);
	convert_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::VectorizerSettings::on_convert_pressed));

	Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-cancel"))));
	cancel_button->set_tooltip_text("Close the dialog");
	cancel_button->show();
	add_action_widget(*cancel_button,0);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::VectorizerSettings::on_cancel_pressed));


	set_title(_("Vectorizer Settings - ")+ layer_bitmap_->get_description());

	get_vbox()->show_all();
	Outline_setting_grid->hide();
	on_comboboxtext_mode_changed();

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
VectorizerSettings::on_finished()
{
}

void
VectorizerSettings::set_progress(float value)
{
	float r = value/100.0;
	ProgressBar.set_text( strprintf( "%.1f%%", value ));
	ProgressBar.set_fraction(r);
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
	OneMoment one_moment;
	one_moment.show();
	synfigapp::Action::Handle action(synfigapp::Action::create("Vectorization"));
	std::cout<<"Action Created \n";
	assert(action);
	if(!action)
		return;
	savecurrconfig();
	std::cout<<"Action Asserted \n";
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
			std::cout<<"image inside group layer clicked\n";
			canvas = layer_bitmap_->get_canvas()->parent();
			action->set_param("reference_layer",reference_layer_);
	}
	else
	{
		std::cout<<"image switch layer clicked\n";
		canvas = layer_bitmap_->get_canvas();
	}

	etl::handle<synfigapp::CanvasInterface> canvas_interface = instance->find_canvas_interface( canvas->get_non_inline_ancestor() );
	action->set_param("canvas", canvas); 
	action->set_param("canvas_interface", canvas_interface);

	std::cout<<"Action param passed \n";
	if(!action->is_ready())
	{
		return;
	}
	std::cout<<"Action is ready \n";
	if(!instance->perform_action(action))
	{
		return;
	}
	std::cout<<"Convert Pressed....";
	one_moment.hide();
}

void
VectorizerSettings::on_cancel_pressed()
{
	hide();
}
