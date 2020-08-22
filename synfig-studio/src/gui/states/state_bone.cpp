/* === S Y N F I G ========================================================= */
/*!	\file state_bone.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2020 Aditya Abhiram J
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

#include "state_bone.h"
#include <synfig/general.h>


#include "state_normal.h"
#include <synfigapp/value_desc.h>
#include "canvasview.h"
#include "duckmatic.h"
#include "workarea.h"
#include "app.h"
#include <synfig/valuenodes/valuenode_bline.h>
#include <ETL/hermite>
#include "event_mouse.h"
#include "event_keyboard.h"
#include "event_layerclick.h"
#include "docks/dock_toolbox.h"
#include "docks/dialog_tooloptions.h"
#include "widgets/widget_enum.h"
#include "widgets/widget_distance.h"
#include <synfig/transform.h>
#include <synfigapp/main.h>
#include "synfig/layers/layer_skeleton.h"
#include "synfig/layers/layer_skeletondeformation.h"
#include "synfig/valuenodes/valuenode_bone.h"
#include "synfig/valuenodes/valuenode_composite.h"
#include "synfig/valuenodes/valuenode_staticlist.h"
#include "synfigapp/value_desc.h"
#include "synfig/valuetransformation.h"

#include <gui/localization.h>
#include <gtkmm/radiobutton.h>

#include <gtkmm/separatormenuitem.h>
#include <gtkmm/imagemenuitem.h>
#include "synfigapp/action_system.h"
#include "synfigapp/actions/layeradd.h"
#include "synfigapp/actions/valuedesccreatechildbone.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace studio;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

// indentation for options layout
#ifndef SPACING
#define SPACING(name, px) \
	Gtk::Alignment *name = Gtk::manage(new Gtk::Alignment()); \
	name->set_size_request(px)
#endif

#define GAP	(3)
#define DEFAULT_WIDTH (0.1)

/* === G L O B A L S ======================================================= */

StateBone studio::state_bone;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateBone_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	bool prev_table_status;
	bool prev_workarea_layer_status_;
	Duck::Type prev_type_mask;

	int depth;
	Canvas::Handle canvas;

	Duck::Handle  point2_duck,point1_duck;
	handle<Duckmatic::Bezier> bone_bezier;

	Gtk::Menu menu;

	int active_bone;
	int c_layer;
	bool drawing;

	Point clickOrigin;

	// Toolbox settings
	synfigapp::Settings& settings;

	// holder of optons
	Gtk::Grid options_table;

	// title
	Gtk::Label title_label;

	// layer name:
	Gtk::Label id_label;
	Gtk::HBox id_box;
	Gtk::Entry id_entry;

	//  bone width
	Gtk::Label bone_width_label;
	Widget_Distance bone_width_dist;


	// Layer creation radio group
	Gtk::Label layer_label;
	Gtk::RadioButton::Group radiogroup;
	Gtk::RadioButton radiobutton_skel;
	Gtk::RadioButton radiobutton_skel_deform;
	Gtk::Button create_layer;


public:

	synfig::String get_id() const { return id_entry.get_text();}
	void set_id(const synfig::String& x){ return id_entry.set_text(x);}

	Real get_bone_width() const {
		return bone_width_dist.get_value().get(
				Distance::SYSTEM_UNITS,
				get_canvas_view()->get_canvas()->rend_desc()
		);
	}
	void set_bone_width(Distance x){return bone_width_dist.set_value(x);}


	void update_layer(){
		if(c_layer==0) {
			save_settings();
			c_layer=1;
			load_settings();
		}
		else{
			save_settings();
			c_layer=0;
			load_settings();
		}
	}

	void make_layer();

	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_mouse_drag_handler(const Smach::event& x);
	Smach::event_result event_mouse_release_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	Smach::event_result event_hijack(const Smach::event& /*x*/){return Smach::RESULT_ACCEPT; }

	void refresh_tool_options();

	StateBone_Context(CanvasView* canvas_view);

	~StateBone_Context();

	const etl::handle<CanvasView>& get_canvas_view() const {return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface() const {return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas() const {return canvas_view_->get_canvas();}
	WorkArea * get_work_area() const {return canvas_view_->get_work_area();}
	const synfig::TransformStack& get_transform_stack() const {return get_work_area()->get_curr_transform_stack();}
	int find_bone(Point point,Layer::Handle layer,int lay=0)const;
	void _on_signal_change_active_bone(ValueNode::Handle node);
	int change_active_bone(ValueNode::Handle node);

	void load_settings();
	void save_settings();
	void reset();
	void increment_id();

	bool egress_on_selection_change;
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& /*x*/)
	{
		Layer::Handle layer = get_canvas_interface()->get_selection_manager()->get_selected_layer();
		Layer_Skeleton::Handle  skel_layer = etl::handle<Layer_Skeleton>::cast_dynamic(layer);
		Layer_SkeletonDeformation::Handle deform_layer = etl::handle<Layer_SkeletonDeformation>::cast_dynamic(layer);

		if(!(skel_layer || deform_layer)){
			get_work_area()->set_type_mask(get_work_area()->get_type_mask()-Duck::TYPE_TANGENT-Duck::TYPE_WIDTH);
			get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);
		}
		get_work_area()->queue_draw();
		get_canvas_view()->queue_rebuild_ducks();

		return Smach::RESULT_OK;
	}

}; // END of class StateBone_Context

/* === M E T H O D S ======================================================= */

StateBone::StateBone() :
	Smach::state<StateBone_Context>("bone")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,		&StateBone_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,				&StateBone_Context::event_hijack));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,	&StateBone_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,	&StateBone_Context::event_mouse_drag_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,	&StateBone_Context::event_mouse_release_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,		&StateBone_Context::event_refresh_tool_options));
}

StateBone::~StateBone()
= default;

void* StateBone::enter_state(studio::CanvasView *machine_context) const
{
	return new StateBone_Context(machine_context);
}

void
StateBone_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC,"C");
		string value;
		if(c_layer==0){
			if(settings.get_value("bone.skel_id",value))
				set_id(value);
			else
				set_id(_("NewSkeleton"));
		}else{
			if(settings.get_value("bone.skel_deform_id",value))
				set_id(value);
			else
				set_id(_("NewSkeletonDeformation"));
		}

		if(c_layer==0){
			if(settings.get_value("bone.skel_bone_width",value) && !value.empty())
				set_bone_width(Distance(atof(value.c_str()),Distance::SYSTEM_UNITS));
			else
				set_bone_width(Distance(DEFAULT_WIDTH,Distance::SYSTEM_UNITS)); // default width
		}else{
			if(settings.get_value("bone.skel_deform_bone_width",value) && !value.empty())
				set_bone_width(Distance(atof(value.c_str()),Distance::SYSTEM_UNITS));
			else
				set_bone_width(Distance(DEFAULT_WIDTH,Distance::SYSTEM_UNITS)); // default width
		}
	}
	catch(...)
	{
		synfig::warning("State Bone: Caught exception when attempting to load settings.");
	}
}

void
StateBone_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC,"C");
		if(c_layer==0)
			settings.set_value("bone.skel_id",get_id().c_str());
		else
			settings.set_value("bone.skel_deform_id",get_id().c_str());
		if(c_layer==0){
			settings.set_value("bone.skel_bone_width",bone_width_dist.get_value().get_string());
		}else{
			settings.set_value("bone.skel_deform_bone_width",bone_width_dist.get_value().get_string());
		}
	}
	catch (...)
	{
		synfig::warning("State Bone: Caught exception when attempting to save settings.");
	}
}

void
StateBone_Context::reset()
{
	active_bone = -1;
	set_id(_("NewSkeleton"));
	set_bone_width(Distance(DEFAULT_WIDTH,Distance::SYSTEM_UNITS)); // default width
}

void
StateBone_Context::increment_id()
{
	String id(get_id());
	int number=1;
	int digits=0;

	if(id.empty())
		id="NewSkeleton";

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

StateBone_Context::StateBone_Context(CanvasView *canvas_view) :
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	prev_table_status(false),
	prev_workarea_layer_status_(get_work_area()->get_allow_layer_clicks()),
	depth(-1),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	active_bone(change_active_bone(get_work_area()->get_active_bone_value_node())),
	radiobutton_skel(radiogroup,_("Skeleton Layer")),
	radiobutton_skel_deform(radiogroup,_("Skeleton Deform Layer")),
	c_layer(0),
	drawing(false)
{
	egress_on_selection_change=true;

	/*setting up the tool options menu*/

	// 0, title
	title_label.set_label(_("Skeleton Creation"));
	Pango::AttrList list;
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	list.insert(attr);
	title_label.set_attributes(list);
	title_label.set_alignment(Gtk::ALIGN_START,Gtk::ALIGN_CENTER);

	// 1, layer name label and entry
	id_label.set_label(_("Name:"));
	id_label.set_alignment(Gtk::ALIGN_START,Gtk::ALIGN_CENTER);


	// 2, Bone width
	bone_width_label.set_label(_("Bone Width:"));
	bone_width_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	bone_width_label.set_sensitive(true);
	bone_width_dist.set_digits(2);
	bone_width_dist.set_range(0,10000000);
	bone_width_dist.set_sensitive(true);

	load_settings();

	// 4, Layer choice
	layer_label.set_label(_("Layer to Create:"));
	layer_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	layer_label.set_attributes(list);
	layer_label.set_sensitive(true);
	
	create_layer.set_label(_("Create Layer"));
	create_layer.set_alignment(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER);
	create_layer.set_sensitive(true);


	// pack all options to the options_table

	// 0, title
	options_table.attach(title_label,0, 0,1,1);
	// 1, name
	options_table.attach(id_label,0, 1,1,1);
	options_table.attach(id_entry,2, 1,3,1);
	// 2, default bone width
	options_table.attach(bone_width_label,0,2,1,1);
	options_table.attach(bone_width_dist,2, 2,3,1);
	// 3, Layer choice
	options_table.attach(layer_label,0, 3,1,1);
	options_table.attach(radiobutton_skel,0, 4,2,1);
	options_table.attach(radiobutton_skel_deform,2, 4,2,1);
	options_table.attach(create_layer,2,5,2,1);

	create_layer.signal_clicked().connect(sigc::mem_fun(*this,&StateBone_Context::make_layer));
	radiobutton_skel.signal_toggled().connect(sigc::mem_fun(*this,&StateBone_Context::update_layer));	

	// fine-tune options layout
	options_table.set_border_width(GAP*2); // border width
	options_table.set_row_spacing(GAP); // row gap
	options_table.set_margin_bottom(0);
	options_table.show_all();

	refresh_tool_options();
	App::dialog_tool_options->present();

	// remembering previous type mask
	prev_type_mask = get_work_area()->get_type_mask();

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// Turn on Active Bone rendering
	get_work_area()->set_active_bone_display(true);

	//ducks
	Layer::Handle layer = get_canvas_interface()->get_selection_manager()->get_selected_layer();

	if(etl::handle<Layer_SkeletonDeformation>::cast_dynamic(layer)){
		get_work_area()->set_type_mask(get_work_area()->get_type_mask()-Duck::TYPE_TANGENT|Duck::TYPE_WIDTH);
		get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);
		layer->disable();
		get_canvas_interface()->signal_layer_status_changed()(layer,false);
	}else{
		get_work_area()->set_type_mask(get_work_area()->get_type_mask()-Duck::TYPE_TANGENT-Duck::TYPE_WIDTH);
		get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);
	}


	//signals
	get_canvas_interface()->signal_active_bone_changed().connect(sigc::mem_fun(*this,&studio::StateBone_Context::_on_signal_change_active_bone));

	// Refresh the work area
	get_work_area()->queue_draw();

	// Hide the tables if they are showing
	prev_table_status=get_canvas_view()->tables_are_visible();
	if(prev_table_status)get_canvas_view()->hide_tables();

	// Disable the time bar
	get_canvas_view()->set_sensitive_timebar(false);


	get_work_area()->set_cursor(Gdk::DOT);

	App::dock_toolbox->refresh();

	get_canvas_view()->queue_rebuild_ducks();
}

void
StateBone_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Skeleton Tool"));
	App::dialog_tool_options->set_name("bone");

	App::dialog_tool_options->add_button(
			Gtk::StockID("gtk-clear"),
			_("Clear current Skeleton")
	)->signal_clicked().connect(
			sigc::mem_fun(
					*this,
					&StateBone_Context::reset
			)
	);
}

Smach::event_result
StateBone_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateBone_Context::~StateBone_Context()
{
	save_settings();
	App::dialog_tool_options->clear();

	get_work_area()->reset_cursor();

	// Turn off Active Bone rendering
	get_work_area()->set_active_bone_display(false);

	// Restore duck type mask
	get_work_area()->set_type_mask(prev_type_mask);
	get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);

	Layer::Handle layer = get_canvas_interface()->get_selection_manager()->get_selected_layer();
	if(Layer_SkeletonDeformation::Handle::cast_dynamic(layer)){
		layer->enable();
		get_canvas_interface()->signal_layer_status_changed()(layer,true);
	}

	// Restore layer clicking
	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_status_);

	// Enable the time bar
	get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();

	App::dock_toolbox->refresh();
}

Smach::event_result
StateBone_Context::event_mouse_click_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	Point p(get_work_area()->snap_point_to_grid(event.pos));

	switch(event.button)
	{
		case BUTTON_LEFT:
		{
			clickOrigin = p;

			point1_duck=new Duck();
			point1_duck->set_point(p);
			point1_duck->set_name("p1");
			point1_duck->set_type(Duck::TYPE_POSITION);
			point1_duck->set_editable(false);
			get_work_area()->add_duck(point1_duck);

			point2_duck=new Duck();
			point2_duck->set_point(Vector(0,0));
			point2_duck->set_name("p2");
			point2_duck->set_origin(point1_duck);
			point2_duck->set_scalar(-1);
			point2_duck->set_type(Duck::TYPE_RADIUS);
			point2_duck->set_hover(true);
			get_work_area()->add_duck(point2_duck);

			bone_bezier =new Duckmatic::Bezier();
			bone_bezier->p1=bone_bezier->c1=point1_duck;
			bone_bezier->p2=bone_bezier->c2=point2_duck;
			get_work_area()->add_bezier(bone_bezier);

			drawing = true;

			return Smach::RESULT_ACCEPT;
		}

		default:
			return Smach::RESULT_OK;
	}
}

Smach::event_result StateBone_Context::event_mouse_drag_handler(const Smach::event &x) {
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	if(event.button==BUTTON_LEFT){
		if (!point2_duck) return Smach::RESULT_OK;
		point2_duck->set_point(clickOrigin-get_work_area()->snap_point_to_grid(event.pos));
		get_work_area()->queue_draw();
		return Smach::RESULT_ACCEPT;
	}
	return Smach::RESULT_OK;
}

Smach::event_result
StateBone_Context::event_mouse_release_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	Point releaseOrigin(get_work_area()->snap_point_to_grid(event.pos));

	Layer::Handle layer = get_canvas_interface()->get_selection_manager()->get_selected_layer();
	Layer_Skeleton::Handle  skel_layer = etl::handle<Layer_Skeleton>::cast_dynamic(layer);
	Layer_SkeletonDeformation::Handle deform_layer = etl::handle<Layer_SkeletonDeformation>::cast_dynamic(layer);
	const synfig::TransformStack& transform(get_work_area()->get_curr_transform_stack());

	Action::Handle createChild(Action::Handle(Action::create("ValueDescCreateChildBone")));
	createChild->set_param("canvas",get_canvas());
	createChild->set_param("canvas_interface",get_canvas_interface());
	createChild->set_param("highlight",true);

	Action::Handle setActiveBone(Action::Handle(Action::create("ValueNodeSetActiveBone")));
	setActiveBone->set_param("canvas",get_canvas());
	setActiveBone->set_param("canvas_interface",get_canvas_interface());
	if(drawing){
		get_work_area()->erase_duck(point1_duck);
		get_work_area()->erase_duck(point2_duck);
		get_work_area()->erase_bezier(bone_bezier);
		get_canvas_view()->rebuild_ducks();
	}

	Duck::Handle duck(get_work_area()->find_duck(releaseOrigin,0.1));

	switch(event.button)
	{
		case BUTTON_LEFT:
		{
			clickOrigin = transform.unperform(clickOrigin);
			releaseOrigin = transform.unperform(releaseOrigin);
			if(drawing){ //! if the user was not modifying a duck
				if(skel_layer){ //!if selected layer is a Skeleton Layer and user wants to work on a skeleton layer
					bool is_currently_on(get_work_area()->get_type_mask()&Duck::TYPE_WIDTH);
					if(is_currently_on){
						get_canvas_view()->toggle_duck_mask(Duck::TYPE_WIDTH);
					}

					createChild->set_param("canvas",skel_layer->get_canvas());
					ValueDesc list_desc(layer,"bones");
					int b = -1;
					if((clickOrigin-releaseOrigin).mag()<0.01)
						b=find_bone(clickOrigin,layer);
					if(b!=-1){ //! if bone found around the release point --> set active bone
						active_bone=b;
						ValueNode_StaticList::Handle list_node;
						list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
						ValueDesc value_desc= ValueDesc(list_node,active_bone,list_desc);
						setActiveBone->set_param("active_bone_node",value_desc.get_value_node());
						setActiveBone->set_param("prev_active_bone_node",get_work_area()->get_active_bone_value_node());
						if(setActiveBone->is_ready()){
							try{
								get_canvas_interface()->get_instance()->perform_action(setActiveBone);
							} catch (...) {
								info("Error performing action");
							}
						}
					}
					else{
						ValueNode_StaticList::Handle list_node;
						list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
						//cout<<"Active Bone: "<<active_bone<<endl;
						if(active_bone!=-1 && !list_node->list.empty()){ //! if active bone is already set
							ValueDesc value_desc= ValueDesc(list_node,active_bone,list_desc);
							//cout<<"Active bone name : "<<value_desc.get_name()<<endl;
							ValueNode_Bone::Handle bone_node;
							if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
							{
								error("expected a ValueNode_Bone");
								assert(0);
							}
							ValueDesc v_d = ValueDesc(bone_node,bone_node->get_link_index_from_name("origin"),value_desc);
							Real sx = bone_node->get_link("scalelx")->operator()(get_canvas()->get_time()).get(Real());
							Matrix matrix = value_desc.get_value(get_canvas()->get_time()).get(Bone()).get_animated_matrix();
							Real angle = atan2(matrix.axis(0)[1],matrix.axis(0)[0]);
							Real a =acos(0.0);
							matrix = matrix.get_inverted();
							Point aOrigin = matrix.get_transformed(clickOrigin);
							aOrigin[0]/=sx;

							createChild->set_param("value_desc",Action::Param(v_d));
							createChild->set_param("origin",Action::Param(ValueBase(aOrigin)));
							createChild->set_param("width",Action::Param(ValueBase(get_bone_width())));
							createChild->set_param("tipwidth",Action::Param(ValueBase(get_bone_width())));
							createChild->set_param("prev_active_bone_node",Action::Param(get_work_area()->get_active_bone_value_node()));
							if((clickOrigin-releaseOrigin).mag()>=0.01) {
								a = atan2((releaseOrigin-clickOrigin)[1],(releaseOrigin-clickOrigin)[0]);
								createChild->set_param("angle",Action::Param(ValueBase(Angle::rad(a-angle))));
								createChild->set_param("scalelx", Action::Param(ValueBase((releaseOrigin - clickOrigin).mag())));
							}else{
								createChild->set_param("angle",Action::Param(ValueBase(Angle::rad(a-angle))));
								createChild->set_param("scalelx", Action::Param(ValueBase(1.0)));

							}

							if(createChild->is_ready()){
								try{
									get_canvas_interface()->get_instance()->perform_action(createChild);
								} catch (...) {
									info("Error performing action");
								}
							}
						}
						else{
							Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),"Add Bone");

							Action::Handle action = Action::create("ValueNodeStaticListInsert");
							action->set_param("canvas", get_canvas());
							action->set_param("canvas_interface", get_canvas_interface());
							action->set_param("time", get_canvas()->get_time());

							Bone bone = Bone();

							bone.set_parent(ValueNode_Bone_Root::create(Bone()));
							bone.set_origin(clickOrigin);
							bone.set_width(get_bone_width());
							bone.set_tipwidth(get_bone_width());
							bone.set_angle(Angle::rad(acos(0.0)));
							if((clickOrigin-releaseOrigin).mag()>=0.01) {
								bone.set_scalelx((releaseOrigin - clickOrigin).mag());
								bone.set_angle((releaseOrigin - clickOrigin).angle());
							}

							ValueNode_Bone::Handle bone_node = ValueNode_Bone::create(bone);
							action->set_param("item",ValueNode::Handle::cast_dynamic(bone_node));

							action->set_param("value_desc",ValueDesc(list_node,0));

							if(action->is_ready()){
								try{
									get_canvas_interface()->get_instance()->perform_action(action);
								} catch (...) {
									info("Error performing action");
								}
							}

							Action::Handle setActiveBone(Action::Handle(Action::create("ValueNodeSetActiveBone")));
							setActiveBone->set_param("canvas",get_canvas());
							setActiveBone->set_param("canvas_interface",get_canvas_interface());

							setActiveBone->set_param("active_bone_node",ValueNode::Handle::cast_dynamic(bone_node));
							setActiveBone->set_param("prev_active_bone_node",get_work_area()->get_active_bone_value_node());

							if (setActiveBone->is_ready()){
								try{
									get_canvas_interface()->get_instance()->perform_action(setActiveBone);
								} catch (...) {
									info("Error performing action");
								}
							}
						}

					}
				}
				else if(deform_layer){ //!if selected layer is a Skeleton deform Layer and user wants to work on a skeleton deform layer
					bool is_currently_on(get_work_area()->get_type_mask()&Duck::TYPE_WIDTH);
					if(!is_currently_on){
						get_canvas_view()->toggle_duck_mask(Duck::TYPE_WIDTH);
					}

					createChild->set_param("canvas",deform_layer->get_canvas());
					ValueDesc list_desc(layer,"bones");
					int b = -1;
					if((clickOrigin-releaseOrigin).mag()<0.01)
						b=find_bone(clickOrigin,layer,1);

					if(b!=-1){ //! if bone found around the release point --> set active bone
						active_bone=b;
						ValueNode_StaticList::Handle list_node;
						list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
						ValueDesc value_desc= ValueDesc(list_node,active_bone,list_desc);
						ValueNode_Composite::Handle comp = ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node());

						setActiveBone->set_param("active_bone_node",comp->get_link("first"));
						setActiveBone->set_param("prev_active_bone_node",get_work_area()->get_active_bone_value_node());

						if(setActiveBone->is_ready()){
							try{
								get_canvas_interface()->get_instance()->perform_action(setActiveBone);
							} catch (...) {
								info("Error performing action");
							}
						}
					}else{
						ValueNode_StaticList::Handle list_node;
						list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
						if(active_bone!=-1 && !list_node->list.empty()){ //! if active bone is already set
							ValueDesc value_desc= ValueDesc(list_node,active_bone,list_desc);
							ValueNode_Bone::Handle bone_node;

							ValueNode_Composite::Handle comp = ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node());
							value_desc =  ValueDesc(comp,comp->get_link_index_from_name("first"),value_desc);
							if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
							{
								error("expected a ValueNode_Bone");
								assert(0);
							}
							ValueDesc v_d = ValueDesc(bone_node,bone_node->get_link_index_from_name("origin"),value_desc);
							Real sx = bone_node->get_link("scalelx")->operator()(get_canvas()->get_time()).get(Real());
							Matrix matrix = value_desc.get_value(get_canvas()->get_time()).get(Bone()).get_animated_matrix();
							Real angle = atan2(matrix.axis(0)[1],matrix.axis(0)[0]);
							Real a =acos(0.0);
							matrix = matrix.get_inverted();
							Point aOrigin = matrix.get_transformed(clickOrigin);
							aOrigin[0]/=sx;

							createChild->set_param("value_desc",Action::Param(v_d));
							createChild->set_param("origin",Action::Param(ValueBase(aOrigin)));
							createChild->set_param("width",Action::Param(ValueBase(get_bone_width())));
							createChild->set_param("tipwidth",Action::Param(ValueBase(get_bone_width())));
							createChild->set_param("prev_active_bone_node",Action::Param(get_work_area()->get_active_bone_value_node()));
							if((clickOrigin-releaseOrigin).mag()>=0.01) {
								a = atan2((releaseOrigin-clickOrigin)[1],(releaseOrigin-clickOrigin)[0]);
								createChild->set_param("angle",Action::Param(ValueBase(Angle::rad(a-angle))));
								createChild->set_param("scalelx", Action::Param(ValueBase((releaseOrigin - clickOrigin).mag())));
							}else{
								createChild->set_param("angle",Action::Param(ValueBase(Angle::rad(a-angle))));
								createChild->set_param("scalelx", Action::Param(ValueBase(1.0)));
							}
							if(createChild->is_ready()){
								try{
									get_canvas_interface()->get_instance()->perform_action(createChild);
									value_desc= ValueDesc(list_node,active_bone,list_desc);

								} catch (...) {
									info("Error performing action");
								}
							}
						}else{
							Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),"Add Bone");

							Action::Handle action = Action::create("ValueNodeStaticListInsert");
							action->set_param("canvas", get_canvas());
							action->set_param("canvas_interface", get_canvas_interface());
							action->set_param("time", get_canvas()->get_time());

							Bone bone1 = Bone();

							bone1.set_parent(ValueNode_Bone_Root::create(Bone()));
							bone1.set_origin(clickOrigin);
							bone1.set_width(get_bone_width());
							bone1.set_tipwidth(get_bone_width());
							bone1.set_angle(Angle::rad(acos(0.0)));
							if((clickOrigin-releaseOrigin).mag()>=0.01) {
								bone1.set_scalelx((releaseOrigin - clickOrigin).mag());
								bone1.set_angle((releaseOrigin - clickOrigin).angle());
							}

							Bone bone2 = Bone();

							bone2.set_parent(ValueNode_Bone_Root::create(Bone()));
							bone2.set_origin(clickOrigin);
							bone2.set_width(get_bone_width());
							bone2.set_tipwidth(get_bone_width());
							bone2.set_angle(Angle::rad(acos(0.0)));
							if((clickOrigin-releaseOrigin).mag()>=0.01) {
								bone2.set_scalelx((releaseOrigin - clickOrigin).mag());
								bone2.set_angle((releaseOrigin - clickOrigin).angle());
							}

							ValueNode_Composite::Handle bone_pair = ValueNode_Composite::create(pair<Bone,Bone>(bone1,bone2));

							action->set_param("item",ValueNode::Handle::cast_dynamic(bone_pair));
							action->set_param("value_desc",ValueDesc(list_node,0));
							if(action->is_ready()){
								try{
									get_canvas_interface()->get_instance()->perform_action(action);
								} catch (...) {
									info("Error performing action");
								}
							}

							Action::Handle setActiveBone(Action::Handle(Action::create("ValueNodeSetActiveBone")));
							setActiveBone->set_param("canvas",get_canvas());
							setActiveBone->set_param("canvas_interface",get_canvas_interface());

							setActiveBone->set_param("active_bone_node",ValueNode::Handle::cast_dynamic(bone_pair->get_link("first")));
							setActiveBone->set_param("prev_active_bone_node",get_work_area()->get_active_bone_value_node());

							if (setActiveBone->is_ready()){
								try{
									get_canvas_interface()->get_instance()->perform_action(setActiveBone);
								} catch (...) {
									info("Error performing action");
								}
							}
						}

					}
				}
				else
				{ //! Creating empty layer as there's no active skeleton layer of any type
					egress_on_selection_change=false;
					if(c_layer==0)
						get_canvas_view()->add_layer("skeleton");
					else if(c_layer==1)
						get_canvas_view()->add_layer("skeleton_deformation");
					Layer::Handle new_skel= get_canvas_interface()->get_selection_manager()->get_selected_layer();
					new_skel->set_param("name",get_id().c_str());
					new_skel->set_description(get_id());
					ValueDesc list_desc(new_skel,"bones");
					ValueNode_StaticList::Handle list_node;
					list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
					ValueDesc value_desc= ValueDesc(list_node,0,list_desc);
					active_bone = 0;
					ValueNode_Bone::Handle bone_node;
					if(c_layer==0){
						bool is_currently_on(get_work_area()->get_type_mask()&Duck::TYPE_WIDTH);
						if(is_currently_on){
							get_canvas_view()->toggle_duck_mask(Duck::TYPE_WIDTH);
						}
						get_work_area()->set_active_bone_value_node(value_desc.get_value_node());
						if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
						{
							error("expected a ValueNode_Bone");
							assert(0);
						}
					}else if(c_layer==1){
						new_skel->disable();

						bool is_currently_on(get_work_area()->get_type_mask()&Duck::TYPE_WIDTH);
						if(!is_currently_on){
							get_canvas_view()->toggle_duck_mask(Duck::TYPE_WIDTH);
						}

						ValueNode_Composite::Handle comp = ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node());

						if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(comp->get_link("first"))))
						{
							error("expected a ValueNode_Bone");
							assert(0);
						}
						bone_node->set_link("origin",ValueNode_Const::create(clickOrigin));
						bone_node->set_link("width",ValueNode_Const::create(get_bone_width()));
						bone_node->set_link("tipwidth",ValueNode_Const::create(get_bone_width()));
						bone_node->set_link("angle",ValueNode_Const::create(Angle::rad(acos(0.0))));
						if((clickOrigin - releaseOrigin).mag() >= 0.001){
							bone_node->set_link("angle",ValueNode_Const::create((releaseOrigin-clickOrigin).angle()));
							bone_node->set_link("scalelx",ValueNode_Const::create((releaseOrigin-clickOrigin).mag()));
						}
						bone_node = ValueNode_Bone::Handle::cast_dynamic(comp->get_link("second"));
						get_work_area()->set_active_bone_value_node(comp->get_link("first"));
					}
					bone_node->set_link("origin",ValueNode_Const::create(clickOrigin));
					bone_node->set_link("width",ValueNode_Const::create(get_bone_width()));
					bone_node->set_link("tipwidth",ValueNode_Const::create(get_bone_width()));
					bone_node->set_link("angle",ValueNode_Const::create(Angle::rad(acos(0.0))));
					if((clickOrigin - releaseOrigin).mag() >= 0.001){
						bone_node->set_link("angle",ValueNode_Const::create((releaseOrigin-clickOrigin).angle()));
						bone_node->set_link("scalelx",ValueNode_Const::create((releaseOrigin-clickOrigin).mag()));
					}

					get_canvas_interface()->get_selection_manager()->clear_selected_layers();
					get_canvas_interface()->get_selection_manager()->set_selected_layer(new_skel);
					egress_on_selection_change=true;

					get_canvas_view()->queue_rebuild_ducks();
				}
				drawing = false;
			}
			else{
				if(duck){
					_on_signal_change_active_bone(duck->get_value_desc().get_parent_value_node());
					get_work_area()->set_active_bone_value_node(duck->get_value_desc().get_parent_value_node());
				}
			}
			return Smach::RESULT_ACCEPT;
		}
		default:
			return Smach::RESULT_OK;
	}
}

int
StateBone_Context::find_bone(Point point,Layer::Handle layer,int lay)const
{
	if(lay==0){
		layer = Layer_Skeleton::Handle::cast_dynamic(layer);
		ValueDesc list_desc(layer,"bones");
		vector<Bone> list=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node())->operator()(get_canvas()->get_time()).get_list_of(Bone());
		Real close_line(10000000),close_origin(10000000);
		Vector direction;
		Angle angle;
		int ret;
		Matrix m;
		for(auto iter=list.begin();iter!=list.end();++iter){

			m=iter->get_animated_matrix();
			Point orig = m.get_transformed(Vector(0,0));
			angle = Angle::rad(atan2(m.axis(0)[1],m.axis(0)[0]));
			Real orig_dist((point-orig).mag());
			Real dist=abs(orig_dist*Angle::sin((point-orig).angle()-angle).get());
			Real length = iter->get_length()*iter->get_scalelx();
			if(Angle::cos((point-orig).angle()-angle).get()>0 && orig_dist<=length){
				dist = abs(dist);
				if(dist<close_line){
					close_line=dist;
					close_origin=orig_dist;
					ret = iter-list.begin();
				}else if(fabs(dist-close_line)<0.0000001 && close_line!= 10000000){
					if(orig_dist<close_origin){
						close_origin=orig_dist;
						ret = iter-list.begin();
					}
				}
			}

		}
		if(abs(close_line)<=0.2){
			if(ret<list.size())return ret;
			else return -1;
		}else{
			return -1;
		}
	}
	else if(lay==1){
		layer = Layer_SkeletonDeformation::Handle::cast_dynamic(layer);
		ValueDesc list_desc(layer,"bones");
		vector<pair<Bone,Bone>> list=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node())->operator()(get_canvas()->get_time()).get_list_of(pair<Bone,Bone>());
		Real close_line(10000000),close_origin(10000000);
		Vector direction;
		Angle angle;
		int ret;
		Matrix m;
		for(auto iter=list.begin();iter!=list.end();++iter){

			m=iter->second.get_animated_matrix();
			Point orig = m.get_transformed(Vector(0,0));
			angle = Angle::rad(atan2(m.axis(0)[1],m.axis(0)[0]));
			Real orig_dist((point-orig).mag());
			Real dist=abs(orig_dist*Angle::sin((point-orig).angle()-angle).get());
			Real length = iter->second.get_length()*iter->second.get_scalelx();
			if(Angle::cos((point-orig).angle()-angle).get()>0 && orig_dist<=length){
				dist = abs(dist);
				if(dist<close_line){
					close_line=dist;
					close_origin=orig_dist;
					ret = iter-list.begin();
				}else if(fabs(dist-close_line)<0.0000001 && close_line!= 10000000){
					if(orig_dist<close_origin){
						close_origin=orig_dist;
						ret = iter-list.begin();
					}
				}
			}

		}
		if(abs(close_line)<=0.2){
			if(ret<list.size())return ret;
			else return -1;
		}else{
			return -1;
		}
		return -1;
	}
	return -1;
}

void
StateBone_Context::make_layer(){
	egress_on_selection_change=false;

	bool is_currently_on(get_work_area()->get_type_mask()&Duck::TYPE_WIDTH);
	if(c_layer==0){
		if(is_currently_on){
			get_canvas_view()->toggle_duck_mask(Duck::TYPE_WIDTH);
		}
		get_canvas_view()->add_layer("skeleton");
	}
	else if(c_layer==1){
		if(!is_currently_on){
			get_canvas_view()->toggle_duck_mask(Duck::TYPE_WIDTH);
		}
		get_canvas_view()->add_layer("skeleton_deformation");
	}
	Layer::Handle new_skel= get_canvas_interface()->get_selection_manager()->get_selected_layer();
	new_skel->set_param("name",get_id().c_str());
	new_skel->set_description(get_id());
	ValueDesc list_desc(new_skel,"bones");
	ValueNode_StaticList::Handle list_node;
	list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
	ValueDesc value_desc= ValueDesc(list_node,0,list_desc);
	active_bone = 0;
	ValueNode_Bone::Handle bone_node;

	if(c_layer==0){
		if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
		{
			error("expected a ValueNode_Bone");
			assert(0);
		}
		bone_node->set_link("width",ValueNode_Const::create(get_bone_width()));
		bone_node->set_link("tipwidth",ValueNode_Const::create(get_bone_width()));
	}else if(c_layer==1){
		new_skel->disable();

		ValueNode_Composite::Handle comp = ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node());
		value_desc =  ValueDesc(comp,comp->get_link_index_from_name("second"),value_desc);
		if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
		{
			error("expected a ValueNode_Bone");
			assert(0);
		}
		bone_node->set_link("width",ValueNode_Const::create(get_bone_width()));
		bone_node->set_link("tipwidth",ValueNode_Const::create(get_bone_width()));
		value_desc =  ValueDesc(comp,comp->get_link_index_from_name("first"),value_desc);
		if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
		{
			error("expected a ValueNode_Bone");
			assert(0);
		}
		bone_node->set_link("width",ValueNode_Const::create(get_bone_width()));
		bone_node->set_link("tipwidth",ValueNode_Const::create(get_bone_width()));
	}
	list_node->erase(list_node->list[0]);
	get_canvas_interface()->get_selection_manager()->clear_selected_layers();
	get_canvas_interface()->get_selection_manager()->set_selected_layer(new_skel);
	egress_on_selection_change=true;
	increment_id();
	get_canvas_view()->queue_rebuild_ducks();
}


void
StateBone_Context::_on_signal_change_active_bone(ValueNode::Handle node){
	ValueNode_Bone::Handle bone;
	Layer::Handle layer = get_canvas_interface()->get_selection_manager()->get_selected_layer();
	Layer_Skeleton::Handle  skel_layer = etl::handle<Layer_Skeleton>::cast_dynamic(layer);
	Layer_SkeletonDeformation::Handle deform_layer = etl::handle<Layer_SkeletonDeformation>::cast_dynamic(layer);
	

	if(bone = ValueNode_Bone::Handle::cast_dynamic(node)){
		if(skel_layer){
			ValueDesc list_desc(layer,"bones");
			ValueNode_StaticList::Handle list_node;
			list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
			for(int i=0;i<list_node->link_count();i++){
				ValueDesc value_desc(list_node,i,list_desc);
				if(value_desc.get_value_node()==node){
					active_bone = i;
					return;
				}
			}
	
		}
	}
}

int
StateBone_Context::change_active_bone(ValueNode::Handle node){
	ValueNode_Bone::Handle bone;
	Layer::Handle layer = get_canvas_interface()->get_selection_manager()->get_selected_layer();
	Layer_Skeleton::Handle  skel_layer = etl::handle<Layer_Skeleton>::cast_dynamic(layer);
	Layer_SkeletonDeformation::Handle deform_layer = etl::handle<Layer_SkeletonDeformation>::cast_dynamic(layer);


	if(bone = ValueNode_Bone::Handle::cast_dynamic(node)){
		if(skel_layer){
			ValueDesc list_desc(layer,"bones");
			ValueNode_StaticList::Handle list_node;
			list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
			for(int i=0;i<list_node->link_count();i++){
				ValueDesc value_desc(list_node,i,list_desc);
				if(value_desc.get_value_node()==node){
					active_bone = i;
					return i;
				}
			}
			return -1;
		}
		return -1;
	}
	return -1;
}