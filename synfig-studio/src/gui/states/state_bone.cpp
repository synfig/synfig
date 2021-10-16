/* === S Y N F I G ========================================================= */
/*!	\file state_bone.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2020 Aditya Abhiram J
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

#include <gui/states/state_bone.h>

#include <gtkmm/radiobutton.h>
#include <gtkmm/separatormenuitem.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/event_mouse.h>
#include <gui/localization.h>
#include <gui/widgets/widget_distance.h>
#include <gui/workarea.h>

#include <synfig/general.h>
#include <synfig/layers/layer_skeleton.h>
#include <synfig/layers/layer_skeletondeformation.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/valuenodes/valuenode_staticlist.h>

#include <synfigapp/actions/layeradd.h>
#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace studio;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

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

	//int depth;
	Canvas::Handle canvas;

	Duck::Handle  point2_duck,point1_duck;
	handle<Duckmatic::Bezier> bone_bezier;

	Gtk::Menu menu;

	synfig::ValueNode::Handle active_bone;
	enum SkeletonLayerType {SKELETON_TYPE, SKELETON_DEFORMATION_TYPE};
	SkeletonLayerType c_layer;
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
	Widget_Distance skel_bone_width_dist;
	Widget_Distance skel_deform_bone_width_dist;


	// Layer creation radio group
	Gtk::Label layer_label;
	Gtk::RadioButton::Group radiogroup;
	Gtk::RadioButton radiobutton_skel;
	Gtk::RadioButton radiobutton_skel_deform;
	Gtk::Button create_layer;

	void update_width_duck_status(SkeletonLayerType type);

	void set_active_bone(ValueNode::Handle value_node);

public:

	synfig::String get_id() const { return id_entry.get_text();}
	void set_id(const synfig::String& x){ return id_entry.set_text(x);}

	Real get_skel_bone_width() const {
		return skel_bone_width_dist.get_value().get(
				Distance::SYSTEM_UNITS,
				get_canvas_view()->get_canvas()->rend_desc()
		);
	}
	void set_skel_bone_width(Distance x){return skel_bone_width_dist.set_value(x);}

	Real get_skel_deform_bone_width() const {
		return skel_deform_bone_width_dist.get_value().get(
				Distance::SYSTEM_UNITS,
				get_canvas_view()->get_canvas()->rend_desc()
		);
	}
	void set_skel_deform_bone_width(Distance x){return skel_deform_bone_width_dist.set_value(x);}

	Real get_bone_width() const{
		if(skel_bone_width_dist.is_visible()){
			return get_skel_bone_width();
		}else if(skel_deform_bone_width_dist.is_visible()){
			return get_skel_deform_bone_width();
		}else{
			return DEFAULT_WIDTH;
		}
	}

	void update_layer(){
		save_settings();
		if(c_layer==SKELETON_TYPE)
			c_layer=SKELETON_DEFORMATION_TYPE;
		else
			c_layer=SKELETON_TYPE;
		update_tool_options(c_layer);
		load_settings();
	}

	void make_layer();
	void update_tool_options(SkeletonLayerType type);

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
	int find_bone_index(SkeletonLayerType type, ValueNode_StaticList::Handle list, ValueNode::Handle bone) const;
	ValueNode_Bone::Handle find_bone(Point point, Layer::Handle layer) const;
	void _on_signal_change_active_bone(ValueNode::Handle node);
	void _on_signal_value_desc_set(ValueDesc value_desc,ValueBase value);

	void load_settings();
	void save_settings();
	void reset();
	void increment_id();

	bool egress_on_selection_change;
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& /*x*/);
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
		if(c_layer==SKELETON_TYPE){
			set_id(settings.get_value("bone.skel_id", _("NewSkeleton")));
		}else{
			set_id(settings.get_value("bone.skel_deform_id", _("NewSkeletonDeformation")));
		}

		set_skel_bone_width(Distance(
							settings.get_value("bone.skel_bone_width", DEFAULT_WIDTH),
							Distance::SYSTEM_UNITS)
						);

		set_skel_deform_bone_width(Distance(
							settings.get_value("bone.skel_deform_bone_width", DEFAULT_WIDTH),
							Distance::SYSTEM_UNITS)
						);
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
		if(c_layer==SKELETON_TYPE)
			settings.set_value("bone.skel_id",get_id().c_str());
		else
			settings.set_value("bone.skel_deform_id",get_id().c_str());

		settings.set_value("bone.skel_bone_width",skel_bone_width_dist.get_value().get_string());
		settings.set_value("bone.skel_deform_bone_width",skel_deform_bone_width_dist.get_value().get_string());
	}
	catch (...)
	{
		synfig::warning("State Bone: Caught exception when attempting to save settings.");
	}
}

void
StateBone_Context::reset()
{
	active_bone = nullptr;
	set_id(_("NewSkeleton"));
	set_skel_bone_width(Distance(DEFAULT_WIDTH,Distance::SYSTEM_UNITS)); // default width
	set_skel_deform_bone_width(Distance(DEFAULT_WIDTH,Distance::SYSTEM_UNITS)); // default width
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
	//depth(-1),
	active_bone(get_work_area()->get_active_bone_value_node()),
	c_layer(SKELETON_TYPE),
	drawing(false),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	radiobutton_skel(radiogroup,_("Skeleton Layer")),
	radiobutton_skel_deform(radiogroup,_("Skeleton Deform Layer"))
	
{
	egress_on_selection_change=true;

	get_canvas_interface()->set_state("bone");

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
	skel_bone_width_dist.set_digits(2);
	skel_bone_width_dist.set_range(0,10000000);
	skel_bone_width_dist.set_sensitive(true);
	skel_deform_bone_width_dist.set_digits(2);
	skel_deform_bone_width_dist.set_range(0,10000000);
	skel_deform_bone_width_dist.set_sensitive(true);

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
	options_table.attach(skel_bone_width_dist,2, 2,3,1);
	options_table.attach(skel_deform_bone_width_dist,2, 2,3,1);
	// 3, Layer choice
	options_table.attach(layer_label,0, 4,1,1);
	options_table.attach(radiobutton_skel,0, 5,2,1);
	options_table.attach(radiobutton_skel_deform,2, 5,2,1);
	options_table.attach(create_layer,2,6,2,1);

	create_layer.signal_clicked().connect(sigc::mem_fun(*this,&StateBone_Context::make_layer));
	radiobutton_skel.signal_toggled().connect(sigc::mem_fun(*this,&StateBone_Context::update_layer));

	// fine-tune options layout
	options_table.set_border_width(GAP*2); // border width
	options_table.set_row_spacing(GAP); // row gap
	options_table.set_margin_bottom(0);
	options_table.show_all();

	skel_deform_bone_width_dist.hide();

	refresh_tool_options();
	App::dialog_tool_options->present();

	// remembering previous type mask
	prev_type_mask = get_work_area()->get_type_mask();

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// Turn on Active Bone rendering
	int cnt =get_canvas_interface()->get_selection_manager()->get_selected_layer_count();
	if(cnt<=1){
		Layer::Handle layer = get_canvas_interface()->get_selection_manager()->get_selected_layer();
		if((Layer_Skeleton::Handle::cast_dynamic(layer) || Layer_SkeletonDeformation::Handle::cast_dynamic(layer)) || cnt==0)
			get_work_area()->set_active_bone_display(true);
	}

	//ducks
	Layer::Handle layer = get_canvas_interface()->get_selection_manager()->get_selected_layer();

	if(Layer_SkeletonDeformation::Handle::cast_dynamic(layer)){
		get_work_area()->set_type_mask(get_work_area()->get_type_mask() - (Duck::TYPE_TANGENT | Duck::TYPE_WIDTH));
		get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);
		layer->disable();
		get_canvas_interface()->signal_layer_status_changed()(layer,false);
		update_tool_options(SKELETON_DEFORMATION_TYPE);
	}else{
		get_work_area()->set_type_mask(get_work_area()->get_type_mask()-Duck::TYPE_TANGENT-Duck::TYPE_WIDTH);
		get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);
		update_tool_options(SKELETON_TYPE);
	}


	//signals
	get_canvas_interface()->signal_active_bone_changed().connect(sigc::mem_fun(*this,&studio::StateBone_Context::_on_signal_change_active_bone));
	get_canvas_interface()->signal_value_desc_set().connect(sigc::mem_fun(*this,&studio::StateBone_Context::_on_signal_value_desc_set));

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

void
StateBone_Context::update_tool_options(StateBone_Context::SkeletonLayerType type) {
	if(type==SKELETON_TYPE){
		if(!skel_bone_width_dist.is_visible()){
			skel_bone_width_dist.show();
			skel_deform_bone_width_dist.hide();
		}
	}else if(type==SKELETON_DEFORMATION_TYPE){
		if(!skel_deform_bone_width_dist.is_visible()){
			skel_deform_bone_width_dist.show();
			skel_bone_width_dist.hide();
		}
	}
}

Smach::event_result
StateBone_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateBone_Context::~StateBone_Context()
{
	get_canvas_interface()->set_state("");

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
	Layer_Skeleton::Handle skel_layer = Layer_Skeleton::Handle::cast_dynamic(layer);
	Layer_SkeletonDeformation::Handle deform_layer = Layer_SkeletonDeformation::Handle::cast_dynamic(layer);
	const synfig::TransformStack& transform(get_work_area()->get_curr_transform_stack());

	Action::Handle createChild(Action::Handle(Action::create("ValueDescCreateChildBone")));
	createChild->set_param("canvas",get_canvas());
	createChild->set_param("canvas_interface",get_canvas_interface());
	createChild->set_param("highlight",true);

	if(drawing){
		if (point1_duck)
			get_work_area()->erase_duck(point1_duck);
		if (point2_duck)
			get_work_area()->erase_duck(point2_duck);
		if (bone_bezier)
			get_work_area()->erase_bezier(bone_bezier);
		get_canvas_view()->rebuild_ducks();
	}

	switch(event.button)
	{
		case BUTTON_LEFT:
		{
			clickOrigin = transform.unperform(clickOrigin);
			releaseOrigin = transform.unperform(releaseOrigin);
			if(drawing){ // if the user was not modifying a duck
				if(skel_layer || deform_layer){ // if selected layer is a Skeleton Layer or a Skeleton Deformation Layer
					update_tool_options(skel_layer? SKELETON_TYPE : SKELETON_DEFORMATION_TYPE);
					update_width_duck_status(skel_layer? SKELETON_TYPE : SKELETON_DEFORMATION_TYPE);

					createChild->set_param("canvas",layer->get_canvas());
					ValueDesc list_desc(layer,"bones");

					ValueNode::Handle b = nullptr;
					if((clickOrigin-releaseOrigin).mag()<0.01)
						b=find_bone(clickOrigin,layer);

					if(b){ // if bone found around the release point, then set it as active bone
						ValueNode::Handle bone_value_node = b;
						if (deform_layer) {
							if (ValueNode_Composite::Handle comp = ValueNode_Composite::Handle::cast_dynamic(b))
								bone_value_node = comp->get_link("first");
						}
						set_active_bone(bone_value_node);
					}
					else{ // create a child bone
						ValueNode_StaticList::Handle list_node;
						list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
						int item_index = find_bone_index(skel_layer? SKELETON_TYPE : SKELETON_DEFORMATION_TYPE, list_node, active_bone);
						ValueDesc value_desc = ValueDesc(list_node,item_index,list_desc);
						if(active_bone && item_index >= 0 && !list_node->list.empty()){ // if active bone is already set
							ValueNode_Bone::Handle bone_node = ValueNode_Bone::Handle::cast_dynamic(active_bone);
							if (deform_layer) {
								if (ValueNode_Composite::Handle comp = ValueNode_Composite::Handle::cast_dynamic(active_bone)) {
									value_desc = ValueDesc(comp,comp->get_link_index_from_name("first"),value_desc);
									bone_node = ValueNode_Bone::Handle::cast_dynamic(comp->get_link("first"));
								} else if ((comp = ValueNode_Composite::Handle::cast_dynamic(list_node->get_link(item_index)))) {
									value_desc = ValueDesc(comp, comp->get_link_index_from_name("first"),value_desc);
								} else {
									get_canvas_interface()->get_ui_interface()->error(_("Expected a ValueNode_Composite with a BonePair"));
									assert(0);
									return Smach::RESULT_ERROR;
								}
							}

							if (!bone_node)
							{
								error("expected a ValueNode_Bone");
								drawing = false;
								get_canvas_interface()->get_ui_interface()->error(_("Expected a ValueNode_Bone"));
								assert(0);
								return Smach::RESULT_ERROR;
							}
							ValueDesc v_d = ValueDesc(bone_node,bone_node->get_link_index_from_name("origin"),value_desc);
							Real sx = bone_node->get_link("scalelx")->operator()(get_canvas()->get_time()).get(Real());
							Matrix matrix = (*bone_node)(get_canvas()->get_time()).get(Bone()).get_animated_matrix();
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
						} else {
							Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Add Bone"));

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

							ValueNode::Handle new_active_bone_node;
							if (skel_layer) {
								new_active_bone_node = ValueNode_Bone::create(bone,get_canvas());
								action->set_param("item",new_active_bone_node);
							} else {
								ValueNode_Composite::Handle bone_pair = ValueNode_Composite::create(std::pair<Bone,Bone>(bone,bone),get_canvas());
								new_active_bone_node = bone_pair->get_link("first");
								action->set_param("item",ValueNode::Handle::cast_dynamic(bone_pair));
							}
							action->set_param("value_desc",ValueDesc(list_node,0));

							if(action->is_ready()){
								try{
									get_canvas_interface()->get_instance()->perform_action(action);
								} catch (...) {
									info("Error performing action");
								}
							}

							set_active_bone(new_active_bone_node);
						}

					}
				}
				else
				{ // Creating empty layer as there's no active skeleton layer of any type
					Action::PassiveGrouper group(get_canvas_interface()->get_instance().get(),_("Create Skeleton"));
					egress_on_selection_change=false;
					if(c_layer==SKELETON_TYPE)
						get_canvas_view()->add_layer("skeleton");
					else if(c_layer==SKELETON_DEFORMATION_TYPE)
						get_canvas_view()->add_layer("skeleton_deformation");
					Layer::Handle new_skel= get_canvas_interface()->get_selection_manager()->get_selected_layer();
					new_skel->set_param("name",get_id().c_str());
					new_skel->set_description(get_id());
					ValueDesc list_desc(new_skel,"bones");
					ValueNode_StaticList::Handle list_node;
					list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
					ValueDesc value_desc= ValueDesc(list_node,0,list_desc);
					ValueNode_Bone::Handle bone_node;
					update_tool_options(c_layer);
					update_width_duck_status(c_layer);

					if(c_layer==SKELETON_TYPE){
						set_active_bone(value_desc.get_value_node());
						if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
						{
							error("expected a ValueNode_Bone");
							assert(0);
							return Smach::RESULT_ERROR;
						}
					}else if(c_layer==SKELETON_DEFORMATION_TYPE){
						new_skel->disable();

						ValueNode_Composite::Handle comp = ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node());

						if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(comp->get_link("first"))))
						{
							error("expected a ValueNode_Bone");
							assert(0);
							return Smach::RESULT_ERROR;
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
						set_active_bone(comp->get_link("first"));
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
			else{ // user clicked on a duck. Is it a bone?
				Duck::Handle duck(get_work_area()->find_duck(releaseOrigin,0.1));
				if(duck){
					if(duck->get_value_desc().is_parent_desc_declared()) {
						ValueNode::Handle parent = duck->get_value_desc().get_parent_desc().get_value_node();
						if (ValueNode_Bone::Handle::cast_dynamic(parent)){
							set_active_bone(parent);
						}
					}
				}
			}
			return Smach::RESULT_ACCEPT;
		}
		default:
			return Smach::RESULT_OK;
	}
}

Smach::event_result
StateBone_Context::event_layer_selection_changed_handler(const Smach::event& /*x*/)
{
	int cnt = get_canvas_interface()->get_selection_manager()->get_selected_layer_count();
	Layer::Handle layer = get_canvas_interface()->get_selection_manager()->get_selected_layer();
	Layer_Skeleton::Handle skel_layer = Layer_Skeleton::Handle::cast_dynamic(layer);
	Layer_SkeletonDeformation::Handle deform_layer = Layer_SkeletonDeformation::Handle::cast_dynamic(layer);

	if(cnt<=1){
		if((skel_layer || deform_layer) || cnt==0)
			get_work_area()->set_active_bone_display(true);
	}


	if(skel_layer){
		set_id(settings.get_value("bone.skel_id", _("NewSkeleton")));
		update_tool_options(SKELETON_TYPE);
		get_work_area()->set_type_mask(get_work_area()->get_type_mask()-Duck::TYPE_TANGENT-Duck::TYPE_WIDTH);
	}else if(deform_layer){
		set_id(settings.get_value("bone.skel_deform_id", _("NewSkeletonDeformation")));
		update_tool_options(SKELETON_DEFORMATION_TYPE);
		get_work_area()->set_type_mask(get_work_area()->get_type_mask() - (Duck::TYPE_TANGENT | Duck::TYPE_WIDTH));
		layer->disable();
		get_canvas_interface()->signal_layer_status_changed()(layer,false);
	}else{
		get_work_area()->set_type_mask(get_work_area()->get_type_mask()-Duck::TYPE_TANGENT-Duck::TYPE_WIDTH);
		get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);
	}

	get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);
	if(egress_on_selection_change){
			set_active_bone(nullptr);
	}
	get_work_area()->queue_draw();
	get_canvas_view()->queue_rebuild_ducks();

	return Smach::RESULT_OK;
}

int
StateBone_Context::find_bone_index(SkeletonLayerType type, ValueNode_StaticList::Handle list, ValueNode::Handle bone) const
{
	const int num = list->link_count();
	if (type == SKELETON_TYPE) {
		for (int i = 0; i < num; i++) {
			if (list->get_link(i) == bone)
				return i;
		}
	} else if (type == SKELETON_DEFORMATION_TYPE) {
		for (int i = 0; i < num; i++) {
			ValueNode::LooseHandle vn = list->get_link(i);
			if (auto composite = ValueNode_Composite::Handle::cast_dynamic(vn)) {
				if (composite == bone)
					return i;
				if (composite->get_link("first") == bone)
					return i;
				if (composite->get_link("second") == bone)
					return i;
			}
		}
	}
	return -1;
}

ValueNode_Bone::Handle
StateBone_Context::find_bone(Point point,Layer::Handle layer) const
{
	bool is_skeleton_deform_layer = false;
	std::vector<Bone> bone_list;
	ValueDesc list_desc(layer,"bones");
	if(Layer_Skeleton::Handle::cast_dynamic(layer)){
		bone_list = (*list_desc.get_value_node())(get_canvas()->get_time()).get_list_of(Bone());
	} else if (Layer_SkeletonDeformation::Handle::cast_dynamic(layer)) {
		is_skeleton_deform_layer = true;
		std::vector<std::pair<Bone,Bone>> bone_pair_list = (*list_desc.get_value_node())(get_canvas()->get_time()).get_list_of(std::pair<Bone,Bone>());
		for (const auto& item : bone_pair_list)
			bone_list.push_back(item.first);
	} else {
		return nullptr;
	}

	Real close_line(10000000),close_origin(10000000);
	int ret = -1;
	for(auto iter=bone_list.begin();iter!=bone_list.end();++iter){
		Matrix m=iter->get_animated_matrix();
		Point orig = m.get_transformed(Vector(0,0));
		Angle angle = Angle::rad(atan2(m.axis(0)[1],m.axis(0)[0]));
		Real orig_dist((point-orig).mag());
		Real dist=std::fabs(orig_dist*Angle::sin((point-orig).angle()-angle).get());
		Real length = iter->get_length()*iter->get_scalelx();
		if(Angle::cos((point-orig).angle()-angle).get()>0 && orig_dist<=length){
			dist = std::fabs(dist);
			if(dist<close_line){
				close_line=dist;
				close_origin=orig_dist;
				ret = iter-bone_list.begin();
			}else if(fabs(dist-close_line)<0.0000001 && close_line!= 10000000){
				if(orig_dist<close_origin){
					close_origin=orig_dist;
					ret = iter-bone_list.begin();
				}
			}
		}
	}
	if(std::fabs(close_line)<=0.2){
		if (ret >=0 && ret < bone_list.size()) {
			ValueNode_StaticList::Handle list_node;
			list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
			if (is_skeleton_deform_layer)
				return ValueNode_Bone::Handle::cast_dynamic(ValueNode_Composite::Handle::cast_dynamic(list_node->get_link(ret))->get_link("first"));
			else
				return ValueNode_Bone::Handle::cast_dynamic(list_node->get_link(ret));
		}
	}

	return nullptr;
}

void
StateBone_Context::update_width_duck_status(SkeletonLayerType type)
{
	bool is_width_duck_currently_on(get_work_area()->get_type_mask()&Duck::TYPE_WIDTH);
	if ((type == SKELETON_TYPE && is_width_duck_currently_on)
		|| (type == SKELETON_DEFORMATION_TYPE && !is_width_duck_currently_on)) {
		get_canvas_view()->toggle_duck_mask(Duck::TYPE_WIDTH);
	}
}

void
StateBone_Context::set_active_bone(ValueNode::Handle bone_value_node)
{
	if (active_bone == bone_value_node)
		return;
	Action::Handle setActiveBone(Action::Handle(Action::create("ValueNodeSetActiveBone")));
	setActiveBone->set_param("canvas", get_canvas());
	setActiveBone->set_param("canvas_interface", get_canvas_interface());
	setActiveBone->set_param("prev_active_bone_node", get_work_area()->get_active_bone_value_node());
	setActiveBone->set_param("active_bone_node", bone_value_node);

	if(setActiveBone->is_ready()){
		try{
			get_canvas_interface()->get_instance()->perform_action(setActiveBone);
		} catch (...) {
			get_canvas_interface()->get_ui_interface()->error(_("Error setting the new active bone"));
		}
	}
}

void
StateBone_Context::make_layer(){
	egress_on_selection_change=false;

	update_tool_options(c_layer);
	update_width_duck_status(c_layer);
	if(c_layer==SKELETON_TYPE){
		get_canvas_view()->add_layer("skeleton");
	}
	else if(c_layer==SKELETON_DEFORMATION_TYPE){
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

	if(c_layer==SKELETON_TYPE){
		if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
		{
			error("expected a ValueNode_Bone");
			assert(0);
			return;
		}
		bone_node->set_link("width",ValueNode_Const::create(get_bone_width()));
		bone_node->set_link("tipwidth",ValueNode_Const::create(get_bone_width()));
	}else if(c_layer==SKELETON_DEFORMATION_TYPE){
		new_skel->disable();

		ValueNode_Composite::Handle comp = ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node());
		value_desc =  ValueDesc(comp,comp->get_link_index_from_name("second"),value_desc);
		if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
		{
			error("expected a ValueNode_Bone");
			assert(0);
			return;
		}
		bone_node->set_link("width",ValueNode_Const::create(get_bone_width()));
		bone_node->set_link("tipwidth",ValueNode_Const::create(get_bone_width()));
		value_desc =  ValueDesc(comp,comp->get_link_index_from_name("first"),value_desc);
		if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
		{
			error("expected a ValueNode_Bone");
			assert(0);
			return;
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
	active_bone = nullptr;
	if(ValueNode_Bone::Handle bone = ValueNode_Bone::Handle::cast_dynamic(node)){
		Layer::Handle layer = get_canvas_interface()->get_selection_manager()->get_selected_layer();
		Layer_Skeleton::Handle skel_layer = Layer_Skeleton::Handle::cast_dynamic(layer);
		Layer_SkeletonDeformation::Handle deform_layer = Layer_SkeletonDeformation::Handle::cast_dynamic(layer);

		ValueDesc list_desc(layer,"bones");
		ValueNode_StaticList::Handle list_node;
		list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
		if(list_node){
			for(int i=0;i<list_node->link_count();i++){
				ValueNode::Handle value_node = list_node->get_link(i);
				if(skel_layer){
					if(value_node==node){
						active_bone = node;
						return;
					}
				}else if(deform_layer){
					ValueNode_Composite::Handle v_node = ValueNode_Composite::Handle::cast_dynamic(value_node);
					if(v_node && (v_node->get_link("first")==node || v_node->get_link("second")==node)){
						active_bone = v_node;
						return;
					}
				}
			}
		}
	}
}

void
StateBone_Context::_on_signal_value_desc_set(ValueDesc value_desc,ValueBase value) {
	if (ValueNode_Bone::Handle bone_valuenode = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())) {
		const int index = value_desc.get_index();
		static const int width_index = bone_valuenode->get_link_index_from_name("width");
		static const int tip_width_index = bone_valuenode->get_link_index_from_name("tipwidth");
		if(index==tip_width_index || index==width_index){
			if(skel_bone_width_dist.is_visible())
				set_skel_bone_width(Distance(value.get(Real()),synfig::Distance::SYSTEM_UNITS));
			if(skel_deform_bone_width_dist.is_visible())
				set_skel_deform_bone_width(Distance(value.get(Real()),synfig::Distance::SYSTEM_UNITS));
		}
	}
}
