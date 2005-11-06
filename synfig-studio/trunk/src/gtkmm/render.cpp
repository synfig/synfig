/* === S Y N F I G ========================================================= */
/*!	\file render.cpp
**	\brief Template File
**
**	$Id: render.cpp,v 1.2 2005/01/10 08:13:44 darco Exp $
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

#include "render.h"
#include "app.h"
#include <gtkmm/frame.h>
#include <synfig/target_scanline.h>
#include <synfig/canvas.h>
#include "asyncrenderer.h"

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

RenderSettings::RenderSettings(Gtk::Window& parent,handle<synfigapp::CanvasInterface> canvas_interface):
	Gtk::Dialog(_("Render Settings"),parent,false,true),
	canvas_interface_(canvas_interface),
	adjustment_quality(3,0,9),
	entry_quality(adjustment_quality,1,0),
	adjustment_antialias(1,1,31),
	entry_antialias(adjustment_antialias,1,0),
	toggle_single_frame(_("Use Current Frame"))
{
	widget_rend_desc.show();
	widget_rend_desc.signal_changed().connect(sigc::mem_fun(*this,&studio::RenderSettings::on_rend_desc_changed));
	widget_rend_desc.set_rend_desc(canvas_interface_->get_canvas()->rend_desc());
	
	canvas_interface->signal_rend_desc_changed().connect(sigc::mem_fun(*this,&RenderSettings::on_rend_desc_changed));
	
	menu_target=manage(new class Gtk::Menu());

	menu_target->items().push_back(Gtk::Menu_Helpers::MenuElem(_("Auto"),
			sigc::bind(sigc::mem_fun(*this,&RenderSettings::set_target),String())
		));

	synfig::Target::Book::iterator iter;
	synfig::Target::Book book(synfig::Target::book());
	
	for(iter=book.begin();iter!=book.end();iter++)
	{
		menu_target->items().push_back(Gtk::Menu_Helpers::MenuElem(iter->first,
			sigc::bind(sigc::mem_fun(*this,&RenderSettings::set_target),iter->first)
		));
	}
	optionmenu_target.set_menu(*menu_target);

	optionmenu_target.set_history(0);
	
	
	
	

	Gtk::Button *choose_button(manage(new class Gtk::Button(Gtk::StockID(_("Choose...")))));
	choose_button->show();
	choose_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::RenderSettings::on_choose_pressed));
	
	Gtk::Frame *target_frame=manage(new Gtk::Frame(_("Target")));
	Gtk::Table *target_table=manage(new Gtk::Table(2,2,false));
	target_frame->add(*target_table);
	target_table->attach(*manage(new Gtk::Label(_("Filename"))), 0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	target_table->attach(entry_filename, 1, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	target_table->attach(*choose_button, 2, 3, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	target_table->attach(*manage(new Gtk::Label(_("Target"))), 3, 4, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	target_table->attach(optionmenu_target, 4, 5, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	get_vbox()->pack_start(*target_frame);

	toggle_single_frame.signal_toggled().connect(sigc::mem_fun(*this, &studio::RenderSettings::on_single_frame_toggle));


	Gtk::Frame *settings_frame=manage(new Gtk::Frame(_("Settings")));
	Gtk::Table *settings_table=manage(new Gtk::Table(2,2,false));
	settings_frame->add(*settings_table);
	settings_table->attach(*manage(new Gtk::Label(_("Quality"))), 0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	settings_table->attach(entry_quality, 1, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	settings_table->attach(*manage(new Gtk::Label(_("Anti-Alias"))), 2, 3, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	settings_table->attach(entry_antialias, 3, 4, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	settings_table->attach(toggle_single_frame, 4, 5, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	get_vbox()->pack_start(*settings_frame);
	
	


	get_vbox()->pack_start(widget_rend_desc);
	
	
	Gtk::Button *render_button(manage(new class Gtk::Button(Gtk::StockID("Render"))));
	render_button->show();
	add_action_widget(*render_button,1);
	render_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::RenderSettings::on_render_pressed));

	Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-cancel"))));
	cancel_button->show();
	add_action_widget(*cancel_button,0);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::RenderSettings::on_cancel_pressed));

	//set_default_response(1);
	
	set_title(_("Render Settings")+String(" - ")+canvas_interface_->get_canvas()->get_name());


	toggle_single_frame.set_active(true);
	widget_rend_desc.disable_time_section();
	

	try
	{
		entry_filename.set_text(Glib::build_filename(Glib::get_home_dir(),Glib::ustring("Desktop")+ETL_DIRECTORY_SEPERATOR+Glib::ustring("output.png")));
	}
	catch(...)
	{
		synfig::warning("Averted crash!");
		entry_filename.set_text("output.png");
	}
	
	get_vbox()->show_all();
}

RenderSettings::~RenderSettings()
{
}

void
RenderSettings::on_rend_desc_changed()
{
	widget_rend_desc.set_rend_desc(canvas_interface_->get_canvas()->rend_desc());
}

void
RenderSettings::set_target(synfig::String name)
{
	target_name=name;
}

void
RenderSettings::on_choose_pressed()
{
	String filename=entry_filename.get_text();
	if(App::dialog_saveas_file("Save Render As",filename))
		entry_filename.set_text(filename);
}

void
RenderSettings::on_render_pressed()
{
	String filename=entry_filename.get_text();
	
	if(filename.empty())
	{
		canvas_interface_->get_ui_interface()->error(_("You must supply a filename!"));
		return;
	}

	// If the target type is not yet defined,
	// try to figure it out from the outfile.
	if(target_name.empty())
	{
		try
		{
			String ext=String(find(filename.begin(),filename.end(),'.')+1,filename.end());
			if(Target::ext_book().count(ext))
				target_name=Target::ext_book()[ext];
			else
				target_name=ext;
		}
		catch(std::runtime_error x)
		{
			canvas_interface_->get_ui_interface()->error(_("Unable to determine proper target from filename."));
			return;			
		}
	}

	if(filename.empty() && target_name!="null")
	{
		canvas_interface_->get_ui_interface()->error(_("A filename is required for this target"));
		return;
	}

	Target::Handle target=Target::create(target_name,filename);
	if(!target)
	{
		canvas_interface_->get_ui_interface()->error(_("Unable to create target for ")+filename);
		return;
	}

	hide();
	
	target->set_canvas(canvas_interface_->get_canvas());
	RendDesc rend_desc(widget_rend_desc.get_rend_desc());
	rend_desc.set_antialias((int)adjustment_antialias.get_value());
	
	// If we are to only render the current frame
	if(toggle_single_frame.get_active())
		rend_desc.set_time(canvas_interface_->get_time());

	target->set_rend_desc(&rend_desc);
	target->set_quality((int)adjustment_quality.get_value());

	canvas_interface_->get_ui_interface()->task(_("Rendering ")+filename);

	if(async_renderer)
	{
		async_renderer->stop();
		async_renderer.detach();
	}
	async_renderer=new AsyncRenderer(target);
	async_renderer->signal_finished().connect( sigc::mem_fun(*this,&RenderSettings::on_finished));
	async_renderer->start();
	/*
	if(!target->render(canvas_interface_->get_ui_interface().get()))
	{
		canvas_interface_->get_ui_interface()->error(_("Render Failure"));
		canvas_interface_->get_ui_interface()->amount_complete(0,10000);
		return;
	}
	
	// Success!
	canvas_interface_->get_ui_interface()->task(filename+_(" rendered sucessfuly"));	
	canvas_interface_->get_ui_interface()->amount_complete(0,10000);
	*/
	return;	
}

void
RenderSettings::on_finished()
{
	canvas_interface_->get_ui_interface()->task(_("File rendered sucessfuly"));	
	canvas_interface_->get_ui_interface()->amount_complete(0,10000);
}
	
void
RenderSettings::on_cancel_pressed()
{
	hide();
}

void
RenderSettings::on_single_frame_toggle()
{
	if(toggle_single_frame.get_active())
		widget_rend_desc.disable_time_section();
	else
		widget_rend_desc.enable_time_section();
}
