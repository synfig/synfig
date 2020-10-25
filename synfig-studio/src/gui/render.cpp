/* === S Y N F I G ========================================================= */
/*!	\file gtkmm/render.cpp
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

#include <gui/render.h>

#include <cerrno>
#include <cstring> // strerror()

#include <glib/gstdio.h>

#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>

#include <gtkmm/frame.h>
#include <gtkmm/grid.h>

#include <gui/app.h>
#include <gui/asyncrenderer.h>
#include <gui/dialogs/dialog_ffmpegparam.h>
#include <gui/dialogs/dialog_spritesheetparam.h>
#include <gui/docks/dockmanager.h>
#include <gui/docks/dock_info.h>
#include <gui/localization.h>
#include <gui/progresslogger.h>

#include <map>

#include <synfig/canvas.h>
#include <synfig/general.h>
#include <synfig/soundprocessor.h>

#endif

// MSVC doesn't define W_OK
#ifndef W_OK
#define W_OK 2
#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

RenderSettings::RenderSettings(Gtk::Window& parent, etl::handle<synfigapp::CanvasInterface> canvas_interface):
	Gtk::Dialog(_("Render Settings"),parent),
	canvas_interface_(canvas_interface),
	adjustment_quality(Gtk::Adjustment::create(3,0,9)),
	entry_quality(adjustment_quality,1,0),
	adjustment_antialias(Gtk::Adjustment::create(1,1,31)),
	entry_antialias(adjustment_antialias,1,0),
	toggle_single_frame(_("Render _current frame only"), true),
	toggle_extract_alpha(_("Extract alpha"), true),
	tparam("mpeg4",6000)
{
	this->set_resizable(false);
	progress_logger.reset(new ProgressLogger());
	tparam.sequence_separator=App::sequence_separator;
	widget_rend_desc.show();
	widget_rend_desc.signal_changed().connect(sigc::mem_fun(*this,&studio::RenderSettings::on_rend_desc_changed));
	widget_rend_desc.set_rend_desc(canvas_interface_->get_canvas()->rend_desc());

	canvas_interface->signal_rend_desc_changed().connect(sigc::mem_fun(*this,&RenderSettings::on_rend_desc_changed));

	comboboxtext_target.append(_("Auto"));
	target_names.push_back(String());
	synfig::Target::Book::iterator iter;
	synfig::Target::Book book(synfig::Target::book());
	for(iter=book.begin();iter!=book.end();iter++)
	{
		comboboxtext_target.append(iter->first);
		target_names.push_back(iter->first);
	}
	comboboxtext_target.set_active(0);
	comboboxtext_target.signal_changed().connect(sigc::mem_fun(this, &RenderSettings::on_comboboxtext_target_changed));

	Gtk::Grid *dialogGrid = manage(new Gtk::Grid());
	dialogGrid->get_style_context()->add_class("dialog-main-content");
	dialogGrid->set_row_spacing(12);
	dialogGrid->set_vexpand(true);
	dialogGrid->set_hexpand(true);
	get_content_area()->pack_start(*dialogGrid,false,false,0);

	Gtk::Button *choose_button(manage(new class Gtk::Button(Gtk::StockID(_("Choose...")))));
	choose_button->show();
	choose_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::RenderSettings::on_choose_pressed));

	tparam_button=manage(new class Gtk::Button(Gtk::StockID(_("Parameters..."))));
	tparam_button->show();
	tparam_button->set_sensitive(false);
	tparam_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::RenderSettings::on_targetparam_pressed));

	Gtk::Frame *target_frame=manage(new Gtk::Frame(_("Target")));
	target_frame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) target_frame->get_label_widget())->set_markup(_("<b>Target</b>"));
	dialogGrid->attach(*target_frame, 0, 0, 1, 1);

	Gtk::Grid *target_grid = manage(new Gtk::Grid());
	target_grid->get_style_context()->add_class("dialog-secondary-content");
	target_grid->set_row_spacing(6);
	target_grid->set_column_spacing(12);
	target_grid->set_vexpand(true);
	target_grid->set_hexpand(true);
	target_frame->add(*target_grid);

	Gtk::Label *filenameLabel = manage(new Gtk::Label(_("_Filename"), true));
	filenameLabel->set_halign(Gtk::ALIGN_START);
	filenameLabel->set_valign(Gtk::ALIGN_CENTER);
	filenameLabel->set_mnemonic_widget(entry_filename);
	entry_filename.set_hexpand();
	target_grid->attach(*filenameLabel, 0, 0, 1, 1);
	target_grid->attach(entry_filename, 1, 0, 1, 1);
	target_grid->attach(*choose_button, 2, 0, 1, 1);

	Gtk::Label *targetLabel = manage(new Gtk::Label(_("_Target"), true));
	targetLabel->set_halign(Gtk::ALIGN_START);
	targetLabel->set_valign(Gtk::ALIGN_CENTER);
	targetLabel->set_mnemonic_widget(comboboxtext_target);
	comboboxtext_target.set_hexpand();
	target_grid->attach(*targetLabel, 0, 1, 1, 1);
	target_grid->attach(comboboxtext_target, 1, 1, 1, 1);
	target_grid->attach(*tparam_button, 2, 1, 1, 1);

	toggle_single_frame.signal_toggled().connect(sigc::mem_fun(*this, &studio::RenderSettings::on_single_frame_toggle));

	Gtk::Frame *settings_frame=manage(new Gtk::Frame(_("Settings")));
	settings_frame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) settings_frame->get_label_widget())->set_markup(_("<b>Settings</b>"));
	dialogGrid->attach(*settings_frame, 0, 1, 1, 1);

	Gtk::Grid *settings_grid = manage(new Gtk::Grid());
	settings_grid->get_style_context()->add_class("dialog-secondary-content");
	settings_grid->set_row_spacing(6);
	settings_grid->set_column_spacing(12);
	settings_grid->set_vexpand(true);
	settings_grid->set_hexpand(true);
	settings_frame->add(*settings_grid);

	Gtk::Label *qualityLabel = manage(new Gtk::Label(_("_Quality"), true));
	qualityLabel->set_halign(Gtk::ALIGN_START);
	qualityLabel->set_valign(Gtk::ALIGN_CENTER);
	qualityLabel->set_mnemonic_widget(entry_quality);
	entry_quality.set_hexpand();
	settings_grid->attach(*qualityLabel, 0, 0, 1, 1);
	settings_grid->attach(entry_quality, 1, 0, 1, 1);

	Gtk::Label *antiAliasLabel = manage(new Gtk::Label(_("_Anti-Aliasing"), true));
	antiAliasLabel->set_halign(Gtk::ALIGN_START);
	antiAliasLabel->set_valign(Gtk::ALIGN_CENTER);
	antiAliasLabel->set_mnemonic_widget(entry_antialias);
	entry_antialias.set_hexpand();
	settings_grid->attach(*antiAliasLabel, 0, 1, 1, 1);
	settings_grid->attach(entry_antialias, 1, 1, 1, 1);

	toggle_single_frame.set_halign(Gtk::ALIGN_START);
	toggle_single_frame.set_valign(Gtk::ALIGN_CENTER);
	settings_grid->attach(toggle_single_frame, 2, 0, 1, 1);
	toggle_single_frame.set_active(false);

	toggle_extract_alpha.set_halign(Gtk::ALIGN_START);
	toggle_extract_alpha.set_valign(Gtk::ALIGN_CENTER);
	settings_grid->attach(toggle_extract_alpha, 2, 1, 1, 1);
	toggle_extract_alpha.set_active(false);

	dialogGrid->attach(widget_rend_desc, 0, 2, 1, 1);


	Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-cancel"))));
	cancel_button->show();
	add_action_widget(*cancel_button,0);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::RenderSettings::on_cancel_pressed));

	Gtk::Button *render_button(manage(new class Gtk::Button(Gtk::StockID(_("Render")))));
	render_button->show();
	add_action_widget(*render_button,1);
	render_button->signal_clicked().connect(sigc::mem_fun(*this, &studio::RenderSettings::on_render_pressed));

	set_title(_("Render Settings")+String(" - ")+canvas_interface_->get_canvas()->get_name());

	widget_rend_desc.enable_time_section();

	set_entry_filename();

	get_content_area()->show_all();
}

RenderSettings::~RenderSettings()
{
}

void
RenderSettings::set_entry_filename()
{
	String filename(filename_sans_extension(canvas_interface_->get_canvas()->get_file_name()));

	// if this isn't the root canvas, append (<canvasname>) to the filename
	etl::handle<synfig::Canvas> canvas = canvas_interface_->get_canvas();
	if (!canvas->is_root())
	{
		if(canvas->get_name().empty())
			filename+=" ("+canvas->get_id()+')';
		else
			filename+=" ("+canvas->get_name()+')';
	}

	if (!is_absolute_path(filename))
		filename = Glib::get_home_dir() + ETL_DIRECTORY_SEPARATOR + filename;
	
	try
	{
		if(!comboboxtext_target.get_active_row_number())
			entry_filename.set_text(filename + ".avi");
		// in case the file was saved and loaded again then .ext should be according to target
		else on_comboboxtext_target_changed();
	}
	catch(...)
	{
		synfig::warning("Averted crash!");
		entry_filename.set_text("output.avi");
	}
}

void
RenderSettings::on_comboboxtext_target_changed()
{
	std::map<std::string,std::string> ext = {{"bmp",".bmp"}, {"dv",".dv"},
					{"ffmpeg",".avi"},{"gif",".gif"},{"imagemagick",".png"}, {"jpeg",".jpg"},
					{"magick++",".gif"},{"mng",".mng"},{"openexr",".exr"},{"png",".png"},
					{"png-spritesheet",".png"},{"ppm",".ppm"}, {"yuv420p",".yuv"}, {"libav",".avi"}};
	int i = comboboxtext_target.get_active_row_number();
	if (i < 0 || i >= (int)target_names.size()) return;
	if (target_name == target_names[i]) return;
	auto itr = ext.find(target_names[i]); 
    // check if target_name is there in map
    if(itr != ext.end())
	{
		String filename = entry_filename.get_text();
		String newfilename = filename.substr(0,filename.find_last_of('.'))+itr->second;
		entry_filename.set_text(newfilename);
	}
	set_target(target_names[i]);
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
	//TODO: Replace this condition
	tparam_button->set_sensitive(!(target_name.compare("ffmpeg") && target_name.compare("png-spritesheet")));
}

void
RenderSettings::on_choose_pressed()
{
	String filename=entry_filename.get_text();
	if(App::dialog_save_file_render("Save Render As", filename, RENDER_DIR_PREFERENCE))
		entry_filename.set_text(filename);

	present();
}

void
RenderSettings::on_targetparam_pressed()
{
	Dialog_TargetParam * dialogtp;
	//TODO: Replace this conditions too
	if (!target_name.compare("ffmpeg"))
		dialogtp = new Dialog_FFmpegParam (*this);
	else if (!target_name.compare("png-spritesheet"))
		dialogtp = new Dialog_SpriteSheetParam (*this);
	else
		return;

	RendDesc rend_desc(widget_rend_desc.get_rend_desc());
	dialogtp->set_desc(rend_desc);
	dialogtp->set_tparam(tparam);
	if(dialogtp->run() == Gtk::RESPONSE_OK)
		tparam = dialogtp->get_tparam();
	delete dialogtp;
}

void
RenderSettings::on_render_pressed()
{
	String filename=entry_filename.get_text();
	tparam.sequence_separator = App::sequence_separator;
	
	if(!check_target_destination())
	{
		present();
		entry_filename.grab_focus();
		return;
	}
	
	hide();
		
	render_passes.clear();
		
	if(toggle_extract_alpha.get_active())
	{
		String filename_alpha(filename_sans_extension(filename)+"-alpha"+filename_extension(filename));

		render_passes.push_back(make_pair(TARGET_ALPHA_MODE_EXTRACT, filename_alpha));
		render_passes.push_back(make_pair(TARGET_ALPHA_MODE_REDUCE, filename));

	} else {
		render_passes.push_back(make_pair(TARGET_ALPHA_MODE_KEEP, filename));
	}

	App::dock_info_->set_n_passes_requested(render_passes.size());
	App::dock_info_->set_n_passes_pending(render_passes.size());
	App::dock_info_->set_render_progress(0.0);
	App::dock_manager->find_dockable("info").present(); //Bring Dock_Info to front

	progress_logger->clear();
	submit_next_render_pass();

	return;
}

bool
RenderSettings::check_target_destination()
{
	String filename=entry_filename.get_text();
	calculated_target_name=target_name;

	if(filename.empty())
	{
		canvas_interface_->get_ui_interface()->error(_("You must supply a filename!"));
		return false;
	}

	// If the target type is not yet defined,
	// try to figure it out from the outfile.
	if(calculated_target_name.empty())
	{
		try
		{
			String ext(filename_extension(filename));
			if (ext.size()) ext=ext.substr(1); // skip initial '.'
			synfig::info("render target filename: '%s'; extension: '%s'", filename.c_str(), ext.c_str());
			if(Target::ext_book().count(ext))
			{
				calculated_target_name=Target::ext_book()[ext];
				synfig::info("'%s' is a known extension - using target '%s'", ext.c_str(), calculated_target_name.c_str());
			}
			else
			{
				calculated_target_name=ext;
				synfig::info("unknown extension");
			}
		}
		catch(std::runtime_error& x)
		{
			canvas_interface_->get_ui_interface()->error(_("Unable to determine proper target from filename."));
			return false;
		}
	}

	if(filename.empty() && calculated_target_name!="null")
	{
		canvas_interface_->get_ui_interface()->error(_("A filename is required for this target"));
		return false;
	}
	
	String extension(filename_extension(filename));
	bool ext_multi_file = false; //output target is an image sequence
	int n_frames_overwrite = 0;
	
	//Retrieve current render settings
	RendDesc rend_desc(widget_rend_desc.get_rend_desc());
	
	if(!toggle_single_frame.get_active() && (calculated_target_name != "png-spritesheet")
			&& (rend_desc.get_frame_end() - rend_desc.get_frame_start()) > 0)
	{
		//Check format which could have an image sequence as output
		//If format is selected in comboboxtext_target
		std::map<std::string,std::string> ext_multi = {{"bmp",".bmp"},
					{"imagemagick",".png"}, {"jpeg",".jpg"},{"mng",".mng"},
					{"openexr",".exr"},{"png",".png"},{"ppm",".ppm"}};
	
		std::map<std::string,std::string>::iterator ext_multi_it;
		ext_multi_it = ext_multi.find(calculated_target_name);
	
		//calculated_target_name is a candidate with known output target (not Auto)
		if(ext_multi_it != ext_multi.end())
		{
			extension = ext_multi_it->second;
			ext_multi_file = true;
		}
		//otherwise Auto is selected
		else
		{
			std::list<std::string> ext_multi_auto = {{".bmp"}, {".png"},
					{".jpg"},{".exr"},{".ppm"}};
	
			ext_multi_file = (find(ext_multi_auto.begin(), ext_multi_auto.end(),
					filename_extension(filename)) != ext_multi_auto.end());
		}

		//Image sequence: filename + sequence_separator + time
		if(ext_multi_file)
			for(int n_frame = rend_desc.get_frame_start();
					n_frame <= rend_desc.get_frame_end();
					n_frame++)
			{
				if(Glib::file_test(filename_sans_extension(filename) +
					tparam.sequence_separator +
					etl::strprintf("%04d", n_frame) +
					extension, Glib::FILE_TEST_EXISTS))
					n_frames_overwrite++;
			}
	}

	String message;
	String details;
	
	if(n_frames_overwrite == 0)
	{
		message = strprintf(_("A file named \"%s\" already exists. "
							"Do you want to replace it?"),
							basename(filename).c_str());
	
		details = strprintf(_("The file already exists in \"%s\". "
							"Replacing it will overwrite its contents."),
							dirname(filename).c_str());
	}
	else
	{
		message = strprintf(_("%d files with the same name already exist. "
							"Do you want to replace them?"),
							n_frames_overwrite);
	
		details = strprintf(_("The files already exist in \"%s\". "
							"Replacing them will overwrite their contents."),
							dirname(filename).c_str());
	}

	//Ask user whether to overwrite file with same name
	if(((Glib::file_test(filename, Glib::FILE_TEST_EXISTS)) || n_frames_overwrite > 0)
			&& !App::dialog_message_2b(
		message,
		details,
		Gtk::MESSAGE_QUESTION,
		_("Use Another Name…"),
		_("Replace")))
		return false;
	
	return true;
}

void
RenderSettings::submit_next_render_pass()
{
	if (render_passes.size()>0) {
		std::pair<TargetAlphaMode,String> pass_info = render_passes.back();
		render_passes.pop_back();

		App::dock_info_->set_n_passes_pending(render_passes.size()); //! Decrease until 0
		App::dock_info_->set_render_progress(0.0); //For this pass
		
		TargetAlphaMode pass_alpha_mode = pass_info.first;
		String pass_filename = pass_info.second;

		Target::Handle target=Target::create(calculated_target_name,pass_filename, tparam);
		if(!target)
		{
			canvas_interface_->get_ui_interface()->error(_("Unable to create target for ")+pass_filename);
			return;
		}
		// Test whether the output file is writable (path exists or has write permit)
		if (g_access(dirname(pass_filename).c_str(), W_OK) == -1) {
			canvas_interface_->get_ui_interface()->error(_("Unable to create file for ")+pass_filename+": "+strerror( errno ));
			return;
		}

		target->set_canvas(canvas_interface_->get_canvas());
		RendDesc rend_desc(widget_rend_desc.get_rend_desc());
		rend_desc.set_antialias((int)adjustment_antialias->get_value());
		rend_desc.set_render_excluded_contexts(false);

		// If we are to only render the current frame
		if(toggle_single_frame.get_active())
			rend_desc.set_time(canvas_interface_->get_time());

		target->set_rend_desc(&rend_desc);
		target->set_quality((int)adjustment_quality->get_value());
		if( !target->init(canvas_interface_->get_ui_interface().get()) ){
			canvas_interface_->get_ui_interface()->error(_("Target initialization failure"));
			return;
		}
		if(pass_alpha_mode!=TARGET_ALPHA_MODE_KEEP)
			target->set_alpha_mode(pass_alpha_mode);

		canvas_interface_->get_ui_interface()->task(strprintf(_("Rendering %s"), pass_filename.c_str()));

		async_renderer=new AsyncRenderer(target, progress_logger.get());
		async_renderer->signal_finished().connect( sigc::mem_fun(*this,&RenderSettings::on_finished));
		async_renderer->start();
		App::dock_info_->set_async_render(async_renderer);
	}
	return;
}

void
RenderSettings::on_finished(std::string error_message)
{
	String text(_("Animation rendered successfully"));
	Real execution_time = async_renderer ? async_renderer->get_execution_time() : 0.0;
	if (execution_time > 0) text += strprintf(" (%f %s)", execution_time, _("sec"));

	bool success = error_message.empty();

	if (success)
		canvas_interface_->get_ui_interface()->task(text);
	canvas_interface_->get_ui_interface()->amount_complete(0,10000);

	bool really_finished = (render_passes.size() == 0); //Must be checked BEFORE submit_next_render_pass();
	
	submit_next_render_pass();

	if (really_finished) { // Because of multi-pass render
		if (App::use_render_done_sound && App::sound_render_done) {
			App::sound_render_done->set_position(Time());
			App::sound_render_done->set_playing(true);
		}
		App::dock_info_->set_render_progress(1.0);
	}

	// Play the sound before show error dialog!
	if (!success)
		canvas_interface_->get_ui_interface()->error(error_message);
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
