/* === S Y N F I G ========================================================= */
/*!	\file state_bone.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2010, 2011 Carlos López
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
#include "synfig/valuenodes/valuenode_bone.h"
#include "synfig/valuenodes/valuenode_staticlist.h"
#include "synfigapp/value_desc.h"

#include <gui/localization.h>

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
#define INDENTATION (6)

/* === G L O B A L S ======================================================= */

StateBone studio::state_bone;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateBone_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	bool prev_table_status;
	bool prev_workarea_layer_status_;

	int depth;
	Canvas::Handle canvas;

	Gtk::Menu menu;

	int active_bone;

	Point clickOrigin;

	// Toolbox settings
	synfigapp::Settings& settings;

	// holder of optons
	Gtk::Table options_table;

	// title
	Gtk::Label title_label;

	// layer name:
	Gtk::Label id_label;
	Gtk::HBox id_box;
	Gtk::Entry id_entry;

	// Origin bone width
	Gtk::Label bone_width_label;
	Widget_Distance bone_width_dist;
	
	// Tip bone width
	Gtk::Label tip_width_label;
	Widget_Distance tip_width_dist;

	Action::Handle createChild;

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

	Real get_tip_width() const {
		return tip_width_dist.get_value().get(
				Distance::SYSTEM_UNITS,
				get_canvas_view()->get_canvas()->rend_desc()
		);
	}
	void set_tip_width(Distance x){return tip_width_dist.set_value(x);}

	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_mouse_click_handler(const Smach::event& x);
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
	int find_bone(Point point,Layer_Skeleton::Handle layer)const;

	void load_settings();
	void save_settings();
	void reset();
	void increment_id();

	bool egress_on_selection_change;
	Smach::event_result event_layer_selection_changed_handler(const Smach::event& /*x*/)
	{
		if(egress_on_selection_change)
			throw &state_normal;
		return Smach::RESULT_OK;
	}

}; // END of class StateBone_Context

/* === M E T H O D S ======================================================= */

StateBone::StateBone() :
	Smach::state<StateBone_Context>("bone")
{
	insert(event_def(EVENT_LAYER_SELECTION_CHANGED,		&StateBone_Context::event_layer_selection_changed_handler));
	insert(event_def(EVENT_STOP,						&StateBone_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,						&StateBone_Context::event_refresh_handler));
	insert(event_def(EVENT_REFRESH_DUCKS,				&StateBone_Context::event_hijack));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,	&StateBone_Context::event_mouse_click_handler));
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

		if(settings.get_value("bone.id",value))
			set_id(value);
		else
			set_id(_("NewSkeleton"));

		if(settings.get_value("bone.bone_width",value) && value!="")
			set_bone_width(Distance(atof(value.c_str()),App::distance_system));
		else
			set_bone_width(Distance(6.6,App::distance_system)); // default width

		if(settings.get_value("bone.tip_width",value) && value!="")
			set_tip_width(Distance(atof(value.c_str()),App::distance_system));
		else
			set_tip_width(Distance(6.6,App::distance_system)); // default width
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

		settings.set_value("bone.id",get_id().c_str());
		settings.set_value("bone.bone_width",bone_width_dist.get_value().get_string());
		settings.set_value("bone.tip_width",tip_width_dist.get_value().get_string());
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
	set_bone_width(Distance(6.6,App::distance_system)); // default width
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
	createChild(Action::Handle(Action::create("ValueDescCreateChildBone"))),
	active_bone(-1)
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
	SPACING(id_gap,GAP);
	id_box.pack_start(id_label, Gtk::PACK_SHRINK);
	id_box.pack_start(*id_gap, Gtk::PACK_SHRINK);

	id_box.pack_start(id_entry);

	// 2, Bone width
	bone_width_label.set_label(_("Bone Origin Width:"));
	bone_width_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	bone_width_label.set_sensitive(true);

	bone_width_dist.set_digits(2);
	bone_width_dist.set_range(0,10000000);
	bone_width_dist.set_sensitive(true);

	// 3, Tip width
	tip_width_label.set_label(_("Bone Tip Width:"));
	tip_width_label.set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	tip_width_label.set_sensitive(true);

	tip_width_dist.set_digits(2);
	tip_width_dist.set_range(0,10000000);
	tip_width_dist.set_sensitive(true);
	load_settings();

	// pack all options to the options_table

	// 0, title
	options_table.attach(title_label,
						 0, 2, 0, 1, Gtk::FILL, Gtk::FILL, 0, 0
	);
	// 1, name
	options_table.attach(id_box,
						 0, 2, 1, 2, Gtk::FILL, Gtk::FILL, 0, 0
	);
	// 2, default bone width
	options_table.attach(bone_width_label,
						 0, 1, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
	);
	options_table.attach(bone_width_dist,
						 1, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
	);
	// 3, blend method
	options_table.attach(tip_width_label,
		0, 1, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	options_table.attach(tip_width_dist,
		1, 2, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::FILL, 0, 0
		);
	// fine-tune options layout
	options_table.set_border_width(GAP*2); // border width
	options_table.set_row_spacings(GAP); // row gap
	options_table.set_row_spacing(0, GAP*2); // the gap between first and second row.
	options_table.set_row_spacing(2, 1); // row gap between label and icon of layer type
	//options_table.set_row_spacing(10, 0); //// the final row using border width of table
	options_table.set_margin_bottom(0);
	options_table.show_all();

	refresh_tool_options();
	App::dialog_tool_options->present();

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	//ducks
	get_work_area()->set_type_mask(get_work_area()->get_type_mask()-Duck::TYPE_TANGENT-Duck::TYPE_WIDTH);
	get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);


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

	// Restore layer clicking
	get_work_area()->set_allow_layer_clicks(prev_workarea_layer_status_);

	// Enable the time bar
	get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();
	get_work_area()->set_active_bone_value_node(ValueNode::Handle());

	App::dock_toolbox->refresh();
}

Smach::event_result
StateBone_Context::event_stop_handler(const Smach::event &x)
{
	synfig::info("Skeleton Tool stop handler called");
	reset();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateBone_Context::event_refresh_handler(const Smach::event& x)
{
	synfig::info("Skeleton Tool refresh handler called");
	return Smach::RESULT_ACCEPT;
}



Smach::event_result
StateBone_Context::event_mouse_release_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

	Point releaseOrigin(get_work_area()->snap_point_to_grid(event.pos));
	info("Mouse release at: "+to_string(releaseOrigin[0])+" , "+to_string(releaseOrigin[1]));

	Layer::Handle layer = get_canvas_interface()->get_selection_manager()->get_selected_layer();
	Layer_Skeleton::Handle  skel_layer = etl::handle<Layer_Skeleton>::cast_dynamic(layer);

	Action::Handle addLayer(Action::LayerAdd::create());

	Duck::Handle duck(get_work_area()->find_duck(releaseOrigin,0.1));

	switch(event.button)
	{
		case BUTTON_LEFT:
		{
			if(!duck){ //! if the user was not modifying a duck
				if(skel_layer){ //!if selected layer is a Skeleton Layer
					createChild->set_param("canvas",skel_layer->get_canvas());
					ValueDesc list_desc(layer,"bones");
					int b = -1;
					if((clickOrigin-releaseOrigin).mag()<0.01)
						b=find_bone(clickOrigin,skel_layer);
					if(b!=-1){ //! if bone found around the release point --> set active bone
						active_bone=b;
						//info("Active Bone :"+ to_string(b));
						ValueNode_StaticList::Handle list_node;
						list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
						ValueDesc value_desc= ValueDesc(list_node,active_bone,list_desc);
						//cout<<"Active bone name : "<<value_desc.get_name()<<endl;
						get_work_area()->set_active_bone_value_node(value_desc.get_value_node());
					}else{
						ValueNode_StaticList::Handle list_node;
						list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
						//cout<<"Active Bone: "<<active_bone<<endl;
						if(active_bone!=-1){ //! if active bone is already set
							ValueDesc value_desc= ValueDesc(list_node,active_bone,list_desc);
							//cout<<"Active bone name : "<<value_desc.get_name()<<endl;
							ValueNode_Bone::Handle bone_node;
							if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
							{
								error("expected a ValueNode_Bone");
								assert(0);
							}
							get_work_area()->set_active_bone_value_node(value_desc.get_value_node());
							ValueDesc v_d = ValueDesc(bone_node,bone_node->get_link_index_from_name("origin"),value_desc);
							Real sx = bone_node->get_link("scalelx")->operator()(get_canvas()->get_time()).get(Real());
							Matrix matrix = value_desc.get_value(get_canvas()->get_time()).get(Bone()).get_animated_matrix();
							Real angle = atan2(matrix.axis(0)[1],matrix.axis(0)[0]);
							Real a =0;
							matrix = matrix.get_inverted();
							Point aOrigin = matrix.get_transformed(clickOrigin);
							aOrigin[0]/=sx;

							createChild->set_param("value_desc",Action::Param(v_d));
							createChild->set_param("origin",Action::Param(ValueBase(aOrigin)));
							createChild->set_param("width",Action::Param(ValueBase(get_bone_width())));
							createChild->set_param("tipwidth",Action::Param(ValueBase(get_tip_width())));
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
									get_work_area()->set_active_bone_value_node(value_desc.get_value_node());
								} catch (...) {
									info("Error performing action");
								}
							}
						}else{
							Action::Handle insertBone = Action::create("ValueNodeStaticListInsert");
							ValueDesc value_desc= ValueDesc(list_node,0,list_desc);
							ValueNode_Bone::Handle bone_node;
							if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
							{
								error("expected a ValueNode_Bone");
								assert(0);
							}
							ValueDesc v_d = ValueDesc(bone_node,bone_node->get_link_index_from_name("origin"),value_desc);
							createChild->set_param("value_desc",Action::Param(v_d));
							createChild->set_param("parent",true);
							createChild->set_param("origin",Action::Param(ValueBase(clickOrigin)));
							createChild->set_param("width",Action::Param(ValueBase(get_bone_width())));
							createChild->set_param("tipwidth",Action::Param(ValueBase(get_tip_width())));
							if((clickOrigin-releaseOrigin).mag()>=0.01) {
								createChild->set_param("scalelx", Action::Param(ValueBase((releaseOrigin - clickOrigin).mag())));
								createChild->set_param("angle", Action::Param(ValueBase((releaseOrigin - clickOrigin).angle())));
							}
							if(createChild->is_ready()){
								try{
									get_canvas_interface()->get_instance()->perform_action(createChild);
									value_desc= ValueDesc(list_node,0,list_desc);
									get_work_area()->set_active_bone_value_node(value_desc.get_value_node());
								} catch (...) {
									info("Error performing action");
								}
							}
						}

					}
				}else{ //! Creating empty layer as there's no active skeleton layer
					egress_on_selection_change=false;
					Layer::Handle new_skel= get_canvas_interface()->add_layer_to("skeleton",get_canvas());
					ValueDesc list_desc(new_skel,"bones");
					ValueNode_StaticList::Handle list_node;
					list_node=ValueNode_StaticList::Handle::cast_dynamic(list_desc.get_value_node());
					ValueDesc value_desc= ValueDesc(list_node,0,list_desc);
					get_work_area()->set_active_bone_value_node(value_desc.get_value_node());
					active_bone = 0;
					ValueNode_Bone::Handle bone_node;

					if (!(bone_node = ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node())))
					{
						error("expected a ValueNode_Bone");
						assert(0);
					}


					bone_node->set_link("origin",ValueNode_Const::create(clickOrigin));
					bone_node->set_link("width",ValueNode_Const::create(get_bone_width()));
					bone_node->set_link("tipwidth",ValueNode_Const::create(get_tip_width()));
					if((clickOrigin - releaseOrigin).mag() >= 0.001){
						bone_node->set_link("angle",ValueNode_Const::create((releaseOrigin-clickOrigin).angle()));
						bone_node->set_link("scalelx",ValueNode_Const::create((releaseOrigin-clickOrigin).mag()));
					}

					get_canvas_interface()->get_selection_manager()->clear_selected_layers();
					get_canvas_interface()->get_selection_manager()->set_selected_layer(new_skel);
					egress_on_selection_change=true;

					get_canvas_view()->queue_rebuild_ducks();
				}
			}
			return Smach::RESULT_ACCEPT;

		}

		default:
			return Smach::RESULT_OK;
	}
}

Smach::event_result
StateBone_Context::event_mouse_click_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	Point p(get_work_area()->snap_point_to_grid(event.pos));

	Layer::Handle layer = get_canvas_interface()->get_selection_manager()->get_selected_layer();
	Layer_Skeleton::Handle  skel_layer = etl::handle<Layer_Skeleton>::cast_dynamic(layer);
	info("Mouse click at: "+to_string(p[0])+" , "+to_string(p[1]));
	switch(event.button)
	{
		case BUTTON_LEFT:
		{
			clickOrigin = p;
			return Smach::RESULT_ACCEPT;
		}

		default:
			return Smach::RESULT_OK;
	}
}

int
StateBone_Context::find_bone(Point point,Layer_Skeleton::Handle layer)const
{
	//cout<<"Finding bone!"<<endl;
	ValueDesc list_desc(layer,"bones");
	ValueNode_StaticList::Handle list_node;
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
				cout<<close_line<<" <cl cr>"<<close_origin<<endl;
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