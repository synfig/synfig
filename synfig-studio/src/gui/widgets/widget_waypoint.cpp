/* === S Y N F I G ========================================================= */
/*!	\file widget_waypoint.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**  Copyright (c) 2008 Paul Wise
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/widgets/widget_waypoint.h>

#include <gtkmm/label.h>
#include <gtkmm/stylecontext.h>

#include <gui/localization.h>
#include <gui/widgets/widget_enum.h>
#include <gui/widgets/widget_time.h>
#include <gui/widgets/widget_value.h>

#endif

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Waypoint::Widget_Waypoint(etl::handle<synfig::Canvas> canvas):
	Gtk::Box(Gtk::ORIENTATION_VERTICAL),
	waypoint(synfig::ValueBase(),0),
	adj_tension(Gtk::Adjustment::create(0.0,-20,20,0.1,1)),
	adj_continuity(Gtk::Adjustment::create(0.0,-20,20,0.1,1)),
	adj_bias(Gtk::Adjustment::create(0.0,-20,20,0.1,1)),
	adj_temporal_tension(Gtk::Adjustment::create(0.0,-20,20,0.1,1))
{
	get_style_context()->add_class("dialog-main-content");
	set_spacing(12);

	value_widget=manage(new Widget_ValueBase());
	value_widget->set_canvas(canvas);
	value_widget->show();

	value_node_label=manage(new Gtk::Label(_("(Non-static value)")));


	time_widget=manage(new Widget_Time());
	time_widget->set_fps(canvas->rend_desc().get_frame_rate());

	before_options=manage(new class Widget_Enum());
	before_options->show();
	before_options->set_param_desc(
		ParamDesc("interpolation")
			.set_hint("enum")
			.add_enum_value(INTERPOLATION_CLAMPED,"clamped",_("Clamped"))
			.add_enum_value(INTERPOLATION_TCB,"auto",_("TCB"))
			.add_enum_value(INTERPOLATION_CONSTANT,"constant",_("Constant"))
			.add_enum_value(INTERPOLATION_HALT,"ease",_("Ease In/Out"))
			.add_enum_value(INTERPOLATION_LINEAR,"linear",_("Linear"))
	);
	before_options->set_icon(0, Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-interpolation_type_clamped"),Gtk::ICON_SIZE_MENU));
	before_options->set_icon(1, Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-interpolation_type_tcb"),Gtk::ICON_SIZE_MENU));
	before_options->set_icon(2, Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-interpolation_type_const"),Gtk::ICON_SIZE_MENU));
	before_options->set_icon(3, Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-interpolation_type_ease"),Gtk::ICON_SIZE_MENU));
	before_options->set_icon(4, Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-interpolation_type_linear"),Gtk::ICON_SIZE_MENU));
	before_options->signal_changed().connect(sigc::mem_fun(*this, &Widget_Waypoint::update_tcb_params_visibility));

	after_options=manage(new class Widget_Enum());
	after_options->show();
	after_options->set_param_desc(
		ParamDesc("interpolation")
			.set_hint("enum")
			.add_enum_value(INTERPOLATION_CLAMPED,"clamped",_("Clamped"))
			.add_enum_value(INTERPOLATION_TCB,"auto",_("TCB"))
			.add_enum_value(INTERPOLATION_CONSTANT,"constant",_("Constant"))
			.add_enum_value(INTERPOLATION_HALT,"ease",_("Ease In/Out"))
			.add_enum_value(INTERPOLATION_LINEAR,"linear",_("Linear"))
	);
	after_options->set_icon(0, Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-interpolation_type_clamped"),Gtk::ICON_SIZE_MENU));
	after_options->set_icon(1, Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-interpolation_type_tcb"),Gtk::ICON_SIZE_MENU));
	after_options->set_icon(2, Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-interpolation_type_const"),Gtk::ICON_SIZE_MENU));
	after_options->set_icon(3, Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-interpolation_type_ease"),Gtk::ICON_SIZE_MENU));
	after_options->set_icon(4, Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-interpolation_type_linear"),Gtk::ICON_SIZE_MENU));
	after_options->signal_changed().connect(sigc::mem_fun(*this, &Widget_Waypoint::update_tcb_params_visibility));

	spin_tension=manage(new class Gtk::SpinButton(adj_tension,0.1,3));
	spin_continuity=manage(new class Gtk::SpinButton(adj_continuity,0.1,3));
	spin_bias=manage(new class Gtk::SpinButton(adj_bias,0.1,3));
	spin_temporal_tension=manage(new class Gtk::SpinButton(adj_temporal_tension,0.1,3));
	spin_tension         ->set_hexpand();
	spin_continuity      ->set_hexpand();
	spin_bias            ->set_hexpand();
	spin_temporal_tension->set_hexpand();

	Gtk::Frame *waypointFrame = manage(new Gtk::Frame(_("Waypoint")));
	waypointFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) waypointFrame->get_label_widget())->set_markup(_("<b>Waypoint</b>"));

	auto waypointGrid = manage(new Gtk::Grid());
	waypointGrid->get_style_context()->add_class("dialog-secondary-content");
	waypointGrid->set_row_spacing(6);
	waypointGrid->set_column_spacing(12);

	Gtk::Label *waypointValueLabel = manage(new Gtk::Label(_("_Value"), true));
	waypointValueLabel->set_mnemonic_widget(*value_widget);
	waypointGrid->attach(*waypointValueLabel, 0, 0, 1, 1);
	waypointGrid->attach(*value_widget      , 1, 0, 1, 1);
	waypointGrid->attach(*value_node_label  , 2, 0, 1, 1);

	Gtk::Label *waypointTimeLabel = manage(new Gtk::Label(_("_Time"), true));
	waypointTimeLabel->set_mnemonic_widget(*time_widget);
	waypointGrid->attach(*waypointTimeLabel, 0, 1, 1, 1);
	waypointGrid->attach(*time_widget,       1, 1, 1, 1);

	Gtk::Frame *interpolationFrame = manage(new Gtk::Frame(_("Interpolation")));
	interpolationFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) interpolationFrame->get_label_widget())->set_markup(_("<b>Interpolation</b>"));

	auto interpolationGrid = manage(new Gtk::Grid());
	interpolationGrid->get_style_context()->add_class("dialog-secondary-content");
	interpolationGrid->set_row_spacing(6);
	interpolationGrid->set_column_spacing(12);

	Gtk::Label *interpolationInLabel = manage(new Gtk::Label(_("_In Interpolation"), true));
	interpolationInLabel->set_halign(Gtk::ALIGN_START);
	interpolationInLabel->set_mnemonic_widget(*before_options);
	interpolationGrid->attach(*interpolationInLabel, 0, 0, 1, 1);
	interpolationGrid->attach(*before_options,       1, 0, 1, 1);

	Gtk::Label *interpolationOutLabel = manage(new Gtk::Label(_("O_ut Interpolation"), true));
	interpolationOutLabel->set_halign(Gtk::ALIGN_START);
	interpolationOutLabel->set_mnemonic_widget(*after_options);
	interpolationGrid->attach(*interpolationOutLabel, 0, 1, 1, 1);
	interpolationGrid->attach(*after_options,         1, 1, 1, 1);

	tcbFrame = manage(new Gtk::Frame(_("TCB Parameters")));
	tcbFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) tcbFrame->get_label_widget())->set_markup(_("<b>TCB Parameter</b>"));

	tcbGrid = manage(new Gtk::Grid());
	tcbGrid->get_style_context()->add_class("dialog-secondary-content");
	tcbGrid->set_row_spacing(6);
	tcbGrid->set_column_spacing(12);

	Gtk::Label *tensionLabel = manage(new Gtk::Label(_("T_ension"), true));
	tensionLabel->set_halign(Gtk::ALIGN_START);
	tensionLabel->set_mnemonic_widget(*spin_tension);
	tcbGrid->attach(*tensionLabel, 0, 1, 1, 1);
	tcbGrid->attach(*spin_tension, 1, 1, 1, 1);

	Gtk::Label *continuityLabel = manage(new Gtk::Label(_("Continuit_y"), true));
	continuityLabel->set_halign(Gtk::ALIGN_START);
	continuityLabel->set_mnemonic_widget(*spin_continuity);
	tcbGrid->attach(*continuityLabel, 0, 2, 1, 1);
	tcbGrid->attach(*spin_continuity, 1, 2, 1, 1);

	Gtk::Label *biasLabel = manage(new Gtk::Label(_("_Bias"), true));
	biasLabel->set_halign(Gtk::ALIGN_START);
	biasLabel->set_mnemonic_widget(*spin_bias);
	tcbGrid->attach(*biasLabel, 0, 3, 1, 1);
	tcbGrid->attach(*spin_bias, 1, 3, 1, 1);

	Gtk::Label *temporalTensionLabel = manage(new Gtk::Label(_("Te_mporal Tension"), true));
	temporalTensionLabel->set_halign(Gtk::ALIGN_START);
	temporalTensionLabel->set_mnemonic_widget(*spin_temporal_tension);
	tcbGrid->attach(*temporalTensionLabel,  0, 4, 1, 1);
	tcbGrid->attach(*spin_temporal_tension, 1, 4, 1, 1);

	tcbFrame->add(*tcbGrid);

	set_canvas(canvas);

	add(*waypointFrame);
	add(*waypointGrid);
	add(*interpolationFrame);
	add(*interpolationGrid);
	add(*tcbFrame);
}

void
Widget_Waypoint::set_canvas(synfig::Canvas::Handle x)
{
	canvas=x;
	assert(canvas);

	time_widget->set_fps(canvas->rend_desc().get_frame_rate());
	value_widget->set_canvas(canvas);
}

void
Widget_Waypoint::set_waypoint(synfig::Waypoint &x)
{
	time_widget->set_fps(canvas->rend_desc().get_frame_rate());

	waypoint=x;

	//! \todo This really needs to be fixed to support value node waypoints!
	if(waypoint.is_static())
	{
		value_widget->set_value_desc(
			synfigapp::ValueDesc(
				etl::handle<ValueNode_Const>::cast_dynamic(
					waypoint.get_parent_value_node() )));
		value_widget->set_value(waypoint.get_value());
		value_widget->show();
		value_node_label->hide();
	}
	else
	{
		value_widget->hide();
		value_node_label->show();
	}

	time_widget->set_value(waypoint.get_time());

	before_options->set_value((Waypoint::Interpolation)waypoint.get_before());
	after_options->set_value((Waypoint::Interpolation)waypoint.get_after());
	
	update_tcb_params_visibility();
}

void Widget_Waypoint::set_valuedesc(synfigapp::ValueDesc& value_desc)
{
	if(value_desc.get_value_node() && value_desc.get_value_node()->get_parent_canvas())
		set_canvas(value_desc.get_value_node()->get_parent_canvas());

	value_widget->set_value_desc(value_desc);
	synfig::ParamDesc param_desc;
	if (value_desc.parent_is_layer())
		value_desc.find_param_desc(param_desc);
	value_widget->set_param_desc(param_desc);
}

const synfig::Waypoint &
Widget_Waypoint::get_waypoint()const
{
	//! \todo This too!
	waypoint.set_time(time_widget->get_value());
	if(waypoint.is_static())
		waypoint.set_value(value_widget->get_value());

	waypoint.set_before((Waypoint::Interpolation)before_options->get_value());
	waypoint.set_after((Waypoint::Interpolation)after_options->get_value());

	waypoint.set_tension(adj_tension->get_value());
	waypoint.set_continuity(adj_continuity->get_value());
	waypoint.set_bias(adj_bias->get_value());
	waypoint.set_temporal_tension(adj_temporal_tension->get_value());
	return waypoint;
}

void
Widget_Waypoint::config_tcb_params(bool show_params)
{
	if (show_params) {
		// set the adjustment value
		adj_tension->set_value(waypoint.get_tension());
		adj_continuity->set_value(waypoint.get_continuity());
		adj_bias->set_value(waypoint.get_bias());
		adj_temporal_tension->set_value(waypoint.get_temporal_tension());
	}

	tcbFrame->set_visible(show_params);
}
void
Widget_Waypoint::update_tcb_params_visibility()
{
	if (
		(Waypoint::Interpolation)before_options->get_value() == Interpolation::INTERPOLATION_TCB ||
		(Waypoint::Interpolation)after_options->get_value() == Interpolation::INTERPOLATION_TCB)
		config_tcb_params(true);
	else
		config_tcb_params(false);
}

