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
#include <gui/canvasview.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/trees/historytreestore.h>

#include <synfigapp/action.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */
#define COLUMNID_JUMP		(787584)
#define ColumnID	int

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dock_History::Dock_History():
	Dock_CanvasSpecific("history",_("History"),Gtk::StockID("synfig-history")),
	action_group(Gtk::ActionGroup::create("action_group_dock_history"))
{
	// Make History toolbar small for space efficiency
	get_style_context()->add_class("synfigstudio-efficient-workspace");

	App::signal_instance_deleted().connect(sigc::mem_fun(*this,&studio::Dock_History::delete_instance));
	App::signal_instance_selected().connect(sigc::mem_fun(*this,&studio::Dock_History::set_selected_instance_signal));

	action_group->add(Gtk::Action::create(
		"clear-undo",
		Gtk::StockID("synfig-clear_undo"),
		_("Clear Undo Stack"),
		_("Clear the undo stack")
	),
		sigc::mem_fun(
			*this,
			&Dock_History::clear_undo
		)
	);
	action_group->add(Gtk::Action::create(
		"clear-redo",
		Gtk::StockID("synfig-clear_redo"),
		_("Clear Redo Stack"),
		_("Clear the redo stack")
	),
		sigc::mem_fun(
			*this,
			&Dock_History::clear_redo
		)
	);
	action_group->add(Gtk::Action::create(
		"clear-undo-and-redo",
		Gtk::Stock::CLEAR,
		_("Clear Undo and Redo Stacks"),
		_("Clear the undo and redo stacks")
	),
		sigc::mem_fun(
			*this,
			&Dock_History::clear_undo_and_redo
		)
	);
	action_group->add(Gtk::Action::create(
		"undo",
		Gtk::StockID("synfig-undo"),
		_("Undo"),
		_("Undo the previous action")
	),
		sigc::ptr_fun(studio::App::undo)
	);
	App::instance()->add_action("undo", sigc::ptr_fun(studio::App::undo));
	action_group->add(Gtk::Action::create(
		"redo",
		Gtk::StockID("synfig-redo"),
		_("Redo"),
		_("Redo the previously undone action")
	),
		sigc::ptr_fun(studio::App::redo)
	);
	App::instance()->add_action("redo", sigc::ptr_fun(studio::App::redo));

	action_group->add( Gtk::Action::create("toolbar-history", _("History")) );
	App::ui_manager()->insert_action_group(action_group);

	Glib::ustring ui_info =
	"<ui>"
	"	<toolbar action='toolbar-history'>"
	"	<toolitem action='undo' />"
	"	<toolitem action='redo' />"
	"	<toolitem action='clear-undo' />"
	"	<toolitem action='clear-redo' />"
	"	<toolitem action='clear-undo-and-redo' />"
	"	</toolbar>"
	"</ui>"
	;

	App::ui_manager()->add_ui_from_string(ui_info);

	action_group->set_sensitive(false);
	auto action = App::instance()->lookup_action("undo");
	if (auto undo_action = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action))
		undo_action->set_enabled(false);
	action = App::instance()->lookup_action("redo");
	if (auto redo_action = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action))
		redo_action ->set_enabled(false);

	if (Gtk::Toolbar* toolbar = dynamic_cast<Gtk::Toolbar*>(App::ui_manager()->get_widget("/toolbar-history"))) {
		set_toolbar(*toolbar);
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
	etl::handle<Instance> instance=App::get_selected_instance();
	if(instance)
	{
		action_group->get_action("undo")->set_sensitive(instance->get_undo_status());
		action_group->get_action("clear-undo")->set_sensitive(instance->get_undo_status());
		action_group->get_action("redo")->set_sensitive(instance->get_redo_status());
		action_group->get_action("clear-redo")->set_sensitive(instance->get_redo_status());
		action_group->get_action("clear-undo-and-redo")->set_sensitive(instance->get_undo_status() || instance->get_redo_status());

		auto action = App::instance()->lookup_action("undo");
		if (auto undo_action = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action))
			undo_action->set_enabled(instance->get_undo_status());
		action = App::instance()->lookup_action("redo");
		if (auto redo_action = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action))
			redo_action ->set_enabled(instance->get_redo_status());
		//TODO: Need to make buttons in toolbar sensitive
		//This code is cuasing Assertion failed: (obj), function operator->, file _handle.h, line 740.
		/*
		if(auto *button = App::get_selected_canvas_view()->get_undo_button())
			button->set_sensitive(instance->get_undo_status());
		if(auto *button = App::get_selected_canvas_view()->get_redo_button())
			button->set_sensitive(instance->get_redo_status());
		*/
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
		action_group->set_sensitive(true);
	}
	else
	{
		action_tree->set_model(Glib::RefPtr< Gtk::TreeModel >());
		action_tree->hide();
		action_group->set_sensitive(false);
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
				etl::handle<synfigapp::Action::Undoable> action(row[model.action]);
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

	handle<synfigapp::Action::Undoable> action=row[history_tree_model.action];

	selected_instance->synfigapp::Instance::set_action_status(action,!action->is_active());
}
