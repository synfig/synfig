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

#include <cassert>

#include <gtkmm/scrolledwindow.h>

#include <synfig/general.h>

#include <gui/localization.h>
#include <app.h>
#include <instance.h>
#include <canvasview.h>
#include <workarea.h>
#include <trees/layerparamtreestore.h>
#include <widgets/widget_curves.h>

#include "dock_curves.h"

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
	Dock_CanvasSpecific("curves",_("Graphs"),Gtk::StockID("synfig-curves")),
	table_(),
	last_widget_curves_()
{ }

Dock_Curves::~Dock_Curves()
{
	if (table_) delete table_;
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
	//! Curves is registered thru CanvasView::set_ext_widget
	//! and will be deleted during CanvasView::~CanvasView()
	//! \see CanvasView::set_ext_widget
	//! \see CanvasView::~CanvasView
	Widget_Curves* curves(new Widget_Curves());
	curves->set_time_model(canvas_view->time_model());

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
	tree_layer->signal_param_tree_header_height_changed().connect(
		sigc::mem_fun(*this, &studio::Dock_Curves::on_update_header_height) );

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
		delete table_;
		table_ = 0;

		last_widget_curves_ = 0;

		hscrollbar_.unset_adjustment();
		vscrollbar_.unset_adjustment();

		widget_timeslider_.set_canvas_view( CanvasView::Handle() );

		widget_kf_list_.set_time_model( etl::handle<TimeModel>() );
		widget_kf_list_.set_canvas_interface( etl::loose_handle<synfigapp::CanvasInterface>() );
	}


	if(canvas_view)
	{
		last_widget_curves_=dynamic_cast<Widget_Curves*>( canvas_view->get_ext_widget(get_name()) );

		vscrollbar_.set_adjustment(last_widget_curves_->get_range_adjustment());
		hscrollbar_.set_adjustment(canvas_view->time_model()->scroll_time_adjustment());

		widget_timeslider_.set_canvas_view(canvas_view);

		widget_kf_list_.set_time_model(canvas_view->time_model());
		widget_kf_list_.set_canvas_interface(canvas_view->canvas_interface());

		table_=new Gtk::Table(3, 2);
		table_->attach(widget_kf_list_,      0, 1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(widget_timeslider_,   0, 1, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(*last_widget_curves_, 0, 1, 2, 3, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND);
		table_->attach(hscrollbar_,          0, 1, 3, 4, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(vscrollbar_,          1, 2, 0, 3, Gtk::FILL|Gtk::SHRINK, Gtk::FILL|Gtk::EXPAND);
		table_->show_all();
		add(*table_);
	}
}

void
Dock_Curves::on_update_header_height(int height)
{
	int w = 0, h = 0;
	widget_kf_list_.get_size_request(w, h);
	int ts_height = std::max(1, height - h);

	widget_timeslider_.get_size_request(w, h);
	if (h != ts_height)
		widget_timeslider_.set_size_request(-1, ts_height);
}
