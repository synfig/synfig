/* === S Y N F I G ========================================================= */
/*!	\file dock_curves.cpp
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

#include "docks/dock_curves.h"
#include "app.h"

#include <gtkmm/scrolledwindow.h>
#include <cassert>
#include "instance.h"
#include <sigc++/signal.h>
#include <sigc++/hide.h>
#include <sigc++/slot.h>
#include "canvasview.h"
#include "trees/layerparamtreestore.h"
#include "workarea.h"
#include "widgets/widget_curves.h"
#include <gtkmm/table.h>
#include <gtkmm/scrollbar.h>
#include "widgets/widget_timeslider.h"

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dock_Curves::Dock_Curves():
	Dock_CanvasSpecific("curves",_("Graphs"),Gtk::StockID("synfig-curves"))
{
	last_widget_curves_=0;
	table_=0;

	hscrollbar_=new Gtk::HScrollbar();
	vscrollbar_=new Gtk::VScrollbar();
	widget_timeslider_= new Widget_Timeslider();
}

Dock_Curves::~Dock_Curves()
{
	if(table_)delete table_;
	delete hscrollbar_;
	delete vscrollbar_;
	delete widget_timeslider_;
}

static void
_curve_selection_changed(Gtk::TreeView* param_tree_view,Widget_Curves* curves)
{
	LayerParamTreeStore::Model model;
	Gtk::TreeIter iter;
	if(!param_tree_view->get_selection()->count_selected_rows())
	{
		curves->clear();
		curves->refresh();
		return;
	}

	std::list<synfigapp::ValueDesc> value_descs;

	iter=param_tree_view->get_selection()->get_selected();
	value_descs.push_back((*iter)[model.value_desc]);
	curves->set_value_descs(value_descs);
}

void
Dock_Curves::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	//! Curves is registred thrue CanvasView::set_ext_widget
	//! and will be deleted during CanvasView::~CanvasView()
	//! \see CanvasView::set_ext_widget
	//! \see CanvasView::~CanvasView
	Widget_Curves* curves(new Widget_Curves());
	curves->set_time_adjustment(canvas_view->time_adjustment());

	Gtk::TreeView* param_tree_view(
		static_cast<Gtk::TreeView*>(canvas_view->get_ext_widget("params"))
	);

	param_tree_view->get_selection()->signal_changed().connect(
		sigc::bind(
			sigc::bind(
				sigc::ptr_fun(
					_curve_selection_changed
				),curves
			),param_tree_view
		)
	);

	studio::LayerTree* tree_layer(dynamic_cast<studio::LayerTree*>(canvas_view->get_ext_widget("layers_cmp")));
	tree_layer->signal_param_tree_header_height_changed().connect(sigc::mem_fun(*this, &studio::Dock_Curves::on_update_header_height));

	canvas_view->set_ext_widget(get_name(),curves);
}

void
Dock_Curves::refresh_selected_param()
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
Dock_Curves::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
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
		last_widget_curves_=dynamic_cast<Widget_Curves*>(
			canvas_view->get_ext_widget(get_name())
		);

		vscrollbar_->set_adjustment(last_widget_curves_->get_range_adjustment());
		hscrollbar_->set_adjustment(canvas_view->time_window_adjustment());
		widget_timeslider_->set_time_adjustment(canvas_view->time_adjustment());
		widget_timeslider_->set_bounds_adjustment(canvas_view->time_window_adjustment());
		widget_timeslider_->set_global_fps(canvas_view->get_canvas()->rend_desc().get_frame_rate());

		table_=new Gtk::Table(2,2);
		table_->attach(*widget_timeslider_, 0, 1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(*last_widget_curves_, 0, 1, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND);
		table_->attach(*hscrollbar_, 0, 1, 2, 3, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(*vscrollbar_, 1, 2, 0, 2, Gtk::FILL|Gtk::SHRINK, Gtk::FILL|Gtk::EXPAND);
		add(*table_);

		//add(*last_widget_curves_);
		last_widget_curves_->show();
		table_->show_all();
		show_all();
	}
	else
	{
		//clear_previous();
	}
}

void
Dock_Curves::on_update_header_height( int header_height)
{
	// FIXME very bad hack (timetrack dock also contains this)
	//! Adapt the border size "according" to different windows manager rendering
#ifdef WIN32
	header_height-=2;
#elif defined(__APPLE__)
	header_height+=6;
#else
// *nux and others
	header_height+=2;
#endif

	widget_timeslider_->set_size_request(-1,header_height+1);
}
