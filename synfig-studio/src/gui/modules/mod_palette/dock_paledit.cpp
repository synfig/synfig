/* === S Y N F I G ========================================================= */
/*!	\file dock_paledit.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2010 Nikita Kitaev
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

#include "dock_paledit.h"

#include <errno.h>
#include <sys/stat.h>

#include <gtkmm/imagemenuitem.h>
#include <gtkmm/menu.h>
#include <gtkmm/stylecontext.h>

#include <gui/app.h>
#include <gui/dialogs/dialog_color.h>
#include <gui/localization.h>
#include <gui/widgets/widget_color.h>

#include <synfig/general.h>
#include <synfigapp/main.h>
#include <synfigapp/uimanager.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */
/*
class studio::PaletteSettings : public synfigapp::Settings
{
	Dock_PalEdit* dialog_palette;
	synfig::String name;
public:
	PaletteSettings(Dock_PalEdit* window,const synfig::String& name):
		dialog_palette(window),
		name(name)
	{
		dialog_palette->dialog_settings.add_domain(this,name);
	}

	virtual ~PaletteSettings()
	{
		dialog_palette->dialog_settings.remove_domain(name);
	}

	virtual bool get_value(const synfig::String& key, synfig::String& value)const
	{
		int i(atoi(key.c_str()));
		if(i<0 || i>=dialog_palette->size())
			return false;
		Color c(dialog_palette->get_color(i));
		value=strprintf("%f %f %f %f",c.get_r(),c.get_g(),c.get_b(),c.get_a());
		return true;
	}

	virtual bool set_value(const synfig::String& key,const synfig::String& value)
	{
		int i(atoi(key.c_str()));
		if(i<0)
			return false;
		if(i>=dialog_palette->size())
			dialog_palette->palette_.resize(i+1);
		float r,g,b,a;
		if(!strscanf(value,"%f %f %f %f",&r,&g,&b,&a))
			return false;
		dialog_palette->set_color(Color(r,g,b,a),i);
		return true;
	}

	virtual KeyList get_key_list()const
	{
		synfigapp::Settings::KeyList ret(synfigapp::Settings::get_key_list());

		int i;
		for(i=0;i<dialog_palette->size();i++)
			ret.push_back(strprintf("%03d",i));
		return ret;
	}
};
*/
/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dock_PalEdit::Dock_PalEdit():
	Dockable("pal_edit",_("Palette Editor"),Gtk::StockID("synfig-palette")),
	//palette_settings(new PaletteSettings(this,"colors")),
	table(2,2,false)
{
	// Make Palette Editor toolbar buttons small for space efficiency
	auto context = get_style_context();
	context->add_class("synfigstudio-efficient-workspace");

	action_group=Gtk::ActionGroup::create("action_group_pal_edit");
	action_group->add(Gtk::Action::create(
		"palette-add-color",
		Gtk::StockID("gtk-add"),
		_("Add Color"),
		_("Add current outline color\nto the palette")
	),
		sigc::mem_fun(
			*this,
			&Dock_PalEdit::on_add_pressed
		)
	);
	action_group->add(Gtk::Action::create(
		"palette-save",
		Gtk::StockID("gtk-save"),
		_("Save palette"),
		_("Save the current palette")
	),
		sigc::mem_fun(
			*this,
			&Dock_PalEdit::on_save_pressed
		)
	);
	action_group->add(Gtk::Action::create(
		"palette-load",
		Gtk::StockID("gtk-open"),
		_("Open a palette"),
		_("Open a saved palette")
	),
		sigc::mem_fun(
			*this,
			&Dock_PalEdit::on_open_pressed
		)
	);
	action_group->add(Gtk::Action::create(
		"palette-set-default",
		Gtk::StockID("gtk-refresh"),
		_("Load default"),
		_("Load default palette")
	),
		sigc::mem_fun(
			*this,
			&Dock_PalEdit::set_default_palette
		)
	);


	App::ui_manager()->insert_action_group(action_group);

    Glib::ustring ui_info =
	"<ui>"
	"	<toolbar action='toolbar-palette'>"
	"	<toolitem action='palette-add-color' />"
	"	<toolitem action='palette-save' />"
	"	<toolitem action='palette-load' />"
	"	<toolitem action='palette-set-default' />"
	"	</toolbar>"
	"</ui>"
	;

	App::ui_manager()->add_ui_from_string(ui_info);

	set_toolbar(*dynamic_cast<Gtk::Toolbar*>(App::ui_manager()->get_widget("/toolbar-palette")));

	/*
	add_button(
		Gtk::StockID("gtk-add"),
		_("Add current outline color\nto the palette")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&Dock_PalEdit::on_add_pressed
		)
	);
	*/

	add(table);
	table.set_homogeneous(true);

	set_default_palette();

	show_all_children();
}

Dock_PalEdit::~Dock_PalEdit()
{
	//delete palette_settings;
}

void
Dock_PalEdit::set_palette(const synfig::Palette& x)
{
	palette_=x;
	refresh();
}

void
Dock_PalEdit::on_add_pressed()
{
	add_color(synfigapp::Main::get_outline_color());
}

void
Dock_PalEdit::on_save_pressed()
{
	// it would be nice to have initial spal file name same as current canvas name, 
	// use "My Palette" as temporary spal file name as a hack.
	//synfig::String filename = selected_instance->get_file_name();
	std::string filename = "My Palette";
	while (App::dialog_save_file_spal(_("Please choose a file name"), filename, ANIMATION_DIR_PREFERENCE))
	{
		// If the filename still has wildcards, then we should
		// continue looking for the file we want
		if (etl::basename(filename).find('*') != std::string::npos) {
			continue;
		}

		{
			struct stat	s;
			int stat_return = stat(filename.c_str(), &s);

			// if stat() fails with something other than 'file doesn't exist', there's been a real
			// error of some kind.  let's give up now and ask for a new path.
			if (stat_return == -1 && errno != ENOENT)
			{
				perror(filename.c_str());
				std::string msg(etl::strprintf(_("Unable to check whether '%s' exists."), filename.c_str()));
				App::dialog_message_1b("ERROR", msg, "details", _("Close"));
				continue;
			}

			// if the file exists and the user doesn't want to overwrite it, keep prompting for a filename
			std::string message = etl::strprintf(_("A file named \"%s\" already exists. "
							"Do you want to replace it?"),
							etl::basename(filename).c_str());

			std::string details = etl::strprintf(_("The file already exists in \"%s\". "
							"Replacing it will overwrite its contents."),
							etl::basename(etl::dirname(filename)).c_str());

			if ((stat_return == 0) && !App::dialog_message_2b(
				message,
				details,
				Gtk::MESSAGE_QUESTION,
				_("Use Another Name…"),
				_("Replace"))
			)
				continue;
		}
		try {
			palette_.save_to_file(filename);
		} catch (const std::string& err) {
			synfig::error(err);
			App::dialog_message_1b("ERROR", err, "details", _("Close"));
		}
		return;
	}
}

void
Dock_PalEdit::on_open_pressed()
{
	synfig::String filename = "*.spal";
	while(App::dialog_open_file_spal(_("Please select a palette file"), filename, ANIMATION_DIR_PREFERENCE))
	{
		// If the filename still has wildcards, then we should
		// continue looking for the file we want
		if (filename.find('*') != std::string::npos) {
			continue;
		}

		try	{
			palette_=synfig::Palette::load_from_file(filename);
		} catch (const std::string& err) {
			App::get_ui_interface()->error(err);
			break;
		} catch (...) {
			App::get_ui_interface()->error(std::string(_("Unable to open file")) + " " + _("Unknown error"));
			break;
		}
		break;
	}
	refresh();
}

void
Dock_PalEdit::show_menu(int i)
{
	Gtk::Menu* menu(manage(new Gtk::Menu()));
	menu->signal_hide().connect(sigc::bind(sigc::ptr_fun(&delete_widget), menu));

	Gtk::MenuItem *item;
	item = manage(new Gtk::ImageMenuItem(Gtk::StockID("gtk-select-color")));
	item->signal_activate().connect(
		sigc::bind(
			sigc::mem_fun(*this,&studio::Dock_PalEdit::edit_color),
			i ));
	item->show_all();
	menu->append(*item);

	item = manage(new Gtk::ImageMenuItem(Gtk::StockID("gtk-delete")));
	item->signal_activate().connect(
		sigc::bind(
			sigc::mem_fun(*this,&studio::Dock_PalEdit::erase_color),
			i ));
	item->show_all();
	menu->append(*item);

	menu->popup(3,gtk_get_current_event_time());
}

int
Dock_PalEdit::add_color(const synfig::Color& x)
{
	palette_.push_back(x);
	signal_changed()();
	refresh();
	return size()-1;
}

void
Dock_PalEdit::set_color(synfig::Color x, int i)
{
	palette_[i].color=x;
	signal_changed()();
	refresh();
}

Color
Dock_PalEdit::get_color(int i)const
{
	return palette_[i].color;
}

void
Dock_PalEdit::erase_color(int i)
{
	palette_.erase(palette_.begin()+i);
	signal_changed()();
	refresh();
}

void
Dock_PalEdit::refresh()
{
	const int width(12);

	// Clear the table
	table.foreach(sigc::mem_fun(table,&Gtk::Table::remove));

	for(int i=0;i<size();i++)
	{
		Widget_Color* widget_color(manage(new Widget_Color()));
		widget_color->set_value(get_color(i));
		widget_color->set_size_request(12,12);
		widget_color->signal_activate().connect(
			sigc::bind(
				sigc::mem_fun(*this,&studio::Dock_PalEdit::select_fill_color),
				i
			)
		);
		widget_color->signal_middle_click().connect(
			sigc::bind(
				sigc::mem_fun(*this,&studio::Dock_PalEdit::select_outline_color),
				i
			)
		);
		widget_color->signal_right_click().connect(
			sigc::bind(
				sigc::mem_fun(*this,&studio::Dock_PalEdit::show_menu),
				i
			)
		);
		int c(i%width),r(i/width);
		table.attach(*widget_color, c, c+1, r, r+1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	}
	table.show_all();
	queue_draw();
}


void
Dock_PalEdit::edit_color(int i)
{
	App::dialog_color->reset();
	App::dialog_color->set_color(get_color(i));
	App::dialog_color->signal_edited().connect(
		sigc::bind(
			sigc::mem_fun(*this,&studio::Dock_PalEdit::set_color),
			i
		)
	);
	App::dialog_color->present();
}

void
Dock_PalEdit::select_fill_color(int i)
{
	synfigapp::Main::set_fill_color(get_color(i));
}

void
Dock_PalEdit::select_outline_color(int i)
{
	synfigapp::Main::set_outline_color(get_color(i));
}

void
Dock_PalEdit::set_default_palette()
{
	int width=12;

	palette_.clear();

	// Greys
	palette_.push_back(Color::alpha());
	for(int i=0;i<width-1;i++)
	{
		Color c(
			float(i)/(float)(width-2),
			float(i)/(float)(width-2),
			float(i)/(float)(width-2)
		);
		palette_.push_back(c);
	}

	// Tans
	for(int i=0;i<width;i++)
	{
		float x(float(i)/(float)(width-1));
		const Color tan1(0.2,0.05,0);
		const Color tan2(0.85,0.64,0.20);

		palette_.push_back(Color::blend(tan2,tan1,x));
	}

	// Solids
	palette_.push_back(Color::red());
	palette_.push_back(Color(1.0f,0.25f,0.0f));	// Orange
	palette_.push_back(Color::yellow());
	palette_.push_back(Color(0.25f,1.00f,0.0f));	// yellow-green
	palette_.push_back(Color::green());
	palette_.push_back(Color(0.0f,1.00f,0.25f));	// green-blue
	palette_.push_back(Color::cyan());
	palette_.push_back(Color(0.0f,0.25f,1.0f));	// Sea Blue
	palette_.push_back(Color::blue());
	palette_.push_back(Color(0.25f,0.0f,1.0f));
	palette_.push_back(Color::magenta());
	palette_.push_back(Color(1.0f,0.0f,0.25f));


	const int levels(3);

	// Colors
	for(int j=0;j<levels;j++)
	for(int i=0;i<width;i++)
	{
		Color c(Color::red());
		c.set_hue(c.get_hue()-Angle::rot(float(i)/(float)(width)));
		c=c.clamped();
		float s(float(levels-j)/float(levels));
		s*=s;
		c.set_r(c.get_r()*s);
		c.set_g(c.get_g()*s);
		c.set_b(c.get_b()*s);
		palette_.push_back(c);
	}


	/*
	const int levels(3);

	for(int i=0;i<levels*levels*levels;i++)
	{
		Color c(
			float(i%levels)/(float)(levels-1),
			float(i/levels%levels)/(float)(levels-1),
			float(i/(levels*levels))/(float)(levels-1)
		);
		palette_.push_back(c);
	}
	*/
	refresh();
}

int
Dock_PalEdit::size()const
{
	return palette_.size();
}
