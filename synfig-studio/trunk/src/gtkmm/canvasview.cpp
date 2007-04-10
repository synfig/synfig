/* === S Y N F I G ========================================================= */
/*!	\file canvasview.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <sigc++/adaptors/hide.h>

#include <ETL/clock>
#include <sstream>

#include <gtkmm/paned.h>
#include <gtkmm/scale.h>
#include <gtkmm/dialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/treemodelsort.h>
#include <gtkmm/buttonbox.h>

#include <gtk/gtktreestore.h>
#include <gtk/gtkversion.h>

#include <synfig/valuenode_reference.h>
#include <synfig/valuenode_subtract.h>
#include <synfig/valuenode_linear.h>
#include <synfig/valuenode_timedswap.h>
#include <synfig/valuenode_scale.h>
#include <synfig/valuenode_dynamiclist.h>
#include <synfig/valuenode_twotone.h>
#include <synfig/valuenode_stripes.h>
#include <synfig/layer.h>

#include <synfigapp/uimanager.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/selectionmanager.h>
//#include <synfigapp/action_setwaypoint.h>
//#include <synfigapp/action_deletewaypoint.h>

#include <sigc++/retype_return.h>
#include <sigc++/retype.h>
//#include <sigc++/hide.h>

#include "canvasview.h"
#include "instance.h"
#include "app.h"
#include "cellrenderer_value.h"
#include "cellrenderer_timetrack.h"
#include "workarea.h"
#include "dialog_color.h"
#include "eventkey.h"

#include "state_polygon.h"
#include "state_bline.h"
#include "state_normal.h"
#include "state_eyedrop.h"
#include "state_draw.h"

#include "ducktransform_scale.h"
#include "ducktransform_translate.h"
#include "ducktransform_rotate.h"

#include "event_mouse.h"
#include "event_layerclick.h"

#include "toolbox.h"

#include "dialog_preview.h"
#include "dialog_soundselect.h"

#include "preview.h"
#include "audiocontainer.h"
#include "widget_timeslider.h"

#include <synfigapp/main.h>
#include <synfigapp/inputdevice.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;
using namespace SigC;

/* === M A C R O S ========================================================= */

#define GRAB_HINT_DATA(y)	{ \
		String x; \
		if(synfigapp::Main::settings().get_value(String("pref.")+y+"_hints",x)) \
		{ \
			set_type_hint((Gdk::WindowTypeHint)atoi(x.c_str()));	\
		} \
	}

#define DEFAULT_TIME_WINDOW_SIZE		(10.0)

/*
#ifdef DEBUGPOINT
#undef DEBUGPOINT
#endif
#define DEBUGPOINT()
*/

#ifndef SMALL_BUTTON
#define SMALL_BUTTON(button,stockid,tooltip)	\
	button = manage(new class Gtk::Button());	\
	icon=manage(new Gtk::Image(Gtk::StockID(stockid),iconsize));	\
	button->add(*icon);	\
	tooltips.set_tip(*button,tooltip);	\
	icon->set_padding(0,0);\
	icon->show();	\
	button->set_relief(Gtk::RELIEF_NONE); \
	button->show()
#endif

#ifndef NORMAL_BUTTON
#define NORMAL_BUTTON(button,stockid,tooltip)	\
	button = manage(new class Gtk::Button());	\
	icon=manage(new Gtk::Image(Gtk::StockID(stockid),Gtk::ICON_SIZE_BUTTON));	\
	button->add(*icon);	\
	tooltips.set_tip(*button,tooltip);	\
	icon->set_padding(0,0);\
	icon->show();	\
	/*button->set_relief(Gtk::RELIEF_NONE);*/ \
	button->show()
#endif

#define NEW_SMALL_BUTTON(x,y,z)	Gtk::Button *SMALL_BUTTON(x,y,z)

#define NOT_IMPLEMENTED_SLOT sigc::mem_fun(*reinterpret_cast<studio::CanvasViewUIInterface*>(get_ui_interface().get()),&studio::CanvasViewUIInterface::not_implemented)

#define SLOT_EVENT(x)	sigc::hide_return(sigc::bind(sigc::mem_fun(*this,&studio::CanvasView::process_event_key),x))

/* === C L A S S E S ======================================================= */


class studio::UniversalScrubber
{
	CanvasView *canvas_view;

	bool		scrubbing;
	etl::clock	scrub_timer;

	sigc::connection end_scrub_connection;
public:
	UniversalScrubber(CanvasView *canvas_view):
		canvas_view(canvas_view),
		scrubbing(false)
	{
		canvas_view->canvas_interface()->signal_time_changed().connect(
			sigc::mem_fun(*this,&studio::UniversalScrubber::on_time_changed)
		);
	}

	~UniversalScrubber()
	{
		end_scrub_connection.disconnect();
	}

	void on_time_changed()
	{
		// Make sure we are changing the time quickly
		// before we enable scrubbing
		if(!scrubbing && scrub_timer()>1)
		{
			scrub_timer.reset();
			return;
		}

		// If we aren't scrubbing already, enable it
		if(!scrubbing)
		{
			scrubbing=true;
			audio_container()->start_scrubbing(canvas_view->get_time());
		}

		// Reset the scrubber ender
		end_scrub_connection.disconnect();
		end_scrub_connection=Glib::signal_timeout().connect(
			sigc::bind_return(
				sigc::mem_fun(*this,&UniversalScrubber::end_of_scrubbing),
				false
			),
			1000
		);

		// Scrub!
		audio_container()->scrub(canvas_view->get_time());

		scrub_timer.reset();
	}

	void end_of_scrubbing()
	{
		scrubbing=false;
		audio_container()->stop_scrubbing();
		scrub_timer.reset();
	}

	handle<AudioContainer> audio_container()
	{
		assert(canvas_view->audio);
		return canvas_view->audio;
	}
};


class studio::CanvasViewUIInterface : public synfigapp::UIInterface
{
	CanvasView *view;

public:

	CanvasViewUIInterface(CanvasView *view):
		view(view)
	{

		view->statusbar->push("Idle");
	}

	~CanvasViewUIInterface()
	{
		//view->statusbar->pop();
		//view->progressbar->set_fraction(0);
	}

	virtual Response yes_no(const std::string &title, const std::string &message,Response dflt=RESPONSE_YES)
	{
		view->present();
		//while(studio::App::events_pending())studio::App::iteration(false);
		Gtk::Dialog dialog(
			title,		// Title
			*view,		// Parent
			true,		// Modal
			true		// use_separator
		);
		Gtk::Label label(message);
		label.show();

		dialog.get_vbox()->pack_start(label);
		dialog.add_button(Gtk::StockID("gtk-yes"),RESPONSE_YES);
		dialog.add_button(Gtk::StockID("gtk-no"),RESPONSE_NO);

		dialog.set_default_response(dflt);
		dialog.show();
		return (Response)dialog.run();
	}
	virtual Response yes_no_cancel(const std::string &title, const std::string &message,Response dflt=RESPONSE_YES)
	{
		view->present();
		//while(studio::App::events_pending())studio::App::iteration(false);
		Gtk::Dialog dialog(
			title,		// Title
			*view,		// Parent
			true,		// Modal
			true		// use_separator
		);
		Gtk::Label label(message);
		label.show();

		dialog.get_vbox()->pack_start(label);
		dialog.add_button(Gtk::StockID("gtk-yes"),RESPONSE_YES);
		dialog.add_button(Gtk::StockID("gtk-no"),RESPONSE_NO);
		dialog.add_button(Gtk::StockID("gtk-cancel"),RESPONSE_CANCEL);

		dialog.set_default_response(dflt);
		dialog.show();
		return (Response)dialog.run();
	}
	virtual Response ok_cancel(const std::string &title, const std::string &message,Response dflt=RESPONSE_OK)
	{
		view->present();
		//while(studio::App::events_pending())studio::App::iteration(false);
		Gtk::Dialog dialog(
			title,		// Title
			*view,		// Parent
			true,		// Modal
			true		// use_separator
		);
		Gtk::Label label(message);
		label.show();

		dialog.get_vbox()->pack_start(label);
		dialog.add_button(Gtk::StockID("gtk-ok"),RESPONSE_OK);
		dialog.add_button(Gtk::StockID("gtk-cancel"),RESPONSE_CANCEL);

		dialog.set_default_response(dflt);
		dialog.show();
		return (Response)dialog.run();
	}

	virtual bool
	task(const std::string &task)
	{
		if(!view->is_playing_)
		{
			view->statusbar->pop();
			view->statusbar->push(task);
		}
		//while(studio::App::events_pending())studio::App::iteration(false);
		if(view->cancel){return false;}
		return true;
	}

	virtual bool
	error(const std::string &err)
	{
		view->statusbar->push("ERROR");

		// If we are in the process of canceling,
		// then just go ahead and return false --
		// don't bother displaying a dialog
		if(view->cancel)return false;
		Gtk::MessageDialog dialog(*view, err, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
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

		//while(studio::App::events_pending())studio::App::iteration(false);
		if(view->cancel)return false;
		return true;
	}

	virtual bool
	amount_complete(int current, int total)
	{
		if(!view->is_playing_)
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
			view->progressbar->set_fraction(x);
		}
		//while(studio::App::events_pending())studio::App::iteration(false);
		if(view->cancel){/*view->cancel=false;*/return false;}
		return true;
	}

	void
	not_implemented()
	{
		error("Feature not yet implemented");
	}
};

class studio::CanvasViewSelectionManager : public synfigapp::SelectionManager
{
	CanvasView *view;
	CanvasView::LayerTreeModel layer_tree_model;
	CanvasView::ChildrenTreeModel children_tree_model;
public:

	CanvasViewSelectionManager(CanvasView *view): view(view)
{

 }


private:
	void _set_selected_layer(const synfig::Layer::Handle &layer)
	{
		view->layer_tree->select_layer(layer);
/*
		// Don't change the selection while we are busy
		// I cannot remember exactly why I put this here...
		// It musta been for some reason, but I cannot recall.
		//if(App::Busy::count)
		//	return;

		if(view->layer_tree->get_selection()->get_selected())
		{
			const Gtk::TreeRow row = *(view->layer_tree->get_selection()->get_selected());

			// Don't do anything if that layer is already selected
			if(layer == static_cast<synfig::Layer::Handle>(row[layer_tree_model.layer]))
				return;
		}
		Gtk::TreeModel::Children::iterator iter;
		if(view->layer_tree_store()->find_layer_row(layer,iter))
		{
			Gtk::TreePath path(iter);
			for(int i=path.get_depth();i;i--)
			{
				int j;
				path=Gtk::TreePath(iter);
				for(j=i;j;j--)
					path.up();
				view->layer_tree->get_tree_view().expand_row(path,false);
			}
			view->layer_tree->get_tree_view().scroll_to_row(Gtk::TreePath(iter));
			view->layer_tree->get_selection()->select(iter);
		}
*/
	}
public:

	//! Returns the number of layers selected.
	virtual int get_selected_layer_count()const
	{
		return get_selected_layers().size();
	}

	//! Returns a list of the currently selected layers.
	virtual LayerList get_selected_layers()const
	{
//		assert(view->layer_tree);

		if(!view->layer_tree) { DEBUGPOINT(); synfig::error("canvas_view.layer_tree not defined!?"); return LayerList(); }
		return view->layer_tree->get_selected_layers();
	}

	//! Returns the first layer selected or an empty handle if none are selected.
	virtual synfig::Layer::Handle get_selected_layer()const
	{
//		assert(view->layer_tree);

		if(!view->layer_tree) { DEBUGPOINT(); synfig::error("canvas_view.layer_tree not defined!?"); return 0; }
		return view->layer_tree->get_selected_layer();
	}

	//! Sets which layers should be selected
	virtual void set_selected_layers(const LayerList &layer_list)
	{
//		assert(view->layer_tree);

		if(!view->layer_tree) { DEBUGPOINT(); synfig::error("canvas_view.layer_tree not defined!?"); return; }
		view->layer_tree->select_layers(layer_list);
		//view->get_smach().process_event(EVENT_REFRESH_DUCKS);

		//view->queue_rebuild_ducks();
	}

	//! Sets which layer should be selected.
	virtual void set_selected_layer(const synfig::Layer::Handle &layer)
	{
//		assert(view->layer_tree);

		if(!view->layer_tree) { DEBUGPOINT(); synfig::error("canvas_view.layer_tree not defined!?"); return; }
		view->layer_tree->select_layer(layer);
		//view->queue_rebuild_ducks();
	}

	//! Clears the layer selection list
	virtual void clear_selected_layers()
	{
		if(!view->layer_tree) return;
		view->layer_tree->clear_selected_layers();
	}



	//! Returns the number of value_nodes selected.
	virtual int get_selected_children_count()const
	{
		return get_selected_children().size();
	}

	static inline void __child_grabber(const Gtk::TreeModel::iterator& iter, ChildrenList* ret)
	{
		const CanvasView::ChildrenTreeModel children_tree_model;
		synfigapp::ValueDesc value_desc((*iter)[children_tree_model.value_desc]);
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

		selection->selected_foreach_iter(
			sigc::bind(
				sigc::ptr_fun(
					&studio::CanvasViewSelectionManager::__child_grabber
				),
				&ret
			)
		);

		/*
		Gtk::TreeModel::Children::iterator iter(view->children_tree_store()->children().begin());
		iter++;
		Gtk::TreeModel::Children children = iter->children();
		for(iter = children.begin(); iter != children.end(); ++iter)
		{
			Gtk::TreeModel::Row row = *iter;
			if(selection->is_selected(row))
				ret.push_back((synfigapp::ValueDesc)row[children_tree_model.value_desc]);
		}
		*/
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
	virtual void set_selected_children(const ChildrenList &children_list)
	{
		return;
	}

	//! Sets which value_node should be selected. Empty handle if none.
	virtual void set_selected_child(const ChildrenList::value_type &child)
	{
		return;
	}

	//! Clears the value_node selection list
	virtual void clear_selected_children()
	{
		return;
	}



	int get_selected_layer_parameter_count()const
	{
		return get_selected_layer_parameters().size();
	}

	LayerParamList get_selected_layer_parameters()const
	{
		if(!view->layer_tree) return LayerParamList();

		Glib::RefPtr<Gtk::TreeSelection> selection=view->layer_tree->get_selection();

		if(!selection)
			return LayerParamList();

		LayerParamList ret;

		Gtk::TreeModel::Children children = const_cast<CanvasView*>(view)->layer_tree_store()->children();
		Gtk::TreeModel::Children::iterator iter;
		for(iter = children.begin(); iter != children.end(); ++iter)
		{
			Gtk::TreeModel::Row row = *iter;
			Gtk::TreeModel::Children::iterator iter;
			for(iter=row.children().begin();iter!=row.children().end();iter++)
			{
				Gtk::TreeModel::Row row = *iter;
				if(selection->is_selected(row))
					ret.push_back(LayerParam(row[layer_tree_model.layer],(Glib::ustring)row[layer_tree_model.id]));
			}
		}
		return ret;
	}

	LayerParam get_selected_layer_parameter() const
	{
		if(!view->layer_tree) return LayerParam();
		return get_selected_layer_parameters().front();
	}

	void set_selected_layer_parameters(const LayerParamList &layer_param_list)
	{
		return;
	}

	void set_selected_layer_param(const LayerParam &layer_param)
	{
		return;
	}

	void clear_selected_layer_parameters()
	{
		return;
	}

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

CanvasView::CanvasView(etl::loose_handle<Instance> instance,etl::handle<synfigapp::CanvasInterface> canvas_interface_):
	smach_					(this),
	instance_				(instance),
	canvas_interface_		(canvas_interface_),
	//layer_tree_store_		(LayerTreeStore::create(canvas_interface_)),
	//children_tree_store_	(ChildrenTreeStore::create(canvas_interface_)),
	//keyframe_tree_store_	(KeyframeTreeStore::create(canvas_interface_)),
	time_adjustment_		(0,0,25,0,0,0),
	time_window_adjustment_	(0,0,25,0,0,0),
	statusbar				(manage(new class Gtk::Statusbar())),

	timeslider				(new Widget_Timeslider),

	ui_interface_			(new CanvasViewUIInterface(this)),
	selection_manager_		(new CanvasViewSelectionManager(this)),
	is_playing_				(false),

	working_depth			(0),
	cancel					(false),

	canvas_properties		(*this,canvas_interface_),
	canvas_options			(this),
	render_settings			(*this,canvas_interface_),
	waypoint_dialog			(*this,canvas_interface_->get_canvas()),
	keyframe_dialog			(*this,canvas_interface_),
	preview_dialog			(new Dialog_Preview),
	sound_dialog			(new Dialog_SoundSelect(*this,canvas_interface_))
{
	layer_tree=0;
	children_tree=0;
	duck_refresh_flag=true;

	smach_.set_default_state(&state_normal);

	disp_audio = new Widget_Sound();

	//synfig::info("Canvasview: Entered constructor");
	// Minor hack
	get_canvas()->set_time(0);
	//layer_tree_store_->rebuild();

	// Set up the UI and Selection managers
	canvas_interface()->set_ui_interface(get_ui_interface());
	canvas_interface()->set_selection_manager(get_selection_manager());
	rebuild_ducks_queued=false;

	//notebook=manage(new class Gtk::Notebook());
	//Gtk::VPaned *vpaned = manage(new class Gtk::VPaned());
	//vpaned->pack1(*create_work_area(), Gtk::EXPAND|Gtk::SHRINK);
	//vpaned->pack2(*notebook, Gtk::SHRINK);
	//vpaned->show_all();


	//notebook->show();

	//notebook->append_page(*create_layer_tree(),"Layers");
	//notebook->append_page(*create_children_tree(),"Children");
	//notebook->append_page(*create_keyframe_tree(),"Keyframes");

	//synfig::info("Canvasview: Before big chunk of allocation and tabling stuff");
	//create all allocated stuff for this canvas
	audio = new AudioContainer();

	Gtk::Table *layout_table= manage(new class Gtk::Table(1, 3, false));
	//layout_table->attach(*vpaned, 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	layout_table->attach(*create_work_area(), 0, 1, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	init_menus();
	//layout_table->attach(*App::ui_manager()->get_widget("/menu-main"), 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);


	layout_table->attach(*create_time_bar(), 0, 1, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	layout_table->attach(*create_status_bar(), 0, 1, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);

	update_title();

	layout_table->show();
	add(*layout_table);

	//set_transient_for(*App::toolbox);

	//synfig::info("Canvasview: Before Signals");
	/*
 --	** -- Signals -------------------------------------------------------------
	*/

	canvas_interface()->signal_dirty_preview().connect(sigc::mem_fun(*this,&studio::CanvasView::on_dirty_preview));
	canvas_interface()->signal_mode_changed().connect(sigc::mem_fun(*this,&studio::CanvasView::on_mode_changed));

	canvas_interface()->signal_time_changed().connect(sigc::mem_fun(*this,&studio::CanvasView::on_time_changed));

	//canvas_interface()->signal_time_changed().connect(sigc::mem_fun(*this,&studio::CanvasView::refresh_tables));
	canvas_interface()->signal_id_changed().connect(sigc::mem_fun(*this,&studio::CanvasView::on_id_changed));
	canvas_interface()->signal_rend_desc_changed().connect(sigc::mem_fun(*this,&studio::CanvasView::refresh_rend_desc));
	waypoint_dialog.signal_changed().connect(sigc::mem_fun(*this,&studio::CanvasView::on_waypoint_changed));
	waypoint_dialog.signal_delete().connect(sigc::mem_fun(*this,&studio::CanvasView::on_waypoint_delete));

	//MODIFIED TIME ADJUSTMENT STUFF....
	time_window_adjustment().set_child_adjustment(&time_adjustment());
	time_window_adjustment().signal_value_changed().connect(sigc::mem_fun(*this,&studio::CanvasView::refresh_time_window));
	time_adjustment().signal_value_changed().connect(sigc::mem_fun(*this,&studio::CanvasView::time_was_changed));


	work_area->signal_layer_selected().connect(sigc::mem_fun(*this,&studio::CanvasView::workarea_layer_selected));
	work_area->signal_input_device_changed().connect(sigc::mem_fun(*this,&studio::CanvasView::on_input_device_changed));

	canvas_interface()->signal_canvas_added().connect(
		sigc::hide(
			sigc::mem_fun(*instance,&studio::Instance::refresh_canvas_tree)
		)
	);
	canvas_interface()->signal_canvas_removed().connect(
		sigc::hide(
			sigc::mem_fun(*instance,&studio::Instance::refresh_canvas_tree)
		)
	);

	canvas_interface()->signal_layer_param_changed().connect(
		sigc::hide(
			sigc::hide(
				SLOT_EVENT(EVENT_REFRESH_DUCKS)
			)
		)
	);


	//MUCH TIME STUFF TAKES PLACE IN HERE
	refresh_rend_desc();
	refresh_time_window();

	/*! \todo We shouldn't need to do this at construction --
	**	This should be preformed at the first time the window
	** 	becomes visible.
	*/
	work_area->queue_render_preview();

	// If the canvas is really big, zoom out so that we can fit it all in the window
	/*! \todo In other words, this is a zoom-to-fit, and should be
	** in it's own function.
	*/
	int w=get_canvas()->rend_desc().get_w()+70;
	int h=get_canvas()->rend_desc().get_h()+70;
	while(w>700 || h>600)
	{
		work_area->zoom_out();
		w=round_to_int(get_canvas()->rend_desc().get_w()*work_area->get_zoom()+70);
		h=round_to_int(get_canvas()->rend_desc().get_h()*work_area->get_zoom()+70);
	}
	if(w>700)w=700;
	if(h>600)h=600;
	set_default_size(w,h);
	property_window_position().set_value(Gtk::WIN_POS_NONE);




	std::list<Gtk::TargetEntry> listTargets;
	listTargets.push_back( Gtk::TargetEntry("STRING") );
	listTargets.push_back( Gtk::TargetEntry("text/plain") );
	listTargets.push_back( Gtk::TargetEntry("image") );

	drag_dest_set(listTargets);
	signal_drag_data_received().connect( sigc::mem_fun(*this, &studio::CanvasView::on_drop_drag_data_received) );


	/*
	Time length(get_canvas()->rend_desc().get_time_end()-get_canvas()->rend_desc().get_time_start());
	if(length<10.0)
	{
		time_window_adjustment().set_page_increment(length);
		time_window_adjustment().set_page_size(length);
	}
	else
	{
		time_window_adjustment().set_page_increment(10.0);
		time_window_adjustment().set_page_size(10.0);
	}
	*/

	//synfig::info("Canvasview: Before Sound Hookup");
	//load sound info from meta data
	{
		//synfig::warning("Should load Audio: %s with %s offset",apath.c_str(),aoffset.c_str());

		on_audio_file_notify(); //redundant setting of the metadata, but oh well, it's no big deal :)
		on_audio_offset_notify();

		//signal connection - since they are all associated with the canvas view

		//hook in signals for sound options box
		sound_dialog->signal_file_changed().connect(sigc::mem_fun(*this,&CanvasView::on_audio_file_change));
		sound_dialog->signal_offset_changed().connect(sigc::mem_fun(*this,&CanvasView::on_audio_offset_change));

		//attach to the preview when it's visible
		//preview_dialog->get_widget().signal_play().connect(sigc::mem_fun(*this,&CanvasView::play_audio));
		//preview_dialog->get_widget().signal_stop().connect(sigc::mem_fun(*this,&CanvasView::stop_audio));

		//hook to metadata signals
		get_canvas()->signal_meta_data_changed("audiofile").connect(sigc::mem_fun(*this,&CanvasView::on_audio_file_notify));
		get_canvas()->signal_meta_data_changed("audiooffset").connect(sigc::mem_fun(*this,&CanvasView::on_audio_offset_notify));

		//universal_scrubber=std::auto_ptr<UniversalScrubber>(new UniversalScrubber(this));
	}

	//synfig::info("Canvasview: Before Final time set up");
	//MORE TIME STUFF
	time_window_adjustment().set_value(get_canvas()->rend_desc().get_time_start());
	time_window_adjustment().value_changed();


	GRAB_HINT_DATA("canvas_view");
	/*
	{
	set_skip_taskbar_hint(true);
	set_skip_pager_hint(true);
	set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);
	}
	*/

	refresh_rend_desc();
	hide_tables();

	on_time_changed();
	//synfig::info("Canvasview: Constructor Done");
}

CanvasView::~CanvasView()
{
	signal_deleted()();

	App::ui_manager()->remove_action_group(action_group);

	// Shut down the smach
	smach_.egress();
	smach_.set_default_state(0);

	// We want to ensure that the UI_Manager and
	// the selection manager get destructed right now.
	ui_interface_.reset();
	selection_manager_.reset();

	// Delete any external widgets
	for(;!ext_widget_book_.empty();ext_widget_book_.erase(ext_widget_book_.begin()))
	{
		if(ext_widget_book_.begin()->second)
			delete ext_widget_book_.begin()->second;
	}

	//delete preview
	audio.reset();

	hide();

	synfig::info("CanvasView:~CanvasView(): Destructor Finished");
}



Gtk::Widget *
CanvasView::create_time_bar()
{
	Gtk::Image *icon;

	Gtk::HScrollbar *time_window_scroll = manage(new class Gtk::HScrollbar(time_window_adjustment()));
	//Gtk::HScrollbar *time_scroll = manage(new class Gtk::HScrollbar(time_adjustment()));
	//TIME BAR TEMPORARY POSITION
	//Widget_Timeslider *time_scroll = manage(new Widget_Timeslider);
	timeslider->show();
	timeslider->set_time_adjustment(&time_adjustment());
	timeslider->set_bounds_adjustment(&time_window_adjustment());
	//layout_table->attach(*timeslider, 0, 1, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL);


	tooltips.set_tip(*time_window_scroll,_("Moves the time window"));
	tooltips.set_tip(*timeslider,_("Changes the current time"));
	time_window_scroll->show();
	timeslider->show();
	time_window_scroll->set_flags(Gtk::CAN_FOCUS);
	timeslider->set_flags(Gtk::CAN_FOCUS);

	//time_scroll->signal_value_changed().connect(sigc::mem_fun(*work_area, &studio::WorkArea::render_preview_hook));
	//time_scroll->set_update_policy(Gtk::UPDATE_DISCONTINUOUS);

	NORMAL_BUTTON(animatebutton,"gtk-yes","Animate");
	animatebutton->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasView::on_animate_button_pressed));
	animatebutton->show();

	NORMAL_BUTTON(keyframebutton,"synfig-keyframe_lock_all","All Keyframes Locked");
	keyframebutton->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasView::on_keyframe_button_pressed));
	keyframebutton->show();

	Gtk::Table *table= manage(new class Gtk::Table(2, 3, false));

	//setup the audio display
	disp_audio->set_size_request(-1,32); //disp_audio->show();
	disp_audio->set_time_adjustment(&time_adjustment());
	disp_audio->signal_start_scrubbing().connect(
		sigc::mem_fun(*audio,&AudioContainer::start_scrubbing)
	);
	disp_audio->signal_scrub().connect(
		sigc::mem_fun(*audio,&AudioContainer::scrub)
	);
	disp_audio->signal_stop_scrubbing().connect(
		sigc::mem_fun(*audio,&AudioContainer::stop_scrubbing)
	);

	table->attach(*manage(disp_audio), 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	table->attach(*timeslider, 0, 1, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK, 0, 0);
	table->attach(*time_window_scroll, 0, 1, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK, 0, 0);

	table->attach(*animatebutton, 1, 2, 0, 3, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	table->attach(*keyframebutton, 2, 3, 0, 3, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	timebar=table;
	table->show();
	return table;
}

Gtk::Widget *
CanvasView::create_work_area()
{
	work_area=std::auto_ptr<WorkArea>(new class studio::WorkArea(canvas_interface_));
	work_area->set_instance(get_instance());
	work_area->set_canvas(get_canvas());
	work_area->set_canvas_view(this);
	work_area->set_progress_callback(get_ui_interface().get());
	work_area->signal_popup_menu().connect(sigc::mem_fun(*this, &studio::CanvasView::popup_main_menu));
	work_area->show();
	return work_area.get();
}


Gtk::Widget*
CanvasView::create_status_bar()
{
	Gtk::Image *icon;
	Gtk::IconSize iconsize=Gtk::IconSize::from_name("synfig-small_icon");
	cancel=false;

	// Create the status bar at the bottom of the window
	Gtk::Table *statusbartable= manage(new class Gtk::Table(7, 1, false));
//	statusbar = manage(new class Gtk::Statusbar()); // This is already done at construction
	progressbar =manage(new class Gtk::ProgressBar());
	SMALL_BUTTON(stopbutton,"gtk-stop","Stop");
	SMALL_BUTTON(refreshbutton,"gtk-refresh","Refresh");
	//SMALL_BUTTON(treetogglebutton,"gtk-go-down","Toggle Layer Tree");
//	NEW_SMALL_BUTTON(raisebutton,"gtk-go-up","Raise Layer");
//	NEW_SMALL_BUTTON(lowerbutton,"gtk-go-down","Lower Layer");
	//statusbartable->attach(*treetogglebutton, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
//	statusbartable->attach(*lowerbutton, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
//	statusbartable->attach(*raisebutton, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);


	current_time_widget=manage(new Widget_Time);
	current_time_widget->set_value(get_time());
	current_time_widget->set_fps(get_canvas()->rend_desc().get_frame_rate());
	current_time_widget->signal_value_changed().connect(
		sigc::mem_fun(*this,&CanvasView::on_current_time_widget_changed)
	);

	statusbartable->attach(*current_time_widget, 0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	statusbartable->attach(*statusbar, 3, 4, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	statusbartable->attach(*progressbar, 4, 5, 0, 1, Gtk::SHRINK, Gtk::EXPAND|Gtk::FILL, 0, 0);
	statusbartable->attach(*refreshbutton, 5, 6, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	statusbartable->attach(*stopbutton, 6, 7, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	statusbar->set_has_resize_grip(false);
	statusbar->show();
	stopbutton->show();
	refreshbutton->show();
	progressbar->show();
	stopbutton->set_sensitive(false);

	//refreshbutton->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasView::on_refresh_pressed));
	//stopbutton->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasView::stop));
	//treetogglebutton->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasView::toggle_tables));

	refreshbutton->signal_clicked().connect(SLOT_EVENT(EVENT_REFRESH));
	stopbutton->signal_clicked().connect(SLOT_EVENT(EVENT_STOP));


	statusbartable->show_all();
	return statusbartable;
}

void
CanvasView::on_current_time_widget_changed()
{
	set_time(current_time_widget->get_value());
}

Gtk::Widget*
CanvasView::create_children_tree()
{
	// Create the layer tree
	children_tree=manage(new class ChildrenTree());

	// Set up the layer tree
	//children_tree->set_model(children_tree_store());
	if(children_tree)children_tree->set_time_adjustment(time_adjustment());
	if(children_tree)children_tree->show();

	// Connect Signals
	if(children_tree)children_tree->signal_edited_value().connect(sigc::mem_fun(*this, &studio::CanvasView::on_edited_value));
	if(children_tree)children_tree->signal_user_click().connect(sigc::mem_fun(*this, &studio::CanvasView::on_children_user_click));
	if(children_tree)children_tree->signal_waypoint_clicked().connect(sigc::mem_fun(*this, &studio::CanvasView::on_waypoint_clicked));
	if(children_tree)children_tree->get_selection()->signal_changed().connect(SLOT_EVENT(EVENT_REFRESH_DUCKS));

	return children_tree;
}

Gtk::Widget*
CanvasView::create_keyframe_tree()
{
	keyframe_tree=manage(new KeyframeTree());

	//keyframe_tree->get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	//keyframe_tree->show();
	//keyframe_tree->set_model(keyframe_tree_store());
	keyframe_tree->set_editable(true);
	//keyframe_tree->signal_edited().connect(sigc::hide_return(sigc::mem_fun(*canvas_interface(), &synfigapp::CanvasInterface::update_keyframe)));

	keyframe_tree->signal_event().connect(sigc::mem_fun(*this, &studio::CanvasView::on_keyframe_tree_event));

	Gtk::ScrolledWindow *scroll_layer_tree = manage(new class Gtk::ScrolledWindow());
	scroll_layer_tree->set_flags(Gtk::CAN_FOCUS);
	scroll_layer_tree->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scroll_layer_tree->add(*keyframe_tree);
	scroll_layer_tree->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	//scroll_layer_tree->show();


	Gtk::Table *layout_table= manage(new Gtk::Table(1, 2, false));
	layout_table->attach(*scroll_layer_tree, 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	Gtk::Image *icon;
	Gtk::IconSize iconsize(Gtk::IconSize::from_name("synfig-small_icon"));

	NEW_SMALL_BUTTON(button_add,"gtk-add","New Keyframe");
	NEW_SMALL_BUTTON(button_duplicate,"synfig-duplicate","Duplicate Keyframe");
	NEW_SMALL_BUTTON(button_delete,"gtk-delete","Delete Keyframe");

	Gtk::HBox *hbox(manage(new Gtk::HBox()));
	layout_table->attach(*hbox, 0, 1, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK, 0, 0);

	hbox->pack_start(*button_add,Gtk::PACK_SHRINK);
	hbox->pack_start(*button_duplicate,Gtk::PACK_SHRINK);
	hbox->pack_start(*button_delete,Gtk::PACK_SHRINK);

	/*
	button_raise->set_relief(Gtk::RELIEF_HALF);
	button_lower->set_relief(Gtk::RELIEF_HALF);
	button_duplicate->set_relief(Gtk::RELIEF_HALF);
	button_delete->set_relief(Gtk::RELIEF_HALF);
	*/

	button_add->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasView::on_keyframe_add_pressed));
	button_duplicate->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasView::on_keyframe_duplicate_pressed));
	button_delete->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasView::on_keyframe_remove_pressed));

	//layout_table->show_all();

	keyframe_tab_child=layout_table;


	layout_table->hide();

	return layout_table;
}

Gtk::Widget*
CanvasView::create_layer_tree()
{
	// Create the layer tree
	layer_tree=manage(new class LayerTree());

	// Set up the layer tree
	//layer_tree->set_model(layer_tree_store());
	layer_tree->set_time_adjustment(time_adjustment());
	layer_tree->show();

	// Connect Signals
	layer_tree->signal_layer_toggle().connect(sigc::mem_fun(*this, &studio::CanvasView::on_layer_toggle));
	layer_tree->signal_edited_value().connect(sigc::mem_fun(*this, &studio::CanvasView::on_edited_value));
	layer_tree->signal_layer_user_click().connect(sigc::mem_fun(*this, &studio::CanvasView::on_layer_user_click));
	layer_tree->signal_param_user_click().connect(sigc::mem_fun(*this, &studio::CanvasView::on_children_user_click));
	layer_tree->signal_waypoint_clicked().connect(sigc::mem_fun(*this, &studio::CanvasView::on_waypoint_clicked));
	layer_tree->get_selection()->signal_changed().connect(SLOT_EVENT(EVENT_REFRESH_DUCKS));

	layer_tree->hide();
	return layer_tree;
}

void
CanvasView::init_menus()
{
/*
	mainmenu.set_accel_group(get_accel_group());
	mainmenu.set_accel_path("<Canvas-view>");

	filemenu.set_accel_group(get_accel_group());
	filemenu.set_accel_path("<Canvas-view>/File");

	editmenu.set_accel_group(get_accel_group());
	editmenu.set_accel_path("<Canvas-view>/Edit");

	layermenu.set_accel_group(get_accel_group());
	layermenu.set_accel_path("<Canvas-view>/Layer");
*/
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
	action_group = Gtk::ActionGroup::create();

	//action_group->add( Gtk::Action::create("MenuFile", "_File") );
	action_group->add( Gtk::Action::create("save", Gtk::Stock::SAVE),
		hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::save))
	);
	action_group->add( Gtk::Action::create("save-as", Gtk::Stock::SAVE_AS),
		sigc::hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::dialog_save_as))
	);
	action_group->add( Gtk::Action::create("revert", Gtk::Stock::REVERT_TO_SAVED),
		sigc::hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::safe_revert))
	);
	action_group->add( Gtk::Action::create("cvs-add", Gtk::StockID("synfig-cvs_add")),
		sigc::hide_return(sigc::mem_fun(*get_instance(), &studio::Instance::dialog_cvs_add))
	);
	action_group->add( Gtk::Action::create("cvs-update", Gtk::StockID("synfig-cvs_update")),
		sigc::hide_return(sigc::mem_fun(*get_instance(), &studio::Instance::dialog_cvs_update))
	);
	action_group->add( Gtk::Action::create("cvs-revert", Gtk::StockID("synfig-cvs_revert")),
		sigc::hide_return(sigc::mem_fun(*get_instance(), &studio::Instance::dialog_cvs_revert))
	);
	action_group->add( Gtk::Action::create("cvs-commit", Gtk::StockID("synfig-cvs_commit")),
		sigc::hide_return(sigc::mem_fun(*get_instance(), &studio::Instance::dialog_cvs_commit))
	);
	action_group->add( Gtk::Action::create("import", _("Import")),
		sigc::hide_return(sigc::mem_fun(*this, &studio::CanvasView::image_import))
	);
	action_group->add( Gtk::Action::create("render", _("Render")),
		sigc::mem_fun0(render_settings,&studio::RenderSettings::present)
	);
	action_group->add( Gtk::Action::create("preview", _("Preview")),
		sigc::mem_fun(*this,&CanvasView::on_preview_option)
	);
	action_group->add( Gtk::Action::create("sound", _("Sound File")),
		sigc::mem_fun(*this,&CanvasView::on_audio_option)
	);
	action_group->add( Gtk::Action::create("options", _("Options")),
		sigc::mem_fun0(canvas_options,&studio::CanvasOptions::present)
	);
	action_group->add( Gtk::Action::create("close", Gtk::StockID("gtk-close")),
		sigc::hide_return(sigc::mem_fun(*this,&studio::CanvasView::close))
	);

	//action_group->add( Gtk::Action::create("undo", Gtk::StockID("gtk-undo")),
	//	SLOT_EVENT(EVENT_UNDO)
	//);

	//action_group->add( Gtk::Action::create("redo", Gtk::StockID("gtk-redo")),
	//	SLOT_EVENT(EVENT_REDO)
	//);

	action_group->add( Gtk::Action::create("select-all-ducks", _("Select All Ducks")),
		sigc::mem_fun(*work_area,&studio::WorkArea::select_all_ducks)
	);

	action_group->add( Gtk::Action::create("unselect-all-layers", _("Unselect All Layers")),
		sigc::mem_fun(*this,&CanvasView::on_unselect_layers)
	);

	action_group->add( Gtk::Action::create("stop", Gtk::StockID("gtk-stop")),
		SLOT_EVENT(EVENT_STOP)
	);

	action_group->add( Gtk::Action::create("refresh", Gtk::StockID("gtk-refresh")),
		SLOT_EVENT(EVENT_REFRESH)
	);

	action_group->add( Gtk::Action::create("properties", Gtk::StockID("gtk-properties")),
		sigc::mem_fun0(canvas_properties,&studio::CanvasProperties::present)
	);

	// Preview Quality Menu
	{
		int i;
		action_group->add( Gtk::RadioAction::create(quality_group,"quality-00", _("Use Parametric Renderer")),
			sigc::bind(
				sigc::mem_fun(*work_area, &studio::WorkArea::set_quality),
				0
			)
		);
		for(i=1;i<=10;i++)
		{
			Glib::RefPtr<Gtk::RadioAction> action(Gtk::RadioAction::create(quality_group,strprintf("quality-%02d",i), strprintf("Set Quality to %d",i)));
			if(i==10)
				action->property_value()=10;
			action_group->add( action,
				sigc::bind(
					sigc::mem_fun(*work_area, &studio::WorkArea::set_quality),
					i
				)
			);
		}
	}

	action_group->add( Gtk::Action::create("play", Gtk::StockID("synfig-play")),
		sigc::mem_fun(*this, &studio::CanvasView::play)
	);

	action_group->add( Gtk::Action::create("dialog-flipbook", _("Flipbook Dialog")),
		sigc::mem_fun0(*preview_dialog, &studio::Dialog_Preview::present)
	);

	action_group->add( Gtk::Action::create("toggle-grid-show", _("Toggle Grid Show")),
		sigc::mem_fun(*work_area, &studio::WorkArea::toggle_grid)
	);
	action_group->add( Gtk::Action::create("toggle-grid-snap", _("Toggle Grid Snap")),
		sigc::mem_fun(*work_area, &studio::WorkArea::toggle_grid_snap)
	);
	action_group->add( Gtk::Action::create("toggle-guide-show", _("Toggle Guide Show")),
		sigc::mem_fun(*work_area, &studio::WorkArea::toggle_guide_snap)
	);
	action_group->add( Gtk::Action::create("toggle-low-res", _("Toggle Low-Res")),
		sigc::mem_fun(*work_area, &studio::WorkArea::toggle_low_resolution_flag)
	);
	action_group->add( Gtk::Action::create("toggle-onion-skin", _("Toggle Onion Skin")),
		sigc::mem_fun(*work_area, &studio::WorkArea::toggle_onion_skin)
	);


	action_group->add( Gtk::Action::create("canvas-zoom-fit", Gtk::StockID("gtk-zoom-fit")),
		sigc::mem_fun(*work_area, &studio::WorkArea::zoom_fit)
	);
	action_group->add( Gtk::Action::create("canvas-zoom-100", Gtk::StockID("gtk-zoom-100")),
		sigc::mem_fun(*work_area, &studio::WorkArea::zoom_norm)
	);


	{
		Glib::RefPtr<Gtk::Action> action;

		action=Gtk::Action::create("seek-next-frame", Gtk::Stock::GO_FORWARD,_("Next Frame"),_("Next Frame"));
		action_group->add(action,sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek_frame),1));
		action=Gtk::Action::create("seek-prev-frame", Gtk::Stock::GO_BACK,_("Prev Frame"),_("Prev Frame"));
		action_group->add( action, sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek_frame),-1));

		action=Gtk::Action::create("seek-next-second", Gtk::Stock::GO_FORWARD,_("Seek Foward"),_("Seek Foward"));
		action_group->add(action,sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek_time),Time(1)));
		action=Gtk::Action::create("seek-prev-second", Gtk::Stock::GO_BACK,_("Seek Backward"),_("Seek Backward"));
		action_group->add( action, sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek_time),Time(-1)));

		action=Gtk::Action::create("seek-end", Gtk::Stock::GOTO_LAST,_("Seek to End"),_("Seek to End"));
		action_group->add(action,sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek_time),Time::end()));

		action=Gtk::Action::create("seek-begin", Gtk::Stock::GOTO_FIRST,_("Seek to Begin"),_("Seek to Begin"));
		action_group->add( action, sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek_time),Time::begin()));

		action=Gtk::Action::create("jump-next-keyframe", Gtk::Stock::GO_FORWARD,_("Jump to Next Keyframe"),_("Jump to Next Keyframe"));
		action_group->add( action,sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::jump_to_next_keyframe));

		action=Gtk::Action::create("jump-prev-keyframe", Gtk::Stock::GO_BACK,_("Jump to Prev Keyframe"),_("Jump to Prev Keyframe"));
		action_group->add( action,sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::jump_to_prev_keyframe));

		action=Gtk::Action::create("canvas-zoom-in", Gtk::Stock::ZOOM_IN);
		action_group->add( action,sigc::mem_fun(*work_area, &studio::WorkArea::zoom_in));

		action=Gtk::Action::create("canvas-zoom-out", Gtk::Stock::ZOOM_OUT);
		action_group->add( action, sigc::mem_fun(*work_area, &studio::WorkArea::zoom_out) );

		action=Gtk::Action::create("time-zoom-in", Gtk::Stock::ZOOM_IN, _("Zoom In on Timeline"));
		action_group->add( action, sigc::mem_fun(*this, &studio::CanvasView::time_zoom_in) );

		action=Gtk::Action::create("time-zoom-out", Gtk::Stock::ZOOM_OUT, _("Zoom Out on Timeline"));
		action_group->add( action, sigc::mem_fun(*this, &studio::CanvasView::time_zoom_out) );

	}


#define DUCK_MASK(lower,upper)	\
	duck_mask_##lower=Gtk::ToggleAction::create("mask-" #lower "-ducks", _("Show "#lower" ducks")); \
	duck_mask_##lower->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_##upper)); \
	action_group->add( duck_mask_##lower, \
		sigc::bind( \
			sigc::mem_fun(*this, &studio::CanvasView::toggle_duck_mask), \
			Duck::TYPE_##upper \
		) \
	)
	DUCK_MASK(position,POSITION);
	DUCK_MASK(tangent,TANGENT);
	DUCK_MASK(vertex,VERTEX);
	DUCK_MASK(radius,RADIUS);
	DUCK_MASK(width,WIDTH);
	DUCK_MASK(angle,ANGLE);
#undef DUCK_MASK

	add_accel_group(App::ui_manager()->get_accel_group());

/*	// Here is where we add the actions that may have conflicting
	// keyboard accelerators.
	{
		Glib::RefPtr<Gtk::ActionGroup> accel_action_group(Gtk::ActionGroup::create("canvas_view"));
		Glib::RefPtr<Gtk::Action> action;

		action=Gtk::Action::create("seek-next-frame", Gtk::StockID("gtk-forward"),_("Next Frame"),_("Next Frame"));
		accel_action_group->add(action,sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek),1));

		action=Gtk::Action::create("seek-prev-frame", Gtk::StockID("gtk-forward"),_("Prev Frame"),_("Prev Frame"));
		accel_action_group->add( action, sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek),-1));

		action=Gtk::Action::create("jump-next-keyframe", Gtk::StockID("gtk-forward"),_("Jump to Next Keyframe"),_("Jump to Next Keyframe"));
		accel_action_group->add( action,sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::jump_to_next_keyframe));

		action=Gtk::Action::create("jump-prev-keyframe", Gtk::StockID("gtk-back"),_("Jump to Prev Keyframe"),_("Jump to Prev Keyframe"));
		accel_action_group->add( action,sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::jump_to_prev_keyframe));

		action=Gtk::Action::create("canvas-zoom-in", Gtk::StockID("gtk-zoom-in"));
		accel_action_group->add( action,sigc::mem_fun(*work_area, &studio::WorkArea::zoom_in));

		action=Gtk::Action::create("canvas-zoom-out", Gtk::StockID("gtk-zoom-out"));
		accel_action_group->add( action, sigc::mem_fun(*work_area, &studio::WorkArea::zoom_out) );

		action=Gtk::Action::create("time-zoom-in", Gtk::StockID("gtk-zoom-in"), _("Zoom In on Timeline"));
		accel_action_group->add( action, sigc::mem_fun(*this, &studio::CanvasView::time_zoom_in) );

		action=Gtk::Action::create("time-zoom-out", Gtk::StockID("gtk-zoom-out"), _("Zoom Out on Timeline"));
		accel_action_group->add( action, sigc::mem_fun(*this, &studio::CanvasView::time_zoom_out) );

		Glib::RefPtr<Gtk::UIManager> accel_ui_manager(Gtk::UIManager::create());

		Glib::ustring ui_info =
		"
		<ui>
			<accelerator action='seek-next-frame' />
			<accelerator action='seek-prev-frame' />
			<accelerator action='jump-next-keyframe' />
			<accelerator action='jump-prev-keyframe' />
			<accelerator action='canvas-zoom-in' />
			<accelerator action='canvas-zoom-out' />
			<accelerator action='time-zoom-in' />
			<accelerator action='time-zoom-out' />
		</ui>
		";

		accel_ui_manager->add_ui_from_string(ui_info);
		add_accel_group(accel_ui_manager->get_accel_group());

		accel_ui_manager->insert_action_group(accel_action_group);
		set_ref_obj("accel_ui_manager",accel_ui_manager);
		set_ref_obj("accel_action_group",accel_action_group);
	}
*/



#if 0

	//Test some key stuff

	filemenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-save"),
		hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::save))));
	filemenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-save-as"),sigc::hide_return(sigc::mem_fun(*get_instance(), &studio::Instance::dialog_save_as))));
	filemenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-revert-to-saved"),hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::safe_revert))));
 	filemenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());

	filemenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("synfig-cvs_add"),sigc::hide_return(sigc::mem_fun(*get_instance(), &studio::Instance::dialog_cvs_add))));
	filemenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("synfig-cvs_update"),sigc::hide_return(sigc::mem_fun(*get_instance(), &studio::Instance::dialog_cvs_update))));
	filemenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("synfig-cvs_commit"),sigc::hide_return(sigc::mem_fun(*get_instance(), &studio::Instance::dialog_cvs_commit))));

 	filemenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	filemenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Import..."),Gtk::AccelKey('I',Gdk::CONTROL_MASK),sigc::hide_return(sigc::mem_fun(*this, &studio::CanvasView::image_import))));
 	filemenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	filemenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Render"),Gtk::AccelKey("F9"),
		sigc::mem_fun(render_settings,&studio::RenderSettings::present)
	));
	filemenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Preview"),Gtk::AccelKey("F11"),
		sigc::mem_fun(*this,&CanvasView::on_preview_option)
	));
	filemenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Sound File"),
		sigc::mem_fun(*this,&CanvasView::on_audio_option)
	));

 	filemenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	filemenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Options"),Gtk::AccelKey("F12"),
		sigc::mem_fun(canvas_options,&studio::CanvasOptions::present)
	));
 	filemenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	filemenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-close"),sigc::hide_return(sigc::mem_fun(*this,&studio::CanvasView::close))));

	editmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-undo"),Gtk::AccelKey('Z',Gdk::CONTROL_MASK),SLOT_EVENT(EVENT_UNDO)));
	editmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-redo"),Gtk::AccelKey('R',Gdk::CONTROL_MASK),SLOT_EVENT(EVENT_REDO)));
 	editmenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	editmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-cut"),NOT_IMPLEMENTED_SLOT));
	editmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-copy"),NOT_IMPLEMENTED_SLOT));
	editmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-paste"),NOT_IMPLEMENTED_SLOT));
	editmenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	editmenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Select All Ducks"),Gtk::AccelKey('E',Gdk::CONTROL_MASK),sigc::mem_fun(*work_area,&studio::WorkArea::select_all_ducks)));
	editmenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Unselect All Layers"),Gtk::AccelKey('D',Gdk::CONTROL_MASK),sigc::mem_fun(*this,&CanvasView::on_unselect_layers)));
	editmenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());

	//editmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-stop"),Gtk::AccelKey(GDK_Escape,static_cast<Gdk::ModifierType>(0)),sigc::hide_return(sigc::mem_fun(*this, &studio::CanvasView::stop))));
	//editmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-refresh"),Gtk::AccelKey('k',Gdk::CONTROL_MASK),sigc::hide_return(sigc::mem_fun(*this, &studio::CanvasView::on_refresh_pressed))));
	editmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-stop"),Gtk::AccelKey(GDK_Escape,static_cast<Gdk::ModifierType>(0)),SLOT_EVENT(EVENT_STOP)));
	editmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-refresh"),Gtk::AccelKey('k',Gdk::CONTROL_MASK),SLOT_EVENT(EVENT_REFRESH)));
	editmenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
	editmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-properties"),Gtk::AccelKey("F8"),
		sigc::mem_fun(canvas_properties,&studio::CanvasProperties::present)
	));

	build_new_layer_menu(newlayermenu);
	layermenu.items().push_back(Gtk::Menu_Helpers::MenuElem("New",newlayermenu));


	{
		synfigapp::Action::ParamList param_list;
		param_list.add("canvas",Canvas::Handle(get_canvas()));
		param_list.add("canvas_interface",canvas_interface());
		add_actions_to_menu(&canvasmenu, param_list,synfigapp::Action::CATEGORY_CANVAS);
	}


	//canvasmenu.items().push_back(Gtk::Menu_Helpers::MenuElem("Keyframe Dialog",sigc::mem_fun(keyframe_dialog,&studio::Dialog_Keyframe::present)));

	// Duck Mask Menu
	if(1)
		{
		duckmaskmenu.items().push_back(Gtk::Menu_Helpers::TearoffMenuElem());

		duckmaskmenu.items().push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Position Ducks"),Gtk::AccelKey('1',Gdk::MOD1_MASK)));
		duck_mask_position=static_cast<Gtk::CheckMenuItem*>(&duckmaskmenu.items().back());
		duck_mask_position->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_POSITION));
		duck_mask_position->signal_toggled().connect(
			sigc::bind(
				sigc::mem_fun(*this, &studio::CanvasView::toggle_duck_mask),
				Duck::TYPE_POSITION
			)
		);

		duckmaskmenu.items().push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Vertex Ducks"),Gtk::AccelKey('2',Gdk::MOD1_MASK)));
		duck_mask_vertex=static_cast<Gtk::CheckMenuItem*>(&duckmaskmenu.items().back());
		duck_mask_vertex->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_VERTEX));
		duck_mask_vertex->signal_toggled().connect(
			sigc::bind(
				sigc::mem_fun(*this, &studio::CanvasView::toggle_duck_mask),
				Duck::TYPE_VERTEX
			)
		);

		duckmaskmenu.items().push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Tangent Ducks"),Gtk::AccelKey('3',Gdk::MOD1_MASK)));
		duck_mask_tangent=static_cast<Gtk::CheckMenuItem*>(&duckmaskmenu.items().back());
		duck_mask_tangent->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_TANGENT));
		duck_mask_tangent->signal_toggled().connect(
			sigc::bind(
				sigc::mem_fun(*this, &studio::CanvasView::toggle_duck_mask),
				Duck::TYPE_TANGENT
			)
		);

		duckmaskmenu.items().push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Radius Ducks"),Gtk::AccelKey('4',Gdk::MOD1_MASK)));
		duck_mask_radius=static_cast<Gtk::CheckMenuItem*>(&duckmaskmenu.items().back());
		duck_mask_radius->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_RADIUS));
		duck_mask_radius->signal_toggled().connect(
			sigc::bind(
				sigc::mem_fun(*this, &studio::CanvasView::toggle_duck_mask),
				Duck::TYPE_RADIUS
			)
		);

		duckmaskmenu.items().push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Width Ducks"),Gtk::AccelKey('5',Gdk::MOD1_MASK)));
		duck_mask_width=static_cast<Gtk::CheckMenuItem*>(&duckmaskmenu.items().back());
		duck_mask_width->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_WIDTH));
		duck_mask_width->signal_toggled().connect(
			sigc::bind(
				sigc::mem_fun(*this, &studio::CanvasView::toggle_duck_mask),
				Duck::TYPE_WIDTH
			)
		);

		duckmaskmenu.items().push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Angle Ducks"),Gtk::AccelKey('6',Gdk::MOD1_MASK)));
		duck_mask_angle=static_cast<Gtk::CheckMenuItem*>(&duckmaskmenu.items().back());
		duck_mask_angle->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_ANGLE));
		duck_mask_angle->signal_toggled().connect(
			sigc::bind(
				sigc::mem_fun(*this, &studio::CanvasView::toggle_duck_mask),
				Duck::TYPE_ANGLE
			)
		);

		viewmenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("_Mask Ducks"),duckmaskmenu));
	}

	// Preview Quality Menu
	if(1)
	{
		qualitymenu.items().push_back(Gtk::Menu_Helpers::TearoffMenuElem());
		int i;
		qualitymenu.items().push_back(Gtk::Menu_Helpers::MenuElem(strprintf(_("Use Parametric Renderer"),0),
			sigc::bind(
				sigc::mem_fun(*work_area, &studio::WorkArea::set_quality),
				0
			)
		));
		for(i=1;i<=10;i++)
		{
			qualitymenu.items().push_back(Gtk::Menu_Helpers::MenuElem(strprintf(_("Set Quality to %d"),i),Gtk::AccelKey('0'+(i%10),Gdk::CONTROL_MASK),
				sigc::bind(
					sigc::mem_fun(*work_area, &studio::WorkArea::set_quality),
					i
				)
			));
		}
		viewmenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Preview Quality"),qualitymenu));
	}

	viewmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("synfig-play"),
		sigc::mem_fun(*this, &studio::CanvasView::play)));
	viewmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("Flipbook Dialog"),
		sigc::mem_fun(*preview_dialog, &studio::Dialog_Preview::present)));

	viewmenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());

	viewmenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Toggle Grid Show"),Gtk::AccelKey('g',Gdk::CONTROL_MASK),
		sigc::mem_fun(*work_area, &studio::WorkArea::toggle_grid)));
	viewmenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Toggle Grid Snap"),Gtk::AccelKey('l',Gdk::CONTROL_MASK),
		sigc::mem_fun(*work_area, &studio::WorkArea::toggle_grid_snap)));
	viewmenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Toggle Guide Snap"),Gtk::AccelKey('k',Gdk::CONTROL_MASK),
		sigc::mem_fun(*work_area, &studio::WorkArea::toggle_guide_snap)));
	viewmenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Toggle Low-Res"),Gtk::AccelKey('`',Gdk::CONTROL_MASK),
		sigc::mem_fun(*work_area, &studio::WorkArea::toggle_low_resolution_flag)));

 	viewmenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());

	viewmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-zoom-in"),Gtk::AccelKey('=',static_cast<Gdk::ModifierType>(0)),
		sigc::mem_fun(*work_area, &studio::WorkArea::zoom_in)));
	viewmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-zoom-out"),Gtk::AccelKey('-',static_cast<Gdk::ModifierType>(0)),
		sigc::mem_fun(*work_area, &studio::WorkArea::zoom_out)));
	viewmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-zoom-fit"),
		sigc::mem_fun(*work_area, &studio::WorkArea::zoom_fit)));
	viewmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-zoom-100"),Gtk::AccelKey('`',static_cast<Gdk::ModifierType>(0)),
		sigc::mem_fun(*work_area, &studio::WorkArea::zoom_norm)));
 	viewmenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());

	viewmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-zoom-in"),Gtk::AccelKey('+',static_cast<Gdk::ModifierType>(0)),
		sigc::mem_fun(*this, &studio::CanvasView::time_zoom_in)));
	viewmenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-zoom-out"),Gtk::AccelKey('_',static_cast<Gdk::ModifierType>(0)),
		sigc::mem_fun(*this, &studio::CanvasView::time_zoom_out)));

	viewmenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Jump to Next Keyframe"),Gtk::AccelKey(']',static_cast<Gdk::ModifierType>(0)),
		sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::jump_to_next_keyframe)));
	viewmenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Jump to Prev Keyframe"),Gtk::AccelKey('[',static_cast<Gdk::ModifierType>(0)),
		sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::jump_to_prev_keyframe)));

	mainmenu.items().push_back(Gtk::Menu_Helpers::TearoffMenuElem());
	mainmenu.items().push_back(Gtk::Menu_Helpers::MenuElem("_File",filemenu));
	mainmenu.items().push_back(Gtk::Menu_Helpers::MenuElem("_Edit",editmenu));
	mainmenu.items().push_back(Gtk::Menu_Helpers::MenuElem("_View",viewmenu));
	mainmenu.items().push_back(Gtk::Menu_Helpers::MenuElem("_Canvas",canvasmenu));
	mainmenu.items().push_back(Gtk::Menu_Helpers::MenuElem("_Layer",layermenu));

	mainmenu.accelerate(*this);

	{

		trackmenu.items().push_back(Gtk::Menu_Helpers::MenuElem("New Waypoint",NOT_IMPLEMENTED_SLOT));
		trackmenu.items().push_back(Gtk::Menu_Helpers::MenuElem("Delete Waypoint",NOT_IMPLEMENTED_SLOT));
		trackmenu.items().push_back(Gtk::Menu_Helpers::MenuElem("Export",NOT_IMPLEMENTED_SLOT));
		trackmenu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
		trackmenu.items().push_back(Gtk::Menu_Helpers::MenuElem("Properties",NOT_IMPLEMENTED_SLOT));
	}
	mainmenu.show();
	filemenu.show();
	editmenu.show();
	canvasmenu.show();
	layermenu.show();

	keyframemenu.items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-properties"),
		sigc::mem_fun(*this,&studio::CanvasView::show_keyframe_dialog)
	));


	get_accel_group()->unlock();

	//Set the accelerator paths for all the menus
	filemenu.set_accel_path("<synfig>/File");
	editmenu.set_accel_path("<synfig>/Edit");
	layermenu.set_accel_path("<synfig>/Layer");
	//mainmenu.set_accel_path("<synfig-main>");
	canvasmenu.set_accel_path("<synfig>/Canvas");
	viewmenu.set_accel_path("<synfig>/View");
	duckmaskmenu.set_accel_path("<synfig>/DuckMask");
#endif
}

void
CanvasView::on_unselect_layers()
{
	layer_tree->clear_selected_layers();
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
CanvasView::add_layer(synfig::String x)
{
	Canvas::Handle canvas;

	synfigapp::SelectionManager::LayerList layer_list(get_selection_manager()->get_selected_layers());

	int target_depth(0);

	if(layer_list.empty())
	{
		canvas=get_canvas();
	}
	else
	{
		canvas=(*layer_list.begin())->get_canvas();
		target_depth=canvas->get_depth(*layer_list.begin());
	}


	Layer::Handle layer(canvas_interface()->add_layer_to(x,canvas,target_depth));
	if(layer)
	{
		get_selection_manager()->clear_selected_layers();
		get_selection_manager()->set_selected_layer(layer);
	}
}

void
CanvasView::popup_layer_menu(synfig::Layer::Handle layer)
{
	//Gtk::Menu* menu(manage(new Gtk::Menu));
	Gtk::Menu* menu(&parammenu);
	menu->items().clear();

	synfigapp::Action::ParamList param_list;
	param_list.add("time",canvas_interface()->get_time());
	param_list.add("canvas",Canvas::Handle(layer->get_canvas()));
	param_list.add("canvas_interface",canvas_interface());
	param_list.add("layer",layer);

	//Gtk::Menu *newlayers(manage(new Gtk::Menu()));
	//build_new_layer_menu(*newlayers);

	//parammenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("New Layer"),*newlayers));

	if(layer->get_name()=="PasteCanvas")
	{
		menu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Select All Children"),
			sigc::bind(
				sigc::mem_fun(
					*layer_tree,
					&studio::LayerTree::select_all_children_layers
				),
				layer
			)
		));
	}

	add_actions_to_menu(menu, param_list,synfigapp::Action::CATEGORY_LAYER);

	menu->popup(3,gtk_get_current_event_time());
}

void
CanvasView::register_layer_type(synfig::Layer::Book::value_type &lyr,std::map<synfig::String,Gtk::Menu*>* category_map)
{
/*	if(lyr.second.category==_("Do Not Use"))
		return;

	if(category_map->count(lyr.second.category)==0)
		(*category_map)[lyr.second.category]=manage(new Gtk::Menu());

	(*category_map)[lyr.second.category]->items().push_back(Gtk::Menu_Helpers::MenuElem(lyr.second.local_name,
		sigc::hide_return(
			sigc::bind(
				sigc::mem_fun(*this,&studio::CanvasView::add_layer),
				lyr.first
			)
		)
	));
*/
}

void
CanvasView::build_new_layer_menu(Gtk::Menu &menu)
{
/*
	std::map<synfig::String,Gtk::Menu*> category_map;

	std::for_each(
		synfig::Layer::book().begin(),
		synfig::Layer::book().end(),
		sigc::bind(
			sigc::mem_fun(
				*this,
				&studio::CanvasView::register_layer_type
			),
			&category_map
		)
	);

	menu.items().clear();
	menu.items().push_back(Gtk::Menu_Helpers::TearoffMenuElem());

	std::map<synfig::String,Gtk::Menu*>::iterator iter;
	for(iter=category_map.begin();iter!=category_map.end();++iter)
		menu.items().push_back(Gtk::Menu_Helpers::MenuElem(iter->first,*iter->second));

	menu.show();
*/
}

void
CanvasView::popup_main_menu()
{
	//mainmenu.popup(0,gtk_get_current_event_time());
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
	work_area->queue_render_preview();
}

void
CanvasView::workarea_layer_selected(synfig::Layer::Handle layer)
{
	get_selection_manager()->clear_selected_layers();
	if(layer)
		get_selection_manager()->set_selected_layer(layer);
}


void
CanvasView::refresh_rend_desc()
{
	current_time_widget->set_fps(get_canvas()->rend_desc().get_frame_rate());


	//????
	//synfig::info("Canvasview: Refreshing render desc info");
	if(!get_time().is_equal(time_adjustment().get_value()))
	{
		time_adjustment().set_value(get_time());
		time_adjustment().value_changed();
	}

	Time length(get_canvas()->rend_desc().get_time_end()-get_canvas()->rend_desc().get_time_start());
	if(length<DEFAULT_TIME_WINDOW_SIZE)
	{
		time_window_adjustment().set_page_increment(length);
		time_window_adjustment().set_page_size(length);
	}
	else
	{
		time_window_adjustment().set_page_increment(DEFAULT_TIME_WINDOW_SIZE);
		time_window_adjustment().set_page_size(DEFAULT_TIME_WINDOW_SIZE);
	}

	//set the FPS of the timeslider
	timeslider->set_global_fps(get_canvas()->rend_desc().get_frame_rate());

	//set the beginning and ending time of the time slider
	Time begin_time=get_canvas()->rend_desc().get_time_start();
	Time end_time=get_canvas()->rend_desc().get_time_end();

	// Setup the time_window adjustment
	time_window_adjustment().set_lower(begin_time);
	time_window_adjustment().set_upper(end_time);
	time_window_adjustment().set_step_increment(synfig::Time(1.0/get_canvas()->rend_desc().get_frame_rate()));

	//Time length(get_canvas()->rend_desc().get_time_end()-get_canvas()->rend_desc().get_time_start());
	if(length < time_window_adjustment().get_page_size())
	{
		time_window_adjustment().set_page_increment(length);
		time_window_adjustment().set_page_size(length);
	}

	/*synfig::info("w: %p - [%.3f,%.3f] (%.3f,%.3f) child: %p\n",
				&time_window_adjustment_, time_window_adjustment_.get_lower(),
				time_window_adjustment_.get_upper(),time_window_adjustment_.get_value(),
				time_window_adjustment_.get_page_size(),time_window_adjustment_.get_child_adjustment()
	);*/

	time_window_adjustment().changed(); //only non-value stuff was changed

	// Setup the time adjustment

	//NOTE THESE TWO SHOULD BE CHANGED BY THE changed() CALL ABOVE
	//time_adjustment().set_lower(time_window_adjustment().get_value());
	//time_adjustment().set_upper(time_window_adjustment().get_value()+time_window_adjustment().get_page_size());

//	time_adjustment().set_lower(get_canvas()->rend_desc().get_time_start());
//	time_adjustment().set_upper(get_canvas()->rend_desc().get_time_end());
	time_adjustment().set_step_increment(synfig::Time(1.0/get_canvas()->rend_desc().get_frame_rate()));
	time_adjustment().set_page_increment(synfig::Time(1.0));
	time_adjustment().set_page_size(0);

	time_adjustment().changed();

	/*synfig::info("w: %p - [%.3f,%.3f] (%.3f,%.3f) child: %p\n",
				&time_window_adjustment_, time_window_adjustment_.get_lower(),
				time_window_adjustment_.get_upper(),time_window_adjustment_.get_value(),
				time_window_adjustment_.get_page_size(),time_window_adjustment_.get_child_adjustment()
	);	*/

	if(begin_time==end_time)
	{
		hide_timebar();
	}
	else
	{
		show_timebar();
	}

	//clamp time to big bounds...
	if(time_adjustment().get_value() < begin_time)
	{
		time_adjustment().set_value(begin_time);
		time_adjustment().value_changed();
	}

	if(time_adjustment().get_value() > end_time)
	{
		time_adjustment().set_value(end_time);
		time_adjustment().value_changed();
	}

	/*synfig::info("Time stats: \n"
				"w: %p - [%.3f,%.3f] (%.3f,%.3f) child: %p\n"
				"t: %p - [%.3f,%.3f] %.3f",
				&time_window_adjustment_, time_window_adjustment_.get_lower(),
				time_window_adjustment_.get_upper(),time_window_adjustment_.get_value(),
				time_window_adjustment_.get_page_size(),time_window_adjustment_.get_child_adjustment(),
				&time_adjustment_,time_adjustment_.get_lower(),time_adjustment_.get_upper(),
				time_adjustment_.get_value()
	);*/

	work_area->queue_render_preview();
}


bool
CanvasView::close()
{
	get_instance()->safe_close();
	return false;
}

handle<CanvasView>
CanvasView::create(loose_handle<Instance> instance,handle<Canvas> canvas)
{
	etl::handle<studio::CanvasView> view(new CanvasView(instance,instance->synfigapp::Instance::find_canvas_interface(canvas)));
	instance->canvas_view_list().push_front(view);
	instance->signal_canvas_view_created()(view.get());
	return view;
}

void
CanvasView::update_title()
{
	string title;

	title+=etl::basename(get_instance()->get_file_name())
		+" : ";
	if(get_canvas()->get_name().empty())
		title+='"'+get_canvas()->get_id()+'"';
	else
		title+='"'+get_canvas()->get_name()+'"';

	if(get_instance()->synfigapp::Instance::get_action_count())
		title+=_(" (Unsaved)");

	if(get_instance()->synfigapp::Instance::in_repository())
	{
		title+=" (CVS";
		if(get_instance()->synfigapp::Instance::is_modified())
			title+=_("-MODIFIED");
		if(get_instance()->synfigapp::Instance::is_updated())
			title+=_("-UPDATED");
		title+=')';
	}

	if(get_canvas()->is_root())
		title+=_(" (Root)");

	set_title(title);
}


void
CanvasView::on_hide()
{
	smach_.egress();
	Gtk::Window::on_hide();
}

void
CanvasView::present()
{
	grab_focus();//on_focus_in_event(0);
	Gtk::Window::present();
}

bool
CanvasView::on_focus_in_event(GdkEventFocus*x)
{
	if(studio::App::get_selected_canvas_view()!=this)
	{
		if(studio::App::get_selected_canvas_view())
		{
			studio::App::get_selected_canvas_view()->get_smach().process_event(EVENT_YIELD_TOOL_OPTIONS);
			App::ui_manager()->remove_action_group(App::get_selected_canvas_view()->action_group);
		}

		get_smach().process_event(EVENT_REFRESH_TOOL_OPTIONS);

		studio::App::set_selected_canvas_view(this);

		App::ui_manager()->insert_action_group(action_group);
	}

	// HACK ... Questionable...?
	if(x)
		return Gtk::Window::on_focus_in_event(x);

	return true;
}

bool
CanvasView::on_focus_out_event(GdkEventFocus*x)
{
	//App::ui_manager()->remove_action_group(action_group);
	//App::ui_manager()->ensure_update();
	return Gtk::Window::on_focus_out_event(x);
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
CanvasView::on_layer_toggle(synfig::Layer::Handle layer)
{
	synfigapp::Action::Handle action(synfigapp::Action::create("layer_activate"));
	assert(action);

	if(!action)
		return;

	action->set_param("canvas",Canvas::Handle(layer->get_canvas()));
	if(!action->set_param("canvas_interface",canvas_interface()))
//	if(!action->set_param("canvas_interface",get_instance()->find_canvas_interface(layer->get_canvas())))
		synfig::error("LayerActivate didn't like CanvasInterface...?");
	action->set_param("time",get_time());
	action->set_param("layer",layer);
	action->set_param("new_status",!layer->active());

	assert(action->is_ready());

	canvas_interface()->get_instance()->perform_action(action);
}


void
CanvasView::popup_param_menu(synfigapp::ValueDesc value_desc, float location)
{
	parammenu.items().clear();
	get_instance()->make_param_menu(&parammenu,get_canvas(),value_desc,location);

	parammenu.popup(3,gtk_get_current_event_time());
}

void
CanvasView::add_actions_to_menu(Gtk::Menu *menu, const synfigapp::Action::ParamList &param_list,synfigapp::Action::Category category)const
{
	get_instance()->add_actions_to_menu(menu, param_list, category);
}

bool
CanvasView::on_layer_user_click(int button, Gtk::TreeRow row, LayerTree::ColumnID column_id)
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
				menu->get_submenu()->popup(button,gtk_get_current_event_time());
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

			//synfigapp::ValueDesc value_desc(row[layer_param_tree_model.value_desc]);
			//ValueNode::Handle value_node(row[layer_param_tree_model.value_node]);
			//ValueNode::Handle parent_value_node;
			//ValueBase value=row[layer_param_tree_model.value];

			//if(row.parent())
			//{
			//	parent_value_node=(*row.parent())[layer_tree_model.value_node];
			//}

			{
				Layer::Handle layer(row[layer_tree_model.layer]);
				synfigapp::Action::ParamList param_list;
				param_list.add("time",canvas_interface()->get_time());
				param_list.add("canvas",Canvas::Handle(row[layer_tree_model.canvas]));
				param_list.add("canvas_interface",canvas_interface());
				if(!multiple_selected)
					param_list.add("layer",layer);
				else
				{
					synfigapp::SelectionManager::LayerList layer_list(get_selection_manager()->get_selected_layers());
					synfigapp::SelectionManager::LayerList::iterator iter;

					for(iter=layer_list.begin();iter!=layer_list.end();++iter)
						param_list.add("layer",Layer::Handle(*iter));
				}

				parammenu.items().clear();

				Gtk::Menu *newlayers(manage(new Gtk::Menu()));
				build_new_layer_menu(*newlayers);

				parammenu.items().push_back(Gtk::Menu_Helpers::MenuElem("New Layer",*newlayers));
				if(!multiple_selected && layer->get_name()=="PasteCanvas")
				{
					parammenu.items().push_back(Gtk::Menu_Helpers::MenuElem(_("Select All Children"),
						sigc::bind(
							sigc::mem_fun(
								*layer_tree,
								&studio::LayerTree::select_all_children_layers
							),
							layer
						)
					));
				}

				add_actions_to_menu(&parammenu, param_list,synfigapp::Action::CATEGORY_LAYER);
				parammenu.popup(button,gtk_get_current_event_time());
				return true;
			}
/*
			else if(column_id==LayerTree::COLUMNID_TIME_TRACK && value_node && handle<synfig::ValueNode_Animated>::cast_dynamic(value_node))
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
					parammenu.items().push_back(Gtk::Menu_Helpers::MenuElem("Connect",
						hide_return(sigc::mem_fun(*canvas_interface().get(),&synfigapp::CanvasInterface::connect_selected_layer_params))
					));
					parammenu.items().push_back(Gtk::Menu_Helpers::MenuElem("Disconnect",
						hide_return(sigc::mem_fun(*canvas_interface().get(),&synfigapp::CanvasInterface::disconnect_selected_layer_params))
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
		break;

	default:
		return false;
		break;
	}
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
				synfigapp::ValueDesc value_desc=row[children_tree_model.value_desc];
				assert(value_desc);
				popup_param_menu(value_desc);
				return true;
			}
		}
		return true;
		break;

	default:
		return false;
		break;
	}
}

bool
CanvasView::on_keyframe_tree_event(GdkEvent *event)
{
    switch(event->type)
    {
	case GDK_BUTTON_PRESS:
		switch(event->button.button)
		{
			case 3:
			{
				//keyframemenu.popup(event->button.button,gtk_get_current_event_time());
				return true;
			}
			break;
		}
		break;
	case GDK_MOTION_NOTIFY:
		break;
	case GDK_BUTTON_RELEASE:
		break;
	default:
		break;
	}
	return false;
}


void
CanvasView::refresh_time_window()
{
	//THESE SHOULD AUTOMATICALLY BE TAKEN CARE OF
	//time_adjustment().set_lower(time_window_adjustment().get_value());
	//time_adjustment().set_upper(time_window_adjustment().get_value()+time_window_adjustment().get_page_size());

	time_adjustment().set_page_increment(1.0); // One second
	time_adjustment().set_page_size(0);

	if(get_canvas())
		time_adjustment().set_step_increment(1.0/get_canvas()->rend_desc().get_frame_rate());
	time_adjustment().changed();

	//NOTE THIS SHOULD HOOK INTO THE CORRECT SIGNALS...
	if(children_tree)
		children_tree->queue_draw();
}

void
CanvasView::on_time_changed()
{
	Time time(get_time());

	current_time_widget->set_value(time);
	try {
		get_canvas()->keyframe_list().find(time);
		current_time_widget->modify_text(Gtk::STATE_NORMAL,Gdk::Color("#FF0000"));
	}catch(...){
		current_time_widget->modify_text(Gtk::STATE_NORMAL,Gdk::Color("#000000"));
	}

	if(get_time() != time_adjustment().get_value())
	{

		//Recenters the window, causing it to jump (possibly undesirably... but whatever)
		if(time < time_window_adjustment().get_value() ||
			time > time_window_adjustment().get_value()+time_window_adjustment().get_page_size())
		{
			time_window_adjustment().set_value(
				time-time_window_adjustment().get_page_size()/2
			);
		}
		time_adjustment().set_value(time);
		time_adjustment().value_changed();

		// Shouldn't these trees just hook into
		// the time changed signal...?
		//YES THEY SHOULD...
		if(layer_tree)layer_tree->queue_draw();
		if(children_tree)children_tree->queue_draw();
	}
}

void
CanvasView::time_zoom_in()
{
	time_window_adjustment().set_page_size(time_window_adjustment().get_page_size()*0.75);
	time_window_adjustment().changed();

	refresh_time_window();
}

void
CanvasView::time_zoom_out()
{
	time_window_adjustment().set_page_size(time_window_adjustment().get_page_size()/0.75);
	time_window_adjustment().changed();

	refresh_time_window();
}

void
CanvasView::time_was_changed()
{
	synfig::Time time((synfig::Time)(double)time_adjustment().get_value());
	set_time(time);
}

void
CanvasView::on_edited_value(synfigapp::ValueDesc value_desc,synfig::ValueBase new_value)
{
	canvas_interface()->change_value(value_desc,new_value);
}

/*
void
CanvasView::on_children_edited_value(const Glib::ustring&path_string,synfig::ValueBase value)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(children_tree->get_model()->get_iter(path));

	assert((bool)row[children_tree_model.is_value_node]);

	synfigapp::ValueDesc value_desc=row[children_tree_model.value_desc];
	assert(value_desc);

	on_edited_value(value_desc,value);
}
*/

void
CanvasView::on_id_changed()
{
	update_title();
}


void
CanvasView::on_mode_changed(synfigapp::CanvasInterface::Mode mode)
{
	// If the aninimate flag was set in mode...
	if(mode&synfigapp::MODE_ANIMATE)
	{
		Gtk::Image *icon;
		icon=manage(new Gtk::Image(Gtk::StockID("gtk-no"),Gtk::ICON_SIZE_BUTTON));
		animatebutton->remove();
		animatebutton->add(*icon);
		tooltips.set_tip(*animatebutton,_("In Animate Editing Mode"));
		icon->set_padding(0,0);
		icon->show();
	}
	else
	{
		Gtk::Image *icon;
		icon=manage(new Gtk::Image(Gtk::StockID("gtk-yes"),Gtk::ICON_SIZE_BUTTON));
		animatebutton->remove();
		animatebutton->add(*icon);
		tooltips.set_tip(*animatebutton,_("Not in Animate Editing Mode"));
		icon->set_padding(0,0);
		icon->show();
	}

	if((mode&synfigapp::MODE_ANIMATE_FUTURE) && (mode&synfigapp::MODE_ANIMATE_PAST))
	{
		Gtk::Image *icon;
		icon=manage(new Gtk::Image(Gtk::StockID("synfig-keyframe_lock_all"),Gtk::ICON_SIZE_BUTTON));
		keyframebutton->remove();
		keyframebutton->add(*icon);
		tooltips.set_tip(*keyframebutton,_("All Keyframes Locked"));
		icon->set_padding(0,0);
		icon->show();
	}
	else if((mode&synfigapp::MODE_ANIMATE_FUTURE) && !(mode&synfigapp::MODE_ANIMATE_PAST))
	{
		Gtk::Image *icon;
		icon=manage(new Gtk::Image(Gtk::StockID("synfig-keyframe_lock_future"),Gtk::ICON_SIZE_BUTTON));
		keyframebutton->remove();
		keyframebutton->add(*icon);
		tooltips.set_tip(*keyframebutton,_("Future Keyframes Locked"));
		icon->set_padding(0,0);
		icon->show();
	}
	else if(!(mode&synfigapp::MODE_ANIMATE_FUTURE) && (mode&synfigapp::MODE_ANIMATE_PAST))
	{
		Gtk::Image *icon;
		icon=manage(new Gtk::Image(Gtk::StockID("synfig-keyframe_lock_past"),Gtk::ICON_SIZE_BUTTON));
		keyframebutton->remove();
		keyframebutton->add(*icon);
		tooltips.set_tip(*keyframebutton,_("Past Keyframes Locked"));
		icon->set_padding(0,0);
		icon->show();
	}
	else if(!(mode&synfigapp::MODE_ANIMATE_FUTURE) && !(mode&synfigapp::MODE_ANIMATE_PAST))
	{
		Gtk::Image *icon;
		icon=manage(new Gtk::Image(Gtk::StockID("synfig-keyframe_lock_none"),Gtk::ICON_SIZE_BUTTON));
		keyframebutton->remove();
		keyframebutton->add(*icon);
		tooltips.set_tip(*keyframebutton,_("No Keyframes Locked"));
		icon->set_padding(0,0);
		icon->show();
	}

	work_area->queue_draw();
}

void
CanvasView::on_animate_button_pressed()
{
	if(get_mode()&synfigapp::MODE_ANIMATE)
		set_mode(get_mode()-synfigapp::MODE_ANIMATE);
	else
		set_mode(get_mode()|synfigapp::MODE_ANIMATE);
}

void
CanvasView::on_keyframe_button_pressed()
{
	synfigapp::CanvasInterface::Mode mode(get_mode());

	if((mode&synfigapp::MODE_ANIMATE_FUTURE) && (mode&synfigapp::MODE_ANIMATE_PAST))
	{
		set_mode(get_mode()-synfigapp::MODE_ANIMATE_FUTURE);
	}
	else if(!(mode&synfigapp::MODE_ANIMATE_FUTURE) && (mode&synfigapp::MODE_ANIMATE_PAST))
	{
		set_mode(get_mode()-synfigapp::MODE_ANIMATE_PAST|synfigapp::MODE_ANIMATE_FUTURE);
	}
	else if((mode&synfigapp::MODE_ANIMATE_FUTURE) && !(mode&synfigapp::MODE_ANIMATE_PAST))
	{
		set_mode(get_mode()-synfigapp::MODE_ANIMATE_FUTURE);
	}
	else if(!(mode&synfigapp::MODE_ANIMATE_FUTURE) && !(mode&synfigapp::MODE_ANIMATE_PAST))
	{
		set_mode(get_mode()|synfigapp::MODE_ANIMATE_FUTURE|synfigapp::MODE_ANIMATE_PAST);
	}
}

bool
CanvasView::duck_change_param(const Point &value,synfig::Layer::Handle layer, synfig::String param_name)
{
	return canvas_interface()->change_value(synfigapp::ValueDesc(layer,param_name),value);
}

bool
CanvasView::on_duck_changed(const synfig::Point &value,const synfigapp::ValueDesc& value_desc)
{
	switch(value_desc.get_value_type())
	{
	case ValueBase::TYPE_REAL:
		return canvas_interface()->change_value(value_desc,value.mag());
		break;
	case ValueBase::TYPE_ANGLE:
		return canvas_interface()->change_value(value_desc,Angle::tan(value[1],value[0]));
		break;
	default:
		return canvas_interface()->change_value(value_desc,value);
		break;
	}

	return true;
}

void
CanvasView::selected_layer_color_set(Color color)
{
	synfigapp::SelectionManager::LayerList selected_list(get_selection_manager()->get_selected_layers());
	synfigapp::SelectionManager::LayerList::iterator iter;

	// Create the action group
	//synfigapp::PassiveGrouper group(canvas_interface()->get_instance(),_("Set Colors"));

	Layer::Handle layer;
	for(iter=selected_list.begin();iter!=selected_list.end();++iter)
	{
		if(*iter==layer)
			continue;
		layer=*iter;
		on_edited_value(synfigapp::ValueDesc(layer,"color"),color);
	}
}

void
CanvasView::rebuild_ducks_layer_(synfig::TransformStack& transform_stack, Canvas::Handle canvas, std::set<synfig::Layer::Handle>& selected_list)
{
	int transforms(0);
	String layer_name;

#define QUEUE_REBUILD_DUCKS		sigc::mem_fun(*this,&CanvasView::queue_rebuild_ducks)

	if(!canvas)
	{
		synfig::warning("CanvasView::rebuild_ducks_layer_(): Layer doesn't have canvas set");
		return;
	}
	for(Canvas::iterator iter(canvas->begin());iter!=canvas->end();++iter)
	{
		Layer::Handle layer(*iter);

		if(selected_list.count(layer))
		{
			if(!curr_transform_stack_set)
			{
				curr_transform_stack_set=true;
				curr_transform_stack=transform_stack;
			}

			// This layer is currently selected.
			duck_changed_connections.push_back(layer->signal_changed().connect(QUEUE_REBUILD_DUCKS));

			// do the bounding box thing
			bbox|=transform_stack.perform(layer->get_bounding_rect());

			// Grab the layer's list pf parameters
			Layer::ParamList paramlist(layer->get_param_list());

			// Grab the layer vocabulary
			Layer::Vocab vocab=layer->get_param_vocab();
			Layer::Vocab::iterator iter;

			for(iter=vocab.begin();iter!=vocab.end();iter++)
			{
				if(!iter->get_hidden() && !iter->get_invisible_duck())
				{
					synfigapp::ValueDesc value_desc(layer,iter->get_name());
					work_area->add_to_ducks(value_desc,this,transform_stack,&*iter);
					if(value_desc.is_value_node())
						duck_changed_connections.push_back(value_desc.get_value_node()->signal_changed().connect(QUEUE_REBUILD_DUCKS));
				}
				if(iter->get_name()=="color")
				{
					/*
					if(!App::dialog_color->busy())
					{
						App::dialog_color->reset();
						App::dialog_color->set_color(layer->get_param("color").get(Color()));
						App::dialog_color->signal_edited().connect(
							sigc::mem_fun(
								*this,
								&studio::CanvasView::selected_layer_color_set
							)
						);
					}
					*/
				}
			}
		}

		layer_name=layer->get_name();

		if(layer->active())
		{
			Transform::Handle trans(layer->get_transform());
			if(trans)
			{
				transform_stack.push(trans);
				transforms++;
			}

/*			// Add transforms onto the stack
			if(layer_name=="Translate")
			{
				transform_stack.push(synfig::Transform_Translate(layer->get_param("origin").get(Vector())));
				transforms++;
			}else
			if(layer_name=="Zoom")
			{
				Vector scale;
				scale[0]=scale[1]=exp(layer->get_param("amount").get(Real()));
				transform_stack.push(synfig::Transform_Scale(scale,layer->get_param("center").get(Vector())));
				transforms++;
			}else
			if(layer_name=="stretch")
			{
				Vector scale(layer->get_param("amount").get(Vector()));
				transform_stack.push(synfig::Transform_Scale(scale,layer->get_param("center").get(Vector())));
				transforms++;
			}else
			if(layer_name=="Rotate")
			{
				transform_stack.push(synfig::Transform_Rotate(layer->get_param("amount").get(Angle()),layer->get_param("origin").get(Vector())));
				transforms++;
			}
*/
		}

		// If this is a paste canvas layer, then we need to
		// descend into it
		if(layer_name=="PasteCanvas")
		{
			Vector scale;
			scale[0]=scale[1]=exp(layer->get_param("zoom").get(Real()));
			Vector origin(layer->get_param("origin").get(Vector()));

			Canvas::Handle child_canvas(layer->get_param("canvas").get(Canvas::Handle()));

			if(!scale.is_equal_to(Vector(1,1)))
				transform_stack.push(new Transform_Scale(scale,origin));
			if(!scale.is_equal_to(Vector(0,0)))
				transform_stack.push(new Transform_Translate(origin));

			rebuild_ducks_layer_(transform_stack,child_canvas,selected_list);

			if(!scale.is_equal_to(Vector(0,0)))
				transform_stack.pop();
			if(!scale.is_equal_to(Vector(1,1)))
				transform_stack.pop();
		}
	}
	// Remove all of the transforms we have added
	while(transforms--) { transform_stack.pop(); }

#undef QUEUE_REBUILD_DUCKS
}

void
CanvasView::queue_rebuild_ducks()
{
#if 0
	if(rebuild_ducks_queued)
		return;
#else
	if(rebuild_ducks_queued)
		queue_rebuild_ducks_connection.disconnect();
#endif

	queue_rebuild_ducks_connection=Glib::signal_timeout().connect(
		sigc::bind_return(
			sigc::mem_fun(*this,&CanvasView::rebuild_ducks),
			false
		),
		50
	);

	rebuild_ducks_queued=true;
}

void
CanvasView::rebuild_ducks()
{
	/*static int i=0;
	i++;
	if(i>30)
		synfig::info("%d",i/(i-i));
	*/

	rebuild_ducks_queued=false;
	//queue_rebuild_ducks_connection.disconnect();

	if(work_area->is_dragging())
	{
		queue_rebuild_ducks();
		return;
	}

	if(!duck_refresh_flag)
	{
		duck_refresh_needed=true;
		return;
	}

	bbox=Rect::zero();

	work_area->clear_ducks();
	work_area->set_time(get_time());
	get_canvas()->set_time(get_time());
	curr_transform_stack.clear();
	//curr_transform_stack.push(new Transform_Translate(Point(0,0)));
	curr_transform_stack_set=false;

	for(;!duck_changed_connections.empty();duck_changed_connections.pop_back())duck_changed_connections.back().disconnect();

	//get_canvas()->set_time(get_time());
	bool not_empty(false);

	// First do the layers...
	do{
		synfigapp::SelectionManager::LayerList selected_list(get_selection_manager()->get_selected_layers());
		std::set<synfig::Layer::Handle> layer_set(selected_list.begin(),selected_list.end());

		if(!layer_set.empty())
			not_empty=true;

		synfig::TransformStack transform_stack;

		rebuild_ducks_layer_(transform_stack, get_canvas(), layer_set);

	}while(0);

	// Now do the children
	do{
		synfigapp::SelectionManager::ChildrenList selected_list(get_selection_manager()->get_selected_children());
		synfigapp::SelectionManager::ChildrenList::iterator iter;
		synfig::TransformStack transform_stack;

		if(selected_list.empty())
		{
			break;
		}
		else
		{
			not_empty=true;
			for(iter=selected_list.begin();iter!=selected_list.end();++iter)
			{
				work_area->add_to_ducks(*iter,this,transform_stack);
			}
		}
	}while(0);
	work_area->refresh_selected_ducks();
	work_area->queue_draw_preview();
}

void
CanvasView::on_dirty_preview()
{
	if(!is_playing_)
	{
		IsWorking is_working(*this);

		work_area->queue_render_preview();
	}
}

void
CanvasView::play()
{
	assert(get_canvas());

	// If we are already busy, don't play!
	if(working_depth)return;

	// Set us up as working
	IsWorking is_working(*this);

	etl::clock timer;
	Time
		time=work_area->get_time(),
		endtime=get_canvas()->rend_desc().get_time_end();

	// If we are already at the end of time, start over
	if(time==endtime)
		time=get_canvas()->rend_desc().get_time_start();

	is_playing_=true;

	work_area->clear_ducks();

	for(timer.reset(); time + timer() < endtime;)
	{
		//Clamp the time window so we can see the time value as it races across the horizon
		bool timewindreset = false;

		while( time + timer() > Time(time_window_adjustment().get_sub_upper()) )
		{
			time_window_adjustment().set_value(
					min(
						time_window_adjustment().get_value()+time_window_adjustment().get_page_size()/2,
						time_window_adjustment().get_upper()-time_window_adjustment().get_page_size() )
				);
			timewindreset = true;
		}

		while( time + timer() < Time(time_window_adjustment().get_sub_lower()) )
		{
			time_window_adjustment().set_value(
				max(
					time_window_adjustment().get_value()-time_window_adjustment().get_page_size()/2,
					time_window_adjustment().get_lower())
			);

			timewindreset = true;
		}

		//we need to tell people that the value changed
		if(timewindreset) time_window_adjustment().value_changed();

		//update actual time to next step
		time_adjustment().set_value(time+timer());
		time_adjustment().value_changed();

		if(!work_area->sync_render_preview())
			break;

		studio::App::iteration(false);

		if(get_cancel_status())
			return;
	}
	is_playing_=false;

	time_adjustment().set_value(endtime);
	time_adjustment().value_changed();
}

void
CanvasView::show_tables()
{
/*
	Smach::event_result x(process_event_key(EVENT_TABLES_SHOW));
	if(x==Smach::RESULT_OK || x==Smach::RESULT_ACCEPT)
	{
		Gtk::IconSize iconsize=Gtk::IconSize::from_name("synfig-small_icon");
		treetogglebutton->remove();
		treetogglebutton->add(*manage(new Gtk::Image(Gtk::StockID("gtk-go-down"),iconsize)));
		treetogglebutton->show_all();
		notebook->show();
	}
*/
}

void
CanvasView::hide_tables()
{
/*
	Smach::event_result x(process_event_key(EVENT_TABLES_HIDE));
	if(x==Smach::RESULT_OK || x==Smach::RESULT_ACCEPT)
	{
		Gtk::IconSize iconsize=Gtk::IconSize::from_name("synfig-small_icon");
		treetogglebutton->remove();
		treetogglebutton->add(*manage(new Gtk::Image(Gtk::StockID("gtk-go-up"),iconsize)));
		treetogglebutton->show_all();
		notebook->hide();
	}
*/
}

bool
CanvasView::tables_are_visible()
{
//	return notebook->is_visible();
	return false;
}

void
CanvasView::toggle_tables()
{
//	if(tables_are_visible())
//		hide_tables();
//	else
//		show_tables();
}

void
CanvasView::show_timebar()
{
	timebar->show();
	current_time_widget->show();

	//keyframe_tab_child->show();
	if(layer_tree)
		layer_tree->set_show_timetrack(true);
	if(children_tree)
		children_tree->set_show_timetrack(true);
}

void
CanvasView::hide_timebar()
{
	timebar->hide();
	current_time_widget->hide();
	//keyframe_tab_child->hide();
	if(layer_tree)
		layer_tree->set_show_timetrack(false);
	if(children_tree)
		children_tree->set_show_timetrack(false);
}

void
CanvasView::set_sensitive_timebar(bool sensitive)
{
	timebar->set_sensitive(sensitive);
	current_time_widget->set_sensitive(sensitive);
	//keyframe_tab_child->set_sensitive(sensitive);
	if(layer_tree)
		layer_tree->set_sensitive(sensitive);
	if(children_tree)
		children_tree->set_sensitive(sensitive);
}


void
CanvasView::on_waypoint_clicked(synfigapp::ValueDesc value_desc,synfig::Waypoint waypoint,int button)
{
	waypoint_dialog.set_value_desc(value_desc);
	waypoint_dialog.set_waypoint(waypoint);

	switch(button)
	{
	case -1:
		waypoint_dialog.show();
		break;
	case 2:
		{
			Gtk::Menu* waypoint_menu(manage(new Gtk::Menu()));

			waypoint_menu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-jump-to"),
				sigc::bind(
					sigc::mem_fun(
						*canvas_interface(),
						&synfigapp::CanvasInterface::set_time
					),
					waypoint.get_time()
				)
			));

			waypoint_menu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("Edit Waypoint"),
				sigc::mem_fun(
					waypoint_dialog,
					&Gtk::Widget::show
				)
			));

			waypoint_menu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("synfig-duplicate"),
				sigc::bind(
					sigc::bind(
						sigc::mem_fun(
							*canvas_interface(),
							&synfigapp::CanvasInterface::waypoint_duplicate
						),
						waypoint
					),
					value_desc
				)
			));
			waypoint_menu->items().push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::StockID("gtk-delete"),
				sigc::bind(
					sigc::bind(
						sigc::mem_fun(
							*canvas_interface(),
							&synfigapp::CanvasInterface::waypoint_remove
						),
						waypoint
					),
					value_desc
				)
			));
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
	synfigapp::Action::ParamList param_list;
	param_list.add("canvas",get_canvas());
	param_list.add("canvas_interface",canvas_interface());
	param_list.add("value_node",waypoint_dialog.get_value_desc().get_value_node());
	param_list.add("waypoint",waypoint_dialog.get_waypoint());
//	param_list.add("time",canvas_interface()->get_time());

	get_instance()->process_action("waypoint_set_smart", param_list);
}

void
CanvasView::on_waypoint_delete()
{
	synfigapp::Action::ParamList param_list;
	param_list.add("canvas",get_canvas());
	param_list.add("canvas_interface",canvas_interface());
	param_list.add("value_node",waypoint_dialog.get_value_desc().get_value_node());
	param_list.add("waypoint",waypoint_dialog.get_waypoint());
//	param_list.add("time",canvas_interface()->get_time());

	get_instance()->process_action("waypoint_remove", param_list);
}

void
CanvasView::on_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data_, guint info, guint time)
{
	// We will make this true once we have a solid drop
	bool success(false);
	//synfig::info("Droped data of type \"%s\"",selection_data.get_data_type());
	//synfig::info("Droped data of target \"%s\"",gdk_atom_name(selection_data->target));
	//synfig::info("selection=\"%s\"",gdk_atom_name(selection_data->selection));

	if ((selection_data_.get_length() >= 0) && (selection_data_.get_format() == 8))
	{
		if(synfig::String(selection_data_.get_data_type())=="STRING")do
		{
			synfig::String selection_data((gchar *)(selection_data_.get_data()));

			Layer::Handle layer(synfig::Layer::create("Text"));
			if(!layer)
				break;
			if(!layer->set_param("text",ValueBase(selection_data)))
				break;

			synfigapp::Action::Handle 	action(synfigapp::Action::create("layer_add"));

			assert(action);
			if(!action)
				break;

			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",canvas_interface());
			action->set_param("new",layer);

			if(!get_instance()->perform_action(action))
				break;

			// Ok, we have successfuly imported at least one item.
			success=true;
		} while(0); // END of "STRING"

		if(synfig::String(selection_data_.get_data_type())=="text/plain")
		{
			synfig::String selection_data((gchar *)(selection_data_.get_data()));

			// For some reason, GTK hands us a list of URL's seperated
			// by not only Carrage-Returns, but also Line-Feeds.
			// Line-Feeds will mess us up. Remove all the line-feeds.
			while(selection_data.find_first_of('\r')!=synfig::String::npos)
				selection_data.erase(selection_data.begin()+selection_data.find_first_of('\r'));

			std::stringstream stream(selection_data);

			//synfigapp::PassiveGrouper group(canvas_interface()->get_instance(),_("Insert Image"));
			while(stream)
			{
				synfig::String filename,URI;
				getline(stream,filename);

				// If we don't have a filename, move on.
				if(filename.empty())
					continue;

				// Make sure this URL is of the "file://" type.
				URI=String(filename.begin(),filename.begin()+sizeof("file://")-1);
				if(URI!="file://")
				{
					synfig::warning("Unknown URI (%s) in \"%s\"",URI.c_str(),filename.c_str());
					continue;
				}

				// Strip the "file://" part from the filename
				filename=synfig::String(filename.begin()+sizeof("file://")-1,filename.end());

				String ext;
				try{ext=(String(filename.begin()+filename.find_last_of('.')+1,filename.end()));}catch(...){continue;}

				// If this is a SIF file, then we need to do things slightly differently
				if(ext=="sketch")
				{
					if(work_area->load_sketch(filename))
					{
						success=true;
						work_area->queue_draw();
					}
				}
				else
				{
					if(canvas_interface()->import(filename))
						success=true;
				}

				continue;
			}
		} // END of "text/plain"
	}
	else
		ui_interface_->error("Drop failed: bad selection data");

	// Finish the drag
	context->drag_finish(success, false, time);
}

void
CanvasView::on_keyframe_add_pressed()
{
	synfigapp::Action::Handle action(synfigapp::Action::create("keyframe_add"));

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

	synfigapp::Action::Handle action(synfigapp::Action::create("keyframe_duplicate"));

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

	synfigapp::Action::Handle action(synfigapp::Action::create("keyframe_remove"));

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
CanvasView::toggle_duck_mask(Duckmatic::Type type)
{
	bool is_currently_on(work_area->get_type_mask()&type);

	switch(type)
	{
	case Duck::TYPE_POSITION:
		if(duck_mask_position)
			duck_mask_position->set_active(!is_currently_on);
		break;

	case Duck::TYPE_VERTEX:
		if(duck_mask_vertex)
			duck_mask_vertex->set_active(!is_currently_on);
		break;

	case Duck::TYPE_TANGENT:
		if(duck_mask_tangent)
			duck_mask_tangent->set_active(!is_currently_on);
		break;

	case Duck::TYPE_RADIUS:
		if(duck_mask_radius)
			duck_mask_radius->set_active(!is_currently_on);
		break;

	case Duck::TYPE_WIDTH:
		if(duck_mask_width)
			duck_mask_width->set_active(!is_currently_on);
		break;

	case Duck::TYPE_ANGLE:
		if(duck_mask_angle)
			duck_mask_angle->set_active(!is_currently_on);
		break;

	default:
		synfig::warning("CanvasView::toggle_duck_mask():Unknown duck type!");
		break;
	}

	if(is_currently_on)
		work_area->set_type_mask(work_area->get_type_mask()-type);
	else
		work_area->set_type_mask(work_area->get_type_mask()|type);

	work_area->queue_draw();
}


void
CanvasView::image_import()
{
	String filename(dirname(get_canvas()->get_file_name()));
	if(App::dialog_open_file(_("Import Image"), filename))
		canvas_interface()->import(filename);
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

	synfigapp::InputDevice::Handle input_device;
	input_device=synfigapp::Main::select_input_device(device->name);
	App::toolbox->change_state(input_device->get_state());
	process_event_key(EVENT_INPUT_DEVICE_CHANGED);
}

void
CanvasView::on_preview_option()
{
	Dialog_PreviewOptions *po = dynamic_cast<Dialog_PreviewOptions *>(get_ext_widget("prevoptions"));

	Canvas::Handle	canv = get_canvas();

	if(canv)
	{
		RendDesc &r = canv->rend_desc();
		if(r.get_frame_rate())
		{
			float rate = 1/r.get_frame_rate();
			float beg = r.get_time_start() + r.get_frame_start()*rate;
			float end = r.get_time_start() + r.get_frame_end()*rate;

			if(!po)
			{
				po = new Dialog_PreviewOptions;
				po->set_zoom(work_area->get_zoom()/2);
				po->set_fps(r.get_frame_rate()/2);
				po->set_begintime(beg);
				po->set_begin_override(false);
				po->set_endtime(end);
				po->set_end_override(false);

				set_ext_widget("prevoptions",po);
			}
			/*po->set_zoom(work_area->get_zoom()/2);
			po->set_fps(r.get_frame_rate()/2);
			po->set_begintime(beg);
			po->set_begin_override(false);
			po->set_endtime(end);
			po->set_end_override(false);*/

			po->set_global_fps(r.get_frame_rate());
			po->signal_finish().connect(sigc::mem_fun(*this,&CanvasView::on_preview_create));
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
	prev->set_quality(work_area->get_quality());

	//render it out...
	prev->render();

	Dialog_Preview *pd = preview_dialog.get();
	assert(pd);

	pd->set_default_size(700,510);
	pd->set_preview(prev.get());
	pd->present();
}

void
CanvasView::on_audio_option()
{
	synfig::warning("Launching Audio Options");
	sound_dialog->set_global_fps(get_canvas()->rend_desc().get_frame_rate());
	sound_dialog->present();
}

void
CanvasView::on_audio_file_change(const std::string &f)
{
	//save in meta data - always even when not valid...
	canvas_interface()->set_meta_data("audiofile",f);
}

void
CanvasView::on_audio_offset_change(const Time &t)
{
	canvas_interface()->set_meta_data("audiooffset",t.get_string());
}

void
CanvasView::on_audio_file_notify()
{
	std::string file(get_canvas()->get_meta_data("audiofile"));
	if(!file.c_str()) return;

	if(!audio->load(file,dirname(get_canvas()->get_file_name())+string("/")))
	{
		if(file != "") synfig::warning("Could not load the file: %s", file.c_str());
		get_canvas()->erase_meta_data("audiofile");
		disp_audio->hide();
		disp_audio->set_profile(etl::handle<AudioProfile>());
	}else
	{
		//save in canvasview
		synfig::warning("Getting the profile of the music stuff");

		//profile specific stuff for the preview widget
		//similar for other attachments
		Dialog_Preview *pd = preview_dialog.get();
		pd->get_widget().set_audio(audio);

		handle<AudioProfile>	prof = audio->get_profile();

		if(!prof)
		{
			synfig::warning("Agh, I couldn't build the profile captain!");
		}
		pd->get_widget().set_audioprofile(prof);

		disp_audio->set_profile(audio->get_profile());
		disp_audio->show();

		synfig::warning("successfully set the profiles and stuff");
	}
	disp_audio->queue_draw();
}

void
CanvasView::on_audio_offset_notify()
{
	Time t(get_canvas()->get_meta_data("audiooffset"),get_canvas()->rend_desc().get_frame_rate());
	audio->set_offset(t);
	sound_dialog->set_offset(t);
	disp_audio->queue_draw();

	synfig::info("CanvasView::on_audio_offset_notify(): offset time set to %s",t.get_string(get_canvas()->rend_desc().get_frame_rate()).c_str());
}

void
CanvasView::play_audio(float t)
{
	if(audio.get())
	{
		synfig::info("Playing audio at %f s",t);
		audio->play(t);
	}
}

void
CanvasView::stop_audio()
{
	if(audio.get())
	{
		audio->stop();
	}
}

bool
CanvasView::on_audio_scrub()
{
	disp_audio->draw();
	return true;
}



Glib::RefPtr<Glib::ObjectBase>
CanvasView::get_ref_obj(const synfig::String& x)
{
	return ref_obj_book_[x];
}

Glib::RefPtr<const Glib::ObjectBase>
CanvasView::get_ref_obj(const synfig::String& x)const
{
	return ref_obj_book_.find(x)->second;
}

void
CanvasView::set_ref_obj(const synfig::String& x, Glib::RefPtr<Glib::ObjectBase> y)
{
	ref_obj_book_[x]=y;
}

Glib::RefPtr<Gtk::TreeModel>
CanvasView::get_tree_model(const synfig::String& x)
{
	return Glib::RefPtr<Gtk::TreeModel>::cast_dynamic(ref_obj_book_["_tree_model_"+x]);
}

Glib::RefPtr<const Gtk::TreeModel>
CanvasView::get_tree_model(const synfig::String& x)const
{
	return Glib::RefPtr<Gtk::TreeModel>::cast_dynamic(ref_obj_book_.find("_tree_model_"+x)->second);
}

void
CanvasView::set_tree_model(const synfig::String& x, Glib::RefPtr<Gtk::TreeModel> y)
{
	ref_obj_book_["_tree_model_"+x]=Glib::RefPtr<Glib::ObjectBase>::cast_static(y);
}

Gtk::Widget*
CanvasView::get_ext_widget(const synfig::String& x)
{
	return ext_widget_book_[x];
}

void
CanvasView::set_ext_widget(const synfig::String& x, Gtk::Widget* y)
{
	ext_widget_book_[x]=y;
	if(x=="layers_cmp")
	{
		layer_tree=dynamic_cast<LayerTree*>(y);

		layer_tree->get_selection()->signal_changed().connect(SLOT_EVENT(EVENT_LAYER_SELECTION_CHANGED));
		layer_tree->get_selection()->signal_changed().connect(SLOT_EVENT(EVENT_REFRESH_DUCKS));
		layer_tree->signal_layer_user_click().connect(sigc::mem_fun(*this, &studio::CanvasView::on_layer_user_click));
		layer_tree->signal_param_user_click().connect(sigc::mem_fun(*this, &studio::CanvasView::on_children_user_click));
		layer_tree->signal_waypoint_clicked().connect(sigc::mem_fun(*this, &studio::CanvasView::on_waypoint_clicked));
	}
	if(x=="children")
	{
		children_tree=dynamic_cast<ChildrenTree*>(y);
		if(children_tree)children_tree->signal_user_click().connect(sigc::mem_fun(*this, &studio::CanvasView::on_children_user_click));
		if(children_tree)children_tree->signal_waypoint_clicked().connect(sigc::mem_fun(*this, &studio::CanvasView::on_waypoint_clicked));
		if(children_tree)children_tree->get_selection()->signal_changed().connect(SLOT_EVENT(EVENT_REFRESH_DUCKS));
	}
	if(x=="keyframes")
		keyframe_tree=dynamic_cast<KeyframeTree*>(y);
}

static bool _close_instance(etl::handle<Instance> instance)
{
	etl::handle<Instance> argh(instance);
	instance->safe_close();
	synfig::info("closed");
	return false;
}

bool
CanvasView::on_delete_event(GdkEventAny* event)
{
	if(get_instance()->get_visible_canvases()==1)
	{
		// Schedule a close to occur in a few moments
		Glib::signal_timeout().connect(
			sigc::bind(
				sigc::ptr_fun(_close_instance),
				(etl::handle<Instance>)get_instance()
			)
			,250
		);
	}
	if(event)
		return Gtk::Window::on_delete_event(event);

	return true;
}
