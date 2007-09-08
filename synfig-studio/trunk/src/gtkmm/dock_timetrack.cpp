/* === S Y N F I G ========================================================= */
/*!	\file dock_timetrack.cpp
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

#include "dock_timetrack.h"
#include "app.h"

#include <gtkmm/scrolledwindow.h>
#include <cassert>
#include "instance.h"
#include <sigc++/signal.h>
#include <sigc++/hide.h>
#include <sigc++/slot.h>
#include "canvasview.h"
#include "layerparamtreestore.h"
#include "workarea.h"
#include "widget_timeslider.h"
#include "layerparamtreestore.h"
#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === C L A S S E S ======================================================= */

class TimeTrackView : public Gtk::TreeView
{
	CellRenderer_TimeTrack *cellrenderer_time_track;

	Glib::RefPtr<LayerParamTreeStore> param_tree_store_;

	Gtk::TreeView *mimic_tree_view;
public:

	sigc::signal<void,synfigapp::ValueDesc,synfig::Waypoint,int> signal_waypoint_clicked;

	LayerParamTreeStore::Model model;

	void set_canvas_view(handle<CanvasView> canvas_view)
	{
		cellrenderer_time_track->set_adjustment(canvas_view->time_adjustment());
	}

	TimeTrackView()
	{
		int label_index(append_column_editable(_("Name"),model.label));
		Gtk::TreeView::Column* label_column = get_column(label_index-1);

		{	// --- T I M E   T R A C K --------------------------------------------
			Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Time Track")) );

			// Set up the value-node cell-renderer
			cellrenderer_time_track=LayerParamTreeStore::add_cell_renderer_value_node(column);
			cellrenderer_time_track->property_mode()=Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
			cellrenderer_time_track->signal_waypoint_clicked().connect(sigc::mem_fun(*this, &TimeTrackView::on_waypoint_clicked) );
			cellrenderer_time_track->signal_waypoint_changed().connect(sigc::mem_fun(*this, &TimeTrackView::on_waypoint_changed) );
			column->add_attribute(cellrenderer_time_track->property_value_desc(), model.value_desc);
			column->add_attribute(cellrenderer_time_track->property_canvas(), model.canvas);
			//column->add_attribute(cellrenderer_time_track->property_visible(), model.is_value_node);

			//column->pack_start(*cellrenderer_time_track);

			// Finish setting up the column
			column->set_reorderable();
			column->set_resizable();
			column->set_min_width(200);


			append_column(*column);
		}
		set_rules_hint();

		set_expander_column(*label_column);
		label_column->set_visible(false);
		set_headers_visible(false);
		set_size_request(-1,64);
	}

	bool
	on_event(GdkEvent *event)
	{
		switch(event->type)
		{
		case GDK_SCROLL:
			if(mimic_tree_view)
			{
				if(event->scroll.direction==GDK_SCROLL_DOWN)
				{
					mimic_tree_view->get_vadjustment()->set_value(
						std::min(
							mimic_tree_view->get_vadjustment()->get_value()+
							mimic_tree_view->get_vadjustment()->get_step_increment(),
							mimic_tree_view->get_vadjustment()->get_upper()-
							mimic_tree_view->get_vadjustment()->get_page_size()
						)
					);
					mimic_tree_view->get_vadjustment()->value_changed();
				}
				else if(event->scroll.direction==GDK_SCROLL_UP)
				{
					mimic_tree_view->get_vadjustment()->set_value(
						std::max(
							mimic_tree_view->get_vadjustment()->get_value()-
							mimic_tree_view->get_vadjustment()->get_step_increment(),
							mimic_tree_view->get_vadjustment()->get_lower()
						)
					);
					mimic_tree_view->get_vadjustment()->value_changed();
				}
			}
			break;
		case GDK_BUTTON_PRESS:
			{
				Gtk::TreeModel::Path path;
				Gtk::TreeViewColumn *column;
				int cell_x, cell_y;
				if(!get_path_at_pos(
					int(event->button.x),int(event->button.y),	// x, y
					path, // TreeModel::Path&
					column, //TreeViewColumn*&
					cell_x,cell_y //int&cell_x,int&cell_y
					)
				) break;
				const Gtk::TreeRow row = *(get_model()->get_iter(path));

				if(column && column->get_first_cell_renderer()==cellrenderer_time_track)
				{
					Gdk::Rectangle rect;
					get_cell_area(path,*column,rect);
					cellrenderer_time_track->property_value_desc()=row[model.value_desc];
					cellrenderer_time_track->property_canvas()=row[model.canvas];
					cellrenderer_time_track->activate(event,*this,path.to_string(),rect,rect,Gtk::CellRendererState());
					queue_draw_area(rect.get_x(),rect.get_y(),rect.get_width(),rect.get_height());
					return true;
					//return signal_param_user_click()(event->button.button,row,COLUMNID_TIME_TRACK);
				}
/*				else
				{
					if(event->button.button==3)
					{
						LayerList layer_list(get_selected_layers());
						if(layer_list.size()<=1)
						{
							synfigapp::ValueDesc value_desc(row[model.value_desc]);
							Gtk::Menu* menu(manage(new Gtk::Menu()));
							App::get_instance(param_tree_store_->canvas_interface()->get_canvas())->make_param_menu(menu,param_tree_store_->canvas_interface()->get_canvas(),value_desc,0.5f);
							menu->popup(event->button.button,gtk_get_current_event_time());
							return true;
						}
						Gtk::Menu* menu(manage(new Gtk::Menu()));
						std::list<synfigapp::ValueDesc> value_desc_list;
						ParamDesc param_desc(row[model.param_desc]);
						for(;!layer_list.empty();layer_list.pop_back())
							value_desc_list.push_back(synfigapp::ValueDesc(layer_list.back(),param_desc.get_name()));
						App::get_instance(param_tree_store_->canvas_interface()->get_canvas())->make_param_menu(menu,param_tree_store_->canvas_interface()->get_canvas(),value_desc_list);
						menu->popup(event->button.button,gtk_get_current_event_time());
						return true;
					}
					else
					{
						if(column->get_first_cell_renderer()==cellrenderer_value)
							return signal_param_user_click()(event->button.button,row,COLUMNID_VALUE);
						else
							return signal_param_user_click()(event->button.button,row,COLUMNID_NAME);
					}
				}
				*/
			}
			break;

		case GDK_MOTION_NOTIFY:
			{
				Gtk::TreeModel::Path path;
				Gtk::TreeViewColumn *column;
				int cell_x, cell_y;
				if(!get_path_at_pos(
					(int)event->motion.x,(int)event->motion.y,	// x, y
					path, // TreeModel::Path&
					column, //TreeViewColumn*&
					cell_x,cell_y //int&cell_x,int&cell_y
					)
				) break;

				if(!get_model()->get_iter(path))
					break;

				Gtk::TreeRow row = *(get_model()->get_iter(path));

				if((event->motion.state&GDK_BUTTON1_MASK ||event->motion.state&GDK_BUTTON3_MASK) && column && cellrenderer_time_track==column->get_first_cell_renderer())
				{
					Gdk::Rectangle rect;
					get_cell_area(path,*column,rect);
					cellrenderer_time_track->property_value_desc()=row[model.value_desc];
					cellrenderer_time_track->property_canvas()=row[model.canvas];
					cellrenderer_time_track->activate(event,*this,path.to_string(),rect,rect,Gtk::CellRendererState());
					queue_draw();
					//queue_draw_area(rect.get_x(),rect.get_y(),rect.get_width(),rect.get_height());
					return true;
				}
/*				else
				if(last_tooltip_path.get_depth()<=0 || path!=last_tooltip_path)
				{
					tooltips_.unset_tip(*this);
					Glib::ustring tooltips_string(row[layer_model.tooltip]);
					last_tooltip_path=path;
					if(!tooltips_string.empty())
					{
						tooltips_.set_tip(*this,tooltips_string);
						tooltips_.force_window();
					}
				}
*/
				return true;
			}
			break;
		case GDK_BUTTON_RELEASE:
			{
				Gtk::TreeModel::Path path;
				Gtk::TreeViewColumn *column;
				int cell_x, cell_y;
				if(!get_path_at_pos(
					(int)event->button.x,(int)event->button.y,	// x, y
					path, // TreeModel::Path&
					column, //TreeViewColumn*&
					cell_x,cell_y //int&cell_x,int&cell_y
					)
				) break;

				if(!get_model()->get_iter(path))
					break;

				Gtk::TreeRow row = *(get_model()->get_iter(path));

				if(column && cellrenderer_time_track==column->get_first_cell_renderer())
				{
					Gdk::Rectangle rect;
					get_cell_area(path,*column,rect);
					cellrenderer_time_track->property_value_desc()=row[model.value_desc];
					cellrenderer_time_track->property_canvas()=row[model.canvas];
					cellrenderer_time_track->activate(event,*this,path.to_string(),rect,rect,Gtk::CellRendererState());
					queue_draw();
					queue_draw_area(rect.get_x(),rect.get_y(),rect.get_width(),rect.get_height());
					return true;
				}
			}
			break;
		default:
			break;
		}
		mimic_resync();
		return Gtk::TreeView::on_event(event);
	}

	void
	queue_draw_msg()
	{
		synfig::info("*************QUEUE_DRAW***************** (time track view)");
		Widget::queue_draw();
	}
	void set_model(Glib::RefPtr<LayerParamTreeStore> store)
	{
		Gtk::TreeView::set_model(store);
		param_tree_store_=store;
		cellrenderer_time_track->set_canvas_interface(param_tree_store_->canvas_interface());
		store->signal_changed().connect(sigc::mem_fun(*this, &TimeTrackView::queue_draw));
	}

	void
	on_waypoint_changed( synfig::Waypoint waypoint , synfig::ValueNode::Handle value_node)
	{
		synfigapp::Action::ParamList param_list;
		param_list.add("canvas",param_tree_store_->canvas_interface()->get_canvas());
		param_list.add("canvas_interface",param_tree_store_->canvas_interface());
		param_list.add("value_node",value_node);
		param_list.add("waypoint",waypoint);
	//	param_list.add("time",canvas_interface()->get_time());

		etl::handle<studio::Instance>::cast_static(param_tree_store_->canvas_interface()->get_instance())->process_action("waypoint_set_smart", param_list);
	}

	void mimic(Gtk::TreeView *param_tree_view)
	{
		mimic_tree_view=param_tree_view;
		param_tree_view->signal_row_expanded().connect(
			sigc::hide<0>(
			sigc::hide_return(
				sigc::bind<-1>(
					sigc::mem_fun(
						*this,
						&Gtk::TreeView::expand_row
					),
					false
				)
			))
		);
		param_tree_view->signal_row_collapsed().connect(
			sigc::hide<0>(
			sigc::hide_return(
					sigc::mem_fun(
						*this,
						&Gtk::TreeView::collapse_row
					)
			))
		);
		mimic_resync();
	}

	void mimic_resync()
	{

		if(mimic_tree_view)
		{
			Gtk::Adjustment &adjustment(*mimic_tree_view->get_vadjustment());
			set_vadjustment(adjustment);

			if(adjustment.get_page_size()>get_height())
				adjustment.set_page_size(get_height());

			int row_height = 0;
			if(getenv("SYNFIG_TIMETRACK_ROW_HEIGHT"))
				row_height = atoi(getenv("SYNFIG_TIMETRACK_ROW_HEIGHT"));
			if (row_height < 3)
				row_height = 18;
									  
			cellrenderer_time_track->set_fixed_size(-1,row_height);
		}
	}

	void
	on_waypoint_clicked(const Glib::ustring &/*path_string*/, synfig::Waypoint waypoint,int button)
	{
/*
		Gtk::TreePath path(path_string);

		const Gtk::TreeRow row = *(get_model()->get_iter(path));
		if(!row)
			return;
*/

		ValueNode::Handle value_node(waypoint.get_parent_value_node());
		assert(value_node);

		Gtk::TreeRow row;
		if(!param_tree_store_->find_first_value_node(value_node, row))
		{
			synfig::error(__FILE__":%d: Unable to find the valuenode",__LINE__);
			return;
		}

		if(!row)
			return;

		synfigapp::ValueDesc value_desc(static_cast<synfigapp::ValueDesc>(row[model.value_desc]));

		signal_waypoint_clicked(value_desc,waypoint,button);
	}
};

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dock_Timetrack::Dock_Timetrack():
	Dock_CanvasSpecific("timetrack",_("Timetrack"),Gtk::StockID("synfig-timetrack"))
{
	table_=0;
	widget_timeslider_= new Widget_Timeslider();

	int header_height = 0;
	if(getenv("SYNFIG_TIMETRACK_HEADER_HEIGHT"))
		header_height = atoi(getenv("SYNFIG_TIMETRACK_HEADER_HEIGHT"));
	if (header_height < 3)
		header_height = 22;

	widget_timeslider_->set_size_request(-1,header_height);
	hscrollbar_=new Gtk::HScrollbar();
	vscrollbar_=new Gtk::VScrollbar();
}

Dock_Timetrack::~Dock_Timetrack()
{
	if(table_)delete table_;
	delete hscrollbar_;
	delete vscrollbar_;
	delete widget_timeslider_;
}

void
Dock_Timetrack::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	LayerParamTreeStore::Model model;

	Glib::RefPtr<LayerParamTreeStore> tree_store(
		Glib::RefPtr<LayerParamTreeStore>::cast_dynamic(
			canvas_view->get_tree_model("params")
		)
	);

	TimeTrackView* tree_view(new TimeTrackView());
	tree_view->set_canvas_view(canvas_view);
	tree_view->set_model(tree_store);
	Gtk::TreeView* param_tree_view(dynamic_cast<Gtk::TreeView*>(canvas_view->get_ext_widget("params")));
	tree_view->mimic(param_tree_view);

	tree_view->signal_waypoint_clicked.connect(sigc::mem_fun(*canvas_view, &studio::CanvasView::on_waypoint_clicked));


	canvas_view->time_adjustment().signal_value_changed().connect(sigc::mem_fun(*tree_view,&Gtk::TreeView::queue_draw));
	canvas_view->time_adjustment().signal_changed().connect(sigc::mem_fun(*tree_view,&Gtk::TreeView::queue_draw));

	canvas_view->set_ext_widget(get_name(),tree_view);
}

void
Dock_Timetrack::refresh_selected_param()
{
/*	Gtk::TreeView* tree_view(
		static_cast<Gtk::TreeView*>(get_canvas_view()->get_ext_widget(get_name()))
	);
	Gtk::TreeModel::iterator iter(tree_view->get_selection()->get_selected());

	if(iter)
	{
		LayerParamTreeStore::Model model;
		get_canvas_view()->work_area->set_selected_value_node(
			(synfig::ValueNode::Handle)(*iter)[model.value_node]
		);
	}
	else
	{
		get_canvas_view()->work_area->set_selected_value_node(0);
	}
*/
}

void
Dock_Timetrack::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	if(table_)
	{
		table_->hide();
		delete table_;
		hscrollbar_->unset_adjustment();
		vscrollbar_->unset_adjustment();
		//widget_timeslider_->unset_adjustment();
		table_=0;
	}


	if(canvas_view)
	{
		TimeTrackView* tree_view(dynamic_cast<TimeTrackView*>(canvas_view->get_ext_widget(get_name())));
	Gtk::TreeView* param_tree_view(dynamic_cast<Gtk::TreeView*>(canvas_view->get_ext_widget("params")));
	tree_view->set_vadjustment(*param_tree_view->get_vadjustment());

		assert(tree_view);

		widget_timeslider_->set_time_adjustment(&canvas_view->time_adjustment());
		widget_timeslider_->set_bounds_adjustment(&canvas_view->time_window_adjustment());
		widget_timeslider_->set_global_fps(canvas_view->get_canvas()->rend_desc().get_frame_rate());

		vscrollbar_->set_adjustment(*tree_view->get_vadjustment());
		hscrollbar_->set_adjustment(canvas_view->time_window_adjustment());
		table_=new Gtk::Table(2,2);
		table_->attach(*widget_timeslider_, 0, 1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(*tree_view, 0, 1, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND);
		table_->attach(*hscrollbar_, 0, 1, 2, 3, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(*vscrollbar_, 1, 2, 0, 2, Gtk::FILL|Gtk::SHRINK, Gtk::FILL|Gtk::EXPAND);
		add(*table_);

		//add(*last_widget_curves_);
		table_->show_all();
		show_all();
	}
	else
	{
		//clear_previous();
	}
}
