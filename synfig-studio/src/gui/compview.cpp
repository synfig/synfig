/* === S Y N F I G ========================================================= */
/*!	\file compview.cpp
**	\brief writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "compview.h"
#include "app.h"
#include <gtkmm/scrolledwindow.h>
#include <cassert>
#include <iostream>
#include "instance.h"
#include <sigc++/signal.h>
#include <sigc++/hide.h>
#include <sigc++/slot.h>
#include "canvasview.h"
#include <synfigapp/action.h>

#include "general.h"

#endif

/* === M A C R O S ========================================================= */

#define ADD_TOOLBOX_BUTTON(button,stockid,tooltip)	\
	Gtk::Button *button = manage(new class Gtk::Button());	\
	button->add(*manage(new Gtk::Image(Gtk::StockID(stockid),Gtk::IconSize(4))));	\
	tooltip.set_tip(*button,tooltip);	\
	button->show_all()

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

#define COLUMNID_JUMP		(787584)
#define ColumnID	int

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

CompView::CompView():
	Gtk::Window(Gtk::WINDOW_TOPLEVEL),
	dialog_settings(this,"compview")
{
	assert(0); //CHECK: This class does not appear to be used.
	init_menu();
	set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);

	Gtk::Table *table = manage(new class Gtk::Table(2, 1, false));

	instance_selector.show();
	instance_selector.signal_changed().connect(sigc::mem_fun(this, &CompView::on_instance_selector_changed));

	table->attach(instance_selector, 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK, 0, 0);

	notebook=manage(new class Gtk::Notebook());

	table->attach(*notebook, 0, 1, 1,2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	notebook->append_page(*create_canvas_tree(),_("Canvases"));
	notebook->append_page(*create_action_tree(),_("History"));



/*

	studio::Instance::ImageColumnModel image_column_model;
	image_list=manage(new class Gtk::TreeView());
	image_list->append_column(_("Name"),image_column_model.name);
	image_list->signal_row_activated().connect(sigc::mem_fun(*this,&CompView::on_image_activate));
	image_list->set_rules_hint();

	Gtk::Table *image_page = manage(new class Gtk::Table(2, 1, false));
	Gtk::ScrolledWindow *image_list_scroll = manage(new class Gtk::ScrolledWindow());
	image_list_scroll->set_can_focus(true);
	image_list_scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	image_list_scroll->add(*image_list);
	image_page->attach(*image_list_scroll, 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	Gtk::HBox *image_buttons=manage(new class Gtk::HBox());
	image_page->attach(*image_buttons, 0, 1, 1,2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK, 0, 0);
	ADD_TOOLBOX_BUTTON(button_image_new,"gtk-new",_("Create a new image"));
	ADD_TOOLBOX_BUTTON(button_image_delete,"gtk-delete",_("Delete image"));
	ADD_TOOLBOX_BUTTON(button_image_rename,"gtk-rename",_("Rename image"));
	ADD_TOOLBOX_BUTTON(button_image_copy,"gtk-copy",_("Duplicate image"));
	button_image_new->signal_clicked().connect(sigc::mem_fun(*this,&CompView::new_image));
	button_image_delete->signal_clicked().connect(sigc::mem_fun(*this,&CompView::delete_image));
	button_image_rename->signal_clicked().connect(sigc::mem_fun(*this,&CompView::rename_image));
	button_image_copy->signal_clicked().connect(sigc::mem_fun(*this,&CompView::copy_image));
	image_buttons->pack_start(*button_image_new);
	image_buttons->pack_start(*button_image_delete);
	image_buttons->pack_start(*button_image_rename);
	image_buttons->pack_start(*button_image_copy);

	studio::Instance::ValueNodeColumnModel valuenode_column_model;
	valuenode_list=manage(new class Gtk::TreeView());
	valuenode_list->append_column(_("Name"),valuenode_column_model.name);
	valuenode_list->signal_row_activated().connect(sigc::mem_fun(*this,&CompView::on_valuenode_activate));
	valuenode_list->set_rules_hint();

	Gtk::Table *valuenode_page = manage(new class Gtk::Table(2, 1, false));
	Gtk::ScrolledWindow *valuenode_list_scroll = manage(new class Gtk::ScrolledWindow());
	valuenode_list_scroll->set_can_focus(true);
	valuenode_list_scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	valuenode_list_scroll->add(*valuenode_list);
	valuenode_page->attach(*valuenode_list_scroll, 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	Gtk::HBox *valuenode_buttons=manage(new class Gtk::HBox());
	valuenode_page->attach(*valuenode_buttons, 0, 1, 1,2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK, 0, 0);
	ADD_TOOLBOX_BUTTON(button_valuenode_new,"gtk-new",_("Create a new value_node"));
	ADD_TOOLBOX_BUTTON(button_valuenode_delete,"gtk-delete",_("Delete value_node"));
	ADD_TOOLBOX_BUTTON(button_valuenode_rename,"gtk-rename",_("Rename value_node"));
	ADD_TOOLBOX_BUTTON(button_valuenode_copy,"gtk-copy",_("Duplicate value_node"));
	button_valuenode_new->signal_clicked().connect(sigc::mem_fun(*this,&CompView::new_value_node));
	button_valuenode_delete->signal_clicked().connect(sigc::mem_fun(*this,&CompView::delete_value_node));
	button_valuenode_rename->signal_clicked().connect(sigc::mem_fun(*this,&CompView::rename_value_node));
	button_valuenode_copy->signal_clicked().connect(sigc::mem_fun(*this,&CompView::copy_value_node));
	valuenode_buttons->pack_start(*button_valuenode_new);
	valuenode_buttons->pack_start(*button_valuenode_delete);
	valuenode_buttons->pack_start(*button_valuenode_rename);
	valuenode_buttons->pack_start(*button_valuenode_copy);


	notebook->append_page(*image_page,_("Images"));
	notebook->append_page(*valuenode_page,_("ValueNodes"));

	image_page->show_all();
	valuenode_page->show_all();
*/
//	notebook->set_current_page(0);
	signal_delete_event().connect(sigc::hide(sigc::mem_fun(*this, &CompView::close)));
	App::signal_instance_created().connect(sigc::mem_fun(*this,&studio::CompView::new_instance));
	App::signal_instance_deleted().connect(sigc::mem_fun(*this,&studio::CompView::delete_instance));
	App::signal_instance_selected().connect(sigc::mem_fun(*this,&studio::CompView::set_selected_instance_signal));

	table->show_all();
	add(*table);


	set_title(_("Canvas Browser"));
	set_modal(false);
	set_resizable(true);
	property_window_position().set_value(Gtk::WIN_POS_NONE);
	set_default_size(200,300);
}

CompView::~CompView()
{
}

etl::loose_handle<studio::CanvasView>
CompView::get_selected_canvas_view()
{
	return get_selected_instance()->find_canvas_view(get_selected_canvas());
}

Gtk::Widget*
CompView::create_canvas_tree()
{
	studio::Instance::CanvasTreeModel canvas_tree_model;
	canvas_tree=manage(new class Gtk::TreeView());
	{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("ID")) );
//		Gtk::CellRendererPixbuf* icon_cellrenderer = Gtk::manage( new Gtk::CellRendererPixbuf() );

		//column->pack_start(*icon_cellrenderer,false);
		column->pack_start(canvas_tree_model.icon, false); //false = don't expand.
		column->pack_start(canvas_tree_model.label);

//#ifdef NDEBUG
//		column->add_attribute(icon_cellrenderer->property_pixbuf(), canvas_tree_model.icon);
//#endif

		canvas_tree->append_column(*column);
	}
	canvas_tree->set_rules_hint();
	canvas_tree->signal_row_activated().connect(sigc::mem_fun(*this,&CompView::on_row_activate));
	canvas_tree->signal_event().connect(sigc::mem_fun(*this,&CompView::on_tree_event));
	canvas_tree->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	canvas_tree->add_events(Gdk::BUTTON1_MOTION_MASK);
	canvas_tree->show();

	Gtk::ScrolledWindow *scrolledwindow = manage(new class Gtk::ScrolledWindow());
	scrolledwindow->set_can_focus(true);
	scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scrolledwindow->add(*canvas_tree);
	scrolledwindow->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	scrolledwindow->show();

	return scrolledwindow;
}

Gtk::Widget*
CompView::create_action_tree()
{
	studio::HistoryTreeStore::Model history_tree_model;
	action_tree=manage(new class Gtk::TreeView());
	{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column("") );

		Gtk::CellRendererToggle* toggle_cr = Gtk::manage( new Gtk::CellRendererToggle() );
		toggle_cr->signal_toggled().connect(sigc::mem_fun(*this, &studio::CompView::on_action_toggle) );

		column->pack_start(*toggle_cr); //false = don't expand.
		column->add_attribute(toggle_cr->property_active(),history_tree_model.is_active);
		column->set_resizable();
		column->set_clickable();

		action_tree->append_column(*column);
	}
	/*{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Canvas")) );
		Gtk::CellRendererText *text_cr=Gtk::manage(new Gtk::CellRendererText());
		text_cr->property_foreground()=Glib::ustring("#7f7f7f");

		column->pack_start(*text_cr);
		column->add_attribute(text_cr->property_text(),history_tree_model.canvas_id);
		column->add_attribute(text_cr->property_foreground_set(),history_tree_model.is_redo);

		action_tree->append_column(*column);
	}*/
	{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Jump")) );

		Gtk::CellRendererText* cell_renderer_jump=Gtk::manage(new Gtk::CellRendererText());
		column->pack_start(*cell_renderer_jump,true);

		cell_renderer_jump->property_text()=_("(JMP)");
		cell_renderer_jump->property_foreground()="#003a7f";

		column->set_resizable();
		column->set_clickable();

		column->set_sort_column(COLUMNID_JUMP);

		action_tree->append_column(*column);
		//column->clicked();
	}
	{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Action")) );

		Gtk::CellRendererText *text_cr=Gtk::manage(new Gtk::CellRendererText());
		text_cr->property_foreground()=Glib::ustring("#7f7f7f");



		//column->pack_start(history_tree_model.icon, false); //false = don't expand.
		column->pack_start(*text_cr);
		column->add_attribute(text_cr->property_text(),history_tree_model.name);
		column->add_attribute(text_cr->property_foreground_set(),history_tree_model.is_redo);

		action_tree->append_column(*column);
	}


	action_tree->set_rules_hint();
//	action_tree->signal_row_activated().connect(sigc::mem_fun(*this,&CompView::on_row_activate));
	action_tree->signal_event().connect(sigc::mem_fun(*this,&CompView::on_action_event));
//	action_tree->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
//	action_tree->add_events(Gdk::BUTTON1_MOTION_MASK);
	action_tree->show();

	Gtk::ScrolledWindow *scrolledwindow = manage(new class Gtk::ScrolledWindow());
	scrolledwindow->set_can_focus(true);
	scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scrolledwindow->add(*action_tree);
	scrolledwindow->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	scrolledwindow->show();

	Gtk::Button* clear_button(manage(new Gtk::Button(_("Clear Undo"))));
	clear_button->signal_pressed().connect(sigc::mem_fun(*this,&studio::CompView::clear_history));

	Gtk::Button* clear_redo_button(manage(new Gtk::Button(_("Clear Redo"))));
	clear_redo_button->signal_pressed().connect(sigc::mem_fun(*this,&studio::CompView::clear_redo));

	Gtk::Table* table(manage(new Gtk::Table()));
	table->attach(*scrolledwindow, 0, 2, 0,1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	table->attach(*clear_button, 0, 1, 1,2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK, 0, 0);
	table->attach(*clear_redo_button, 1, 2, 1,2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK, 0, 0);

	table->show_all();

	return table;
}

bool
CompView::close()
{
	hide();
	return true;
}

void
CompView::clear_history()
{

	if (selected_instance && App::dialog_message_b2(
			_("Clear History"),
			_("You will not be able to undo any changes that you have made! "
			"Are you sure you want to clear the undo stack?")
			Gtk::MESSAGE_QUESTION,
			_("Cancel"),
			_("Clear"))
	)
		selected_instance->clear_undo_stack();
}


void
CompView::clear_redo()
{
	if (selected_instance && App::dialog_message_2b(
			_("Clear History"),
			_("You will not be able to redo any changes that you have made! "
			"Are you sure you want to clear the redo stack?")
			Gtk::MESSAGE_QUESTION,
			_("Cancel"),
			_("Clear"))
	)
		selected_instance->clear_redo_stack();
}

void
CompView::init_menu()
{
	Gtk::MenuItem *item = NULL;

	item = manage(new Gtk::SeparatorMenuItem());
	item->show();
	menu.append(*item);

	item = manage(new Gtk::ImageMenuItem(Gtk::StockID("synfig-canvas_new")));
	item->signal_activate().connect(
		sigc::mem_fun(*this,&CompView::menu_new_canvas));
	item->show_all();
	menu.append(*item);

	item = manage(new Gtk::ImageMenuItem(Gtk::StockID("gtk-delete")));
	item->signal_activate().connect(
		sigc::mem_fun(*this,&CompView::menu_delete));
	item->show_all();
	menu.append(*item);

	item = manage(new Gtk::ImageMenuItem(Gtk::StockID("synfig-rename")));
	item->signal_activate().connect(
		sigc::mem_fun(*this,&CompView::menu_rename));
	item->show_all();
	menu.append(*item);
}

etl::loose_handle<synfig::Canvas>
CompView::get_selected_canvas()
{
	Glib::RefPtr<Gtk::TreeSelection> selection=canvas_tree->get_selection();

	if(!selection || !selection->get_selected())
		return 0;

	studio::Instance::CanvasTreeModel canvas_tree_model;

	return static_cast<etl::handle<synfig::Canvas> >((*selection->get_selected())[canvas_tree_model.canvas]);
}

void
CompView::menu_new_canvas()
{
#warning Update Me!
#if 0
	get_selected_canvas_view()->new_child_canvas();
#endif
}

void
CompView::menu_delete()
{
	studio::App::dialog_not_implemented();
}

void
CompView::menu_rename()
{
	studio::App::dialog_not_implemented();
}

void
CompView::set_selected_instance_signal(etl::handle<studio::Instance> x)
{
	set_selected_instance(x);
}

void
CompView::set_selected_instance_(etl::handle<studio::Instance> instance)
{
	if(studio::App::shutdown_in_progress)
		return;

	selected_instance=instance;
	if(instance)
	{
		canvas_tree->set_model(instance->canvas_tree_store());
		action_tree->set_model(instance->history_tree_store());
		canvas_tree->show();
		action_tree->show();
	}
	else
	{
		canvas_tree->set_model(Glib::RefPtr< Gtk::TreeModel >());
		action_tree->set_model(Glib::RefPtr< Gtk::TreeModel >());
		canvas_tree->hide();
		action_tree->hide();
	}
}

void
CompView::on_instance_selector_changed()
{
	int i = instance_selector.get_active_row_number();
	if (i < 0 || i >= (int)instances.size()) return;
	if (selected_instance == instances[i]) return;
	studio::App::set_selected_instance(instances[i]);
}

void
CompView::set_selected_instance(etl::loose_handle<studio::Instance> x)
{
	if(studio::App::shutdown_in_progress)
		return;

	// if it's already selected, don't select it again
	if (x==selected_instance)
		return;

	std::list<etl::handle<studio::Instance> >::iterator iter;

	if(x)
	{
		int i;
		for(i=0,iter=studio::App::instance_list.begin();iter!=studio::App::instance_list.end() && ((*iter)!=x);iter++,i++);

		assert(*iter==x);

		instance_selector.set_active(i);
	}
	else
		instance_selector.set_active(0);

	set_selected_instance_(x);
}

void
CompView::new_instance(etl::handle<studio::Instance> instance)
{
	if(studio::App::shutdown_in_progress)
		return;

	assert(instance);

	etl::loose_handle<studio::Instance> loose_instance(instance);

	instance->synfigapp::Instance::signal_filename_changed().connect(sigc::mem_fun(*this,&CompView::refresh_instances));
	instance->synfigapp::Instance::signal_filename_changed().connect(
		sigc::bind<etl::loose_handle<studio::Instance> >(
			sigc::mem_fun(*this,&CompView::set_selected_instance),
			loose_instance
		)
	);

	{
		std::string name=basename(instance->get_file_name());
		instance_selector.append(name);
		instances.push_back(loose_instance);
	}
}

void
CompView::delete_instance(etl::handle<studio::Instance> instance)
{
	if(studio::App::shutdown_in_progress)
		return;

	refresh_instances();

	if(selected_instance==instance)
	{
		set_selected_instance(0);
		instance_selector.set_active(0);
	}
}

void
CompView::refresh_instances()
{
	if(studio::App::shutdown_in_progress)
		return;

	instances.clear();
	instance_selector.set_active(-1);
	instance_selector.remove_all();

	std::list<etl::handle<studio::Instance> >::iterator iter;
	for(iter=studio::App::instance_list.begin();iter!=studio::App::instance_list.end();iter++)
	{
		std::string name=basename((*iter)->get_file_name());
		instance_selector.append(name);
		instances.push_back( etl::loose_handle<studio::Instance>(*iter) );
	}
}

void
CompView::on_row_activate(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *)
{
	assert(get_selected_instance());
	studio::Instance::CanvasTreeModel canvas_tree_model;
	const Gtk::TreeRow row = *(get_selected_instance()->canvas_tree_store()->get_iter(path));
	if(row[canvas_tree_model.is_canvas])
		get_selected_instance()->focus(row[canvas_tree_model.canvas]);
	else
		studio::App::dialog_not_implemented();
}

bool
CompView::on_action_event(GdkEvent *event)
{
	studio::HistoryTreeStore::Model model;
    switch(event->type)
    {
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
		{
			Gtk::TreeModel::Path path;
			Gtk::TreeViewColumn *column;
			int cell_x, cell_y;
			if(!action_tree->get_path_at_pos(
				int(event->button.x),int(event->button.y),	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;
			const Gtk::TreeRow row = *(action_tree->get_model()->get_iter(path));

			//signal_user_click()(event->button.button,row,(ColumnID)column->get_sort_column_id());
			if((ColumnID)column->get_sort_column_id()==COLUMNID_JUMP)
			{
				etl::handle<synfigapp::Action::Undoable> action(row[model.action]);
				if((bool)row[model.is_undo])
				{
					while(get_selected_instance()->undo_action_stack().size() && get_selected_instance()->undo_action_stack().front()!=action)
						get_selected_instance()->undo();
				}
				else if((bool)row[model.is_redo])
				{
					while(get_selected_instance()->redo_action_stack().size() && get_selected_instance()->undo_action_stack().front()!=action)
						get_selected_instance()->redo();
				}
			}
			break;
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
CompView::on_tree_event(GdkEvent *event)
{
    switch(event->type)
    {
	case GDK_BUTTON_PRESS:
		switch(event->button.button)
		{
		case 3:
		if(get_selected_canvas())
		{
			std::vector<Gtk::Widget*> children = menu.get_children();
			for(std::vector<Gtk::Widget*>::iterator i = children.begin(); i != children.end(); ++i)
				menu.remove(**i);

			synfigapp::Action::ParamList param_list;
			param_list.add("canvas",synfig::Canvas::Handle(get_selected_canvas()));
			param_list.add("canvas_interface",get_selected_instance()->find_canvas_interface(get_selected_canvas()));
			get_selected_instance()->find_canvas_view(get_selected_canvas())->add_actions_to_menu(&menu, param_list,synfigapp::Action::CATEGORY_CANVAS);
			menu.popup(0,0);
			menu.show();
		}
		break;
		default:
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
CompView::on_action_toggle(const Glib::ustring& path_string)
{
	studio::HistoryTreeStore::Model history_tree_model;

	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(selected_instance->history_tree_store()->get_iter(path));

	handle<synfigapp::Action::Undoable> action=row[history_tree_model.action];

	selected_instance->synfigapp::Instance::set_action_status(action,!action->is_active());
}
