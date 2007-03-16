/* === S Y N F I G ========================================================= */
/*!	\file layertree.cpp
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

#include "layertree.h"
#include "layerparamtreestore.h"
#include "cellrenderer_value.h"
#include "cellrenderer_timetrack.h"
#include <synfigapp/action.h>
#include <synfigapp/instance.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/paned.h>
#include "app.h"
#include "instance.h"
#include <gtkmm/treemodelsort.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef SMALL_BUTTON
#define SMALL_BUTTON(button,stockid,tooltip)	\
	button = manage(new class Gtk::Button());	\
	icon=manage(new Gtk::Image(Gtk::StockID(stockid),iconsize));	\
	button->add(*icon);	\
	tooltips_.set_tip(*button,tooltip);	\
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
	tooltips_.set_tip(*button,tooltip);	\
	icon->set_padding(0,0);\
	icon->show();	\
	/*button->set_relief(Gtk::RELIEF_NONE);*/ \
	button->show()
#endif

#define NEW_SMALL_BUTTON(x,y,z)	Gtk::Button *SMALL_BUTTON(x,y,z)

#define NOT_IMPLEMENTED_SLOT sigc::mem_fun(*reinterpret_cast<studio::CanvasViewUIInterface*>(get_ui_interface().get()),&studio::CanvasViewUIInterface::not_implemented)

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

LayerTree::LayerTree():
	layer_amount_adjustment_(1,0,1,0.01,0.01,0)
{
	param_tree_view_=new Gtk::TreeView;
	layer_tree_view_=new Gtk::TreeView;

	//Gtk::HPaned* hpaned(manage(new Gtk::HPaned()));
	//hpaned->show();
	//attach(*hpaned, 0, 3, 0, 1, Gtk::EXPAND|Gtk::FILL,Gtk::EXPAND|Gtk::FILL, 0, 0);
	//attach(*create_layer_tree(), 0, 3, 0, 1, Gtk::EXPAND|Gtk::FILL,Gtk::EXPAND|Gtk::FILL, 0, 0);

	create_layer_tree();
	create_param_tree();

	//hpaned->pack1(*create_layer_tree(),false,false);
	//hpaned->pack2(*create_param_tree(),true,false);
	//hpaned->set_position(200);
	hbox=manage(new Gtk::HBox());

	attach(*hbox, 0, 1, 1, 2, Gtk::FILL|Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(blend_method_widget, 2, 3, 1, 2,Gtk::SHRINK, Gtk::SHRINK, 0, 0);

	layer_amount_hscale=manage(new Gtk::HScale(layer_amount_adjustment_));
	layer_amount_hscale->set_digits(2);
	layer_amount_hscale->set_value_pos(Gtk::POS_LEFT);
	layer_amount_hscale->set_sensitive(false);
	layer_amount_hscale->set_update_policy( Gtk::UPDATE_DISCONTINUOUS);
	attach(*layer_amount_hscale, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK, 1, 1);
	layer_amount_adjustment_.signal_value_changed().connect(sigc::mem_fun(*this, &studio::LayerTree::on_amount_value_changed));




	Gtk::Image *icon;
	//Gtk::IconSize iconsize(Gtk::IconSize::from_name("synfig-small_icon"));
	Gtk::IconSize iconsize(Gtk::ICON_SIZE_SMALL_TOOLBAR);

	SMALL_BUTTON(button_raise,"gtk-go-up","Raise");
	SMALL_BUTTON(button_lower,"gtk-go-down","Lower");
	SMALL_BUTTON(button_duplicate,"synfig-duplicate","Duplicate");
	SMALL_BUTTON(button_delete,"gtk-delete","Delete");

	hbox->pack_start(*button_raise,Gtk::PACK_SHRINK);
	hbox->pack_start(*button_lower,Gtk::PACK_SHRINK);
	hbox->pack_start(*button_duplicate,Gtk::PACK_SHRINK);
	hbox->pack_start(*button_delete,Gtk::PACK_SHRINK);

	button_raise->signal_clicked().connect(sigc::mem_fun(*this, &studio::LayerTree::on_raise_pressed));
	button_lower->signal_clicked().connect(sigc::mem_fun(*this, &studio::LayerTree::on_lower_pressed));
	button_duplicate->signal_clicked().connect(sigc::mem_fun(*this, &studio::LayerTree::on_duplicate_pressed));
	button_delete->signal_clicked().connect(sigc::mem_fun(*this, &studio::LayerTree::on_delete_pressed));

	button_raise->set_sensitive(false);
	button_lower->set_sensitive(false);
	button_duplicate->set_sensitive(false);
	button_delete->set_sensitive(false);




	get_selection()->signal_changed().connect(sigc::mem_fun(*this, &studio::LayerTree::on_selection_changed));


	get_layer_tree_view().set_reorderable(true);
	get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	//get_param_tree_view().get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	get_layer_tree_view().show();
	param_tree_view().show();


	hbox->show();
	layer_amount_hscale->show();
	blend_method_widget.show();

	tooltips_.enable();
	disable_amount_changed_signal=false;





	blend_method_widget.set_param_desc(ParamDesc(Color::BlendMethod(),"blend_method"));

	blend_method_widget.set_value((int)Color::BLEND_COMPOSITE);
	blend_method_widget.set_size_request(150,-1);
	blend_method_widget.set_sensitive(false);
	blend_method_widget.signal_activate().connect(sigc::mem_fun(*this, &studio::LayerTree::on_blend_method_changed));
}


LayerTree::~LayerTree()
{
	synfig::info("LayerTree::~LayerTree(): Deleted");
}

Gtk::Widget*
LayerTree::create_layer_tree()
{
	const LayerTreeStore::Model model;


	{	// --- O N / O F F ----------------------------------------------------
		//int index;
		//index=get_layer_tree_view().append_column_editable(_(" "),layer_model.active);
		//Gtk::TreeView::Column* column = get_layer_tree_view().get_column(index-1);

		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_(" ")) );

		// Set up the icon cell-renderer
		Gtk::CellRendererToggle* cellrenderer = Gtk::manage( new Gtk::CellRendererToggle() );
		cellrenderer->signal_toggled().connect(sigc::mem_fun(*this, &studio::LayerTree::on_layer_toggle));

		column->pack_start(*cellrenderer,false);
		column->add_attribute(cellrenderer->property_active(), layer_model.active);
		get_layer_tree_view().append_column(*column);
	}

	{	// --- I C O N --------------------------------------------------------
		int index;
		index=get_layer_tree_view().append_column(_("Z"),layer_model.icon);
		Gtk::TreeView::Column* column = get_layer_tree_view().get_column(index-1);
		get_layer_tree_view().set_expander_column(*column);


		column->set_sort_column_id(layer_model.z_depth);
		//column->set_reorderable();
		//column->set_resizable();
		//column->set_clickable();

		//Gtk::CellRendererPixbuf* icon_cellrenderer = Gtk::manage( new Gtk::CellRendererPixbuf() );
		//column->pack_start(*icon_cellrenderer,false);
		//column->add_attribute(icon_cellrenderer->property_pixbuf(), layer_model.icon);
	}
	//get_layer_tree_view().append_column(_("Z"),layer_model.z_depth);
	{	// --- N A M E --------------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Layer")) );

		// Set up the icon cell-renderer
		Gtk::CellRendererText* cellrenderer = Gtk::manage( new Gtk::CellRendererText() );
		cellrenderer->signal_edited().connect(sigc::mem_fun(*this, &studio::LayerTree::on_layer_renamed));
		cellrenderer->property_editable()=true;

		column->pack_start(*cellrenderer,false);
		column->add_attribute(cellrenderer->property_text(), layer_model.label);
		get_layer_tree_view().append_column(*column);

		//		int index;
//		index=get_layer_tree_view().append_column_editable(_("Layer"),layer_model.label);
		//Gtk::TreeView::Column* column = get_layer_tree_view().get_column(index-1);

		//column->set_sort_column_id(layer_model.index);

		//get_layer_tree_view().set_expander_column(*column);
		//column->set_reorderable();
		//column->set_resizable();
		//column->set_clickable(false);

		//Gtk::CellRendererPixbuf* icon_cellrenderer = Gtk::manage( new Gtk::CellRendererPixbuf() );
		//column->pack_start(*icon_cellrenderer,false);
		//column->add_attribute(icon_cellrenderer->property_pixbuf(), layer_model.icon);
	}
	{	// --- Z D E P T H ----------------------------------------------------
		int index;
		index=get_layer_tree_view().append_column(_("Z"),layer_model.z_depth);
		column_z_depth=get_layer_tree_view().get_column(index-1);

		column_z_depth->set_reorderable();
		column_z_depth->set_resizable();
		column_z_depth->set_clickable();

		column_z_depth->set_sort_column_id(layer_model.z_depth);
	}

	get_layer_tree_view().set_enable_search(true);
	get_layer_tree_view().set_search_column(layer_model.label);
	get_layer_tree_view().set_search_equal_func(sigc::ptr_fun(&studio::LayerTreeStore::search_func));

	std::list<Gtk::TargetEntry> listTargets;
	listTargets.push_back( Gtk::TargetEntry("LAYER") );
	get_layer_tree_view().drag_dest_set(listTargets);


	// This makes things easier to read.
	get_layer_tree_view().set_rules_hint();

	// Make us more sensitive to several events
	//get_layer_tree_view().add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON1_MOTION_MASK | Gdk::BUTTON2_MOTION_MASK|Gdk::POINTER_MOTION_MASK);

	get_layer_tree_view().signal_event().connect(sigc::mem_fun(*this, &studio::LayerTree::on_layer_tree_event));
	get_layer_tree_view().show();



	Gtk::ScrolledWindow *scroll = manage(new class Gtk::ScrolledWindow());
	scroll->set_flags(Gtk::CAN_FOCUS);
	scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	//scroll->add(get_layer_tree_view());
	scroll->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	scroll->show();

	return scroll;
}

Gtk::Widget*
LayerTree::create_param_tree()
{
	Pango::AttrList attr_list;
	{
		Pango::AttrInt pango_size(Pango::Attribute::create_attr_size(Pango::SCALE*8));
		pango_size.set_start_index(0);
		pango_size.set_end_index(64);
		attr_list.change(pango_size);
	}

	Gtk::IconSize icon_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);

	{	// --- N A M E --------------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Param")) );

		// Set up the icon cell-renderer
		Gtk::CellRendererPixbuf* icon_cellrenderer = Gtk::manage( new Gtk::CellRendererPixbuf() );
		column->pack_start(*icon_cellrenderer,false);
		column->add_attribute(icon_cellrenderer->property_pixbuf(), param_model.icon);

		// Pack the label into the column
		//column->pack_start(layer_model.label,true);
		Gtk::CellRendererText* text_cellrenderer = Gtk::manage( new Gtk::CellRendererText() );
		column->pack_start(*text_cellrenderer,false);
		column->add_attribute(text_cellrenderer->property_text(), param_model.label);
		text_cellrenderer->property_attributes()=attr_list;

		text_cellrenderer->property_foreground()=Glib::ustring("#7f7f7f");
		column->add_attribute(text_cellrenderer->property_foreground_set(),param_model.is_inconsistent);

		// Pack the label into the column
		//column->pack_start(param_model.label,true);

		// Set up the value-node icon cell-renderer to be on the far right
		Gtk::CellRendererPixbuf* valuenode_icon_cellrenderer = Gtk::manage( new Gtk::CellRendererPixbuf() );
		column->pack_end(*valuenode_icon_cellrenderer,false);
		valuenode_icon_cellrenderer->property_pixbuf()=Gtk::Button().render_icon(Gtk::StockID("synfig-value_node"),icon_size);
		column->add_attribute(valuenode_icon_cellrenderer->property_visible(), param_model.is_shared);

		// Finish setting up the column
		column->set_reorderable();
		column->set_resizable();
		column->set_clickable();

		param_tree_view().append_column(*column);
	}
	{	// --- V A L U E  -----------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("ValueBase")) );

		// Set up the value cell-renderer
		cellrenderer_value=LayerParamTreeStore::add_cell_renderer_value(column);
		cellrenderer_value->signal_edited().connect(sigc::mem_fun(*this, &studio::LayerTree::on_edited_value));
		cellrenderer_value->property_value()=synfig::ValueBase();
		column->add_attribute(cellrenderer_value->property_param_desc(), param_model.param_desc);
		column->add_attribute(cellrenderer_value->property_inconsistant(),param_model.is_inconsistent);
		//cellrenderer_value->property_canvas()=canvas_interface->get_canvas(); // Is this line necessary?
		cellrenderer_value->property_attributes()=attr_list;

		// Finish setting up the column
		param_tree_view().append_column(*column);
		column->set_sizing(Gtk::TREE_VIEW_COLUMN_AUTOSIZE);
		column->set_clickable();
		column->set_min_width(120);
		column->set_reorderable();
		column->set_resizable();
	}
	/*{	// --- T I M E   T R A C K --------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Time Track")) );
		column_time_track=column;

		// Set up the value-node cell-renderer
		cellrenderer_time_track=LayerParamTreeStore::add_cell_renderer_value_node(column);
		cellrenderer_time_track->property_mode()=Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
		cellrenderer_time_track->signal_waypoint_clicked().connect(sigc::mem_fun(*this, &studio::LayerTree::on_waypoint_clicked) );
		cellrenderer_time_track->signal_waypoint_changed().connect(sigc::mem_fun(*this, &studio::LayerTree::on_waypoint_changed) );
		column->add_attribute(cellrenderer_time_track->property_value_desc(), param_model.value_desc);
		column->add_attribute(cellrenderer_time_track->property_canvas(), param_model.canvas);
		//column->add_attribute(cellrenderer_time_track->property_visible(), model.is_value_node);

		//column->pack_start(*cellrenderer_time_track);

		// Finish setting up the column
		column->set_reorderable();
		column->set_resizable();
		column->set_min_width(200);
		//param_tree_view().append_column(*column);
	}*/



	// This makes things easier to read.
	param_tree_view().set_rules_hint();

	// Make us more sensitive to several events
	param_tree_view().add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON1_MOTION_MASK | Gdk::BUTTON2_MOTION_MASK|Gdk::POINTER_MOTION_MASK);

	param_tree_view().signal_event().connect(sigc::mem_fun(*this, &studio::LayerTree::on_param_tree_event));
	param_tree_view().show();

	Gtk::ScrolledWindow *scroll = manage(new class Gtk::ScrolledWindow());
	scroll->set_flags(Gtk::CAN_FOCUS);
	scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	//scroll->add(param_tree_view());
	scroll->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	scroll->show();

	//column_time_track->set_visible(false);

	return scroll;
}

void
LayerTree::on_waypoint_changed( synfig::Waypoint waypoint , synfig::ValueNode::Handle value_node)
{
	synfigapp::Action::ParamList param_list;
	param_list.add("canvas",layer_tree_store_->canvas_interface()->get_canvas());
	param_list.add("canvas_interface",layer_tree_store_->canvas_interface());
	param_list.add("value_node",value_node);
	param_list.add("waypoint",waypoint);
//	param_list.add("time",canvas_interface()->get_time());

	etl::handle<studio::Instance>::cast_static(layer_tree_store_->canvas_interface()->get_instance())->process_action("waypoint_set_smart", param_list);
}

void
LayerTree::select_layer(Layer::Handle layer)
{
	Gtk::TreeModel::Children::iterator iter;
	if(layer_tree_store_->find_layer_row(layer,iter))
	{
		if(sorted_layer_tree_store_)
			iter=sorted_layer_tree_store_->convert_child_iter_to_iter(iter);

		Gtk::TreePath path(iter);
		for(int i=path.get_depth();i;i--)
		{
			int j;
			path=Gtk::TreePath(iter);
			for(j=i;j;j--)
				path.up();
			get_layer_tree_view().expand_row(path,false);
		}
		get_layer_tree_view().scroll_to_row(Gtk::TreePath(iter));
		get_layer_tree_view().get_selection()->select(iter);
	}
}

void
LayerTree::select_all_children(Gtk::TreeModel::Children::iterator iter)
{
	get_layer_tree_view().get_selection()->select(iter);
	if((bool)(*iter)[layer_model.children_lock])
		return;
	get_layer_tree_view().expand_row(layer_tree_store_->get_path(iter),false);
	Gtk::TreeModel::Children children(iter->children());
	for(iter=children.begin();iter!=children.end();++iter)
		select_all_children(iter);
}

void
LayerTree::select_all_children_layers(synfig::Layer::Handle layer)
{
	Gtk::TreeModel::Children::iterator iter;
	if(layer_tree_store_->find_layer_row(layer,iter))
		select_all_children(iter);
}

void
LayerTree::select_layers(const LayerList &layer_list)
{
	LayerList::const_iterator iter;
	for(iter = layer_list.begin(); iter != layer_list.end(); ++iter)
		select_layer(*iter);
}

static inline void __layer_grabber(const Gtk::TreeModel::iterator& iter, LayerTree::LayerList* ret)
{
	const LayerTreeStore::Model layer_tree_model;
	ret->push_back((Layer::Handle)(*iter)[layer_tree_model.layer]);
}

LayerTree::LayerList
LayerTree::get_selected_layers()const
{
	Glib::RefPtr<Gtk::TreeSelection> selection=const_cast<Gtk::TreeView&>(get_layer_tree_view()).get_selection();

	if(!selection)
		return LayerList();

	LayerList ret;

	selection->selected_foreach_iter(
		sigc::bind(
			sigc::ptr_fun(
				&__layer_grabber
			),
			&ret
		)
	);

	return ret;
}

synfig::Layer::Handle
LayerTree::get_selected_layer()const
{
	LayerList layers(get_selected_layers());

	if(layers.empty())
		return 0;

	return *layers.begin();
}

void
LayerTree::clear_selected_layers()
{
	get_layer_tree_view().get_selection()->unselect_all();
}













void
LayerTree::set_show_timetrack(bool x)
{
	//column_time_track->set_visible(x);
//	column_time_track->set_visible(false);
	column_z_depth->set_visible(x);
}

void
LayerTree::set_model(Glib::RefPtr<LayerTreeStore> layer_tree_store)
{
	layer_tree_store_=layer_tree_store;


	if(false)
	{
		sorted_layer_tree_store_=Gtk::TreeModelSort::create(layer_tree_store);

		sorted_layer_tree_store_->set_default_sort_func(sigc::ptr_fun(&studio::LayerTreeStore::z_sorter));

		//sorted_store->set_sort_func(model.time.index(),sigc::mem_fun(&studio::KeyframeTreeStore::time_sorter));
		//sorted_store->set_sort_column_id(model.time.index(), Gtk::SORT_ASCENDING);

		get_layer_tree_view().set_model(sorted_layer_tree_store_);
	}
	else
		get_layer_tree_view().set_model(layer_tree_store_);

	layer_tree_store_->canvas_interface()->signal_dirty_preview().connect(sigc::mem_fun(*this,&studio::LayerTree::on_dirty_preview));

	//layer_tree_store_->canvas_interface()->signal_dirty_preview().connect(sigc::mem_fun(*this,&studio::LayerTree::on_dirty_preview));

	layer_tree_store_->canvas_interface()->signal_time_changed().connect(
		sigc::mem_fun(
			&param_tree_view(),
			&Gtk::Widget::queue_draw
		)
	);
	if(!param_tree_store_)
	{
		param_tree_store_=LayerParamTreeStore::create(layer_tree_store_->canvas_interface(), this);
		param_tree_view().set_model(param_tree_store_);
	}

/*	if(cellrenderer_time_track && layer_tree_store_ && layer_tree_store_->canvas_interface())
	{
		cellrenderer_time_track->set_canvas_interface(layer_tree_store_->canvas_interface());
	}
*/
}

void
LayerTree::set_time_adjustment(Gtk::Adjustment &adjustment)
{
	//cellrenderer_time_track->set_adjustment(adjustment);
	adjustment.signal_value_changed().connect(sigc::mem_fun(param_tree_view(),&Gtk::TreeView::queue_draw));
	adjustment.signal_changed().connect(sigc::mem_fun(param_tree_view(),&Gtk::TreeView::queue_draw));
}

void
LayerTree::on_dirty_preview()
{
/*
	if(quick_layer && !disable_amount_changed_signal)
	{
		layer_amount_hscale->set_sensitive(true);
		disable_amount_changed_signal=true;
		layer_amount_adjustment_.set_value(quick_layer->get_param("amount").get(Real()));
		disable_amount_changed_signal=false;
		if(quick_layer->get_param("blend_method").is_valid())
		{
			blend_method_widget.set_sensitive(true);
			disable_amount_changed_signal=true;
			blend_method_widget.set_value(quick_layer->get_param("blend_method"));
			disable_amount_changed_signal=false;
		}
	}
*/
}

void
LayerTree::on_selection_changed()
{
	synfigapp::SelectionManager::LayerList layer_list(get_selection_manager()->get_selected_layers());

	Gtk::TreeIter iter;
	if(last_top_selected_layer && !layer_tree_store_->find_layer_row(last_top_selected_layer,iter))
	{
		if(layer_list.empty())
		{
			last_top_selected_layer=0;
			layer_tree_view_->get_selection()->select(last_top_selected_path);
			return;
		}
	}


	{
		if(!layer_list.empty())
		{
			last_top_selected_layer=layer_list.front();
			last_top_selected_path=*layer_tree_view_->get_selection()->get_selected_rows().begin();
		}
		else
		{
			last_top_selected_layer=0;
		}
	}


	if(layer_list.empty())
	{
		button_raise->set_sensitive(false);
		button_lower->set_sensitive(false);
		button_duplicate->set_sensitive(false);
		button_delete->set_sensitive(false);
		layer_amount_hscale->set_sensitive(false);
		blend_method_widget.set_sensitive(false);
		return;
	}

	button_raise->set_sensitive(true);
	button_lower->set_sensitive(true);
	button_duplicate->set_sensitive(true);
	button_delete->set_sensitive(true);

	if(layer_list.size()==1 && (*layer_list.begin())->get_param("amount").is_valid()&& (*layer_list.begin())->get_param("amount").same_as(Real()))
	{
		quick_layer=*layer_list.begin();
	}
	else
		quick_layer=0;

	if(quick_layer)
	{
		layer_amount_hscale->set_sensitive(true);
		disable_amount_changed_signal=true;
		layer_amount_adjustment_.set_value(quick_layer->get_param("amount").get(Real()));
		disable_amount_changed_signal=false;
		if(quick_layer->get_param("blend_method").is_valid())
		{
			blend_method_widget.set_sensitive(true);
			disable_amount_changed_signal=true;
			blend_method_widget.set_value(quick_layer->get_param("blend_method"));
			disable_amount_changed_signal=false;
		}
		else
			blend_method_widget.set_sensitive(false);
	}
	else
	{
		layer_amount_hscale->set_sensitive(false);
		blend_method_widget.set_sensitive(false);
	}
}


void
LayerTree::on_blend_method_changed()
{
	if(disable_amount_changed_signal)
		return;
	if(!quick_layer)
		return;

	if(quick_layer->get_param("blend_method").is_valid())
	{
		disable_amount_changed_signal=true;
		signal_edited_value()(synfigapp::ValueDesc(quick_layer,"blend_method"),blend_method_widget.get_value());
		disable_amount_changed_signal=false;
	}
}

void
LayerTree::on_amount_value_changed()
{
	if(disable_amount_changed_signal)
		return;
	if(!quick_layer)
		return;

	disable_amount_changed_signal=true;
	signal_edited_value()(synfigapp::ValueDesc(quick_layer,"amount"),synfig::ValueBase(layer_amount_adjustment_.get_value()));
	disable_amount_changed_signal=false;
}


void
LayerTree::on_edited_value(const Glib::ustring&path_string,synfig::ValueBase value)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(param_tree_view().get_model()->get_iter(path));
	if(!row)
		return;
	row[param_model.value]=value;
	//signal_edited_value()(row[param_model.value_desc],value);
}

void
LayerTree::on_layer_renamed(const Glib::ustring&path_string,const Glib::ustring& value)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(get_layer_tree_view().get_model()->get_iter(path));
	if(!row)
		return;
	row[layer_model.label]=value;
}

void
LayerTree::on_layer_toggle(const Glib::ustring& path_string)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(get_layer_tree_view().get_model()->get_iter(path));
	bool active=static_cast<bool>(row[layer_model.active]);
	row[layer_model.active]=!active;
}

void
LayerTree::on_waypoint_clicked(const Glib::ustring &path_string, synfig::Waypoint waypoint,int button)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(param_tree_view().get_model()->get_iter(path));
	if(!row)
		return;

	signal_waypoint_clicked()(static_cast<synfigapp::ValueDesc>(row[param_model.value_desc]),waypoint,button);
}

bool
LayerTree::on_layer_tree_event(GdkEvent *event)
{
    switch(event->type)
    {
	case GDK_BUTTON_PRESS:
		{
			Gtk::TreeModel::Path path;
			Gtk::TreeViewColumn *column;
			int cell_x, cell_y;
			if(!get_layer_tree_view().get_path_at_pos(
				int(event->button.x),int(event->button.y),	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;
			const Gtk::TreeRow row = *(get_layer_tree_view().get_model()->get_iter(path));

			//if(column->get_first_cell_renderer()==cellrenderer_time_track)
			//	return signal_layer_user_click()(event->button.button,row,COLUMNID_TIME_TRACK);
			//else
				if(column->get_first_cell_renderer()==cellrenderer_value)
				return signal_layer_user_click()(event->button.button,row,COLUMNID_VALUE);
			else
				return signal_layer_user_click()(event->button.button,row,COLUMNID_NAME);

		}
		break;

	case GDK_MOTION_NOTIFY:
		{
			Gtk::TreeModel::Path path;
			Gtk::TreeViewColumn *column;
			int cell_x, cell_y;
			if(!get_layer_tree_view().get_path_at_pos(
				(int)event->button.x,(int)event->button.y,	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;

			if(!get_layer_tree_view().get_model()->get_iter(path))
				break;

			Gtk::TreeRow row = *(get_layer_tree_view().get_model()->get_iter(path));

			/*
			if(cellrenderer_time_track==column->get_first_cell_renderer())
			{
				// Movement on TimeLine
				return true;
			}
			else
				*/
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
		}
		break;
	case GDK_BUTTON_RELEASE:
		break;
	default:
		break;
	}
	return false;
}


bool
LayerTree::on_param_tree_event(GdkEvent *event)
{
    switch(event->type)
    {
	case GDK_BUTTON_PRESS:
		{
			Gtk::TreeModel::Path path;
			Gtk::TreeViewColumn *column;
			int cell_x, cell_y;
			if(!param_tree_view().get_path_at_pos(
				int(event->button.x),int(event->button.y),	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;
			const Gtk::TreeRow row = *(param_tree_view().get_model()->get_iter(path));

/*			if(column && column->get_first_cell_renderer()==cellrenderer_time_track)
			{
				Gdk::Rectangle rect;
				param_tree_view().get_cell_area(path,*column,rect);
				cellrenderer_time_track->property_value_desc()=row[param_model.value_desc];
				cellrenderer_time_track->property_canvas()=row[param_model.canvas];
				cellrenderer_time_track->activate(event,*this,path.to_string(),rect,rect,Gtk::CellRendererState());
				param_tree_view().queue_draw_area(rect.get_x(),rect.get_y(),rect.get_width(),rect.get_height());
				return true;
				//return signal_param_user_click()(event->button.button,row,COLUMNID_TIME_TRACK);
			}
			else
*/			{
				if(event->button.button==3)
				{
					LayerList layer_list(get_selected_layers());
					if(layer_list.size()<=1)
					{
						synfigapp::ValueDesc value_desc(row[param_model.value_desc]);
						Gtk::Menu* menu(manage(new Gtk::Menu()));
						App::get_instance(param_tree_store_->canvas_interface()->get_canvas())->make_param_menu(menu,param_tree_store_->canvas_interface()->get_canvas(),value_desc,0.5f);
						menu->popup(event->button.button,gtk_get_current_event_time());
						return true;
					}
					Gtk::Menu* menu(manage(new Gtk::Menu()));
					std::list<synfigapp::ValueDesc> value_desc_list;
					ParamDesc param_desc(row[param_model.param_desc]);
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
		}
		break;

	case GDK_MOTION_NOTIFY:
		{
			Gtk::TreeModel::Path path;
			Gtk::TreeViewColumn *column;
			int cell_x, cell_y;
			if(!param_tree_view().get_path_at_pos(
				(int)event->motion.x,(int)event->motion.y,	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;

			if(!param_tree_view().get_model()->get_iter(path))
				break;

			Gtk::TreeRow row = *(param_tree_view().get_model()->get_iter(path));

/*			if((event->motion.state&GDK_BUTTON1_MASK ||event->motion.state&GDK_BUTTON3_MASK) && column && cellrenderer_time_track==column->get_first_cell_renderer())
			{
				Gdk::Rectangle rect;
				param_tree_view().get_cell_area(path,*column,rect);
				cellrenderer_time_track->property_value_desc()=row[param_model.value_desc];
				cellrenderer_time_track->property_canvas()=row[param_model.canvas];
				cellrenderer_time_track->activate(event,*this,path.to_string(),rect,rect,Gtk::CellRendererState());
				param_tree_view().queue_draw();
				//param_tree_view().queue_draw_area(rect.get_x(),rect.get_y(),rect.get_width(),rect.get_height());
				return true;
			}
			else
*/			if(last_tooltip_path.get_depth()<=0 || path!=last_tooltip_path)
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
		}
		break;
	case GDK_BUTTON_RELEASE:
		{
			Gtk::TreeModel::Path path;
			Gtk::TreeViewColumn *column;
			int cell_x, cell_y;
			if(!param_tree_view().get_path_at_pos(
				(int)event->button.x,(int)event->button.y,	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;

			if(!param_tree_view().get_model()->get_iter(path))
				break;

			Gtk::TreeRow row = *(param_tree_view().get_model()->get_iter(path));

/*			if(column && cellrenderer_time_track==column->get_first_cell_renderer())
			{
				Gdk::Rectangle rect;
				param_tree_view().get_cell_area(path,*column,rect);
				cellrenderer_time_track->property_value_desc()=row[param_model.value_desc];
				cellrenderer_time_track->property_canvas()=row[param_model.canvas];
				cellrenderer_time_track->activate(event,*this,path.to_string(),rect,rect,Gtk::CellRendererState());
				param_tree_view().queue_draw();
				param_tree_view().queue_draw_area(rect.get_x(),rect.get_y(),rect.get_width(),rect.get_height());
				return true;

			}
*/
		}
		break;
	default:
		break;
	}
	return false;
}

void
LayerTree::on_raise_pressed()
{
	synfigapp::Action::ParamList param_list;
	param_list.add("time",layer_tree_store_->canvas_interface()->get_time());
	param_list.add("canvas",layer_tree_store_->canvas_interface()->get_canvas());
	param_list.add("canvas_interface",layer_tree_store_->canvas_interface());

	{
		synfigapp::SelectionManager::LayerList layer_list(get_selection_manager()->get_selected_layers());
		synfigapp::SelectionManager::LayerList::iterator iter;

		for(iter=layer_list.begin();iter!=layer_list.end();++iter)
			param_list.add("layer",Layer::Handle(*iter));
	}
	synfigapp::Action::Handle action(synfigapp::Action::create("layer_raise"));
	action->set_param_list(param_list);
	layer_tree_store_->canvas_interface()->get_instance()->perform_action(action);
}

void
LayerTree::on_lower_pressed()
{
	synfigapp::Action::ParamList param_list;
	param_list.add("time",layer_tree_store_->canvas_interface()->get_time());
	param_list.add("canvas",layer_tree_store_->canvas_interface()->get_canvas());
	param_list.add("canvas_interface",layer_tree_store_->canvas_interface());

	{
		synfigapp::SelectionManager::LayerList layer_list(get_selection_manager()->get_selected_layers());
		synfigapp::SelectionManager::LayerList::iterator iter;

		for(iter=layer_list.begin();iter!=layer_list.end();++iter)
			param_list.add("layer",Layer::Handle(*iter));
	}

	synfigapp::Action::Handle action(synfigapp::Action::create("layer_lower"));
	action->set_param_list(param_list);
	layer_tree_store_->canvas_interface()->get_instance()->perform_action(action);
}

void
LayerTree::on_duplicate_pressed()
{
	synfigapp::Action::ParamList param_list;
	param_list.add("time",layer_tree_store_->canvas_interface()->get_time());
	param_list.add("canvas",layer_tree_store_->canvas_interface()->get_canvas());
	param_list.add("canvas_interface",layer_tree_store_->canvas_interface());

	{
		synfigapp::SelectionManager::LayerList layer_list(get_selection_manager()->get_selected_layers());
		synfigapp::SelectionManager::LayerList::iterator iter;

		for(iter=layer_list.begin();iter!=layer_list.end();++iter)
			param_list.add("layer",Layer::Handle(*iter));
	}

	synfigapp::Action::Handle action(synfigapp::Action::create("layer_duplicate"));
	action->set_param_list(param_list);
	layer_tree_store_->canvas_interface()->get_instance()->perform_action(action);
}

void
LayerTree::on_delete_pressed()
{
	synfigapp::Action::ParamList param_list;
	param_list.add("time",layer_tree_store_->canvas_interface()->get_time());
	param_list.add("canvas",layer_tree_store_->canvas_interface()->get_canvas());
	param_list.add("canvas_interface",layer_tree_store_->canvas_interface());

	{
		synfigapp::SelectionManager::LayerList layer_list(get_selection_manager()->get_selected_layers());
		synfigapp::SelectionManager::LayerList::iterator iter;

		for(iter=layer_list.begin();iter!=layer_list.end();++iter)
			param_list.add("layer",Layer::Handle(*iter));
	}

	synfigapp::Action::Handle action(synfigapp::Action::create("layer_remove"));
	action->set_param_list(param_list);
	layer_tree_store_->canvas_interface()->get_instance()->perform_action(action);
}




















/*
void
LayerTree::on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>&context, Gtk::SelectionData& selection_data, guint info, guint time)
{
	synfig::info("Dragged data of type \"%s\"",selection_data.get_data_type());
	synfig::info("Dragged data of target \"%s\"",gdk_atom_name(selection_data->target));
	synfig::info("Dragged selection=\"%s\"",gdk_atom_name(selection_data->selection));

	DEBUGPOINT();

	Gtk::TreeModel::Path path;
	Gtk::TreeViewColumn *column;
	int cell_x, cell_y;
	if(get_selection()
	Gtk::TreeRow row = *(get_selection()->get_selected());
	DEBUGPOINT();

	if(synfig::String(gdk_atom_name(selection_data->target))=="LAYER" && (bool)row[model.is_layer])
	{
		DEBUGPOINT();
		Layer* layer(((Layer::Handle)row[model.layer]).get());
		assert(layer);
		selection_data.set(8, reinterpret_cast<const guchar*>(&layer), sizeof(layer));
		return;
	}
	DEBUGPOINT();
}

void
LayerTree::on_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, Gtk::SelectionData& selection_data, guint info, guint time)
{
	synfig::info("Dropped data of type \"%s\"",selection_data.get_data_type());
	synfig::info("Dropped data of target \"%s\"",gdk_atom_name(selection_data->target));
	synfig::info("Dropped selection=\"%s\"",gdk_atom_name(selection_data->selection));
	synfig::info("Dropped x=%d, y=%d",x,y);
	bool success=false;
	bool dropped_on_specific_row=false;

	Gtk::TreeModel::Path path;
	Gtk::TreeViewColumn *column;
	int cell_x, cell_y;
	if(!get_path_at_pos(
		x,y,	// x, y
		path, // TreeModel::Path&
		column, //TreeViewColumn*&
		cell_x,cell_y //int&cell_x,int&cell_y
		)
	)
	{
		dropped_on_specific_row=false;
	}
	else
		dropped_on_specific_row=true;

	Gtk::TreeRow row = *(get_model()->get_iter(path));


	if ((selection_data.get_length() >= 0) && (selection_data.get_format() == 8))
	{
		if(synfig::String(selection_data.get_data_type())=="LAYER")do
		{
			Layer::Handle src(*reinterpret_cast<Layer**>(selection_data.get_data()));
			assert(src);

			Canvas::Handle dest_canvas;
			Layer::Handle dest_layer;

			if(dropped_on_specific_row)
			{
				dest_canvas=(Canvas::Handle)(row[model.canvas]);
				dest_layer=(Layer::Handle)(row[model.layer]);
				assert(dest_canvas);
			}
			else
				dest_canvas=layer_tree_store_->canvas_interface()->get_canvas();

			// In this case, we are just moving.
			if(dest_canvas==src->get_canvas())
			{
				if(!dest_layer || dest_layer==src)
					break;

				synfigapp::Action::Handle action(synfigapp::Action::create("layer_move"));
				action->set_param("canvas",dest_canvas);
				action->set_param("canvas_interface",layer_tree_store_->canvas_interface());
				action->set_param("layer",src);
				action->set_param("new_index",dest_canvas->get_depth(dest_layer));
				if(layer_tree_store_->canvas_interface()->get_instance()->perform_action(action))
					success=true;
				else
					success=false;
				break;
			}
		}while(0);
	}

	// Finish the drag
	context->drag_finish(success, false, time);
}
*/

/*bool
LayerTree::on_drag_motion(const Glib::RefPtr<Gdk::DragContext>& context,int x, int    y, guint    time)
{
	return get_layer_tree_view().on_drag_motion(context,x,y,time);
}

void
LayerTree::on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, Gtk::SelectionData& selection_data, guint info, guint time)
{
	get_layer_tree_view().on_drag_data_received(context,x,y,selection_data,info,time);
*/
/*
	if(context->gobj()->source_window==context->gobj()->dest_window)
	{
		Gtk::TreeView::on_drag_data_received(context,x,y,selection_data,info,time);
		return;
	}

	Gtk::TreeModel::Path path;
	Gtk::TreeViewColumn *column;
	int cell_x, cell_y;
	if(!get_path_at_pos(
		x,y,	// x, y
		path, // TreeModel::Path&
		column, //TreeViewColumn*&
		cell_x,cell_y //int&cell_x,int&cell_y
		)
	)
	{
		context->drag_finish(false, false, time);
	}

	if(layer_tree_store_->row_drop_possible(path,selection_data))
	{
		if(layer_tree_store_->drag_data_received(path,selection_data))
			context->drag_finish(true, false, time);
	}
	context->drag_finish(false, false, time);
}
*/
