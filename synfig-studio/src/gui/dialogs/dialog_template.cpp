/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_template.cpp
**	\brief Dialog design list and panel template Implementation
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2016 Jerome Blanchi
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

#include "dialog_template.h"

#include <gtkmm/eventbox.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */
using namespace synfig;
using namespace std;
using namespace studio;

/* === M A C R O S ========================================================= */
// TODO Group All HARDCODED user interface information somewhere "global"
// TODO All UI info from .rc
#define DIALOG_TEMPLATE_UI_INIT_GRID(grid) 					\
		grid->set_orientation(Gtk::ORIENTATION_HORIZONTAL);		\
		grid->set_row_spacing(6);								\
		grid->set_column_spacing(12);							\
		grid->set_border_width(8);								\
		grid->set_column_homogeneous(false);

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */
Dialog_Template::Dialog_Template(Gtk::Window& parent, synfig::String dialog_title):
	Dialog(dialog_title.c_str(),parent,true),
	page_index(0)
{
		// Setup the buttons
	Gtk::Button *restore_button(manage(new class Gtk::Button(_("Restore Defaults"))));
	restore_button->show();
	add_action_widget(*restore_button,1);
	restore_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Template::on_restore_pressed));

	Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-cancel"))));
	cancel_button->show();
	add_action_widget(*cancel_button,0);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Template::hide));

	Gtk::Button *ok_button(manage(new class Gtk::Button(Gtk::StockID("gtk-ok"))));
	ok_button->show();
	add_action_widget(*ok_button,2);
	ok_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Template::on_ok_pressed));


	//! TODO Make global this design to being used else where in other dialogs
	//! TODO UI design information to .rc
	// Style for title and section
	Pango::AttrInt attr = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	//! Nota section_attrlist also use for BOLDING " X " string in document page
	section_attrlist.insert(attr);
	title_attrlist.insert(attr);
	Pango::AttrInt pango_size(Pango::Attribute::create_attr_size(Pango::SCALE*26));
	title_attrlist.change(pango_size);
	// create negative foreground/background attributes
	Gdk::RGBA colorcontext(get_style_context()->get_color());
	Gdk::RGBA bgcolorcontext(get_style_context()->get_background_color());
	Pango::AttrColor bgcolor = Pango::Attribute::create_attr_background(colorcontext.get_red_u(),
			colorcontext.get_green_u(),
			colorcontext.get_blue_u()
			);
	title_attrlist.change(bgcolor);
	Pango::AttrColor color = Pango::Attribute::create_attr_foreground(bgcolorcontext.get_red_u(),
			bgcolorcontext.get_green_u(),
			bgcolorcontext.get_blue_u()
			);
	title_attrlist.change(color);

	// Notebook
	notebook=manage(new class Gtk::Notebook());
	// Main preferences notebook
	notebook->set_show_tabs (false);
	notebook->set_show_border (false);

	{
		// WARNING FIXED ORDER : the page added to notebook same has treeview (see create_xxxx_page() upper)
		categories_reftreemodel = Gtk::TreeStore::create(categories);
		categories_treeview.set_model(categories_reftreemodel);

		categories_treeview.set_headers_visible(false);
		categories_treeview.append_column(_("Category"), categories.category_name);

		categories_treeview.get_selection()->signal_changed().connect(
				sigc::mem_fun(*this, &Dialog_Template::on_treeviewselection_changed));

		categories_scrolledwindow.add(categories_treeview);
		categories_scrolledwindow.set_size_request(-1, 80);
		categories_scrolledwindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	}

	main_grid.attach(categories_scrolledwindow, 0, 0, 1 ,1);
	notebook->show_all();

	main_grid.attach(*notebook, 1, 0, 1, 1);
	main_grid.set_border_width(6);

	//! TODO create a warning zone to push message on rare events (like no brush path, zero fps ...)
	//! this warning zone could also hold normal message like : "x change to come"  (and a link to "Changes summary" page)

	get_vbox()->pack_start(main_grid);
	get_vbox()->set_border_width(12);

	show_all_children();
}

Dialog_Template::~Dialog_Template()
{
}


void
Dialog_Template::on_ok_pressed()
{
	on_apply_pressed();
	hide();
}

void
Dialog_Template::on_treeviewselection_changed()
{
	if(const Gtk::TreeModel::iterator iter = categories_treeview.get_selection()->get_selected())
	{
		notebook->set_current_page((int) ((*iter)[categories.category_id]));
	}
}

void
Dialog_Template::attach_label(Gtk::Grid *grid, synfig::String str, guint row)
{
	Gtk::Label* label(manage(new Gtk::Label((str + ":").c_str())));
	label->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	label->set_margin_start(10);
	grid->attach(*label, 0, row, 1, 1);
}

void
Dialog_Template::attach_label_section(Gtk::Grid *grid, synfig::String str, guint row)
{
	Gtk::Label* label(manage(new Gtk::Label(str)));
	label->set_attributes(section_attrlist);
	label->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	grid->attach(*label, 0, row, 1, 1);
}

void
Dialog_Template::attach_label_title(Gtk::Grid *grid, synfig::String str)
{
	Gtk::Label* label(manage(new Gtk::Label(str)));
	label->set_attributes(title_attrlist);
	label->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	label->set_margin_start(20);
	Gtk::EventBox* box(manage(new Gtk::EventBox()));
	box->add(*label);
	box->override_background_color(get_style_context()->get_color());
	grid->attach(*box, 0, 0, 1, 1);
}

Gtk::Label*
Dialog_Template::attach_label(Gtk::Grid *grid, synfig::String str, guint row, guint col, bool endstring)
{
	str = endstring?str+":":str;
	Gtk::Label* label(manage(new Gtk::Label(str)));
	label->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	grid->attach(*label, col, row, 1, 1);
	return label;
}

Dialog_Template::PageInfo
Dialog_Template::add_page(synfig::String page_title)
{
	Dialog_Template::PageInfo pageinfo;

	pageinfo.grid=manage(new Gtk::Grid());
	Gtk::Grid *page_grid=manage(new Gtk::Grid());
	DIALOG_TEMPLATE_UI_INIT_GRID(pageinfo.grid);
	notebook->append_page(*page_grid,page_title);
	attach_label_title(page_grid,page_title);
	page_grid->attach(*(pageinfo.grid), 0,1,1,1);

	pageinfo.row = *(categories_reftreemodel->append());
	pageinfo.row[categories.category_id] = page_index++;
	pageinfo.row[categories.category_name] = page_title;

	return pageinfo;
}

Dialog_Template::PageInfo
Dialog_Template::add_child_page(synfig::String page_title, Gtk::TreeRow parentrow)
{
	Dialog_Template::PageInfo pageinfo;

	pageinfo.grid=manage(new Gtk::Grid());
	Gtk::Grid *page_grid=manage(new Gtk::Grid());
	DIALOG_TEMPLATE_UI_INIT_GRID(pageinfo.grid);
	notebook->append_page(*page_grid,page_title);
	attach_label_title(page_grid,page_title);
	page_grid->attach(*(pageinfo.grid), 0,1,1,1);

	pageinfo.row = *(categories_reftreemodel->append(parentrow.children()));
	pageinfo.row[categories.category_id] = page_index++;
	pageinfo.row[categories.category_name] = page_title;

	categories_treeview.expand_all();

	return pageinfo;
}
