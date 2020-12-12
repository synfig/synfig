/* === S Y N F I G ========================================================= */
/*!	\file dialogs/canvasproperties.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "dialogs/canvasproperties.h"
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/alignment.h>
#include <synfigapp/canvasinterface.h>
#include "trees/metadatatreestore.h"
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>
#include "app.h"

#include <gui/localization.h>
#include <synfigapp/action_system.h>
#include <synfigapp/instance.h>

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

CanvasProperties::CanvasProperties(Gtk::Window& parent,etl::handle<synfigapp::CanvasInterface> canvas_interface):
	Gtk::Dialog(_("Canvas Properties"),parent),
	canvas_interface_(canvas_interface)
{
	widget_rend_desc.show();
	widget_rend_desc.signal_changed().connect(sigc::mem_fun(*this,&studio::CanvasProperties::on_rend_desc_changed));

	Gtk::Alignment *dialogPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	dialogPadding->set_padding(12, 12, 12, 12);
	get_vbox()->pack_start(*dialogPadding, false, false, 0);

	Gtk::VBox *dialogBox = manage(new Gtk::VBox(false, 12));
	dialogPadding->add(*dialogBox);

	Gtk::Frame *info_frame=manage(new Gtk::Frame(_("Canvas Info")));
	info_frame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) info_frame->get_label_widget())->set_markup(_("<b>Canvas Info</b>"));
	dialogBox->pack_start(*info_frame, false, false, 0);

	Gtk::Alignment *infoPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	infoPadding->set_padding(6, 0, 24, 0);
	info_frame->add(*infoPadding);

	Gtk::Table *info_table=manage(new Gtk::Table(2,2,false));
	info_table->set_row_spacings(6);
	info_table->set_col_spacings(12);
	infoPadding->add(*info_table);

	// The root canvas doesn't have an ID, so don't
	// display it if this is a root canvas.
	if(!canvas_interface_->get_canvas()->is_root())
	{
		Gtk::Label *idLabel = manage(new Gtk::Label(_("_ID"), true));
		idLabel->set_alignment(0, 0.5);
		idLabel->set_mnemonic_widget(entry_id);
		info_table->attach(*idLabel, 0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
		info_table->attach(entry_id, 1, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	}
	Gtk::Label *nameLabel = manage(new Gtk::Label(_("_Name"), true));
	nameLabel->set_alignment(0, 0.5);
	nameLabel->set_mnemonic_widget(entry_name);
	Gtk::Label *descriptionLabel = manage(new Gtk::Label(_("_Description"), true));
	descriptionLabel->set_alignment(0, 0.5);
	descriptionLabel->set_mnemonic_widget(entry_description);
	info_table->attach(*nameLabel,        0, 1, 1, 2, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	info_table->attach(*descriptionLabel, 0, 1, 2, 3, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	info_table->attach(entry_name,        1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	info_table->attach(entry_description, 1, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);

	dialogBox->pack_start(widget_rend_desc, false, false, 0);

	canvas_interface_->signal_rend_desc_changed().connect(sigc::mem_fun(*this,&studio::CanvasProperties::refresh));
	canvas_interface_->signal_id_changed().connect(sigc::mem_fun(*this,&studio::CanvasProperties::refresh));

	Gtk::Button *apply_button(manage(new class Gtk::Button(Gtk::StockID("gtk-apply"))));
	apply_button->show();
	add_action_widget(*apply_button,1);
	apply_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasProperties::on_apply_pressed));

	Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-close"))));
	cancel_button->show();
	add_action_widget(*cancel_button,0);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasProperties::on_cancel_pressed));

	Gtk::Button *ok_button(manage(new class Gtk::Button(Gtk::StockID("gtk-ok"))));
	ok_button->show();
	add_action_widget(*ok_button,2);
	ok_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::CanvasProperties::on_ok_pressed));

	get_vbox()->show_all();
	refresh();

	update_title();
}

//Gtk::Widget&
//CanvasProperties::create_meta_data_view()
//{
	//MetaDataTreeStore::Model model;
	//meta_data_tree_view=(manage(new class Gtk::TreeView()));

	//meta_data_tree_view->append_column(_("Key"),model.key);
	//meta_data_tree_view->append_column_editable(_("Data"),model.data);
	//meta_data_tree_view->set_model(MetaDataTreeStore::create(canvas_interface_));
	//meta_data_tree_view->set_rules_hint();
	//meta_data_tree_view->show();

	//Gtk::ScrolledWindow *scrolledwindow = manage(new class Gtk::ScrolledWindow());
	//scrolledwindow->set_can_focus(true);
	//scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	//scrolledwindow->add(*meta_data_tree_view);
	//scrolledwindow->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	//scrolledwindow->show();



	//Gtk::Table *table=manage(new Gtk::Table());
	//table->attach(*scrolledwindow, 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	//Gtk::Button* button_add(manage(new Gtk::Button(Gtk::StockID("gtk-add"))));
	//button_add->show();
	//button_add->signal_clicked().connect(sigc::mem_fun(*this,&CanvasProperties::on_button_meta_data_add));
	//table->attach(*button_add, 0, 1, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);

	//Gtk::Button* button_delete(manage(new Gtk::Button(Gtk::StockID("gtk-delete"))));
	//button_delete->show();
	//button_delete->signal_clicked().connect(sigc::mem_fun(*this,&CanvasProperties::on_button_meta_data_delete));
	//table->attach(*button_delete, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);

	//table->show();
	//return *table;
//}

//void
//CanvasProperties::on_button_meta_data_add()
//{
	//synfig::String key;
	//if(App::dialog_entry(_("New MetaData Entry"), _("Please enter the name of the key"),key) && !key.empty())
	//{
		//canvas_interface_->set_meta_data(key," ");
	//}

//}

//void
//CanvasProperties::on_button_meta_data_delete()
//{
//}

void
CanvasProperties::update_title()
{
	set_title(_("Properties")+String(" - ")+canvas_interface_->get_canvas()->get_name());
}

void
CanvasProperties::refresh()
{
	widget_rend_desc.set_rend_desc(canvas_interface_->get_canvas()->rend_desc());
	entry_id.set_text(canvas_interface_->get_canvas()->get_id());
	entry_name.set_text(canvas_interface_->get_canvas()->get_name());
	entry_description.set_text(canvas_interface_->get_canvas()->get_description());

	dirty_rend_desc=false;

	update_title();
}

CanvasProperties::~CanvasProperties()
{
}

void
CanvasProperties::on_rend_desc_changed()
{
	dirty_rend_desc=true;
}

void
CanvasProperties::on_apply_pressed()
{
	synfigapp::Action::PassiveGrouper group(canvas_interface_->get_instance().get(),_("Edit Canvas Properties"));

	// fetch these three values first, because each set_() method refreshes the dialog with currently set values
	String id = entry_id.get_text();
	String name = entry_name.get_text();
	String description = entry_description.get_text();

	// do this first, because the other three cause the dialog to be refreshed with currently set values
	if (dirty_rend_desc) canvas_interface_->set_rend_desc(widget_rend_desc.get_rend_desc());

	if (id != canvas_interface_->get_canvas()->get_id() && !id.empty())		canvas_interface_->set_id(id);
	if (name != canvas_interface_->get_canvas()->get_name())				canvas_interface_->set_name(name);
	if (description != canvas_interface_->get_canvas()->get_description())	canvas_interface_->set_description(description);

	dirty_rend_desc=false;
}

void
CanvasProperties::on_ok_pressed()
{
	on_apply_pressed();
	hide();
}

void
CanvasProperties::on_cancel_pressed()
{
	refresh();
	hide();
}
