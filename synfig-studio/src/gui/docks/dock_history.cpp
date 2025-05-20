/* === S Y N F I G ========================================================= */
/*!	\file dock_history.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "docks/dock_history.h"

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>
#include <gtkmm/stylecontext.h>

#include <gui/app.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/trees/historytreestore.h>

#include <synfigapp/action.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */
#define COLUMNID_JUMP		(787584)
#define ColumnID	int

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dock_History::Dock_History():
 	Dock_CanvasSpecific("history",_("History"),"history_icon"),
    action_group(Gio::SimpleActionGroup::create())
{
    // Make History toolbar small for space efficiency
    get_style_context()->add_class("synfigstudio-efficient-workspace");

	App::signal_instance_deleted().connect(sigc::mem_fun(*this,&studio::Dock_History::delete_instance));
	App::signal_instance_selected().connect(sigc::mem_fun(*this,&studio::Dock_History::set_selected_instance_signal));

	action_group->add_action("undo", 
			[]() { studio::App::undo(); } 
	);
	action_group->add_action("redo", 
			[]() { studio::App::redo(); }
	);
	action_group->add_action("clear-undo", 
			sigc::mem_fun(*this, &Dock_History::clear_undo)
	);
	action_group->add_action("clear-redo", 
			sigc::mem_fun(*this, &Dock_History::clear_redo)
	);
	action_group->add_action("clear-undo-and-redo", 
			sigc::mem_fun(*this, &Dock_History::clear_undo_and_redo)
	);
	const char* ui_xml = R"(
	<?xml version="1.0" encoding="UTF-8"?>
	<interface>
	<object class="GtkBox" id="toolbar-history">
		<property name="orientation">horizontal</property>
		<property name="spacing">4</property>
		<property name="visible">true</property>

		<!-- Undo Button -->
		<child>
		<object class="GtkToolButton" id="undo-toolbutton">
			<property name="visible">true</property>
			<property name="action-name">action_group_dock_history.undo</property>
			<property name="tooltip-text" translatable="yes">Undo the previous action</property>
			<property name="icon-name">action_doc_undo_icon</property>
		</object>
		<packing>
			<property name="expand">false</property>
		</packing>
		</child>

		<!-- Redo Button -->
		<child>
		<object class="GtkToolButton" id="redo-toolbutton">
			<property name="visible">true</property>
			<property name="action-name">action_group_dock_history.redo</property>
			<property name="tooltip-text" translatable="yes">Redo the previously undone action</property>
			<property name="icon-name">action_doc_redo_icon</property>
		</object>
		<packing>
			<property name="expand">false</property>
		</packing>
		</child>

		<!-- Clear Undo Stack Button -->
		<child>
		<object class="GtkToolButton" id="clear-undo-toolbutton">
			<property name="visible">true</property>
			<property name="action-name">action_group_dock_history.clear-undo</property>
			<property name="tooltip-text" translatable="yes">Clear the undo stack</property>
			<property name="icon-name">clear_undo_icon</property>
		</object>
		<packing>
			<property name="expand">false</property>
		</packing>

		</child>
		<!-- Clear Redo Stack Button -->
		<child>
		<object class="GtkToolButton" id="clear-redo-toolbutton">
			<property name="visible">true</property>
			<property name="action-name">action_group_dock_history.clear-redo</property>
			<property name="tooltip-text" translatable="yes">Clear the redo stack</property>
			<property name="icon-name">clear_redo_icon</property>
		</object>
		<packing>
			<property name="expand">false</property>
		</packing>
		</child>

		<!-- Clear Undo and Redo Stacks Button -->
		<child>
		<object class="GtkToolButton" id="clear-undo-redo-toolbutton">
			<property name="visible">true</property>
			<property name="action-name">action_group_dock_history.clear-undo-and-redo</property>
			<property name="tooltip-text" translatable="yes">Clear the undo and redo stacks</property>
			<property name="icon-name">edit-clear</property>
		</object>
		<packing>
			<property name="expand">false</property>
		</packing>
		</child>

	</object>
	</interface>
		)"
	;
	
	App::ui_builder()->add_from_string(ui_xml);
	Gtk::Box* toolbar_box = nullptr;
	App::ui_builder()->get_widget("toolbar-history", toolbar_box);
	if (toolbar_box) {
		toolbar_box->get_style_context()->add_class("synfigstudio-efficient-workspace");
		toolbar_box->insert_action_group("action_group_dock_history", action_group);
		set_toolbar(*toolbar_box);
		App::set_history_action_group(action_group);
		// try to set size 16*16
		for (auto child : toolbar_box->get_children()) {
			if (auto tool_button = dynamic_cast<Gtk::ToolButton*>(child)) {			
				auto image = Gtk::make_managed<Gtk::Image>(tool_button->get_icon_name(), Gtk::ICON_SIZE_MENU);
				image->set_pixel_size(16);
				tool_button->set_icon_widget(*image);
				tool_button->show_all(); 
			}
		}
	}
	// Set initial sensitivity
	const std::vector<Glib::ustring> actions = {
		"undo", "redo", "clear-undo", "clear-redo", "clear-undo-and-redo"
	};
	
	for(const auto& action : actions) {
		action_group->action_enabled_changed(
			"action_group_dock_history." + action, false);
	}
    add(*create_action_tree());
}

Dock_History::~Dock_History()
{
}

void
Dock_History::init_instance_vfunc(etl::loose_handle<Instance> instance)
{
	instance->signal_undo_redo_status_changed().connect(
		sigc::mem_fun(*this,&Dock_History::update_undo_redo)
	);
}

Gtk::Widget*
Dock_History::create_action_tree()
{
	studio::HistoryTreeStore::Model history_tree_model;
	action_tree=manage(new class Gtk::TreeView());
	{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column("") );

		Gtk::CellRendererToggle* toggle_cr = Gtk::manage( new Gtk::CellRendererToggle() );
		toggle_cr->signal_toggled().connect(sigc::mem_fun(*this, &studio::Dock_History::on_action_toggle) );

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

	action_tree->set_enable_search(true);
	action_tree->set_search_column(history_tree_model.name);
	action_tree->set_search_equal_func(sigc::ptr_fun(&studio::HistoryTreeStore::search_func));

	action_tree->set_rules_hint();
//	action_tree->signal_row_activated().connect(sigc::mem_fun(*this,&Dock_History::on_row_activate));
	action_tree->signal_event().connect(sigc::mem_fun(*this,&Dock_History::on_action_event));
//	action_tree->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
//	action_tree->add_events(Gdk::BUTTON1_MOTION_MASK);
	action_tree->show();

	Gtk::ScrolledWindow *scrolledwindow = manage(new class Gtk::ScrolledWindow());
	scrolledwindow->set_can_focus(true);
	scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scrolledwindow->add(*action_tree);
	scrolledwindow->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	scrolledwindow->show_all();

/*	{
		Gtk::Widget& widget(*action_tree);
		Pango::FontDescription font(widget.get_modifier_style()->get_font());
		font.set_size(Pango::SCALE*5);
		widget.get_modifier_style()->set_font(font);
		widget.modify_font(font);
	}
*/
	return scrolledwindow;
}

void
Dock_History::clear_undo()
{
	if (selected_instance && App::dialog_message_2b(
		_("Clear History"),
		_("You will not be able to undo any changes that you have made! "
			"Are you sure you want to clear the undo stack?"),
		Gtk::MESSAGE_QUESTION,
		_("Cancel"),
		_("Clear"))
	)
		selected_instance->clear_undo_stack();
}

void
Dock_History::clear_redo()
{
	if(selected_instance && App::dialog_message_2b(
		_("Clear History"),
		_("You will not be able to redo any changes that you have made! "
			"Are you sure you want to clear the redo stack?"),
		Gtk::MESSAGE_QUESTION,
		_("Cancel"),
		_("Clear"))
	)
		selected_instance->clear_redo_stack();
}

void
Dock_History::clear_undo_and_redo()
{
	if(selected_instance && App::dialog_message_2b(
		_("Clear History"),
		_("You will not be able to undo or redo any changes that you have made! "
			"Are you sure you want to clear the undo and redo stacks?"),
		Gtk::MESSAGE_QUESTION,
		_("Cancel"),
		_("Clear"))
	)
	{
		selected_instance->clear_undo_stack();
		selected_instance->clear_redo_stack();
	}
}

void
Dock_History::update_undo_redo()
{
    etl::handle<Instance> instance = App::get_selected_instance();
    const bool has_instance = static_cast<bool>(instance);

    // Helper function to safely set action states
    auto set_action_state = [&](const char* action_name, bool state) {
        if (auto action = action_group->lookup_action(action_name)) {
            if (auto simple_action = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action)) {
                simple_action->set_enabled(has_instance && state);
            }
        }
    };

    if (instance) {
        set_action_state("undo", instance->get_undo_status());
        set_action_state("redo", instance->get_redo_status());
        set_action_state("clear-undo", instance->get_undo_status());
        set_action_state("clear-redo", instance->get_redo_status());
        set_action_state("clear-undo-and-redo", 
            instance->get_undo_status() || instance->get_redo_status());
    } else {
        // Explicitly disable all actions when no instance exists
        const char* actions[] = {
            "undo", "redo", "clear-undo", "clear-redo", "clear-undo-and-redo"
        };
        
        for (const auto& action : actions) {
            if (auto act = action_group->lookup_action(action)) {
                if (auto simple_act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(act)) {
                    simple_act->set_enabled(false);
                }
            }
        }
    }
}

void
Dock_History::on_undo_tree_changed()
{
	Gtk::TreeModel::Children children(selected_instance->history_tree_store()->children());

	if (!children.size())
		return;

	studio::HistoryTreeStore::Model model;

	Gtk::TreeModel::Children::iterator iter, prev = children.end();
	for (iter = children.begin(); iter != children.end(); prev = iter++)
		if ((*iter)[model.is_redo])
		{
			if (prev == children.end())
				action_tree->get_selection()->unselect_all();
			else
			{
				action_tree->scroll_to_row(Gtk::TreePath(prev), 0.5);
				action_tree->get_selection()->select(prev);
			}
			return;
		}

	action_tree->scroll_to_row(Gtk::TreePath(prev), 0.5);
	action_tree->get_selection()->select(prev);
}

void
Dock_History::set_selected_instance_(etl::handle<studio::Instance> instance)
{
    if(studio::App::shutdown_in_progress)
        return;

    if (on_undo_tree_changed_connection)
        on_undo_tree_changed_connection.disconnect();

    selected_instance=instance;
    if(instance)
    {
        on_undo_tree_changed_connection = selected_instance->history_tree_store()->signal_undo_tree_changed().connect(
            sigc::mem_fun(*this,&Dock_History::on_undo_tree_changed));

        action_tree->set_model(instance->history_tree_store());
        action_tree->show();
        update_undo_redo();
    }
    else
    {
        action_tree->set_model(Glib::RefPtr< Gtk::TreeModel >());
        action_tree->hide();
        
        // Disable all actions when no instance exists
        const std::vector<Glib::ustring> actions = {
            "undo", "redo", "clear-undo", "clear-redo", "clear-undo-and-redo"
        };
        
        for(const auto& action : actions) {
            if(auto act = action_group->lookup_action(action)) {
                if(auto simple_act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(act)) {
                    simple_act->set_enabled(false);
                }
            }
        }
    }
}

void
Dock_History::set_selected_instance_signal(etl::handle<studio::Instance> x)
{
	set_selected_instance(x);
}

void
Dock_History::set_selected_instance(etl::loose_handle<studio::Instance> x)
{
	if(studio::App::shutdown_in_progress)
		return;

	// if it's already selected, don't select it again
	if (x==selected_instance)
		return;

	set_selected_instance_(x);
}

void
Dock_History::delete_instance(etl::handle<studio::Instance> instance)
{
	if(studio::App::shutdown_in_progress)
		return;

	if(selected_instance==instance)
	{
		set_selected_instance(0);
	}
}

bool
Dock_History::on_action_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
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
				synfigapp::Action::Undoable::Handle action(row[model.action]);
				try{
				if((bool)row[model.is_undo])
				{
					while(get_selected_instance()->undo_action_stack().size() && get_selected_instance()->undo_action_stack().front()!=action)
						if(get_selected_instance()->undo()==false)
							throw int();
				}
				else if((bool)row[model.is_redo])
				{
					while(get_selected_instance()->redo_action_stack().size() && get_selected_instance()->undo_action_stack().front()!=action)
						if(get_selected_instance()->redo()==false)
							throw int();
				}
				}
				catch(int)
				{
					return true;
				}
			}
			break;
		}

	case GDK_BUTTON_RELEASE:
		break;
	default:
		break;
	}
	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

void
Dock_History::on_action_toggle(const Glib::ustring& path_string)
{
	studio::HistoryTreeStore::Model history_tree_model;

	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(selected_instance->history_tree_store()->get_iter(path));

	synfigapp::Action::Undoable::Handle action = row[history_tree_model.action];

	selected_instance->set_action_status(action, !action->is_active());
}
