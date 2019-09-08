/* === S Y N F I G ========================================================= */
/*!	\file layertree.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2011 Carlos López
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

#include <synfig/general.h>

#include "layertree.h"
#include "layerparamtreestore.h"
#include "cellrenderer/cellrenderer_value.h"
#include "cellrenderer/cellrenderer_timetrack.h"
#include <synfigapp/action.h>
#include <synfigapp/action_system.h>
#include <synfigapp/actions/layerremove.h>
#include <synfigapp/instance.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/paned.h>
#include "app.h"
#include "instance.h"
#include <gtkmm/treemodelsort.h>

#ifdef TIMETRACK_IN_PARAMS_PANEL
#  include <synfig/timepointcollect.h>
#endif	// TIMETRACK_IN_PARAMS_PANEL

#include <gui/localization.h>

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
	button->set_tooltip_text(tooltip);	\
	icon->set_padding(0,0);\
	icon->show();	\
	button->set_relief(Gtk::RELIEF_NONE); \
	button->show()
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/*
	Return true if we process event,
	False to pass it
*/
bool LayerTree::onKeyPress(GdkEventKey* event)
{

	switch (event->keyval) {
		case GDK_KEY_Delete: {
			LayerList layers = get_selected_layers();

			if (layers.size() == 0) return true; // nothing to do
			

			synfigapp::Action::Handle action(synfigapp::Action::LayerRemove::create());
			assert(action);
			if (!action)
				return true;
			
			action->set_param("canvas", layer_tree_store_->canvas_interface()->get_canvas());
			action->set_param("canvas_interface", layer_tree_store_->canvas_interface());

			for(LayerList::const_iterator i = layers.begin(); i != layers.end(); ++i)
				action->set_param("layer", *i);

			layer_tree_store_->canvas_interface()->get_instance()->perform_action(action);
			return true;

		} 
		case GDK_KEY_F2: {
			Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
			std::vector<Gtk::TreeModel::Path> pathList = selection->get_selected_rows();
			if (pathList.size() != 1) return true;

			const Gtk::TreeModel::Path& path = pathList.front();
			Gtk::TreeView::Column& focus_column = *(layer_tree_view_->get_column(2));

			layer_tree_view_->set_cursor(
				path, 
				focus_column, 
				true
			);
			return true;
		}
	}

    return false;
}


LayerTree::LayerTree():
	layer_amount_adjustment_(Gtk::Adjustment::create(1,0,1,0.01,0.01,0))
{
	param_tree_view_=new Gtk::TreeView;
	layer_tree_view_=new Gtk::TreeView;

	layer_tree_view_->signal_key_press_event().connect(sigc::mem_fun(*this, &LayerTree::onKeyPress));

	create_layer_tree();
	create_param_tree();

	hbox=manage(new Gtk::HBox());

	attach(*hbox, 0, 1, 1, 2, Gtk::FILL|Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	attach(blend_method_widget, 2, 3, 1, 2,Gtk::SHRINK, Gtk::SHRINK, 0, 0);

	layer_amount_hscale=manage(new Gtk::HScale(layer_amount_adjustment_));
	layer_amount_hscale->set_digits(2);
	layer_amount_hscale->set_value_pos(Gtk::POS_LEFT);
	layer_amount_hscale->set_sensitive(false);
	attach(*layer_amount_hscale, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK, 1, 1);
	layer_amount_adjustment_->signal_value_changed().connect(sigc::mem_fun(*this, &studio::LayerTree::on_amount_value_changed));

	Gtk::Image *icon;
	Gtk::IconSize iconsize(Gtk::ICON_SIZE_SMALL_TOOLBAR);

	SMALL_BUTTON(button_raise,"gtk-go-up","Raise");
	SMALL_BUTTON(button_lower,"gtk-go-down","Lower");
	SMALL_BUTTON(button_duplicate,"synfig-duplicate","Duplicate");
	SMALL_BUTTON(button_encapsulate,"synfig-encapsulate","Group");
	SMALL_BUTTON(button_delete,"gtk-delete","Delete");

	hbox->pack_start(*button_raise,Gtk::PACK_SHRINK);
	hbox->pack_start(*button_lower,Gtk::PACK_SHRINK);
	hbox->pack_start(*button_duplicate,Gtk::PACK_SHRINK);
	hbox->pack_start(*button_encapsulate,Gtk::PACK_SHRINK);
	hbox->pack_start(*button_delete,Gtk::PACK_SHRINK);

	// button_raise->signal_clicked().connect(sigc::mem_fun(*this, &studio::LayerTree::on_raise_pressed));
	// button_lower->signal_clicked().connect(sigc::mem_fun(*this, &studio::LayerTree::on_lower_pressed));
	// button_duplicate->signal_clicked().connect(sigc::mem_fun(*this, &studio::LayerTree::on_duplicate_pressed));
	// button_encapsulate->signal_clicked().connect(sigc::mem_fun(*this, &studio::LayerTree::on_encapsulate_pressed));
	// button_delete->signal_clicked().connect(sigc::mem_fun(*this, &studio::LayerTree::on_delete_pressed));

	button_raise->set_sensitive(false);
	button_lower->set_sensitive(false);
	button_duplicate->set_sensitive(false);
	button_encapsulate->set_sensitive(false);
	button_delete->set_sensitive(false);

	get_selection()->signal_changed().connect(sigc::mem_fun(*this, &studio::LayerTree::on_selection_changed));

	get_layer_tree_view().set_reorderable(true);
	get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	//get_param_tree_view().get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	get_layer_tree_view().show();
	get_param_tree_view().show();

	hbox->show();
	layer_amount_hscale->show();
	blend_method_widget.show();

	get_param_tree_view().set_has_tooltip();
	get_layer_tree_view().set_has_tooltip();

	disable_amount_changed_signal=false;

	blend_method_widget.set_param_desc(ParamDesc((int)Color::BlendMethod(),"blend_method"));

	blend_method_widget.set_value((int)Color::BLEND_COMPOSITE);
	blend_method_widget.set_size_request(150,-1);
	blend_method_widget.set_sensitive(false);
	blend_method_widget.signal_activate().connect(sigc::mem_fun(*this, &studio::LayerTree::on_blend_method_changed));
}

LayerTree::~LayerTree()
{
	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("LayerTree::~LayerTree(): Deleted");
}

void
LayerTree::create_layer_tree()
{
	{	// --- O N / O F F ----------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_(" ")) );

		// Set up the on/off cell-renderer
		Gtk::CellRendererToggle* cellrenderer = Gtk::manage( new Gtk::CellRendererToggle() );
		cellrenderer->signal_toggled().connect(sigc::mem_fun(*this, &studio::LayerTree::on_layer_toggle));
		column->pack_start(*cellrenderer,false);
		column->add_attribute(cellrenderer->property_active(), layer_model.active);
		get_layer_tree_view().append_column(*column);
	}

	{	// --- I C O N --------------------------------------------------------
		int index;
		// Set up the icon cell-renderer
		index=get_layer_tree_view().append_column(_("Icon"),layer_model.icon);
		Gtk::TreeView::Column* column = get_layer_tree_view().get_column(index-1);
		get_layer_tree_view().set_expander_column(*column);
	}
	{	// --- N A M E --------------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Name")) );

		// Set up the Layer label cell-renderer
		Gtk::CellRendererText* cellrenderer = Gtk::manage( new Gtk::CellRendererText() );
		column->pack_start(*cellrenderer,false);
		column->add_attribute(cellrenderer->property_text(), layer_model.label);
		column->add_attribute(cellrenderer->property_style(), layer_model.style);
		column->add_attribute(cellrenderer->property_weight(), layer_model.weight);
		column->add_attribute(cellrenderer->property_underline(), layer_model.underline);
		column->add_attribute(cellrenderer->property_strikethrough(), layer_model.strikethrough);
		cellrenderer->signal_edited().connect(sigc::mem_fun(*this, &studio::LayerTree::on_layer_renamed));
		cellrenderer->property_editable()=true;

		column->set_reorderable();
		column->set_resizable();
		column->set_clickable(true);
		column->set_sort_column(layer_model.label);

		get_layer_tree_view().append_column(*column);
	}
	{	// --- Z D E P T H ----------------------------------------------------
		int index;
		index=get_layer_tree_view().append_column(_("Z Depth"),layer_model.z_depth);
		// Set up the Z-Depth label cell-renderer

		column_z_depth=get_layer_tree_view().get_column(index-1);
		column_z_depth->set_reorderable();
		column_z_depth->set_resizable();
		column_z_depth->set_clickable();

		column_z_depth->set_sort_column(layer_model.z_depth);
	}

	get_layer_tree_view().set_enable_search(true);
	get_layer_tree_view().set_search_column(layer_model.label);
	get_layer_tree_view().set_search_equal_func(sigc::ptr_fun(&studio::LayerTreeStore::search_func));

	std::vector<Gtk::TargetEntry> listTargets;
	listTargets.push_back( Gtk::TargetEntry("LAYER") );
	get_layer_tree_view().drag_dest_set(listTargets);

	// This makes things easier to read.
	get_layer_tree_view().set_rules_hint();

	// Make us more sensitive to several events
	//get_layer_tree_view().add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON1_MOTION_MASK | Gdk::BUTTON2_MOTION_MASK|Gdk::POINTER_MOTION_MASK);

	get_layer_tree_view().signal_event().connect(sigc::mem_fun(*this, &studio::LayerTree::on_layer_tree_event));
	get_layer_tree_view().signal_query_tooltip().connect(sigc::mem_fun(*this, &studio::LayerTree::on_layer_tree_view_query_tooltip));
	get_layer_tree_view().show();
}

void
LayerTree::create_param_tree()
{
	//Text attributes must be the same that TimeTrackView tree's to have aligned rows
	Pango::AttrList attr_list;
	{
		Pango::AttrInt pango_size(Pango::Attribute::create_attr_size(Pango::SCALE*8));
		pango_size.set_start_index(0);
		pango_size.set_end_index(64);
		attr_list.change(pango_size);
	}

	Gtk::IconSize icon_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);

	{	// --- N A M E --------------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Name")) );

		// Affect a widget to the first column button to retrieve it when style is updated
		Gtk::Label* columnzero_label = Gtk::manage( new Gtk::Label() );
		columnzero_label->set_text(column->get_title());
		column->set_widget(*columnzero_label);
		columnzero_label->show();

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

		// Set up the value-node icon cell-renderer to be on the far right
		Gtk::CellRendererPixbuf* valuenode_icon_cellrenderer = Gtk::manage( new Gtk::CellRendererPixbuf() );
		column->pack_end(*valuenode_icon_cellrenderer,false);
		valuenode_icon_cellrenderer->property_pixbuf()=Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-value_node"),icon_size);
		column->add_attribute(valuenode_icon_cellrenderer->property_visible(), param_model.is_shared);

		// Finish setting up the column
		column->set_reorderable();
		column->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
		column->set_fixed_width(150);
		column->set_min_width(75);
		column->set_resizable();
		column->set_clickable();
		column->set_sort_column(param_model.name);

		get_param_tree_view().append_column(*column);
	}
	{	// --- V A L U E  -----------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Value")) );

		// Set up the value cell-renderer
		cellrenderer_value=LayerParamTreeStore::add_cell_renderer_value(column);
		cellrenderer_value->signal_edited().connect(sigc::mem_fun(*this, &studio::LayerTree::on_edited_value));
		cellrenderer_value->property_value()=synfig::ValueBase();
		column->add_attribute(cellrenderer_value->property_param_desc(), param_model.param_desc);
		column->add_attribute(cellrenderer_value->property_value_desc(), param_model.value_desc);
		column->add_attribute(cellrenderer_value->property_child_param_desc(), param_model.child_param_desc);
		column->add_attribute(cellrenderer_value->property_inconsistent(),param_model.is_inconsistent);
		//cellrenderer_value->property_canvas()=canvas_interface->get_canvas(); // Is this line necessary?
		cellrenderer_value->property_attributes()=attr_list;

		// Finish setting up the column
		get_param_tree_view().append_column(*column);
		column->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
		column->set_fixed_width(150);
		column->set_min_width(75);
		column->set_clickable();
		column->set_reorderable();
		column->set_resizable();
	}
	{
		// --- F L A G S (static/interpolation) -----------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_(" ")) );

		// Set up the interpolation icon cell-renderer to be on the far right
		Gtk::CellRendererPixbuf* interpolation_icon_cellrenderer = Gtk::manage( new Gtk::CellRendererPixbuf() );
		column->pack_end(*interpolation_icon_cellrenderer,false);
		column->add_attribute(interpolation_icon_cellrenderer->property_pixbuf(),param_model.interpolation_icon);
		column->add_attribute(interpolation_icon_cellrenderer->property_visible(), param_model.interpolation_icon_visible);

		// Set up the static icon cell-renderer to be on the far right
		Gtk::CellRendererPixbuf* static_icon_cellrenderer = Gtk::manage( new Gtk::CellRendererPixbuf() );
		column->pack_end(*static_icon_cellrenderer,false);
		static_icon_cellrenderer->property_pixbuf()=Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-valuenode_forbidanimation"),icon_size);
		column->add_attribute(static_icon_cellrenderer->property_visible(), param_model.is_static);

		// Finish setting up the column
		get_param_tree_view().append_column(*column);
		column->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
		column->set_fixed_width(26);
		column->set_min_width(26);
		column->set_clickable();
		column->set_reorderable();
		column->set_resizable();
	}
	{	// --- T Y P E --------------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Type")) );
		Gtk::CellRendererText* text_cellrenderer = Gtk::manage( new Gtk::CellRendererText() );
		column->pack_start(*text_cellrenderer,false);
		column->add_attribute(text_cellrenderer->property_text(), param_model.type);
		text_cellrenderer->property_attributes()=attr_list;

		get_param_tree_view().append_column(*column);
		column->set_reorderable();
		column->set_resizable();
		column->set_clickable();
		column->set_sort_column(param_model.type);
		column->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
		column->set_fixed_width(75);
		column->set_min_width(50);
	}
#ifdef TIMETRACK_IN_PARAMS_PANEL
	{	// --- T I M E   T R A C K --------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Time Track")) );
		column_time_track=column;

		// Set up the value-node cell-renderer
		cellrenderer_time_track=LayerParamTreeStore::add_cell_renderer_value_node(column);
		cellrenderer_time_track->property_mode()=Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
		cellrenderer_time_track->signal_waypoint_clicked_cellrenderer().connect(sigc::mem_fun(*this, &studio::LayerTree::on_waypoint_clicked_layertree) );
		cellrenderer_time_track->signal_waypoint_changed().connect(sigc::mem_fun(*this, &studio::LayerTree::on_waypoint_changed) );
		column->add_attribute(cellrenderer_time_track->property_value_desc(), param_model.value_desc);
		// this already added in constructor
		//column->add_attribute(cellrenderer_time_track->property_canvas(), param_model.canvas);
	    column->add_attribute(cellrenderer_time_track->property_visible(), param_model.is_value_node);

		// Finish setting up the column
		column->set_reorderable();
		column->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
		column->set_fixed_width(200);
		column->set_min_width(100);
		column->set_resizable();

		if (!getenv("SYNFIG_DISABLE_PARAMS_PANEL_TIMETRACK"))
			get_param_tree_view().append_column(*column);
	}
#endif	// TIMETRACK_IN_PARAMS_PANEL

	// This makes things easier to read.
	get_param_tree_view().set_rules_hint();

	// Make us more sensitive to several events
	get_param_tree_view().add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON1_MOTION_MASK | Gdk::BUTTON2_MOTION_MASK|Gdk::POINTER_MOTION_MASK);

	get_param_tree_view().signal_event().connect(sigc::mem_fun(*this, &studio::LayerTree::on_param_tree_event));
	get_param_tree_view().signal_query_tooltip().connect(sigc::mem_fun(*this, &studio::LayerTree::on_param_tree_view_query_tooltip));
	// Column widget label event used to retrieve column size
	Gtk::Widget* columnzero_label = get_param_tree_view().get_column(0)->get_widget ();
	columnzero_label->signal_style_updated().connect(sigc::mem_fun(*this, &studio::LayerTree::on_param_column_label_tree_style_updated));
	columnzero_label->signal_draw().connect(sigc::mem_fun(*this, &studio::LayerTree::on_param_column_label_tree_draw));
	get_param_tree_view().show();

	// To get the initial style
	param_tree_style_changed = true;
	param_tree_header_height = 0;

	//column_time_track->set_visible(false);
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

	etl::handle<studio::Instance>::cast_static(layer_tree_store_->canvas_interface()->get_instance())->process_action("WaypointSetSmart", param_list);
}

void
LayerTree::select_layer(synfig::Layer::Handle layer)
{
	Gtk::TreeModel::Children::iterator iter;
	if(layer_tree_store_->find_layer_row(layer,iter))
	{
		if(sorted_layer_tree_store_)
			iter=sorted_layer_tree_store_->convert_child_iter_to_iter(iter);

		Gtk::TreePath path(iter);
		for(int i=(int)path.size();i;i--)
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
	LayerTreeStore::RecordType record_type((*iter)[layer_tree_model.record_type]);
	Layer::Handle layer((*iter)[layer_tree_model.layer]);
	if (record_type == LayerTreeStore::RECORD_TYPE_LAYER && layer)
		ret->push_back(layer);
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

	return layers.front();
}

void
LayerTree::clear_selected_layers()
{
	get_layer_tree_view().get_selection()->unselect_all();
}

void
LayerTree::expand_layer(synfig::Layer::Handle layer)
{
	if (!layer) return;
	Gtk::TreeModel::Children::iterator iter;
	if(layer_tree_store_->find_layer_row(layer,iter))
	{
		if(sorted_layer_tree_store_)
			iter=sorted_layer_tree_store_->convert_child_iter_to_iter(iter);

		Gtk::TreePath path(iter);
		get_layer_tree_view().expand_to_path(path);
	}
}

void
LayerTree::expand_layers(const LayerList& layer_list)
{
	for(LayerList::const_iterator i = layer_list.begin(); i != layer_list.end(); ++i)
		expand_layer(*i);
}

LayerTree::LayerList
LayerTree::get_expanded_layers()const
{
	LayerList list;
	get_expanded_layers(list, layer_tree_store_->children());
	return list;
}

void
LayerTree::get_expanded_layers(LayerList &list, const Gtk::TreeNodeChildren &rows)const
{
	const LayerTreeStore::Model model;
	for(Gtk::TreeNodeChildren::const_iterator i = rows.begin(); i != rows.end(); ++i)
	{
		if ( (LayerTreeStore::RecordType)(*i)[model.record_type] == LayerTreeStore::RECORD_TYPE_LAYER
		  && (Layer::Handle)(*i)[model.layer] )
		{
			Gtk::TreeNodeChildren::const_iterator j = i;
			if(sorted_layer_tree_store_)
				j = sorted_layer_tree_store_->convert_child_iter_to_iter(i);
			Gtk::TreePath path(j);
			if (const_cast<Gtk::TreeView*>(&get_layer_tree_view())->row_expanded(path))
			{
				list.push_back( (Layer::Handle)(*i)[model.layer] );
				get_expanded_layers(list, i->children());
			}
		}
	}
}

void
LayerTree::set_show_timetrack(bool x)
{
	//column_time_track->set_visible(x);
//	column_time_track->set_visible(false);
	column_z_depth->set_visible(x);
}

void
LayerTree::select_param(const synfigapp::ValueDesc& valuedesc)
{
    get_param_tree_view().get_selection()->unselect_all();

    Gtk::TreeIter iter;
    if(param_tree_store_->find_value_desc(valuedesc, iter))
    {
        Gtk::TreePath path(iter);
        for(int i=(int)path.size();i;i--)
        {
            int j;
            path=Gtk::TreePath(iter);
            for(j=i;j;j--)
                path.up();
            get_param_tree_view().expand_row(path,false);
        }

        get_param_tree_view().scroll_to_row(Gtk::TreePath(iter));
        get_param_tree_view().get_selection()->select(iter);
    }
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
		//sorted_store->set_sort_column(model.time.index(), Gtk::SORT_ASCENDING);

		get_layer_tree_view().set_model(sorted_layer_tree_store_);
	}
	else
		get_layer_tree_view().set_model(layer_tree_store_);

	layer_tree_store_->canvas_interface()->signal_dirty_preview().connect(sigc::mem_fun(*this,&studio::LayerTree::on_dirty_preview));

	//layer_tree_store_->canvas_interface()->signal_dirty_preview().connect(sigc::mem_fun(*this,&studio::LayerTree::on_dirty_preview));

	layer_tree_store_->canvas_interface()->signal_time_changed().connect(
		sigc::mem_fun(
			&get_param_tree_view(),
			&Gtk::Widget::queue_draw
		)
	);
	if(!param_tree_store_)
	{
		param_tree_store_=LayerParamTreeStore::create(layer_tree_store_->canvas_interface(), this);
		get_param_tree_view().set_model(param_tree_store_);
	}

#ifdef TIMETRACK_IN_PARAMS_PANEL
	if(cellrenderer_time_track && layer_tree_store_ && layer_tree_store_->canvas_interface())
		cellrenderer_time_track->set_canvas_interface(layer_tree_store_->canvas_interface());
#endif	// TIMETRACK_IN_PARAMS_PANEL
}

void
LayerTree::set_time_model(const etl::handle<TimeModel> &x)
{
	#ifdef TIMETRACK_IN_PARAMS_PANEL
	cellrenderer_time_track->set_time_model(x);
	#endif
	x->signal_time_changed().connect(sigc::mem_fun(get_param_tree_view(),&Gtk::TreeView::queue_draw));
}

void
LayerTree::on_dirty_preview()
{
/*
	if(quick_layer && !disable_amount_changed_signal)
	{
		layer_amount_hscale->set_sensitive(true);
		disable_amount_changed_signal=true;
		layer_amount_adjustment_->set_value(quick_layer->get_param("amount").get(Real()));
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
		button_encapsulate->set_sensitive(false);
		button_delete->set_sensitive(false);
		layer_amount_hscale->set_sensitive(false);
		blend_method_widget.set_sensitive(false);
		return;
	}

	button_raise->set_sensitive(true);
	button_lower->set_sensitive(true);
	button_duplicate->set_sensitive(true);
	button_encapsulate->set_sensitive(true);
	button_delete->set_sensitive(true);

	if(layer_list.size()==1 && (*layer_list.begin())->get_param("amount").is_valid()&& (*layer_list.begin())->get_param("amount").same_type_as(Real()))
	{
		quick_layer=*layer_list.begin();
	}
	else
		quick_layer=0;

	if(quick_layer)
	{
		layer_amount_hscale->set_sensitive(true);
		disable_amount_changed_signal=true;
		layer_amount_adjustment_->set_value(quick_layer->get_param("amount").get(Real()));
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
	signal_edited_value()(synfigapp::ValueDesc(quick_layer,"amount"),synfig::ValueBase(layer_amount_adjustment_->get_value()));
	disable_amount_changed_signal=false;
}

void
LayerTree::on_edited_value(const Glib::ustring&path_string,synfig::ValueBase value)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(get_param_tree_view().get_model()->get_iter(path));
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
	get_layer_tree_view().columns_autosize();
}

void
LayerTree::on_layer_toggle(const Glib::ustring& path_string)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(get_layer_tree_view().get_model()->get_iter(path));
	bool active=static_cast<bool>(row[layer_model.active]);
	row[layer_model.active]=!active;
}

#ifdef TIMETRACK_IN_PARAMS_PANEL
void
LayerTree::on_waypoint_clicked_layertree(const etl::handle<synfig::Node>& node __attribute__ ((unused)),
										 const synfig::Time& time __attribute__ ((unused)),
										 const synfig::Time& time_offset __attribute__ ((unused)),
										 const synfig::Time& time_dilation __attribute__ ((unused)),
										 int button __attribute__ ((unused)))
{
	std::set<synfig::Waypoint, std::less<UniqueID> > waypoint_set;
	synfig::waypoint_collect(waypoint_set,time,node);

	synfigapp::ValueDesc value_desc;

	if (waypoint_set.size() == 1)
	{
		ValueNode::Handle value_node(waypoint_set.begin()->get_parent_value_node());
		assert(value_node);

		Gtk::TreeRow row;
		if (param_tree_store_->find_first_value_node(value_node, row) && row)
			value_desc = static_cast<synfigapp::ValueDesc>(row[param_tree_store_->model.value_desc]);
	}

	if (!waypoint_set.empty())
		signal_waypoint_clicked_layertree()(value_desc,waypoint_set,button);
}
#endif	// TIMETRACK_IN_PARAMS_PANEL

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
			   )
				return signal_no_layer_user_click()(reinterpret_cast<GdkEventButton*>(event));
			const Gtk::TreeRow row = *(get_layer_tree_view().get_model()->get_iter(path));

#ifdef TIMETRACK_IN_PARAMS_PANEL
			if(column->get_first_cell()==cellrenderer_time_track)
				return signal_layer_user_click()(event->button.button,row,COLUMNID_TIME_TRACK);
			else
#endif	// TIMETRACK_IN_PARAMS_PANEL
			if(column->get_first_cell()==cellrenderer_value)
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

			//Gtk::TreeRow row = *(get_layer_tree_view().get_model()->get_iter(path));

#ifdef TIMETRACK_IN_PARAMS_PANEL
			if(cellrenderer_time_track==column->get_first_cell())
				// Movement on TimeLine
				return true;
			//else
#endif	// TIMETRACK_IN_PARAMS_PANEL
			//if(last_tooltip_path.get_depth()<=0 || path!=last_tooltip_path)
			//{
				//tooltips_.unset_tip(*this);
				//Glib::ustring tooltips_string(row[layer_model.tooltip]);
				//last_tooltip_path=path;
				//if(!tooltips_string.empty())
				//{
					//tooltips_.set_tip(*this,tooltips_string);
					//tooltips_.force_window();
				//}
			//}
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
			if(!get_param_tree_view().get_path_at_pos(
				int(event->button.x),int(event->button.y),	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;
			const Gtk::TreeRow row = *(get_param_tree_view().get_model()->get_iter(path));

#ifdef TIMETRACK_IN_PARAMS_PANEL
			if(column && column->get_first_cell()==cellrenderer_time_track)
			{
				Gdk::Rectangle rect;
				get_param_tree_view().get_cell_area(path,*column,rect);
				cellrenderer_time_track->property_value_desc()=row[param_model.value_desc];
				cellrenderer_time_track->property_canvas()=row[param_model.canvas];
				cellrenderer_time_track->activate(event,*this,path.to_string(),rect,rect,Gtk::CellRendererState());
				get_param_tree_view().queue_draw_area(rect.get_x(),rect.get_y(),rect.get_width(),rect.get_height());
				return true;
				//return signal_param_user_click()(event->button.button,row,COLUMNID_TIME_TRACK);
			}
			else
#endif	// TIMETRACK_IN_PARAMS_PANEL
			{
				if(event->button.button==3)
				{
					LayerList layer_list(get_selected_layers());
					if(layer_list.size()<=1)
					{
						synfigapp::ValueDesc value_desc(row[param_model.value_desc]);
						Gtk::Menu* menu(manage(new Gtk::Menu()));
						menu->signal_hide().connect(sigc::bind(sigc::ptr_fun(&delete_widget), menu));
						App::get_instance(param_tree_store_->canvas_interface()->get_canvas())->make_param_menu(menu,param_tree_store_->canvas_interface()->get_canvas(),value_desc,0.5f);
						menu->popup(event->button.button,gtk_get_current_event_time());
						return true;
					}
					Gtk::Menu* menu(manage(new Gtk::Menu()));
					menu->signal_hide().connect(sigc::bind(sigc::ptr_fun(&delete_widget), menu));
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
					if(column->get_first_cell()==cellrenderer_value)
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
			if(!get_param_tree_view().get_path_at_pos(
				(int)event->motion.x,(int)event->motion.y,	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;

			if(!get_param_tree_view().get_model()->get_iter(path))
				break;

			Gtk::TreeRow row = *(get_param_tree_view().get_model()->get_iter(path));

#ifdef TIMETRACK_IN_PARAMS_PANEL
			if(((event->motion.state&GDK_BUTTON1_MASK) || (event->motion.state&GDK_BUTTON3_MASK)) && column && cellrenderer_time_track==column->get_first_cell())
			{
				Gdk::Rectangle rect;
				get_param_tree_view().get_cell_area(path,*column,rect);
				cellrenderer_time_track->property_value_desc()=row[param_model.value_desc];
				cellrenderer_time_track->property_canvas()=row[param_model.canvas];
				cellrenderer_time_track->activate(event,*this,path.to_string(),rect,rect,Gtk::CellRendererState());
				get_param_tree_view().queue_draw();
				//get_param_tree_view().queue_draw_area(rect.get_x(),rect.get_y(),rect.get_width(),rect.get_height());
				return true;
			}
#endif	// TIMETRACK_IN_PARAMS_PANEL
		}
		break;
	case GDK_BUTTON_RELEASE:
		{
			Gtk::TreeModel::Path path;
			Gtk::TreeViewColumn *column;
			int cell_x, cell_y;
			if(!get_param_tree_view().get_path_at_pos(
				(int)event->button.x,(int)event->button.y,	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;

			if(!get_param_tree_view().get_model()->get_iter(path))
				break;

			Gtk::TreeRow row = *(get_param_tree_view().get_model()->get_iter(path));

#ifdef TIMETRACK_IN_PARAMS_PANEL
			if(column && cellrenderer_time_track==column->get_first_cell())
			{
				Gdk::Rectangle rect;
				get_param_tree_view().get_cell_area(path,*column,rect);
				cellrenderer_time_track->property_value_desc()=row[param_model.value_desc];
				cellrenderer_time_track->property_canvas()=row[param_model.canvas];
				cellrenderer_time_track->activate(event,*this,path.to_string(),rect,rect,Gtk::CellRendererState());
				get_param_tree_view().queue_draw();
				get_param_tree_view().queue_draw_area(rect.get_x(),rect.get_y(),rect.get_width(),rect.get_height());
				return true;

			}
#endif	// TIMETRACK_IN_PARAMS_PANEL
		}
		break;
	default:
		break;
	}
	return false;
}


bool
LayerTree::on_param_tree_view_query_tooltip(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip)
{
	if(keyboard_tooltip)
		return false;
	Gtk::TreeModel::Path path;
	Gtk::TreeViewColumn *column;
	int cell_x, cell_y;
	int bx, by;
	get_param_tree_view().convert_widget_to_bin_window_coords(x, y, bx, by);
	if(!get_param_tree_view().get_path_at_pos(bx, by, path, column, cell_x,cell_y))
		return false;
	Gtk::TreeIter iter(get_param_tree_view().get_model()->get_iter(path));
	if(!iter)
		return false;
	Gtk::TreeRow row = *(iter);
	Glib::ustring tooltip_string(row[param_model.tooltip]);
	if(tooltip_string.empty())
		return false;
	tooltip->set_text(tooltip_string);
	get_param_tree_view().set_tooltip_row(tooltip, path);
	return true;
}

bool
LayerTree::on_layer_tree_view_query_tooltip(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip)
{
	if(keyboard_tooltip)
		return false;
	Gtk::TreeModel::Path path;
	Gtk::TreeViewColumn *column;
	int cell_x, cell_y;
	int bx, by;
	get_layer_tree_view().convert_widget_to_bin_window_coords(x, y, bx, by);
	if(!get_layer_tree_view().get_path_at_pos(bx, by, path, column, cell_x,cell_y))
		return false;
	Gtk::TreeIter iter(get_param_tree_view().get_model()->get_iter(path));
	if(!iter)
		return false;
	Gtk::TreeRow row = *(iter);
	Glib::ustring tooltip_string(row[layer_model.tooltip]);
	if(tooltip_string.empty())
		return false;
	tooltip->set_text(tooltip_string);
	get_param_tree_view().set_tooltip_row(tooltip, path);
	return true;
}

void
LayerTree::on_param_column_label_tree_style_updated()
{
	param_tree_style_changed = true;
}

bool
LayerTree::on_param_column_label_tree_draw(const ::Cairo::RefPtr< ::Cairo::Context>& /*cr*/)
{
	if (param_tree_style_changed)
	{
		update_param_tree_header_height();
		param_tree_style_changed = false;
	}
	return true;
}

void
LayerTree::update_param_tree_header_height()
{
	int width = 0, height = 0;
	get_param_tree_view().convert_bin_window_to_widget_coords(0, 0, width, height);
	if (param_tree_header_height != height) {
		param_tree_header_height = height;
		Glib::signal_timeout().connect_once(
			sigc::bind( signal_param_tree_header_height_changed(), param_tree_header_height ),
			0 );
	}
}
