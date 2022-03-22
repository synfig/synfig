/* === S Y N F I G ========================================================= */
/*!	\file state_text.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
**  Copyright (c) 2010 Carlos López
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "state_text.h"
#include "widgets/widget_fontfamily.h"

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/duckmatic.h>
#include <gui/event_mouse.h>
#include <gui/localization.h>
#include <gui/states/state_normal.h>
#include <gui/widgets/widget_enum.h>
#include <gui/workarea.h>

#include <synfig/general.h>

#include <synfigapp/action.h>
#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef LAYER_CREATION
#define LAYER_CREATION(button, stockid, tooltip)	\
	{ \
		Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID(stockid), \
			Gtk::ICON_SIZE_SMALL_TOOLBAR)); \
		button.add(*icon); \
        button.set_focus_on_click(false); \
	} \
	button.set_relief(Gtk::RELIEF_NONE); \
	button.set_tooltip_text(tooltip); \
	button.signal_toggled().connect(sigc::mem_fun(*this, \
		&studio::StateText_Context::toggle_layer_creation))
#endif

const int GAP = 3;

/* === G L O B A L S ======================================================= */

StateText studio::state_text;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateText_Context
{
	etl::handle<CanvasView> canvas_view;
	CanvasView::IsWorking is_working;

	Duckmatic::Push duckmatic_push;

	void refresh_ducks();

	bool prev_workarea_layer_status_;

	// Toolbox settings
	synfigapp::Settings& settings;

	Gtk::Grid options_grid;

	Gtk::Label title_label;

	Gtk::Label id_label;
	Gtk::Entry id_entry;
	Gtk::Box id_box;

	Gtk::Label layer_types_label;
	Gtk::ToggleButton layer_text_togglebutton;
	Gtk::Box layer_types_box;

	Gtk::Label blend_label;
	Widget_Enum blend_enum;
	Gtk::Box blend_box;

	Gtk::Label opacity_label;
	Gtk::Scale opacity_hscl;

	Gtk::Label family_label;
	Widget_FontFamily fontfamily_widget;

	Gtk::Label size_label;
	Widget_Vector size_widget;

	Gtk::Label orientation_label;
	Widget_Vector orientation_widget;

	Gtk::Label paragraph_label;
	Gtk::CheckButton paragraph_checkbutton;
	Gtk::Box paragraph_box;

public:

	synfig::String get_id()const { return id_entry.get_text(); }
	void set_id(const synfig::String& x) { return id_entry.set_text(x); }

	bool get_layer_text_flag()const { return layer_text_togglebutton.get_active(); }
	void set_layer_text_flag(bool x) { return layer_text_togglebutton.set_active(x); }

	int get_blend()const { return blend_enum.get_value(); }
	void set_blend(int x) { return blend_enum.set_value(x); }

	Real get_opacity()const { return opacity_hscl.get_value(); }
	void set_opacity(Real x) { opacity_hscl.set_value(x); }

	bool get_paragraph_flag()const { return paragraph_checkbutton.get_active(); }
	void set_paragraph_flag(bool x) { return paragraph_checkbutton.set_active(x); }

	Vector get_size() { return size_widget.get_value(); }
	void set_size(Vector s) { return size_widget.set_value(s); }

	Vector get_orientation() { return orientation_widget.get_value(); }
	void set_orientation(Vector s) { return orientation_widget.set_value(s); }

	String get_family()const { return fontfamily_widget.get_value(); }
	void set_family(String s) { return fontfamily_widget.set_value(s); }

	bool layer_text_flag;

	void refresh_tool_options(); //to refresh the toolbox

	//events
	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	Smach::event_result event_workarea_mouse_button_down_handler(const Smach::event& x);

	//constructor destructor
	StateText_Context(CanvasView *canvas_view);
	~StateText_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view->canvas_interface();}
	WorkArea * get_work_area()const{return canvas_view->get_work_area();}

	//Modifying settings etc.
	void load_settings();
	void save_settings();
	void reset();
	void increment_id();
	bool egress_on_selection_change;
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& /*x*/)
	{
		if(egress_on_selection_change)
			throw &state_normal; //throw Smach::egress_exception();
		return Smach::RESULT_OK;
	}

	void make_text(const Point& point);
	void toggle_layer_creation();

}; // END of class StateText_Context

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

StateText::StateText():
	Smach::state<StateText_Context>("text")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,&StateText_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_STOP,&StateText_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateText_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,&StateText_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateText_Context::event_workarea_mouse_button_down_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateText_Context::event_refresh_tool_options));
}

StateText::~StateText()
{
}

void* StateText::enter_state(studio::CanvasView* machine_context) const
{
	return new StateText_Context(machine_context);
}

void
StateText_Context::load_settings()
{
	try
	{
		Vector v;

		//parse the arguments yargh!
		set_id(settings.get_value("text.id", "Text"));

		set_blend(settings.get_value("text.blend", Color::BLEND_COMPOSITE));

		set_opacity(settings.get_value("text.opacity", 1.0));

		set_paragraph_flag(settings.get_value("text.paragraph", false));

		v[0] = settings.get_value("text.size_x", 0.25);
		v[1] = settings.get_value("text.size_y", 0.25);
		set_size(v);

		v[0] = settings.get_value("text.orient_x", 0.5);
		v[1] = settings.get_value("text.orient_y", 0.5);
		set_orientation(v);

		set_family(settings.get_value("text.family", "Sans Serif"));

		// since we have only text layer creation button, always turn it on.
//		set_layer_text_flag(settings.get_value("text.layer_text", true));
		set_layer_text_flag(true);

		// determine layer flags
		layer_text_flag = get_layer_text_flag();
	}
	catch(...)
	{
		synfig::warning("State Text: Caught exception when attempting to load settings.");
	}
}

void
StateText_Context::save_settings()
{
	try
	{
		settings.set_value("text.id",get_id());
		settings.set_value("text.layer_polygon",get_layer_text_flag());
		settings.set_value("text.blend",get_blend());
		settings.set_value("text.opacity",get_opacity());
		settings.set_value("text.paragraph",get_paragraph_flag());
		settings.set_value("text.size_x",get_size()[0]);
		settings.set_value("text.size_y",get_size()[1]);
		settings.set_value("text.orient_x",get_orientation()[0]);
		settings.set_value("text.orient_y",get_orientation()[1]);
		settings.set_value("text.family",get_family());
	}
	catch(...)
	{
		synfig::warning("State Text: Caught exception when attempting to save settings.");
	}
}

void
StateText_Context::reset()
{
	refresh_ducks();
}

void
StateText_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="Text";

	// If there is a number
	// already at the end of the
	// id, then remove it.
	if(id[id.size()-1]<='9' && id[id.size()-1]>='0')
	{
		// figure out how many digits it is
		for (digits = 0;
			 (int)id.size()-1 >= digits && id[id.size()-1-digits] <= '9' && id[id.size()-1-digits] >= '0';
			 digits++)
			;

		String str_number;
		str_number=String(id,id.size()-digits,id.size());
		id=String(id,0,id.size()-digits);

		number=atoi(str_number.c_str());
	}
	else
	{
		number=1;
		digits=3;
	}

	number++;

	// Add the number back onto the id
	{
		const String format(strprintf("%%0%dd",digits));
		id+=strprintf(format.c_str(),number);
	}

	// Set the ID
	set_id(id);
}

StateText_Context::StateText_Context(CanvasView *canvasView):
	canvas_view(canvasView),
	is_working(*canvasView),
	duckmatic_push(get_work_area()),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	opacity_hscl(Gtk::Adjustment::create(1.0, 0.0, 1.0, 0.01, 0.1))
{
	egress_on_selection_change=true;

	// Toolbox widgets
	title_label.set_label(_("Text Tool"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_hexpand();
	title_label.set_halign(Gtk::ALIGN_START);
	title_label.set_valign(Gtk::ALIGN_CENTER);

	id_label.set_label(_("Name:"));
	id_label.set_halign(Gtk::ALIGN_START);
	id_label.set_valign(Gtk::ALIGN_CENTER);
	id_label.get_style_context()->add_class("gap");
	id_box.pack_start(id_label, false, false, 0);
	id_box.pack_start(id_entry, true, true, 0);

	layer_types_label.set_label(_("Layer Type:"));
	layer_types_label.set_halign(Gtk::ALIGN_START);
	layer_types_label.set_valign(Gtk::ALIGN_CENTER);

	LAYER_CREATION(layer_text_togglebutton,
		("synfig-layer_other_text"), _("Create a text layer"));

	layer_text_togglebutton.get_style_context()->add_class("indentation");
	layer_types_box.pack_start(layer_text_togglebutton, false, false, 0);

	blend_label.set_label(_("Blend Method:"));
	blend_label.set_halign(Gtk::ALIGN_START);
	blend_label.set_valign(Gtk::ALIGN_CENTER);
	blend_label.get_style_context()->add_class("gap");
	blend_box.pack_start(blend_label, false, false, 0);

	blend_enum.set_param_desc(ParamDesc(Color::BLEND_COMPOSITE,"blend_method")
		.set_local_name(_("Blend Method"))
		.set_description(_("Defines the blend method to be used for texts")));

	opacity_label.set_label(_("Opacity:"));
	opacity_label.set_halign(Gtk::ALIGN_START);
	opacity_label.set_valign(Gtk::ALIGN_CENTER);

	opacity_hscl.set_digits(2);
	opacity_hscl.set_value_pos(Gtk::POS_LEFT);
	opacity_hscl.set_tooltip_text(_("Opacity"));

	family_label.set_label(_("Font Family:"));
	family_label.set_halign(Gtk::ALIGN_START);
	family_label.set_valign(Gtk::ALIGN_CENTER);

	size_label.set_label(_("Size:"));
	size_label.set_halign(Gtk::ALIGN_START);
	size_label.set_valign(Gtk::ALIGN_CENTER);

	size_widget.set_digits(2);
	size_widget.set_canvas(canvas_view->get_canvas());

	orientation_label.set_label(_("Orientation:"));
	orientation_label.set_halign(Gtk::ALIGN_START);
	orientation_label.set_valign(Gtk::ALIGN_CENTER);

	orientation_widget.set_digits(2);

	paragraph_label.set_label(_("Multiline Text"));
	paragraph_label.set_halign(Gtk::ALIGN_START);
	paragraph_label.set_valign(Gtk::ALIGN_CENTER);
	paragraph_box.pack_start(paragraph_label, true, true, 0);
	paragraph_box.pack_start(paragraph_checkbutton, false, false, 0);

	// Toolbox layout
	options_grid.attach(title_label,
		0, 0, 2, 1);
	options_grid.attach(id_box,
		0, 1, 2, 1);
	options_grid.attach(layer_types_label,
		0, 2, 2, 1);
	options_grid.attach(layer_types_box,
		0, 3, 2, 1);
	options_grid.attach(blend_box,
		0, 4, 1, 1);
	options_grid.attach(blend_enum,
		1, 4, 1, 1);
	options_grid.attach(opacity_label,
		0, 5, 1, 1);
	options_grid.attach(opacity_hscl,
		1, 5, 1, 1);
	options_grid.attach(family_label,
		0, 6, 1, 1);
	options_grid.attach(fontfamily_widget,
		1, 6, 1, 1);
	options_grid.attach(size_label,
		0, 7, 1, 1);
	options_grid.attach(size_widget,
		1, 7, 1, 1);
	options_grid.attach(orientation_label,
		0, 8, 1, 1);
	options_grid.attach(orientation_widget,
		1, 8, 1, 1);
	options_grid.attach(paragraph_box,
		0, 9, 2, 1);

	options_grid.set_vexpand(false);
	options_grid.set_border_width(GAP*2);
	options_grid.set_row_spacing(GAP);
	options_grid.set_margin_bottom(0);
	options_grid.show_all();

	load_settings();

	refresh_tool_options();
	App::dialog_tool_options->present();

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// clear out the ducks
	get_work_area()->clear_ducks();

	// Refresh the work area
	get_work_area()->queue_draw();

	// Hide the tables if they are showing
	//prev_table_status=get_canvas_view()->tables_are_visible();
	//if(prev_table_status)get_canvas_view()->hide_tables();

	// Disable the time bar
	//get_canvas_view()->set_sensitive_timebar(false);

	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StateText_Context::on_user_click));
	get_work_area()->set_cursor(Gdk::XTERM);

	App::dock_toolbox->refresh();
}

void
StateText_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_grid);
	App::dialog_tool_options->set_local_name(_("Text Tool"));
	App::dialog_tool_options->set_name("text");
}

Smach::event_result
StateText_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateText_Context::~StateText_Context()
{
	save_settings();

	// Restore layer clicking
	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_status_);
	get_work_area()->reset_cursor();

	App::dialog_tool_options->clear();

	get_work_area()->queue_draw();

	get_canvas_view()->queue_rebuild_ducks();

	App::dock_toolbox->refresh();
}

Smach::event_result
StateText_Context::event_stop_handler(const Smach::event& /*x*/)
{
	//throw Smach::egress_exception();
	throw &state_normal;
	return Smach::RESULT_OK;
}

Smach::event_result
StateText_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

void
StateText_Context::make_text(const Point& _point)
{
	if (get_layer_text_flag())
	{

	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("New Text"));
	synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

	Layer::Handle layer;

	Canvas::Handle canvas(get_canvas_view()->get_canvas());
	int depth(0);

	// we are temporarily using the layer to hold something
	layer=get_canvas_view()->get_selection_manager()->get_selected_layer();
	if(layer)
	{
		depth=layer->get_depth();
		canvas=layer->get_canvas();
	}

	synfigapp::SelectionManager::LayerList layer_selection;

	const synfig::TransformStack& transform(get_work_area()->get_curr_transform_stack());
	const Point point(transform.unperform(_point));

	// Set blend_method to static (consistent with other Layers)
	ValueBase blend_param_value(get_blend());
	blend_param_value.set_static(true);

	String text;
	if (get_paragraph_flag())
		App::dialog_paragraph(_("Text Paragraph"), _("Enter text here:"), text);
	else
		if (!App::dialog_entry(_("Input text"), _("Text: "), text, _("Cancel"), _("Ok"))) return;

	egress_on_selection_change=false;
	layer=get_canvas_interface()->add_layer_to("text",canvas,depth);
	egress_on_selection_change=true;
	if (!layer)
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
		group.cancel();
		return;
	}
	layer_selection.push_back(layer);

	layer->set_param("blend_method", blend_param_value);
	get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

	layer->set_param("amount", get_opacity());
	get_canvas_interface()->signal_layer_param_changed()(layer, "amount");

	layer->set_param("origin",point);
	get_canvas_interface()->signal_layer_param_changed()(layer,"origin");

	layer->set_param("text",text);
	get_canvas_interface()->signal_layer_param_changed()(layer,"text");

	layer->set_param("size",get_size());
	get_canvas_interface()->signal_layer_param_changed()(layer,"size");

	layer->set_param("orient",get_orientation());
	get_canvas_interface()->signal_layer_param_changed()(layer,"orient");

	layer->set_param("family",get_family());
	get_canvas_interface()->signal_layer_param_changed()(layer,"family");
	/*
	layer->set_description(get_id());
	get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());
	*/
	egress_on_selection_change=false;
	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	get_canvas_interface()->get_selection_manager()->set_selected_layers(layer_selection);
	egress_on_selection_change=true;

	reset();
	increment_id();
}
}

Smach::event_result
StateText_Context::event_workarea_mouse_button_down_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	if(event.button==BUTTON_LEFT)
	{
		make_text(get_work_area()->snap_point_to_grid(event.pos));

		get_work_area()->clear_ducks();
		return Smach::RESULT_ACCEPT;
	}
	return Smach::RESULT_OK;
}

void
StateText_Context::refresh_ducks()
{
	get_work_area()->clear_ducks();
	get_work_area()->queue_draw();
}

void
StateText_Context::toggle_layer_creation()
{
  // don't allow none layer creation
  if (get_layer_text_flag() == 0)
  {
    if(layer_text_flag) set_layer_text_flag(true);
  }

  // update layer flags
  layer_text_flag = get_layer_text_flag();
}
