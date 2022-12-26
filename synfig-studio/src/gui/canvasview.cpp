/* === S Y N F I G ========================================================= */
/*!	\file canvasview.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009, 2011, 2012 Carlos López
**	Copyright (c) 2009, 2011 Nikita Kitaev
**	Copyright (c) 2012 Konstantin Dmitriev
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

#include <gui/canvasview.h>

#include <algorithm>
#include <cmath>

#include <glibmm/convert.h>
#include <glibmm/uriutils.h>

#include <gtkmm/eventbox.h>
#include <gtkmm/hvseparator.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/stock.h>
#include <gtkmm/stylecontext.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/toolitem.h>

#include <ETL/stringf>

#include <gui/app.h>
#include <gui/dialogs/dialog_canvasdependencies.h>
#include <gui/dials/keyframedial.h>
#include <gui/dials/resolutiondial.h>
#include <gui/docks/dockbook.h>
#include <gui/docks/dockmanager.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/eventkey.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/preview.h>
#include <gui/states/state_normal.h>
#include <gui/widgets/widget_canvastimeslider.h>
#include <gui/widgets/widget_interpolation.h>
#include <gui/workarea.h>

#include <pangomm.h>
#include <sstream>
#include <string>

#include <synfig/rendering/renderer.h>
#include <synfig/valuenodes/valuenode_animated.h>

#include <synfigapp/canvasinterface.h>
#include <synfigapp/main.h>
#include <synfigapp/inputdevice.h>
#include <synfigapp/selectionmanager.h>
#include <synfigapp/uimanager.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace synfigapp;
using namespace studio;

/* === M A C R O S ========================================================= */

#define DEFAULT_TIME_WINDOW_SIZE (10.0)

#define SLOT_EVENT(x) \
	sigc::hide_return(sigc::bind(sigc::mem_fun(*this, &CanvasView::process_event_key), x))

/* === P R O C E D U R E S ================================================= */

static Gtk::Image*
create_image_from_icon(const std::string& icon_name, Gtk::IconSize icon_size)
{
#if GTK_CHECK_VERSION(3,24,0)
	return new Gtk::Image(icon_name, icon_size);
#else
	Gtk::Image* image = new Gtk::Image();
	image->set_from_icon_name(icon_name, icon_size);
	return image;
#endif
}

/* === C L A S S E S ======================================================= */

class studio::CanvasViewUIInterface : public UIInterface
{
	CanvasView *view;
private:
	float cur_progress = 0.0;
public:

	CanvasViewUIInterface(CanvasView *view):
		view(view)
		{ 
			view->statusbar->push(_("Idle")); 
			view->progressbar->hide();
		}
	~CanvasViewUIInterface() { }

	virtual Response confirmation(
			const std::string &message,
			const std::string &details,
			const std::string &confirm,
			const std::string &cancel,
			Response dflt = RESPONSE_OK )
	{
		view->present();
		//App::process_all_events();
		Gtk::MessageDialog dialog(
			*App::main_window,
			message,
			false,
			Gtk::MESSAGE_WARNING,
			Gtk::BUTTONS_NONE,
			true
		);

		if (! details.empty())
			dialog.set_secondary_text(details);

		dialog.add_button(cancel, RESPONSE_CANCEL);
		dialog.add_button(confirm, RESPONSE_OK);
		dialog.set_default_response(dflt);

		dialog.show_all();
		int response = dialog.run();
		if (response != RESPONSE_OK)
			return RESPONSE_CANCEL;
		return RESPONSE_OK;
	}

	virtual Response yes_no_cancel(
				const std::string &message,
				const std::string &details,
				const std::string &button1,
				const std::string &button2,
				const std::string &button3,
				bool hasDestructiveAction,
				Response dflt=RESPONSE_YES )
	{
		view->present();
		//App::process_all_events();
		Gtk::MessageDialog dialog(
			*App::main_window,
			message,
			false,
			Gtk::MESSAGE_QUESTION,
			Gtk::BUTTONS_NONE,
			true
		);

		dialog.set_secondary_text(details);
		Gtk::Button* no_button = dialog.add_button(button1, RESPONSE_NO);
		dialog.add_button(button2, RESPONSE_CANCEL);
		dialog.add_button(button3, RESPONSE_YES);
		//add destructive-action colored button if closed without saving
		if (hasDestructiveAction)
			no_button->get_style_context()->add_class("destructive-action");
		dialog.set_default_response(dflt);
		dialog.show();
		int response = dialog.run();
		if (response != RESPONSE_YES && response != RESPONSE_NO)
			return RESPONSE_CANCEL;
		return Response(response);
	}

	virtual bool
	task(const std::string &task)
	{
		if(!view->is_playing())
		{
			view->statusbar->pop();
			view->statusbar->push(task);
			view->statusbar->set_tooltip_text(task);
		}
		//App::process_all_events();
		if(view->cancel){return false;}
		return true;
	}

	virtual bool
	error(const std::string &err)
	{
		view->statusbar->push(_("ERROR"));

		// If we are in the process of canceling,
		// then just go ahead and return false --
		// don't bother displaying a dialog
		if(view->cancel)return false;
		Gtk::MessageDialog dialog(*App::main_window, err, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
		dialog.show();
		dialog.run();
		view->statusbar->pop();
		return true;
	}

	virtual bool
	warning(const std::string &err)
	{
		view->statusbar->pop();
		view->statusbar->push(err);

		//App::process_all_events();
		if(view->cancel)return false;
		return true;
	}

	virtual bool
	amount_complete(int current, int total)
	{
		float temp = (float)current/(float)total;
		
		if(temp >= 1.0)
		{
			view->statusbar->show();
			view->progressbar->hide();
		}

		else if(fabs(temp-cur_progress) >= 0.01)
		{
			cur_progress = temp;
			view->statusbar->hide();
			view->progressbar->show();
			view->progressbar->set_fraction(cur_progress);
			studio::App::process_all_events(); 
			return true;
		}
		
		
		
		if(!view->is_playing())
		{
			if(!view->working_depth)
			{
				if(current)
					view->stopbutton->set_sensitive(true);
				else
					view->stopbutton->set_sensitive(false);
			}
			float x((float)current/(float)total);
			if(x<0)x=0;
			else if(x>1)x=1;
		}
		//App::process_all_events();
		if(view->cancel){/*view->cancel=false;*/return false;}
		return true;
	}

	void
	not_implemented()
		{ error(_("Feature not yet implemented")); }
};


class studio::CanvasViewSelectionManager : public SelectionManager
{
	CanvasView *view;
	CanvasView::LayerTreeModel layer_tree_model;
	CanvasView::ChildrenTreeModel children_tree_model;

public:
	CanvasViewSelectionManager(CanvasView *view): view(view) { }

private:
	void _set_selected_layer(const Layer::Handle &layer)
		{ view->layer_tree->select_layer(layer); }

public:
	//! Returns the number of layers selected.
	virtual int get_selected_layer_count()const
		{ return get_selected_layers().size(); }

	//! Returns a list of the currently selected layers.
	virtual LayerList get_selected_layers()const
	{
		if(!view->layer_tree) { error("%s:%d canvas_view.layer_tree not defined!?", __FILE__, __LINE__); return LayerList(); }
		return view->layer_tree->get_selected_layers();
	}

	//! Returns the first layer selected or an empty handle if none are selected.
	virtual Layer::Handle get_selected_layer()const
	{
		if(!view->layer_tree) { error("%s:%d canvas_view.layer_tree not defined!?", __FILE__, __LINE__); return 0; }
		return view->layer_tree->get_selected_layer();
	}

	//! Sets which layers should be selected
	virtual void set_selected_layers(const LayerList &layer_list)
	{
		if(!view->layer_tree) { error("%s:%d canvas_view.layer_tree not defined!?", __FILE__, __LINE__); return; }
		view->layer_tree->select_layers(layer_list);
	}

	//! Sets which layer should be selected.
	virtual void set_selected_layer(const Layer::Handle &layer)
	{
		if(!view->layer_tree) { error("canvas_view.layer_tree not defined!?"); return; }
		view->layer_tree->select_layer(layer);
	}

	//! Clears the layer selection list
	virtual void clear_selected_layers()
	{
		if(!view->layer_tree) return;
		view->layer_tree->clear_selected_layers();
	}

	virtual LayerList get_expanded_layers()const
	{
		if(!view->layer_tree) { error("%s:%d canvas_view.layer_tree not defined!?", __FILE__, __LINE__); return LayerList(); }
		return view->layer_tree->get_expanded_layers();
	}

	virtual void set_expanded_layers(const LayerList &layer_list)
	{
		if(!view->layer_tree) { error("%s:%d canvas_view.layer_tree not defined!?", __FILE__, __LINE__); return; }
		view->layer_tree->expand_layers(layer_list);
	}

	//! Returns the number of value_nodes selected.
	virtual int get_selected_children_count()const
		{ return get_selected_children().size(); }

	static inline void __child_grabber(const Gtk::TreeModel::iterator& iter, ChildrenList* ret)
	{
		const CanvasView::ChildrenTreeModel children_tree_model;
		ValueDesc value_desc((*iter)[children_tree_model.value_desc]);
		if(value_desc)
			ret->push_back(value_desc);
	}

	//! Returns a list of the currently selected value_nodes.
	virtual ChildrenList get_selected_children()const
	{
		if(!view->children_tree) return ChildrenList();
		Glib::RefPtr<Gtk::TreeSelection> selection=view->children_tree->get_selection();
		if(!selection)
			return ChildrenList();
		ChildrenList ret;
		selection->selected_foreach_iter( sigc::bind(
				sigc::ptr_fun( &CanvasViewSelectionManager::__child_grabber ),
				&ret ));
		return ret;
	}

	//! Returns the first value_node selected or an empty handle if none are selected.
	virtual ChildrenList::value_type get_selected_child()const
	{
		if(!view->children_tree) return ChildrenList::value_type();
		ChildrenList children(get_selected_children());
		if(children.empty())
			return ChildrenList::value_type();
		return children.front();
	}

	//! Sets which value_nodes should be selected
	virtual void set_selected_children(const ChildrenList &/*children_list*/) { }

	//! Sets which value_node should be selected. Empty handle if none.
	virtual void set_selected_child(const ChildrenList::value_type &/*child*/) { }

	//! Clears the value_node selection list
	virtual void clear_selected_children() { }

	int get_selected_layer_parameter_count()const
		{ return get_selected_layer_parameters().size(); }

	LayerParamList get_selected_layer_parameters()const
	{
		if(!view->layer_tree) return LayerParamList();
		Glib::RefPtr<Gtk::TreeSelection> selection=view->layer_tree->get_selection();
		if(!selection)
			return LayerParamList();
		LayerParamList ret;
		Gtk::TreeModel::Children children = view->layer_tree_store()->children();
		for(Gtk::TreeModel::Children::const_iterator i = children.begin(); i != children.end(); ++i)
			for(Gtk::TreeModel::Children::const_iterator j = i->children().begin(); j != i->children().end(); ++j)
				if(selection->is_selected(*j))
					ret.push_back(LayerParam((*j)[layer_tree_model.layer], (Glib::ustring)(*j)[layer_tree_model.id]));
		return ret;
	}

	LayerParam get_selected_layer_parameter() const
	{
		if(!view->layer_tree) return LayerParam();
		return get_selected_layer_parameters().front();
	}

	void set_selected_layer_parameters(const LayerParamList &/*layer_param_list*/) { }

	void set_selected_layer_param(const LayerParam &/*layer_param*/) { }

	void clear_selected_layer_parameters() { }
}; // END of class SelectionManager


CanvasView::IsWorking::IsWorking(CanvasView &canvas_view_):
	canvas_view_(canvas_view_)
{
	if(!canvas_view_.working_depth)
		canvas_view_.stopbutton->set_sensitive(true);
	canvas_view_.working_depth++;
	canvas_view_.cancel=false;
}

CanvasView::IsWorking::~IsWorking()
{
	canvas_view_.working_depth--;
	if(!canvas_view_.working_depth)
		canvas_view_.stopbutton->set_sensitive(false);
}

CanvasView::IsWorking::operator bool()const
{
	if(canvas_view_.cancel)
		return false;
	return true;
}

/* === M E T H O D S ======================================================= */

LockDucks::LockDucks(const CanvasView::Handle &canvas_view):
	canvas_view_handle(canvas_view.get()),
	canvas_view(canvas_view.get())
{
	if (!this->canvas_view) return;
	++(this->canvas_view->ducks_locks);
}

LockDucks::LockDucks(CanvasView &canvas_view):
	canvas_view(&canvas_view)
{
	if (!this->canvas_view) return;
	++(this->canvas_view->ducks_locks);
}

LockDucks::~LockDucks() {
	if (!canvas_view) return;
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	if (--(canvas_view->ducks_locks) == 0) {
		if (canvas_view->ducks_rebuild_requested)
			canvas_view->rebuild_ducks();
		else
		if (canvas_view->ducks_rebuild_queue_requested)
			canvas_view->queue_rebuild_ducks();
	}
	SYNFIG_EXCEPTION_GUARD_END()
}


CanvasView::ActivationIndex CanvasView::ActivationIndex::last__;

CanvasView::CanvasView(etl::loose_handle<studio::Instance> instance,etl::handle<CanvasInterface> canvas_interface_):
	Dockable(synfig::GUID().get_string(),_("Canvas View")),
	work_area                (),
	activation_index_        (true),
	smach_                   (this),
	instance_                (instance),
	canvas_interface_        (canvas_interface_),
	context_params_          (true),
	time_model_              (new TimeModel()),
	statusbar                (manage(new class Gtk::Statusbar())),
	progressbar              (manage(new class Gtk::ProgressBar())),
	toggleducksdial          (Gtk::IconSize::from_name("synfig-small_icon_16x16")),
	resolutiondial_          (new studio::ResolutionDial()),
	future_onion_adjustment_ (Gtk::Adjustment::create(0,0,ONION_SKIN_FUTURE,1,1,0)),
	past_onion_adjustment_   (Gtk::Adjustment::create(1,0,ONION_SKIN_PAST,1,1,0)),

	timeslider               (manage(new Widget_CanvasTimeslider)),
	widget_kf_list           (manage(new Widget_Keyframe_List)),

	ui_interface_            (new CanvasViewUIInterface(this)),
	selection_manager_       (new CanvasViewSelectionManager(this)),

	jack_enabled             (false),
	jack_actual_enabled      (false),
	jack_locks               (0),
	jack_enabled_in_preview  (false),

#ifdef WITH_JACK
	jack_client              (nullptr),
	jack_synchronizing       (true),
	jack_is_playing          (false),
	jack_time                (0),
	toggling_jack            (false),
#endif

	ducks_locks              (0),
	ducks_rebuild_requested  (false),
	ducks_rebuild_queue_requested(false),

	working_depth            (0),
	cancel                   (false),

	canvas_properties        (*App::main_window,canvas_interface_),
	render_settings          (*App::main_window,canvas_interface_),
	waypoint_dialog          (*App::main_window,canvas_interface_->get_canvas()),
	keyframe_dialog          (*App::main_window,canvas_interface_),
	preview_dialog           ()
{
	// Make this toolbar small for space efficiency
	get_style_context()->add_class("synfigstudio-efficient-workspace");
	set_name("canvasview");

	canvas_options = CanvasOptions::create(*App::main_window, this);

	layer_tree=0;
	children_tree=0;
	toggling_ducks_=false;
	toggling_animate_mode_=false;
	changing_resolution_=false;
	toggling_show_grid=false;
	toggling_snap_grid=false;
	toggling_show_guides=false;
	toggling_snap_guides=false;
	toggling_onion_skin=false;
	toggling_onion_skin_keyframes=false;
	toggling_background_rendering=false;

	set_use_scrolled(false);

	//info("Canvasview: Entered constructor");
	// Minor hack
	get_canvas()->set_time(0);
	//layer_tree_store_->rebuild();

	// Set up the UI and Selection managers
	canvas_interface()->set_ui_interface(get_ui_interface());
	canvas_interface()->set_selection_manager(get_selection_manager());

	//info("Canvasview: Before big chunk of allocation and tabling stuff");
	//create all allocated stuff for this canvas

	Gtk::Widget *widget_work_area = create_work_area();
	widget_work_area->set_margin_top(4);
	init_menus();
	Gtk::Widget *widget_top_bar = create_top_toolbar();
	Gtk::Widget *widget_stopbutton = create_stop_button();
	Gtk::Widget *widget_right_bar = create_right_toolbar();
	Gtk::Widget *widget_time_bar = create_time_bar();
	
	Gtk::Grid *layout_grid = manage(new Gtk::Grid());
	layout_grid->attach(*widget_top_bar,     0, 0, 1, 1);
	layout_grid->attach(*widget_stopbutton,  1, 0, 1, 1);
	layout_grid->attach(*widget_right_bar,   1, 1, 1, 2);
	layout_grid->attach(*widget_work_area,   0, 2, 1, 1);
	layout_grid->attach(*widget_time_bar,    0, 3, 2, 1);
	layout_grid->show();

	Gtk::EventBox *event_box = manage(new Gtk::EventBox());
	event_box->add(*layout_grid);
	event_box->show();
	event_box->signal_button_press_event().connect(sigc::mem_fun(*this,&CanvasView::on_button_press_event));
	add(*event_box);

	update_title();

	smach_.set_default_state(&state_normal);

	//info("Canvasview: Before Signals");
	//SIGNALS
	#define CONNECT(x, y) x().connect(sigc::mem_fun(*this, y))
	CONNECT(canvas_interface()->signal_dirty_preview, &CanvasView::on_dirty_preview);
	CONNECT(canvas_interface()->signal_mode_changed,  &CanvasView::on_mode_changed);
	CONNECT(canvas_interface()->signal_time_changed,  &CanvasView::on_interface_time_changed);
	#undef CONNECT

	canvas_interface()->signal_id_changed().connect(sigc::mem_fun(*this,&CanvasView::on_id_changed));
	canvas_interface()->signal_rend_desc_changed().connect(sigc::mem_fun(*this,&CanvasView::refresh_rend_desc));
	waypoint_dialog.signal_changed().connect(sigc::mem_fun(*this,&CanvasView::on_waypoint_changed));
	waypoint_dialog.signal_delete().connect(sigc::mem_fun(*this,&CanvasView::on_waypoint_delete));

	//MODIFIED TIME ADJUSTMENT STUFF....
	time_model()->signal_time_changed().connect(
		sigc::mem_fun(*this, &CanvasView::on_time_changed));
	time_model()->signal_visible_changed().connect(
		sigc::mem_fun(*this, &CanvasView::refresh_time_window));
	time_model()->signal_play_bounds_changed().connect(
		sigc::mem_fun(*this, &CanvasView::refresh_time_window));

	work_area->signal_layer_selected().connect(sigc::mem_fun(*this,&CanvasView::workarea_layer_selected));
	work_area->signal_input_device_changed().connect(sigc::mem_fun(*this,&CanvasView::on_input_device_changed));
	work_area->signal_meta_data_changed().connect(sigc::mem_fun(*this,&CanvasView::on_meta_data_changed));

	canvas_interface()->signal_canvas_added().connect(
		sigc::hide( sigc::mem_fun(*instance,&studio::Instance::refresh_canvas_tree)));
	canvas_interface()->signal_canvas_removed().connect(
		sigc::hide( sigc::mem_fun(*instance,&studio::Instance::refresh_canvas_tree)));
	canvas_interface()->signal_layer_param_changed().connect(
		sigc::hide(sigc::hide( SLOT_EVENT(EVENT_REFRESH_DUCKS))));
	canvas_interface()->signal_keyframe_properties().connect(
		sigc::mem_fun(*this,&CanvasView::show_keyframe_dialog));

	//MUCH TIME STUFF TAKES PLACE IN HERE
	refresh_rend_desc();
	refresh_time_window();

	std::vector<Gtk::TargetEntry> listTargets;
	listTargets.push_back(Gtk::TargetEntry("text/uri-list"));
	listTargets.push_back(Gtk::TargetEntry("text/plain"));
	listTargets.push_back(Gtk::TargetEntry("STRING"));

	drag_dest_set(listTargets);
	signal_drag_data_received().connect(sigc::mem_fun(*this, &CanvasView::on_drop_drag_data_received));

	hide_tables();
	show();

	instance->canvas_view_list().push_front(this);
	instance->signal_canvas_view_created()(this);
	//info("Canvasview: Constructor Done");

	if (App::jack_is_locked())
		jack_lock();

	#ifdef WITH_JACK
	jack_dispatcher.connect(sigc::mem_fun(*this, &CanvasView::on_jack_sync));
	#endif

	App::dock_manager->register_dockable(*this);
	App::main_window->main_dock_book().add(*this);

	time_model()->all_changed();
	present();
	App::set_selected_canvas_view(this);
}

CanvasView::~CanvasView()
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()

	#ifdef WITH_JACK
	set_jack_enabled(false);
	#endif

	App::dock_manager->unregister_dockable(*this);
	signal_deleted()();

	// I didn't find a proper way to check if actiongroup is already removed on CanvasView::deactivate
	// So here is a quick-hack. This error is mostly invisible because it fails on App exiting
	// but i didn't think it worth to spend time to it, because remove_action_group is deprecated
	// and this code is required to rewrite.

	if (!this->_action_group_removed)
		App::ui_manager()->remove_action_group(action_group);

	// Shut down the smach
	smach_.egress();
	smach_.set_default_state(0);

	// We want to ensure that the UI_Manager and
	// the selection manager get destructed right now.
	ui_interface_.reset();
	selection_manager_.reset();

	// don't be calling on_dirty_preview once this object has been deleted;
	// this was causing a crash before
	canvas_interface()->signal_dirty_preview().clear();

	delete canvas_options;
	delete resolutiondial_;

	DEBUG_LOG("SYNFIG_DEBUG_DESTRUCTORS",
		"CanvasView::~CanvasView(): Deleted");

	SYNFIG_EXCEPTION_GUARD_END()
}

void CanvasView::activate()
{
	activation_index_.activate();
	get_smach().process_event(EVENT_REFRESH_TOOL_OPTIONS);
	App::ui_manager()->insert_action_group(action_group);
	this->_action_group_removed = false;
	update_title();
	present();
	grab_focus();
}

void CanvasView::deactivate()
{
	get_smach().process_event(EVENT_YIELD_TOOL_OPTIONS);
	App::ui_manager()->remove_action_group(action_group);
	this->_action_group_removed = true;
	update_title();
}

void CanvasView::present()
{
	show(); // gtk_widget_show also checks visibility, so there is no need to call `is_visible()`
	App::set_selected_canvas_view(this);
	update_title();
	Dockable::present();
}

void CanvasView::jack_lock()
{
	++jack_locks;
	#ifdef WITH_JACK
	if (jack_locks == 1)
		set_jack_enabled(get_jack_enabled());
	#endif
}

void CanvasView::jack_unlock()
{
	--jack_locks;
	assert(jack_locks >= 0);
	#ifdef WITH_JACK
	if (jack_locks == 0)
		set_jack_enabled(get_jack_enabled());
	#endif
}

#ifdef WITH_JACK
void CanvasView::set_jack_enabled(bool value)
{
	bool actual_value = value && !jack_is_locked();
	if (jack_actual_enabled != actual_value) {
		jack_actual_enabled = actual_value;
		if (jack_actual_enabled)
		{
			// initialize jack
			stop();
			jack_client = jack_client_open("synfigstudiocanvas", JackNullOption, 0);
			jack_set_sync_callback(jack_client, jack_sync_callback, this);
			if (jack_activate(jack_client) != 0)
			{
				jack_client_close(jack_client);
				jack_client = nullptr;
				jack_actual_enabled = false;
				// make conditions to update button
				jack_enabled = true;
				value = false;
			}
		}
		else
		{
			// deinitialize jack
			jack_deactivate(jack_client);
			jack_client_close(jack_client);
			jack_client = nullptr;
		}
	}

	if (jack_enabled != value)
	{
		jack_enabled = value;
		jackdial->set_state(jack_enabled);
	}
}
#endif

std::list<int>&
CanvasView::get_pixel_sizes()
{
	// prime factors of 64 is 2, 2, 2, 2, 2, 2 - see TILE_SIZE in synfig-core/trunk/src/synfig/target_tile.h
	// also see available low-res renderer engines in rendering::Renderer::initialize_renderers()
	static int pixel_size_array[] = {2, 4, 8, 16};
	static std::list<int> pixel_sizes(pixel_size_array, pixel_size_array + sizeof(pixel_size_array)/sizeof(int));
	return pixel_sizes;
}

Gtk::Widget*
CanvasView::create_time_bar()
{
	//Setup the keyframe list widget
	widget_kf_list->set_time_model(time_model());
	widget_kf_list->set_canvas_interface(canvas_interface());
	widget_kf_list->show();

	// Setup Time Slider
	timeslider->set_canvas_view(this);
	timeslider->set_can_focus(true);
	timeslider->show();

	// Setup Time Scroll
	Gtk::Scrollbar *time_window_scroll = manage(new class Gtk::Scrollbar(time_model()->scroll_time_adjustment()));
	time_window_scroll->set_tooltip_text(_("Moves the time window"));
	//time_window_scroll->set_can_focus(true); // Uncomment this produce bad render of the HScroll
	time_window_scroll->show();

	timetrack = manage(new Gtk::Grid());
	time_window_scroll->set_hexpand(true);
	timetrack->attach(*time_window_scroll, 0, 0, 1, 1);
	timeslider->set_hexpand(true);
	timetrack->attach(*timeslider, 0, 1, 1, 1);
	timeslider->set_hexpand(true);
	timetrack->attach(*widget_kf_list, 0, 2, 1, 1);
	timetrack->hide();

	// Interpolation widget
	widget_interpolation = manage(new Widget_Interpolation(Widget_Interpolation::SIDE_BOTH));
	widget_interpolation->set_tooltip_text(_("Default Interpolation"));
	widget_interpolation->set_popup_fixed_width(false);
	widget_interpolation->set_hexpand(false);
	widget_interpolation->show();
	widget_interpolation->signal_changed().connect(sigc::mem_fun(*this, &CanvasView::on_interpolation_changed));

	synfigapp::Main::signal_interpolation_changed().connect(sigc::mem_fun(*this, &CanvasView::interpolation_refresh));
	synfigapp::Main::set_interpolation(INTERPOLATION_CLAMPED); // Clamped by default.
	interpolation_refresh();

	//Setup the Animation Mode Button and the Keyframe Lock button
	{
		Gtk::IconSize iconsize=Gtk::IconSize::from_name("synfig-small_icon_16x16");
		animatebutton = Gtk::manage(new Gtk::ToggleButton());
		animatebutton->set_image_from_icon_name("animate_mode_off_icon", iconsize);
		animatebutton->set_tooltip_text(_("Turn on animate editing mode"));

		// Set hotkey to toggle animate button on and off
		auto accel_group = App::ui_manager()->get_accel_group();
		animatebutton->set_accel_path("<Actions>/canvasview/animate", accel_group);

		animatebutton->signal_toggled().connect(sigc::mem_fun(*this, &CanvasView::toggle_animatebutton));
		animatebutton->set_relief(Gtk::RELIEF_NONE);
		animatebutton->show();
	}

	{
		Gtk::IconSize iconsize=Gtk::IconSize::from_name("synfig-small_icon_16x16");
		timetrackbutton = Gtk::manage(new Gtk::ToggleButton());
		timetrackbutton->set_image_from_icon_name("time_track_icon", iconsize);
		timetrackbutton->set_tooltip_text(_("Toggle timebar"));

		timetrackbutton->signal_toggled().connect(sigc::mem_fun(*this, &CanvasView::toggle_timetrackbutton));
		timetrackbutton->set_relief(Gtk::RELIEF_NONE);
		timetrackbutton->show();
	}

	//Setup the current time widget
	current_time_widget=manage(new Widget_Time);
	current_time_widget->set_value(get_time());
	current_time_widget->set_fps(get_canvas()->rend_desc().get_frame_rate());
	current_time_widget->signal_value_changed().connect(
		sigc::mem_fun(*this,&CanvasView::on_current_time_widget_changed)
	);
	current_time_widget->set_size_request(0,-1); // request horizontal shrink
	current_time_widget->set_width_chars(5);
	current_time_widget->set_tooltip_text(_("Current time"));
	current_time_widget->show();

	//Setup the FrameDial widget
	framedial = manage(new class FrameDial());
	//Setup end time widget
	framedial->set_end_time(get_canvas()->rend_desc().get_frame_rate(), get_canvas()->rend_desc().get_time_end());

	framedial->signal_seek_begin().connect(
		sigc::mem_fun(*this, &CanvasView::on_seek_begin_pressed) );
	framedial->signal_seek_prev_keyframe().connect(
		sigc::mem_fun(*canvas_interface().get(), &CanvasInterface::jump_to_prev_keyframe));
	framedial->signal_seek_prev_frame().connect(
		sigc::bind(sigc::mem_fun(*canvas_interface().get(), &CanvasInterface::seek_frame), -1));
	framedial->signal_play().connect(
		sigc::mem_fun(*this, &CanvasView::on_play_pause_pressed));
	framedial->signal_pause().connect(
		sigc::mem_fun(*this, &CanvasView::on_play_pause_pressed));
	framedial->signal_seek_next_frame().connect(
		sigc::bind(sigc::mem_fun(*canvas_interface().get(), &CanvasInterface::seek_frame), 1));
	framedial->signal_seek_next_keyframe().connect(
		sigc::mem_fun(*canvas_interface().get(), &CanvasInterface::jump_to_next_keyframe));
	framedial->signal_seek_end().connect(
		sigc::mem_fun(*this, &CanvasView::on_seek_end_pressed) );
	framedial->signal_end_time_changed().connect(
		sigc::mem_fun(*this,&CanvasView::on_set_end_time_widget_changed));
	framedial->signal_repeat().connect(
		sigc::mem_fun(*time_model(), &TimeModel::set_play_repeat));
	framedial->signal_bounds_enable().connect(
		sigc::mem_fun(*time_model(), &TimeModel::set_play_bounds_enabled));
	framedial->signal_bound_lower().connect(
		sigc::bind(sigc::mem_fun(*time_model(), &TimeModel::set_play_bounds_enabled), true));
	framedial->signal_bound_lower().connect(
		sigc::mem_fun(*time_model(), &TimeModel::set_play_bounds_lower_to_current));
	framedial->signal_bound_upper().connect(
		sigc::bind(sigc::mem_fun(*time_model(), &TimeModel::set_play_bounds_enabled), true));
	framedial->signal_bound_upper().connect(
		sigc::mem_fun(*time_model(), &TimeModel::set_play_bounds_upper_to_current));
	framedial->show();

	Gtk::Separator *separator = manage(new Gtk::Separator());
	separator->show();

	//Setup the KeyFrameDial widget
	keyframedial = Gtk::manage(new KeyFrameDial());
	keyframedial->signal_toggle_keyframe_past().connect(sigc::mem_fun(*this, &CanvasView::toggle_past_keyframe_button));
	keyframedial->signal_toggle_keyframe_future().connect(sigc::mem_fun(*this, &CanvasView::toggle_future_keyframe_button));
	keyframedial->set_margin_start(4);
	keyframedial->set_margin_end(4);
	keyframedial->show();

	//Adjust both widgets to be the same as the
	int header_height = 0;
	if(getenv("SYNFIG_TIMETRACK_HEADER_HEIGHT"))
		header_height = atoi(getenv("SYNFIG_TIMETRACK_HEADER_HEIGHT"));
	if (header_height < 3)
		header_height = 24;
	timeslider->set_size_request(-1,header_height-header_height/3+1);
	widget_kf_list->set_size_request(-1,header_height/3+1);

	jackdial = manage(new class JackDial());

	#ifdef WITH_JACK
	jackdial->signal_toggle_jack().connect(sigc::mem_fun(*this, &CanvasView::toggle_jack_button));
	jackdial->signal_offset_changed().connect(sigc::mem_fun(*this, &CanvasView::on_jack_offset_changed));
	jackdial->set_fps(get_canvas()->rend_desc().get_frame_rate());
	jackdial->set_offset(get_jack_offset());
	if ( !getenv("SYNFIG_DISABLE_JACK") )
		jackdial->show();
	#endif

	// fix thickness of statusbar
	assert(statusbar);

	Gtk::Widget *widget = statusbar;
	while(Gtk::Container *container = dynamic_cast<Gtk::Container*>(widget)) {
		widget->set_margin_top(0);
		widget->set_margin_bottom(0);
		widget = container->get_children().empty() ? nullptr : container->get_children().front();
	}
	statusbar->show();

	//Attach widgets to the timebar

	Gtk::Grid *controls = manage(new Gtk::Grid());
	{
		int left_pos = 0;
		controls->attach(*timetrackbutton, left_pos++, 0, 1, 1);
		controls->attach(*current_time_widget, left_pos++, 0, 1, 1);
		controls->attach(*framedial, left_pos++, 0, 1, 1);
		controls->attach(*separator, left_pos++, 0, 1, 1);
		controls->attach(*jackdial, left_pos++, 0, 1, 1);
		controls->attach(*statusbar, left_pos++, 0, 1, 1);
		controls->attach(*progressbar, left_pos++, 0, 1, 1);
		controls->attach(*widget_interpolation, left_pos++, 0, 1, 1);
		controls->attach(*keyframedial, left_pos++, 0, 1, 1);
		controls->attach(*animatebutton, left_pos++, 0, 1, 1);

		// Make progress bar bigger than the default GTK one
		progressbar->set_name("status-progress");

		progressbar->set_hexpand(true);
		progressbar->set_halign(Gtk::Align::ALIGN_FILL);

		statusbar->set_hexpand(true);
		statusbar->set_halign(Gtk::Align::ALIGN_FILL);

		controls->show();
	}

	timebar = Gtk::manage(new Gtk::Grid());
	timebar->attach(*controls, 0, 0, 1, 1);
	timebar->attach(*timetrack, 0, 1, 1, 1);
	timebar->set_hexpand();
	timebar->show();

	return timebar;
}

Gtk::Widget *
CanvasView::create_work_area()
{
	work_area = manage(new WorkArea(canvas_interface_));
	work_area->set_instance(get_instance());
	work_area->set_canvas(get_canvas());
	work_area->set_canvas_view(this);
	work_area->set_progress_callback(get_ui_interface().get());
	work_area->signal_popup_menu().connect(sigc::mem_fun(*this, &CanvasView::popup_main_menu));
	work_area->set_hexpand();
	work_area->set_vexpand();
	work_area->show();
	return work_area;
}

Gtk::ToolButton*
CanvasView::create_action_toolbutton(const Glib::RefPtr<Gtk::Action> &action)
{
	Gtk::ToolButton *button = Gtk::manage(new Gtk::ToolButton());
	button->set_related_action(action);
	button->show();
	return button;
}

Gtk::SeparatorToolItem*
CanvasView::create_tool_separator()
{
	Gtk::SeparatorToolItem *separator = Gtk::manage(new Gtk::SeparatorToolItem());
	separator->show();
	return separator;
}

void CanvasView::toggle_render_combobox()
{
	App::navigator_renderer = App::workarea_renderer = render_combobox->get_active_id();

	App::save_settings();
	App::setup_changed();
}

Gtk::Widget*
CanvasView::create_top_toolbar()
{
	displaybar = manage(new Gtk::Toolbar());
	displaybar->set_icon_size(Gtk::IconSize::from_name("synfig-small_icon_16x16"));
	displaybar->set_toolbar_style(Gtk::TOOLBAR_BOTH_HORIZ);

	// File buttons
	if (App::show_file_toolbar) {
		displaybar->append(*create_action_toolbutton(App::ui_manager()->get_action("/toolbar-main/new")));
		displaybar->append(*create_action_toolbutton(App::ui_manager()->get_action("/toolbar-main/open")));
		displaybar->append(*create_action_toolbutton(action_group->get_action("save")));
		displaybar->append(*create_action_toolbutton(action_group->get_action("save-as")));
		displaybar->append(*create_action_toolbutton(action_group->get_action("save-all")));

		// Separator
		displaybar->append( *create_tool_separator() );
	}

	// Undo/Redo buttons
	displaybar->append(*create_action_toolbutton(App::ui_manager()->get_action("/toolbar-main/undo")));
	displaybar->append(*create_action_toolbutton(App::ui_manager()->get_action("/toolbar-main/redo")));

	// Separator
	displaybar->append(*create_tool_separator());

	{ // Preview Settings dialog button
		preview_options_button = Gtk::manage(new Gtk::ToolButton());
		preview_options_button->set_icon_name("preview_options_icon");
		preview_options_button->signal_clicked().connect(
			sigc::mem_fun(*this,&CanvasView::on_preview_option));
		preview_options_button->set_label(_("Preview"));
		preview_options_button->set_tooltip_text(_("Shows the Preview Settings Dialog"));
		preview_options_button->show();

		displaybar->append(*preview_options_button);
	}

	{ // Render Settings dialog button
		render_options_button = Gtk::manage(new Gtk::ToolButton());
		render_options_button->set_icon_name("render_options_icon");
		render_options_button->signal_clicked().connect(
			sigc::mem_fun0(render_settings,&RenderSettings::present));
		render_options_button->set_label(_("Render"));
		render_options_button->set_tooltip_text(_("Shows the Render Settings Dialog"));
		render_options_button->show();

		displaybar->append(*render_options_button);
	}

	// Separator
	displaybar->append(*create_tool_separator());

	{ // Refresh button
		refreshbutton = Gtk::manage(new Gtk::ToolButton());
		refreshbutton->set_icon_name("view-refresh");
		refreshbutton->signal_clicked().connect(SLOT_EVENT(EVENT_REFRESH));
		refreshbutton->set_label(_("Refresh"));
		refreshbutton->set_tooltip_text( _("Refresh workarea"));
		refreshbutton->show();

		displaybar->append(*refreshbutton);
	}

	{ // Rendering mode ComboBox
		render_combobox = Gtk::manage(new class Gtk::ComboBoxText());
		render_combobox->append("software-draft", _("Draft"));
		if (synfig::rendering::Renderer::get_renderers().count("gl") != 0)
			render_combobox->append("gl", _("GL"));
		render_combobox->append("software-preview", _("Preview"));
		render_combobox->append("software", _("Final"));
		render_combobox->signal_changed().connect(sigc::mem_fun(*this, &CanvasView::toggle_render_combobox));
		render_combobox->set_tooltip_text(_("Select rendering mode"));
		render_combobox->set_active(1);
		render_combobox->show();
		auto container = Gtk::manage(new class Gtk::ToolItem());
		container->add(*render_combobox);

		container->show();
		displaybar->add(*container);
	}

	{ // Background rendering button
		background_rendering_button = Gtk::manage(new Gtk::ToggleToolButton());
		background_rendering_button->set_active(work_area->get_background_rendering());
		background_rendering_button->set_icon_name("background_rendering_icon");
		background_rendering_button->signal_toggled().connect(
			sigc::mem_fun(*this, &CanvasView::toggle_background_rendering));
		background_rendering_button->set_label(_("Background rendering"));
		background_rendering_button->set_tooltip_text(_("Render future and past frames in background when enabled"));
		background_rendering_button->show();

		displaybar->append(*background_rendering_button);
	}

	// Separator
	displaybar->append(*create_tool_separator());

	// ResolutionDial widget
	resolutiondial_->update_lowres(work_area->get_low_resolution_flag());
	resolutiondial_->signal_increase_resolution().connect(
		sigc::mem_fun(*this, &CanvasView::decrease_low_res_pixel_size));
	resolutiondial_->signal_decrease_resolution().connect(
		sigc::mem_fun(*this, &CanvasView::increase_low_res_pixel_size));
	resolutiondial_->signal_use_low_resolution().connect(
		sigc::mem_fun(*this, &CanvasView::toggle_low_res_pixel_flag));
	resolutiondial_->insert_to_toolbar(*displaybar);

	// Separator
	displaybar->append(*create_tool_separator());

	{ // Onion skin toggle button
		onion_skin = Gtk::manage(new Gtk::ToggleToolButton());
		onion_skin->set_active(work_area->get_onion_skin());
		onion_skin->set_icon_name("onion_skin_icon");
		onion_skin->signal_toggled().connect(
			sigc::mem_fun(*this, &CanvasView::toggle_onion_skin));
		onion_skin->set_label(_("Onion Skin"));
		onion_skin->set_tooltip_text(_("Show Onion Skin when enabled"));
		onion_skin->show();

		displaybar->append(*onion_skin);
	}

	{ // Past onion skin spin button
		past_onion_spin=Gtk::manage(new class Gtk::SpinButton(past_onion_adjustment_));
		past_onion_spin->set_value(work_area->get_onion_skins()[0]);
		past_onion_spin->signal_value_changed().connect(
			sigc::mem_fun(*this, &CanvasView::set_onion_skins));
		past_onion_spin->set_tooltip_text(_("Past Onion Skins"));
		past_onion_spin->show();

		Gtk::ToolItem *toolitem = Gtk::manage(new Gtk::ToolItem());
		toolitem->add(*past_onion_spin);
		toolitem->set_is_important(true);
		toolitem->show();

		displaybar->append(*toolitem);
	}

	{ // Future onion skin spin button
		future_onion_spin=Gtk::manage(new class Gtk::SpinButton(future_onion_adjustment_));
		future_onion_spin->set_value(work_area->get_onion_skins()[1]);
		future_onion_spin->signal_value_changed().connect(
			sigc::mem_fun(*this, &CanvasView::set_onion_skins));
		future_onion_spin->set_tooltip_text(_("Future Onion Skins"));
		future_onion_spin->show();

		Gtk::ToolItem *toolitem = Gtk::manage(new Gtk::ToolItem());
		toolitem->add(*future_onion_spin);
		toolitem->set_is_important(true);
		toolitem->show();

		displaybar->append(*toolitem);
	}

	{ // Onion skin on Keyframes/Frames toggle button
		onion_skin_keyframes = Gtk::manage(new Gtk::ToggleToolButton());
		onion_skin_keyframes->set_active(work_area->get_onion_skin_keyframes());
		onion_skin_keyframes->set_icon_name("keyframe_icon");
		onion_skin_keyframes->signal_toggled().connect(
			sigc::mem_fun(*this, &CanvasView::toggle_onion_skin_keyframes));
		onion_skin_keyframes->set_label(_("Keyframes"));
		onion_skin_keyframes->set_tooltip_text(_("Show Onion Skin on Keyframes when enabled, on Frames when disabled"));
		onion_skin_keyframes->show();

		displaybar->append(*onion_skin_keyframes);
	}

	if(App::enable_mainwin_toolbar)
		displaybar->show();
	else
		displaybar->hide();
	cancel=false;

	return displaybar;
}

Gtk::Widget*
CanvasView::create_stop_button()
{
	stopbutton = Gtk::manage(new Gtk::Button());
	stopbutton->set_image_from_icon_name("process-stop");
	stopbutton->signal_clicked().connect(SLOT_EVENT(EVENT_STOP));
	stopbutton->set_relief(Gtk::RELIEF_NONE);
	stopbutton->set_tooltip_text(_("Stop current operation"));
	stopbutton->set_sensitive(false);
	stopbutton->show();

	return stopbutton;
}

Gtk::Widget*
CanvasView::create_right_toolbar()
{
	displaybar = manage(new Gtk::Toolbar());
	displaybar->set_icon_size(Gtk::IconSize::from_name("synfig-small_icon_16x16"));
	displaybar->set_toolbar_style(Gtk::TOOLBAR_ICONS);
	displaybar->set_property("orientation", Gtk::ORIENTATION_VERTICAL);

	{ // Show grid toggle button
		show_grid = Gtk::manage(new Gtk::ToggleToolButton());
		show_grid->set_active(work_area->grid_status());
		show_grid->set_icon_name("show_grid_icon");
		show_grid->signal_toggled().connect(
			sigc::mem_fun(*this, &CanvasView::toggle_show_grid));
		show_grid->set_label(_("Show Grid"));
		show_grid->set_tooltip_text(_("Show Grid when enabled"));
		show_grid->show();

		displaybar->append(*show_grid);
	}

	{ // Snap to grid toggle button
		snap_grid = Gtk::manage(new Gtk::ToggleToolButton());
		snap_grid->set_active(work_area->grid_status());
		snap_grid->set_icon_name("snap_grid_icon");
		snap_grid->signal_toggled().connect(
			sigc::mem_fun(*this, &CanvasView::toggle_snap_grid));
		snap_grid->set_label(_("Snap to Grid"));
		snap_grid->set_tooltip_text(_("Snap to Grid when enabled"));
		snap_grid->show();

		displaybar->append(*snap_grid);
	}

	{ // Show guide toggle button
		show_guides = Gtk::manage(new Gtk::ToggleToolButton());
		show_guides->set_active(work_area->get_show_guides());
		show_guides->set_icon_name("show_guideline_icon");
		show_guides->signal_toggled().connect(
			sigc::mem_fun(*this, &CanvasView::toggle_show_guides));
		show_guides->set_label(_("Show Guides"));
		show_guides->set_tooltip_text(_("Show Guides when enabled"));
		show_guides->show();

		displaybar->append(*show_guides);
	}

	{ // Snap to guides toggle button
		snap_guides = Gtk::manage(new Gtk::ToggleToolButton());
		snap_guides->set_active(work_area->get_guide_snap());
		snap_guides->set_icon_name("snap_guideline_icon");
		snap_guides->signal_toggled().connect(
			sigc::mem_fun(*this, &CanvasView::toggle_snap_guides));
		snap_guides->set_label(_("Snap to Guides"));
		snap_guides->set_tooltip_text(_("Snap to Guides when enabled"));
		snap_guides->show();

		displaybar->append(*snap_guides);
	}

	// Separator
	displaybar->append(*create_tool_separator());

	// ToggleDuckDial widget
	Duck::Type m = work_area->get_type_mask();
	toggleducksdial.update_toggles(m);
	toggleducksdial.signal_ducks_position().connect(
		sigc::bind(sigc::mem_fun(*this, &CanvasView::toggle_duck_mask),Duck::TYPE_POSITION));
	toggleducksdial.signal_ducks_vertex().connect(
		sigc::bind(sigc::mem_fun(*this, &CanvasView::toggle_duck_mask),Duck::TYPE_VERTEX));
	toggleducksdial.signal_ducks_tangent().connect(
		sigc::bind(sigc::mem_fun(*this, &CanvasView::toggle_duck_mask),Duck::TYPE_TANGENT));
	toggleducksdial.signal_ducks_radius().connect(
		sigc::bind(sigc::mem_fun(*this, &CanvasView::toggle_duck_mask),Duck::TYPE_RADIUS));
	toggleducksdial.signal_ducks_width().connect(
		sigc::bind(sigc::mem_fun(*this, &CanvasView::toggle_duck_mask),Duck::TYPE_WIDTH));
	toggleducksdial.signal_ducks_angle().connect(
		sigc::bind(sigc::mem_fun(*this, &CanvasView::toggle_duck_mask),Duck::TYPE_ANGLE));
	toggleducksdial.insert_to_toolbar(*displaybar);

	displaybar->show();

	return displaybar;
}

void CanvasView::grab_focus()
{
	work_area->grab_focus();
}

void
CanvasView::on_current_time_widget_changed()
{
	set_time(current_time_widget->get_value());

	// show the value being used - it will have been rounded to nearest frame
	// this was already being done elsewhere, but only if the time was really changed;
	// if the current time was 6f and the user edited it to 6.1f, then the 6.1f would
	// be left in the display without the following line to fix it
	current_time_widget->set_value(get_time());
	current_time_widget->set_position(-1); // leave the cursor at the end
}

void
CanvasView::on_set_end_time_widget_changed()
{
	if (framedial->get_end_time() == get_canvas()->rend_desc().get_time_end())
		return;

	RendDesc rend_desc = get_canvas()->rend_desc();
	rend_desc.set_time_end(framedial->get_end_time());
	canvas_interface()->set_rend_desc(rend_desc);
}

void
CanvasView::init_menus()
{
	//cache the position of desired widgets

	/*Menus to worry about:
	- filemenu
	- editmenu
	- layermenu
	- duckmaskmenu
	- mainmenu
	- canvasmenu
	- viewmenu
	*/
	action_group = Gtk::ActionGroup::create("canvasview");

	action_group->add( Gtk::Action::create("save", Gtk::StockID("synfig-save"), _("Save"), _("Save")),
		hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::save))
	);
	action_group->add( Gtk::Action::create_with_icon_name("save-as", "action_doc_saveas_icon", _("Save As..."), _("Save As")),
		sigc::hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::dialog_save_as))
	);
	action_group->add( Gtk::Action::create("export", Gtk::StockID("synfig-export"), _("Export..."), _("Export")),
		sigc::hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::dialog_export))
	);
	action_group->add( Gtk::Action::create("revert", Gtk::Stock::REVERT_TO_SAVED),
		sigc::hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::safe_revert))
	);
	action_group->add( Gtk::Action::create("import", _("Import...")),
		sigc::hide_return(sigc::mem_fun(*this, &CanvasView::import_file))
	);
	action_group->add( Gtk::Action::create("import-sequence", _("Import Sequence...")),
		sigc::hide_return(sigc::mem_fun(*this, &CanvasView::import_sequence))
	);
	action_group->add( Gtk::Action::create("render", Gtk::StockID("synfig-render_options"), _("Render...")),
		sigc::mem_fun0(render_settings,&RenderSettings::present)
	);
	action_group->add( Gtk::Action::create("preview", Gtk::StockID("synfig-preview_options"), _("Preview...")),
		sigc::mem_fun(*this,&CanvasView::on_preview_option)
	);
	action_group->add( Gtk::Action::create("options", _("Options...")),
		sigc::mem_fun0(canvas_options,&CanvasOptions::present)
	);
	action_group->add( Gtk::Action::create("close-document", Gtk::StockID("gtk-close"), _("Close Document")),
		sigc::hide_return(sigc::mem_fun(*this,&CanvasView::close_instance))
	);
	action_group->add( Gtk::Action::create("show-dependencies", _("Show Dependencies...")),
		sigc::hide_return(sigc::mem_fun(*this,&CanvasView::show_dependencies))
	);
	action_group->add( Gtk::Action::create("quit", Gtk::StockID("gtk-quit"), _("Quit")),
		sigc::hide_return(sigc::ptr_fun(&App::quit))
	);

	action_group->add( Gtk::Action::create("select-all-ducks", _("Select All Handles")),
		sigc::mem_fun(*work_area,&WorkArea::select_all_ducks)
	);

	action_group->add( Gtk::Action::create("unselect-all-ducks", _("Unselect All Handles")),
		sigc::mem_fun(*work_area,&WorkArea::unselect_all_ducks)
	);

	action_group->add( Gtk::Action::create("select-all-layers", _("Select All Layers")),
		sigc::mem_fun(*this,&CanvasView::on_select_layers)
	);

	action_group->add( Gtk::Action::create("unselect-all-layers", _("Unselect All Layers")),
		sigc::mem_fun(*this,&CanvasView::on_unselect_layers)
	);

	action_group->add( Gtk::Action::create("select-parent-layer", _("Select Parent Layer")),
		sigc::mem_fun(*this,&CanvasView::on_select_parent_layer)
	);

	action_group->add( Gtk::Action::create("pause", Gtk::StockID("synfig-animate_pause")),
		sigc::mem_fun(*this, &CanvasView::stop_async)
	);

	action_group->add( Gtk::Action::create("refresh", Gtk::StockID("gtk-refresh")),
		SLOT_EVENT(EVENT_REFRESH)
	);

	action_group->add( Gtk::Action::create("properties", Gtk::StockID("gtk-properties"), _("Properties...")),
		sigc::mem_fun0(canvas_properties,&CanvasProperties::present)
	);

    auto instance = get_instance().get();
	for ( const auto& plugin : App::plugin_manager.plugins() )
    {
		std::string id = plugin.id;
		action_group->add(
			Gtk::Action::create(id, plugin.name.get()),
			[instance, id](){instance->run_plugin(id, true);}
        );
    }

	// Low-Res Quality Menu
	for(std::list<int>::iterator i = get_pixel_sizes().begin(); i != get_pixel_sizes().end(); ++i) {
		Glib::RefPtr<Gtk::RadioAction> action = Gtk::RadioAction::create(
			low_res_pixel_size_group,
			synfig::strprintf("lowres-pixel-%d", *i),
			synfig::strprintf(_("Set Low-Res pixel size to %d"), *i) );
		if (*i == 2) { // default pixel size
			action->set_active();
			work_area->set_low_res_pixel_size(*i);
		}
		action_group->add(
			action,
			sigc::bind(sigc::mem_fun(*work_area, &WorkArea::set_low_res_pixel_size), *i) );
	}
	action_group->add(
		Gtk::Action::create("decrease-low-res-pixel-size", _("Decrease Low-Res Pixel Size")),
		sigc::mem_fun(this, &CanvasView::decrease_low_res_pixel_size) );
	action_group->add(
		Gtk::Action::create("increase-low-res-pixel-size",  _("Increase Low-Res Pixel Size")),
		sigc::mem_fun(this, &CanvasView::increase_low_res_pixel_size) );


	action_group->add(
		Gtk::Action::create("play", Gtk::Stock::MEDIA_PLAY),
		sigc::mem_fun(*this, &CanvasView::on_play_pause_pressed) );
	action_group->add(
		Gtk::Action::create("dialog-flipbook", _("Preview Window")),
		sigc::mem_fun0(preview_dialog, &Dialog_Preview::present) );

	// Prevent call to preview window before preview option has created the preview window
	action_group->get_action("dialog-flipbook")->set_sensitive(false);

	{
		Glib::RefPtr<Gtk::ToggleAction> action;

		rulers_show_toggle = Gtk::ToggleAction::create("toggle-rulers-show", _("Show Rulers"));
		rulers_show_toggle->set_active(work_area->get_show_rulers());
		work_area->set_show_rulers(work_area->get_show_rulers());
		action_group->add(rulers_show_toggle, sigc::mem_fun(*this, &CanvasView::toggle_show_ruler));

		grid_show_toggle = Gtk::ToggleAction::create("toggle-grid-show", _("Show Grid"));
		grid_show_toggle->set_active(work_area->grid_status());
		action_group->add(grid_show_toggle, sigc::mem_fun(*this, &CanvasView::toggle_show_grid));

		grid_snap_toggle = Gtk::ToggleAction::create("toggle-grid-snap", _("Snap to Grid"));
		grid_snap_toggle->set_active(work_area->get_grid_snap());
		action_group->add(grid_snap_toggle, sigc::mem_fun(*this, &CanvasView::toggle_snap_grid));

		guides_show_toggle = Gtk::ToggleAction::create("toggle-guide-show", _("Show Guides"));
		guides_show_toggle->set_active(work_area->get_show_guides());
		action_group->add(guides_show_toggle, sigc::mem_fun(*this, &CanvasView::toggle_show_guides));

		guides_snap_toggle = Gtk::ToggleAction::create("toggle-guide-snap", _("Snap to Guides"));
		guides_snap_toggle->set_active(work_area->get_guide_snap());
		action_group->add(guides_snap_toggle, sigc::mem_fun(*this, &CanvasView::toggle_snap_guides));

		action = Gtk::ToggleAction::create("toggle-low-res", _("Use Low-Res"));
		action->set_active(work_area->get_low_resolution_flag());
		action_group->add(action, sigc::mem_fun(*this, &CanvasView::toggle_low_res_pixel_flag));

		background_rendering_toggle = Gtk::ToggleAction::create("toggle-background-rendering", _("Enable rendering in background"));
		background_rendering_toggle->set_active(work_area->get_background_rendering());
		action_group->add(background_rendering_toggle, sigc::mem_fun(*this, &CanvasView::toggle_background_rendering));

		onion_skin_toggle = Gtk::ToggleAction::create("toggle-onion-skin", _("Show Onion Skin"));
		onion_skin_toggle->set_active(work_area->get_onion_skin());
		action_group->add(onion_skin_toggle, sigc::mem_fun(*this, &CanvasView::toggle_onion_skin));

		onion_skin_keyframes_toggle = Gtk::ToggleAction::create("toggle-onion-skin-keyframes", _("Onion Skin on Keyframes"));
		onion_skin_keyframes_toggle->set_active(work_area->get_onion_skin_keyframes());
		action_group->add(onion_skin_keyframes_toggle, sigc::mem_fun(*this, &CanvasView::toggle_onion_skin_keyframes));
	}

	action_group->add(
		Gtk::Action::create("canvas-zoom-fit", Gtk::StockID("gtk-zoom-fit")),
		sigc::mem_fun(*work_area, &WorkArea::zoom_fit) );
	action_group->add(
		Gtk::Action::create("canvas-zoom-100", Gtk::StockID("gtk-zoom-100")),
		sigc::mem_fun(*work_area, &WorkArea::zoom_norm) );
	action_group->add(
		Gtk::Action::create("canvas-zoom-fit-2", Gtk::StockID("gtk-zoom-fit")),
		sigc::mem_fun(*work_area, &WorkArea::zoom_fit) );		

	{
		Glib::RefPtr<Gtk::Action> action;

		action=Gtk::Action::create("seek-next-frame", Gtk::StockID("synfig-animate_seek_next_frame"));
		action_group->add(action,sigc::bind(sigc::mem_fun(*canvas_interface().get(), &CanvasInterface::seek_frame),1));
		action=Gtk::Action::create("seek-prev-frame", Gtk::StockID("synfig-animate_seek_prev_frame"));
		action_group->add( action, sigc::bind(sigc::mem_fun(*canvas_interface().get(), &CanvasInterface::seek_frame),-1));

		action=Gtk::Action::create("seek-next-second", Gtk::Stock::GO_FORWARD,_("Seek Forward"),_("Seek Forward"));
		action_group->add(action,sigc::bind(sigc::mem_fun(*canvas_interface().get(), &CanvasInterface::seek_time),Time(1)));
		action=Gtk::Action::create("seek-prev-second", Gtk::Stock::GO_BACK,_("Seek Backward"),_("Seek Backward"));
		action_group->add( action, sigc::bind(sigc::mem_fun(*canvas_interface().get(), &CanvasInterface::seek_time),Time(-1)));

		action=Gtk::Action::create("seek-end", Gtk::StockID("synfig-animate_seek_end"));
		action_group->add(action, sigc::mem_fun(*this, &CanvasView::on_seek_end_pressed));

		action=Gtk::Action::create("seek-begin", Gtk::StockID("synfig-animate_seek_begin"));
		action_group->add( action, sigc::mem_fun(*this, &CanvasView::on_seek_begin_pressed));

		action=Gtk::Action::create("jump-next-keyframe", Gtk::StockID("synfig-animate_seek_next_keyframe"));
		action_group->add( action,sigc::mem_fun(*canvas_interface().get(), &CanvasInterface::jump_to_next_keyframe));

		action=Gtk::Action::create("jump-prev-keyframe", Gtk::StockID("synfig-animate_seek_prev_keyframe"));
		action_group->add( action,sigc::mem_fun(*canvas_interface().get(), &CanvasInterface::jump_to_prev_keyframe));

		action=Gtk::Action::create("canvas-zoom-in", Gtk::Stock::ZOOM_IN);
		action_group->add( action,sigc::mem_fun(*work_area, &WorkArea::zoom_in));
		
		action=Gtk::Action::create("canvas-zoom-in-2", Gtk::Stock::ZOOM_IN);
		action_group->add( action,sigc::mem_fun(*work_area, &WorkArea::zoom_in));		

		action=Gtk::Action::create("canvas-zoom-out", Gtk::Stock::ZOOM_OUT);
		action_group->add( action, sigc::mem_fun(*work_area, &WorkArea::zoom_out) );
		
		action=Gtk::Action::create("canvas-zoom-out-2", Gtk::Stock::ZOOM_OUT);
		action_group->add( action, sigc::mem_fun(*work_area, &WorkArea::zoom_out) );		

		action=Gtk::Action::create("time-zoom-in", Gtk::Stock::ZOOM_IN, _("Zoom In on Timeline"));
		action_group->add( action, sigc::mem_fun(*this, &CanvasView::time_zoom_in) );

		action=Gtk::Action::create("time-zoom-out", Gtk::Stock::ZOOM_OUT, _("Zoom Out on Timeline"));
		action_group->add( action, sigc::mem_fun(*this, &CanvasView::time_zoom_out) );

	}

	{
		Glib::RefPtr<Gtk::ToggleAction> action;
		//! toggle none/last visible
		action= Gtk::ToggleAction::create("mask-none-ducks", _("Toggle None/Last visible Handles"));
		action->set_active(false);
		action_group->add(action,  sigc::mem_fun(*this,&CanvasView::toggle_duck_mask_all));

#define DUCK_MASK(lower,upper,string)												\
		action=Gtk::ToggleAction::create("mask-" #lower "-ducks", string);			\
		action->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_##upper));	\
		action_group->add(action,													\
			sigc::bind(																\
				sigc::mem_fun(*this, &CanvasView::toggle_duck_mask),		\
				Duck::TYPE_##upper))

		DUCK_MASK(position,POSITION,_("Show Position Handles"));
		DUCK_MASK(tangent,TANGENT,_("Show Tangent Handles"));
		DUCK_MASK(vertex,VERTEX,_("Show Vertex Handles"));
		DUCK_MASK(radius,RADIUS,_("Show Radius Handles"));
		DUCK_MASK(width,WIDTH,_("Show Width Handles"));
		DUCK_MASK(widthpoint-position, WIDTHPOINT_POSITION, _("Show WidthPoints Position Handles"));
		DUCK_MASK(angle,ANGLE,_("Show Angle Handles"));
		action_mask_bone_setup_ducks = action;
		DUCK_MASK(bone-recursive,BONE_RECURSIVE,_("Show Recursive Scale Bone Handles"));
		action_mask_bone_recursive_ducks = action;

#undef DUCK_MASK

		action_group->add(Gtk::Action::create("mask-bone-ducks", _("Next Bone Handles")),
						  sigc::mem_fun(*this,&CanvasView::mask_bone_ducks));
	}

}

void
CanvasView::on_select_layers()
{
	Canvas::Handle canvas(get_canvas());
	for (CanvasBase::iterator iter = canvas->begin(); iter != canvas->end(); iter++)
		layer_tree->select_all_children_layers(*iter);
}

void
CanvasView::on_unselect_layers()
{
	layer_tree->clear_selected_layers();
}

void
CanvasView::on_select_parent_layer()
{
	Layer::Handle layer = get_selection_manager()->get_selected_layer();
	if (!layer)
		return;

	Layer::Handle parent_layer = layer->get_parent_paste_canvas_layer();
	if (!parent_layer)
		return;

	get_selection_manager()->clear_selected_layers();
	get_selection_manager()->set_selected_layer(parent_layer);
}

void
CanvasView::add_layer(String x)
{
	Canvas::Handle canvas;
	SelectionManager::LayerList layer_list(get_selection_manager()->get_selected_layers());
	int target_depth(0);
	Layer::Handle layer;

	if (layer_list.empty()) {
		canvas = get_canvas();
	} 
	else 
	{
		canvas = layer_list.front()->get_canvas();
		target_depth=canvas->get_depth(*layer_list.begin());
	}
	// check if import or sound layer then show an input dialog window
	if(x=="import"||x=="sound")
	{
		String filename;
		bool selected = false;
		if (x == "sound") {
			selected = App::dialog_open_file_audio(_("Please choose an audio file"), filename, ANIMATION_DIR_PREFERENCE);
		} else {
			selected = App::dialog_open_file_image(_("Please choose an image file"), filename, IMAGE_DIR_PREFERENCE);
		}
		if (selected)
		{
			String errors, warnings;
			layer = canvas_interface()->import(filename, errors, warnings, App::resize_imported_images);
			if (!warnings.empty()) {
				App::dialog_message_1b("WARNING", synfig::strprintf("%s:\n\n%s", _("Warning"), warnings.c_str()),
					"details",	_("Close"));
			}
		}		
	}
	else
	{
		layer = canvas_interface()->add_layer_to(x,canvas,target_depth);
	}

	if(layer)
	{
		get_selection_manager()->clear_selected_layers();
		get_selection_manager()->set_selected_layer(layer);
	}
}

void
CanvasView::popup_layer_menu(Layer::Handle layer)
{
	if (!layer)
		return;

	Gtk::Menu* menu(&parammenu);
	std::vector<Widget*> children = menu->get_children();
	for(std::vector<Widget*>::iterator i = children.begin(); i != children.end(); ++i)
		menu->remove(**i);

	Action::ParamList param_list;
	param_list.add("time",canvas_interface()->get_time());
	param_list.add("canvas",Canvas::Handle(layer->get_canvas()));
	param_list.add("canvas_interface",canvas_interface());

	SelectionManager::LayerList layer_list = get_selection_manager()->get_selected_layers();

	auto found = std::find(layer_list.cbegin(), layer_list.cend(), layer);
	if (found == layer_list.cend())	{
		// if the layer is not in the list of already selected layers, then clear selection and select it
		get_selection_manager()->clear_selected_layers();
		get_selection_manager()->set_selected_layer(layer);
		param_list.add("layer", layer);
	} else {
		for (auto& layer_handle : layer_list) {
			param_list.add("layer", layer_handle);
		}
	}

	//Gtk::Menu *newlayers(manage(new Gtk::Menu()));
	//build_new_layer_menu(*newlayers);

	//parammenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("New Layer"),*newlayers));

	if(etl::handle<Layer_PasteCanvas>::cast_dynamic(layer))
	{
		Gtk::MenuItem *item = manage(new Gtk::ImageMenuItem(
			*manage(create_image_from_icon("select_all_child_layers_icon", Gtk::ICON_SIZE_MENU)),
			_("Select All Children") ));
		item->signal_activate().connect(
			sigc::bind(
				sigc::mem_fun(
					*layer_tree,
					&LayerTree::select_all_children_layers ),
				layer ));
		item->show_all();
		menu->append(*item);
	}

	add_actions_to_menu(menu, param_list,Action::CATEGORY_LAYER);
	get_instance()->add_special_layer_actions_to_menu(menu, layer);

	menu->popup(3,gtk_get_current_event_time());
}

void
CanvasView::register_layer_type(Layer::Book::value_type &/*lyr*/,std::map<String,Gtk::Menu*>* /*category_map*/)
{
/*	if(lyr.second.category==CATEGORY_DO_NOT_USE)
		return;

	if(category_map->count(lyr.second.category)==0)
		(*category_map)[lyr.second.category]=manage(new Gtk::Menu());

	(*category_map)[lyr.second.category]->items().push_back(Gtk::Menu_Helpers::MenuElem(lyr.second.local_name,
		sigc::hide_return(
			sigc::bind(
				sigc::mem_fun(*this,&CanvasView::add_layer),
				lyr.first
			)
		)
	));
*/
}

void
CanvasView::build_new_layer_menu(Gtk::Menu &/*menu*/)
{
/*
	std::map<String,Gtk::Menu*> category_map;

	std::for_each(
		Layer::book().begin(),
		Layer::book().end(),
		sigc::bind(
			sigc::mem_fun(
				*this,
				&CanvasView::register_layer_type
			),
			&category_map
		)
	);

	menu.items().clear();
	menu.items().push_back(Gtk::Menu_Helpers::TearoffMenuElem());

	std::map<String,Gtk::Menu*>::iterator iter;
	for(iter=category_map.begin();iter!=category_map.end();++iter)
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(iter->first,*iter->second));

	menu.show();
*/
}

void
CanvasView::popup_main_menu()
{
	Gtk::Menu* menu = dynamic_cast<Gtk::Menu*>(App::ui_manager()->get_widget("/menu-main"));
	if(menu)
	{
		//menu->set_accel_group(App::ui_manager()->get_accel_group());
		//menu->accelerate(*this);
		menu->popup(0,gtk_get_current_event_time());
	}
}

void
CanvasView::on_refresh_pressed()
{
	rebuild_tables();
	rebuild_ducks();
	work_area->queue_render();
}

void
CanvasView::workarea_layer_selected(Layer::Handle layer)
{
	get_selection_manager()->clear_selected_layers();
	if(layer)
		get_selection_manager()->set_selected_layer(layer);
}

void
CanvasView::refresh_rend_desc()
{
	Time begin_time =get_canvas()->rend_desc().get_time_start();
	Time end_time = get_canvas()->rend_desc().get_time_end();
	float current_frame_rate = get_canvas()->rend_desc().get_frame_rate();

	// "responsive" current time widget width on time format
	const int current_time_min_lenght = 6;
	int current_time_lenght = current_time_widget->get_value().get_string(current_frame_rate, App::get_time_format()).length();
	current_time_lenght = current_time_lenght < current_time_min_lenght ? current_time_min_lenght : current_time_lenght;
	current_time_widget->set_width_chars(current_time_lenght);

	current_time_widget->set_fps(current_frame_rate);
	jackdial->set_fps(current_frame_rate);

	time_model()->set_bounds(begin_time, end_time, current_frame_rate);
	time_model()->set_visible_bounds(
		time_model()->get_time() - Time(DEFAULT_TIME_WINDOW_SIZE)*0.5,
		time_model()->get_time() + Time(DEFAULT_TIME_WINDOW_SIZE)*0.5 );

	//Update end time widget values
	framedial->set_end_time(get_canvas()->rend_desc().get_frame_rate(), get_canvas()->rend_desc().get_time_end());
	framedial->on_end_time_widget_changed();

	//if (begin_time == end_time) hide_timebar(); else show_timebar();

	work_area->queue_render();
}

bool
CanvasView::close_view()
{
	//prevent double click
	closebutton->set_sensitive(false);

	if(get_instance()->get_visible_canvases()==1)
		close_instance();
	else
		hide();
	return false;
}

static bool _close_instance(etl::handle<studio::Instance> instance)
{
	if (instance->safe_close())
		info("closed");
	return false;
}

bool
CanvasView::close_instance()
{
	Glib::signal_timeout().connect(
		sigc::bind(
			sigc::ptr_fun(_close_instance),
			(etl::handle<studio::Instance>)get_instance() ),
		250 );
	return false;
}

etl::handle<CanvasView>
CanvasView::create(etl::loose_handle<studio::Instance> instance, etl::handle<Canvas> canvas)
	{ return new CanvasView(instance,instance->Instance::find_canvas_interface(canvas)); }

void
CanvasView::update_title()
{
	bool modified = get_instance()->get_action_count() > 0;
	bool is_root = get_canvas()->is_root();
	String filename = get_instance()->has_real_filename()
					? etl::basename(get_instance()->get_file_name()) : "";
	String canvas_name = get_canvas()->get_name();
	String canvas_id = get_canvas()->get_id();
	String &canvas_title = canvas_name.empty() ? canvas_id : canvas_name;

	String title = filename.empty() ? canvas_title
			     : is_root ? filename
			     : filename + " (" + canvas_title + ")";
	if (modified) title = "*" + title;

	set_local_name(title);
	App::dock_manager->update_window_titles();
}

void
CanvasView::on_hide()
{
	smach_.egress();
	Dockable::on_hide();
}

Gtk::Widget*
CanvasView::create_tab_label()
{
	Gtk::EventBox* event_box(manage(new Gtk::EventBox()));

	attach_dnd_to(*event_box);

	Glib::ustring text(get_local_name());

	Gtk::Grid* grid(manage(new Gtk::Grid()));
	event_box->add(*grid);
	grid->set_hexpand(false);
	grid->show();

	Gtk::Label* label(manage(new Gtk::Label(text)));
	label->set_halign(Gtk::ALIGN_START);
	label->set_hexpand();
	grid->attach(*label, 0, 0, 1, 1);
	if (this == App::get_selected_canvas_view().get())
	{
		Pango::AttrList list;
		Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
		list.insert(attr);
		label->set_attributes(list);
	}
	label->show();

	closebutton = manage(new Gtk::Button());
	// Margin is needed to make sure the button is not partially hidden with Adwaita theme
	closebutton->set_margin_end(4);
	grid->attach(*closebutton, 1, 0, 1, 1);
	closebutton->set_image_from_icon_name("window-close");
	closebutton->signal_clicked().connect(
		sigc::hide_return(sigc::mem_fun(*this,&CanvasView::close_view)));
	closebutton->set_relief(Gtk::RELIEF_NONE);
	closebutton->show_all();

	return event_box;
}

bool
CanvasView::on_button_press_event(GdkEventButton * /* event */)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	if (this != App::get_selected_canvas_view())
		App::set_selected_canvas_view(this);
	return false;
	//return Dockable::on_button_press_event(event);
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool
CanvasView::on_key_press_event(GdkEventKey* event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	Gtk::Widget* focused_widget = App::main_window->get_focus();
	if(focused_widget && focused_widget_has_priority(focused_widget))
	{
		if(focused_widget->event((GdkEvent*)event))
			return true;
	}
	else if(Dockable::on_key_press_event(event))
			return true;
		else
			if (focused_widget) {
				if (focused_widget->event((GdkEvent*)event))
					return true;
			}

	if (event->type == GDK_KEY_PRESS) {
		switch (event->keyval) {
		case GDK_KEY_Home:
		case GDK_KEY_KP_Home:
			action_group->get_action("seek-begin")->activate();
			return  true;
		case GDK_KEY_End:
		case GDK_KEY_KP_End:
			action_group->get_action("seek-end")->activate();
			return  true;
		}
	}
	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool
CanvasView::focused_widget_has_priority(Gtk::Widget * focused)
{
	if(dynamic_cast<Gtk::Entry*>(focused))
		return true;
	return false;
}

void
CanvasView::refresh_tables()
{
//	if(layer_tree_store_)layer_tree_store_->refresh();
//	if(children_tree_store_)children_tree_store_->refresh();
}

void
CanvasView::rebuild_tables()
{
//	layer_tree_store_->rebuild();
//	children_tree_store_->rebuild();
}

void
CanvasView::build_tables()
{
//	layer_tree_store_->rebuild();
//	children_tree_store_->rebuild();
}

void
CanvasView::on_layer_toggle(Layer::Handle layer)
{
	Action::Handle action(Action::create("LayerActivate"));
	assert(action);

	if(!action)
		return;

	action->set_param("canvas",Canvas::Handle(layer->get_canvas()));
	if(!action->set_param("canvas_interface",canvas_interface()))
//	if(!action->set_param("canvas_interface",get_instance()->find_canvas_interface(layer->get_canvas())))
		error("LayerActivate didn't like CanvasInterface...?");
	action->set_param("time",get_time());
	action->set_param("layer",layer);
	action->set_param("new_status",!layer->active());

	assert(action->is_ready());

	canvas_interface()->get_instance()->perform_action(action);
}

void
CanvasView::popup_param_menu(ValueDesc value_desc, float location, bool bezier)
{
	std::vector<Widget*> children = parammenu.get_children();
	for(std::vector<Widget*>::iterator i = children.begin(); i != children.end(); ++i)
		parammenu.remove(**i);
	get_instance()->make_param_menu(&parammenu,get_canvas(),value_desc,location,bezier);
	parammenu.popup(3,gtk_get_current_event_time());
}

void
CanvasView::create_new_vertex_on_bline(float location, synfigapp::ValueDesc value_desc)
{
	synfigapp::Action::Handle action = synfigapp::Action::create("ValueNodeDynamicListInsertSmartKeepShape");
	auto param_list = canvas_interface()->generate_param_list(value_desc);
	param_list.add("origin", location);
	action->set_param_list(param_list);
	if (action->is_ready())
		canvas_interface()->get_instance()->perform_action(action);
}

void
CanvasView::add_actions_to_menu(Gtk::Menu *menu, const Action::ParamList &param_list,Action::Category category)const
{
	get_instance()->add_actions_to_menu(menu, param_list, category);
}


bool
CanvasView::on_layer_user_click(int button, Gtk::TreeRow /*row*/, LayerTree::ColumnID /*column_id*/)
{
	switch(button)
	{
	case 3:
		{
			Gtk::MenuItem* menu = dynamic_cast<Gtk::MenuItem*>(App::ui_manager()->get_widget("/menu-main/menu-layer"));
			if(menu && menu->get_submenu())
			{
				//menu->set_accel_group(App::ui_manager()->get_accel_group());
				//menu->accelerate(*this);
			#if GTK_CHECK_VERSION(3, 22, 0)
				menu->get_submenu()->popup_at_pointer(nullptr);
			#else
				menu->get_submenu()->popup(button,gtk_get_current_event_time());
			#endif
			}

			#if 0
			bool multiple_selected=true;

			if(layer_tree->get_selection()->count_selected_rows()<=1)
				multiple_selected=false;

			// If the clicked row is not selected, then unselect
			// everything that isn't selected and select this row
			if(multiple_selected && !layer_tree->get_selection()->is_selected(row))
			{
				layer_tree->get_selection()->unselect_all();
				layer_tree->get_selection()->select(row);
				multiple_selected=false;
			}

			if(column_id==COLUMNID_TIME_TRACK)
				return false;

			//ValueDesc value_desc(row[layer_param_tree_model.value_desc]);
			//ValueNode::Handle value_node(row[layer_param_tree_model.value_node]);
			//ValueNode::Handle parent_value_node;
			//ValueBase value=row[layer_param_tree_model.value];

			//if(row.parent())
			//{
			//	parent_value_node=(*row.parent())[layer_tree_model.value_node];
			//}

			{
				Layer::Handle layer(row[layer_tree_model.layer]);
				Action::ParamList param_list;
				param_list.add("time",canvas_interface()->get_time());
				param_list.add("canvas",Canvas::Handle(row[layer_tree_model.canvas]));
				param_list.add("canvas_interface",canvas_interface());
				if(!multiple_selected)
					param_list.add("layer",layer);
				else
				{
					SelectionManager::LayerList layer_list(get_selection_manager()->get_selected_layers());
					SelectionManager::LayerList::iterator iter;

					for(iter=layer_list.begin();iter!=layer_list.end();++iter)
						param_list.add("layer",Layer::Handle(*iter));
				}

				parammenu.items().clear();

				Gtk::Menu *newlayers(manage(new Gtk::Menu()));
				// do we need this?  the code is all #ifdef'ed out anyway
				// newlayers->signal_hide().connect(sigc::bind(sigc::ptr_fun(&delete_widget), newlayers));
				build_new_layer_menu(*newlayers);

				parammenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("New Layer"),*newlayers));
				if(!multiple_selected && etl::handle<Layer_PasteCanvas>::cast_dynamic(layer))
				{
					parammenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Select All Children"),
						sigc::bind(
							sigc::mem_fun(
								*layer_tree,
								&LayerTree::select_all_children_layers
							),
							layer
						)
					));
				}

				add_actions_to_menu(&parammenu, param_list,Action::CATEGORY_LAYER);
				parammenu.popup(button,gtk_get_current_event_time());
				return true;
			}
/*
			else if(column_id==LayerTree::COLUMNID_TIME_TRACK && value_node && handle<ValueNode_Animated>::cast_dynamic(value_node))
			{
				// Right-click on time track with animated
//				trackmenu.popup(0,0);
				return true;
			}
			else
			{
				if(!multiple_selected)
				{
					popup_param_menu(value_desc);
					return true;
				}
				else
				{
#warning update me!
#if 0
					parammenu.items().clear();
					parammenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Connect"),
						hide_return(sigc::mem_fun(*canvas_interface().get(),&CanvasInterface::connect_selected_layer_params))
					));
					parammenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Disconnect"),
						hide_return(sigc::mem_fun(*canvas_interface().get(),&CanvasInterface::disconnect_selected_layer_params))
					));
					parammenu.popup(0,0);
#endif
				}
				return true;
			}
		*/
#endif
}
		return true;

	default:
		break;
	}
	return false;
}

bool
CanvasView::on_children_user_click(int button, Gtk::TreeRow row, ChildrenTree::ColumnID column_id)
{
	switch(button)
	{
	case 3:
		{
			if(column_id==COLUMNID_TIME_TRACK)
				return false;
			if(!(bool)row[children_tree_model.is_canvas])
			{
				ValueDesc value_desc=row[children_tree_model.value_desc];
				if (!value_desc)
				{
					//! \todo fix properly -- what is the child dialog for?
					info("preventing child dialog right-click crash");
					return true;
				}
				assert(value_desc);
				popup_param_menu(value_desc);
				return true;
			}
		}
		return true;

	default:
		break;
	}
	return false;
}

bool
CanvasView::on_keyframe_tree_event(GdkEvent *event)
{
	if ( event->type == GDK_BUTTON_PRESS
	  && event->button.button == 3 )
	{
		//keyframemenu.popup(event->button.button,gtk_get_current_event_time());
		return true;
	}
	return false;
}

void
CanvasView::refresh_time_window()
{
	framedial->toggle_repeat(time_model()->get_play_repeat());
	framedial->toggle_bounds_enable(time_model()->get_play_bounds_enabled());

	//NOTE THIS SHOULD HOOK INTO THE CORRECT SIGNALS...
	if (children_tree)
		children_tree->queue_draw();
}

void
CanvasView::on_interface_time_changed()
	{ time_model()->set_time(canvas_interface_->get_time()); }

void
CanvasView::time_zoom_in()
	{ time_model()->zoom(1.0/0.75); }

void
CanvasView::time_zoom_out()
	{ time_model()->zoom(0.75); }

void
CanvasView::on_time_changed()
{
	Time time = time_model()->get_time();

	if (!is_playing() && canvas_interface_->get_time() != time)
		canvas_interface_->set_time(time);
	else {
		work_area->queue_draw();
	}

	if (!time_model()->almost_equal_to_current(soundProcessor.get_position(), Time(0.5)))
		soundProcessor.set_position(time);

	#ifdef WITH_JACK
	if ( jack_enabled
	 && !jack_synchronizing
	 && !time_model()->almost_equal_to_current(jack_time - get_jack_offset()) )
	{
		jack_nframes_t sr = jack_get_sample_rate(jack_client);
		jack_nframes_t nframes = (jack_nframes_t)((time + get_jack_offset())*(double)sr);
		jack_transport_locate(jack_client, nframes);
	}
	#endif

	current_time_widget->set_value(time);
	if (!is_playing())
	{
		KeyframeList::iterator iter;
		if (get_canvas()->keyframe_list().find(time, iter)) {
			// Widget::override_color() is deprecated since Gtkmm 3.16: Use a custom style provider and style classes instead.
			// This function is very slow!
			current_time_widget->override_color(Gdk::RGBA("#FF0000"));
		} else {
			// Widget::override_color() is deprecated since Gtkmm 3.16: Use a custom style provider and style classes instead.
			// This function is very slow!
			current_time_widget->override_color(Gdk::RGBA(0));
		}

		// Shouldn't these trees just hook into
		// the time changed signal...?
		if (layer_tree) layer_tree->queue_draw();
		if (children_tree) children_tree->queue_draw();
		// Do we need this here?
		queue_rebuild_ducks();
	}
}

void
CanvasView::on_edited_value(ValueDesc value_desc,ValueBase new_value)
	{ canvas_interface()->change_value(value_desc,new_value); }

void
CanvasView::on_id_changed()
	{ update_title(); }

void
CanvasView::on_mode_changed(CanvasInterface::Mode mode)
{
	if(toggling_animate_mode_)
		return;
	toggling_animate_mode_=true;
	// If the animate flag was set in mode...
	if(mode&MODE_ANIMATE)
	{
		animatebutton->set_image_from_icon_name("animate_mode_on_icon");
		animatebutton->set_tooltip_text(_("Turn off animate editing mode"));
		animatebutton->set_active(true);
	}
	else
	{
		animatebutton->set_image_from_icon_name("animate_mode_off_icon");
		animatebutton->set_tooltip_text(_("Turn on animate editing mode"));
		animatebutton->set_active(false);
	}
	//Keyframe lock icons
	keyframedial->on_mode_changed(mode);

	work_area->queue_draw();
	toggling_animate_mode_=false;
}

void
CanvasView::toggle_animatebutton()
{
	if(toggling_animate_mode_)
		return;
	if(get_mode()&MODE_ANIMATE)
		set_mode(get_mode()-MODE_ANIMATE);
	else
		set_mode(get_mode()|MODE_ANIMATE);
}

void
CanvasView::toggle_timetrackbutton()
{
	if (timetrackbutton->get_active())
		timetrack->set_visible(true);
	else
		timetrack->set_visible(false);
}

void
CanvasView::toggle_past_keyframe_button()
{
	if(toggling_animate_mode_)
		return;
	CanvasInterface::Mode mode(get_mode());
	if((mode&MODE_ANIMATE_PAST) )
		set_mode(get_mode()-MODE_ANIMATE_PAST);
	else
		set_mode((get_mode()|MODE_ANIMATE_PAST));
}


void
CanvasView::toggle_future_keyframe_button()
{
	if(toggling_animate_mode_)
		return;
 	CanvasInterface::Mode mode(get_mode());
	if((mode&MODE_ANIMATE_FUTURE) )
		set_mode(get_mode()-MODE_ANIMATE_FUTURE);
	else
		set_mode(get_mode()|MODE_ANIMATE_FUTURE);
}

bool
CanvasView::duck_change_param(const Point &value,Layer::Handle layer, String param_name)
{
	return canvas_interface()->change_value(ValueDesc(layer,param_name),value);
}

void
CanvasView::selected_layer_color_set(Color color)
{
	SelectionManager::LayerList selected_list(get_selection_manager()->get_selected_layers());
	SelectionManager::LayerList::iterator iter;

	// Create the action group
	//PassiveGrouper group(canvas_interface()->get_instance(),_("Set Colors"));

	Layer::Handle layer;
	for(iter=selected_list.begin();iter!=selected_list.end();++iter)
	{
		if(*iter==layer)
			continue;
		layer=*iter;
		on_edited_value(ValueDesc(layer,"color"),color);
	}
}

void
CanvasView::queue_rebuild_ducks()
{
	queue_rebuild_ducks_connection.disconnect();

	if (is_ducks_locked())
		{ ducks_rebuild_queue_requested = true; return; }

	queue_rebuild_ducks_connection = Glib::signal_timeout().connect(
		sigc::bind_return(
			sigc::mem_fun(*this,&CanvasView::rebuild_ducks),
			false
		),
		50
	);
}

void
CanvasView::rebuild_ducks()
{
	if (is_ducks_locked())
		{ ducks_rebuild_requested = true; return; }

	ducks_rebuild_queue_requested = false;
	ducks_rebuild_requested = false;
	queue_rebuild_ducks_connection.disconnect();

	bbox = Rect::zero();
	work_area->clear_ducks();
	work_area->clear_curr_transform_stack();
	work_area->set_time(get_time());
	get_canvas()->set_time(get_time());

	// First do the layers...
	TransformStack transform_stack;
	SelectionManager::LayerList selected_layers(get_selection_manager()->get_selected_layers());
	std::set<Layer::Handle> layer_set(selected_layers.begin(), selected_layers.end());
	work_area->add_ducks_layers(get_canvas(), layer_set, this, transform_stack);

	// Now do the children
	transform_stack.clear();
	SelectionManager::ChildrenList selected_children = get_selection_manager()->get_selected_children();
	for(SelectionManager::ChildrenList::iterator i = selected_children.begin(); i != selected_children.end(); ++i)
		work_area->add_to_ducks(*i, this, transform_stack);
	work_area->refresh_selected_ducks();
	work_area->queue_draw();
}

void
CanvasView::decrease_low_res_pixel_size()
{
	if(changing_resolution_)
		return;
	changing_resolution_=true;
	std::list<int> sizes = CanvasView::get_pixel_sizes();
	int pixel_size = work_area->get_low_res_pixel_size();
	for (std::list<int>::iterator iter = sizes.begin(); iter != sizes.end(); ++iter)
		if (*iter == pixel_size) {
			if (iter == sizes.begin()) {
				// we already have the smallest low-res pixels possible - turn off low-res instead
				work_area->set_low_resolution_flag(false);
			} else {
				--iter;
				Glib::RefPtr<Gtk::Action> action = action_group->get_action(synfig::strprintf("lowres-pixel-%d", *iter));
				assert(action);
				action->activate(); // to make sure the radiobutton in the menu is updated too
				work_area->set_low_resolution_flag(true);
			}
			break;
		}
	// Update the "toggle-low-res" action
	Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("toggle-low-res"));
	action->set_active(work_area->get_low_resolution_flag());
	// Update toggle low res button
	resolutiondial_->update_lowres(work_area->get_low_resolution_flag());
	changing_resolution_=false;
}

void
CanvasView::increase_low_res_pixel_size()
{
	if(changing_resolution_)
		return;
	changing_resolution_=true;
	std::list<int> sizes = CanvasView::get_pixel_sizes();
	int pixel_size = work_area->get_low_res_pixel_size();
	if (!work_area->get_low_resolution_flag())
	{
		// We were using "hi res" so change it to low res.
		work_area->set_low_resolution_flag(true);
		// Update the "toggle-low-res" action
		Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("toggle-low-res"));
		action->set_active(true);
		// Update the toggle low res button
		resolutiondial_->update_lowres(true);
		changing_resolution_=false;
		return;
	}

	for (std::list<int>::iterator iter = sizes.begin(); iter != sizes.end(); iter++)
		if (*iter == pixel_size) {
			if (++iter != sizes.end()) {
				Glib::RefPtr<Gtk::Action> action = action_group->get_action(synfig::strprintf("lowres-pixel-%d", *iter));
				assert(action);
				action->activate(); // to make sure the radiobutton in the menu is updated too
				work_area->set_low_resolution_flag(true);
			}
			break;
		}
	// Update the "toggle-low-res" action
	Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("toggle-low-res"));
	action->set_active(work_area->get_low_resolution_flag());
	// Update toggle low res button
	resolutiondial_->update_lowres(work_area->get_low_resolution_flag());
	changing_resolution_=false;
}

void
CanvasView::toggle_low_res_pixel_flag()
{
	if(changing_resolution_)
		return;
	changing_resolution_=true;
	work_area->toggle_low_resolution_flag();
	// Update the toggle low res button
	resolutiondial_->update_lowres(work_area->get_low_resolution_flag());
	// Update the "toggle-low-res" action
	Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("toggle-low-res"));
	action->set_active(work_area->get_low_resolution_flag());
	changing_resolution_=false;
}

void
CanvasView::set_onion_skins()
{
	if(toggling_onion_skin)
		return;
	int onion_skins[2];
	onion_skins[0]=past_onion_spin->get_value();
	onion_skins[1]=future_onion_spin->get_value();
	work_area->set_onion_skins(onion_skins);
}

void
CanvasView::toggle_show_ruler()
{
	bool visible = !(work_area->get_show_rulers());
	work_area->set_show_rulers(visible);
}

void
CanvasView::toggle_show_grid()
{
	if(toggling_show_grid)
		return;
	toggling_show_grid=true;
	work_area->toggle_grid();
	// Update the toggle grid show action
	set_grid_show_toggle(work_area->grid_status());
	// Update the toggle grid show check button
	show_grid->set_active(work_area->grid_status());
	toggling_show_grid=false;
}

void
CanvasView::toggle_snap_grid()
{
	if(toggling_snap_grid)
		return;
	toggling_snap_grid=true;
	work_area->toggle_grid_snap();
	// Update the toggle grid snap action
	set_grid_snap_toggle(work_area->get_grid_snap());
	// Update the toggle grid snap check button
	snap_grid->set_active(work_area->get_grid_snap());
	toggling_snap_grid=false;
}

void
CanvasView::toggle_show_guides()
{
	if(toggling_show_guides)
		return;
	toggling_show_guides=true;
	work_area->toggle_show_guides();
	set_guides_show_toggle(work_area->get_show_guides());
	show_guides->set_active(work_area->get_show_guides());
	toggling_show_guides=false;
}

void
CanvasView::toggle_snap_guides()
{
	if(toggling_snap_guides)
		return;
	toggling_snap_guides=true;
	work_area->toggle_guide_snap();
	set_guides_snap_toggle(work_area->get_guide_snap());
	snap_guides->set_active(work_area->get_guide_snap());
	toggling_snap_guides=false;
}

void
CanvasView::toggle_onion_skin()
{
	if(toggling_onion_skin)
		return;
	toggling_onion_skin=true;
	work_area->set_onion_skin(!work_area->get_onion_skin());
	// Update the toggle onion skin action
	set_onion_skin_toggle(work_area->get_onion_skin());
	// Update the toggle onion skin button
	onion_skin->set_active(work_area->get_onion_skin());
	toggling_onion_skin=false;
}

void
CanvasView::toggle_onion_skin_keyframes()
{
	if(toggling_onion_skin_keyframes)
		return;
	toggling_onion_skin_keyframes=true;
	work_area->set_onion_skin_keyframes(!work_area->get_onion_skin_keyframes());
	set_onion_skin_keyframes_toggle(work_area->get_onion_skin_keyframes());
	onion_skin_keyframes->set_active(work_area->get_onion_skin_keyframes());
	toggling_onion_skin_keyframes=false;
}

void
CanvasView::toggle_background_rendering()
{
	if(toggling_background_rendering)
		return;
	toggling_background_rendering=true;
	work_area->set_background_rendering(!work_area->get_background_rendering());
	// Update the toggle background rendering action
	set_background_rendering_toggle(work_area->get_background_rendering());
	// Update the toggle background rendering button
	background_rendering_button->set_active(work_area->get_background_rendering());
	toggling_background_rendering=false;
}

void
CanvasView::on_dirty_preview()
{
	if (!is_playing()) {
		IsWorking is_working(*this);
		work_area->queue_render();
	}
}

void
CanvasView::play_async()
{
	if (is_playing()) return;

	playing_timer.reset();
	playing_time = time_model()->get_actual_play_time();

	// If we are already at the end of time, start over
	if (playing_time >= time_model()->get_actual_play_bounds_upper())
		playing_time = time_model()->get_actual_play_bounds_lower();

	ducks_playing_lock = new LockDucks(*this);

	work_area->clear_ducks();

	float fps = get_canvas()->rend_desc().get_frame_rate();
	int timeout = fps <= 0.f ? 0 : (int)roundf(500.f/fps);
	if (timeout < 10) timeout = 10;

	framedial->toggle_play_pause_button(is_playing());
	// Widget::override_color() is deprecated since Gtkmm 3.16: Use a custom style provider and style classes instead.
	// Also, this function is heavily slowdowns playback.
	//current_time_widget->override_color(Gdk::RGBA(0));

	soundProcessor.clear();
	canvas_interface()->get_canvas()->fill_sound_processor(soundProcessor);
	soundProcessor.set_position(playing_time);
	soundProcessor.set_playing(true);

	playing_connection = Glib::signal_timeout().connect(
		sigc::bind_return( sigc::mem_fun(*this, &CanvasView::on_play_timeout), true ),
		timeout,
		Glib::PRIORITY_LOW);
}

void
CanvasView::stop_async()
{
	playing_connection.disconnect();
	soundProcessor.set_playing(false);
	ducks_playing_lock.reset();
	framedial->toggle_play_pause_button(is_playing());

	on_time_changed();
}

void
CanvasView::on_play_timeout()
{
	// Used ifdef WITH_JACK
	bool repeat = time_model()->get_play_repeat();
	Time lower = time_model()->get_actual_play_bounds_lower();
	Time upper = time_model()->get_actual_play_bounds_upper();

	Time time;
	if (jack_enabled) {
		#ifdef WITH_JACK
		jack_position_t pos;
		jack_transport_query(jack_client, &pos);
		jack_time = Time((Time::value_type)pos.frame/(Time::value_type)pos.frame_rate);
		time = time_model()->round_time(jack_time - get_jack_offset());
		if (repeat) {
			if (time > upper) {
				time = time_model()->round_time(time - upper + lower);
				jack_nframes_t sr = jack_get_sample_rate(jack_client);
				jack_nframes_t nframes = (jack_nframes_t)round((double)(time + get_jack_offset())/(double)sr);
				jack_transport_locate(jack_client, nframes);
			}
		}
		time = synfig::clamp(time, lower, upper);
		#endif
	} else {
		time = time_model()->round_time(playing_time + playing_timer());
		if (repeat) {
			if (time > upper) {
				playing_time = lower;
				playing_timer.pop_time();
				time = time_model()->round_time(lower);
			}
		} else
		if (time >= upper) {
			time_model()->set_time(upper);
			stop_async();
			return;
		}
	}

	// scroll the time window so we can see the time value as it races across the horizon
	Time::value_type step = (Time::value_type)(time_model()->get_page_size())*0.5;
	if (time < time_model()->get_visible_lower()) {
		Time::value_type dist = (Time::value_type)(time_model()->get_visible_lower() - time);
		time_model()->move_by( -Time(ceil(dist/step)*step) );
	} else
	if (time > time_model()->get_visible_upper()) {
		Time::value_type dist = (Time::value_type)(time - time_model()->get_visible_upper());
		time_model()->move_by( Time(ceil(dist/step)*step) );
	}

	// update actual time to next step
	time_model()->set_time(time);

	work_area->sync_render(false);
}

void
CanvasView::show_timebar()
{
	timebar->show();

	if(layer_tree)
		layer_tree->set_show_timetrack(true);
	if(children_tree)
		children_tree->set_show_timetrack(true);
}

void
CanvasView::hide_timebar()
{
	timebar->hide();
	if(layer_tree)
		layer_tree->set_show_timetrack(false);
	if(children_tree)
		children_tree->set_show_timetrack(false);
}

void
CanvasView::set_sensitive_timebar(bool sensitive)
{
	timebar->set_sensitive(sensitive);
	if(layer_tree)
		layer_tree->set_sensitive(sensitive);
	if(children_tree)
		children_tree->set_sensitive(sensitive);
}

static void
set_waypoint_model(std::set<Waypoint, std::less<UniqueID> > waypoints,
				   Waypoint::Model model,
				   etl::loose_handle<CanvasInterface> canvas_interface)
{
	// Create the action group
	Action::PassiveGrouper group(canvas_interface->get_instance().get(),_("Change Waypoint Group"));

	std::set<Waypoint, std::less<UniqueID> >::const_iterator iter;
	for(iter=waypoints.begin();iter!=waypoints.end();++iter)
	{
		Waypoint waypoint(*iter);
		waypoint.apply_model(model);

		Action::Handle action(Action::create("WaypointSet"));

		assert(action);

		action->set_param("canvas",canvas_interface->get_canvas());
		action->set_param("canvas_interface",canvas_interface);

		action->set_param("waypoint",waypoint);
		action->set_param("value_node",waypoint.get_parent_value_node());

		if(!canvas_interface->get_instance()->perform_action(action))
		{
			group.cancel();
			return;
		}
	}
}

static void
duplicate_waypoints(std::set<Waypoint, std::less<UniqueID> > waypoints,
					etl::loose_handle<CanvasInterface> canvas_interface)
{
	// Create the action group
	Action::PassiveGrouper group(canvas_interface->get_instance().get(),_("Duplicate Waypoints"));

	std::set<Waypoint, std::less<UniqueID> >::const_iterator iter;
	for (iter = waypoints.begin(); iter != waypoints.end(); iter++)
	{
		Waypoint waypoint(*iter);
		ValueNode::Handle value_node(iter->get_parent_value_node());
		canvas_interface->waypoint_duplicate(value_node, waypoint);
	}
}

static void
remove_waypoints(std::set<Waypoint, std::less<UniqueID> > waypoints,
				 etl::loose_handle<CanvasInterface> canvas_interface)
{
	// Create the action group
	Action::PassiveGrouper group(canvas_interface->get_instance().get(),_("Remove Waypoints"));

	std::set<Waypoint, std::less<UniqueID> >::const_iterator iter;
	for (iter = waypoints.begin(); iter != waypoints.end(); iter++)
	{
		Waypoint waypoint(*iter);
		ValueNode::Handle value_node(iter->get_parent_value_node());
		canvas_interface->waypoint_remove(value_node, waypoint);
	}
}

void
CanvasView::on_waypoint_clicked_canvasview(ValueDesc value_desc,
										   std::set<Waypoint, std::less<UniqueID> > waypoint_set,
										   int button)
{
	int size = waypoint_set.size();
	Waypoint waypoint(*(waypoint_set.begin()));
	Time time(waypoint.get_time());

	if (size == 1)
	{
		waypoint_dialog.set_value_desc(value_desc);
		waypoint_dialog.set_waypoint(waypoint);
	}

	switch(button)
	{
	case -1:
		if (size == 1)
			waypoint_dialog.show();
		break;
	case 2:
	{
		Gtk::Menu* waypoint_menu(manage(new Gtk::Menu()));
		waypoint_menu->signal_hide().connect(sigc::bind(sigc::ptr_fun(&delete_widget), waypoint_menu));

		Gtk::Menu* interp_menu_in(manage(new Gtk::Menu()));
		Gtk::Menu* interp_menu_out(manage(new Gtk::Menu()));
		Gtk::MenuItem *item = nullptr;

		// ------------------------------------------------------------------------
		if (size == 1)
		{
			const ValueDesc value_desc(ValueNode_Animated::Handle::cast_reinterpret(waypoint.get_parent_value_node()), time);
			get_instance()->make_param_menu(waypoint_menu,canvas_interface()->get_canvas(),value_desc,0.5f);

			// ------------------------------------------------------------------------
			item = manage(new Gtk::SeparatorMenuItem());
			item->show();
			waypoint_menu->append(*item);
		}

		// ------------------------------------------------------------------------
		item = manage(new Gtk::MenuItem(_("_Jump To")));
		item->set_use_underline(true);
		item->signal_activate().connect(
			sigc::bind(sigc::mem_fun(*canvas_interface(), &CanvasInterface::set_time), time));
		item->show();
		waypoint_menu->append(*item);

		item = manage(new Gtk::MenuItem(_("_Duplicate")));
		item->set_use_underline(true);
		item->signal_activate().connect(
			sigc::bind(sigc::ptr_fun(duplicate_waypoints), waypoint_set, canvas_interface()));
		item->show();
		waypoint_menu->append(*item);

		item = manage(new Gtk::MenuItem(size == 1 ? _("_Remove") : synfig::strprintf(_("_Remove %d Waypoints"), size)));
		item->set_use_underline(true);
		item->signal_activate().connect(
			sigc::bind(sigc::ptr_fun(remove_waypoints), waypoint_set, canvas_interface()));
		item->show();
		waypoint_menu->append(*item);

		if (size == 1 && value_desc.is_valid())
		{
			item = manage(new Gtk::MenuItem(_("_Edit")));
			item->set_use_underline(true);
			item->signal_activate().connect(
					sigc::mem_fun(waypoint_dialog,&Gtk::Widget::show));
			item->show();
			waypoint_menu->append(*item);
		}

		// ------------------------------------------------------------------------

		item = manage(new Gtk::SeparatorMenuItem());
		item->show();
		waypoint_menu->append(*item);

		// ------------------------------------------------------------------------

		item = manage(new Gtk::MenuItem(_("_In")));
		item->set_use_underline(true);
		item->set_submenu(*interp_menu_in);
		item->show();
		waypoint_menu->append(*item);

		item = manage(new Gtk::MenuItem(_("_Out")));
		item->set_use_underline(true);
		item->set_submenu(*interp_menu_out);
		item->show();
		waypoint_menu->append(*item);
		
		// ------------------------------------------------------------------------
		
		item = manage(new Gtk::SeparatorMenuItem());
		item->show();
		waypoint_menu->append(*item);
		
		// ------------------------------------------------------------------------
		
		{
			Waypoint::Model model;
			Gtk::Image *image;

			#define APPEND_MENU_ITEM(menu, IconName, Text) \
				image = manage(create_image_from_icon(IconName, Gtk::IconSize::from_name("synfig-small_icon"))); \
				item = manage(new Gtk::ImageMenuItem( \
					*image, \
					_(Text) )); \
				item->set_use_underline(true); \
				item->signal_activate().connect( \
					sigc::bind(sigc::ptr_fun(set_waypoint_model), waypoint_set, model, canvas_interface())); \
				item->show_all(); \
				menu->append(*item);

			#define APPEND_ITEMS_TO_ALL_MENUS3(Interpolation, IconName, TextIn, TextOut, TextBoth) \
				model.reset(); \
				model.set_before(Interpolation); \
				APPEND_MENU_ITEM(interp_menu_in, IconName, TextIn) \
				model.reset(); \
				model.set_after(Interpolation); \
				APPEND_MENU_ITEM(interp_menu_out, IconName, TextOut) \
				model.set_before(Interpolation); \
				APPEND_MENU_ITEM(waypoint_menu, IconName, TextBoth)

			#define APPEND_ITEMS_TO_ALL_MENUS(Interpolation, IconName, Text) \
				APPEND_ITEMS_TO_ALL_MENUS3(Interpolation, IconName, Text, Text, Text)

			APPEND_ITEMS_TO_ALL_MENUS(INTERPOLATION_CLAMPED, "interpolation_type_clamped_icon", _("_Clamped"))
			APPEND_ITEMS_TO_ALL_MENUS(INTERPOLATION_TCB, "interpolation_type_tcb_icon", _("_TCB"))
			APPEND_ITEMS_TO_ALL_MENUS(INTERPOLATION_CONSTANT, "interpolation_type_const_icon", _("_Constant"))
			APPEND_ITEMS_TO_ALL_MENUS3(INTERPOLATION_HALT, "interpolation_type_ease_icon", _("_Ease In"), _("_Ease Out"), _("_Ease In/Out"))
			APPEND_ITEMS_TO_ALL_MENUS(INTERPOLATION_LINEAR, "interpolation_type_linear_icon", _("_Linear"))

			#undef APPEND_ITEMS_TO_ALL_MENUS
			#undef APPEND_ITEMS_TO_ALL_MENUS3
			#undef APPEND_MENU_ITEM
		}

		// ------------------------------------------------------------------------
		waypoint_menu->popup(button+1,gtk_get_current_event_time());
	}
	break;

	default:
		break;
	}
}

void
CanvasView::on_waypoint_changed()
{
	Action::ParamList param_list;
	param_list.add("canvas",get_canvas());
	param_list.add("canvas_interface",canvas_interface());
	param_list.add("value_node",waypoint_dialog.get_value_desc().get_value_node());
	param_list.add("waypoint",waypoint_dialog.get_waypoint());
//	param_list.add("time",canvas_interface()->get_time());

	get_instance()->process_action("WaypointSetSmart", param_list);
}

void
CanvasView::on_waypoint_delete()
{
	Action::ParamList param_list;
	param_list.add("canvas",get_canvas());
	param_list.add("canvas_interface",canvas_interface());
	param_list.add("value_node",waypoint_dialog.get_value_desc().get_value_node());
	param_list.add("waypoint",waypoint_dialog.get_waypoint());
//	param_list.add("time",canvas_interface()->get_time());

	get_instance()->process_action("WaypointRemove", param_list);
}

void
CanvasView::on_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int /*x*/, int /*y*/, const Gtk::SelectionData& selection_data_, guint /*info*/, guint time)
{
	// We will make this true once we have a solid drop
	bool success(false);
	//info("Dropped data of type \"%s\"",selection_data.get_data_type());
	//info("Dropped data of target \"%s\"",gdk_atom_name(selection_data->target));
	//info("selection=\"%s\"",gdk_atom_name(selection_data->selection));

	if ((selection_data_.get_length() >= 0) && (selection_data_.get_format() == 8))
	{
		if(String(selection_data_.get_data_type())=="STRING"
		|| String(selection_data_.get_data_type())=="text/plain")do
		{
			String selection_data((gchar *)(selection_data_.get_data()));

			Layer::Handle layer(Layer::create("Text"));
			if(!layer)
				break;
			if(!layer->set_param("text",ValueBase(selection_data)))
				break;

			Action::Handle 	action(Action::create("LayerAdd"));

			assert(action);
			if(!action)
				break;

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",canvas_interface());
			action->set_param("new",layer);

			if(!get_instance()->perform_action(action))
				break;

			// Ok, we have successfully imported at least one item.
			success=true;
		} while(0); // END of "STRING"

		if(String(selection_data_.get_data_type())=="text/uri-list")
		{
			std::vector<Glib::ustring> uris = selection_data_.get_uris();
			for (const auto& URI : uris) {
				// If we don't have an URI, move on.
				if(URI.empty())
					continue;

				// Extract protocol name from URI.
				String protocol( Glib::uri_parse_scheme(URI) );
				if(protocol.empty())
				{
					warning("Cannot extract protocol from URI \"%s\"", URI.c_str());
					continue;
				}

				// Only 'file' protocol supported
				if(protocol != "file")
				{
					warning("Protocol \"%s\" is unsupported (URI \"%s\")", protocol.c_str(), URI.c_str());
					continue;
				}

				// Converts an escaped UTF-8 encoded URI to a local filename
				// in the encoding used for filenames.
				String filename( Glib::filename_from_uri(URI) );
				if(filename.empty())
				{
					warning("Cannot extract filename from URI \"%s\"", URI.c_str());
					continue;
				}

				String ext = etl::filename_extension(filename);
				if (!ext.empty()) ext = ext.substr(1); // skip initial '.'

				// If this is a SIF file, then we need to do things slightly differently
				if (ext == "sketch") {
					if(work_area->load_sketch(filename)) {
						success=true;
						work_area->queue_draw();
					}
				} else {
					String errors, warnings;
					if(canvas_interface()->import(filename, errors, warnings, App::resize_imported_images))
						success=true;
					if (warnings != "")
						App::dialog_message_1b(
							"WARNING",
							synfig::strprintf("%s:\n\n%s",_("Warning"),warnings.c_str()),
							"details",
							_("Close") );
				}
			}
		} // END of "text/uri-list"
	}
	else
		ui_interface_->error("Drop failed: bad selection data");

	// Finish the drag
	context->drag_finish(success, false, time);
}

void
CanvasView::on_keyframe_add_pressed()
{
	Action::Handle action(Action::create("KeyframeAdd"));

	if(!action)
	{
		ui_interface_->error("I am unable to find the appropriate action");
		return;
	}

	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",canvas_interface());
	action->set_param("keyframe",Keyframe(get_time()));

	canvas_interface()->get_instance()->perform_action(action);
}

void
CanvasView::on_keyframe_duplicate_pressed()
{
	const KeyframeTreeStore::Model model;
	const Gtk::TreeRow row(*keyframe_tree->get_selection()->get_selected());
	Keyframe keyframe;
	if(!row)
	{
		ui_interface_->error("I am unable to duplicate the keyframe");
		return;
	}
	keyframe=row[model.keyframe];

	Action::Handle action(Action::create("KeyframeDuplicate"));

	if(!action)
	{
		ui_interface_->error("I am unable to find the appropriate action");
		return;
	}

	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",canvas_interface());
	action->set_param("keyframe",keyframe);
	action->set_param("time",get_time());

	canvas_interface()->get_instance()->perform_action(action);
}

void
CanvasView::on_keyframe_remove_pressed()
{
	const KeyframeTreeStore::Model model;
	const Gtk::TreeRow row(*keyframe_tree->get_selection()->get_selected());
	Keyframe keyframe;
	if(!row)
	{
		ui_interface_->error("I am unable to remove the keyframe");
		return;
	}
	keyframe=row[model.keyframe];

	Action::Handle action(Action::create("KeyframeRemove"));

	if(!action)
	{
		ui_interface_->error("I am unable to find the appropriate action");
		return;
	}

	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",canvas_interface());
	action->set_param("keyframe",keyframe);

	canvas_interface()->get_instance()->perform_action(action);
}

void
CanvasView::show_keyframe_dialog()
{
	Glib::RefPtr<Gtk::TreeSelection> selection(keyframe_tree->get_selection());
	if(selection->get_selected())
	{
		Gtk::TreeRow row(*selection->get_selected());

		Keyframe keyframe(row[keyframe_tree->model.keyframe]);

		keyframe_dialog.set_keyframe(keyframe);
		keyframe_dialog.present();
	}
}

void
CanvasView::on_keyframe_toggle()
{
	Glib::RefPtr<Gtk::TreeSelection> selection(keyframe_tree->get_selection());
	if(selection->get_selected())
	{
		Gtk::TreeRow row(*selection->get_selected());

		Keyframe keyframe(row[keyframe_tree->model.keyframe]);

		Action::Handle action(Action::create("KeyframeToggl"));

		if(!action)
			return;
		action->set_param("canvas",canvas_interface()->get_canvas());
		action->set_param("canvas_interface",canvas_interface());
		action->set_param("keyframe",keyframe);
		action->set_param("new_status",!keyframe.active ());

		canvas_interface()->get_instance()->perform_action(action);

	}
}

void
CanvasView::on_keyframe_description_set()
{
	Glib::RefPtr<Gtk::TreeSelection> selection(keyframe_tree->get_selection());
	if(selection->get_selected())
	{
		Gtk::TreeRow row(*selection->get_selected());

		Keyframe keyframe(row[keyframe_tree->model.keyframe]);

		Action::Handle action(Action::create("KeyframeSet"));

		if(!action)
			return;

		String str(keyframe.get_description ());
		if(!App::dialog_entry(_("Set Keyframe Description"),
					_("Description: "),
					//action->get_local_name(),
					str,
					_("Cancel"),
					_("Ok")))
			return;

		keyframe.set_description(str);

		action->set_param("canvas",canvas_interface()->get_canvas());
		action->set_param("canvas_interface",canvas_interface());
		action->set_param("keyframe",keyframe);

		canvas_interface()->get_instance()->perform_action(action);

	}
}

void
CanvasView::toggle_duck_mask(Duckmatic::Type type)
{
	if(toggling_ducks_)
		return;
	toggling_ducks_=true;
	if(type & Duck::TYPE_WIDTH)
		type=type|Duck::TYPE_WIDTHPOINT_POSITION;
	bool is_currently_on(work_area->get_type_mask()&type);

	if(is_currently_on)
		work_area->set_type_mask(work_area->get_type_mask()-type);
	else
		work_area->set_type_mask(work_area->get_type_mask()|type);

	if (type == Duck::TYPE_BONE_RECURSIVE)
		queue_rebuild_ducks();

	work_area->queue_draw();
	try
	{
		// Update the toggle ducks actions
		Glib::RefPtr<Gtk::ToggleAction> action;
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("mask-position-ducks"));
		action->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_POSITION));
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("mask-tangent-ducks"));
		action->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_TANGENT));
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("mask-vertex-ducks"));
		action->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_VERTEX));
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("mask-radius-ducks"));
		action->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_RADIUS));
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("mask-width-ducks"));
		action->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_WIDTH));
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("mask-angle-ducks"));
		action->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_ANGLE));
		// Update toggle ducks buttons
		action->get_active();
		toggleducksdial.update_toggles(work_area->get_type_mask());
	}
	catch(...)
	{
		toggling_ducks_=false;
	}
	toggling_ducks_=false;
}

void
CanvasView::toggle_duck_mask_all()
{
    if (work_area->get_type_mask_state ()== Duck::TYPE_NONE)
    {
        work_area->set_type_mask_state ( work_area->get_type_mask());
        work_area->set_type_mask(Duck::TYPE_NONE);
        toggle_duck_mask(Duck::TYPE_NONE);
    }
    else
    {
        work_area->set_type_mask(work_area->get_type_mask_state());
        work_area->set_type_mask_state ( Duck::TYPE_NONE);
        toggle_duck_mask(Duck::TYPE_NONE);
    }
}

void
CanvasView::mask_bone_ducks()
{
	Duck::Type mask(work_area->get_type_mask());
	bool recursive(mask & Duck::TYPE_BONE_RECURSIVE);

	if (recursive)
	{
		action_mask_bone_setup_ducks->set_active(true);
		action_mask_bone_recursive_ducks->set_active(false);
	}
	else
		action_mask_bone_recursive_ducks->set_active(true);
}

void
CanvasView::on_meta_data_changed()
{
	// update the buttons and actions that are associated
	toggling_show_grid=true;
	toggling_snap_grid=true;
	toggling_show_guides=true;
	toggling_snap_guides=true;
	toggling_onion_skin=true;
	toggling_onion_skin_keyframes=true;
	toggling_background_rendering=true;
	try
	{
		// Update the toggle ducks actions
		Glib::RefPtr<Gtk::ToggleAction> action;
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("toggle-background-rendering"));
		action->set_active((bool)(work_area->get_background_rendering()));
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("toggle-onion-skin"));
		action->set_active((bool)(work_area->get_onion_skin()));
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("toggle-onion-skin-keyframes"));
		action->set_active((bool)(work_area->get_onion_skin_keyframes()));
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("toggle-grid-show"));
		action->set_active((bool)(work_area->grid_status()));
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("toggle-grid-snap"));
		action->set_active((bool)(work_area->get_grid_snap()));
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("toggle-guide-show"));
		action->set_active((bool)(work_area->get_show_guides()));
		action = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(action_group->get_action("toggle-guide-snap"));
		action->set_active((bool)(work_area->get_guide_snap()));
		// Update the toggle buttons
		background_rendering_button->set_active(work_area->get_background_rendering());
		onion_skin->set_active(work_area->get_onion_skin());
		onion_skin_keyframes->set_active(work_area->get_onion_skin_keyframes());
		snap_grid->set_active(work_area->get_grid_snap());
		show_grid->set_active(work_area->grid_status());
		snap_guides->set_active(work_area->get_guide_snap());
		show_guides->set_active(work_area->get_show_guides());
		// Update the onion skin spins
		past_onion_spin->set_value(work_area->get_onion_skins()[0]);
		future_onion_spin->set_value(work_area->get_onion_skins()[1]);
	}
	catch(...)
	{
		toggling_show_grid=false;
		toggling_snap_grid=false;
		toggling_show_guides=false;
		toggling_snap_guides=false;
		toggling_onion_skin=false;
		toggling_onion_skin_keyframes=false;
		toggling_background_rendering=false;
	}
	toggling_show_grid=false;
	toggling_snap_grid=false;
	toggling_show_guides=false;
	toggling_snap_guides=false;
	toggling_onion_skin=false;
	toggling_onion_skin_keyframes=false;
	toggling_background_rendering=false;
}

void
CanvasView::import_file()
{
	// String filename(dirname(get_canvas()->get_file_name()));
	std::vector<std::string> filenames;
	LayerTree::LayerList layers;
	filenames.push_back("*.*");
	String errors, warnings;
	if(App::dialog_open_file(_("Please select files"), filenames, IMAGE_DIR_PREFERENCE))
	{
		for(const std::string &filename : filenames){
		// Don't let user import a file to itself
		// Check if it's the same file of this canvas
		{
			bool same_file = is_same_file(filename);
			if (same_file) {
				App::dialog_message_1b(
					"ERROR",
					_("You cannot import a file to itself"),
					"details",
					_("Close"));
				return;
			}
		}

		// Import
		Layer::Handle layer = canvas_interface()->import(filename, errors, warnings, App::resize_imported_images);
		if (!errors.empty())
			App::dialog_message_1b(
				"ERROR",
				synfig::strprintf("%s:\n\n%s", _("Error"), errors.c_str()),
				"details",
				_("Close"));
		if (!warnings.empty())
			App::dialog_message_1b(
				"WARNING",
				synfig::strprintf("%s:\n\n%s", _("Warning"), warnings.c_str()),
				"details",
				_("Close"));

		if (layer) {
			layers.push_back(layer);
		}
	}
	get_selection_manager()->clear_selected_layers();
	get_selection_manager()->set_selected_layers(layers);
	}
}

bool
CanvasView::is_same_file(const std::string &filename)
{
	bool is_same_file = get_canvas()->get_file_name() == filename;
	if (!is_same_file) {
		Glib::RefPtr<Gio::File> current_file;
		try {
			current_file = Gio::File::create_for_path(get_canvas()->get_file_name());
			if (current_file) {
				Glib::RefPtr<Gio::File> import_file = Gio::File::create_for_path(filename);
				is_same_file = current_file->equal(import_file);
				if (!is_same_file && import_file) {
					// One more sanity check
					Glib::RefPtr<Gio::FileInfo> current_file_info = current_file->query_info(G_FILE_ATTRIBUTE_ID_FILE);
					Glib::RefPtr<Gio::FileInfo> import_file_info = import_file->query_info(G_FILE_ATTRIBUTE_ID_FILE);
					is_same_file = current_file_info->get_attribute_string(G_FILE_ATTRIBUTE_ID_FILE) == import_file_info->get_attribute_string(G_FILE_ATTRIBUTE_ID_FILE);
				}
			}
		} catch (...) {
		}
	}
	return is_same_file;
}

void
CanvasView::import_sequence()
{
	std::set<String> filenames;
	String errors, warnings;
	if(App::dialog_open_file_image_sequence(_("Please select a file"), filenames, IMAGE_DIR_PREFERENCE))
	{
		int answer = get_ui_interface()->yes_no_cancel(
				("Remove Duplicates while importing?"),
				_("Synfig will detect and remove consecutive duplicates (if any)."),
				_("No"),
				_("Cancel"),
				_("Yes"),
				false,
				UIInterface::RESPONSE_NO);
		if(answer!=UIInterface::RESPONSE_CANCEL){
			canvas_interface()->import_sequence(filenames, errors, warnings, App::resize_imported_images,answer);
			if (!errors.empty())
				App::dialog_message_1b(
						"ERROR",
						synfig::strprintf("%s:\n\n%s", _("Error"), errors.c_str()),
						"details",
						_("Close"));
			if (!warnings.empty())
				App::dialog_message_1b(
						"WARNING",
						synfig::strprintf("%s:\n\n%s", _("Warning"), warnings.c_str()),
						"details",
						_("Close"));
		}
	}
}

Smach::event_result
CanvasView::process_event_key(EventKey x)
{
	return smach_.process_event(x);
}

void
CanvasView::on_input_device_changed(GdkDevice* device)
{
	if(!device)
	{
		get_smach().egress();
	}
	assert(device);

	InputDevice::Handle input_device;
	input_device = synfigapp::Main::select_input_device(gdk_device_get_name(device));
	App::dock_toolbox->change_state(input_device->get_state(), true);
	process_event_key(EVENT_INPUT_DEVICE_CHANGED);
}

void
CanvasView::on_preview_option()
{
	if(Canvas::Handle canv = get_canvas())
	{
		RendDesc &r = canv->rend_desc();
		if(r.get_frame_rate())
		{
			float rate = 1/r.get_frame_rate();
			float beg = r.get_time_start() + r.get_frame_start()*rate;
			float end = r.get_time_start() + r.get_frame_end()*rate;

			Dialog_PreviewOptions *po = dynamic_cast<Dialog_PreviewOptions *>( get_ext_widget("prevoptions") );
			if(!po)
			{
				po = Dialog_PreviewOptions::create();
				po->set_fps(r.get_frame_rate()/2);
				set_ext_widget("prevoptions",po);
			}

			if (!po->get_begin_override())
				po->set_begintime(beg);
			if (!po->get_end_override())
				po->set_endtime(end);

			po->set_global_fps(r.get_frame_rate());
			po->signal_finish().connect(sigc::mem_fun(*this, &CanvasView::on_preview_create));
			po->present();
		}
	}
}

void
CanvasView::on_preview_create(const PreviewInfo &info)
{
	//set all the options
	etl::handle<Preview>	prev = new Preview;

	prev->set_canvasview(this);
	prev->set_zoom(info.zoom);
	prev->set_fps(info.fps);
	prev->set_overbegin(info.overbegin);
	prev->set_begintime(info.begintime);
	prev->set_overend(info.overend);
	prev->set_endtime(info.endtime);
#ifdef WITH_JACK
	prev->set_jack_offset(get_jack_offset());
#endif

	//render it out...
	prev->render();

	preview_dialog.set_default_size(700,510);
	preview_dialog.set_preview(prev.get());
	preview_dialog.present();

	// Preview Window created, the action can be enabled
	{
		Glib::RefPtr< Gtk::Action > action = action_group->get_action("dialog-flipbook");
		action->set_sensitive(true);
	}

}

Glib::RefPtr<Glib::ObjectBase>
CanvasView::get_ref_obj(const String& x)
{
	RefObjBook::const_iterator i = ref_obj_book_.find(x);
	return i == ref_obj_book_.end() ? Glib::RefPtr<Glib::ObjectBase>() : i->second;
}

void
CanvasView::set_ref_obj(const String& x, Glib::RefPtr<Glib::ObjectBase> y)
{
	if (y)
		ref_obj_book_[x] = y;
	else
		ref_obj_book_.erase(x);
}

Glib::RefPtr<Gtk::TreeModel>
CanvasView::get_tree_model(const String& x)
{
	return Glib::RefPtr<Gtk::TreeModel>::cast_dynamic(get_ref_obj("_tree_model_"+x));
}

void
CanvasView::set_tree_model(const String& x, Glib::RefPtr<Gtk::TreeModel> y)
{
	set_ref_obj("_tree_model_"+x, y);
}

Gtk::Widget*
CanvasView::get_ext_widget(const String& x)
{
	WidgetBook::const_iterator i = ext_widget_book_.find(x);
	return i == ext_widget_book_.end() ? 0 : i->second.get();
}

void
CanvasView::set_ext_widget(const String& x, Gtk::Widget* y, bool own)
{
	assert(y);
	assert(!get_ext_widget(x));
	
	ext_widget_book_[x].set(y, own);
	if(x=="layers_cmp")
	{
		if ((layer_tree=dynamic_cast<LayerTree*>(y))) {
			layer_tree->get_selection()->signal_changed().connect(SLOT_EVENT(EVENT_LAYER_SELECTION_CHANGED));
			layer_tree->get_selection()->signal_changed().connect(SLOT_EVENT(EVENT_REFRESH_DUCKS));
			layer_tree->signal_layer_user_click().connect(sigc::mem_fun(*this, &CanvasView::on_layer_user_click));
	//		layer_tree->signal_param_user_click().connect(sigc::mem_fun(*this, &CanvasView::on_param_user_click));
			layer_tree->signal_waypoint_clicked_layertree().connect(sigc::mem_fun(*this, &CanvasView::on_waypoint_clicked_canvasview));
		}
	}
	if(x=="children")
	{
		if ((children_tree=dynamic_cast<ChildrenTree*>(y))) {
			children_tree->signal_user_click().connect(sigc::mem_fun(*this, &CanvasView::on_children_user_click));
			children_tree->signal_waypoint_clicked_childrentree().connect(sigc::mem_fun(*this, &CanvasView::on_waypoint_clicked_canvasview));
			children_tree->get_selection()->signal_changed().connect(SLOT_EVENT(EVENT_REFRESH_DUCKS));
		}
	}
	if(x=="keyframes")
		keyframe_tree=dynamic_cast<KeyframeTree*>(y);
}

AdjustmentGroup::Handle
CanvasView::get_adjustment_group(const synfig::String& x)
{
	AdjustmentGroupBook::const_iterator i = adjustment_group_book_.find(x);
	return i == adjustment_group_book_.end() ? AdjustmentGroup::Handle() : i->second;
}

void
CanvasView::set_adjustment_group(const synfig::String& x, AdjustmentGroup::Handle y)
{
	if (y)
		adjustment_group_book_[x] = y;
	else
		adjustment_group_book_.erase(x);
}

Gtk::UIManager::ui_merge_id
CanvasView::get_popup_id()
{
	return merge_id_popup_;
}

void
CanvasView::set_popup_id(Gtk::UIManager::ui_merge_id popup_id)
{
	merge_id_popup_ = popup_id;
}

Gtk::UIManager::ui_merge_id
CanvasView::get_toolbar_id()
{
	return merge_id_toolbar_;
}

void
CanvasView::set_toolbar_id(Gtk::UIManager::ui_merge_id toolbar_id)
{
	merge_id_toolbar_ = toolbar_id;
}

bool
CanvasView::on_delete_event(GdkEventAny* /*event*/)
{
	close_view();

	//! \todo This causes the window to be deleted straight away - but what if we prompt 'save?' and the user cancels?
	//		  Is there ever any need to pass on the delete event to the window here?
	// if(event) return Gtk::Window::on_delete_event(event);

	return true;
}

void
CanvasView::on_seek_begin_pressed()
{
	canvas_interface()->set_time(time_model()->get_actual_play_bounds_lower());
}

void
CanvasView::on_seek_end_pressed()
{
	canvas_interface()->set_time(time_model()->get_actual_play_bounds_upper());
}

//! Modify the play stop button apearence and play stop the animation
void
CanvasView::on_play_pause_pressed()
{
	if (jack_enabled)
	{
		#ifdef WITH_JACK
		if (jack_is_playing) {
			jack_transport_stop(jack_client);
			on_jack_sync();
			stop_async();
		} else
			jack_transport_start(jack_client);
		#endif
	}
	else
	{
		if(!is_playing())
			play_async();
		else
			stop_async();
	}
}

#ifdef WITH_JACK
void
CanvasView::toggle_jack_button()
{
	if (!toggling_jack)
	{
		String message;
		String details;
		if (get_jack_enabled())
		{
			message = synfig::strprintf(_("Are you sure you want to disable JACK synchronization?" ));
			details = synfig::strprintf(_("The JACK server will remain running."));
		} else {
			message = synfig::strprintf(_("Are you sure you want to enable JACK synchronization?" ));
			details = synfig::strprintf(_("This operation will launch a JACK server, if it isn't started yet."));
		}

		UIInterface::Response answer = get_ui_interface()->confirmation(
			message,
			details,
			_("Yes"),
			_("No"),
			UIInterface::RESPONSE_OK );
		if (answer == UIInterface::RESPONSE_OK)
			set_jack_enabled(!get_jack_enabled());

		// Update button state
		toggling_jack = true;
		jackdial->set_state(get_jack_enabled());
		toggling_jack = false;
	}
}

void
CanvasView::on_jack_offset_changed()
{
	set_jack_offset(jackdial->get_offset());
	if (get_jack_enabled()) on_jack_sync();
}

Time
CanvasView::get_jack_offset()const {
	return work_area->get_jack_offset();
}

void
CanvasView::set_jack_offset(const Time &value) {
	work_area->set_jack_offset(value);
}

void
CanvasView::on_jack_sync()
{
	jack_position_t pos;
	jack_transport_state_t state = jack_transport_query(jack_client, &pos);

	jack_is_playing = state == JackTransportRolling || state == JackTransportStarting;
	jack_time = Time((Time::value_type)pos.frame/(Time::value_type)pos.frame_rate);

	if (is_playing() != jack_is_playing) {
		if (jack_is_playing)
			play_async();
		else
			stop_async();
	}

	jack_synchronizing = true;
	time_model()->set_time(jack_time - get_jack_offset());
	jack_synchronizing = false;
}


int
CanvasView::jack_sync_callback(jack_transport_state_t /* state */, jack_position_t * /* pos */, void *arg)
{
	CanvasView *canvasView = static_cast<CanvasView*>(arg);
	canvasView->jack_dispatcher.emit();
	return 1;
}

#endif

void
CanvasView::show_dependencies() const
{
	Canvas::ConstHandle canvas = get_canvas();
	if (!canvas)
		return;

	Dialog_CanvasDependencies* dialog = Dialog_CanvasDependencies::create(*App::main_window);
	if (!dialog) {
		synfig::warning("Can't load Dialog_CanvasDependencies");
		return;
	}
	dialog->set_canvas(get_canvas());
	dialog->run();
	delete dialog;
}

void
CanvasView::interpolation_refresh()
	{ widget_interpolation->set_value(synfigapp::Main::get_interpolation()); }

void
CanvasView::on_interpolation_changed()
	{ synfigapp::Main::set_interpolation(Waypoint::Interpolation(widget_interpolation->get_value())); }

void 
CanvasView::toggle_show_toolbar(){
	if(App::enable_mainwin_toolbar)
		displaybar->show();
	else
		displaybar->hide();
};
