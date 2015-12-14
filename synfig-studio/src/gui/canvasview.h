/* === S Y N F I G ========================================================= */
/*!	\file canvasview.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009 Carlos López
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

#ifndef __SYNFIG_STUDIO_GTKMM_CANVASVIEW_H
#define __SYNFIG_STUDIO_GTKMM_CANVASVIEW_H

/* === H E A D E R S ======================================================= */

#include <glibmm/dispatcher.h>

#include <gtkmm/window.h>
#include <gtkmm/image.h>
#include <gtkmm/tooltip.h>
#include <gtkmm/table.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/button.h>
#include <gtkmm/menu.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/notebook.h>
#include <gdkmm/device.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/alignment.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/toolitem.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/toggletoolbutton.h>
#include <gtkmm/separatortoolitem.h>


#include <synfigapp/canvasinterface.h>
#include <synfigapp/selectionmanager.h>

#include <ETL/clock>

#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/string.h>
#include <synfig/time.h>

#include "instance.h"
#include "dialogs/canvasproperties.h"
#include "dialogs/canvasoptions.h"
#include "render.h"
#include "cellrenderer/cellrenderer_timetrack.h"
#include "app.h"

#include "trees/layertreestore.h"
#include "trees/layertree.h"
#include "trees/childrentreestore.h"
#include "trees/childrentree.h"
#include "trees/keyframetreestore.h"
#include "trees/keyframetree.h"

#include "dialogs/dialog_waypoint.h"
#include "dialogs/dialog_keyframe.h"
#include "dials/framedial.h"
#include "dials/jackdial.h"
#include "dials/toggleducksdial.h"
#include "dials/resolutiondial.h"
#include "widgets/widget_keyframe_list.h"

#include "duckmatic.h"
#include <gtkmm/scale.h>

#include <gtkmm/uimanager.h>

#include "smach.h"

#include <memory>
#include <set>
#include <map>
#include <gtkmm/toggleaction.h>
#include <gtkmm/radioaction.h>
#include <synfig/rect.h>

#include "adjust_window.h"

#include <synfig/transform.h>
#include <synfig/soundprocessor.h>

#include "docks/dockable.h"

#ifdef WITH_JACK
#include <jack/jack.h>
#include <jack/transport.h>
#endif

#ifndef ONION_SKIN_PAST
	#define ONION_SKIN_PAST 10
#endif

#ifndef ONION_SKIN_FUTURE
	#define ONION_SKIN_FUTURE 10
#endif

/* === M A C R O S ========================================================= */

#ifndef DEBUGPOINT_CLASS
#if	_DEBUG
#define DEBUGPOINT_CLASS(x)		struct debugpointclass_ ## x { debugpointclass_ ## x () { DEBUGPOINT(); } ~debugpointclass_ ## x () { DEBUGPOINT(); } } badfthguae_ ## x ;
#else
#define DEBUGPOINT_CLASS(x)
#endif
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
	class TransformStack;
}

namespace studio {

class CanvasViewUIInterface;
class CanvasViewSelectionManager;

class CellRenderer_TimeTrack;
class CellRenderer_ValueBase;
class UniversalScrubber;
class WorkArea;

class Duckmatic;

class Widget_Enum;

class Preview;
struct PreviewInfo;
class AudioContainer;

class Widget_Sound;
class Widget_Timeslider;
class Widget_Time;

class Dialog_SoundSelect;
class Dialog_Preview;

class Dock_Layers;
class Dock_Children;
class Dock_Keyframes;

/*!	\class studio::CanvasView
**	\brief \writeme
**
**	\writeme
*/
class CanvasView : public Dockable, public etl::shared_object
{
	friend class UniversalScrubber;
	friend class Dock_Layers;
	friend class Dock_Children;
	friend class Dock_Keyframes;

	friend class CanvasViewUIInterface;
	friend class CanvasViewSelectionManager;

	friend class Duckmatic;

	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	typedef etl::handle<CanvasView> Handle;

	typedef etl::handle<const CanvasView> ConstHandle;

	typedef etl::loose_handle<CanvasView> LooseHandle;

	typedef LayerTreeStore::Model LayerTreeModel;

	typedef ChildrenTreeStore::Model ChildrenTreeModel;

	//! Create an instance of this class whenever doing a longer task.
	/*! Make sure that you check the bool value of this class occasionally
	**	to make sure the action has not been canceled. */
	class IsWorking
	{
		CanvasView &canvas_view_;

	public:
		IsWorking(CanvasView &canvas_view_);
		~IsWorking();
		operator bool()const;
	};
	friend class IsWorking;

	class ActivationIndex {
	private:
		static ActivationIndex last__;
	public:
		long long int activation_index;
		long long int creation_index;

		void create() { creation_index = ++last__.creation_index; }
		void activate() { activation_index = ++last__.activation_index; }

		explicit ActivationIndex(bool create = false): activation_index(0), creation_index(0)
		{
			if (create) this->create();
		}

		bool operator < (const ActivationIndex &other) const
		{
			if (activation_index < other.activation_index) return true;
			if (other.activation_index < activation_index) return false;
			return creation_index < other.creation_index;
		}
	};

	typedef synfigapp::CanvasInterface::Mode Mode;

	void set_grid_snap_toggle(bool flag) { grid_snap_toggle->set_active(flag); }
	void set_grid_show_toggle(bool flag) { grid_show_toggle->set_active(flag); }
	void set_onion_skin_toggle(bool flag) { onion_skin_toggle->set_active(flag); }

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

public:
	std::auto_ptr<WorkArea> work_area;

	WorkArea* get_work_area() { return work_area.get(); }

private:
	synfig::SoundProcessor soundProcessor;

	ActivationIndex activation_index_;

	synfig::Rect bbox;

	// DEBUGPOINT_CLASS(1);

	//! State Machine
	Smach smach_;

	// DEBUGPOINT_CLASS(2);

	etl::loose_handle<Instance> instance_;
	etl::handle<synfigapp::CanvasInterface> canvas_interface_;
	synfig::ContextParams context_params_;

	// DEBUGPOINT_CLASS(3);

	//! Sound and information to play it
	etl::handle<AudioContainer>		audio;
	studio::Widget_Sound			*disp_audio; //should this be put into thing too?

	sigc::connection				playcon;
	sigc::connection				stopcon;

	std::auto_ptr<UniversalScrubber> universal_scrubber;

	// DEBUGPOINT_CLASS(4);

	//! TreeModel for the layers
	LayerTreeModel layer_tree_model;

	//! TreeModel for the the children
	ChildrenTreeModel children_tree_model;

	//Glib::RefPtr<LayerTreeStore> layer_tree_store_;

	//Glib::RefPtr<ChildrenTreeStore> children_tree_store_;

	//Glib::RefPtr<KeyframeTreeStore> keyframe_tree_store_;

	// DEBUGPOINT_CLASS(5);

	//std::map<synfig::String,Glib::RefPtr<Gtk::TreeModel> > tree_model_book_;
	std::map<synfig::String,Glib::RefPtr<Glib::ObjectBase> > ref_obj_book_;
	std::map<synfig::String,Gtk::Widget*> ext_widget_book_;

	//! The time adjustment's scope is defined by the time_window adjustment
	Glib::RefPtr<Gtk::Adjustment> time_adjustment_;

	//! The time_window adjustment governs the position of the time window on the whole time line
	//Glib::RefPtr<Gtk::Adjustment> time_window_adjustment_;
	Glib::RefPtr<studio::Adjust_Window> time_window_adjustment_;

	LayerTree *layer_tree;

	ChildrenTree *children_tree;

	KeyframeTree *keyframe_tree;

	Gtk::Widget *keyframe_tab_child;

	Gtk::ProgressBar *progressbar;
	Gtk::Statusbar *statusbar;

	Gtk::TreeRow children_canvas_row;
	Gtk::TreeRow children_valuenode_row;

	Gtk::Button *closebutton;
	Gtk::Button *stopbutton;
	Gtk::ToolButton *refreshbutton;
	Gtk::Button *treetogglebutton;  // not used
	Gtk::Notebook *notebook; // not used
	Gtk::Table *timebar;
	Gtk::Toolbar *displaybar;
	Widget_Enum *widget_interpolation;
	Gtk::ScrolledWindow *widget_interpolation_scroll;
	Gtk::ToggleButton *animatebutton;
	Gtk::ToggleButton *timetrackbutton;
	Gtk::VBox *timetrack;
	Gtk::Button *keyframebutton;
	Gtk::ToggleButton *pastkeyframebutton;
	Gtk::ToggleButton *futurekeyframebutton;
	bool toggling_animate_mode_;
	FrameDial *framedial;
	JackDial *jackdial;
	Gtk::ToggleButton *jackbutton;
	Widget_Time *offset_widget;
	ToggleDucksDial toggleducksdial;
	bool toggling_ducks_;
	ResolutionDial resolutiondial;
	bool changing_resolution_;
	Glib::RefPtr<Gtk::Adjustment> quality_adjustment_;
	Gtk::SpinButton *quality_spin;
	Glib::RefPtr<Gtk::Adjustment> future_onion_adjustment_;
	Glib::RefPtr<Gtk::Adjustment> past_onion_adjustment_;
	Gtk::SpinButton *past_onion_spin;
	Gtk::SpinButton *future_onion_spin;
	bool updating_quality_;
	Gtk::ToggleToolButton *show_grid;
	Gtk::ToggleToolButton *snap_grid;
	Gtk::ToggleToolButton *onion_skin;
	Gtk::ToolButton *render_options_button;
	Gtk::ToolButton *preview_options_button;
	bool toggling_show_grid;
	bool toggling_snap_grid;
	bool toggling_onion_skin;
	//! Shows current time and allows edition
	Widget_Time *current_time_widget;
	void on_current_time_widget_changed();

	//! Time slider class. Same than the Time track panel
	std::auto_ptr<Widget_Timeslider> timeslider;

	//!Keyframe list slider
	std::auto_ptr<Widget_Keyframe_List> widget_kf_list;

	std::list<sigc::connection> duck_changed_connections;

/*	DEBUGPOINT_CLASS(8);

	Gtk::Menu duckmaskmenu;
	DEBUGPOINT_CLASS(77);
	Gtk::Menu qualitymenu;
	DEBUGPOINT_CLASS(6);

	Gtk::Menu filemenu;
	DEBUGPOINT_CLASS(777);
	Gtk::Menu editmenu;
	DEBUGPOINT_CLASS(71);
	Gtk::Menu canvasmenu;
	DEBUGPOINT_CLASS(73);
public:
	Gtk::Menu layermenu;
private:
	DEBUGPOINT_CLASS(74);
	Gtk::Menu newlayermenu;
	DEBUGPOINT_CLASS(76);
	Gtk::Menu viewmenu;

	DEBUGPOINT_CLASS(99);
	Gtk::Menu keyframemenu;

	Gtk::Menu parammenu;
	DEBUGPOINT_CLASS(9);
	Gtk::Menu trackmenu;
	DEBUGPOINT_CLASS(7);

	Gtk::CheckMenuItem* duck_mask_position;
	Gtk::CheckMenuItem* duck_mask_vertex;
	Gtk::CheckMenuItem* duck_mask_tangent;
	Gtk::CheckMenuItem* duck_mask_radius;
	Gtk::CheckMenuItem* duck_mask_width;
	Gtk::CheckMenuItem* duck_mask_angle;
*/
	//! Menu members
	Gtk::Menu parammenu;

	Gtk::UIManager::ui_merge_id merge_id_popup_;
	Gtk::UIManager::ui_merge_id merge_id_toolbar_;

	Glib::RefPtr<Gtk::ToggleAction> grid_snap_toggle;
	Glib::RefPtr<Gtk::ToggleAction> grid_show_toggle;
	Glib::RefPtr<Gtk::ToggleAction> onion_skin_toggle;

	Gtk::RadioButtonGroup quality_group;
	Gtk::RadioButtonGroup low_res_pixel_size_group;

	Glib::RefPtr<Gtk::ActionGroup> action_group;

	etl::handle<synfigapp::UIInterface> ui_interface_;
	etl::handle<synfigapp::SelectionManager> selection_manager_;

	bool is_playing_;
	sigc::connection playing_connection;
	etl::clock playing_timer;
	synfig::Time playing_time;

	sigc::signal<void> signal_deleted_;

	bool rebuild_ducks_queued;
	sigc::connection queue_rebuild_ducks_connection;

	bool jack_enabled;
	bool jack_actual_enabled;
	int jack_locks;
	bool jack_enabled_in_preview;
#ifdef WITH_JACK
	Glib::Dispatcher jack_dispatcher;
	jack_client_t *jack_client;
	bool jack_synchronizing;
	bool jack_is_playing;
	synfig::Time jack_time;
	bool toggling_jack;
#endif

	Glib::RefPtr<Gtk::ToggleAction> action_mask_bone_setup_ducks, action_mask_bone_recursive_ducks;

	/*
 -- ** -- P U B L I C   D A T A -----------------------------------------------
	*/

public:
	void queue_rebuild_ducks();
	sigc::signal<void>& signal_deleted() { return signal_deleted_; }

	Gtk::Menu mainmenu;

	bool duck_refresh_flag;
	bool duck_refresh_needed;

	//! This is for the IsWorking class.
	int working_depth;

	bool cancel;

	/*
 -- ** -- D I A L O G S -------------------------------------------------------
	*/

public:

	CanvasProperties canvas_properties;
	CanvasOptions canvas_options;
	RenderSettings render_settings;
	Dialog_Waypoint waypoint_dialog;
	Dialog_Keyframe keyframe_dialog;

	std::auto_ptr<Dialog_Preview>			preview_dialog;
	//std::auto_ptr<Dialog_PreviewOptions>	previewoption_dialog;
	std::auto_ptr<Dialog_SoundSelect>		sound_dialog;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	// Constructor is private to force the use of the "create()" constructor
	CanvasView(etl::loose_handle<Instance> instance,etl::handle<synfigapp::CanvasInterface> canvas_interface);

	//! Constructor Helper
	// Gtk::Widget* create_layer_tree();

	//! Constructor Helper
	// Gtk::Widget* create_children_tree();

	//! Constructor Helper
	// Gtk::Widget* create_keyframe_tree();

	//! Constructor Helper - Initializes all of the menus
	void init_menus();

	bool duck_change_param(const synfig::Point &value,synfig::Layer::Handle layer, synfig::String param_name);

	void refresh_time_window();

	void time_was_changed();

	void refresh_rend_desc();

	void mask_bone_ducks();

	//! Constructor Helper - Create the workarea and connect data and signal
	/*! \see popup_main_menu() */
	Gtk::Widget *create_work_area();

	Gtk::Widget *create_time_bar();

	Gtk::ToolButton* create_action_toolbutton(const Glib::RefPtr<Gtk::Action> &action);
	Gtk::SeparatorToolItem* create_tool_separator();
	Gtk::Widget* create_display_bar();

	//! Pop up menu for the bezier (bline, draw) tool (?)
	void popup_param_menu_bezier(float location, synfigapp::ValueDesc value_desc)
	{ popup_param_menu(value_desc,location,true); }

	//! Pop up menu for the tools but not the bezier ones.
	void popup_param_menu(synfigapp::ValueDesc value_desc, float location=0, bool bezier=false);

	void workarea_layer_selected(synfig::Layer::Handle layer);

	void selected_layer_color_set(synfig::Color color);

	void register_layer_type(synfig::Layer::Book::value_type &lyr,std::map<synfig::String,Gtk::Menu*>*);

	//! Rebuilds the "new layer" menu
	void build_new_layer_menu(Gtk::Menu &menu);

	void decrease_low_res_pixel_size();
	void increase_low_res_pixel_size();
	void toggle_low_res_pixel_flag();
	void set_quality(int x);
	void set_onion_skins();
	void toggle_show_grid();
	void toggle_snap_grid();
	void toggle_onion_skin();

	void toggle_animatebutton();
	void toggle_timetrackbutton();

	void on_play_timeout();
	
	void interpolation_refresh();
	void on_interpolation_changed();
	void on_interpolation_event(GdkEvent *event);
	
	static void save_all();

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	ActivationIndex get_activation_index() { return activation_index_; }

	void activate();
	void deactivate();
	void present();

	bool jack_is_locked() const { return jack_locks > 0; }
	void jack_lock();
	void jack_unlock();
	bool get_jack_enabled_in_preview() const { return jack_enabled_in_preview; }
	void set_jack_enabled_in_preview(bool x) { jack_enabled_in_preview = x; }
#ifdef WITH_JACK
	bool get_jack_enabled() const { return jack_enabled; }
	bool get_jack_actual_enabled() const { return jack_actual_enabled; }
	void set_jack_enabled(bool value);
#endif

	synfig::Rect& get_bbox() { return bbox; }

	Glib::RefPtr<Glib::ObjectBase> get_ref_obj(const synfig::String& x);
	Glib::RefPtr<const Glib::ObjectBase> get_ref_obj(const synfig::String& x)const;
	void set_ref_obj(const synfig::String& x, Glib::RefPtr<Glib::ObjectBase> y);

	Glib::RefPtr<Gtk::TreeModel> get_tree_model(const synfig::String& x);
	Glib::RefPtr<const Gtk::TreeModel> get_tree_model(const synfig::String& x)const;
	void set_tree_model(const synfig::String& x, Glib::RefPtr<Gtk::TreeModel> y);

	Gtk::Widget* get_ext_widget(const synfig::String& x);
	void set_ext_widget(const synfig::String& x, Gtk::Widget* y);

	Gtk::UIManager::ui_merge_id get_popup_id();
	void set_popup_id(Gtk::UIManager::ui_merge_id popup_id);
	Gtk::UIManager::ui_merge_id get_toolbar_id();
	void set_toolbar_id(Gtk::UIManager::ui_merge_id toolbar_id);

	//std::map<synfig::String,Gtk::Widget*>& tree_view_book() { return tree_view_book_; }
	//std::map<synfig::String,Gtk::Widget*>& ext_widget_book() { return tree_view_book_; }

	//! Pop up menu for the main menu and the caret menu (not tools and not the bezier ones).
	/*! Signal handler for work_area->signal_popup_menu */
	/*! \see create_work_area(), popup_param_menu(), popup_param_menu_bezier() */
	void popup_main_menu();

	Smach& get_smach() { return smach_; }

	const Smach& get_smach()const { return smach_; }

	Smach::event_result process_event_key(EventKey x);

	void popup_layer_menu(synfig::Layer::Handle layer);

	virtual ~CanvasView();

	const synfig::ContextParams& get_context_params()const { return context_params_; }

	void set_context_params(const synfig::ContextParams &x) { context_params_ = x; }

	void set_mode(Mode x) { canvas_interface()->set_mode(x); }

	Mode get_mode()const { return canvas_interface()->get_mode(); }

	Glib::RefPtr<Gtk::Adjustment> time_adjustment() { return time_adjustment_; }
	Glib::RefPtr<const Gtk::Adjustment> time_adjustment()const { return time_adjustment_; }

	Glib::RefPtr<studio::Adjust_Window> time_window_adjustment() { return time_window_adjustment_; }
	Glib::RefPtr<const studio::Adjust_Window> time_window_adjustment()const { return time_window_adjustment_; }

	etl::handle<synfigapp::UIInterface> get_ui_interface() { return ui_interface_;}

	etl::handle<synfigapp::SelectionManager> get_selection_manager() { return selection_manager_; }

	Glib::RefPtr<Gtk::TreeModel> layer_tree_store() { return get_tree_model("layers"); }

	Glib::RefPtr<const Gtk::TreeModel> layer_tree_store()const { return get_tree_model("layers"); }

	Glib::RefPtr<Gtk::TreeModel> children_tree_store() { return get_tree_model("children"); }

	Glib::RefPtr<const Gtk::TreeModel> children_tree_store()const { return get_tree_model("children"); }

	Glib::RefPtr<Gtk::TreeModel> keyframe_tree_store() { return get_tree_model("keyframes"); }

	Glib::RefPtr<const Gtk::TreeModel> keyframe_tree_store()const { return get_tree_model("keyframes"); }

	void set_time(synfig::Time t) { canvas_interface_->set_time(t); }

	synfig::Time get_time() { return canvas_interface_->get_time(); }

	etl::handle<synfig::Canvas> get_canvas()const { return canvas_interface_->get_canvas(); }

	etl::handle<Instance> get_instance()const { return instance_; }

	etl::handle<synfigapp::CanvasInterface> canvas_interface() { return canvas_interface_; }

	etl::handle<const synfigapp::CanvasInterface> canvas_interface()const { return canvas_interface_; }

	void add_actions_to_menu(Gtk::Menu *menu,   const synfigapp::Action::ParamList &param_list, synfigapp::Action::Category category=synfigapp::Action::CATEGORY_ALL)const;

	//! Updates the title of the window
	void update_title();

	//! Closes this document
	bool close_instance();

	//! Closes this canvas view
	bool close_view();

	//!	Stops the currently executing action
	/*! \see get_cancel_status(), reset_cancel_status(), IsWorking */
	void stop() { cancel=true; }

	//! Returns the cancel status
	/*! \see stop(), reset_cancel_status(), IsWorking */
	bool get_cancel_status()const { return cancel; }

	//! Resets the cancel status
	/*! \see stop(), get_cancel_status(), IsWorking */
	void reset_cancel_status() { cancel=false; }

	void new_child_canvas();

	//! Rebuilds layer_tree_store_ from the Canvas. Maintains selected items.
	void rebuild_tables();

	//! Builds layer_tree_store_ from the Canvas. Does not maintain selected items.
	void build_tables();

	//! Refreshes the data for the tables
	void refresh_tables();

	//void rebuild_layer_table();
	//void build_layer_table();
	//void refresh_layer_table();

//	void rebuild_canvas_table();
//	void build_canvas_table();
//	void refresh_canvas_table();

//	void rebuild_valuenode_table();
//	void build_valuenode_table();
//	void refresh_valuenode_table();

	//! \writeme
	void rebuild_ducks();

	//bool add_to_ducks(synfigapp::ValueDesc value_desc, synfig::ParamDesc *param_desc=NULL);

	//! Starts "playing" the animation in real-time
	void play();

	void play_async();
	void stop_async();

	//! Shows the tables (Layer/Children)
	void show_tables();

	//! Hides the tables (Layer/Children)
	void hide_tables();

	//! Toggles the tables
	void toggle_tables();

	//! Gets the table status
	bool tables_are_visible();

	//! Shows the time bar
	void show_timebar();

	//! Hides the time bar
	void hide_timebar();

	//! Enables or disables interaction with the timebar
	void set_sensitive_timebar(bool sensitive);

	void time_zoom_in();
	void time_zoom_out();

	void add_layer(synfig::String x);

	void show_keyframe_dialog();
	void on_keyframe_toggle();
	void on_keyframe_description_set();

	void play_audio(float t);
	void stop_audio();

	void image_import();

	void on_waypoint_clicked_canvasview(synfigapp::ValueDesc,std::set<synfig::Waypoint,std::less<synfig::UniqueID> >, int button);

	void preview_option() {on_preview_option();}

	bool is_playing() { return is_playing_; }

	void update_quality();

	//! Toggle given handle type
	//! \Param[in]  type The Duckmatic::Type to toggle
	//! \Sa             DuckMatic::set_type_mask(), DuckMatic::get_type_mask()
	void toggle_duck_mask(Duckmatic::Type type);
	//! Toggle between none/last visible handles
	//! \Sa             DuckMatic::set_type_mask_state(), DuckMatic::get_type_mask_state()
	void toggle_duck_mask_all();

	bool is_time_equal_to_current_frame(const synfig::Time &time, const synfig::Time &range = synfig::Time(0.0));

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	void on_select_layers();
	void on_unselect_layers();

	void on_input_device_changed(GdkDevice*);

	void on_hide();

	Gtk::Widget* create_tab_label();

	bool on_button_press_event(GdkEventButton *event);

	//bool on_children_tree_event(GdkEvent *event);

	bool on_keyframe_tree_event(GdkEvent *event);

	//void on_children_edited_value(const Glib::ustring&path_string,synfig::ValueBase value);

	void on_dirty_preview();

	bool on_children_user_click(int, Gtk::TreeRow, ChildrenTree::ColumnID);

	bool on_layer_user_click(int, Gtk::TreeRow, LayerTree::ColumnID);

//	void on_layer_toggle(const Glib::ustring& path_string, Gtk::TreeModelColumn<bool> column);

	void on_mode_changed(synfigapp::CanvasInterface::Mode mode);

//	void on_layer_waypoint_clicked(const Glib::ustring &, synfig::ValueNode_Animated::WaypointList::iterator);

	//void on_children_waypoint_clicked(const Glib::ustring &, synfig::ValueNode_Animated::WaypointList::iterator);

	void on_waypoint_changed();

	void on_waypoint_delete();

	void on_refresh_pressed();

	void on_id_changed();

	void on_time_changed();

	/*
	void on_layer_raise_pressed();
	void on_layer_lower_pressed();
	void on_layer_duplicate_pressed();
	void on_layer_delete_pressed();
	*/

	void on_keyframe_add_pressed();

	void on_keyframe_duplicate_pressed();

	void on_keyframe_remove_pressed();

	void on_animate_button_pressed();

	void on_keyframe_button_pressed();
	void toggle_past_keyframe_button();
	void toggle_future_keyframe_button();

	void on_preview_option();
	void on_preview_create(const PreviewInfo &);

	void on_audio_option();
	void on_audio_file_change(const std::string &f);
	void on_audio_offset_change(const synfig::Time &t);

	void on_audio_file_notify();
	void on_audio_offset_notify();

	void on_layer_toggle(synfig::Layer::Handle);

	void on_edited_value(synfigapp::ValueDesc,synfig::ValueBase);

	void on_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);

	//void on_audio_play();
	bool on_audio_scrub();

	void on_play_pause_pressed();

#ifdef WITH_JACK
	void on_jack_offset_changed();
	void toggle_jack_button();

	synfig::Time get_jack_offset()const;
	void set_jack_offset(const synfig::Time &value);
#endif

	void on_meta_data_changed();

	//! Keyboard event dispatcher following window priority
	bool on_key_press_event(GdkEventKey* event);
	bool focused_widget_has_priority(Gtk::Widget * focused);


protected:
	bool close_instance_when_safe();
	bool on_delete_event(GdkEventAny* event);

	/*
 -- ** -- S T A T I C   P U B L I C   M E T H O D S ---------------------------
	*/

public:

	static etl::handle<studio::CanvasView> create(etl::loose_handle<Instance> instance,etl::handle<synfig::Canvas> canvas);
	static std::list<int>& get_pixel_sizes();

private:
#ifdef WITH_JACK
	void on_jack_sync();
	static int jack_sync_callback(jack_transport_state_t state, jack_position_t *pos, void *arg);
#endif

}; // END of class CanvasView

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
