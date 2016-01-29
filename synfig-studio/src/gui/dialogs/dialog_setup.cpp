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
**	Copyright (c) 2014 Yu Chen
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

#include "dialogs/dialog_setup.h"

#include <gtkmm/eventbox.h>
#include <gtkmm/scale.h>

#include "canvasview.h"
#include "widgets/widget_enum.h"
#include "autorecover.h"
#include "duck.h"

#include <ETL/stringf>
#include <ETL/misc>

#include <synfig/rendering/renderer.h>

#include <synfigapp/canvasinterface.h>
#include <synfigapp/main.h>

#include <gui/localization.h>

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

Dialog_Setup::Dialog_Setup(Gtk::Window& parent):
	Dialog_Template(parent,_("Synfig Studio Preferences")),
	input_settings(synfigapp::Main::get_selected_input_device()->settings()),
	listviewtext_brushes_path(manage (new Gtk::ListViewText(1, true, Gtk::SELECTION_BROWSE))),
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
	synfig::String interface_str(_("Interface")),
			document_str(_("Document")),
			editing_str(_("Editing")),
			render_str(_("Render")),
			system_str(_("System")),
			gamma_str(_("Gamma"));
	// WARNING FIXED ORDER : the page added to notebook same has treeview
	// Interface
	create_interface_page(add_page(interface_str));
	// Document
	create_document_page(add_page(document_str));
	// Editing
	create_editing_page(add_page(editing_str));
	// Render
	create_render_page(add_page(render_str));
	// System
	PageInfo pi = add_page(system_str);
	create_system_page(pi);
	// Gamma
	create_gamma_page(add_child_page(gamma_str, pi.row));

	show_all_children();
}

Dialog_Setup::~Dialog_Setup()
{
}

void
Dialog_Setup::create_gamma_page(PageInfo pi)
{
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

	int row(1);
#ifndef __APPLE__
	pi.grid->attach(gamma_pattern, 0, row, 2, 1);
	gamma_pattern.set_halign(Gtk::ALIGN_CENTER);
#endif
	Gtk::Scale* scale_gamma_r(manage(new Gtk::Scale(adj_gamma_r)));
	attach_label(pi.grid, _("Red"), ++row);
	pi.grid->attach(*scale_gamma_r, 1, row, 1, 1);
	scale_gamma_r->set_hexpand_set(true);
	adj_gamma_r->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_gamma_r_change));

	Gtk::Scale* scale_gamma_g(manage(new Gtk::Scale(adj_gamma_g)));
	attach_label(pi.grid, _("Green"), ++row);
	pi.grid->attach(*scale_gamma_g, 1, row, 1, 1);
	scale_gamma_g->set_hexpand(true);
	adj_gamma_g->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_gamma_g_change));

	Gtk::Scale* scale_gamma_b(manage(new Gtk::Scale(adj_gamma_b)));
	attach_label(pi.grid, _("Blue"), ++row);
	pi.grid->attach(*scale_gamma_b, 1, row, 1, 1);
	scale_gamma_b->set_hexpand(true);
	adj_gamma_b->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_gamma_b_change));

	attach_label(pi.grid, _("Black Level"), ++row);
	pi.grid->attach(black_level_selector, 1, row, 1, 1);
	black_level_selector.set_hexpand(true);
	black_level_selector.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_black_level_change));

	//attach_label(pi.grid,_("Red-Blue Level"), ++row);
	//pi.grid->attach(red_blue_level_selector, 1, row, 1, 1);
	//red_blue_level_selector.set_hexpand(true);
	//red_blue_level_selector.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_red_blue_level_change));
}

void
Dialog_Setup::create_system_page(PageInfo pi)
{
	/*---------System--------------------*\
	 * UNITS
	 *  Timestamp  [_____________________]
	 *  UnitSystem [_____________________]
	 * RECENTFILE  [_____________________]
	 * AUTOBACKUP  [x| ]
	 *  Interval   [_____________________]
	 * BROWSER     [_____________________]
	 * BRUSH       [_____________________]
	 */

	int row(1);
	// System _ Units section
	attach_label_section(pi.grid, _("Units"), row);
	// System - 0 Timestamp
	timestamp_menu=manage(new class Gtk::Menu());
	attach_label(pi.grid, _("Timestamp"), ++row);
	pi.grid->attach(timestamp_comboboxtext, 1, row, 1, 1);
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

	// System - 1 Unit system
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

		attach_label(pi.grid, _("Unit System"), ++row);
		pi.grid->attach(*widget_enum, 1, row, 1, 1);
		widget_enum->set_hexpand(true);
	}

	// System - Recent files
	attach_label_section(pi.grid, _("Recent Files"), ++row);
	Gtk::SpinButton* recent_files_spinbutton(manage(new Gtk::SpinButton(adj_recent_files,1,0)));
	pi.grid->attach(*recent_files_spinbutton, 1, row, 1, 1);

	// System - Auto backup interval
	attach_label_section(pi.grid, _("Auto Backup"), ++row);
	pi.grid->attach(toggle_autobackup, 1, row, 1, 1);
	toggle_autobackup.set_hexpand(false);
	toggle_autobackup.set_halign(Gtk::ALIGN_START);
	toggle_autobackup.property_active().signal_changed().connect(
			sigc::mem_fun(*this, &Dialog_Setup::on_autobackup_changed));

	attach_label(pi.grid, _("Interval"), ++row);
	pi.grid->attach(auto_backup_interval, 1, row, 1, 1);
	auto_backup_interval.set_hexpand(false);

	// System - Browser_command
	attach_label_section(pi.grid, _("Browser Command"), ++row);
	pi.grid->attach(textbox_browser_command, 1, row, 1, 1);
	textbox_browser_command.set_hexpand(true);

	// System - Brushes path
	{
		attach_label_section(pi.grid, _("Brush Presets Path"), ++row);
		// TODO Check if Gtk::ListStore::create need something like manage
		brushpath_refmodel = Gtk::ListStore::create(prefs_brushpath);
		listviewtext_brushes_path->set_model(brushpath_refmodel);

		Gtk::ScrolledWindow* scroll(manage (new Gtk::ScrolledWindow()));
		scroll->add(*listviewtext_brushes_path);
		listviewtext_brushes_path->set_headers_visible(false);
		pi.grid->attach(*scroll, 1, row, 1,3);

		// Brushes path buttons
		Gtk::Grid* brush_path_btn_grid(manage (new Gtk::Grid()));
		Gtk::Button* brush_path_add(manage (new Gtk::Button()));
		brush_path_add->set_image_from_icon_name("add", Gtk::ICON_SIZE_BUTTON);
		brush_path_btn_grid->attach(*brush_path_add, 0, 0, 1, 1);
		brush_path_add->set_halign(Gtk::ALIGN_END);
		brush_path_add->signal_clicked().connect(
				sigc::mem_fun(*this, &Dialog_Setup::on_brush_path_add_clicked));
		Gtk::Button* brush_path_remove(manage (new Gtk::Button()));
		brush_path_remove->set_image_from_icon_name("remove", Gtk::ICON_SIZE_BUTTON);
		brush_path_btn_grid->attach(*brush_path_remove, 0, 1, 1, 1);
		brush_path_remove->set_halign(Gtk::ALIGN_END);
		brush_path_remove->signal_clicked().connect(
				sigc::mem_fun(*this, &Dialog_Setup::on_brush_path_remove_clicked));
		pi.grid->attach(*brush_path_btn_grid, 0, ++row, 1,1);
		brush_path_btn_grid->set_halign(Gtk::ALIGN_END);
	}
	// System - 11 enable_experimental_features
	//attach_label(pi.grid, _("Experimental features (restart needed)"), ++row);
	//pi.grid->attach(toggle_enable_experimental_features, 1, row, 1, 1);
	//toggle_enable_experimental_features.set_hexpand(true);

#ifdef SINGLE_THREADED
	// System - 12 single_threaded
	attach_label(pi.grid, _("Single thread only (CPUs)"), ++row);
	pi.grid->attach(toggle_single_threaded, 1, row, 1, 1);
	toggle_single_threaded.set_hexpand(true);
#endif

	// signal for change resume
	auto_backup_interval.signal_changed().connect(
			sigc::bind<int>(sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_AUTOBACKUP));
	toggle_autobackup.property_active().signal_changed().connect(
			sigc::bind<int>(sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_AUTOBACKUP));
}

void
Dialog_Setup::create_document_page(PageInfo pi)
{
	/*---------Document------------------*\
	 * NEW CANVAS
	 *  prefix  ___________________
	 *  fps   [_]                    [FPS]
	 *  size
	 *        H[_]xW[_]      [resolutions]
	 *
	 *
	 */

	int row(1);
	attach_label_section(pi.grid, _("New Canvas"), row);
	// Document - Preferred file name prefix
	attach_label(pi.grid, _("Name prefix"), ++row);
	pi.grid->attach(textbox_custom_filename_prefix, 1, row, 6, 1);
	textbox_custom_filename_prefix.set_tooltip_text( _("File name prefix for the new created document"));
	textbox_custom_filename_prefix.set_hexpand(true);

	// TODO add label with some FPS description ( ex : 23.976 FPS->NTSC television , 25 PAL, 48->Film Industrie, 30->cinematic-like appearance ...)
	// Document - New Document FPS
	pref_fps_spinbutton = Gtk::manage(new Gtk::SpinButton(adj_pref_fps, 1, 3));
	attach_label(pi.grid,_("FPS"), ++row);
	pi.grid->attach(*pref_fps_spinbutton, 1, row, 1, 1);
	pref_fps_spinbutton->set_tooltip_text(_("Frames per second of the new created document"));
	pref_fps_spinbutton->set_hexpand(true);

	//Document - Template for predefined fps
	fps_template_combo = Gtk::manage(new Gtk::ComboBoxText());
	pi.grid->attach(*fps_template_combo, 6, row, 1, 1);
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

	//Document - Size
	attach_label(pi.grid, _("Size"),++row);
	// TODO chain icon for ratio / ratio indication (see Widget_RendDesc)
	// Document - New Document X size
	Gtk::Label* label = attach_label(pi.grid,_("Width"), ++row, 1);
	label->set_alignment(Gtk::ALIGN_END, Gtk::ALIGN_CENTER);
	pref_x_size_spinbutton = Gtk::manage(new Gtk::SpinButton(adj_pref_x_size, 1, 0));
	pi.grid->attach(*pref_x_size_spinbutton, 2, row, 1, 1);
	pref_x_size_spinbutton->set_tooltip_text(_("Width in pixels of the new created document"));
	pref_x_size_spinbutton->set_hexpand(true);

	label = attach_label(pi.grid, "X", row, 3, false);// "X" stand for multiply operation
	// NOTA : Use of "section" attributes for BOLDING
	label->set_attributes(section_attrlist);
	label->set_alignment(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER);

	// Document - New Document Y size
	attach_label(pi.grid,_("Height"), row, 4);
	pref_y_size_spinbutton = Gtk::manage(new Gtk::SpinButton(adj_pref_y_size, 1, 0));
	pi.grid->attach(*pref_y_size_spinbutton, 5, row, 1, 1);
	pref_y_size_spinbutton->set_tooltip_text(_("Height in pixels of the new created document"));
	pref_y_size_spinbutton->set_hexpand(true);

	//Document - Template for predefined sizes of canvases.
	size_template_combo = Gtk::manage(new Gtk::ComboBoxText());
	pi.grid->attach(*size_template_combo, 6, row, 1, 1);
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

	fps_template_combo->prepend(DEFAULT_PREDEFINED_FPS);
}

void
Dialog_Setup::create_editing_page(PageInfo pi)
{
	/*---------Editing------------------*\
	 * IMPORTED IMAGE
	 *  [x] Scale to fit
	 * OTHER
	 *  [x] Linear color
	 *  [x] Restrict radius
	 *
	 *
	 */

	int row(1);
	// Editing Imported image section
	attach_label_section(pi.grid, _("Imported Image"), row);

	// Editing - Scaling New Imported Images to Fit Canvas
	pi.grid->attach(toggle_resize_imported_images, 0, ++row, 1, 1);
	toggle_resize_imported_images.set_tooltip_text(_("When you import images, check this option if you want they fit the Canvas size."));
	toggle_resize_imported_images.set_halign(Gtk::ALIGN_END);
	toggle_resize_imported_images.set_hexpand(false);
	Gtk::Label* label = attach_label(pi.grid,_("Scale to fit canvas"), row, 1, false);
	label->set_hexpand(true);

	// Editing Other section
	attach_label_section(pi.grid, _("Other"), ++row);

	// Editing - Visually Linear Color Selection
	pi.grid->attach(toggle_use_colorspace_gamma, 0, ++row, 1, 1);
	toggle_use_colorspace_gamma.set_halign(Gtk::ALIGN_END);
	toggle_use_colorspace_gamma.set_hexpand(false);
	toggle_use_colorspace_gamma.set_tooltip_text(_("Color output is non-linear, if 0 \
is black and 100 is white, then 50 is only about 22 percent of the brightness \
of white, rather than 50% as you might expect. \
Option (ON by default) to make sure that if you ask for 50, you get 50% of the brightness of white."));
	label = attach_label(pi.grid,_("Visually linear color selection"), row, 1, false);
	label->set_hexpand(true);

	// Editing - Restrict Really-valued Handles to Top Right Quadrant
	pi.grid->attach(toggle_restrict_radius_ducks, 0, ++row, 1, 1);
	toggle_restrict_radius_ducks.set_halign(Gtk::ALIGN_END);
	toggle_restrict_radius_ducks.set_hexpand(false);
	toggle_restrict_radius_ducks.set_tooltip_text("Restrict the position of the handle \
(especially for radius) to be in the top right quadrant of the 2D space. Allow to set \
the real value to any number and also easily reach the value of 0.0 just \
dragging the handle to the left bottom part of your 2D space.");
	label = attach_label(pi.grid,_("Restrict really-valued handles to top right quadrant"), row, 1, false);
	label->set_hexpand(true);
}

void
Dialog_Setup::create_render_page(PageInfo pi)
{
	/*---------Render------------------*\
	 *
	 *  sequence separator _________
	 *   navigator [ Legacy ]
	 *   workarea  [ Legacy ]
	 *
	 *
	 */

	int row(1);
	// Render - Image sequence separator
	attach_label(pi.grid, _("Image Sequence Separator String"), row);
	pi.grid->attach(image_sequence_separator, 1, row, 1, 1);
	image_sequence_separator.set_hexpand(true);
	// Render - Navigator
	attach_label(pi.grid, _("Navigator renderer"), ++row);
	pi.grid->attach(navigator_renderer_combo, 1, row, 1, 1);
	// Render - WorkArea
	attach_label(pi.grid, _("WorkArea renderer"), ++row);
	pi.grid->attach(workarea_renderer_combo, 1, row, 1, 1);

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
Dialog_Setup::create_interface_page(PageInfo pi)
{
	/*---------Interface------------------*\
	 * LANGUAGE
	 *  [________________________________]
	 * COLORTHEME
	 *  DarkUI          [x]
	 * HANDLETOOLTIP
	 *  Widthpoint      [x| ]
	 *  Radius          [x| ]
	 *  Transformation  [x| ]
	 *        [x] Name
	 *        [x] Value
	 */

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
	int row(1);
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

	// row is used now for ui construction
	row = 1;
	// Interface - Language section
	attach_label_section(pi.grid, _("Language"), row);
	pi.grid->attach(ui_language_combo, 0, ++row, 4, 1);
	ui_language_combo.set_hexpand(true);
	ui_language_combo.set_margin_start(10);

	// Interface - Color Theme section
	attach_label_section(pi.grid, _("Color Theme"), ++row);
	// Interface - Dark UI theme
	attach_label(pi.grid, _("Dark UI theme (if available)"), ++row);
	pi.grid->attach(toggle_use_dark_theme, 1, row, 1, 1);

	// Interface - Handle tooltip section
	attach_label_section(pi.grid, _("Handle Tooltips"), ++row);
	// Interface - width point tooltip
	attach_label(pi.grid, _("Width point"), ++row);
	pi.grid->attach(toggle_handle_tooltip_widthpoint, 1, row, 1, 1);
	toggle_handle_tooltip_widthpoint.set_halign(Gtk::ALIGN_START);
	toggle_handle_tooltip_widthpoint.set_hexpand(false);
	// Interface - radius tooltip
	attach_label(pi.grid, _("Radius"), ++row);
	pi.grid->attach(toggle_handle_tooltip_radius, 1, row, 1, 1);
	toggle_handle_tooltip_radius.set_halign(Gtk::ALIGN_START);
	toggle_handle_tooltip_radius.set_hexpand(false);
	// Interface - transformation widget tooltip
	attach_label(pi.grid, _("Transformation widget"), ++row);
	pi.grid->attach(toggle_handle_tooltip_transformation, 1, row, 1, 1);
	toggle_handle_tooltip_transformation.set_halign(Gtk::ALIGN_START);
	toggle_handle_tooltip_transformation.set_hexpand(false);
	toggle_handle_tooltip_transformation.property_active().signal_changed().connect(
			sigc::mem_fun(*this, &Dialog_Setup::on_tooltip_transformation_changed));

	Gtk::Grid* toggle_transfo( manage (new Gtk::Grid()));
	toggle_transfo->attach(toggle_handle_tooltip_transfo_name, 0, 1, 1, 1);
	Gtk::Label* tmp = attach_label(toggle_transfo, _("Name"), 1, 1, FALSE);// HANDLE_TOOLTIP_TRANSFO_NAME
	toggle_transfo->attach(toggle_handle_tooltip_transfo_value, 0, 2, 1, 1);
	attach_label(toggle_transfo, _("Value"), 2, 1, FALSE);// HANDLE_TOOLTIP_TRANSFO_VALUE
	pi.grid->attach(*toggle_transfo, 0, ++row, 1,1);
	toggle_transfo->set_halign(Gtk::ALIGN_END);
	toggle_transfo->show_all_children();

	//! change resume signal connexion
	ui_language_combo.signal_changed().connect(
			sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_LANGUAGE));
	//TODO signal change on value
	//toggle_use_dark_theme.signal_changed().connect(
	//		sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_THEME));
	toggle_handle_tooltip_widthpoint.property_active().signal_changed().connect(
			sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_HANDLE_TOOLTIP));
	toggle_handle_tooltip_radius.property_active().signal_changed().connect(
			sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_HANDLE_TOOLTIP));
	toggle_handle_tooltip_transformation.property_active().signal_changed().connect(
			sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_HANDLE_TOOLTIP));
	toggle_handle_tooltip_transfo_name.property_active().signal_changed().connect(
			sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_HANDLE_TOOLTIP));
	toggle_handle_tooltip_transfo_value.property_active().signal_changed().connect(
			sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_HANDLE_TOOLTIP));
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

	//if(pref_modification_flag&CHANGE_AUTOBACKUP)
	// TODO catch change event on auto_backup_interval before use CHANGE_AUTOBACKUP
	{
		// Set the auto backup status
		App::auto_recover->enable(toggle_autobackup.get_active());
		// Set the auto backup interval
		App::auto_recover->set_timeout(auto_backup_interval.get_value() * 1000);
	}

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

	//! TODO Create Change mecanism has Class for being used elsewhere
	// Set the preferred brush path(s)
	if (pref_modification_flag&CHANGE_BRUSH_PATH)
	{
		App::brushes_path.clear();
		int path_count = 0;

		Glib::RefPtr<Gtk::ListStore> liststore = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(
				listviewtext_brushes_path->get_model());

		for(Gtk::TreeIter ui_iter = liststore->children().begin();
				ui_iter!=liststore->children().end();ui_iter++)
		{
			const Gtk::TreeRow row = *(ui_iter);
			// TODO utf_8 path : care to other locale than english ?
			synfig::String path((row[prefs_brushpath.path]));
			input_settings.set_value(strprintf("brush.path_%d", path_count++), path);
			App::brushes_path.insert(path);
		}
		input_settings.set_value("brush.path_count", strprintf("%d", path_count));
	}

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

	// Set the navigator render flag
	App::navigator_renderer=navigator_renderer_combo.get_active_id();

	// Set the workarea render flag
	App::workarea_renderer=workarea_renderer_combo.get_active_id();

	// Set ui language
	if (pref_modification_flag&CHANGE_UI_LANGUAGE)
		App::ui_language = (_lang_codes[ui_language_combo.get_active_row_number()]).c_str();

	if (pref_modification_flag&CHANGE_UI_HANDLE_TOOLTIP)
	{
		// Set ui tooltip on widht point
		App::ui_handle_tooltip_flag=toggle_handle_tooltip_widthpoint.get_active()?Duck::STRUCT_WIDTHPOINT:Duck::STRUCT_NONE;
		// Set ui tooltip on radius
		App::ui_handle_tooltip_flag|=toggle_handle_tooltip_radius.get_active()?Duck::STRUCT_RADIUS:Duck::STRUCT_NONE;
		// Set ui tooltip on transformation
		if(toggle_handle_tooltip_transformation.get_active())
		{
			if(toggle_handle_tooltip_transfo_name.get_active())
			{
				App::ui_handle_tooltip_flag|=Duck::STRUCT_TRANSFORMATION;
			}
			if(toggle_handle_tooltip_transfo_value.get_active())
			{
				App::ui_handle_tooltip_flag|=Duck::STRUCT_TRANSFO_BY_VALUE;
			}
		}
	}


	App::save_settings();
	App::setup_changed();

	if ((pref_modification_flag&CHANGE_BRUSH_PATH) &&
			String(App::get_selected_canvas_view()->get_smach().get_state_name()) == String("brush"))
	{
		App::get_selected_canvas_view()->get_smach().process_event(EVENT_REFRESH_TOOL_OPTIONS);
	}

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
Dialog_Setup::on_autobackup_changed()
{
	auto_backup_interval.set_sensitive(toggle_autobackup.get_active());
	App::auto_recover->enable(toggle_autobackup.get_active());
}

void
Dialog_Setup::on_tooltip_transformation_changed()
{
	toggle_handle_tooltip_transfo_name.set_sensitive(toggle_handle_tooltip_transformation.get_active());
	toggle_handle_tooltip_transfo_value.set_sensitive(toggle_handle_tooltip_transformation.get_active());
}

void
Dialog_Setup::refresh()
{
	refreshing = true;
	pref_modification_flag = CHANGE_NONE;
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
	toggle_autobackup.set_active(App::auto_recover->get_enable());
	// Refresh the value of the auto backup interval
	auto_backup_interval.set_value(App::auto_recover->get_timeout() / 1000);
	auto_backup_interval.set_sensitive(App::auto_recover->get_enable());

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

	// Refresh the brush path(s)
	Glib::RefPtr<Gtk::ListStore> liststore = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(
			listviewtext_brushes_path->get_model());
	//! Keep "brushes_path" preferences entry for backward compatibilty (15/12 - v1.0.3)
	//! Now brush path(s) are hold by input preferences : brush.path_count & brush.path_%d
	String value;
	Gtk::TreeIter ui_iter;
	bool bvalue(input_settings.get_value("brush.path_count",value));
	int i(atoi(value.c_str()));
	App::brushes_path.clear();
	liststore->clear();
	if(!bvalue || (bvalue && i<=0))
	{
		App::brushes_path.insert(App::get_base_path()+ETL_DIRECTORY_SEPARATOR+"share"+ETL_DIRECTORY_SEPARATOR+"synfig"+ETL_DIRECTORY_SEPARATOR+"brushes");
	}
	else
	{
		for(int j = 0; j<i;j++)
		{
			if(input_settings.get_value(strprintf("brush.path_%d", j),value))
			{
				App::brushes_path.insert(value);
			}
		}
	}
	for (set<synfig::String>::iterator setiter = App::brushes_path.begin();
			setiter != App::brushes_path.end(); setiter++)
	{
		ui_iter = liststore->append();
		(*ui_iter)[prefs_brushpath.path]=*setiter;
	}
	// Select the first brush path entry
	//listviewtext_brushes_path->get_selection()->select(
	//		listviewtext_brushes_path->get_model()->children().begin());

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
	toggle_handle_tooltip_widthpoint.set_active(App::ui_handle_tooltip_flag&Duck::STRUCT_WIDTHPOINT);
	toggle_handle_tooltip_radius.set_active(App::ui_handle_tooltip_flag&Duck::STRUCT_RADIUS);
	if((App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFORMATION) ||
			(App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFO_BY_VALUE))
	{
		toggle_handle_tooltip_transformation.set_active(true);
		toggle_handle_tooltip_transfo_name.set_active(
			(App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFORMATION));
		toggle_handle_tooltip_transfo_value.set_active(
			(App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFO_BY_VALUE));
	}
	else
	{
		toggle_handle_tooltip_transformation.set_active(false);
		toggle_handle_tooltip_transfo_name.set_sensitive(false);
		toggle_handle_tooltip_transfo_value.set_sensitive(false);
	}

	refreshing = false;
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

void
Dialog_Setup::on_brush_path_add_clicked()
{
	synfig::String foldername;
	//! TODO dialog_add_folder
	if(App::dialog_open_folder(_("Select a new path for brush"), foldername, MISC_DIR_PREFERENCE, *this))
	{
		// add the new path
		Glib::RefPtr<Gtk::ListStore> liststore = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(
				listviewtext_brushes_path->get_model());
		Gtk::TreeIter it(liststore->append());
		(*it)[prefs_brushpath.path]=foldername;
		// high light it in the brush path list
		listviewtext_brushes_path->scroll_to_row(listviewtext_brushes_path->get_model()->get_path(*it));
		listviewtext_brushes_path->get_selection()->select(listviewtext_brushes_path->get_model()->get_path(*it));

		pref_modification_flag|=CHANGE_BRUSH_PATH;
	}
}

void
Dialog_Setup::on_brush_path_remove_clicked()
{
	Glib::RefPtr<Gtk::ListStore> refLStore = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(listviewtext_brushes_path->get_model());
	refLStore->erase(listviewtext_brushes_path->get_selection()->get_selected());

	pref_modification_flag|=CHANGE_BRUSH_PATH;
	//! TODO if list size == 0: push warning to warning zone
}

void
Dialog_Setup::on_value_change(int valueflag)
{
	if(!refreshing) pref_modification_flag |= valueflag;
}
