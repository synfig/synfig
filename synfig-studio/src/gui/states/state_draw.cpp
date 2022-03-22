/* === S Y N F I G ========================================================= */
/*!	\file state_draw.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
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

#include <gui/states/state_draw.h>

#include <gtkmm/radiobutton.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/event_mouse.h>
#include <gui/localization.h>
#include <gui/states/state_normal.h>
#include <gui/states/state_stroke.h>
#include <gui/widgets/widget_distance.h>
#include <gui/widgets/widget_enum.h>
#include <gui/workarea.h>

#include <synfig/general.h>
#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/valuenodes/valuenode_wplist.h>
#include <synfig/valuenodes/valuenode_composite.h>

#include <synfigapp/blineconvert.h>
#include <synfigapp/main.h>
#include <synfigapp/wplistconverter.h>

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
        button.set_focus_on_click(false);\
	} \
	button.set_relief(Gtk::RELIEF_NONE); \
	button.set_tooltip_text(tooltip); \
	button.signal_toggled().connect(sigc::mem_fun(*this, \
		&studio::StateDraw_Context::toggle_layer_creation))
#endif

const int GAP = 3;

/* === G L O B A L S ======================================================= */

StateDraw studio::state_draw;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateDraw_Context : public sigc::trackable
{
	typedef etl::smart_ptr<std::list<synfig::Point> > StrokeData;
	typedef etl::smart_ptr<std::list<synfig::Real> > WidthData;

	typedef std::list< std::pair<StrokeData,WidthData> > StrokeQueue;

	StrokeQueue stroke_queue;


	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	WorkArea::PushState push_state;

	bool prev_table_status;
	//bool loop_;

	int nested;
	sigc::connection process_queue_connection;

	ValueNode_BLine::Handle last_stroke;
	synfig::String last_stroke_id;
	ValueNode::Handle last_value_node_origin;

	Gtk::Menu menu;

	std::list< etl::smart_ptr<std::list<synfig::Point> > > stroke_list;

	void refresh_ducks();

	void fill_last_stroke();
	Smach::event_result fill_last_stroke_and_unselect_other_layers();

	Smach::event_result new_bline(std::list<synfig::BLinePoint> bline, std::list<synfig::WidthPoint> wplist, bool loop_bline_flag, float radius, ValueNode::Handle& value_node_origin);
	Smach::event_result new_region(std::list<synfig::BLinePoint> bline, synfig::Real radius, ValueNode::Handle& value_node_origin);

	Smach::event_result extend_bline_from_begin(ValueNode_BLine::Handle value_node,std::list<synfig::BLinePoint> bline,std::list<synfig::WidthPoint> wplist,bool complete_loop);
	Smach::event_result extend_bline_from_end(ValueNode_BLine::Handle value_node,std::list<synfig::BLinePoint> bline,std::list<synfig::WidthPoint> wplist,bool complete_loop);
	void reverse_bline(std::list<synfig::BLinePoint> &bline);
	void reverse_wplist(std::list<synfig::WidthPoint> &wplist);

	bool link_layer_origin(Layer::Handle& layer, ValueNode::Handle &value_node) const;

	// Toolbox settings
	synfigapp::Settings& settings;

	Gtk::Grid options_grid;

	Gtk::Label title_label;

	Gtk::Label id_label;
	Gtk::Entry id_entry;
	Gtk::Box id_box;

	Gtk::Label layer_types_label;
	Gtk::ToggleButton layer_region_togglebutton;
	Gtk::ToggleButton layer_outline_togglebutton;
	Gtk::ToggleButton layer_advanced_outline_togglebutton;
	Gtk::Box layer_types_box;

	Gtk::Label blend_label;
	Widget_Enum blend_enum;
	Gtk::Box blend_box;

	Gtk::Label opacity_label;
	Gtk::Scale opacity_hscl;

	Gtk::Label bline_width_label;
	Widget_Distance bline_width_dist;

	Gtk::Label pressure_width_label;
	Gtk::CheckButton pressure_width_checkbutton;
	Gtk::Box pressure_width_box;

	Gtk::Label min_pressure_label;
	Gtk::Box min_pressure_label_box;

	Gtk::CheckButton min_pressure_checkbutton;
	Glib::RefPtr<Gtk::Adjustment> min_pressure_adj;
	Gtk::SpinButton  min_pressure_spin;
	Gtk::Box min_pressure_box;

	Gtk::Label smoothness_label;
	Gtk::RadioButton::Group smoothness_group;

	Gtk::RadioButton localthres_radiobutton;
	Glib::RefPtr<Gtk::Adjustment> localthres_adj;
	Gtk::SpinButton localthres_spin;
	Gtk::Box localthres_box;

	Gtk::RadioButton globalthres_radiobutton;
	Glib::RefPtr<Gtk::Adjustment> globalthres_adj;
	Gtk::SpinButton globalthres_spin;
	Gtk::Box globalthres_box;

	Gtk::Label width_max_error_label;
	Gtk::Box width_max_error_box;
	Glib::RefPtr<Gtk::Adjustment> width_max_error_adj;
	Gtk::SpinButton width_max_error_spin;

	Gtk::Label round_ends_label;
	Gtk::CheckButton round_ends_checkbutton;
	Gtk::Box round_ends_box;

	Gtk::Label auto_loop_label;
	Gtk::CheckButton auto_loop_checkbutton;
	Gtk::Box auto_loop_box;

	Gtk::Label auto_extend_label;
	Gtk::CheckButton auto_extend_checkbutton;
	Gtk::Box auto_extend_box;

	Gtk::Label auto_link_label;
	Gtk::CheckButton auto_link_checkbutton;
	Gtk::Box auto_link_box;

	Gtk::Label feather_label;
	Widget_Distance feather_dist;

	Gtk::Label link_origins_label;
	Gtk::CheckButton layer_link_origins_checkbutton;
	Gtk::Box link_origins_box;

	Gtk::Label auto_export_label;
	Gtk::CheckButton auto_export_checkbutton;
	Gtk::Box auto_export_box;

	Gtk::Button fill_last_stroke_button;


	void UpdateUsePressure();
	void UpdateCreateAdvancedOutline();
	void UpdateSmoothness();

	//Added by Adrian - data drive HOOOOO
	synfigapp::BLineConverter blineconv;
	synfigapp::WPListConverter wplistconv;

public:
	synfig::String get_id()const { return id_entry.get_text(); }
	void set_id(const synfig::String& x) { return id_entry.set_text(x); }

	int get_blend()const { return blend_enum.get_value(); }
	void set_blend(int x) { return blend_enum.set_value(x); }

	Real get_opacity()const { return opacity_hscl.get_value(); }
	void set_opacity(Real x) { opacity_hscl.set_value(x); }

	Real get_bline_width() const {
		return bline_width_dist.get_value().get(
			Distance::SYSTEM_UNITS,
			get_canvas_view()->get_canvas()->rend_desc()
		);
	}
	void set_bline_width(Distance x) { return bline_width_dist.set_value(x);}

	bool get_pressure_width_flag()const { return pressure_width_checkbutton.get_active(); }
	void set_pressure_width_flag(bool x) { return pressure_width_checkbutton.set_active(x); }

	bool get_auto_loop_flag()const { return auto_loop_checkbutton.get_active(); }
	void set_auto_loop_flag(bool x) { return auto_loop_checkbutton.set_active(x); }

	bool get_auto_extend_flag()const { return auto_extend_checkbutton.get_active(); }
	void set_auto_extend_flag(bool x) { return auto_extend_checkbutton.set_active(x); }

	bool get_auto_link_flag()const { return auto_link_checkbutton.get_active(); }
	void set_auto_link_flag(bool x) { return auto_link_checkbutton.set_active(x); }

	bool get_layer_region_flag()const { return layer_region_togglebutton.get_active(); }
	void set_layer_region_flag(bool x) { return layer_region_togglebutton.set_active(x); }

	bool get_layer_outline_flag()const { return layer_outline_togglebutton.get_active(); }
	void set_layer_outline_flag(bool x) { return layer_outline_togglebutton.set_active(x); }

	bool get_layer_advanced_outline_flag()const { return layer_advanced_outline_togglebutton.get_active(); }
	void set_layer_advanced_outline_flag(bool x) { return layer_advanced_outline_togglebutton.set_active(x); }

	bool get_layer_link_origins_flag()const { return layer_link_origins_checkbutton.get_active(); }
	void set_layer_link_origins_flag(bool x) { return layer_link_origins_checkbutton.set_active(x); }

	bool get_auto_export_flag()const { return auto_export_checkbutton.get_active(); }
	void set_auto_export_flag(bool x) { return auto_export_checkbutton.set_active(x); }

	Real get_min_pressure() const { return min_pressure_adj->get_value(); }
	void set_min_pressure(Real x) { return min_pressure_adj->set_value(x); }

	Real get_feather_size() const {
		return feather_dist.get_value().get(Distance::SYSTEM_UNITS,
		get_canvas_view()->get_canvas()->rend_desc());
	}
	void set_feather_size(Distance x) { return feather_dist.set_value(x); }

	Real get_gthres() const { return globalthres_adj->get_value(); }
	void set_gthres(Real x) { return globalthres_adj->set_value(x); }

	Real get_lthres() const { return localthres_adj->get_value(); }
	void set_lthres(Real x) { return localthres_adj->set_value(x); }

	Real get_width_max_error() const { return width_max_error_adj->get_value(); }
	void set_width_max_error(Real x) { return width_max_error_adj->set_value(x); }

	bool get_local_threshold_flag() const { return localthres_radiobutton.get_active(); }
	void set_local_threshold_flag(bool x) { localthres_radiobutton.set_active(x); }

	bool get_global_threshold_flag() const { return globalthres_radiobutton.get_active(); }
	void set_global_threshold_flag(bool x) { globalthres_radiobutton.set_active(x); }

	bool get_min_pressure_flag()const { return min_pressure_checkbutton.get_active(); }
	void set_min_pressure_flag(bool x) { min_pressure_checkbutton.set_active(x); }

	bool get_round_ends_flag()const { return round_ends_checkbutton.get_active();}
	void set_round_ends_flag(bool x) {round_ends_checkbutton.set_active(x);}

  bool layer_region_flag;
  bool layer_outline_flag;
  bool layer_advanced_outline_flag;

	void load_settings();
	void save_settings();
	void increment_id();

	Smach::event_result event_stop_handler(const Smach::event& x);

	Smach::event_result event_refresh_handler(const Smach::event& x);

	Smach::event_result event_mouse_down_handler(const Smach::event& x);

	Smach::event_result event_stroke(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	void refresh_tool_options();

	Smach::event_result process_stroke(StrokeData stroke_data, WidthData width_data, bool region_flag=false);

	bool process_queue();


	StateDraw_Context(CanvasView* canvas_view);

	~StateDraw_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Time get_time()const { return get_canvas_interface()->get_time(); }
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	//void on_user_click(synfig::Point point);

//	bool run();

	void toggle_layer_creation();

};	// END of class StateDraw_Context


/* === M E T H O D S ======================================================= */

StateDraw::StateDraw():
	Smach::state<StateDraw_Context>("draw")
{
	insert(event_def(EVENT_STOP,&StateDraw_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateDraw_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateDraw_Context::event_mouse_down_handler));
	insert(event_def(EVENT_WORKAREA_STROKE,&StateDraw_Context::event_stroke));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateDraw_Context::event_refresh_tool_options));
}

StateDraw::~StateDraw()
{
}

void* StateDraw::enter_state(studio::CanvasView* machine_context) const
{
	return new StateDraw_Context(machine_context);
}

void
StateDraw_Context::load_settings()
{
	try
	{
		set_id(settings.get_value("draw.id","NewDrawing"));

		set_blend(settings.get_value("draw.blend", int(Color::BLEND_COMPOSITE)));

		set_opacity(settings.get_value("draw.opacity", 1.0));

		set_bline_width(settings.get_value("draw.bline_width", Distance("1px")));

		set_pressure_width_flag(settings.get_value("draw.pressure_width", true));

		set_auto_loop_flag(settings.get_value("draw.auto_loop", true));

		set_auto_extend_flag(settings.get_value("draw.auto_extend", true));

		set_auto_link_flag(settings.get_value("draw.auto_link", true));

		set_layer_region_flag(settings.get_value("draw.region", true));

		set_layer_outline_flag(settings.get_value("draw.outline", true));

		set_layer_advanced_outline_flag(settings.get_value("draw.advanced_outline", true));

		set_layer_link_origins_flag(settings.get_value("draw.layer_link_origins", true));

		set_auto_export_flag(settings.get_value("draw.auto_export", false));

		set_min_pressure_flag(settings.get_value("draw.min_pressure_on", true));

		set_min_pressure(settings.get_value("draw.min_pressure", 0.0));

		set_feather_size(settings.get_value("draw.feather", Distance("0px")));

		set_gthres(settings.get_value("draw.gthreshold", 0.7));

		set_width_max_error(settings.get_value("draw.widthmaxerror", 1.0));

		set_lthres(settings.get_value("draw.lthreshold", 20.0));

		set_local_threshold_flag(settings.get_value("draw.localize", false));

		set_round_ends_flag(settings.get_value("draw.round_ends", false));

		// determine layer flags
		layer_region_flag = get_layer_region_flag();
		layer_outline_flag = get_layer_outline_flag();
		layer_advanced_outline_flag = get_layer_advanced_outline_flag();
	}
	catch(...)
	{
		synfig::warning("State Draw: Caught exception when attempting to load settings.");
	}
}

void
StateDraw_Context::save_settings()
{
	try
	{
		settings.set_value("draw.id",get_id());
		settings.set_value("draw.blend",get_blend());
		settings.set_value("draw.opacity",get_opacity());
		settings.set_value("draw.bline_width", bline_width_dist.get_value());
		settings.set_value("draw.pressure_width",get_pressure_width_flag());
		settings.set_value("draw.auto_loop",get_auto_loop_flag());
		settings.set_value("draw.auto_extend",get_auto_extend_flag());
		settings.set_value("draw.auto_link",get_auto_link_flag());
		settings.set_value("draw.region",get_layer_region_flag());
		settings.set_value("draw.outline",get_layer_outline_flag());
		settings.set_value("draw.advanced_outline",get_layer_advanced_outline_flag());
		settings.set_value("draw.layer_link_origins",get_layer_link_origins_flag());
		settings.set_value("draw.auto_export",get_auto_export_flag());
		settings.set_value("draw.min_pressure",get_min_pressure());
		settings.set_value("draw.feather",feather_dist.get_value());
		settings.set_value("draw.min_pressure_on",get_min_pressure_flag());
		settings.set_value("draw.gthreshold",get_gthres());
		settings.set_value("draw.widthmaxerror",get_width_max_error());
		settings.set_value("draw.lthreshold",get_lthres());
		settings.set_value("draw.localize",get_local_threshold_flag());
		settings.set_value("draw.round_ends", get_round_ends_flag());
	}
	catch(...)
	{
		synfig::warning("State Draw: Caught exception when attempting to save settings.");
	}
}

void
StateDraw_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="Drawing";

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
		// synfig::info("---------------- \"%s\"",str_number.c_str());

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

StateDraw_Context::StateDraw_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	push_state(*get_work_area()),
	//loop_(false),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	opacity_hscl(Gtk::Adjustment::create(1.0, 0.0, 1.0, 0.01, 0.1)),
	min_pressure_adj(Gtk::Adjustment::create(0,0,1,0.01,0.1)),
	min_pressure_spin(min_pressure_adj,0.1,3),
	localthres_adj(Gtk::Adjustment::create(20, 1, 100000, 0.1, 1)),
	localthres_spin(localthres_adj, 0.1, 1),
	globalthres_adj(Gtk::Adjustment::create(0.70f, 0.01, 10000, 0.01, 0.1)),
	globalthres_spin(globalthres_adj, 0.01, 3),
	width_max_error_adj(Gtk::Adjustment::create(1.0f, 0.01, 100.0, 0.1,1)),
	width_max_error_spin(width_max_error_adj, 0.01, 2),
	fill_last_stroke_button(_("Fill Last Stroke"))
{
	// Toolbox widgets
	title_label.set_label(_("Draw Tool"));
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

	LAYER_CREATION(layer_region_togglebutton,
		("synfig-layer_geometry_region"), _("Create a region layer"));
	LAYER_CREATION(layer_outline_togglebutton,
		("synfig-layer_geometry_outline"), _("Create an outline layer"));
	LAYER_CREATION(layer_advanced_outline_togglebutton,
		("synfig-layer_geometry_advanced_outline"), _("Create an advanced outline layer"));

	layer_region_togglebutton.get_style_context()->add_class("indentation");

	layer_types_box.pack_start(layer_region_togglebutton, false, false, 0);
	layer_types_box.pack_start(layer_outline_togglebutton, false, false, 0);
	layer_types_box.pack_start(layer_advanced_outline_togglebutton, false, false, 0);

	blend_label.set_label(_("Blend Method:"));
	blend_label.set_halign(Gtk::ALIGN_START);
	blend_label.set_valign(Gtk::ALIGN_CENTER);
	blend_label.get_style_context()->add_class("gap");
	blend_box.pack_start(blend_label, false, false, 0);

	blend_enum.set_param_desc(ParamDesc(Color::BLEND_COMPOSITE,"blend_method")
		.set_local_name(_("Blend Method"))
		.set_description(_("Defines the blend method to be used for draws")));

	opacity_label.set_label(_("Opacity:"));
	opacity_label.set_halign(Gtk::ALIGN_START);
	opacity_label.set_valign(Gtk::ALIGN_CENTER);

	opacity_hscl.set_digits(2);
	opacity_hscl.set_value_pos(Gtk::POS_LEFT);
	opacity_hscl.set_tooltip_text(_("Opacity"));

	bline_width_label.set_label(_("Brush Size:"));
	bline_width_label.set_halign(Gtk::ALIGN_START);
	bline_width_label.set_valign(Gtk::ALIGN_CENTER);

	bline_width_dist.set_digits(2);
	bline_width_dist.set_range(0,10000000);

	pressure_width_label.set_label(_("Pressure Sensitive"));
	pressure_width_label.set_halign(Gtk::ALIGN_START);
	pressure_width_label.set_valign(Gtk::ALIGN_CENTER);

	pressure_width_box.pack_start(pressure_width_label, true, true, 0);
	pressure_width_box.pack_start(pressure_width_checkbutton, false, false, 0);

	min_pressure_label.get_style_context()->add_class("indentation");
	min_pressure_spin.get_style_context()->add_class("gap");

	min_pressure_label.set_label(_("Min Width:"));
	min_pressure_label.set_halign(Gtk::ALIGN_START);
	min_pressure_label.set_valign(Gtk::ALIGN_CENTER);
	min_pressure_label_box.pack_start(min_pressure_label, false, false, 0);

	min_pressure_box.pack_start(min_pressure_spin, true, true, 0);
	min_pressure_box.pack_start(min_pressure_checkbutton, false, false, 0);

	smoothness_label.set_label(_("Smoothness"));
	smoothness_label.set_halign(Gtk::ALIGN_START);
	smoothness_label.set_valign(Gtk::ALIGN_CENTER);

	localthres_radiobutton.get_style_context()->add_class("indentation");
	localthres_box.pack_start(localthres_radiobutton, false, false, 0);
	localthres_radiobutton.set_label(_("Local:"));

	globalthres_radiobutton.get_style_context()->add_class("indentation");
	globalthres_box.pack_start(globalthres_radiobutton, false, false, 0);
	globalthres_radiobutton.set_label(_("Global:"));

	smoothness_group = localthres_radiobutton.get_group();
	globalthres_radiobutton.set_group(smoothness_group);

	width_max_error_label.set_label(_("Width Max Error:"));
	width_max_error_label.get_style_context()->add_class("gap");
	width_max_error_label.set_halign(Gtk::ALIGN_START);
	width_max_error_label.set_valign(Gtk::ALIGN_CENTER);
	width_max_error_box.pack_start(width_max_error_label, false, false, 0);

	round_ends_label.set_label(_("Round Ends"));
	round_ends_label.set_halign(Gtk::ALIGN_START);
	round_ends_label.set_valign(Gtk::ALIGN_CENTER);

	round_ends_box.pack_start(round_ends_label, true, true, 0);
	round_ends_box.pack_start(round_ends_checkbutton, false, false, 0);

	auto_loop_label.set_label(_("Auto Loop"));
	auto_loop_label.set_halign(Gtk::ALIGN_START);
	auto_loop_label.set_valign(Gtk::ALIGN_CENTER);

	auto_loop_box.pack_start(auto_loop_label, true, true, 0);
	auto_loop_box.pack_start(auto_loop_checkbutton, false, false, 0);

	auto_extend_label.set_label(_("Auto Extend"));
	auto_extend_label.set_halign(Gtk::ALIGN_START);
	auto_extend_label.set_valign(Gtk::ALIGN_CENTER);

	auto_extend_box.pack_start(auto_extend_label, true, true, 0);
	auto_extend_box.pack_start(auto_extend_checkbutton, false, false, 0);

	auto_link_label.set_label(_("Auto Link"));
	auto_link_label.set_halign(Gtk::ALIGN_START);
	auto_link_label.set_valign(Gtk::ALIGN_CENTER);

	auto_link_box.pack_start(auto_link_label, true, true, 0);
	auto_link_box.pack_start(auto_link_checkbutton, false, false, 0);

	feather_label.set_label(_("Feather:"));
	feather_label.set_halign(Gtk::ALIGN_START);
	feather_label.set_valign(Gtk::ALIGN_CENTER);

	feather_dist.set_digits(2);
	feather_dist.set_range(0,10000000);

	link_origins_label.set_label(_("Link Origins"));
	link_origins_label.set_halign(Gtk::ALIGN_START);
	link_origins_label.set_valign(Gtk::ALIGN_CENTER);

	link_origins_box.pack_start(link_origins_label, true, true, 0);
	link_origins_box.pack_start(layer_link_origins_checkbutton, false, false, 0);
	link_origins_box.set_sensitive(false);

	auto_export_label.set_label(_("Auto Export"));
	auto_export_label.set_halign(Gtk::ALIGN_START);
	auto_export_label.set_valign(Gtk::ALIGN_CENTER);

	auto_export_box.pack_start(auto_export_label, true, true, 0);
	auto_export_box.pack_start(auto_export_checkbutton, false, false, 0);

	nested=0;
	load_settings();

	UpdateUsePressure();
	UpdateCreateAdvancedOutline();
	UpdateSmoothness();

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
	options_grid.attach(bline_width_label,
		0, 6, 1, 1);
	options_grid.attach(bline_width_dist,
		1, 6, 1, 1);
	options_grid.attach(pressure_width_box,
		0, 7, 2, 1);
	options_grid.attach(min_pressure_label_box,
		0, 8, 1, 1);
	options_grid.attach(min_pressure_box,
		1, 8, 1, 1);
	options_grid.attach(smoothness_label,
		0, 9, 2, 1);
	options_grid.attach(localthres_box,
		0, 10, 1, 1);
	options_grid.attach(localthres_spin,
		1, 10, 1, 1);
	options_grid.attach(globalthres_box,
		0, 11, 1, 1);
	options_grid.attach(globalthres_spin,
		1, 11, 1, 1);
	options_grid.attach(width_max_error_box,
		0, 12, 1, 1);
	options_grid.attach(width_max_error_spin,
		1, 12, 1, 1);
	options_grid.attach(round_ends_box,
		0, 13, 2, 1);
	options_grid.attach(auto_loop_box,
		0, 14, 2, 1);
	options_grid.attach(auto_extend_box,
		0, 15, 2, 1);
	options_grid.attach(auto_link_box,
		0, 16, 2, 1);
	options_grid.attach(feather_label,
		0, 17, 1, 1);
	options_grid.attach(feather_dist,
		1, 17, 1, 1);
	options_grid.attach(link_origins_box,
		0, 18, 2, 1);
	options_grid.attach(auto_export_box,
		0, 19, 2, 1);

	options_grid.set_vexpand(false);
	options_grid.set_border_width(GAP*2);
	options_grid.set_row_spacing(GAP);
	options_grid.set_margin_bottom(0);
	options_grid.show_all();

	fill_last_stroke_button.signal_pressed().connect(
		sigc::mem_fun(*this, &StateDraw_Context::fill_last_stroke));
	pressure_width_checkbutton.signal_toggled().connect(
		sigc::mem_fun(*this, &StateDraw_Context::UpdateUsePressure));
	layer_advanced_outline_togglebutton.signal_toggled().connect(
		sigc::mem_fun(*this, &StateDraw_Context::UpdateCreateAdvancedOutline));
	localthres_spin.signal_value_changed().connect(sigc::mem_fun(*this,
		&StateDraw_Context::UpdateSmoothness));
	globalthres_spin.signal_value_changed().connect(sigc::mem_fun(*this,
		&StateDraw_Context::UpdateSmoothness));

	refresh_tool_options();
	App::dialog_tool_options->present();

	// Hide all tangent and width ducks
	get_work_area()->set_type_mask(get_work_area()->get_type_mask()-Duck::TYPE_TANGENT-Duck::TYPE_WIDTH);
	get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// Turn off duck clicking
	get_work_area()->set_allow_duck_clicks(false);

	// Hide the tables if they are showing
	prev_table_status=get_canvas_view()->tables_are_visible();
	//if(prev_table_status)get_canvas_view()->hide_tables();

	// Disable the time bar
	get_canvas_view()->set_sensitive_timebar(false);

	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StateDraw_Context::on_user_click));

	get_work_area()->set_cursor(Gdk::PENCIL);

	App::dock_toolbox->refresh();

	refresh_ducks();
}


void
StateDraw_Context::UpdateUsePressure()
{
	bool status(get_pressure_width_flag());
	min_pressure_label.set_sensitive(status);
	min_pressure_checkbutton.set_sensitive(status);
	min_pressure_spin.set_sensitive(status);
}

void
StateDraw_Context::UpdateCreateAdvancedOutline()
{
	width_max_error_label.set_sensitive(get_layer_advanced_outline_flag());
	width_max_error_spin.set_sensitive(get_layer_advanced_outline_flag());
}


void
StateDraw_Context::UpdateSmoothness()
{
	localthres_radiobutton.set_active(localthres_spin.is_focus());
	globalthres_radiobutton.set_active(globalthres_spin.is_focus());
}


void
StateDraw_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_grid);
	App::dialog_tool_options->set_local_name(_("Draw Tool"));
	App::dialog_tool_options->set_name("draw");

	App::dialog_tool_options->add_button(
		Gtk::StockID("synfig-fill"),
		_("Fill Last Stroke")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&StateDraw_Context::fill_last_stroke));
}

Smach::event_result
StateDraw_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateDraw_Context::~StateDraw_Context()
{
	save_settings();

	App::dialog_tool_options->clear();

	get_work_area()->reset_cursor();

	// Enable the time bar
	get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();

	App::dock_toolbox->refresh();
}

Smach::event_result
StateDraw_Context::event_stop_handler(const Smach::event& /*x*/)
{
	//throw Smach::egress_exception();
	throw &state_normal;
	return Smach::RESULT_OK;
}

Smach::event_result
StateDraw_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateDraw_Context::event_mouse_down_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		{
			// Enter the stroke state to get the stroke
			get_canvas_view()->get_smach().push_state(&state_stroke);
			return Smach::RESULT_ACCEPT;
		}

	default:
		return Smach::RESULT_OK;
	}
}

#define SIMILAR_TANGENT_THRESHOLD	(0.2)

struct debugclass
{
	synfig::String x;
	debugclass(const synfig::String &x):x(x)
	{
//		synfig::warning(">>>>>>>>>>>>>>>>>>> "+x);
	}
	~debugclass()
	{
//		synfig::warning("<<<<<<<<<<<<<<<<<<< "+x);
	}
};

struct DepthCounter
{
	int &i;
	DepthCounter(int &i):i(i) { i++; }
	~DepthCounter() { i--; }
};

Smach::event_result
StateDraw_Context::event_stroke(const Smach::event& x)
{
//	debugclass debugger("StateDraw_Context::event_stroke(const Smach::event& x)");

	const EventStroke& event(*reinterpret_cast<const EventStroke*>(&x));

	assert(event.stroke_data);

	get_work_area()->add_stroke(event.stroke_data,synfigapp::Main::get_outline_color());

	if(nested==0)
	{
		WorkArea::DirtyTrap dirty_trap(*get_work_area());
		Smach::event_result result;
		result = process_stroke(event.stroke_data, event.width_data, (event.modifier&Gdk::CONTROL_MASK) || (event.modifier&Gdk::BUTTON2_MASK));
		process_queue();
		return result;
	}

	stroke_queue.push_back(std::pair<StrokeData,WidthData>(event.stroke_data,event.width_data));

	return Smach::RESULT_ACCEPT;
}

bool
StateDraw_Context::process_queue()
{
//	debugclass debugger("StateDraw_Context::process_queue()");
	if(nested)
		return true;
	DepthCounter depth_counter(nested);
	while(!stroke_queue.empty())
	{
		std::pair<StrokeData,WidthData> front(stroke_queue.front());
		process_stroke(front.first,front.second);
		stroke_queue.pop_front();
	}
	return false;
}

Smach::event_result
StateDraw_Context::process_stroke(StrokeData stroke_data, WidthData width_data, bool region_flag)
{
//	debugclass debugger("StateDraw_Context::process_stroke");
	DepthCounter depth_counter(nested);

	const float radius(
		// synfigapp::Main::get_bline_width().units(get_canvas()->rend_desc()) +
		get_bline_width() +
		(std::fabs(get_work_area()->get_pw())+ std::fabs(get_work_area()->get_ph()))*5);

	// If we aren't using pressure width,
	// then set all the width to 1
	if(!get_pressure_width_flag())
	{
		std::list<synfig::Real>::iterator iter;
		for(iter=width_data->begin();iter!=width_data->end();++iter)
		{
			*iter=1.0;
		}
	}

	std::list<synfig::BLinePoint> bline;
	std::list<synfig::WidthPoint> wplist;
	bool loop_bline_flag(false);

	//Changed by Adrian - use resident class :)
	//synfigapp::convert_stroke_to_bline(bline, *event.stroke_data,*event.width_data, synfigapp::Main::get_bline_width());
	// blineconv.width = synfigapp::Main::get_bline_width().units(get_canvas()->rend_desc());
	blineconv.width = get_bline_width();

	if (get_local_threshold_flag())
	{
		float pw = get_work_area()->get_pw();
		float ph = get_work_area()->get_ph();

		blineconv.pixelwidth = sqrt(pw*pw+ph*ph);
		blineconv.smoothness = get_lthres();
	}

	if (get_global_threshold_flag())
	{
		blineconv.pixelwidth = 1;
		blineconv.smoothness = get_gthres();
	}

	blineconv(bline,*stroke_data,*width_data);

	if(get_layer_advanced_outline_flag())
	{
		wplistconv.err2max=get_width_max_error()/100;
		wplistconv(wplist, *stroke_data,*width_data);
		// returned widths are homogeneous position
		// let's convert it to standard position
		// as it is the default for new adv. outlines layers
		std::list<synfig::WidthPoint>::iterator iter;
		for(iter=wplist.begin(); iter!=wplist.end(); iter++)
			iter->set_position(hom_to_std(ValueBase::List(bline.begin(), bline.end()), iter->get_position(), false, false));
	}
	// print out resutls
	//synfig::info("-----------widths");
	//std::list<synfig::WidthPoint>::iterator iter;
	//for(iter=wplist.begin();iter!=wplist.end();iter++)
	//{
		//if(!iter->get_dash())
			//synfig::info("Widthpoint W=%f, P=%f", iter->get_width(), iter->get_position());
	//}
	// results end

	//Postprocess to require minimum pressure
	if(get_min_pressure_flag())
	{
		synfigapp::BLineConverter::EnforceMinWidth(bline,get_min_pressure());
		if(get_layer_advanced_outline_flag())
			synfigapp::WPListConverter::EnforceMinWidth(wplist,get_min_pressure());
	}

	// If the start and end points are similar, then make them the same point
	if (get_auto_loop_flag() &&
		bline.size() > 2 &&
		(bline.front().get_vertex() - bline.back().get_vertex()).mag() <= radius)
	{
		loop_bline_flag=true;
		Vector tangent;
		Real width(0);

		while (bline.size() > 2 &&
			   (bline.front().get_vertex() - bline.back().get_vertex()).mag() <= radius)
		{
			Real size(Real(bline.size()));
			tangent=bline.back().get_tangent1();
			width=bline.back().get_width();
			bline.pop_back();
			std::list<synfig::WidthPoint>::iterator iter;
			if(get_layer_advanced_outline_flag())
				for(iter=wplist.begin(); iter!=wplist.end(); iter++)
					iter->set_position(iter->get_position()+1/(size-1));
		}

		if(std::fabs(bline.front().get_tangent1().norm()*tangent.norm().perp())>SIMILAR_TANGENT_THRESHOLD)
		{
			// If the tangents are not similar, then
			// split the tangents
			bline.front().set_split_tangent_both(true);
			bline.front().set_tangent1(tangent);
		}
		else
		{
			// If the tangents are similar, then set the tangent
			// to the average of the two
			bline.front().set_tangent((tangent+bline.front().get_tangent1())*0.5f);
		}

		// Add the widths of the two points
		{
			Real tmp_width(bline.front().get_width()+width);
			tmp_width=tmp_width<=1?tmp_width:1;
			bline.front().set_width(tmp_width);
			if(get_layer_advanced_outline_flag())
			{
				Real width_front=wplist.front().get_width();
				Real width_back=wplist.back().get_width();
				wplist.front().set_width((width_front+width_back)/2.0);
				wplist.pop_back();
			}
		}
	}

	// If the bline only has one blinepoint, then there is nothing to do.
	if(bline.size() < 2)
	{
		// hide the 'stroke' line we were drawing, unless the user
		// explicitly requests that they are kept
		if (!getenv("SYNFIG_KEEP_ABORTED_DRAW_LINES"))
			refresh_ducks();

		return Smach::RESULT_OK;
	}

	ValueNode::Handle value_node_origin = nullptr;
	int layers_to_create = get_layer_region_flag() +
			get_layer_outline_flag() +
			get_layer_advanced_outline_flag();
	if (get_layer_link_origins_flag() && layers_to_create > 1)
		value_node_origin = ValueNode_Const::create(Vector());

	if(region_flag)
		return new_region(bline,radius,value_node_origin);

	return new_bline(bline,wplist,loop_bline_flag,radius,value_node_origin);
}

Smach::event_result
StateDraw_Context::new_bline(std::list<synfig::BLinePoint> bline,std::list<synfig::WidthPoint> wplist,bool loop_bline_flag,float radius,ValueNode::Handle &value_node_origin)
{
	synfigapp::SelectionManager::LayerList layer_list;

	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Sketch Spline"));

	bool shift_origin = false;
	Vector shift_origin_vector;
	bool join_start_no_extend=false,join_finish_no_extend=false;
	synfigapp::ValueDesc start_duck_value_desc,finish_duck_value_desc;
	bool extend_start=false,extend_finish=false,complete_loop=false;
	bool extend_start_join_same=false,extend_start_join_different=false;
	bool extend_finish_join_same=false,extend_finish_join_different=false;
	int start_duck_index = 0,finish_duck_index = 0; // initialized to keep the compiler happy; shouldn't be needed though
	ValueNode_BLine::Handle start_duck_value_node_bline=NULL,finish_duck_value_node_bline=NULL;

	// Set blend_method to static (consistent with other Layers)
	ValueBase blend_param_value(get_blend());
	blend_param_value.set_static(true);

	// Find any ducks at the start or end that we might attach to
	// (this used to only run if we didn't just draw a loop - ie. !loop_bline_flag
	// but having loops auto-connect can be useful as well)
	if(get_auto_extend_flag() || get_auto_link_flag())
	{
		etl::handle<Duck> start_duck(get_work_area()->find_duck(bline.front().get_vertex(),radius,Duck::TYPE_VERTEX));
		etl::handle<Duck> finish_duck(get_work_area()->find_duck(bline.back().get_vertex(),radius,Duck::TYPE_VERTEX));

		// check whether the start of the new line extends an
		// existing line.  this is only the case if the new
		// line isn't a self-contained loop, and if the new
		// line starts at one of the ends of an existing line
		if(start_duck)do
		{
			if(!(start_duck_value_desc=start_duck->get_value_desc()))break;
			if(loop_bline_flag)break; // loops don't extend anything
			if(!start_duck_value_desc.parent_is_value_node())break;
			start_duck_index=start_duck_value_desc.get_index(); // which point on the line did we start drawing at
			start_duck_value_node_bline=ValueNode_BLine::Handle::cast_dynamic(start_duck_value_desc.get_parent_value_node());
			if(!get_auto_extend_flag())break;

			// don't extend looped blines
			if(start_duck_value_node_bline&&!start_duck_value_node_bline->get_loop()&&
			   // did we start drawing at either end of the line?
			   (start_duck_index==0||start_duck_index==start_duck_value_node_bline->link_count()-1))
			{
				extend_start=true;
				shift_origin=true;
				shift_origin_vector=start_duck->get_origin();
			}
		}while(0);

		// check whether the end of the new line extends an
		// existing line.  this is only the case if the new
		// line isn't a self-contained loop, and if the new
		// line ends at one of the ends of an existing line
		if(finish_duck)do
		{
			if(!(finish_duck_value_desc=finish_duck->get_value_desc()))break;
			if(loop_bline_flag)break;
			if(!finish_duck_value_desc.parent_is_value_node())break;
			finish_duck_index=finish_duck_value_desc.get_index();
			finish_duck_value_node_bline=ValueNode_BLine::Handle::cast_dynamic(finish_duck_value_desc.get_parent_value_node());
			if(!get_auto_extend_flag())break;

			// don't extend looped blines
			if(finish_duck_value_node_bline&&!finish_duck_value_node_bline->get_loop()&&
			   (finish_duck_index==0||finish_duck_index==finish_duck_value_node_bline->link_count()-1))
			{
				if(extend_start)
				{
					// we've started and finished drawing at the end of a bline.  we can't
					// extend both blines, so unless we started and finished at the 2 ends
					// of the same bline, only extend the one we started on
					if(start_duck_value_node_bline==finish_duck_value_node_bline)
						complete_loop=extend_finish=true;
				}else{
					extend_finish=true;
					shift_origin=true;
					shift_origin_vector=finish_duck->get_origin();
				}
			}
		}while(0);

		// if the new line's start didn't extend an existing line,
		// check whether it needs to be linked to an existing duck
		if(!extend_start&&get_auto_link_flag()&&start_duck&&start_duck_value_desc)
		{
			synfig::Type &type(start_duck_value_desc.get_value_type());
			if (type == synfig::type_bline_point)
				start_duck_value_desc=synfigapp::ValueDesc(LinkableValueNode::Handle::cast_dynamic(start_duck_value_desc.get_value_node()),0);
			if (type == synfig::type_bline_point || type == synfig::type_vector)
			{
				if (!shift_origin || shift_origin_vector == start_duck->get_origin())
				{
					shift_origin = true;
					shift_origin_vector = start_duck->get_origin();
					get_canvas_interface()->auto_export(start_duck_value_desc);
					if (extend_finish)
						if(start_duck_value_node_bline&&start_duck_value_node_bline==finish_duck_value_node_bline)
							extend_finish_join_same=true;
						else
							extend_finish_join_different=true;
					else
						join_start_no_extend=true;
				}
			}
		}
		// if the new line's end didn't extend an existing line,
		// check whether it needs to be linked to an existing duck
		if(!extend_finish&&get_auto_link_flag()&&finish_duck&&finish_duck_value_desc)
		{
			synfig::Type &type(finish_duck_value_desc.get_value_type());
			if (type == synfig::type_bline_point)
				finish_duck_value_desc=synfigapp::ValueDesc(LinkableValueNode::Handle::cast_dynamic(finish_duck_value_desc.get_value_node()),0);
			if (type == synfig::type_bline_point || type == synfig::type_vector)
			{
				if (!shift_origin || shift_origin_vector == finish_duck->get_origin())
				{
					shift_origin = true;
					shift_origin_vector = finish_duck->get_origin();
					get_canvas_interface()->auto_export(finish_duck_value_desc);
					if(extend_start)
						if(finish_duck_value_node_bline&&start_duck_value_node_bline==finish_duck_value_node_bline)
							extend_start_join_same=true;
						else
							extend_start_join_different=true;
					else
						join_finish_no_extend=true;
				}
			}
		}
	}

	ValueNode_BLine::Handle value_node;
	std::list<synfig::BLinePoint> trans_bline;

	{
		std::list<synfig::BLinePoint>::iterator iter;
		const synfig::TransformStack& transform(get_work_area()->get_curr_transform_stack());

		for(iter=bline.begin();iter!=bline.end();++iter)
		{
			BLinePoint bline_point(*iter);
			Point new_vertex(transform.unperform(bline_point.get_vertex()));

			bline_point.set_tangent1(
				transform.unperform(
					bline_point.get_tangent1()+bline_point.get_vertex()
				) -new_vertex
			);

			bline_point.set_tangent2(
				transform.unperform(
					bline_point.get_tangent2()+bline_point.get_vertex()
				) -new_vertex
			);

			if (shift_origin)
				new_vertex=new_vertex-shift_origin_vector;

			bline_point.set_vertex(new_vertex);

			trans_bline.push_back(bline_point);
		}
		value_node=ValueNode_BLine::create(synfig::ValueBase(synfig::ValueBase::List(trans_bline.begin(), trans_bline.end()),loop_bline_flag));

		Canvas::Handle canvas(get_canvas_view()->get_canvas());
		Layer::Handle layer(get_canvas_view()->get_selection_manager()->get_selected_layer());
		if (layer) canvas=layer->get_canvas();
		value_node->set_member_canvas(canvas);
	}

	Smach::event_result result;
	synfig::ValueNode_DynamicList::ListEntry source;

	// the new line's start extends an existing line
	if(extend_start)
	{
		int target_offset = 0;
		if(complete_loop)trans_bline.pop_back();
		trans_bline.pop_front();
		if(start_duck_index==0)
		{	// We need to reverse the BLine first.
			reverse_bline(trans_bline);
			reverse_wplist(wplist);
			result=extend_bline_from_begin(start_duck_value_node_bline,trans_bline,wplist,complete_loop);
			source=start_duck_value_node_bline->list.front();
			target_offset=trans_bline.size();
		}
		else
		{
			result=extend_bline_from_end(start_duck_value_node_bline,trans_bline,wplist,complete_loop);
			source=start_duck_value_node_bline->list.back();
		}

		if(extend_start_join_different)
			LinkableValueNode::Handle::cast_dynamic(source.value_node)->
				set_link("point",finish_duck_value_desc.get_value_node());
		else if(extend_start_join_same)
			LinkableValueNode::Handle::cast_dynamic(source.value_node)->
				set_link("point",synfigapp::ValueDesc(LinkableValueNode::Handle::cast_dynamic(start_duck_value_node_bline->
													list[target_offset+finish_duck_index].value_node),0).get_value_node());
		return result;
	}

	// the new line's end extends an existing line
	if(extend_finish)
	{
		int target_offset = 0;
		trans_bline.pop_back();
		if(finish_duck_index==0)
		{
			result=extend_bline_from_begin(finish_duck_value_node_bline,trans_bline, wplist,false);
			source=finish_duck_value_node_bline->list.front();
			target_offset=trans_bline.size();
		}
		else
		{	// We need to reverse the BLine first.
			reverse_bline(trans_bline);
			reverse_wplist(wplist);
			result=extend_bline_from_end(finish_duck_value_node_bline,trans_bline, wplist,false);
			source=finish_duck_value_node_bline->list.back();
		}

		if(extend_finish_join_different)
			LinkableValueNode::Handle::cast_dynamic(source.value_node)->
				set_link("point",start_duck_value_desc.get_value_node());
		else if(extend_finish_join_same)
			LinkableValueNode::Handle::cast_dynamic(source.value_node)->
				set_link("point",synfigapp::ValueDesc(LinkableValueNode::Handle::cast_dynamic(finish_duck_value_node_bline->
													list[target_offset+start_duck_index].value_node),0).get_value_node());
		return result;
	}

	if (join_start_no_extend)
		LinkableValueNode::Handle::cast_dynamic(value_node->list.front().value_node)->
		  set_link("point",start_duck_value_desc.get_value_node());

	if (join_finish_no_extend)
		LinkableValueNode::Handle::cast_dynamic(value_node->list.back().value_node)->
		  set_link("point",finish_duck_value_desc.get_value_node());

	if(get_auto_export_flag())
		if (!get_canvas_interface()->add_value_node(value_node,get_id()))
		{
			/* it's no big deal, is it?  let's keep the shape anyway */
			// get_canvas_view()->get_ui_interface()->error(_("Unable to add value node"));
			// group.cancel();
			// increment_id();
			// return Smach::RESULT_ERROR;
		}

	last_stroke=value_node;
	last_stroke_id=get_id();
	last_value_node_origin = value_node_origin;

	{
		// Create the layer(s)
		Layer::Handle layer, layer2, somelayer;
		Canvas::Handle canvas(get_canvas_view()->get_canvas());
		int depth(0);

		// we are temporarily using the layer to hold something
		somelayer=get_canvas_view()->get_selection_manager()->get_selected_layer();
		if(somelayer)
		{
			depth=somelayer->get_depth();
			canvas=somelayer->get_canvas();
		}

		// fill_last_stroke() will take care of clearing the selection if we're calling it
		if((get_layer_outline_flag() || get_layer_advanced_outline_flag()) && get_layer_region_flag())
		{
			if (fill_last_stroke_and_unselect_other_layers() == Smach::RESULT_ERROR)
			{
				get_canvas_view()->get_selection_manager()->set_selected_layers(layer_list);
				get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
				group.cancel();
				return Smach::RESULT_ERROR;
			}
		}
		else
			get_canvas_interface()->get_selection_manager()->clear_selected_layers();

		//int number(synfig::UniqueID().get_uid());

		synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

		// if they're both defined, we'll add the region later
		if(get_layer_outline_flag() || get_layer_advanced_outline_flag())
		{
			if(get_layer_outline_flag())
			{
				layer=get_canvas_interface()->add_layer_to("outline",canvas,depth);
				if (!layer)
				{
					get_canvas_view()->get_selection_manager()->set_selected_layers(layer_list);
					get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
					group.cancel();
					return Smach::RESULT_ERROR;
				}
				layer->set_description(get_id()+_(" Outline"));

				layer->set_param("blend_method",blend_param_value);
				get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

				layer->set_param("amount",get_opacity());
				get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

				layer->set_param("width",get_bline_width());
				get_canvas_interface()->signal_layer_param_changed()(layer,"width");

				layer->set_param("round_tip[0]", get_round_ends_flag());
				get_canvas_interface()->signal_layer_param_changed()(layer, "round_tip[0]");

				layer->set_param("round_tip[1]", get_round_ends_flag());
				get_canvas_interface()->signal_layer_param_changed()(layer, "round_tip[1]");
			}
			if(get_layer_advanced_outline_flag())
			{
				layer2=get_canvas_interface()->add_layer_to("advanced_outline",canvas,depth);
				if (!layer2)
				{
					get_canvas_view()->get_selection_manager()->set_selected_layers(layer_list);
					get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
					group.cancel();
					return Smach::RESULT_ERROR;
				}
				layer2->set_description(get_id()+_(" Advanced Outline"));

				layer2->set_param("blend_method",blend_param_value);
				get_canvas_interface()->signal_layer_param_changed()(layer2,"blend_method");

				layer2->set_param("amount",get_opacity());
				get_canvas_interface()->signal_layer_param_changed()(layer2,"amount");

				layer2->set_param("width",get_bline_width());
				get_canvas_interface()->signal_layer_param_changed()(layer2,"width");

				// advanced outline tip types: 1, rounded 2, squared 3, peak 4, flat
				if(get_round_ends_flag())
				{
					layer2->set_param((ValueBase(),"start_tip"), 1);
					get_canvas_interface()->signal_layer_param_changed()(layer2, (ValueBase(),"start_tip"));

					layer2->set_param((ValueBase(),"end_tip"), 1);
					get_canvas_interface()->signal_layer_param_changed()(layer2, (ValueBase(), "end_tip"));
				}
				else
				{
					layer2->set_param((ValueBase(),"start_tip"), 4);
					get_canvas_interface()->signal_layer_param_changed()(layer2, (ValueBase(), "start_tip"));

					layer2->set_param((ValueBase(),"end_tip"), 4);
					get_canvas_interface()->signal_layer_param_changed()(layer2, (ValueBase(), "end_tip"));
				}
			}
		}
		else if(get_layer_region_flag())
		{
			layer=get_canvas_interface()->add_layer_to("region",canvas,depth);
			if (!layer)
			{
				get_canvas_view()->get_selection_manager()->set_selected_layers(layer_list);
				get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
				group.cancel();
				return Smach::RESULT_ERROR;
			}
			layer->set_description(get_id()+_(" Region"));

			layer->set_param("blend_method",blend_param_value);
			get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

			layer->set_param("amount",get_opacity());
			get_canvas_interface()->signal_layer_param_changed()(layer,"amount");
		}

		if(get_feather_size())
		{
			if(layer)
			{
				layer->set_param("feather",get_feather_size());
				get_canvas_interface()->signal_layer_param_changed()(layer,"feather");
			}
			if(get_layer_advanced_outline_flag())
			{
				layer2->set_param("feather",get_feather_size());
				get_canvas_interface()->signal_layer_param_changed()(layer2,"feather");
			}

		}
		if(get_layer_outline_flag()) assert(layer);
		if(get_layer_advanced_outline_flag()) assert(layer2);
		//layer->set_description(strprintf("Stroke %d",number));
		//get_canvas_interface()->signal_layer_new_description()(layer,layer->get_description());

		if (shift_origin)
		{
			if(layer)
				get_canvas_interface()->change_value(synfigapp::ValueDesc(layer,"origin"),shift_origin_vector);
			if(layer2)
				get_canvas_interface()->change_value(synfigapp::ValueDesc(layer2,"origin"),shift_origin_vector);
		}
			// Regular Outline or Region
		if(layer)
		{
			synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
			assert(action);
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			action->set_param("layer",layer);
			if(!action->set_param("param",String("bline")))
				synfig::error("LayerParamConnect didn't like \"param\"");
			if(!action->set_param("value_node",ValueNode::Handle(value_node)))
				synfig::error("LayerParamConnect didn't like \"value_node\"");

			if(!get_canvas_interface()->get_instance()->perform_action(action))
			{
				get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
				group.cancel();
				increment_id();
				//refresh_ducks();
				return Smach::RESULT_ERROR;
			}
			layer_list.push_back(layer);

			// only link the outline's origin parameter if the option is selected and we're creating more than one layer
			if (value_node_origin)
			{
				bool linked = link_layer_origin(layer, value_node_origin);
				if (!linked)
				{
					group.cancel();
					get_canvas_view()->get_ui_interface()->error(_("Unable to create Outline layer"));
					return Smach::RESULT_ERROR;
				}
			}
		}
		// Advanced Outline
		if(layer2)
		{
			synfigapp::Action::Handle action2(synfigapp::Action::create("LayerParamConnect"));
			assert(action2);
			ValueNode_WPList::Handle value_node_wplist;
			value_node_wplist=ValueNode_WPList::create(synfig::ValueBase(synfig::ValueBase::List(wplist.begin(), wplist.end())));
			if(value_node_wplist) value_node_wplist->set_member_canvas(get_canvas());
			action2->set_param("canvas",get_canvas());
			action2->set_param("canvas_interface",get_canvas_interface());
			action2->set_param("layer", layer2);
			action2->set_param("param", String("wplist"));
			action2->set_param("value_node", ValueNode::Handle(value_node_wplist));
			if(!get_canvas_interface()->get_instance()->perform_action(action2))
			{
				get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
				group.cancel();
				increment_id();
				//refresh_ducks();
				return Smach::RESULT_ERROR;
			}
			synfigapp::Action::Handle action3(synfigapp::Action::create("LayerParamConnect"));
			assert(action3);
			action3->set_param("canvas",get_canvas());
			action3->set_param("canvas_interface",get_canvas_interface());
			action3->set_param("layer", layer2);
			if(!action3->set_param("param",String("bline")))
				synfig::error("LayerParamConnect didn't like \"param\"");
			if(!action3->set_param("value_node",ValueNode::Handle(value_node)))
				synfig::error("LayerParamConnect didn't like \"value_node\"");
			if(!get_canvas_interface()->get_instance()->perform_action(action3))
			{
				get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
				group.cancel();
				increment_id();
				//refresh_ducks();
				return Smach::RESULT_ERROR;
			}
			layer_list.push_back(layer2);

			// only link the advanced outline's origin parameter if the option is selected and we're creating more than one layer
			if (value_node_origin)
			{
				bool linked = link_layer_origin(layer2, value_node_origin);
				if (!linked)
				{
					group.cancel();
					get_canvas_view()->get_ui_interface()->error(_("Unable to create Advanced Outline layer"));
					return Smach::RESULT_ERROR;
				}
			}
		}
		get_canvas_view()->get_selection_manager()->set_selected_layers(layer_list);
	}
	increment_id();
	return Smach::RESULT_ACCEPT;
}

#ifdef _DEBUG
static void
debug_show_vertex_list(int iteration, std::list<synfigapp::ValueDesc>& vertex_list,
					   std::string title, int current)
{
	std::list<synfigapp::ValueDesc>::iterator i = vertex_list.begin();
	printf("\n%s\n  ----- iter %d : ", title.c_str(), iteration);
	int c = 0;
	synfig::LinkableValueNode::Handle last = 0;
	int start = -1;
	int index;
	int prev;
	int dir = 0;
	bool started = false;
	for(;i!=vertex_list.end();i++,c++)
	{
		synfigapp::ValueDesc value_desc(*i);

		if (value_desc.parent_is_value_node()) {
			if(value_desc.parent_is_linkable_value_node())
			{
				index = value_desc.get_index();
				// printf("<%d>", index);
				if (last == synfig::LinkableValueNode::Handle::cast_reinterpret(value_desc.get_parent_value_node()))
				{
					// printf("\n%s:%d\n", __FILE__, __LINE__);
					if (start != -1)
					{
						// printf("\n%s:%d\n", __FILE__, __LINE__);
						if (c == current)
						{
							// printf("\n%s:%d\n", __FILE__, __LINE__);
							if (dir)
							{
								if (started) printf(", "); else started = true;
								printf("%d--%d", start, prev);
							}
							else
							{
								if (started) printf(", "); else started = true;
								printf("%d", start);
							}
							printf(", *%d*", index);
							start = -1;
						}
						else if (dir == 0)
						{
							// printf("\n%s:%d\n", __FILE__, __LINE__);
							if (index == start + 1)
							{
								// printf("\n%s:%d\n", __FILE__, __LINE__);
								dir = 1;
								prev = index;
							}
							else if (index == start - 1)
							{
								// printf("\n%s:%d\n", __FILE__, __LINE__);
								dir = -1;
								prev = index;
							}
							else
							{
								if (started) printf(", "); else started = true;
								printf("%d", start);
								start = index;
							}
						}
						else if (index == prev + dir)
						{
							// printf("\n%s:%d\n", __FILE__, __LINE__);
							prev = index;
						}
						else
						{
							// printf("\n%s:%d\n", __FILE__, __LINE__);
							if (started) printf(", "); else started = true;
							if (prev != start)
								printf("%d--%d", start, prev);
							else
								printf("%d", start);
							// printf("\n%s:%d\n", __FILE__, __LINE__);
							start = index;
							dir = 0;
						}
					}
					else
					{
						// printf("\n%s:%d\n", __FILE__, __LINE__);
						if (c == current)
						{
							if (started) printf(", "); else started = true;
							printf("*%d*", index);
						}
						else
						{
							// printf("\n%s:%d\n", __FILE__, __LINE__);
							start = index;
							dir = 0;
						}
					}
				}
				else
				{
					// printf("\n%s:%d\n", __FILE__, __LINE__);
					if (last)
					{
						// printf("\n%s:%d\n", __FILE__, __LINE__);
						if (start != -1)
						{
							if (started) printf(", "); else started = true;
							if (dir != 0)
								printf("%d--%d", start, prev);
							else
								printf("%d", start);
						}
						// printf("\n%s:%d\n", __FILE__, __LINE__);
						printf(") ");
					}
					// printf("\n%s:%d\n", __FILE__, __LINE__);
					last = synfig::LinkableValueNode::Handle::cast_reinterpret(value_desc.get_parent_value_node());
					printf("%d:(", synfig::LinkableValueNode::Handle::cast_reinterpret(value_desc.get_parent_value_node())->link_count());
					started = false;
					// printf("\n%s:%d\n", __FILE__, __LINE__);
					if (c == current)
					{
						start = -1;
						printf("*%d*", index);
					}
					else
					{
						// printf("\n%s:%d\n", __FILE__, __LINE__);
						start = index;
						dir = 0;
					}
					// printf("\n%s:%d\n", __FILE__, __LINE__);
				}
				// printf("\n%s:%d\n", __FILE__, __LINE__);
			}
			else if (last)
				if (last) printf("?!) ");
		}
		else
		{
			last = 0;
			printf("? ");
		}
	}
	if (last)
	{
		if (started) printf(", "); else started = true;
		if (start != -1)
		{
			if (dir != 0)
				printf("%d--%d", start, prev);
			else
				printf("%d", start);
		}
		printf(")");
	}
	printf("\n");
}
#else  // _DEBUG
#define debug_show_vertex_list(a,b,c,d)
#endif	// _DEBUG

Smach::event_result
StateDraw_Context::new_region(std::list<synfig::BLinePoint> bline, synfig::Real radius, ValueNode::Handle &value_node_origin)
{
	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Define Region"));

	std::list<synfigapp::ValueDesc> vertex_list;

	printf("new_region with %zd bline points\n", bline.size());

	// First we need to come up with a rough list of
	// BLinePoints that we are going to be using to
	// define our region.
	{
		std::list<synfig::BLinePoint>::iterator iter;
		for(iter=bline.begin();iter!=bline.end();++iter)
		{
			etl::handle<Duck> duck(get_work_area()->find_duck(iter->get_vertex(),0,Duck::TYPE_VERTEX));

			if(!duck)
			{
				synfig::info(__FILE__":%d: Nothing to enclose!",__LINE__);
				return Smach::RESULT_OK;
			}

			assert(duck->get_type()==Duck::TYPE_VERTEX);

			synfigapp::ValueDesc value_desc(duck->get_value_desc());

			if(!value_desc)
			{
				synfig::info(__FILE__":%d: Got a hit, but no ValueDesc on this duck",__LINE__);
				continue;
			}

			if (value_desc.get_value_type() == synfig::type_bline_point)
			{
				//if(vertex_list.empty() || value_desc!=vertex_list.back())
				vertex_list.push_back(value_desc);
				assert(vertex_list.back().is_valid());
			}
		}
	}

	assert(vertex_list.back().is_valid());

	printf("vertex list with %zd bline points\n", vertex_list.size());

	// Remove any duplicates
	{
	}

	ValueNode_BLine::Handle value_node_bline;

	// Now we need to test for the trivial case,
	// which is where all of the vertices
	// come from one BLine.
	if(vertex_list.front().parent_is_linkable_value_node())
	{
		bool trivial_case(true);
		ValueNode::Handle trivial_case_value_node;

		trivial_case_value_node=vertex_list.front().get_parent_value_node();

		std::list<synfigapp::ValueDesc>::iterator iter;
		for(iter=vertex_list.begin();iter!=vertex_list.end();++iter)
		{
			if(trivial_case_value_node!=iter->get_parent_value_node())
			{
				trivial_case=false;
				break;
			}
		}

		// \todo - re-enable this code
		if(trivial_case && false)
		{
			synfig::info("all points are on the same bline, so just fill that line");
			value_node_bline=ValueNode_BLine::Handle::cast_dynamic(trivial_case_value_node);

			synfig::info("the line has %d vertices", value_node_bline->link_count());

			if(value_node_bline->link_count() <= 2)
			{
				synfig::info(__FILE__":%d: Vertex list too small to make region.",__LINE__);
				return Smach::RESULT_OK;
			}
		}
	}

	if(!value_node_bline && vertex_list.size() <= 2)
	{
		synfig::info(__FILE__":%d: Vertex list too small to make region.",__LINE__);
		return Smach::RESULT_OK;
	}

	// Now we need to clean the list of vertices up
	// a bit. This includes inserting missing vertices
	// and removing extraneous ones.
	// We can do this in multiple passes.
	if(!value_node_bline)
	{
		debug_show_vertex_list(0, vertex_list, "before shifting stuff", -1);
		// rearrange the list so that the first and last node are on different blines
		std::list<synfigapp::ValueDesc>::iterator iter, start;
		ValueNode::Handle last_value_node = vertex_list.back().get_parent_value_node();
		for(iter = vertex_list.begin(); iter!=vertex_list.end(); iter++)
			if (iter->get_parent_value_node() != last_value_node)
			{
				vertex_list.insert(vertex_list.end(), vertex_list.begin(), iter);
				vertex_list.erase(vertex_list.begin(), iter);
				break;
			}

		debug_show_vertex_list(0, vertex_list, "before detecting direction and limits", -1);
		// rearrange the list so that the first and last node are on different blines
		iter = vertex_list.begin();
		while (iter!=vertex_list.end())
		{
			// make a note of which bline we're looking at
			ValueNode::Handle parent_value_node = iter->get_parent_value_node();
			start = iter;
			int points_in_line = synfig::LinkableValueNode::Handle::cast_reinterpret(parent_value_node)->link_count();
			bool looped = (*parent_value_node)(get_time()).get_loop();
			int this_index, last_index = iter->get_index();
			int min_index = last_index, max_index = last_index;
			bool whole;
			int direction = 0;

			// printf("there are %d points in this line - first is index %d\n", points_in_line, last_index);

			// while we're looking at the same bline, keep going
			iter++;
			while (iter != vertex_list.end() && iter->get_parent_value_node() == parent_value_node)
			{
				this_index = iter->get_index();
				// printf("index went from %d to %d\n", last_index, this_index);
				if (looped)
				{
					if (this_index - last_index > points_in_line/2)
						while (this_index - last_index > points_in_line/2)
							this_index -= points_in_line;
					else if (last_index - this_index > points_in_line/2)
						while (last_index - this_index > points_in_line/2)
							this_index += points_in_line;
				}

				if (this_index < min_index) min_index = this_index;
				if (this_index > max_index) max_index = this_index;

				// printf("so let's imagine index went from %d to %d\n", last_index, this_index);
				if (this_index > last_index)
					direction++;
				else if (this_index < last_index)
					direction--;

				last_index = this_index;
				iter++;
			}

			// printf("min %d and max %d\n", min_index, max_index);
			whole = max_index - min_index >= points_in_line;
			min_index = (min_index % points_in_line + points_in_line) % points_in_line;
			max_index = (max_index % points_in_line + points_in_line) % points_in_line;
			// they drew around a shape more than once - what's the start/end point?  does it matter?
			if (whole) min_index = max_index = (min_index + max_index) / 2;
			// printf("processed min %d max %d whole %d\n", min_index, max_index, whole);

			LinkableValueNode::Handle linkable_parent = LinkableValueNode::Handle::cast_dynamic(parent_value_node);
			assert(linkable_parent);

			if (direction < 0)
			{
				if (whole)
				{
					// printf("whole (down) (%d) ", min_index);
					for (int i = min_index; i >= 0; i--)
					{
						// printf("%d ", i);
						vertex_list.insert(start, synfigapp::ValueDesc(linkable_parent, i));
					}
					for (int i = points_in_line - 1; i >= min_index; i--)
					{
						// printf("%d ", i);
						vertex_list.insert(start, synfigapp::ValueDesc(linkable_parent, i));
					}
				}
				else
				{
					// printf("part (down) (%d -> %d) ", max_index, min_index);
					for (int i = max_index; i != min_index; i--)
					{
						if (i == -1) i = points_in_line - 1;
						// printf("%d ", i);
						vertex_list.insert(start, synfigapp::ValueDesc(linkable_parent, i));
					}
					vertex_list.insert(start, synfigapp::ValueDesc(linkable_parent, min_index));
				}
			}
			else
			{
				if (whole)
				{
					// printf("whole (%d) ", min_index);
					for (int i = min_index; i < points_in_line; i++)
					{
						// printf("%d ", i);
						vertex_list.insert(start, synfigapp::ValueDesc(linkable_parent, i));
					}
					for (int i = 0; i <= min_index; i++)
					{
						// printf("%d ", i);
						vertex_list.insert(start, synfigapp::ValueDesc(linkable_parent, i));
					}
				}
				else
				{
					// printf("part (%d -> %d) ", min_index, max_index);
					for (int i = min_index; i != max_index; i++)
					{
						if (i == points_in_line) i = 0;
						// printf("%d ", i);
						vertex_list.insert(start, synfigapp::ValueDesc(linkable_parent, i));
					}
					vertex_list.insert(start, synfigapp::ValueDesc(linkable_parent, max_index));
				}
			}
			// printf("\n");
			// debug_show_vertex_list(0, vertex_list, "after insert", -1);
			vertex_list.erase(start, iter);
			// debug_show_vertex_list(0, vertex_list, "after delete", -1);
		}

		debug_show_vertex_list(0, vertex_list, "continuous vertices", -1);

		// \todo re-enable or delete this section
		int i=100;
		for(bool done=false;!done && i<30;i++)
		{
			debug_show_vertex_list(i, vertex_list, "in big loop", -1);

			// Set done to "true" for now. If
			// any updates are performed, we will
			// change it back to false.
			done=true;

			std::list<synfigapp::ValueDesc>::iterator prev,next;
			prev=vertex_list.end();prev--;	// Set prev to the last ValueDesc
			next=vertex_list.begin();
			iter=next++; // Set iter to the first value desc, and next to the second

			int current = 0;
			for(;iter!=vertex_list.end();prev=iter,iter++,next++,current++)
			{
				// we need to be able to erase(next) and can't do that if next is end()
				if (next == vertex_list.end()) next = vertex_list.begin();
				debug_show_vertex_list(i, vertex_list, "in loop around vertices", current);
				synfigapp::ValueDesc value_prev(*prev);
				synfigapp::ValueDesc value_desc(*iter);
				synfigapp::ValueDesc value_next(*next);

				assert(value_desc.is_valid());
				assert(value_next.is_valid());
				assert(value_prev.is_valid());

				// synfig::info("-------");
				// synfig::info(__FILE__":%d: value_prev 0x%08X:%d",__LINE__,value_prev.get_parent_value_node().get(),value_prev.get_index());
				// synfig::info(__FILE__":%d: value_desc 0x%08X:%d",__LINE__,value_desc.get_parent_value_node().get(),value_desc.get_index());
				// synfig::info(__FILE__":%d: value_next 0x%08X:%d",__LINE__,value_next.get_parent_value_node().get(),value_next.get_index());

				/*
				  if(value_prev.parent_is_value_node() && value_desc.parent_is_value_node() && value_next.parent_is_value_node())
				  {
				  // Remove random extraneous vertices
				  if(value_prev.get_parent_value_node()==value_next.get_parent_value_node() &&
				  value_prev.get_parent_value_node()!=value_desc.get_parent_value_node())
				  {
				  vertex_list.erase(iter);
				  done=false;
				  break;
				  }
				  }
				*/

				// // Remove duplicate vertices

				// // if previous is the same as current or
				// //    current is the same as next, remove current
				// if(value_prev.get_value_node()==value_desc.get_value_node() ||
				//    value_desc.get_value_node()==value_next.get_value_node())
				// {
				//	vertex_list.erase(iter);
				//	done=false;
				//	printf("erased node - i = %d\n", i);
				//	break;
				// }

				// // if previous is the same as next, remove previous?  or next?
				// if(value_prev.get_value_node()==value_next.get_value_node())
				// {
				// 	vertex_list.erase(next);
				// 	// vertex_list.erase(prev);
				// 	done=false;
				// 	printf("erased node - i = %d\n", i);
				// 	break;
				// }

				// if 'this' and 'next' both have parents
				if (value_desc.parent_is_value_node() && value_next.parent_is_value_node())
				{
					// if they are both on the same bline - this has been handled by new code above
					if (value_desc.get_parent_value_node() == value_next.get_parent_value_node())
					{
						// // if (next != vertex_list.end())
						// {
						// 	printf("parent loop is %d and node loop is ??\n",
						// 		   (*(value_desc.get_parent_value_node()))(get_time()).get_loop()
						// 		   // value_desc.get_value_node().get_loop(),
						// 		);
						//
						// 	// Fill in missing vertices
						// 	// \todo take loops into account: seeing (15, 2, 3, 4) probably means that (0, 1) is missing, not 14 through 3
						// 	if(value_desc.get_index()<value_next.get_index()-1)
						// 	{
						// 		debug_show_vertex_list(i, vertex_list,
						// 							   strprintf("same parent, different points this %d < next-1 %d",
						// 										 value_desc.get_index(), ((value_next.get_index()-1))),
						// 							   current);
						// 		for (int index = value_desc.get_index()+1; index < value_next.get_index(); index++)
						// 		{
						// 			printf("inserting up %d\n", index);
						// 			vertex_list.insert(next, synfigapp::ValueDesc(value_desc.get_parent_value_node(), index));
						// 		}
						// 		debug_show_vertex_list(i, vertex_list, "new list", current);
						// 		done=false;
						// 		break;
						// 	}
						// 	if(value_next.get_index()<value_desc.get_index()-1)
						// 	{
						// 		debug_show_vertex_list(i, vertex_list,
						// 							   strprintf("same parent, different points next %d < this-1 %d",
						// 										 value_next.get_index(), ((value_desc.get_index()-1))),
						// 							   current);
						// 		for (int index = value_desc.get_index()-1; index > value_next.get_index(); index--)
						// 		{
						// 			printf("inserting down %d\n", index);
						// 			vertex_list.insert(next, synfigapp::ValueDesc(value_desc.get_parent_value_node(), index));
						// 		}
						// 		debug_show_vertex_list(i, vertex_list, "new list", current);
						// 		done=false;
						// 		break;
						// 	}
						// }
					}
					// 'this' and 'next' have different parents
					else
					{
						ValueNode::Handle v1 = value_desc.get_value_node();
						ValueNode::Handle v2 = value_desc.get_parent_value_node();
						if (v1 == v2)
							printf("same\n");
						else
							printf("different\n");

						if (value_desc.get_value_node() != value_next.get_value_node())
						{
							// Ensure that connections between blines are properly connected
							BLinePoint vertex(value_desc.get_value(get_time()).get(BLinePoint()));
							BLinePoint vertex_next(value_next.get_value(get_time()).get(BLinePoint()));

							//synfig::info("--------");
							//synfig::info(__FILE__":%d: vertex: [%f, %f]",__LINE__,vertex.get_vertex()[0],vertex.get_vertex()[1]);
							//synfig::info(__FILE__":%d: vertex_next: [%f, %f]",__LINE__,vertex_next.get_vertex()[0],vertex_next.get_vertex()[1]);

							// if this vertex is close to the next one, replace this vertex with a new one
							// and erase the next one
							printf("this point is %5.2f from the next point - compare with %5.2f\n",
								   (vertex.get_vertex()-vertex_next.get_vertex()).mag_squared(),
								   radius*radius);
							if((vertex.get_vertex()-vertex_next.get_vertex()).mag_squared()<radius*radius)
							{
								printf("in one - it's close\n");
								ValueNode_Composite::Handle value_node;
								ValueNode_Composite::Handle value_node_next;
								value_node=ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()->
																					 clone(value_desc.get_value_node()->get_parent_canvas()));
								value_node_next=ValueNode_Composite::Handle::cast_dynamic(value_next.get_value_node()->
																						  clone(value_next.get_value_node()->get_parent_canvas()));
								if(!value_node || !value_node_next)
								{
									synfig::info(__FILE__":%d: Unable to properly connect blines.",__LINE__);
									continue;
								}
								// \todo if next isn't split, don't we want to copy its 'Tangent 1' instead?
								value_node->set_link("t2",value_node_next->get_link("t2"));
								value_node->set_link("split",ValueNode_Const::create(true));

								// get_canvas_interface()->auto_export(value_node);
								printf("exporting\n");
								get_canvas_interface()->add_value_node(value_node,value_node->get_id() + strprintf("foo %d", rand()));

								assert(value_node->is_exported());
								// replace 'this' with the new valuenode
								*iter=synfigapp::ValueDesc(get_canvas(),value_node->get_id());
								printf("erasing next\n");
								printf("erasing next point\n");
								vertex_list.erase(next);
								done=false;
								break;
							} // this vertex isn't close to the next one
							else if (value_prev.parent_is_value_node())
							{
								printf("in two - it's far\n");
								// \todo this only makes sense if prev is on the same bline
								printf("this is index %d\n", value_desc.get_index());
								printf("prev is index %d\n", value_prev.get_index());
								bool positive_trend(value_desc.get_index()>value_prev.get_index());

								if(positive_trend)
								{
									printf("positive trend\n");
									printf("comparing index %d < link_count()-1 = %d-1 = %d\n",
										   value_desc.get_index(),
										   LinkableValueNode::Handle::cast_static(value_desc.get_parent_value_node())->link_count(),
										   LinkableValueNode::Handle::cast_static(value_desc.get_parent_value_node())->link_count()-1);
									if (value_desc.get_index()<LinkableValueNode::Handle::cast_static(value_desc.get_parent_value_node())->link_count()-1)
									{
										printf("in two - b\n");
										printf("inserting node with index %d\n", value_desc.get_index()+1);
										vertex_list.insert(next,
														   synfigapp::ValueDesc(LinkableValueNode::Handle::cast_dynamic(value_desc.get_parent_value_node()),
																				value_desc.get_index()+1));
										done=false;
										break;
									}
								}
								else // !positive_trend
								{
									printf("negative trend\n");
									if(value_desc.get_index()>0)
									{
										printf("in two - a\n");
										printf("inserting node on this line with index %d\n",
											   value_desc.get_index()-1);
										vertex_list.insert(next,
														   synfigapp::ValueDesc(LinkableValueNode::Handle::cast_dynamic(value_desc.get_parent_value_node()),
																				value_desc.get_index()-1));
										done=false;
										break;
									}
								}
							}
						}
					}
				}
			}
		}

		if(vertex_list.size()<=2)
		{
			synfig::info(__FILE__":%d: Vertex list too small to make region.",__LINE__);
			return Smach::RESULT_OK;
		}

		debug_show_vertex_list(i, vertex_list, "finished tidying list", -1);
	}

	// If we aren't the trivial case,
	// then go ahead and create the new
	// BLine value node
	if(!value_node_bline)
	{
		synfig::info("not all points are on the same bline");
		value_node_bline=ValueNode_BLine::create();

		std::list<synfigapp::ValueDesc>::iterator iter;
		for(iter=vertex_list.begin();iter!=vertex_list.end();++iter)
		{
			// Ensure that the vertex is exported.
			get_canvas_interface()->auto_export(*iter);

			value_node_bline->add(iter->get_value_node());
			//value_node_bline->add(ValueNode_BLine::ListEntry(iter->get_value_node()));
		}

		value_node_bline->set_loop(true);
	}

	get_canvas_interface()->auto_export(value_node_bline);

	// Now we create the region layer
	// Create the layer
	{
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

		synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

		layer=get_canvas_interface()->add_layer_to("region",canvas,depth);
		if (!layer)
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create layer"));
			group.cancel();
			return Smach::RESULT_ERROR;
		}

		if(get_feather_size())
		{
			layer->set_param("feather",get_feather_size());
			get_canvas_interface()->signal_layer_param_changed()(layer,"feather");
		}
		get_canvas_interface()->signal_layer_param_changed()(layer,"color");

		synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));

		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("layer",layer);
		if(!action->set_param("param",String("bline")))
			synfig::error("LayerParamConnect didn't like \"param\"");
		if(!action->set_param("value_node",ValueNode::Handle(value_node_bline)))
			synfig::error("LayerParamConnect didn't like \"value_node\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to create Region layer"));
			group.cancel();
			return Smach::RESULT_ERROR;
		}

		// only link the region's origin parameter if the option is selected and we're creating more than one layer
		if (value_node_origin)
		{
			bool linked = link_layer_origin(layer, value_node_origin);
			if (!linked) {
				group.cancel();
				throw String(_("Unable to create Region layer"));
				return Smach::RESULT_ERROR;
			}
		}

		get_canvas_view()->get_selection_manager()->set_selected_layer(layer);
	}

	return Smach::RESULT_ACCEPT;
}

void
StateDraw_Context::refresh_ducks()
{
	get_canvas_view()->queue_rebuild_ducks();
/*
	get_work_area()->clear_ducks();


	std::list< etl::smart_ptr<std::list<synfig::Point> > >::iterator iter;

	for(iter=stroke_list.begin();iter!=stroke_list.end();++iter)
	{
		get_work_area()->add_stroke(*iter);
	}

	get_work_area()->queue_draw();
*/
}


Smach::event_result
StateDraw_Context::extend_bline_from_begin(ValueNode_BLine::Handle value_node,std::list<synfig::BLinePoint> bline,std::list<synfig::WidthPoint> wplist,bool complete_loop)
{

	// Recreate the bline that is going to be inserted
	// First copy the list of BlinePoints
	std::list<synfig::BLinePoint> inserted_bline(bline.begin(), bline.end());
	// Add at the end the first BLinePoint of the bline to extend (it is the place where it connects)
	inserted_bline.push_back((*value_node)(get_canvas()->get_time()).get_list().front().get(BLinePoint()));
	// if doing complete loop then add at the start the last BLinePoint of the bline to extend
	// (it is where the loop closes)
	if(complete_loop)
		inserted_bline.push_front((*value_node)(get_canvas()->get_time()).get_list().back().get(BLinePoint()));
	// store the length of the inserted bline and the number of segments
	Real inserted_length(bline_length(ValueBase::List(inserted_bline.begin(), inserted_bline.end()), false, NULL));
	int inserted_size(inserted_bline.size());
	// Determine if the bline that the layer belongs to is a Advanced Outline
	bool is_advanced_outline(false);
	Layer::Handle layer_parent;
	std::set<Node*>::iterator niter;
	for(niter=value_node->parent_set.begin();niter!=value_node->parent_set.end();++niter)
	{
		layer_parent=Layer::Handle::cast_dynamic(*niter);
		if(layer_parent && layer_parent->get_name() == "advanced_outline")
		{
			is_advanced_outline=true;
			break;
		}
	}

	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Extend Spline"));

	if(is_advanced_outline)
	{
		ValueNode_WPList::Handle wplist_value_node(ValueNode_WPList::Handle::cast_dynamic(layer_parent->dynamic_param_list().find("wplist")->second));
		if(wplist_value_node)
		{
			// Calculate the number of blinepoints of the original bline
			int value_node_size((*value_node)(get_canvas()->get_time()).get_list().size());
			// Calculate the length of the original bline
			Real value_node_length(bline_length(ValueBase((*value_node)(get_canvas()->get_time()).get_list()), false, NULL));
			// Retrieve the homogeneous parameter value form the layer
			bool homogeneous(layer_parent->get_param("homogeneous").get(bool()));
			//
			// Calculate the new boundaries for each width point on the old wplist
			// and modify the boundaries on the old wplist
			//
			std::list<synfig::WidthPoint> old_wplist;
			ValueBase wplist_value_base((*wplist_value_node)(get_canvas()->get_time()));
			const ValueBase::List &wplist_value_base_list = wplist_value_base.get_list();
			for(ValueBase::List::const_iterator i = wplist_value_base_list.begin(); i != wplist_value_base_list.end(); ++i)
				old_wplist.push_back(i->get(synfig::WidthPoint()));
			std::list<synfig::WidthPoint>::iterator witer;
			int i;
			for(i=0, witer=old_wplist.begin(); witer!=old_wplist.end(); witer++, i++)
			{
				synfigapp::Action::Handle action(synfigapp::Action::create("ValueDescSet"));
				assert(action);
				action->set_param("canvas", get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				synfigapp::ValueDesc value_desc;
				Real lb_new;
				ValueNode_Composite::Handle composite(ValueNode_Composite::Handle::cast_dynamic(wplist_value_node->get_link(i)));
				if(composite)
				{
					value_desc=synfigapp::ValueDesc(composite,4);
					assert(value_desc.is_valid());
					WidthPoint wpi(*witer);
					Real lb(wpi.get_lower_bound());
					Real ub(wpi.get_upper_bound());
					Real range(ub-lb);
					Real l1(inserted_length);
					Real l2(value_node_length);
					int s1(complete_loop?inserted_size-2:inserted_size-1);
					int s2(value_node_size-1);
					if(homogeneous)
					{
						lb_new=ub-(l1+l2)*range/l2;
					}
					else
					{
						lb_new=ub-(s1+s2)*range/s2;
					}
				}
				else
				{
					group.cancel();
					return Smach::RESULT_ERROR;
				}
				action->set_param("value_desc",value_desc);
				action->set_param("new_value", ValueBase(lb_new));
				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					get_canvas_view()->get_ui_interface()->error(_("Unable to set lower boundary for wplist"));
					group.cancel();
					return Smach::RESULT_ERROR;
				}
			}
			//
			// Calculate the new boundaries for each widthpoint of the inserted wplist
			// and insert each one in the wplist form the layer.
			// Don't add the widthpoint with position equal to 1.0
			// to avoid conflicts with the first of the existing wplist.
			// Don't add the widthpoint with position equal to 0.0 if doing
			// complete loops.
			//
			for(witer=wplist.begin(); witer!=wplist.end();witer++)
			{
				if(witer->get_position() == 1.0)
					continue;
				if(complete_loop && witer->get_position() == 0.0)
					continue;
				synfigapp::Action::Handle action(synfigapp::Action::create("ValueNodeDynamicListInsert"));
				assert(action);
				synfigapp::ValueDesc value_desc(wplist_value_node,0);
				action->set_param("canvas", get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_desc", value_desc);
				// Prepare the time to insert
				Real lb(witer->get_lower_bound());
				Real ub(witer->get_upper_bound());
				Real range(ub-lb);
				Real l1(inserted_length);
				Real l2(value_node_length);
				int s1(complete_loop?inserted_size-2:inserted_size-1);
				int s2(value_node_size-1);
				if(homogeneous)
				{
					witer->set_upper_bound(lb+(l1+l2)*range/l1);
				}
				else
				{
					witer->set_upper_bound(lb+(s1+s2)*range/s1);
				}
				if(!action->set_param("item",ValueNode::Handle(ValueNode_Composite::create(*witer))))
					synfig::error("ACTION didn't like \"item\"");
				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					get_canvas_view()->get_ui_interface()->error(_("Unable to insert item"));
					group.cancel();
					return Smach::RESULT_ERROR;
				}
			}
		} // endif wplist_value_node exists
	} // endif is advanced outline

	if (complete_loop)
	{
		synfigapp::Action::Handle action(synfigapp::Action::create("ValueNodeDynamicListLoop"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_node",ValueNode::Handle(value_node));

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to set loop for spline"));
			group.cancel();
			return Smach::RESULT_ERROR;
		}
	}

	std::list<synfig::BLinePoint>::reverse_iterator iter;
	for(iter=bline.rbegin();!(iter==bline.rend());++iter)
	{
		ValueNode_Composite::Handle composite(ValueNode_Composite::create(*iter));

		synfigapp::Action::Handle action(synfigapp::Action::create("ValueNodeDynamicListInsert"));

		assert(action);
		synfigapp::ValueDesc value_desc(value_node,0);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",value_desc);
		if(!action->set_param("item",ValueNode::Handle(composite)))
			synfig::error("ACTION didn't like \"item\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to insert item"));
			group.cancel();
			return Smach::RESULT_ERROR;
		}
	}

	last_stroke=value_node;
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateDraw_Context::extend_bline_from_end(ValueNode_BLine::Handle value_node,std::list<synfig::BLinePoint> bline,std::list<synfig::WidthPoint> wplist,bool complete_loop)
{
	// Recreate the bline that is going to be inserted
	// First copy the list of BlinePoints
	std::list<synfig::BLinePoint> inserted_bline(bline.begin(), bline.end());
	// Add at the start, the last BLinePoint of the bline to extend (it is the place where it connects)
	inserted_bline.push_front((*value_node)(get_canvas()->get_time()).get_list().back().get(BLinePoint()));
	// if doing complete loop then add at the end the last BLinePoint of the bline to extend
	// (it is where the loop closes)
	if(complete_loop)
		inserted_bline.push_back((*value_node)(get_canvas()->get_time()).get_list().front().get(BLinePoint()));
	// store the length of the inserted bline and the number of segments
	Real inserted_length(bline_length(ValueBase::List(inserted_bline.begin(), inserted_bline.end()), false, NULL));
	int inserted_size(inserted_bline.size());
	// Determine if the bline that the layer belongs to is a Advanced Outline
	bool is_advanced_outline(false);
	Layer::Handle layer_parent;
	std::set<Node*>::iterator niter;
	for(niter=value_node->parent_set.begin();niter!=value_node->parent_set.end();++niter)
	{
		layer_parent=Layer::Handle::cast_dynamic(*niter);
		if(layer_parent && layer_parent->get_name() == "advanced_outline")
		{
			is_advanced_outline=true;
			break;
		}
	}

	// Create the action group
	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Extend Spline"));

	if(is_advanced_outline)
	{
		ValueNode_WPList::Handle wplist_value_node(ValueNode_WPList::Handle::cast_dynamic(layer_parent->dynamic_param_list().find("wplist")->second));
		if(wplist_value_node)
		{
			// Calculate the number of blinepoints of the original bline
			int value_node_size((*value_node)(get_canvas()->get_time()).get_list().size());
			// Calculate the length of the original bline
			Real value_node_length(bline_length(ValueBase((*value_node)(get_canvas()->get_time()).get_list()), false, NULL));
			// Retrieve the homogeneous parameter value form the layer
			bool homogeneous(layer_parent->get_param("homogeneous").get(bool()));
			//
			// Calculate the new boundaries for each width point on the old wplist
			// and modify the boundaries on the old wplist
			//
			std::list<synfig::WidthPoint> old_wplist;
			ValueBase wplist_value_base((*wplist_value_node)(get_canvas()->get_time()));
			const ValueBase::List &wplist_value_base_list = wplist_value_base.get_list();
			for(ValueBase::List::const_iterator i = wplist_value_base_list.begin(); i != wplist_value_base_list.end(); ++i)
				old_wplist.push_back(i->get(synfig::WidthPoint()));
			std::list<synfig::WidthPoint>::iterator witer;
			int i;
			for(i=0, witer=old_wplist.begin(); witer!=old_wplist.end(); witer++, i++)
			{
				synfigapp::Action::Handle action(synfigapp::Action::create("ValueDescSet"));
				assert(action);
				action->set_param("canvas", get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				synfigapp::ValueDesc value_desc;
				Real ub_new;
				ValueNode_Composite::Handle composite(ValueNode_Composite::Handle::cast_dynamic(wplist_value_node->get_link(i)));
				if(composite)
				{
					value_desc=synfigapp::ValueDesc(composite,5);
					assert(value_desc.is_valid());
					WidthPoint wpi(*witer);
					Real lb(wpi.get_lower_bound());
					Real ub(wpi.get_upper_bound());
					Real range(ub-lb);
					Real l1(inserted_length);
					Real l2(value_node_length);
					int s1(complete_loop?inserted_size-2:inserted_size-1);
					int s2(value_node_size-1);
					if(homogeneous)
					{
						ub_new=lb+(l1+l2)*range/l2;
					}
					else
					{
						ub_new=lb+(s1+s2)*range/s2;
					}
				}
				else
				{
					group.cancel();
					return Smach::RESULT_ERROR;
				}
				action->set_param("value_desc",value_desc);
				action->set_param("new_value", ValueBase(ub_new));
				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					get_canvas_view()->get_ui_interface()->error(_("Unable to set upper boundary for wplist"));
					group.cancel();
					return Smach::RESULT_ERROR;
				}
			}
			//
			// Calculate the new boundaries for each widthpoint of the inserted wplist
			// and insert each one in the wplist form the layer.
			// Don't add the widthpoint with position equal to 1.0
			// to avoid conflicts with the first of the existing wplist.
			// Don't add the widthpoint with position equal to 0.0 if doing
			// complete loops.
			//
			for(witer=wplist.begin(); witer!=wplist.end();witer++)
			{
				if(witer->get_position() == 0.0)
					continue;
				if(complete_loop && witer->get_position() == 1.0)
					continue;
				synfigapp::Action::Handle action(synfigapp::Action::create("ValueNodeDynamicListInsert"));
				assert(action);
				synfigapp::ValueDesc value_desc(wplist_value_node,0);
				action->set_param("canvas", get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_desc", value_desc);
				// Prepare the time to insert
				Real lb(witer->get_lower_bound());
				Real ub(witer->get_upper_bound());
				Real range(ub-lb);
				Real l1(inserted_length);
				Real l2(value_node_length);
				int s1(complete_loop?inserted_size-2:inserted_size-1);
				int s2(value_node_size-1);
				if(homogeneous)
				{
					witer->set_lower_bound(ub-(l1+l2)*range/l1);
				}
				else
				{
					witer->set_lower_bound(ub-(s1+s2)*range/s1);
				}
				if(!action->set_param("item",ValueNode::Handle(ValueNode_Composite::create(*witer))))
					synfig::error("ACTION didn't like \"item\"");
				if(!get_canvas_interface()->get_instance()->perform_action(action))
				{
					get_canvas_view()->get_ui_interface()->error(_("Unable to insert item"));
					group.cancel();
					return Smach::RESULT_ERROR;
				}
			}
		} // endif wplist_value_node exists
	} // endif is advanced outline

	if (complete_loop)
	{
		synfigapp::Action::Handle action(synfigapp::Action::create("ValueNodeDynamicListLoop"));
		assert(action);

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_node",ValueNode::Handle(value_node));

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to set loop for spline"));
			group.cancel();
			return Smach::RESULT_ERROR;
		}
	}

	std::list<synfig::BLinePoint>::iterator iter;
	for(iter=bline.begin();iter!=bline.end();++iter)
	{
		ValueNode_Composite::Handle composite(ValueNode_Composite::create(*iter));

		synfigapp::Action::Handle action(synfigapp::Action::create("ValueNodeDynamicListInsert"));

		assert(action);
		synfigapp::ValueDesc value_desc(value_node,value_node->link_count());

		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_desc",value_desc);
		if(!action->set_param("item",ValueNode::Handle(composite)))
			synfig::error("ACTION didn't like \"item\"");

		if(!get_canvas_interface()->get_instance()->perform_action(action))
		{
			get_canvas_view()->get_ui_interface()->error(_("Unable to insert item"));
			group.cancel();
			//refresh_ducks();
			return Smach::RESULT_ERROR;
		}
	}
	last_stroke=value_node;
	return Smach::RESULT_ACCEPT;
}

void
StateDraw_Context::reverse_bline(std::list<synfig::BLinePoint> &bline)
{
	int i;

	std::list<synfig::BLinePoint>::iterator iter,eiter;
	iter=bline.begin();
	eiter=bline.end();
	eiter--;
	for(i=0;i<(int)bline.size()/2;++iter,--eiter,i++)
	{
		iter_swap(iter,eiter);
		iter->reverse();
		eiter->reverse();
	}
}

void
StateDraw_Context::reverse_wplist(std::list<synfig::WidthPoint> &wplist)
{
	std::list<synfig::WidthPoint>::iterator iter;
	for(iter=wplist.begin();iter!=wplist.end();iter++)
		iter->reverse();
}

bool StateDraw_Context::link_layer_origin(Layer::Handle& layer, ValueNode::Handle& value_node) const
{
	synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));
	assert(action);

	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",get_canvas_interface());
	action->set_param("layer",layer);
	if(!action->set_param("param",String("origin")))
		synfig::error("LayerParamConnect didn't like \"param\"");
	if(!action->set_param("value_node",value_node))
		synfig::error("LayerParamConnect didn't like \"value_node\"");

	if(!get_canvas_interface()->get_instance()->perform_action(action))
		return false;
	return true;
}


Smach::event_result
StateDraw_Context::fill_last_stroke_and_unselect_other_layers()
{
	if(!last_stroke)
		return Smach::RESULT_OK;

	synfigapp::Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Fill Stroke"));

	Layer::Handle layer;

	get_canvas_interface()->auto_export(last_stroke);

	synfigapp::PushMode push_mode(get_canvas_interface(),synfigapp::MODE_NORMAL);

	Canvas::Handle canvas(get_canvas_view()->get_canvas());
	int depth(0);

	layer=get_canvas_view()->get_selection_manager()->get_selected_layer();
	if(layer)
	{
		depth=layer->get_depth();
		canvas=layer->get_canvas();
	}

	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	layer=get_canvas_interface()->add_layer_to("region", canvas, depth);
	if (!layer) return Smach::RESULT_ERROR;
	layer->set_description(last_stroke_id + _(" Region"));

	// Set blend_method to static (consistent with other Layers)
	ValueBase blend_param_value(get_blend());
	blend_param_value.set_static(true);

	layer->set_param("blend_method",blend_param_value);
	get_canvas_interface()->signal_layer_param_changed()(layer,"blend_method");

	layer->set_param("amount",get_opacity());
	get_canvas_interface()->signal_layer_param_changed()(layer,"amount");

	synfigapp::Action::Handle action(synfigapp::Action::create("LayerParamConnect"));

	assert(action);

	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",get_canvas_interface());
	action->set_param("layer",layer);
	if(!action->set_param("param",String("bline")))
		synfig::error("LayerParamConnect didn't like \"param\"");
	if(!action->set_param("value_node",ValueNode::Handle(last_stroke)))
		synfig::error("LayerParamConnect didn't like \"value_node\"");

	if(!get_canvas_interface()->get_instance()->perform_action(action))
	{
		get_canvas_view()->get_ui_interface()->error(_("Unable to create Region layer"));
		group.cancel();
		return Smach::RESULT_OK;
	}

	if (last_value_node_origin)
	{
		bool linked = link_layer_origin(layer, last_value_node_origin);
		if (!linked) {
			group.cancel();
			get_canvas_view()->get_ui_interface()->error(_("Unable to link origin for Region layer"));
			return Smach::RESULT_ERROR;
		}
	}
	get_canvas_view()->get_selection_manager()->set_selected_layer(layer);
	return Smach::RESULT_OK;
}

void
StateDraw_Context::fill_last_stroke()
{
	if(!last_stroke)
		return;

	synfigapp::SelectionManager::LayerList layer_list = get_canvas_view()->get_selection_manager()->get_selected_layers();
	fill_last_stroke_and_unselect_other_layers();
	get_canvas_view()->get_selection_manager()->set_selected_layers(layer_list);
}

void
StateDraw_Context::toggle_layer_creation()
{
  // don't allow none layer creation
  if (get_layer_region_flag() +
     get_layer_outline_flag() +
     get_layer_advanced_outline_flag() == 0)
  {
    if(layer_region_flag) set_layer_region_flag(true);
    else if(layer_outline_flag) set_layer_outline_flag(true);
    else if(layer_advanced_outline_flag) set_layer_advanced_outline_flag(true);
  }

	// update layer flags
	layer_region_flag = get_layer_region_flag();
	layer_outline_flag = get_layer_outline_flag();
	layer_advanced_outline_flag = get_layer_advanced_outline_flag();

	// link origins
	if (get_layer_region_flag() +
		get_layer_outline_flag() +
		get_layer_advanced_outline_flag() >= 2)
		{
			link_origins_box.set_sensitive(true);
		}
	else link_origins_box.set_sensitive(false);
}
