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
#include <gtkmm/stylecontext.h>

#include <gui/actionmanagers/actionmanager.h>
#include <gui/app.h>
#include <gui/canvasview.h>
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

static Gtk::ToolButton*
create_action_toolbutton(const std::string& action_name, const std::string& icon_name, const std::string& tooltip)
{
	Gtk::ToolButton* button = Gtk::manage(new Gtk::ToolButton());
	gtk_actionable_set_action_name(GTK_ACTIONABLE(button->gobj()), action_name.c_str());
	button->set_icon_name(icon_name);
	button->set_tooltip_text(tooltip);
	button->show();
	return button;
}

/* === M E T H O D S ======================================================= */

Dock_History::Dock_History():
	Dock_CanvasSpecific("history",_("History"),"history_icon")
{
	// Make History toolbar small for space efficiency
	get_style_context()->add_class("synfigstudio-efficient-workspace");

	App::signal_instance_deleted().connect(sigc::mem_fun(*this,&studio::Dock_History::delete_instance));
	App::signal_instance_selected().connect(sigc::mem_fun(*this,&studio::Dock_History::set_selected_instance_signal));

	auto toolbar = Gtk::manage(new Gtk::Toolbar());
	toolbar->show_all();
	toolbar->append(*create_action_toolbutton("doc.undo", "action_doc_undo_icon", _("Undo the previous action")));
	toolbar->append(*create_action_toolbutton("doc.redo", "action_doc_redo_icon", _("Redo the previously undone action")));
	toolbar->append(*create_action_toolbutton("doc.clear-undo", "clear_undo_icon", _("Clear the undo stack")));
	toolbar->append(*create_action_toolbutton("doc.clear-redo", "clear_redo_icon", _("Clear the redo stack")));
	toolbar->append(*create_action_toolbutton("doc.clear-undo-and-redo", "edit-clear", _("Clear the undo and redo stacks")));

	set_toolbar(*toolbar);

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

void
Dock_History::init_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
{
	auto action_group = Glib::RefPtr<Gio::SimpleActionGroup>::cast_dynamic(canvas_view->get_action_group("doc"));
	if (action_group) {
		auto action = action_group->add_action("clear-undo", sigc::mem_fun(*this, &Dock_History::clear_undo));
		action->set_enabled(false);
		action = action_group->add_action("clear-redo", sigc::mem_fun(*this, &Dock_History::clear_redo));
		action->set_enabled(false);
		action = action_group->add_action("clear-undo-and-redo", sigc::mem_fun(*this, &Dock_History::clear_undo_and_redo));
		action->set_enabled(false);
	}
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
	etl::handle<Instance> instance=App::get_selected_instance();
	if(instance)
	{
		auto action_group = Glib::RefPtr<Gio::SimpleActionGroup>::cast_dynamic(App::main_window->get_action_group("doc"));
		if (action_group) {
			action_group->lookup_action("undo")->set_property("enabled", instance->get_undo_status());
			action_group->lookup_action("clear-undo")->set_property("enabled", instance->get_undo_status());
			action_group->lookup_action("redo")->set_property("enabled", instance->get_redo_status());
			action_group->lookup_action("clear-redo")->set_property("enabled", instance->get_redo_status());
			action_group->lookup_action("clear-undo-and-redo")->set_property("enabled", instance->get_undo_status() || instance->get_redo_status());
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
