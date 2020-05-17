/* === S Y N F I G ========================================================= */
/*!	\file state_brush.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
**	......... ... 2014 Jerome Blanchi
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

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/toggletoolbutton.h>
#include <glibmm/timeval.h>
#include <giomm.h>

#include <synfig/canvasfilenaming.h>
#include <synfig/layers/layer_switch.h>

#include "state_brush.h"
#include "state_normal.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"
#include <ETL/hermite>
#include <ETL/calculus>
#include <utility>
#include "event_mouse.h"
#include "event_layerclick.h"
#include "docks/dock_toolbox.h"

#include <synfigapp/blineconvert.h>
#include <synfigapp/wplistconverter.h>
#include <synfigapp/main.h>
#include <synfigapp/actions/layerpaint.h>

#include <ETL/gaussian>
#include "docks/dialog_tooloptions.h"

#include "ducktransform_matrix.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */
#ifndef BRUSH_ICON_SIZE
#	define BRUSH_ICON_SIZE 48
#endif

/* === G L O B A L S ======================================================= */

StateBrush studio::state_brush;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateBrush_Context : public sigc::trackable
{
public:
	class BrushConfig {
	public:
		struct MapEntry {
			float x, y;
			MapEntry(): x(0.f), y(0.f) { }
		};

		struct InputEntry {
			int input;
			std::vector<MapEntry> mapping;
			InputEntry(): input(0) { }
		};

		struct Entry {
			float base;
			std::vector<InputEntry> inputs;
			Entry(): base(0.f) { }
		};

		String filename;
		Entry settings[BRUSH_SETTINGS_COUNT];

		void clear();
		void load(const String &filename);
		void apply(brushlib::Brush &brush);

	private:
		static const char * setting_names[BRUSH_SETTINGS_COUNT];
		static const char * input_names[INPUT_COUNT];

		bool read_row(const char **pos);
		bool read_space(const char **pos);
		bool read_to_line_end(const char **pos);
		bool read_key(const char **pos, const char *key);
		bool read_word(const char **pos, String &out_value);
		bool read_float(const char **pos, float &out_value);
		bool read_input_entry(const char **pos, InputEntry &out_value);
		bool read_map_entry(const char **pos, MapEntry &out_value);
	};

private:
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;
	WorkArea::PushState push_state;

	Gtk::Menu menu;

	Glib::TimeVal time;
	etl::handle<synfigapp::Action::LayerPaint> action;
	TransformStack transform_stack;
	BrushConfig selected_brush_config;
	Gtk::ToggleToolButton *selected_brush_button;
	std::map<String, Gtk::ToggleToolButton*> brush_buttons;


	bool scan_directory(const String &path, int scan_sub_levels, std::set<String> &out_files);
	void select_brush(Gtk::ToggleToolButton *button, String filename);
	void refresh_ducks();

	synfigapp::Settings &settings;

	Gtk::CheckButton eraser_checkbox;

	void draw_to(Vector pos, Real pressure);
public:
	void load_settings();
	void save_settings();

	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_mouse_down_handler(const Smach::event& x);
	Smach::event_result event_mouse_up_handler(const Smach::event& x);
	Smach::event_result event_mouse_draw_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	static bool build_transform_stack(
		Canvas::Handle canvas,
		Layer::Handle layer,
		CanvasView::Handle canvas_view,
		TransformStack& transform_stack );

	void refresh_tool_options();

	StateBrush_Context(CanvasView* canvas_view);
	~StateBrush_Context();

	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Time get_time()const { return get_canvas_interface()->get_time(); }
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}
};	// END of class StateBrush_Context


/* === M E T H O D S ======================================================= */

const char * StateBrush_Context::BrushConfig::setting_names[] = {
		"opaque",                      // BRUSH_OPAQUE                       0
		"opaque_multiply",             // BRUSH_OPAQUE_MULTIPLY              1
		"opaque_linearize",            // BRUSH_OPAQUE_LINEARIZE             2
		"radius_logarithmic",          // BRUSH_RADIUS_LOGARITHMIC           3
		"hardness",                    // BRUSH_HARDNESS                     4
		"anti_aliasing",               // BRUSH_ANTI_ALIASING                5
		"dabs_per_basic_radius",       // BRUSH_DABS_PER_BASIC_RADIUS        6
		"dabs_per_actual_radius",      // BRUSH_DABS_PER_ACTUAL_RADIUS       7
		"dabs_per_second",             // BRUSH_DABS_PER_SECOND              8
		"radius_by_random",            // BRUSH_RADIUS_BY_RANDOM             9
		"speed1_slowness",             // BRUSH_SPEED1_SLOWNESS             10
		"speed2_slowness",             // BRUSH_SPEED2_SLOWNESS             11
		"speed1_gamma",                // BRUSH_SPEED1_GAMMA                12
		"speed2_gamma",                // BRUSH_SPEED2_GAMMA                13
		"offset_by_random",            // BRUSH_OFFSET_BY_RANDOM            14
		"offset_by_speed",             // BRUSH_OFFSET_BY_SPEED             15
		"offset_by_speed_slowness",    // BRUSH_OFFSET_BY_SPEED_SLOWNESS    16
		"slow_tracking",               // BRUSH_SLOW_TRACKING               17
		"slow_tracking_per_dab",       // BRUSH_SLOW_TRACKING_PER_DAB       18
		"tracking_noise",              // BRUSH_TRACKING_NOISE              19
		"color_h",                     // BRUSH_COLOR_H                     20
		"color_s",                     // BRUSH_COLOR_S                     21
		"color_v",                     // BRUSH_COLOR_V                     22
		"restore_color",               // BRUSH_RESTORE_COLOR               23
		"change_color_h",              // BRUSH_CHANGE_COLOR_H              24
		"change_color_l",              // BRUSH_CHANGE_COLOR_L              25
		"change_color_hsl_s",          // BRUSH_CHANGE_COLOR_HSL_S          26
		"change_color_v",              // BRUSH_CHANGE_COLOR_V              27
		"change_color_hsv_s",          // BRUSH_CHANGE_COLOR_HSV_S          28
		"smudge",                      // BRUSH_SMUDGE                      29
		"smudge_length",               // BRUSH_SMUDGE_LENGTH               30
		"smudge_radius_log",           // BRUSH_SMUDGE_RADIUS_LOG           31
		"eraser",                      // BRUSH_ERASER                      32
		"stroke_treshold",             // BRUSH_STROKE_THRESHOLD            33
		"stroke_duration_logarithmic", // BRUSH_STROKE_DURATION_LOGARITHMIC 34
		"stroke_holdtime",             // BRUSH_STROKE_HOLDTIME             35
		"custom_input",                // BRUSH_CUSTOM_INPUT                36
		"custom_input_slowness",       // BRUSH_CUSTOM_INPUT_SLOWNESS       37
		"elliptical_dab_ratio",        // BRUSH_ELLIPTICAL_DAB_RATIO        38
		"elliptical_dab_angle",        // BRUSH_ELLIPTICAL_DAB_ANGLE        39
		"direction_filter",            // BRUSH_DIRECTION_FILTER            40
		"lock_alpha"                   // BRUSH_LOCK_ALPHA                  41
};

const char * StateBrush_Context::BrushConfig::input_names[] = {
		"pressure",                    // INPUT_PRESSURE                     0
		"speed1",                      // INPUT_SPEED1                       1
		"speed2",                      // INPUT_SPEED2                       2
		"random",                      // INPUT_RANDOM                       3
		"stroke",                      // INPUT_STROKE                       4
		"direction",                   // INPUT_DIRECTION                    5
		"tilt_declination",            // INPUT_TILT_DECLINATION             6
		"tilt_ascension",              // INPUT_TILT_ASCENSION               7
		"custom",                      // INPUT_CUSTOM                       8
};



StateBrush::StateBrush():
	Smach::state<StateBrush_Context>("brush")
{
	insert(event_def(EVENT_STOP,&StateBrush_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateBrush_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateBrush_Context::event_mouse_down_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateBrush_Context::event_mouse_up_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateBrush_Context::event_mouse_draw_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateBrush_Context::event_refresh_tool_options));
}

StateBrush::~StateBrush()
{
}

void* StateBrush::enter_state(studio::CanvasView* machine_context) const
{
	return new StateBrush_Context(machine_context);
}

void
StateBrush_Context::BrushConfig::clear()
{
	filename.clear();
	for(int i = 0; i < BRUSH_SETTINGS_COUNT; ++i)
	{
		settings[i].base = 0;
		settings[i].inputs.clear();
	}
}

bool StateBrush_Context::BrushConfig::read_space(const char **pos)
{
	while (**pos > 0 && **pos <= ' ') ++(*pos);
	return true;
}

bool StateBrush_Context::BrushConfig::read_to_line_end(const char **pos)
{
	while (**pos != 0 && **pos != '\n' && **pos != '\r') ++(*pos);
	if (**pos == 0) return false;
	if (**pos == '\n' && *(++(*pos)) == '\r') ++(*pos);
	if (**pos == '\r' && *(++(*pos)) == '\n') ++(*pos);
	return true;
}

bool StateBrush_Context::BrushConfig::read_key(const char **pos, const char *key)
{
	size_t l = strlen(key);
	if (strncmp(*pos, key, l) == 0)
		{ *pos += l; return true; }
	return false;
}

bool StateBrush_Context::BrushConfig::read_word(const char **pos, String &out_value)
{
	out_value.clear();
	const char *p = *pos;
	while ((*p >= 'a' && *p <= 'z') || *p == '_') ++p;
	if (p > *pos) { out_value.assign(*pos, p); *pos = p; return true; }
	return false;
}

bool StateBrush_Context::BrushConfig::read_float(const char **pos, float &out_value)
{
	out_value = 0.f;

	const char *p = *pos;
	bool negative = *p == '-';
	if (negative) ++p;
	const char *num_start = p;
	while(*p >= '0' && *p <= '9')
		out_value = 10.f*out_value + (float)(*(p++) - '0');
	if (p <= num_start) return false;

	if (*p == '.')
	{
		++p;
		float amplifier = 1.f;
		while(*p >= '0' && *p <= '9')
			out_value += (amplifier *= 0.1f)*(float)(*(p++) - '0');
	}

	*pos = p;
	return true;
}

bool StateBrush_Context::BrushConfig::read_map_entry(const char **pos, MapEntry &out_value)
{
	out_value.x = 0.f;
	out_value.y = 0.f;
	const char *p = *pos;
	bool success = read_key(&p, "(")
				&& read_space(&p)
	            && read_float(&p, out_value.x)
				&& read_space(&p)
	            && read_float(&p, out_value.y)
				&& read_space(&p)
	            && read_key(&p, ")");
	if (success) { *pos = p; return true; }
	out_value.x = 0.f;
	out_value.y = 0.f;
	return false;
}

bool StateBrush_Context::BrushConfig::read_input_entry(const char **pos, InputEntry &out_value)
{
	out_value.input = 0;
	out_value.mapping.clear();

	const char *p = *pos;
	String word;
	if (read_space(&p) && read_word(&p, word))
	{
		for(int i = 0; i < INPUT_COUNT; ++i)
		{
			if (word == input_names[i])
			{
				MapEntry entry;
				const char *pp = p;
				while(read_space(&pp) && (out_value.mapping.empty() || (read_key(&pp, ",") && read_space(&pp))) && read_map_entry(&pp, entry))
					{ out_value.mapping.push_back(entry); p = pp; }
				if (out_value.mapping.size() > 1)
				{
					out_value.input = i;
					*pos = p;
					return true;
				}
				out_value.mapping.clear();
				break;
			}
		}
	}
	return false;
}

bool
StateBrush_Context::BrushConfig::read_row(const char **pos)
{
	const char *p = *pos;
	String word;
	if (read_space(&p) && read_word(&p, word))
	{
		for(int i = 0; i < BRUSH_SETTINGS_COUNT; ++i)
		{
			if (word == setting_names[i])
			{
				if (read_space(&p) && read_float(&p, settings[i].base))
				{
					InputEntry entry;
					const char *pp = p;
					while(read_space(&pp) && read_key(&pp, "|") && read_space(&pp) && read_input_entry(&pp, entry))
						{ settings[i].inputs.push_back(entry); p = pp; }
					*pos = p;
				}
				break;
			}
		}
	}
	return read_to_line_end(pos);
}

void
StateBrush_Context::BrushConfig::load(const String &filename)
{
	clear();

	char *buffer = NULL;
	{
		Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(filename);
		goffset s = file->query_info()->get_size();
		if (s < 0) return;
		size_t size = s > INT_MAX-1 ? INT_MAX-1 : (size_t)s;
		buffer = new char[size+1];
		memset(buffer, 0, size+1);

		Glib::RefPtr<Gio::FileInputStream> stream = file->read();
		stream->read(buffer, size);
		stream->close();
	}

	const char *pos = buffer;
	if (pos != NULL) while(read_row(&pos)) { }
	if (buffer) delete[] buffer;
	this->filename = filename;
}

void
StateBrush_Context::BrushConfig::apply(brushlib::Brush &brush)
{
	for(int i = 0; i < BRUSH_SETTINGS_COUNT; ++i)
	{
		brush.set_base_value(i, settings[i].base);
		for(int j = 0; j < INPUT_COUNT; ++j)
			brush.set_mapping_n(i, j, 0);
		for(std::vector<InputEntry>::const_iterator j = settings[i].inputs.begin(); j != settings[i].inputs.end(); ++j)
		{
			brush.set_mapping_n(i, j->input, (int)j->mapping.size());
			for(std::vector<MapEntry>::const_iterator k = j->mapping.begin(); k != j->mapping.end(); ++k)
				brush.set_mapping_point(i, j->input, (int)(k - j->mapping.begin()), k->x, k->y);
		}
	}
}

void
StateBrush_Context::load_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");

		String value;
		bool bvalue(settings.get_value("brush.path_count",value));
		int count = atoi(value.c_str());
		if(bvalue && count>0)
		{
			App::brushes_path.clear();
			int count = atoi(value.c_str());
			for(int i = 0; i < count; ++i)
				if(settings.get_value(strprintf("brush.path_%d", i),value))
					App::brushes_path.insert(value);
		}
		else
		{
			if (App::brushes_path.empty())
				App::brushes_path.insert(App::get_base_path()+ETL_DIRECTORY_SEPARATOR+"share"+ETL_DIRECTORY_SEPARATOR+"synfig"+ETL_DIRECTORY_SEPARATOR+"brushes");
			else
				App::brushes_path.insert(*(App::brushes_path.begin()));
		}
		refresh_tool_options();

		if (settings.get_value("brush.selected_brush_filename",value))
			if (brush_buttons.count(value))
				brush_buttons[value]->set_active(true);

		if (settings.get_value("brush.eraser",value))
			eraser_checkbox.set_active(value == "true");
	}
	catch(...)
	{
		synfig::warning("State Brush: Caught exception when attempting to load settings.");
	}
}

void
StateBrush_Context::save_settings()
{
	try
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");

		settings.set_value("brush.path_count", strprintf("%d", (int)App::brushes_path.size()));
		int j = 0;
		for(std::set<String>::const_iterator i = App::brushes_path.begin(); i != App::brushes_path.end(); ++i)
			settings.set_value(strprintf("brush.path_%d", j++), *i);

		settings.set_value("brush.selected_brush_filename", selected_brush_config.filename);
		settings.set_value("brush.eraser", eraser_checkbox.get_active() ? "true" : "false");
	}
	catch(...)
	{
		synfig::warning("State Brush: Caught exception when attempting to save settings.");
	}
}

StateBrush_Context::StateBrush_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	push_state(*get_work_area()),
	selected_brush_button(NULL),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	eraser_checkbox(_("Eraser"))
{
	load_settings();

	//refresh_tool_options();
	App::dialog_tool_options->present();

	// Hide all tangent and width ducks
	get_work_area()->set_type_mask(get_work_area()->get_type_mask()-Duck::TYPE_TANGENT-Duck::TYPE_WIDTH);
	get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);

	// Turn off layer clicking
	get_work_area()->set_allow_layer_clicks(false);

	// Turn off duck clicking
	get_work_area()->set_allow_duck_clicks(false);

	get_work_area()->set_cursor(Gdk::PENCIL);

	App::dock_toolbox->refresh();
	refresh_ducks();
}

StateBrush_Context::~StateBrush_Context()
{
	if (action)
	{
		get_canvas_interface()->get_instance()->perform_action(action);
		action = NULL;
		transform_stack.clear();
	}

	save_settings();

	brush_buttons.clear();
	selected_brush_button = NULL;
	App::dialog_tool_options->clear();

	get_work_area()->reset_cursor();

	// Refresh the work area
	get_work_area()->queue_draw();

	App::dock_toolbox->refresh();
}

bool
StateBrush_Context::scan_directory(const String &path, int scan_sub_levels, std::set<String> &out_files)
{
	if (scan_sub_levels < 0) return false;
	Glib::RefPtr<Gio::File> directory = Gio::File::create_for_path(path);
	Glib::RefPtr<Gio::FileEnumerator> e;

	try
	{
		e = directory->enumerate_children();
	}
	catch(Gio::Error&) { return false; }
	catch(Glib::FileError&) { return false; }

	Glib::RefPtr<Gio::FileInfo> info;
	while((bool)(info = e->next_file()))
	{
		String filepath = FileSystem::fix_slashes(directory->get_child(info->get_name())->get_path());
		if (!scan_directory(filepath, scan_sub_levels-1, out_files))
			out_files.insert(filepath);
	}

	return true;
}

void
StateBrush_Context::refresh_tool_options()
{
	brush_buttons.clear();
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_local_name(_("Brush Tool"));
	App::dialog_tool_options->set_name("brush");

	// create the brush options container
	Gtk::Grid *brush_option_grid= Gtk::manage(new Gtk::Grid());

	brush_option_grid->set_orientation(Gtk::ORIENTATION_VERTICAL);

	// add options
	brush_option_grid->add(eraser_checkbox);

	// create brushes scrollable palette
	Gtk::ToolItemGroup *tool_item_group = manage(new class Gtk::ToolItemGroup());
	gtk_tool_item_group_set_label(tool_item_group->gobj(), NULL);

	Gtk::ToolPalette *palette = manage(new Gtk::ToolPalette());
	palette->add(*tool_item_group);
	palette->set_expand(*tool_item_group);
	palette->set_exclusive(*tool_item_group, true);
	palette->set_icon_size(Gtk::IconSize(BRUSH_ICON_SIZE));
	// let the palette propagate the scroll events
	palette->add_events(Gdk::SCROLL_MASK);

	Gtk::ScrolledWindow *brushes_scroll = manage(new Gtk::ScrolledWindow());
	brushes_scroll->set_hexpand(true);
	brushes_scroll->set_vexpand(true);
	brushes_scroll->add(*palette);

	// load brushes files definition
	// scan directories
	std::set<String> files;
	for(std::set<String>::const_iterator i = App::brushes_path.begin(); i != App::brushes_path.end(); ++i)
		scan_directory(*i, 1, files);

	// run through brush definition and assign a button
	Gtk::ToggleToolButton *first_button = NULL;
	for(std::set<String>::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		if (!brush_buttons.count(*i) && filename_extension(*i) == ".myb")
		{
			const String &brush_file = *i;
			const String icon_file = filename_sans_extension(brush_file) + "_prev.png";
			if (files.count(icon_file))
			{
				// create a single brush button
				Gtk::ToggleToolButton *brush_button = brush_buttons[*i] = (new class Gtk::ToggleToolButton());

				Glib::RefPtr<Gdk::Pixbuf> pixbuf, pixbuf_scaled;
				pixbuf = Gdk::Pixbuf::create_from_file(icon_file);
				pixbuf_scaled = pixbuf->scale_simple(BRUSH_ICON_SIZE, BRUSH_ICON_SIZE, Gdk::INTERP_BILINEAR);

				brush_button->set_icon_widget(*Gtk::manage(new Gtk::Image(pixbuf_scaled)));
				brush_button->set_halign(Gtk::ALIGN_CENTER);

				// connect the button click event and brush file definition
				brush_button->signal_clicked().connect(
					sigc::bind(sigc::mem_fun(*this, &StateBrush_Context::select_brush), brush_button, brush_file) );

				// add the button to the palette
				tool_item_group->insert(*brush_button);

				// keep the first brush
				if (first_button == NULL) first_button = brush_button;
			}
		}
	}

	brush_option_grid->add(*brushes_scroll);
	brush_option_grid->show_all();

	App::dialog_tool_options->add(*brush_option_grid);

	// select first brush
	if (first_button != NULL)
		{
		first_button->set_active(true);
		selected_brush_button = first_button;
		}
}

void
StateBrush_Context::select_brush(Gtk::ToggleToolButton *button, String filename)
{
	if (button != NULL && button->get_active())
	{
		if (selected_brush_button != NULL) selected_brush_button->set_active(false);
		selected_brush_config.load(filename);
		eraser_checkbox.set_active(selected_brush_config.settings[BRUSH_ERASER].base > 0.0);
		selected_brush_button = button;
	}
}

Smach::event_result
StateBrush_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateBrush_Context::event_stop_handler(const Smach::event& /*x*/)
{
	if (action)
	{
		get_canvas_interface()->get_instance()->perform_action(action);
		action = NULL;
	}

	throw &state_normal;
	return Smach::RESULT_OK;
}

Smach::event_result
StateBrush_Context::event_refresh_handler(const Smach::event& /*x*/)
{
	refresh_ducks();
	return Smach::RESULT_ACCEPT;
}

bool
StateBrush_Context::build_transform_stack(
	Canvas::Handle canvas,
	Layer::Handle layer,
	CanvasView::Handle canvas_view,
	TransformStack& transform_stack )
{
	int count = 0;
	for(Canvas::iterator i = canvas->begin(); i != canvas->end() ;++i)
	{
		if(*i == layer) return true;

		if((*i)->active())
		{
			Transform::Handle trans((*i)->get_transform());
			if(trans) { transform_stack.push(trans); count++; }
		}

		// If this is a paste canvas layer, then we need to
		// descend into it
		if(etl::handle<Layer_PasteCanvas> layer_pastecanvas = etl::handle<Layer_PasteCanvas>::cast_dynamic(*i))
		{
			transform_stack.push_back(
				new Transform_Matrix(
						layer_pastecanvas->get_guid(),
					layer_pastecanvas->get_summary_transformation().get_matrix()
				)
			);
			if (build_transform_stack(layer_pastecanvas->get_sub_canvas(), layer, canvas_view, transform_stack))
				return true;
			transform_stack.pop();
		}
	}
	while(count-- > 0) transform_stack.pop();
	return false;
}


Smach::event_result
StateBrush_Context::event_mouse_down_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		{
			// Enter the stroke state to get the stroke
			Layer::Handle selected_layer = canvas_view_->get_selection_manager()->get_selected_layer();
			etl::handle<Layer_Bitmap> layer = etl::handle<Layer_Bitmap>::cast_dynamic(selected_layer);
			if (!layer)
			{
				etl::handle<Layer_Switch> layer_switch = etl::handle<Layer_Switch>::cast_dynamic(selected_layer);
				if (layer_switch) layer = etl::handle<Layer_Bitmap>::cast_dynamic(layer_switch->get_current_layer());
			}

			// No image found to draw in, add it.
			if(!layer)
			{
				canvas_view_->add_layer("import");
				selected_layer = canvas_view_->get_selection_manager()->get_selected_layer();
				layer = etl::handle<Layer_Bitmap>::cast_dynamic(selected_layer);

				// Set temporary description to generate the name
				String temp_description(_("brush image"));
				layer->set_description(temp_description);

				if (selected_layer->get_param_list().count("filename") != 0)
				{
					// generate name based on description
					String description, filename, filename_param;
					get_canvas_interface()
						->get_instance()
						->generate_new_name(
							layer,
							description,
							filename,
							filename_param );

					// create and save surface
					get_canvas_interface()
						->get_instance()
						->save_surface(layer->rendering_surface, filename);

					selected_layer->set_param("filename", filename_param);
					selected_layer->set_description(description);
				}
			}

			if (layer)
			{
				transform_stack.clear();
				if (build_transform_stack(get_canvas(), layer, get_canvas_view(), transform_stack))
				{
					etl::handle<synfigapp::Action::LayerPaint> action = new synfigapp::Action::LayerPaint();
					action->set_param("canvas",get_canvas());
					action->set_param("canvas_interface",get_canvas_interface());
					action->stroke.set_layer(layer);
					selected_brush_config.apply( action->stroke.brush() );

					Color color = synfigapp::Main::get_outline_color();

					Real epsilon = 0.00000001;
					Real r(color.get_r()), g(color.get_g()), b(color.get_b());
					Real max_rgb = max(r, max(g, b));
					Real min_rgb = min(r, min(g, b));
					Real diff = max_rgb-min_rgb;

					Real val = max_rgb;
					Real sat = fabs(max_rgb) > epsilon ? 1.0 - (min_rgb / max_rgb) : 0;
					Real hue = fabs(diff) <= epsilon ?
							0 : max_rgb == r ?
								60.0 * fmod ((g - b)/(diff), 6.0) : max_rgb == g ?
									60.0 * (((b - r)/(diff))+2.0) : 60.0 * (((r - g)/(diff))+4.0);

					Real opaque = color.get_a();
					Real radius = synfigapp::Main::get_bline_width();

					Real eraser = eraser_checkbox.get_active() ? 1.0 : 0.0;

					action->stroke.brush().set_base_value(BRUSH_COLOR_H, hue/360.0);
					action->stroke.brush().set_base_value(BRUSH_COLOR_S, sat);
					action->stroke.brush().set_base_value(BRUSH_COLOR_V, val);
					action->stroke.brush().set_base_value(BRUSH_OPAQUE, opaque);
					action->stroke.brush().set_base_value(BRUSH_RADIUS_LOGARITHMIC, log(radius));
					action->stroke.brush().set_base_value(BRUSH_ERASER, eraser);
					action->stroke.prepare();

					time.assign_current_time();
					this->action = action;
					draw_to(event.pos, 0);

					return Smach::RESULT_ACCEPT;
				}
			}
			break;
		}

	default:
		break;
	}
	return Smach::RESULT_OK;
}

Smach::event_result
StateBrush_Context::event_mouse_up_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		{
			if (action)
			{
				get_canvas_interface()->get_instance()->perform_action(action);
				action = NULL;
				transform_stack.clear();
				return Smach::RESULT_ACCEPT;
			}
			break;
		}

	default:
		break;
	}

	return Smach::RESULT_OK;
}

void
StateBrush_Context::draw_to(Vector pos, Real pressure)
{
	Glib::TimeVal prev_time = time;
	time.assign_current_time();
	double delta_time = (time - prev_time).as_double();
	if (delta_time < 0.00001) delta_time = 0.00001;

	Point p = transform_stack.unperform( pos );
	Point tl = action->stroke.get_layer()->get_param("tl").get(Point());
	Point br = action->stroke.get_layer()->get_param("br").get(Point());
	int w = action->stroke.get_layer()->rendering_surface ? action->stroke.get_layer()->rendering_surface->get_width() : 0;
	int h = action->stroke.get_layer()->rendering_surface ? action->stroke.get_layer()->rendering_surface->get_height() : 0;

	action->stroke.add_point_and_apply(
		synfigapp::Action::LayerPaint::PaintPoint(
			(float)((p[0] - tl[0])/(br[0] - tl[0])*w),
			(float)((p[1] - tl[1])/(br[1] - tl[1])*h),
			(float)pressure,
			delta_time ));
}

Smach::event_result
StateBrush_Context::event_mouse_draw_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	switch(event.button)
	{
	case BUTTON_LEFT:
		{
			if (action)
			{
				draw_to(event.pos, event.pressure);
				return Smach::RESULT_ACCEPT;
			}
			break;
		}

	default:
		break;
	}

	return Smach::RESULT_OK;
}

void
StateBrush_Context::refresh_ducks()
{
	get_canvas_view()->queue_rebuild_ducks();
}
