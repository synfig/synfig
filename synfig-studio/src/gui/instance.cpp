/* === S Y N F I G ========================================================= */
/*!	\file gui/instance.cpp
**	\brief writeme
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2008, 2011 Carlos López
**	Copyright (c) 2009 Nikita Kitaev
**	Copyright (c) 2012 Konstantin Dmitriev
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

#include <gui/instance.h>

#include <cassert>
#include <cerrno>
#include <fstream>
#include <giomm.h>

#include <gtkmm/actiongroup.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/stock.h>

#include <gui/app.h>
#include <gui/autorecover.h>
#include <gui/canvasview.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/iconcontroller.h>
#include <gui/localization.h>
#include <gui/onemoment.h>
#include <gui/widgets/widget_waypointmodel.h>
#include <gui/workarea.h>

#include <sigc++/sigc++.h>

#include <synfig/canvasfilenaming.h>
#include <synfig/general.h>
#include <synfig/layers/layer_switch.h>
#include <synfig/savecanvas.h>
#include <synfig/synfig_iterations.h>
#include <synfig/valuenode_registry.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/valuenodes/valuenode_duplicate.h>
#include <synfig/widthpoint.h>
#include <synfig/zstreambuf.h>

#include <synfigapp/main.h>

#include <sys/stat.h>

#endif

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

int studio::Instance::instance_count_=0;

/* === P R O C E D U R E S ================================================= */

static Gtk::Image*
create_image_from_icon(const std::string& icon_name, Gtk::IconSize icon_size)
{
	if (icon_name == "image-missing")
		return new Gtk::Image();

#if GTK_CHECK_VERSION(3,24,0)
	return new Gtk::Image(icon_name, icon_size);
#else
	Gtk::Image* image = new Gtk::Image();
	image->set_from_icon_name(icon_name, icon_size);
	return image;
#endif
}

static void get_layer_path(const Layer::Handle& l, const Canvas::LooseHandle root_canvas, std::string& path);

struct ValueNodePathBuilder {

	const Canvas::LooseHandle& canvas;
	std::string path;
	std::string link_name;

	synfig::TraverseCallbackAction
	operator() (ValueNode::Handle vn)
	{
		if (vn->get_type() != type_canvas)
			return TRAVERSE_CALLBACK_SKIP;

		if ((*vn)(canvas->get_time()).get(Canvas::Handle()) != canvas)
			return TRAVERSE_CALLBACK_SKIP;

		if (!link_name.empty())
			path += "/" + link_name;

		if (const auto constvn = ValueNode_Const::Handle::cast_dynamic(vn))
			return TRAVERSE_CALLBACK_ABORT;

		if (const auto linkable = LinkableValueNode::Handle::cast_dynamic(vn)) {
			path += "/" + linkable->get_name();
			for (int i=0; i < linkable->link_count(); i++) {
				auto ith_link = linkable->get_link(i);
				ValueNodePathBuilder builder {canvas};
				builder.link_name = linkable->link_name(i);
				traverse_valuenodes(ith_link, [&builder](ValueNode::Handle vn) -> synfig::TraverseCallbackAction {
					auto action = builder(vn);
					return action;
				});
				if (!builder.path.empty()) {
					path += builder.path;
					break;
				}
			}
			return TRAVERSE_CALLBACK_ABORT; // hacked RECURSIVE above;
		}
		return TRAVERSE_CALLBACK_ABORT;
	}
};

static void
get_canvas_path(Canvas::LooseHandle canvas, const Canvas::LooseHandle root_canvas, std::string& path)
{
	if (canvas->is_inline()) {
		if (auto layer = canvas->find_first_parent_of_type<Layer>()) {
			for (const auto& param_item : layer->get_param_list()) {
				if (param_item.second.get_type() == type_canvas) {
					if (param_item.second.get(Canvas::LooseHandle()) == canvas) {
						std::string dynamic_str;
						auto dynamic_param_iter = layer->dynamic_param_list().find(param_item.first);
						if (dynamic_param_iter != layer->dynamic_param_list().end()) {
							ValueNodePathBuilder vn_path_builder {canvas};
							traverse_valuenodes(dynamic_param_iter->second, [&vn_path_builder](ValueNode::Handle vn) -> TraverseCallbackAction {
								return vn_path_builder(vn);
							});
							dynamic_str = vn_path_builder.path;
						}
						path = strprintf("/param[@name='%s']%s/canvas%s", param_item.first.c_str(), dynamic_str.c_str(), path.c_str());
					}
				}
			}
			get_layer_path(layer, root_canvas, path);
		}
	} else {
		path = canvas->get_relative_id(root_canvas) + path;
	}
}

static void
get_layer_path(const Layer::Handle& l, const Canvas::LooseHandle root_canvas, std::string& path)
{
	if (l) {
		auto canvas = l->get_canvas();
		if (!canvas) {
			warning("layer without canvas!");
		} else {
			path = strprintf("/layer[%i]", canvas->size() - canvas->get_depth(l)) + path;
			get_canvas_path(canvas, root_canvas, path);
		}
	}
}

/* === M E T H O D S ======================================================= */

Instance::Instance(synfig::Canvas::Handle canvas, synfig::FileSystem::Handle container):
	synfigapp::Instance		(canvas, container),
	canvas_tree_store_		(Gtk::TreeStore::create(canvas_tree_model)),
	history_tree_store_		(HistoryTreeStore::create(this)),
	undo_status_(false),
	redo_status_(false)
{
	id_=instance_count_++;

	set_clear_redo_stack_on_new_action(true);

	// Connect up all the signals
	signal_filename_changed().connect(sigc::mem_fun(*this,&studio::Instance::update_all_titles));
	signal_unsaved_status_changed().connect(sigc::hide(sigc::mem_fun(*this,&studio::Instance::update_all_titles)));
	signal_undo_status().connect(sigc::mem_fun(*this,&studio::Instance::set_undo_status));
	signal_redo_status().connect(sigc::mem_fun(*this,&studio::Instance::set_redo_status));

	refresh_canvas_tree();
}

Instance::~Instance()
{
}

// this function returns true if the given extension belongs to image layer type
static bool
is_img(const synfig::String& filename)
{
	static const std::set<String> img_ext{".jpg",".jpeg",".png",".bmp",".gif",".tiff",".tif",".dib",".ppm",".pbm",".pgm",".pnm",".webp"};
	return img_ext.find(Glib::ustring(filesystem::Path::filename_extension(filename)).lowercase()) != img_ext.end();
}

synfig::Layer::Handle
Instance::layer_inside_switch(synfig::Layer_Switch::Handle paste) const
{
	synfig::Layer::Handle child_layer;
	synfig::Canvas::Handle canvas = paste->get_sub_canvas();
	if(canvas)
	{
		synfig::String active_layer = paste->get_param("layer_name").get(synfig::String());

		if (active_layer.empty()) {
			int active_layer_index = paste->get_param("layer_depth").get(int());
			auto layer_iter = canvas->byindex(active_layer_index);
			if (layer_iter != canvas->end())
				child_layer = *layer_iter;
		} else {
			for (IndependentContext i = canvas->get_independent_context(); *i; i++) {
				if((*i)->get_description()==active_layer)
					child_layer = (*i);
			}
		}
	}
	return child_layer;
}


int
Instance::get_visible_canvases()const
{
	int count(0);
	CanvasViewList::const_iterator iter;
	for(iter=canvas_view_list_.begin();iter!=canvas_view_list_.end();++iter)
		if((*iter)->get_visible())
			count++;
	return count;
}

etl::handle<Instance>
Instance::create(synfig::Canvas::Handle canvas, synfig::FileSystem::Handle container)
{
	// Construct a new instance
	etl::handle<Instance> instance(new Instance(canvas, container));

	// Set the user preference regarding redo-history behavior
	{
		bool active = synfigapp::Main::settings().get_value("pref.clear_redo_stack_on_new_action", true);
		instance->set_clear_redo_stack_on_new_action(active);
	}

	// Add the new instance to the application's instance list
	App::instance_list.push_back(instance);

	// Set up the instance with the default UI manager
	instance->synfigapp::Instance::set_ui_interface(App::get_ui_interface());

	// Signal the new instance
	App::signal_instance_created()(instance);

	// And then make sure that is has been selected
	App::set_selected_instance(instance);

	// Create the initial window for the root canvas
	instance->focus(canvas);

	return instance;
}

CanvasView::Handle
Instance::find_canvas_view(Canvas::Handle canvas)
{
	if(!canvas)
		return 0;

	while(canvas->is_inline())
		canvas=canvas->parent();

	CanvasViewList::iterator iter;

	for (iter = canvas_view_list().begin(); iter != canvas_view_list().end(); ++iter)
		if((*iter)->get_canvas()==canvas)
			return *iter;

	return CanvasView::create(this,canvas);
}

void
Instance::focus(Canvas::Handle canvas)
{
	CanvasView::Handle canvas_view = find_canvas_view(canvas);
	assert(canvas_view);
	canvas_view->present();
}

void
Instance::set_undo_status(bool x)
{
	undo_status_=x;
	App::dock_toolbox->refresh();
	signal_undo_redo_status_changed()();
}

void
Instance::set_redo_status(bool x)
{
	redo_status_=x;
	App::dock_toolbox->refresh();
	signal_undo_redo_status_changed()();
}

void
studio::Instance::run_plugin(std::string plugin_id, bool modify_canvas, std::vector<std::string> extra_args)
{
	etl::handle<synfigapp::UIInterface> uim = this->find_canvas_view(this->get_canvas())->get_ui_interface();

	std::unordered_map<std::string,std::string> view_state;

	const auto required_args = App::plugin_manager.get_script_args(plugin_id);

	if (auto canvas_interface = find_canvas_interface(get_canvas())) {

		// Current Time

		view_state["current_time"] = canvas_interface->get_time().get_string(synfig::Time::FORMAT_VIDEO);

		// Selected Layers

		if (auto selection = canvas_interface->get_selection_manager()) {

			// Layers
			if (required_args.selected_layers != PluginScript::ArgNecessity::ARGUMENT_UNUSED) {
				for (const auto& layer : selection->get_selected_layers()) {
					std::string xpath;
					get_layer_path(layer, get_canvas(), xpath);
					auto& selected_layers = view_state["selected_layers"];
					if (!selected_layers.empty())
						selected_layers.append(",");
					selected_layers += '"' + JSON::escape_string(xpath) + '"';
				}
			}
		}
	}

	if (required_args.selected_layers == PluginScript::ArgNecessity::ARGUMENT_MANDATORY && view_state.count("selected_layers") == 0) {
		App::dialog_message_1b(
				"ERROR",
				_("The plugin operation has failed."),
				_("This plugin requires layer(s) to be selected.\nSelect one or more layers and re-run the plugin."),
				_("Close"));
		return;
	}

	if ( modify_canvas )
	{
		String message = strprintf(_("Do you really want to run plugin for file \"%s\"?" ),
					this->get_canvas()->get_name().c_str());

		String details = _("This operation cannot be undone and all undo history will be cleared.");

		int answer = uim->confirmation(
					message,
					details,
					_("Proceed"),
					_("Cancel"),
					synfigapp::UIInterface::RESPONSE_OK);

		if(answer != synfigapp::UIInterface::RESPONSE_OK)
			return;
	}

	OneMoment one_moment;

	Canvas::Handle canvas(this->get_canvas());

	backup(true);
	FileSystemTemporary::Handle temporary_filesystem = FileSystemTemporary::Handle::cast_dynamic(get_canvas()->get_file_system());
	filesystem::Path tmp_filename = temporary_filesystem->get_temporary_directory() / temporary_filesystem->get_temporary_filename_base();

	Time cur_time;
	cur_time = canvas->get_time();

	// Generate temporary file name
	filesystem::Path canvas_file = filesystem::Path(get_canvas()->get_file_name());
	filesystem::Path filename_dir = canvas_file.parent_path();
	if (!filename_dir.is_absolute())
		filename_dir = temporary_filesystem->get_temporary_directory();

	auto temp_lock = FileSystemTemporary::reserve_temporary_filename(filename_dir, canvas_file.filename().u8string(), ".sif");
	if (!temp_lock.second) {
		App::dialog_message_1b("ERROR", _("The plugin operation has failed."), _("Couldn't create a temporary file"), _("Close"));
		return;
	}

	filesystem::Path filename_processed = temp_lock.first;


	if ( modify_canvas )
		close(false);

	if (canvas->use_count() != 1 && modify_canvas) {
		one_moment.hide();
		App::dialog_message_1b(
				"ERROR",
				_("The plugin operation has failed."),
				_("This can be due to current file "
					"being referenced by another composition that is already open, "
					"or because of an internal error in Synfig Studio. Try closing "
					"any compositions that might reference this file and try again, "
					"or restart Synfig Studio."),
				_("Close"));

	} else {

		// Save file copy
		String filename_ext = canvas_file.extension().u8string();
		if ( filename_ext.empty() || ( filename_ext != ".sif" && filename_ext != ".sifz") )
			filename_ext = ".sifz";
		FileSystem::ReadStream::Handle stream_in = temporary_filesystem->get_read_stream("#project"+filename_ext);
		if (!stream_in)
		{
			synfig::error(strprintf("run_plugin(): Unable to open file for reading - %s", temporary_filesystem->get_real_uri("#project"+filename_ext).c_str()));
			FileSystemTemporary::Identifier identifier(temporary_filesystem, filename_processed.u8string());
			if ( !save_canvas(identifier, get_canvas(), true) )
			{
				App::dialog_message_1b(
						"ERROR",
						_("The plugin operation has failed."),
						_("Could not read composition file."),
						_("Close"));
				return;
			}
			temporary_filesystem->save_changes();
		}
		else
		{
			if (filename_ext == ".sifz")
				stream_in = new ZReadStream(stream_in, synfig::zstreambuf::gzip);

			FileSystem::WriteStream::Handle outfile = FileSystemNative::instance()->get_write_stream(filename_processed.u8string());
			*outfile << stream_in->rdbuf();
			outfile.reset();

			stream_in.reset();
		}

		one_moment.hide();
		extra_args.insert(extra_args.begin(), filename_processed.u8string());

		bool result = App::plugin_manager.run(plugin_id, extra_args, view_state);

		if (result && modify_canvas){
			// Restore file copy
			FileSystem::WriteStream::Handle stream = temporary_filesystem->get_write_stream("#project"+filename_ext);
			if (stream && filename_ext == ".sifz")
				stream = FileSystem::WriteStream::Handle(new ZWriteStream(stream));

			if (!stream)
			{
				synfig::error("run_plugin(): Unable to open file for write");
			} else {
				FileSystem::ReadStream::Handle infile = FileSystemNative::instance()->get_read_stream(filename_processed.u8string());
				*stream << infile->rdbuf();
				infile.reset();
				stream.reset();
			}
		}
		FileSystemNative::instance()->file_remove(filename_processed.u8string());
	}

	canvas=0;


	if ( modify_canvas ) {
		one_moment.show();
		bool ok = App::open_from_temporary_filesystem(tmp_filename);
		if (!ok) {
			synfig::error("run_plugin(): Cannot reopen file");
			return;
		}

		etl::handle<Instance> new_instance = App::instance_list.back();
		if (!new_instance) {
			synfig::error("run_plugin(): Cannot retrieve new instance");
		} else {
			// Restore time cursor position
			canvas = new_instance->get_canvas();
			etl::handle<synfigapp::CanvasInterface> new_canvas_interface(new_instance->find_canvas_interface(canvas));
			if (!new_canvas_interface) {
				synfig::error("run_plugin(): Cannot retrieve canvas interface");
			} else {
				new_canvas_interface->set_time(cur_time);
			}
		}
	}
}

bool
studio::Instance::save_as(const synfig::String &file_name)
{
	if(synfigapp::Instance::save_as(file_name))
	{
		// after changing the filename, update the render settings with the new filename
		for (const auto& canvas_view : canvas_view_list())
			canvas_view->render_settings.set_entry_filename();
		App::add_recent_file(etl::handle<Instance>(this));

		// check for unsaved layers (external bitmaps, etc)
		std::vector<Layer::Handle> layers;
		find_unsaved_layers(layers);
		if (!layers.empty())
		{
			String message = _("Current canvas saved, but unable to save data for following layers:");
			String details;
			for(std::vector<Layer::Handle>::const_iterator i = layers.begin(); i != layers.end(); ++i)
			{
				details.append((*i)->get_description());
				if ( Layer_Bitmap::Handle::cast_dynamic(*i)
			  	  && (*i)->get_param_list().count("filename") )
			  	{
			  		ValueBase value = (*i)->get_param("filename");
			  		if (value.same_type_as(String()))
			  			details.append(" (" + value.get(String()) + ")");
			  	}
				details.append("\n");
			}
			App::dialog_message_1b("WARNING", message, "details", _("Close"), details);
		}

		return true;
	}
	return false;
}

void
studio::Instance::open()
{
	App::dialog_open(get_file_name());
}

Instance::Status
studio::Instance::save()
{
	// if we don't have a real filename yet then we need to ask where to save it
	if (!has_real_filename())
	{
		if (dialog_save_as())
			return STATUS_OK;
		else
			return STATUS_CANCEL;
	}

	if (!save_as(get_canvas()->get_file_name()))
	{
		std::string msg(strprintf(_("Unable to save to '%s'"), get_file_name().c_str()));
		App::dialog_message_1b(
				"ERROR",
				msg.c_str(),
				"details",
				_("Close"));
		return STATUS_ERROR;
	}

	return STATUS_OK;
}

// the filename will be set to "Synfig Animation 1" or some such when first created
// and will be changed to an absolute path once it has been saved
// so if it still begins with "Synfig Animation " then we don't have a real filename yet
bool
studio::Instance::has_real_filename()
{
	return get_file_name().find(App::custom_filename_prefix.c_str()) != 0;
}

bool
studio::Instance::dialog_save_as()
{
	filesystem::Path filename = get_file_name();
	Canvas::Handle canvas(get_canvas());

	{
		OneMoment one_moment;
		bool problematic = false;
		canvas->foreach_parent([=, &problematic](Node* node)
		{
			for(;node->parent_count();node=node->get_first_parent())
			{
				Layer::Handle parent_layer(dynamic_cast<Layer*>(node));
				if(parent_layer && parent_layer->get_canvas()->get_root()!=get_canvas())
				{
					//! \todo Fix big involving "Save As" with referenced compositions
					std::string msg(strprintf(_("There is currently a bug when using \"SaveAs\"\n"
						"on a composition that is being referenced by other\n"
						"files that are currently open. Close these\n"
						"other files first before trying to use \"SaveAs\".")));
					App::dialog_message_1b(
							"ERROR",
							msg.c_str(),
							"details",
							_("Close"));

					problematic = true;
					return true;
				}
				if(parent_layer)
					break;
			}
			return false;
		});
		if (problematic)
			return false;
	}

	if (has_real_filename())
		filename = filesystem::absolute(filename);

	// show the canvas' name if it has one, else its ID
	while (App::dialog_save_file((_("Please choose a file name") +
								  String(" (") +
								  (canvas->get_name().empty() ? canvas->get_id() : canvas->get_name()) +
								  ")"),
								 filename, ANIMATION_DIR_PREFERENCE))
	{
		// If the filename still has wildcards, then we should
		// continue looking for the file we want
		std::string base_filename = filename.filename().u8string();
		if (base_filename.find('*') != std::string::npos)
			continue;

		// if file extension is not recognized, then forced to .sifz
		if (filename.extension().empty())
			filename.concat(".sifz");

		canvas->set_name(base_filename);
		// forced to .sifz, the below code is not need anymore
		try
		{
			String ext(filename.extension().u8string());
			// todo: ".sfg" literal and others
			if (ext != ".sif" && ext != ".sifz" && ext != ".sfg" && !App::dialog_message_2b(
				_("Unknown extension"),
				_("You have given the file name an extension which I do not recognize. "
					"Are you sure this is what you want?"),
				Gtk::MESSAGE_QUESTION,
				_("Cancel"),
				_("Sure"))
			)
				continue;
		}
		catch(...)
		{
			continue;
		}

		bool file_exists = FileSystemNative::instance()->is_file(filename.u8string());
		if (!file_exists) {
			if (FileSystemNative::instance()->is_exists(filename.u8string())) {
				std::string msg(strprintf(_("There is a folder with the same name '%s'.\nPlease choose another name"), filename.u8_str()));
				App::dialog_message_1b("ERROR", msg.c_str(), "details", _("Close"));
				continue;
			}
		} else {
			// If the file exists and the user doesn't want to overwrite it, keep prompting for a filename
			std::string message = strprintf(_("A file named \"%s\" already exists. "
							"Do you want to replace it?"),
							filename.filename().u8_str());

			std::string details = strprintf(_("The file already exists in \"%s\". "
							"Replacing it will overwrite its contents."),
							filename.parent_path().u8_str());

			if (!App::dialog_message_2b(message, details, Gtk::MESSAGE_QUESTION, _("Use Another Name…"), _("Replace")))
				continue;
		}

		if (save_as(filename.u8string()))
		{
			synfig::set_file_version(synfig::RELEASE_VERSION_CURRENT);
			return true;
		}
		std::string msg(strprintf(_("Unable to save to '%s'"), filename.u8_str()));
		App::dialog_message_1b("ERROR", msg.c_str(), "details", _("Close"));
	}

	return false;
}


bool
studio::Instance::dialog_export()
{
	filesystem::Path filename = get_file_name();
	Canvas::Handle canvas(get_canvas());

	if (has_real_filename())
		filename = filesystem::absolute(filename);

	// show the canvas' name if it has one, else its ID
	std::string plugin_id = App::dialog_export_file(
		(_("Please choose a file name") +
		String(" (") +
		(canvas->get_name().empty() ? canvas->get_id() : canvas->get_name()) +
		")"),
		filename, ANIMATION_DIR_PREFERENCE
	);
	if ( !plugin_id.empty() )
	{
		run_plugin(plugin_id, false, {filename.u8string()});
		return true;
	}

	return false;
}

void
Instance::update_all_titles()
{
	for (const auto& canvas_view : canvas_view_list())
		canvas_view->update_title();
}

void
Instance::close(bool remove_temporary_files)
{
	// This will increase the reference count so we don't get DELETED
	// until we are ready
	etl::handle<Instance> me(this);

	/*
	We need to hide some panels when instance is closed.
	This is done to avoid the crash when two conditions met:
	 1) the list is scrolled down
	 2) user closes file
	*/
	CanvasView::Handle canvas_view = find_canvas_view(get_canvas());
	Gtk::Widget* tree_view_keyframes = canvas_view->get_ext_widget("keyframes");
	tree_view_keyframes->hide();

	Gtk::Widget* tree_view_params = canvas_view->get_ext_widget("params");
	tree_view_params->hide();

	Gtk::Widget* tree_view_children = canvas_view->get_ext_widget("children");
	tree_view_children->hide();

	// Make sure we aren't selected as the current instance
	if(studio::App::get_selected_instance()==this)
		studio::App::set_selected_instance(0);

	// Remove us from the active instance list
	std::list<etl::handle<studio::Instance> >::iterator iter;
	for (iter = studio::App::instance_list.begin(); iter != studio::App::instance_list.end(); ++iter)
		if(*iter==this)
			break;
	assert(iter!=studio::App::instance_list.end());
	if(iter!=studio::App::instance_list.end())
		studio::App::instance_list.erase(iter);

	// Send out a signal that we are being deleted
	studio::App::signal_instance_deleted()(this);

	// Hide all of the canvas views
	for (const auto& canvas_view : canvas_view_list())
		canvas_view->hide();

	// Consume pending events before deleting the canvas views
	App::process_all_events();

	// Delete all of the canvas views
	canvas_view_list().clear();

	// If there is another open instance to select,
	// go ahead and do so. If not, never mind.
	if(studio::App::instance_list.empty())
	{
		studio::App::set_selected_canvas_view(0);
		studio::App::set_selected_instance(0);
	}
	else if(studio::App::instance_list.size() == 1)
	{
		studio::App::instance_list.front()->canvas_view_list().front()->present();
	}

	if (!remove_temporary_files) {
		FileSystemTemporary::Handle temporary_filesystem = FileSystemTemporary::Handle::cast_dynamic(get_canvas()->get_file_system());
		temporary_filesystem->set_keep_files_when_destroyed(true);
	}
}

void
Instance::insert_canvas(Gtk::TreeRow row, synfig::Canvas::Handle canvas)
{
	CanvasTreeModel canvas_tree_model;
	assert(canvas);

	row[canvas_tree_model.icon_name] = "canvas_icon";
	row[canvas_tree_model.id] = canvas->get_id();
	row[canvas_tree_model.name] = canvas->get_name();
	if(canvas->is_root())
		row[canvas_tree_model.label] = filesystem::Path::basename(canvas->get_file_name());
	else
	if(!canvas->get_id().empty())
		row[canvas_tree_model.label] = canvas->get_id();
	else
	if(!canvas->get_name().empty())
		row[canvas_tree_model.label] = canvas->get_name();
	else
		row[canvas_tree_model.label] = _("[Unnamed]");

	row[canvas_tree_model.canvas] = canvas;
	row[canvas_tree_model.is_canvas] = true;
	row[canvas_tree_model.is_value_node] = false;

	{
		synfig::Canvas::Children::iterator iter;
		synfig::Canvas::Children &children(canvas->children());

		for (iter = children.begin(); iter != children.end(); ++iter)
			insert_canvas(*(canvas_tree_store()->append(row.children())),*iter);
	}
}

void
Instance::refresh_canvas_tree()
{
	canvas_tree_store()->clear();
	Gtk::TreeRow row = *(canvas_tree_store()->prepend());
	insert_canvas(row,get_canvas());
}

void
Instance::revert()
{
	OneMoment one_moment;

	filesystem::Path filename(get_file_name());

	Canvas::Handle canvas(get_canvas());

	close();

	if (canvas->use_count() != 1) {
		one_moment.hide();
		App::dialog_message_1b(
				"ERROR",
				_("The revert operation has failed."),
				_("This can be due to it being referenced by another composition"
					" that is already open, or because of an internal error "
					"in Synfig Studio. Try closing any compositions that "
					"might reference this composition and try again, or "
					"restart Synfig Studio."),
				_("Close"));

		one_moment.show();
	}
	canvas=0;

	App::open(filename);
}

bool
Instance::safe_revert()
{
	if(synfigapp::Instance::get_action_count())
	{
		if (!App::dialog_message_2b(
			_("Revert to saved"),
			_("You will lose any changes you have made since your last save. "
				"Are you sure?"),
			Gtk::MESSAGE_QUESTION,
			_("Cancel"),
			_("Revert"))
		)
			return false;
	}

	revert();
	return true;
}

bool
Instance::safe_close()
{
	CanvasView::Handle canvas_view = find_canvas_view(get_canvas());
	etl::handle<synfigapp::UIInterface> uim = canvas_view->get_ui_interface();

	// if the animation is currently playing, closing the window will cause a crash,
	// so don't allow it
	if (canvas_view->is_playing())
	{
		canvas_view->present();
		App::dialog_message_1b(
				"ERROR",
				_("The animation is currently playing so the window cannot be closed."),
				"details",
				_("Thanks!"));

		return false;
	}
	if(get_action_count())
		do
		{
			std::string message = strprintf(_("Save changes to document \"%s\" before closing?"),
					filesystem::Path::basename(get_file_name()).c_str() );

			std::string details = (_("If you don't save, changes from the last time you saved "
					"will be permanently lost."));

			int answer=uim->yes_no_cancel(
						message,
						details,
						_("Close without Saving"),
						_("Cancel"),
						_("Save"),
						true,
						synfigapp::UIInterface::RESPONSE_YES
			);

			if(answer == synfigapp::UIInterface::RESPONSE_YES){
				enum Status status = save();
				if (status == STATUS_OK) break;
				else if (status == STATUS_CANCEL) return false;
			}
			if(answer==synfigapp::UIInterface::RESPONSE_NO)
				break;
			if(answer==synfigapp::UIInterface::RESPONSE_CANCEL)
				return false;
		} while (true);

	close();

	return true;
}

void
Instance::add_actions_to_group(const Glib::RefPtr<Gio::SimpleActionGroup>& action_group, const synfigapp::Action::ParamList& param_list, synfigapp::Action::Category category) const
{
	synfigapp::Action::CandidateList candidate_list;

	candidate_list = synfigapp::Action::compile_visible_candidate_list(param_list, category);

	candidate_list.sort();

	for (const auto& action : candidate_list) {
		action_group->add_action("action-" + action.name,
						sigc::bind(
							sigc::bind(
								sigc::mem_fun(
									*const_cast<studio::Instance*>(this),
									&studio::Instance::process_action
								),
							param_list
							),
						action.name
						)
		);
	}
}

void
Instance::add_actions_to_group_and_menu(
		const Glib::RefPtr<Gio::SimpleActionGroup>& action_group,
		const std::string& action_group_name,
		const Glib::RefPtr<Gio::Menu>& menu,
		const synfigapp::Action::ParamList& param_list,
		synfigapp::Action::Category category) const
{
	const std::string symbolic_suffix = ""; // App::use-symbolic-icons ? "-symbolic" : "";

	synfigapp::Action::CandidateList candidate_list;

	candidate_list=compile_visible_candidate_list(param_list,category);

	candidate_list.sort();

	for(const auto& item : candidate_list) {
		auto a = action_group->add_action("action-" + item.name,
			sigc::bind(
				sigc::bind(
					sigc::mem_fun(
						*const_cast<studio::Instance*>(this),
						&studio::Instance::process_action
					),
					param_list
				),
				item.name
			)
		);

		auto menu_item = Gio::MenuItem::create(item.local_name, action_group_name + ".action-" + item.name);
		const std::string icon_name = get_action_icon_name(item);
		if (!icon_name.empty())
			menu_item->set_icon(Gio::ThemedIcon::create(icon_name + symbolic_suffix));
		menu->append_item(menu_item);
	}
}

void
Instance::add_actions_to_menu(Gtk::Menu *menu, const synfigapp::Action::ParamList &param_list,synfigapp::Action::Category category)const
{
	synfigapp::Action::CandidateList candidate_list;
	synfigapp::Action::CandidateList::iterator iter;

	candidate_list = synfigapp::Action::compile_visible_candidate_list(param_list, category);

	candidate_list.sort();

	if(candidate_list.empty())
		synfig::warning("%s:%d Action CandidateList is empty!", __FILE__, __LINE__);

	for(iter=candidate_list.begin();iter!=candidate_list.end();++iter)
	{
		bool is_item_already_in_menu = false;
		std::vector<Gtk::Widget*> children = menu->get_children();
		for (const Gtk::Widget* child : children) {
			if (const Gtk::MenuItem* menu_item = dynamic_cast<const Gtk::MenuItem*>(child)) {
				if (menu_item->get_label() == iter->local_name) {
					is_item_already_in_menu = true;
					break;
				}
			}
		}

		// Maybe the action supports multiple ducks or layers and is already listed in menu
		if (is_item_already_in_menu)
			continue;

		Gtk::MenuItem *item = Gtk::manage(new Gtk::ImageMenuItem(
			*Gtk::manage(create_image_from_icon(get_action_icon_name(*iter), Gtk::ICON_SIZE_MENU)),
			iter->local_name ));
		item->signal_activate().connect(
			sigc::bind(
				sigc::bind(
					sigc::mem_fun(
						*const_cast<studio::Instance*>(this),
						&studio::Instance::process_action ),
					param_list ),
				iter->name ));
		item->show_all();
		menu->append(*item);
	}
}

void
Instance::add_actions_to_menu(Gtk::Menu *menu, const synfigapp::Action::ParamList &param_list,const synfigapp::Action::ParamList &param_list2,synfigapp::Action::Category category)const
{
	synfigapp::Action::CandidateList candidate_list;
	synfigapp::Action::CandidateList candidate_list2;

	synfigapp::Action::CandidateList::iterator iter;

	candidate_list = synfigapp::Action::compile_visible_candidate_list(param_list, category);
	candidate_list2 = synfigapp::Action::compile_visible_candidate_list(param_list2, category);

	candidate_list.sort();

	if(candidate_list.empty())
		synfig::warning("%s:%d Action CandidateList is empty!", __FILE__, __LINE__);
	if(candidate_list2.empty())
		synfig::warning("%s:%d Action CandidateList2 is empty!", __FILE__, __LINE__);

	// Separate out the candidate lists so that there are no conflicts
	for(iter=candidate_list.begin();iter!=candidate_list.end();++iter)
	{
		synfigapp::Action::CandidateList::iterator iter2(candidate_list2.find(iter->name));
		if(iter2!=candidate_list2.end())
			candidate_list2.erase(iter2);
	}

	for(iter=candidate_list2.begin();iter!=candidate_list2.end();++iter)
	{
		Gtk::MenuItem *item = Gtk::manage(new Gtk::ImageMenuItem(
			*Gtk::manage(create_image_from_icon(get_action_icon_name(*iter), Gtk::ICON_SIZE_MENU)),
			iter->local_name ));
		item->signal_activate().connect(
			sigc::bind(
				sigc::bind(
					sigc::mem_fun(
						*const_cast<studio::Instance*>(this),
						&studio::Instance::process_action ),
					param_list2 ),
				iter->name ));
		item->show_all();
		menu->append(*item);
	}

	for(iter=candidate_list.begin();iter!=candidate_list.end();++iter)
	{
		Gtk::MenuItem *item = Gtk::manage(new Gtk::ImageMenuItem(
			*Gtk::manage(create_image_from_icon(get_action_icon_name(*iter), Gtk::ICON_SIZE_MENU)),
			iter->local_name ));
		item->signal_activate().connect(
			sigc::bind(
				sigc::bind(
					sigc::mem_fun(
						*const_cast<studio::Instance*>(this),
						&studio::Instance::process_action ),
					param_list ),
				iter->name ));
		item->show_all();
		menu->append(*item);
	}
}

void
Instance::process_action(synfig::String name, synfigapp::Action::ParamList param_list)
{
	DEBUG_LOG("SYNFIG_DEBUG_ACTIONS",
		"%s:%d process_action: '%s'", __FILE__, __LINE__, name.c_str());

	assert(synfigapp::Action::book().count(name));

	synfigapp::Action::BookEntry entry(synfigapp::Action::book().find(name)->second);

	synfigapp::Action::Handle action(entry.factory());

	if(!action)
	{
		synfig::error("Bad Action");
		return;
	}

	action->set_param_list(param_list);

	synfigapp::Action::ParamVocab param_vocab(entry.get_param_vocab());
	synfigapp::Action::ParamVocab::const_iterator iter;

	for(iter=param_vocab.begin();iter!=param_vocab.end();++iter)
	{
		if(!iter->get_mutual_exclusion().empty() && param_list.count(iter->get_mutual_exclusion()))
			continue;

		// If the parameter is optionally user-supplied,
		// and has not been already provided in the param_list,
		// then we should go ahead and see if we can
		// provide that data.
		if(iter->get_user_supplied() && param_list.count(iter->get_name())==0)
		{
			switch(iter->get_type())
			{
			case synfigapp::Action::Param::TYPE_STRING:
			{
				String str;
				if(iter->get_value_provided())
				{
					synfigapp::Action::Param param;
					if (action->get_param(iter->get_name(),param))
					{
						if(param.get_type()==synfigapp::Action::Param::TYPE_STRING)
							str = param.get_string();
					}
				}
				String button2 = _("Export");
				String label = _("Name: ");

				// export and rename value dialog
				if (entry.name == "ValueNodeRename")
					button2 = _("Rename");
				// set layer description dialog
				else if (entry.name == "LayerSetDesc")
				{
					button2 = _("OK");
					label = _("Description: ");
				} else if (entry.name == "GroupAddLayers") {
					button2 = _("Add");
				}

				if(entry.name == "GroupAddLayers"){
					if(!studio::App::dialog_sets_entry(entry.local_name,
							label,
							str,
							_("Cancel"),
							button2))
						return;
				} else {
					if(!studio::App::dialog_entry(entry.local_name,
							label,
							str,
							_("Cancel"),
							button2))
						return;
				}
				action->set_param(iter->get_name(),str);
				break;
			}
			default:
				synfig::error("Unsupported user-supplied action parameter");
				return;
				break;
			}
		}
	}

	if(!action->is_ready())
	{
		synfig::error("Action not ready");
		return;
	}

	perform_action(action);
}

void
Instance::make_param_menu(Gtk::Menu *menu,synfig::Canvas::Handle canvas, synfigapp::ValueDesc value_desc, float location, bool bezier)
{
	Gtk::Menu& parammenu(*menu);
	synfigapp::ValueDesc value_desc2(value_desc);
	etl::handle<synfigapp::CanvasInterface> canvas_interface(find_canvas_interface(canvas));
	CanvasView::Handle canvas_view(find_canvas_view(canvas));
	if(!canvas_interface)
		return;

	Gtk::MenuItem* item = nullptr;

	synfigapp::Action::ParamList param_list,param_list2;
	param_list=canvas_interface->generate_param_list(value_desc);
	param_list.add("origin",location);

#ifdef BLINEPOINT_MENU_IS_VERTEX_MENU
	if(value_desc.get_value_type()==type_bline_point && value_desc.is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		param_list2=canvas_interface->generate_param_list(
			synfigapp::ValueDesc(
				ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node())
				,ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node())
                                                           ->get_link_index_from_name("point")
			)
		);
		param_list2.add("origin",location);
	}
#endif	// BLINEPOINT_MENU_IS_VERTEX_MENU

	// Populate the convert menu by looping through
	// the ValueNode book and find the ones that are
	// relevant.

	// show the 'Convert' sub-menu if this valuedesc is anything other than either:
	//   the 'Index' parameter of a Duplicate layer
	// or
	//   a Duplicate ValueNode whose parent is not a (layer or ValueNode)
	if (!((value_desc.parent_is_layer() &&
		   value_desc.get_layer()->get_name() == "duplicate" &&
		   value_desc.get_param_name() == "index") ||
		  (value_desc.is_value_node() &&
		   ValueNode_Duplicate::Handle::cast_dynamic(value_desc.get_value_node()) &&
		   !(value_desc.parent_is_layer() ||
			 value_desc.parent_is_value_node()))))
	{
		Gtk::Menu *convert_menu=Gtk::manage(new Gtk::Menu());
		ValueNodeRegistry::Book::const_iterator iter;
		for(iter=ValueNodeRegistry::book().begin();iter!=ValueNodeRegistry::book().end();++iter)
		{
			if(iter->second.check_type(value_desc.get_value_type()))
			{
				item = Gtk::manage(new Gtk::MenuItem(iter->second.get_local_name()));
				item->signal_activate().connect(
					sigc::hide_return(
						sigc::bind(
							sigc::bind(
								sigc::mem_fun(*canvas_interface.get(),&synfigapp::CanvasInterface::convert),
								iter->first ),
							value_desc )));
				item->show();
				convert_menu->append(*item);
			}
		}

		item = Gtk::manage(new Gtk::ImageMenuItem(
			*Gtk::manage(create_image_from_icon("gtk-convert", Gtk::ICON_SIZE_MENU )),
			_("Convert") ));
		item->set_submenu(*convert_menu);
		item->show();
		parammenu.append(*item);
	}

	// Interpolation menu: Show it only if
	// the value description is constant or animated (layer parameter or exported) and not static
	// and value is not canvas type
	if (((value_desc.is_const() && !value_desc.get_static()) || value_desc.is_animated())
	&&
		value_desc.get_value_type()!= type_canvas
		)
	{
		Gtk::Menu *param_interpolation_menu=Gtk::manage(new Gtk::Menu());
		synfigapp::Action::ParamList param_list;
		param_list.add("canvas", get_canvas());
		param_list.add("value_desc", value_desc);
		param_list.add("canvas_interface",canvas_interface);

		/////// Default
		param_list.add("new_value", INTERPOLATION_UNDEFINED);
		item = Gtk::manage(new Gtk::MenuItem(_("Default")));
		item->signal_activate().connect(
			sigc::bind(
				sigc::bind(
					sigc::mem_fun(*const_cast<studio::Instance*>(this),&studio::Instance::process_action),
					param_list ),
				"ValueDescSetInterpolation" ));
		item->show();
		param_interpolation_menu->append(*item);
		param_list.erase("new_value");

		#define ADD_IMAGE_MENU_ITEM(Interpolation, IconName, Text) \
		param_list.add("new_value", Interpolation); \
		item = Gtk::manage(new Gtk::ImageMenuItem( \
			*Gtk::manage(create_image_from_icon(IconName, Gtk::IconSize::from_name("synfig-small_icon"))), \
			_(Text) )); \
		item->signal_activate().connect( \
			sigc::bind( \
				sigc::bind( \
					sigc::mem_fun(*const_cast<studio::Instance*>(this),&studio::Instance::process_action), \
					param_list ), \
				"ValueDescSetInterpolation" )); \
		item->show_all(); \
		param_interpolation_menu->append(*item); \
		param_list.erase("new_value");


		ADD_IMAGE_MENU_ITEM(INTERPOLATION_CLAMPED, "interpolation_type_clamped_icon", _("Clamped"));
		ADD_IMAGE_MENU_ITEM(INTERPOLATION_TCB, "interpolation_type_tcb_icon", _("TCB"));
		ADD_IMAGE_MENU_ITEM(INTERPOLATION_CONSTANT, "interpolation_type_const_icon", _("Constant"));
		ADD_IMAGE_MENU_ITEM(INTERPOLATION_HALT, "interpolation_type_ease_icon", _("Ease"));
		ADD_IMAGE_MENU_ITEM(INTERPOLATION_LINEAR, "interpolation_type_linear_icon", _("Linear"));

		#undef CREATE_IMAGE
		#undef ADD_IMAGE_MENU_ITEM

		item = Gtk::manage(new Gtk::MenuItem(_("Interpolation")));
		item->set_submenu(*param_interpolation_menu);
		item->show();
		parammenu.append(*item);
	}

	synfigapp::Action::Category categories = synfigapp::Action::CATEGORY_VALUEDESC|synfigapp::Action::CATEGORY_VALUENODE;
	if (bezier)
		categories = categories|synfigapp::Action::CATEGORY_BEZIER;

	const DuckList selected_ducks(find_canvas_view(canvas)->get_work_area()->get_selected_ducks());
	for(DuckList::const_iterator iter=selected_ducks.begin();iter!=selected_ducks.end();++iter)
	{
		synfigapp::ValueDesc selected_value_desc((*iter)->get_value_desc());
		if(selected_value_desc.is_valid() && value_desc != selected_value_desc)
			param_list.add("selected_value_desc",selected_value_desc);
	}

	if (std::string("bone") == canvas_view->get_smach().get_state_name()) {
		param_list.add("active_bone", canvas_view->get_work_area()->get_active_bone_value_node());
	}

	if(param_list2.empty())
		add_actions_to_menu(&parammenu, param_list,categories);
	else
		add_actions_to_menu(&parammenu, param_list2,param_list,categories);

	if((value_desc2.get_value_type()==type_bline_point || value_desc2.get_value_type()==type_width_point)
	 && value_desc2.is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc2.get_value_node()))
	{
		// the index=0 is position for widthpoint and vertex for blinepoint
		value_desc2=synfigapp::ValueDesc(ValueNode_Composite::Handle::cast_dynamic(value_desc2.get_value_node()),0);
	}

	if(value_desc2.is_value_node() && ValueNode_Animated::Handle::cast_dynamic(value_desc2.get_value_node()))
	{
		ValueNode_Animated::Handle value_node(ValueNode_Animated::Handle::cast_dynamic(value_desc2.get_value_node()));

		try
		{
			// try to find a waypoint at the current time - if we
			// can't, we don't want the menu entry - an exception is thrown
			WaypointList::iterator iter(value_node->find(canvas->get_time()));
			std::set<synfig::Waypoint, std::less<UniqueID> > waypoint_set;
			waypoint_set.insert(*iter);

			item = Gtk::manage(new Gtk::MenuItem(_("Edit Waypoint")));
			item->signal_activate().connect(
				sigc::bind(
					sigc::bind(
						sigc::bind(
							sigc::mem_fun(*find_canvas_view(canvas),&studio::CanvasView::on_waypoint_clicked_canvasview),
							-1 ),
						waypoint_set ),
					value_desc2 ));
			item->show();
			parammenu.append(*item);
		}
		catch(...)
		{ }
	}
	//// Add here the rest of actions here for specific single value descriptions
	//
	// Specific actions for Widthpoints (Composite)
	if(value_desc.get_value_type()==type_width_point && value_desc.is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueNode_Composite::Handle wpoint_composite(ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()));
		synfigapp::Action::ParamList param_list;
		param_list.add("canvas",canvas);
		param_list.add("canvas_interface",canvas_interface);
		param_list.add("time",canvas_interface->get_time());
		item = Gtk::manage(new Gtk::SeparatorMenuItem());
		item->show();
		parammenu.append(*item);


		#define ADD_IMAGE_MENU_ITEM(Type, icon_name, Text) \
			param_list.add("new_value", ValueBase((int)WidthPoint::Type)); \
			item = Gtk::manage(new Gtk::ImageMenuItem( \
				*Gtk::manage(create_image_from_icon(icon_name, Gtk::IconSize::from_name("synfig-small_icon"))), \
				_(Text) )); \
			item->signal_activate().connect( \
				sigc::bind( \
					sigc::bind( \
						sigc::mem_fun(*const_cast<studio::Instance*>(this),&studio::Instance::process_action), \
						param_list ), \
					"ValueDescSet" )); \
			item->show_all(); \
			parammenu.append(*item); \
			param_list.erase("new_value");

		////// Before //////////////////
		param_list.add("value_desc",synfigapp::ValueDesc(wpoint_composite, wpoint_composite->get_link_index_from_name("side_before")));

		ADD_IMAGE_MENU_ITEM(TYPE_INTERPOLATE, "action_interpolate_interpolation_icon", _("Cusp Before: Interpolate"))
		ADD_IMAGE_MENU_ITEM(TYPE_ROUNDED, "action_rounded_interpolation_icon", _("Cusp Before: Rounded"))
		ADD_IMAGE_MENU_ITEM(TYPE_SQUARED, "action_squared_interpolation_icon", _("Cusp Before: Squared"))
		ADD_IMAGE_MENU_ITEM(TYPE_PEAK, "action_peak_interpolation_icon", _("Cusp Before: Peak"))
		ADD_IMAGE_MENU_ITEM(TYPE_FLAT, "action_flat_interpolation_icon", _("Cusp Before: Flat"))
		ADD_IMAGE_MENU_ITEM(TYPE_INNER_ROUNDED, "action_innerrounded_interpolation_icon", _("Cusp Before: Inner Rounded"))
		ADD_IMAGE_MENU_ITEM(TYPE_INNER_PEAK, "action_offpeak_interpolation_icon", _("Cusp Before: Off-Peak"))

		///////
		item = Gtk::manage(new Gtk::SeparatorMenuItem());
		item->show();
		parammenu.append(*item);

		////// After ///////////////////////
		param_list.erase("value_desc");
		param_list.add("value_desc",synfigapp::ValueDesc(wpoint_composite, wpoint_composite->get_link_index_from_name("side_after")));

		ADD_IMAGE_MENU_ITEM(TYPE_INTERPOLATE, "action_interpolate_interpolation_icon", _("Cusp After: Interpolate"))
		ADD_IMAGE_MENU_ITEM(TYPE_ROUNDED, "action_rounded_interpolation_icon", _("Cusp After: Rounded"))
		ADD_IMAGE_MENU_ITEM(TYPE_SQUARED, "action_squared_interpolation_icon", _("Cusp After: Squared"))
		ADD_IMAGE_MENU_ITEM(TYPE_PEAK, "action_peak_interpolation_icon", _("Cusp After: Peak"))
		ADD_IMAGE_MENU_ITEM(TYPE_FLAT, "action_flat_interpolation_icon", _("Cusp After: Flat"))
		ADD_IMAGE_MENU_ITEM(TYPE_INNER_ROUNDED, "action_innerrounded_interpolation_icon", _("Cusp After: Inner Rounded"))
		ADD_IMAGE_MENU_ITEM(TYPE_INNER_PEAK, "action_offpeak_interpolation_icon", _("Cusp After: Off-Peak"))

		///////
		item = Gtk::manage(new Gtk::SeparatorMenuItem());
		item->show();
		parammenu.append(*item);

		/////// Set WIDTH to ZERO
		param_list.erase("value_desc");
		param_list.erase("new_value");
		param_list.add("value_desc",synfigapp::ValueDesc(wpoint_composite, wpoint_composite->get_link_index_from_name("width")));
		param_list.add("new_value", ValueBase(Real(0.0)));
		item = Gtk::manage(new Gtk::MenuItem(_("Set width to zero")));
		item->signal_activate().connect(
			sigc::bind(
				sigc::bind(
					sigc::mem_fun(*const_cast<studio::Instance*>(this),&studio::Instance::process_action),
					param_list ),
				"ValueDescSet" ));
		item->show();
		parammenu.append(*item);

		/////// Set WIDTH to DEFAULT
		param_list.erase("new_value");
		param_list.add("value_desc",synfigapp::ValueDesc(wpoint_composite, wpoint_composite->get_link_index_from_name("width")));
		param_list.add("new_value", ValueBase(Real(1.0)));
		item = Gtk::manage(new Gtk::MenuItem(_("Set width to default")));
		item->signal_activate().connect(
			sigc::bind(
				sigc::bind(
					sigc::mem_fun(*const_cast<studio::Instance*>(this),&studio::Instance::process_action),
					param_list ),
				"ValueDescSet" ));
		item->show();
		parammenu.append(*item);
	}
}

void
edit_several_waypoints(CanvasView::Handle canvas_view, std::list<synfigapp::ValueDesc> value_desc_list)
{
	etl::handle<synfigapp::CanvasInterface> canvas_interface(canvas_view->canvas_interface());

	Gtk::Dialog dialog(
		_("Edit Multiple Waypoints"),
		true
	);

	Widget_WaypointModel widget_waypoint_model;
	widget_waypoint_model.show();

	dialog.get_content_area()->pack_start(widget_waypoint_model);

	dialog.add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
	dialog.add_button(_("Apply"), Gtk::RESPONSE_APPLY);
	dialog.show();

	if(dialog.run()!=Gtk::RESPONSE_APPLY || widget_waypoint_model.get_waypoint_model().is_trivial())
		return;
	synfigapp::Action::PassiveGrouper group(canvas_interface->get_instance().get(),_("Set Waypoints"));

	std::list<synfigapp::ValueDesc>::iterator iter;
	for(iter=value_desc_list.begin();iter!=value_desc_list.end();++iter)
	{
		synfigapp::ValueDesc value_desc(*iter);

		if(!value_desc.is_valid())
			continue;

		ValueNode_Animated::Handle value_node;
		// Check if we are dealing with a BLinePoint or a WidthPoint value desc
		// If so, then change the value desc to be the position or the point.

		if(value_desc.is_value_node() && value_desc.parent_is_linkable_value_node())
		{
			synfig::ValueNode_Composite::Handle compo(synfig::ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node()));
			if(compo && compo->get_type() == type_width_point)
			{
				value_desc=synfigapp::ValueDesc(compo, compo->get_link_index_from_name("position"));
				//value_node=ValueNode_Animated::Handle::cast_dynamic(compo->get_link(compo->get_link_index_from_name("position")));
			}
			if(compo && compo->get_type() == type_bline_point)
			{
				value_desc=synfigapp::ValueDesc(compo, compo->get_link_index_from_name("point"));
				//value_node=ValueNode_Animated::Handle::cast_dynamic(compo->get_link(compo->get_link_index_from_name("point")));
			}
		}

		// If this value isn't a ValueNode_Animated, but
		// it is somewhat constant, then go ahead and convert
		// it to a ValueNode_Animated.
		if(!value_desc.is_value_node() || ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueBase value;
			if(value_desc.is_value_node())
				value=ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node())->get_value();
			else
				value=value_desc.get_value();

			value_node=ValueNode_Animated::create(value,canvas_interface->get_time());

			synfigapp::Action::Handle action;

			if(!value_desc.is_value_node())
			{
				action=synfigapp::Action::create("ValueDescConnect");
				action->set_param("dest",value_desc);
				action->set_param("src",ValueNode::Handle(value_node));
			}
			else
			{
				action=synfigapp::Action::create("ValueNodeReplace");
				action->set_param("dest",value_desc.get_value_node());
				action->set_param("src",ValueNode::Handle(value_node));
			}

			action->set_param("canvas",canvas_view->get_canvas());
			action->set_param("canvas_interface",canvas_interface);

			if(!canvas_interface->get_instance()->perform_action(action))
			{
				canvas_view->get_ui_interface()->error(_("Unable to convert to animated waypoint"));
				group.cancel();
				return;
			}
		}
		else
		{
			if(value_desc.is_value_node())
				value_node=ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node());
		}

		if(value_node)
		{
			synfigapp::Action::Handle action(synfigapp::Action::create("WaypointSetSmart"));

			if(!action)
			{
				canvas_view->get_ui_interface()->error(_("Unable to find WaypointSetSmart action"));
				group.cancel();
				return;
			}

			action->set_param("canvas",canvas_view->get_canvas());
			action->set_param("canvas_interface",canvas_interface);
			action->set_param("value_node",ValueNode::Handle(value_node));
			action->set_param("time",canvas_interface->get_time());
			action->set_param("model",widget_waypoint_model.get_waypoint_model());

			if(!canvas_interface->get_instance()->perform_action(action))
			{
				canvas_view->get_ui_interface()->error(_("Unable to set a specific waypoint"));
				group.cancel();
				return;
			}
		}
		else
		{
			//get_canvas_view()->get_ui_interface()->error(_("Unable to animate a specific valuedesc"));
			//group.cancel();
			//return;
		}

	}
}

void
Instance::make_param_menu(Gtk::Menu *menu,synfig::Canvas::Handle canvas,const std::list<synfigapp::ValueDesc>& value_desc_list, const synfigapp::ValueDesc &value_desc)
{
	etl::handle<synfigapp::CanvasInterface> canvas_interface(find_canvas_interface(canvas));

	synfigapp::Action::ParamList param_list;
	param_list=canvas_interface->generate_param_list(value_desc_list);

	add_actions_to_menu(menu, param_list,synfigapp::Action::CATEGORY_VALUEDESC|synfigapp::Action::CATEGORY_VALUENODE);

	// Add the edit waypoints option if that might be useful
	if(canvas->rend_desc().get_time_end()-Time::epsilon()>canvas->rend_desc().get_time_start())
	{
		Gtk::MenuItem *item = Gtk::manage(new Gtk::MenuItem(_("Edit Waypoints")));
		item->signal_activate().connect(
			sigc::bind(
				sigc::bind(
					sigc::ptr_fun(&edit_several_waypoints),
					value_desc_list ),
				find_canvas_view(canvas) ));
		item->show();
		menu->append(*item);
	}
	// add here the rest of specific actions for multiple selected value_descs
	if (value_desc.is_valid())
		make_param_menu(menu,canvas,value_desc, 0.f, false);
}

void
Instance::gather_uri(std::set<synfig::String> &x, const synfig::ValueNode::Handle &value_node) const
{	//check for null value
	if (!value_node || !value_node->get_parent_canvas()) return;
	
	LinkableValueNode::Handle linkable_value_node = LinkableValueNode::Handle::cast_dynamic(value_node);
	if (!linkable_value_node) return;	//check that linkable_value_node is not empty

	FileSystem::Handle file_system = value_node->get_parent_canvas()->get_file_system();
	if (!file_system) return;

	Time t = value_node->get_parent_canvas()->get_time();

	const ParamVocab& vocab = linkable_value_node->get_children_vocab();
	for(ParamVocab::const_iterator i = vocab.begin(); i != vocab.end(); ++i)
	{
		ValueNode::Handle child_node = linkable_value_node->get_link(i->get_name());
		if (!child_node) continue;

		if (i->get_hint() == "filename")
		{
			ValueBase v = (*child_node)(t);
			if (v.can_get(String()))
			{
				String filename = CanvasFileNaming::make_full_filename(value_node->get_parent_canvas()->get_file_name(), v.get(String()));
				String uri = file_system->get_real_uri(filename);
				if (!uri.empty()) x.insert(uri);
			}
		}

		gather_uri(x, child_node);
	}
}

void
Instance::gather_uri(std::set<synfig::String> &x, const synfig::Layer::Handle &layer) const
{
	if (!layer || !layer->get_canvas()) return;
	
	FileSystem::Handle file_system = layer->get_canvas()->get_file_system();
	if (!file_system) return;

	ParamVocab vocab = layer->get_param_vocab();
	for(ParamVocab::const_iterator i = vocab.begin(); i != vocab.end(); ++i)
	{
		if (i->get_hint() == "filename")
		{
			ValueBase v = layer->get_param(i->get_name());
			if (v.can_get(String()))
			{
				String filename = v.get(String());
				if (!filename.empty() && filename[0] != '#')
					filename = filesystem::Path::absolute_path(layer->get_canvas()->get_file_path() + "/", filename);
				String uri = file_system->get_real_uri(filename);
				if (!uri.empty()) x.insert(uri);
			}
		}

		Layer::DynamicParamList::const_iterator j = layer->dynamic_param_list().find(i->get_name());
		if (j != layer->dynamic_param_list().end() && j->second)
			gather_uri(x, j->second);
	}
	
	// This required to allow "Edit Image/Open File" commands to appear when clicked on a switch group
	if (etl::handle<Layer_Switch> layer_switch = etl::handle<Layer_Switch>::cast_dynamic(layer))
		gather_uri(x, layer_inside_switch(layer_switch));

	//if (Layer_PasteCanvas::Handle paste = Layer_PasteCanvas::Handle::cast_dynamic(layer))
	//	gather_uri(x, paste->get_param("canvas").get(Canvas::Handle()));
}

void
Instance::gather_uri(std::set<synfig::String> &x, const synfig::Canvas::Handle &canvas) const
{
	if (!canvas) return;

	FileSystem::Handle file_system = canvas->get_file_system();
	if (!file_system) return;

	for(Canvas::const_iterator i = canvas->begin(); i != canvas->end(); ++i)
		gather_uri(x, *i);
}

// this is the second function for gather_uri - call it gather_uri(set, layerlist)
// iterate through all the layers and call gather_uri(set, layer) for each saperate layer 

void
Instance::gather_uri(std::set<synfig::String> &x, const synfigapp::SelectionManager::LayerList &layers) const
{
	for(synfigapp::SelectionManager::LayerList::const_iterator i = layers.begin(); i != layers.end(); ++i)
		gather_uri(x, *i);
}

// this is the entry point for gather_uri - call it gather_uri(map, layerlist)
void
Instance::gather_uri(std::map<synfig::String, synfig::String> &x, const synfigapp::SelectionManager::LayerList &layers) const
{
	std::set<String> uri_set;
	gather_uri(uri_set, layers);
	std::vector<String> uri_list;
	std::vector<filesystem::Path> filename_list;
	for(std::set<String>::const_iterator i = uri_set.begin(); i != uri_set.end(); ++i)
	{
		String filename = Glib::filename_from_uri(*i);
		if (!filename.empty())
		{
			uri_list.push_back(*i);
			filename_list.push_back(filename);
		}
	}

	std::vector<String> shortnames;
	MainWindow::make_short_filenames(filename_list, shortnames);
	for(int i = 0; i < (int)uri_list.size() && i < (int)shortnames.size(); ++i)
		x[shortnames[i]] = uri_list[i];
}

void
Instance::add_special_layer_actions_to_menu(Gtk::Menu *menu, const synfig::Layer::Handle &layer) const
{
	add_special_layer_actions_to_menu(menu, synfigapp::SelectionManager::LayerList(1, layer));
}

void
Instance::add_special_layer_actions_to_menu(Gtk::Menu *menu, const synfigapp::SelectionManager::LayerList &layers) const
{
	// Open files with external apps
	std::map<String, String> uris;
	gather_uri(uris, layers);
	for (auto i = uris.cbegin(); i != uris.cend(); ++i) {
		String label;
		sigc::slot<void> func;
		// check if layer is image
		if (is_img(i->second)) {
			label = _("Edit image in external tool...");
			func = sigc::bind(sigc::ptr_fun(&App::open_img_in_external), i->second);
		} else {
			label = strprintf(_("Open file '%s'"), i->first.c_str());
			func = sigc::bind(sigc::ptr_fun(&App::open_uri), i->second);
		}
		Gtk::MenuItem *item = manage(new Gtk::ImageMenuItem(Gtk::Stock::OPEN));
		item->set_label(label);
		item->signal_activate().connect(func);
		item->show();
		menu->append(*item);
	}

	// Vectorizer
	if (layers.size() == 1) {
		Layer_Bitmap::Handle layer_bitmap;

		if (auto reference_layer = etl::handle<Layer_Switch>::cast_dynamic(layers.front())) {
			//the layer selected is a switch group
			layer_bitmap = Layer_Bitmap::Handle::cast_dynamic(layer_inside_switch(reference_layer));
		} else {
			layer_bitmap = Layer_Bitmap::Handle::cast_dynamic(layers.front());
		}

		if (layer_bitmap) {
			Gtk::MenuItem *item2 = manage(new Gtk::ImageMenuItem(Gtk::Stock::CONVERT));
			item2->set_label( (String(_("Convert to Vector"))).c_str() );
			item2->signal_activate().connect(
				sigc::bind(sigc::ptr_fun(&App::open_vectorizerpopup), layer_bitmap, layers.front()) );
			item2->show();
			menu->append(*item2);
		}
	}
}

// called whenever we right click any layer under layers panel
// arguments - action_group: the current group of actions on the right click menu, layers: layerlist(because we can select multiple layer and then right click) 
void
Instance::add_special_layer_actions_to_group_and_menu(
		const Glib::RefPtr<Gio::SimpleActionGroup>& action_group,
		const std::string& action_group_name,
		const Glib::RefPtr<Gio::Menu>& menu,
		const synfigapp::SelectionManager::LayerList& layers) const
{
	const std::string symbolic_suffix = ""; // App::use-symbolic-icons ? "-symbolic" : "";
	// Open files with external apps
	std::map<String, String> uris;
	gather_uri(uris, layers);
	int index = 0;
	for (auto i = uris.cbegin(); i != uris.cend(); ++i, ++index) {
		String action_name = strprintf("special-action-open-file-%d", index);
		String local_name;
		sigc::slot<void> func;
		//if the import layer is type image
		if (is_img(i->second)) {
			local_name = _("Edit image in external tool...");
			func = sigc::bind(sigc::ptr_fun(&App::open_img_in_external), i->second);
		} else {
			local_name = strprintf(_("Open file '%s'"), i->first.c_str());
			func = sigc::bind(sigc::ptr_fun(&App::open_uri), i->second);
		}

		action_group->add_action(action_name, func);

		auto menu_item = Gio::MenuItem::create(local_name, action_group_name + "." + action_name);
		menu_item->set_icon(Gio::ThemedIcon::create("document-open" + symbolic_suffix));
		menu->append_item(menu_item);
	}

	// Vectorizer
	if (layers.size() == 1)	{
		String local_name = _("Convert to Vector");
		String action_name = strprintf("special-action-open-file-vectorizer-%d", index);
		Layer_Bitmap::Handle layer_bitmap;

		if (auto reference_layer = etl::handle<Layer_Switch>::cast_dynamic(layers.front())) {
			//the layer selected is a switch group
			layer_bitmap = Layer_Bitmap::Handle::cast_dynamic(layer_inside_switch(reference_layer));
		} else {
			layer_bitmap = Layer_Bitmap::Handle::cast_dynamic(layers.front());
		}

		if (layer_bitmap) {
			action_group->add_action(action_name, sigc::bind(sigc::ptr_fun(&App::open_vectorizerpopup), layer_bitmap, layers.front()));

			auto menu_item = Gio::MenuItem::create(local_name, action_group_name + "." + action_name);
			// FIXME Gtk::StockID STOCK_ID Gtk::Stock::CONVERT
			menu_item->set_icon(Gio::ThemedIcon::create("gtk-convert" + symbolic_suffix));
			menu->append_item(menu_item);
		}
	}
}
