/* === S Y N F I G ========================================================= */
/*!	\file dialog_setup.cpp
**	\brief Dialog Preference implementation
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2008, 2009, 2012 Carlos López
**	Copyright (c) 2015 Jerome Blanchi
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

#include <gtkmm/scale.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>
#include <gtkmm/notebook.h>
#include "widgets/widget_enum.h"
#include "autorecover.h"
#include "duck.h"

#include <ETL/stringf>
#include <ETL/misc>

#include <synfig/rendering/renderer.h>

#include <synfig/rendering/renderer.h>

#include <synfigapp/canvasinterface.h>

#include "dialogs/dialog_setup.h"

#include "app.h"
#include <gui/localization.h>
#include "widgets/widget_enum.h"
#include "autorecover.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#define DIALOG_PREFERENCE_UI_INIT_GRID(grid) 					\
		grid->set_orientation(Gtk::ORIENTATION_HORIZONTAL);		\
		grid->set_row_spacing(6);								\
		grid->set_column_spacing(12);							\
		grid->set_border_width(8);								\
		grid->set_column_homogeneous(false);

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static void
attach_label(Gtk::Grid *grid, String str, guint col)
{
	Gtk::Label* label(manage(new Gtk::Label((str + ":").c_str())));
	label->set_alignment(Gtk::ALIGN_END, Gtk::ALIGN_CENTER);
	grid->attach(*label, 0, col, 1, 1);
}

Dialog_Setup::Dialog_Setup(Gtk::Window& parent):
	Dialog(_("Synfig Studio Preferences"),parent,true),
	adj_gamma_r(Gtk::Adjustment::create(2.2,0.1,3.0,0.025,0.025,0.025)),
	adj_gamma_g(Gtk::Adjustment::create(2.2,0.1,3.0,0.025,0.025,0.025)),
	adj_gamma_b(Gtk::Adjustment::create(2.2,0.1,3.0,0.025,0.025,0.025)),
	adj_recent_files(Gtk::Adjustment::create(15,1,50,1,1,0)),
	adj_undo_depth(Gtk::Adjustment::create(100,10,5000,1,1,1)),
	toggle_use_colorspace_gamma(),
#ifdef SINGLE_THREADED
	toggle_single_threaded(),
#endif
	toggle_restrict_radius_ducks(),
	toggle_resize_imported_images(),
	toggle_enable_experimental_features(),
	toggle_use_dark_theme(),
	adj_pref_x_size(Gtk::Adjustment::create(480,1,10000,1,10,0)),
	adj_pref_y_size(Gtk::Adjustment::create(270,1,10000,1,10,0)),
	adj_pref_fps(Gtk::Adjustment::create(24.0,1.0,100,0.1,1,0))
{
	// Setup the buttons
	Gtk::Button *restore_button(manage(new class Gtk::Button(_("Restore Defaults"))));
	restore_button->show();
	add_action_widget(*restore_button,1);
	restore_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Setup::on_restore_pressed));

	Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-cancel"))));
	cancel_button->show();
	add_action_widget(*cancel_button,0);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Setup::hide));

	Gtk::Button *ok_button(manage(new class Gtk::Button(Gtk::StockID("gtk-ok"))));
	ok_button->show();
	add_action_widget(*ok_button,2);
	ok_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Setup::on_ok_pressed));

	// Notebook
	Gtk::Notebook *notebook=manage(new class Gtk::Notebook());
	get_vbox()->pack_start(*notebook);

	// Gamma
	create_gamma_page(*notebook);
	// Misc
	create_misc_page(*notebook);
	// Document
	create_document_page(*notebook);
	// Render
	create_render_page(*notebook);
	// Interface
	create_interface_page(*notebook);

	show_all_children();
}

Dialog_Setup::~Dialog_Setup()
{
}

void
Dialog_Setup::create_gamma_page(Gtk::Notebook& notebook)
{
	Gtk::Grid *gamma_grid=manage(new Gtk::Grid());
	DIALOG_PREFERENCE_UI_INIT_GRID(gamma_grid);
	notebook.append_page(*gamma_grid,_("Gamma"));

	/*---------Gamma------------------*\
	 *
	 *        *****°°°°°°°#####
	 *        *****°°°°°°°#####
	 *        *****°°°°°°°#####
	 *        *****°°°°°°°#####
	 *        *****°°°°°°°#####
	 *   red ---------x--------------
	 * green ---------x--------------
	 *  blue ---------x--------------
	 * black ---------x--------------
	 *
	 */

	int row(0);
#ifndef __APPLE__
	gamma_grid->attach(gamma_pattern, 0, row, 2, 1);
	gamma_pattern.set_halign(Gtk::ALIGN_CENTER);
#endif
	Gtk::Scale* scale_gamma_r(manage(new Gtk::Scale(adj_gamma_r)));
	attach_label(gamma_grid, _("Red"), ++row);
	gamma_grid->attach(*scale_gamma_r, 1, row, 1, 1);
	scale_gamma_r->set_hexpand_set(true);
	adj_gamma_r->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_gamma_r_change));

	Gtk::Scale* scale_gamma_g(manage(new Gtk::Scale(adj_gamma_g)));
	attach_label(gamma_grid, _("Green"), ++row);
	gamma_grid->attach(*scale_gamma_g, 1, row, 1, 1);
	scale_gamma_g->set_hexpand(true);
	adj_gamma_g->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_gamma_g_change));

	Gtk::Scale* scale_gamma_b(manage(new Gtk::Scale(adj_gamma_b)));
	attach_label(gamma_grid, _("Blue"), ++row);
	gamma_grid->attach(*scale_gamma_b, 1, row, 1, 1);
	scale_gamma_b->set_hexpand(true);
	adj_gamma_b->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_gamma_b_change));

	attach_label(gamma_grid, _("Black Level"), ++row);
	gamma_grid->attach(black_level_selector, 1, row, 1, 1);
	black_level_selector.set_hexpand(true);
	black_level_selector.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_black_level_change));

	//attach_label(gamma_grid,_("Red-Blue Level"), ++row);
	//gamma_grid->attach(red_blue_level_selector, 1, row, 1, 1);
	//red_blue_level_selector.set_hexpand(true);
	//red_blue_level_selector.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_red_blue_level_change));
}

void
Dialog_Setup::create_misc_page(Gtk::Notebook& notebook)
{
	Gtk::Grid *misc_grid=manage(new Gtk::Grid());
	DIALOG_PREFERENCE_UI_INIT_GRID(misc_grid);
	notebook.append_page(*misc_grid,_("Misc."));

	int row=0;
	// Misc - 0 Timestamp
	timestamp_menu=manage(new class Gtk::Menu());
	attach_label(misc_grid, _("Timestamp"), row);
	misc_grid->attach(timestamp_comboboxtext, 1, row, 1, 1);
	timestamp_comboboxtext.set_hexpand(true);

	#define ADD_TIMESTAMP(desc,x) {				\
		timestamp_comboboxtext.append(desc);	\
		time_formats[desc] = x;					\
	}

	ADD_TIMESTAMP("HH:MM:SS.FF",		Time::FORMAT_VIDEO	);
	ADD_TIMESTAMP("(HHh MMm SSs) FFf",	Time::FORMAT_NORMAL	);
	ADD_TIMESTAMP("(HHhMMmSSs)FFf",		Time::FORMAT_NORMAL	| Time::FORMAT_NOSPACES	);
	ADD_TIMESTAMP("HHh MMm SSs FFf",	Time::FORMAT_NORMAL	| Time::FORMAT_FULL		);
	ADD_TIMESTAMP("HHhMMmSSsFFf",		Time::FORMAT_NORMAL	| Time::FORMAT_NOSPACES	| Time::FORMAT_FULL);
	ADD_TIMESTAMP("FFf",				Time::FORMAT_FRAMES );

	#undef ADD_TIMESTAMP

	timestamp_comboboxtext.signal_changed().connect(
		sigc::mem_fun(*this, &Dialog_Setup::on_time_format_changed) );

	// Misc - 1 Unit system
	{
		ParamDesc param_desc;
		param_desc
			.set_hint("enum")
			.add_enum_value(Distance::SYSTEM_UNITS,"u",_("Units"))
			.add_enum_value(Distance::SYSTEM_PIXELS,"px",_("Pixels"))
			.add_enum_value(Distance::SYSTEM_POINTS,"pt",_("Points"))
			.add_enum_value(Distance::SYSTEM_INCHES,"in",_("Inches"))
			.add_enum_value(Distance::SYSTEM_METERS,"m",_("Meters"))
			.add_enum_value(Distance::SYSTEM_CENTIMETERS,"cm",_("Centimeters"))
			.add_enum_value(Distance::SYSTEM_MILLIMETERS,"mm",_("Millimeters"));

		widget_enum=manage(new Widget_Enum());
		widget_enum->set_param_desc(param_desc);

		attach_label(misc_grid, _("Unit System"), ++row);
		misc_grid->attach(*widget_enum, 1, row, 1, 1);
		widget_enum->set_hexpand(true);
	}

	// Misc - 2 Recent files
	Gtk::SpinButton* recent_files_spinbutton(manage(new Gtk::SpinButton(adj_recent_files,1,0)));
	attach_label(misc_grid, _("Recent Files"), ++row);
	misc_grid->attach(*recent_files_spinbutton, 1, row, 1, 1);
	recent_files_spinbutton->set_hexpand(true);

	// Misc - 3 Auto backup interval
	attach_label(misc_grid, _("Auto Backup Interval (0 to disable)"), ++row);
	misc_grid->attach(auto_backup_interval, 1, row, 1, 1);
	auto_backup_interval.set_hexpand(true);

	// Misc - 4 Browser_command
	attach_label(misc_grid, _("Browser Command"), ++row);
	misc_grid->attach(textbox_browser_command, 1, row, 1, 1);
	textbox_browser_command.set_hexpand(true);

	// Misc - 5 Brushes path
	attach_label(misc_grid, _("Brush Presets Path"), ++row);
	misc_grid->attach(textbox_brushes_path, 1, row, 1, 1);
	textbox_brushes_path.set_hexpand(true);

	// Misc - 7 Visually Linear Color Selection
	attach_label(misc_grid, _("Visually linear color selection"), ++row);
	misc_grid->attach(toggle_use_colorspace_gamma, 1, row, 1, 1);
	toggle_use_colorspace_gamma.set_hexpand(true);

	// Misc - 8 Restrict Really-valued Handles to Top Right Quadrant
	attach_label(misc_grid, _("Restrict really-valued handles to top right quadrant"), ++row);
	misc_grid->attach(toggle_restrict_radius_ducks, 1, row, 1, 1);
	toggle_restrict_radius_ducks.set_hexpand(true);

	// Misc - 9 Scaling New Imported Images to Fit Canvas
	attach_label(misc_grid, _("Scaling new imported image to fix canvas"), ++row);
	misc_grid->attach(toggle_resize_imported_images, 1, row, 1, 1);
	toggle_resize_imported_images.set_hexpand(true);

	// Misc - 11 enable_experimental_features
	//attach_label(misc_grid, _("Experimental features (restart needed)"), ++row);
	//misc_grid->attach(toggle_enable_experimental_features, 1, row, 1, 1);
	//toggle_enable_experimental_features.set_hexpand(true);

#ifdef SINGLE_THREADED
	// Misc - 12 single_threaded
	attach_label(misc_grid, _("Single thread only (CPUs)"), ++row);
	misc_grid->attach(toggle_single_threaded, 1, row, 1, 1);
	toggle_single_threaded.set_hexpand(true);
#endif

}

void
Dialog_Setup::create_document_page(Gtk::Notebook& notebook)
{
	Gtk::Grid *document_grid = manage(new Gtk::Grid());
	DIALOG_PREFERENCE_UI_INIT_GRID(document_grid);
	notebook.append_page(*document_grid, _("Document"));

	/*---------Document------------------*\
	 *
	 *  doc prefix  ___________________
	 *  doc x      [_]  predef resolution
	 *  doc y      [_]  [  resolutions  ]
	 *                  predef FPS
	 *  fps        [_]  [      FPS      ]
	 *
	 */

	int row(0);
	// Document - Preferred file name prefix
	attach_label(document_grid, _("New Document filename prefix"), row);
	document_grid->attach(textbox_custom_filename_prefix, 1, row, 2, 1);
	textbox_custom_filename_prefix.set_tooltip_text( _("File name prefix for the new created document"));
	textbox_custom_filename_prefix.set_hexpand(true);

	// Document - New Document X size
	pref_x_size_spinbutton = Gtk::manage(new Gtk::SpinButton(adj_pref_x_size, 1, 0));
	attach_label(document_grid, _("New Document X size"),++row);
	document_grid->attach(*pref_x_size_spinbutton, 1, row, 1, 1);
	pref_x_size_spinbutton->set_tooltip_text(_("Width in pixels of the new created document"));
	pref_x_size_spinbutton->set_hexpand(true);

	//Document - Label for predefined sizes of canvases.
	Gtk::Label* label(manage(new Gtk::Label(_("Predefined Resolutions:"))));
	label->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	document_grid->attach(*label, 2, row, 1, 1);
	label->set_hexpand(true);

	// Document - New Document Y size
	pref_y_size_spinbutton = Gtk::manage(new Gtk::SpinButton(adj_pref_y_size, 1, 0));
	attach_label(document_grid,_("New Document Y size"), ++row);
	document_grid->attach(*pref_y_size_spinbutton, 1, row, 1, 1);
	pref_y_size_spinbutton->set_tooltip_text(_("High in pixels of the new created document"));
	pref_y_size_spinbutton->set_hexpand(true);

	//Document - Template for predefined sizes of canvases.
	size_template_combo = Gtk::manage(new Gtk::ComboBoxText());
	document_grid->attach(*size_template_combo, 2, row, 1, 1);
	size_template_combo->signal_changed().connect(sigc::mem_fun(*this, &studio::Dialog_Setup::on_size_template_combo_change));
	size_template_combo->prepend(_("4096x3112 Full Aperture 4K"));
	size_template_combo->prepend(_("2048x1556 Full Aperture Native 2K"));
	size_template_combo->prepend(_("1920x1080 HDTV 1080p/i"));
	size_template_combo->prepend(_("1280x720  HDTV 720p"));
	size_template_combo->prepend(_("720x576   DVD PAL"));
	size_template_combo->prepend(_("720x480   DVD NTSC"));
	size_template_combo->prepend(_("720x540   Web 720x"));
	size_template_combo->prepend(_("720x405   Web 720x HD"));
	size_template_combo->prepend(_("640x480   Web 640x"));
	size_template_combo->prepend(_("640x360   Web 640x HD"));
	size_template_combo->prepend(_("480x360   Web 480x"));
	size_template_combo->prepend(_("480x270   Web 480x HD"));
	size_template_combo->prepend(_("360x270   Web 360x"));
	size_template_combo->prepend(_("360x203   Web 360x HD"));
	size_template_combo->prepend(DEFAULT_PREDEFINED_SIZE);

	//Document - Label for predefined fps
	Gtk::Label* label1(manage(new Gtk::Label(_("Predefined FPS:"))));
	label1->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	document_grid->attach(*label1,2, ++row, 1,1);
	label1->set_hexpand(true);

	// Document - New Document FPS
	pref_fps_spinbutton = Gtk::manage(new Gtk::SpinButton(adj_pref_fps, 1, 3));
	attach_label(document_grid,_("New Document FPS"), ++row);
	document_grid->attach(*pref_fps_spinbutton, 1, row, 1, 1);
	pref_fps_spinbutton->set_tooltip_text(_("Frames per second of the new created document"));
	pref_fps_spinbutton->set_hexpand(true);

	//Document - Template for predefined fps
	fps_template_combo = Gtk::manage(new Gtk::ComboBoxText());
	document_grid->attach(*fps_template_combo, 2, row, 1, 1);
	fps_template_combo->signal_changed().connect(sigc::mem_fun(*this, &studio::Dialog_Setup::on_fps_template_combo_change));
	fps_template_combo->set_hexpand(true);
	//Document - Fill the FPS combo box with proper strings (not localised)
	float f[8];
	f[0] = 60;
	f[1] = 50;
	f[2] = 30;
	f[3] = 25;
	f[4] = 24.967;
	f[5] = 24;
	f[6] = 15;
	f[7] = 12;
	for (int i=0; i<8; i++)
		fps_template_combo->prepend(strprintf("%5.3f", f[i]));

	fps_template_combo->prepend(DEFAULT_PREDEFINED_FPS);
}

void
Dialog_Setup::create_render_page(Gtk::Notebook& notebook)
{
	/*---------Render------------------*\
	 *
	 *  sequence separator _________
	 *  use cairo navigato _______
	 *  use cairo workarea _____
	 *
	 *
	 */

	Gtk::Grid *render_grid = manage(new Gtk::Grid());
	DIALOG_PREFERENCE_UI_INIT_GRID(render_grid);
	notebook.append_page(*render_grid, _("Render"));

	int row(0);
	// Render - Image sequence separator
	attach_label(render_grid, _("Image Sequence Separator String"), row);
	render_grid->attach(image_sequence_separator, 1, row, 1, 1);
	image_sequence_separator.set_hexpand(true);
	// Render - Use Cairo on Navigator
	attach_label(render_grid, _("Navigator renderer"), ++row);
	render_grid->attach(navigator_renderer_combo, 1, row, 1, 1);
	// Render - Use Cairo on WorkArea
	attach_label(render_grid, _("WorkArea renderer"), ++row);
	render_grid->attach(workarea_renderer_combo, 1, row, 1, 1);

	navigator_renderer_combo.append("", _("Legacy"));
	workarea_renderer_combo.append("", _("Legacy"));
	typedef std::map<synfig::String, synfig::rendering::Renderer::Handle> RendererMap;
	const RendererMap &renderers = synfig::rendering::Renderer::get_renderers();
	for(RendererMap::const_iterator i = renderers.begin(); i != renderers.end(); ++i)
	{
		assert(!i->first.empty());
		navigator_renderer_combo.append(i->first, i->second->get_name());
		workarea_renderer_combo.append(i->first, i->second->get_name());
	}
}

void
Dialog_Setup::create_interface_page(Gtk::Notebook& notebook)
{
	Gtk::Grid *interface_grid=manage(new Gtk::Grid());
	DIALOG_PREFERENCE_UI_INIT_GRID(interface_grid);
	notebook.append_page(*interface_grid,_("Interface"));

	// Interface - UI Language
	Glib::ustring lang_names[] = {
		_("System Language"),
		_("Arabic"),
		_("Basque"),
		_("Basque (Spain)"),
		_("Catalan"),
		_("Chinese (China)"),
		_("Czech"),
		_("Danish"),
		_("Dutch "),
		_("English"),
		_("English (United Kingdom)"),
		_("Farsi (Iran)"),
		_("French "),
		_("German"),
		_("Greek (Greece)"),
		_("Hebrew "),
		_("Hungarian "),
		_("Italian "),
		_("Japanese (Japan)"),
		_("Lithuanian "),
		_("Norwegian (Norway)"),
		_("Polish (Poland)"),
		_("Portuguese (Brazil)"),
		_("Romanian"),
		_("Russian"),
		_("Spanish"),
		_("Sinhala"),
		_("Slovak (Slovakia)"),
		_("Swedish (Sweden)"),
		_("Turkish"),
	};

   Glib::ustring lang_codes[] = {
		"os_LANG",		// System Language
		"ar",			// Arabick
		"eu",			// Basque
		"eu_ES",		// Basque (Spain)
		"ca",			// Catalan
		"zh_CN",		// Chinese (China)
		"cs",			// CZech
		"da",			// Danish
		"nl",			// Dutch
		"en",			// English - default of development
		"en_GB",		// English (United Kingdom)
		"fa_IR",		// Farsi (Iran)
		"fr",			// French
		"de",			// German
		"el_GR",		// Greek (Greece)
		"he",			// Hebrew
		"hu",			// Hungarian
		"it",			// Italian
		"ja_JP",		// Japanese (Japan)
		"lt",			// Lithuanian
		"no_NO",		// Norwegian (Norway)
		"pl_PL",		// Polish (Poland)
		"pt_BR",		// Portuguese (Brazil)
		"ro",			// Romanian
		"ru",			// Russian
		"es",			// Spanish
		"si",			// Sinhala
		"sk_SK",		// Slovak (Slovakia)
		"sv_SE",		// Swedish (Sweden)
		"tr"			// Turkish
   };

	int num_items = G_N_ELEMENTS(lang_names);
	Glib::ustring default_code;
	int row = 0;
	Glib::ustring lang_code = App::ui_language;

	for (int i =0 ; i < num_items; ++i)
	{
		ui_language_combo.append(lang_names[i]);
		_lang_codes.push_back(lang_codes[i]);
			if (lang_code == _lang_codes[i])
			row = i;
	}

	ui_language_combo.set_active(row);
	ui_language_combo.signal_changed().connect(sigc::mem_fun(*this, &studio::Dialog_Setup::on_ui_language_combo_change));

	row = 0;
	attach_label(interface_grid, _("Interface Language"), row);
	interface_grid->attach(ui_language_combo, 1, row, 1, 1);
	ui_language_combo.set_hexpand(true);

	// Interface - Dark UI theme
	attach_label(interface_grid, _("Dark UI theme (if available)"), ++row);
	interface_grid->attach(toggle_use_dark_theme, 1, row, 1, 1);

	// Interface - Handle tooltip
	// TODO attach label title (bold / underline?)
	attach_label(interface_grid, _("Handle Tooltips Visible"), ++row);
	// Interface - width point tooltip
	attach_label(interface_grid, _("Width point tooltips"), ++row);

	interface_grid->attach(toggle_widthpoint_handle_tooltip, 1, row, 1, 1);
	toggle_widthpoint_handle_tooltip.set_halign(Gtk::ALIGN_START);
	toggle_widthpoint_handle_tooltip.set_hexpand(false);
	// Interface - radius tooltip
	attach_label(interface_grid, _("Radius tooltips"), ++row);
	interface_grid->attach(toggle_radius_handle_tooltip, 1, row, 1, 1);
	toggle_radius_handle_tooltip.set_halign(Gtk::ALIGN_START);
	toggle_radius_handle_tooltip.set_hexpand(false);
	// Interface - transformation widget tooltip
	attach_label(interface_grid, _("Transformation widget tooltips"), ++row);
	//TODO rename toggle_handle_tooltip_transformation
	interface_grid->attach(toggle_transformation_handle_tooltip, 1, row, 1, 1);
	toggle_transformation_handle_tooltip.set_halign(Gtk::ALIGN_START);
	toggle_transformation_handle_tooltip.set_hexpand(false);
}

void
Dialog_Setup::on_ok_pressed()
{
    on_apply_pressed();
	hide();
}


void
Dialog_Setup::on_restore_pressed()
{
    App::restore_default_settings();
	hide();
}


void
Dialog_Setup::on_apply_pressed()
{
	App::gamma.set_all(
		1.0/adj_gamma_r->get_value(),
		1.0/adj_gamma_g->get_value(),
		1.0/adj_gamma_b->get_value(),
		black_level_selector.get_value(),
		red_blue_level_selector.get_value());

	App::set_max_recent_files((int)adj_recent_files->get_value());

	// Set the time format
	App::set_time_format(get_time_format());

	// Set the use_colorspace_gamma flag
	App::use_colorspace_gamma=toggle_use_colorspace_gamma.get_active();

#ifdef SINGLE_THREADED
	// Set the single_threaded flag
	App::single_threaded=toggle_single_threaded.get_active();
#endif

	// Set the auto backup interval
	App::auto_recover->set_timeout(auto_backup_interval.get_value() * 1000);

	App::distance_system=Distance::System(widget_enum->get_value());

	// Set the restrict_radius_ducks flag
	App::restrict_radius_ducks=toggle_restrict_radius_ducks.get_active();

	// Set the resize_imported_images flag
	App::resize_imported_images=toggle_resize_imported_images.get_active();

	// Set the experimental features flag
	App::enable_experimental_features=toggle_enable_experimental_features.get_active();

	// Set the dark theme flag
	App::use_dark_theme=toggle_use_dark_theme.get_active();
	App::apply_gtk_settings(App::use_dark_theme);

	// Set the browser_command textbox
	App::browser_command=textbox_browser_command.get_text();

	if ( textbox_brushes_path.get_text() == App::get_base_path()+ETL_DIRECTORY_SEPARATOR+"share"+ETL_DIRECTORY_SEPARATOR+"synfig"+ETL_DIRECTORY_SEPARATOR+"brushes" )
		App::brushes_path="";
	else
		App::brushes_path=textbox_brushes_path.get_text();

	// Set the preferred file name prefix
	App::custom_filename_prefix=textbox_custom_filename_prefix.get_text();

	// Set the preferred new Document X dimension
	App::preferred_x_size=int(adj_pref_x_size->get_value());

	// Set the preferred new Document Y dimension
	App::preferred_y_size=int(adj_pref_y_size->get_value());

	// Set the preferred Predefined size
	App::predefined_size=size_template_combo->get_active_text();

	// Set the preferred Predefined fps
	App::predefined_fps=fps_template_combo->get_active_text();

	// Set the preferred FPS
	App::preferred_fps=Real(adj_pref_fps->get_value());

	// Set the preferred image sequence separator
	App::sequence_separator=image_sequence_separator.get_text();

	// Set the navigator uses cairo flag
	App::navigator_renderer=navigator_renderer_combo.get_active_id();

	// Set the workarea uses cairo flag
	App::workarea_renderer=workarea_renderer_combo.get_active_id();

	// Set ui language
	App::ui_language = (_lang_codes[ui_language_combo.get_active_row_number()]).c_str();

	// Set ui tooltip on widht point
	App::ui_handle_tooltip_flag=toggle_widthpoint_handle_tooltip.get_active()?Duck::STRUCT_WIDTHPOINT:Duck::STRUCT_NONE;
	// Set ui tooltip on widht point
	App::ui_handle_tooltip_flag|=toggle_radius_handle_tooltip.get_active()?Duck::STRUCT_RADIUS:Duck::STRUCT_NONE;
	// Set ui tooltip on widht point
	App::ui_handle_tooltip_flag|=toggle_transformation_handle_tooltip.get_active()?Duck::STRUCT_TRANSFORMATION:Duck::STRUCT_NONE;

	App::save_settings();
	App::setup_changed();

}

void
Dialog_Setup::on_gamma_r_change()
{
	gamma_pattern.set_gamma_r(1.0/adj_gamma_r->get_value());
	gamma_pattern.refresh();
	gamma_pattern.queue_draw();
}

void
Dialog_Setup::on_gamma_g_change()
{
	gamma_pattern.set_gamma_g(1.0/adj_gamma_g->get_value());
	gamma_pattern.refresh();
	gamma_pattern.queue_draw();
}

void
Dialog_Setup::on_gamma_b_change()
{
	gamma_pattern.set_gamma_b(1.0/adj_gamma_b->get_value());
	gamma_pattern.refresh();
	gamma_pattern.queue_draw();
}

void
Dialog_Setup::on_black_level_change()
{
	gamma_pattern.set_black_level(black_level_selector.get_value());
	gamma_pattern.refresh();
	gamma_pattern.queue_draw();
}

void
Dialog_Setup::on_red_blue_level_change()
{
	gamma_pattern.set_red_blue_level(red_blue_level_selector.get_value());
	gamma_pattern.refresh();
	gamma_pattern.queue_draw();
}

void
Dialog_Setup::on_size_template_combo_change()
{
	String selection(size_template_combo->get_active_text());
	if(selection==DEFAULT_PREDEFINED_SIZE)
	{
		pref_y_size_spinbutton->set_sensitive(true);
		pref_x_size_spinbutton->set_sensitive(true);
		return;
	}
	String::size_type locx=selection.find_first_of("x"); // here should be some comparison with string::npos
	String::size_type locspace=selection.find_first_of(" ");
	String x_size(selection.substr(0,locx));
	String y_size(selection.substr(locx+1,locspace));
	int x=atoi(x_size.c_str());
	int y=atoi(y_size.c_str());
	adj_pref_x_size->set_value(x);
	adj_pref_y_size->set_value(y);
	pref_y_size_spinbutton->set_sensitive(false);
	pref_x_size_spinbutton->set_sensitive(false);

	return;
}


void
Dialog_Setup::on_ui_language_combo_change()
{
}


void
Dialog_Setup::on_fps_template_combo_change()
{
	String selection(fps_template_combo->get_active_text());
	if(selection==DEFAULT_PREDEFINED_FPS)
	{
		pref_fps_spinbutton->set_sensitive(true);
		return;
	}
	adj_pref_fps->set_value(atof(selection.c_str()));
	pref_fps_spinbutton->set_sensitive(false);
	return;
}

void
Dialog_Setup::on_time_format_changed()
{
	std::map<std::string, synfig::Time::Format>::iterator i =
		time_formats.find(timestamp_comboboxtext.get_active_text());
	if (i != time_formats.end())
		time_format = i->second;
}

void
Dialog_Setup::refresh()
{
	// Refresh the temporary gamma; do this before adjusting the sliders,
	// or variables will be used before their initialization.
	gamma_pattern.set_gamma_r(App::gamma.get_gamma_r());
	gamma_pattern.set_gamma_g(App::gamma.get_gamma_g());
	gamma_pattern.set_gamma_b(App::gamma.get_gamma_b());
	gamma_pattern.set_black_level(App::gamma.get_black_level());
	gamma_pattern.set_red_blue_level(App::gamma.get_red_blue_level());

	adj_gamma_r->set_value(1.0/App::gamma.get_gamma_r());
	adj_gamma_g->set_value(1.0/App::gamma.get_gamma_g());
	adj_gamma_b->set_value(1.0/App::gamma.get_gamma_b());
	black_level_selector.set_value(App::gamma.get_black_level());
	red_blue_level_selector.set_value(App::gamma.get_red_blue_level());

	gamma_pattern.refresh();

	adj_recent_files->set_value(App::get_max_recent_files());

	// Refresh the time format
	set_time_format(App::get_time_format());

	widget_enum->set_value(App::distance_system);

	// Refresh the status of the use_colorspace_gamma flag
	toggle_use_colorspace_gamma.set_active(App::use_colorspace_gamma);

#ifdef SINGLE_THREADED
	// Refresh the status of the single_threaded flag
	toggle_single_threaded.set_active(App::single_threaded);
#endif

	// Refresh the value of the auto backup interval
	auto_backup_interval.set_value(App::auto_recover->get_timeout() / 1000);

	// Refresh the status of the restrict_radius_ducks flag
	toggle_restrict_radius_ducks.set_active(App::restrict_radius_ducks);

	// Refresh the status of the resize_imported_images flag
	toggle_resize_imported_images.set_active(App::resize_imported_images);

	// Refresh the status of the experimental features flag
	toggle_enable_experimental_features.set_active(App::enable_experimental_features);

	// Refresh the status of the theme flag
	toggle_use_dark_theme.set_active(App::use_dark_theme);

	// Refresh the browser_command textbox
	textbox_browser_command.set_text(App::browser_command);

	if (App::brushes_path == "")
		textbox_brushes_path.set_text(App::get_base_path()+ETL_DIRECTORY_SEPARATOR+"share"+ETL_DIRECTORY_SEPARATOR+"synfig"+ETL_DIRECTORY_SEPARATOR+"brushes");
	else
		textbox_brushes_path.set_text(App::brushes_path);

	// Refresh the preferred filename prefix
	textbox_custom_filename_prefix.set_text(App::custom_filename_prefix);

	// Refresh the preferred new Document X dimension
	adj_pref_x_size->set_value(App::preferred_x_size);

	// Refresh the preferred new Document Y dimension
	adj_pref_y_size->set_value(App::preferred_y_size);

	// Refresh the preferred Predefined size
	size_template_combo->set_active_text(App::predefined_size);

	//Refresh the preferred FPS
	adj_pref_fps->set_value(App::preferred_fps);

	//Refresh the predefined FPS
	fps_template_combo->set_active_text(App::predefined_fps);

	//Refresh the sequence separator
	image_sequence_separator.set_text(App::sequence_separator);

	// Refresh the status of the navigator_renderer
	navigator_renderer_combo.set_active_id(App::navigator_renderer);

	// Refresh the status of the workarea_renderer
	workarea_renderer_combo.set_active_id(App::workarea_renderer);

	// Refresh the ui language

	// refresh ui tooltip handle info
	toggle_widthpoint_handle_tooltip.set_active(App::ui_handle_tooltip_flag&Duck::STRUCT_WIDTHPOINT);
	toggle_radius_handle_tooltip.set_active(App::ui_handle_tooltip_flag&Duck::STRUCT_RADIUS);
	toggle_transformation_handle_tooltip.set_active(App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFORMATION);

}

GammaPattern::GammaPattern():
	gamma_r(),
	gamma_g(),
	gamma_b(),
	black_level(),
	red_blue_level(),
	tile_w(80),
	tile_h(80)
{
	set_size_request(tile_w*4,tile_h*3);
}

GammaPattern::~GammaPattern()
{
}

void
GammaPattern::refresh()
{
	black[0].set_rgb_p(
		r_F32_to_F32(0.0),
		g_F32_to_F32(0.0),
		b_F32_to_F32(0.0)
	);
	white[0].set_rgb_p(
		r_F32_to_F32(1.0),
		g_F32_to_F32(1.0),
		b_F32_to_F32(1.0)
	);
	gray50[0].set_rgb_p(
		r_F32_to_F32(0.5),
		g_F32_to_F32(0.5),
		b_F32_to_F32(0.5)
	);
	gray25[0].set_rgb_p(
		r_F32_to_F32(0.25),
		g_F32_to_F32(0.25),
		b_F32_to_F32(0.25)
	);

	// Reds
	black[1].set_rgb(black[0].get_red(),0,0);
	gray25[1].set_rgb(gray25[0].get_red(),0,0);
	gray50[1].set_rgb(gray50[0].get_red(),0,0);
	white[1].set_rgb(white[0].get_red(),0,0);

	// Greens
	black[2].set_rgb(0,black[0].get_green(),0);
	gray25[2].set_rgb(0,gray25[0].get_green(),0);
	gray50[2].set_rgb(0,gray50[0].get_green(),0);
	white[2].set_rgb(0,white[0].get_green(),0);

	// blues
	black[3].set_rgb(0,0,black[0].get_blue());
	gray25[3].set_rgb(0,0,gray25[0].get_blue());
	gray50[3].set_rgb(0,0,gray50[0].get_blue());
	white[3].set_rgb(0,0,white[0].get_blue());
}

bool
GammaPattern::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	int i;
	Gdk::Color trueblack("#000000");

	int stride = Cairo::ImageSurface::format_stride_for_width(Cairo::FORMAT_A1, 2);
	std::vector<unsigned char> hlines(2*stride, 0);
	hlines[0] = 3;
	Cairo::RefPtr<Cairo::ImageSurface> stipple_mask_img = Cairo::ImageSurface::create(&hlines.front(), Cairo::FORMAT_A1, 2, 2, stride);

	// 50% Pattern
	for(i=0;i<4;i++)
	{
        cr->set_source_rgb(black[i].get_red_p(), black[i].get_green_p(), black[i].get_blue_p());
        cr->rectangle(i*tile_w, 0, tile_w, tile_h);
        cr->fill();

        cr->set_source_rgb(white[i].get_red_p(), white[i].get_green_p(), white[i].get_blue_p());
        cr->mask(stipple_mask_img, 0, 0);
        cr->rectangle(i*tile_w, 0, tile_w, tile_h);
        cr->fill();

        cr->set_source_rgb(gray50[i].get_red_p(), gray50[i].get_green_p(), gray50[i].get_blue_p());
        cr->rectangle(i*tile_w+tile_w/4, tile_h/4, tile_w-tile_w/2, tile_h-tile_h/2);
        cr->fill();
	}

	// 25% Pattern
	for(i=0;i<4;i++)
	{
        cr->set_source_rgb(black[i].get_red_p(), black[i].get_green_p(), black[i].get_blue_p());
        cr->rectangle(i*tile_w, tile_h, tile_w, tile_h);
        cr->fill();

        cr->set_source_rgb(gray50[i].get_red_p(), gray50[i].get_green_p(), gray50[i].get_blue_p());
        cr->mask(stipple_mask_img, 0, 0);
        cr->rectangle(i*tile_w, tile_h, tile_w, tile_h);
        cr->fill();

        cr->set_source_rgb(gray25[i].get_red_p(), gray25[i].get_green_p(), gray25[i].get_blue_p());
        cr->rectangle(i*tile_w+tile_w/4, tile_h+tile_h/4, tile_w-tile_w/2, tile_h-tile_h/2);
        cr->fill();
	}

	// Black-level Pattern
    cr->set_source_rgb(trueblack.get_red_p(), trueblack.get_green_p(), trueblack.get_blue_p());
	cr->rectangle(0, tile_h*2, tile_w*4, tile_h);
	cr->fill();

	for(i=0;i<4;i++)
	{
        cr->set_source_rgb(black[i].get_red_p(), black[i].get_green_p(), black[i].get_blue_p());
        cr->rectangle(i*tile_w+tile_w/4, tile_h*2+tile_h/4, tile_w-tile_w/2, tile_h-tile_h/2);
        cr->fill();
	}

	return true;
}


BlackLevelSelector::BlackLevelSelector():
	level()
{
	set_size_request(-1,24);

	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);
}

BlackLevelSelector::~BlackLevelSelector()
{
}

bool
BlackLevelSelector::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	const int w(get_width()),h(get_height());

	Gdk::Color color;

	int i;

	// Draw the gradient
	for(i=0;i<w;i++)
	{
		double c = (double)i/(double)(w-1);
        cr->set_source_rgb(c,c,c);
        cr->rectangle(i, 0, 1, h);
        cr->fill();
	}

	// Draw a frame
	cr->set_source_rgb(0,0,0);
	cr->rectangle(0, 0, w-1, h-1);
	cr->stroke();

	// Draw the position of the current value
	i=(int)(level*w+0.5);
	cr->set_source_rgb(1,0,0);
	cr->rectangle(i, 1, 1, h-1);
	cr->fill();

	// Print out the value
	Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_pango_context()));
	layout->set_text(etl::strprintf("%0.01f%%",level*100.0f));
	layout->set_alignment(Pango::ALIGN_CENTER);
	cr->set_source_rgb(0.627,1,0);
	cr->move_to(w/2, 4);
	layout->show_in_cairo_context(cr);

	return true;
}



bool
BlackLevelSelector::on_event(GdkEvent *event)
{
	int x(round_to_int(event->button.x));
	//int y(round_to_int(event->button.y));

    switch(event->type)
    {
	case GDK_MOTION_NOTIFY:
		level=(float)x/(float)get_width();
		if(level<0.0f)level=0.0f;
		if(level>1.0f)level=1.0f;
		signal_value_changed_();
		queue_draw();
		return true;
		break;
	case GDK_BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:
		if(event->button.button==1)
		{
			level=(float)x/(float)get_width();
			if(level<0.0f)level=0.0f;
			if(level>1.0f)level=1.0f;
			signal_value_changed_();
			queue_draw();
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}


void
Dialog_Setup::set_time_format(synfig::Time::Format x)
{
	time_format=x;
	if (x <= Time::FORMAT_VIDEO)
		timestamp_comboboxtext.set_active(0);
	else if (x == (Time::FORMAT_NORMAL))
		timestamp_comboboxtext.set_active(1);
	else if (x == (Time::FORMAT_NORMAL | Time::FORMAT_NOSPACES))
		timestamp_comboboxtext.set_active(2);
	else if (x == (Time::FORMAT_NORMAL | Time::FORMAT_FULL))
		timestamp_comboboxtext.set_active(3);
	else if (x == (Time::FORMAT_NORMAL | Time::FORMAT_NOSPACES | Time::FORMAT_FULL))
		timestamp_comboboxtext.set_active(4);
	else if (x == (Time::FORMAT_FRAMES))
		timestamp_comboboxtext.set_active(5);
	else
		timestamp_comboboxtext.set_active(1);
}

RedBlueLevelSelector::RedBlueLevelSelector():
	level()
{
	set_size_request(-1,24);

	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);
}

RedBlueLevelSelector::~RedBlueLevelSelector()
{
}

bool
RedBlueLevelSelector::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	const int w(get_width()),h(get_height());

	Gdk::Color color;

	int i;

	// Draw the gradient
	for(i=0;i<w;i++)
	{
		float red_blue(((float(i)/float(w)+0.5f)-1.0f)/2.0f+1.0f);
		float blue_red(2.0f-(red_blue));
		if(red_blue>1.0f)red_blue=1.0f;
		if(blue_red>1.0f)blue_red=1.0f;

		cr->set_source_rgb(red_blue, sqrt(min(red_blue,blue_red)), blue_red);
		cr->rectangle(i, 0, 1, h);
		cr->fill();
	}

	// Draw a frame
	cr->set_source_rgb(0,0,0);
	cr->rectangle(0, 0, w-1, h-1);
	cr->stroke();

	// Draw the position of the current value
	i=(int)(((level-1.0f)*2.0f+1.0f-0.5f)*w+0.5);
	cr->set_source_rgb(0,1,0);
	cr->rectangle(i, 1, 1, h-1);
	cr->fill();

	// Print out the value
	Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_pango_context()));
	layout->set_text(etl::strprintf("%0.02f",level));
	layout->set_alignment(Pango::ALIGN_CENTER);
	cr->set_source_rgb(0.627,1,0);
	cr->move_to(w/2, 4);
	layout->show_in_cairo_context(cr);

	return true;
}



bool
RedBlueLevelSelector::on_event(GdkEvent *event)
{
	int x(round_to_int(event->button.x));
	//int y(round_to_int(event->button.y));

    switch(event->type)
    {
	case GDK_MOTION_NOTIFY:
		level=(((float)(x)/(float)get_width()+0.5)-1.0f)/2.0f+1.0f;
		if(level<0.5f)level=0.5f;
		if(level>1.5f)level=1.5f;
		signal_value_changed_();
		queue_draw();
		return true;
		break;
	case GDK_BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:
		if(event->button.button==1)
		{
			level=(((float)(x)/(float)get_width()+0.5)-1.0f)/2.0f+1.0f;
			if(level<0.5f)level=0.5f;
			if(level>1.5f)level=1.5f;
			signal_value_changed_();
			queue_draw();
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}
