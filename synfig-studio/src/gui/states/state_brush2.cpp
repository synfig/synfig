/* === S Y N F I G ========================================================= */
/*!	\file state_brush2.cpp
**	\brief Template File
**
**	\legal
**	......... ... 2014 Ivan Mahonin
**	......... ... 2014 Jerome Blanchi
**	......... ... 2025 Abdelhadi Wael
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

#include "state_brush2.h"

#include "state_normal.h"
#include <gui/canvasview.h>
#include <gui/event_mouse.h>
#include <gui/workarea.h>
#include <synfigapp/main.h>
#include <gui/workarearenderer/renderer_brush_overlay.h>
#include <synfig/rendering/software/surfacesw.h>
#include <brushlib.h>
#include <gui/app.h>
#include <gui/docks/dialog_tooloptions.h>
#include <gui/docks/dock_toolbox.h>
#include <gui/ducktransform_matrix.h>
#include <gui/resourcehelper.h>
#include <gui/widgets/widget_link.h>
#include <gtkmm/toolpalette.h>
#include <synfig/general.h>
#include <glibmm/fileutils.h>
#include <synfigapp/actions/layerbrush.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;
using namespace synfig;

/* === M A C R O S ========================================================= */

#ifndef BRUSH_ICON_SIZE
#	define BRUSH_ICON_SIZE 48
#endif

/* === G L O B A L S ======================================================= */

StateBrush2 studio::state_brush2;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateBrush2_Context : public sigc::trackable
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
	CanvasView::Handle canvas_view_;
	WorkArea::PushState push_state;

	synfig::Surface overlay_surface_;
	synfig::Rect overlay_rect_;
	sigc::connection clear_overlay_timer_;

	Glib::TimeVal time_;
	etl::handle<synfigapp::Action::LayerBrush> action_;

	TransformStack transform_stack_;
	static bool build_transform_stack(
	Canvas::Handle canvas,
	Layer::Handle layer,
	CanvasView::Handle canvas_view,
	TransformStack& transform_stack);

	BrushConfig selected_brush_config;
	BrushConfig original_brush_config;

	std::map<int, Gtk::Scale*> setting_controls;
	std::map<int, Gtk::Button*> reset_buttons;
	Gtk::ToggleToolButton *selected_brush_button;
	std::map<String, Gtk::ToggleToolButton*> brush_buttons;
	synfigapp::Settings &settings;
	Gtk::CheckButton eraser_checkbox;

	Gtk::Grid options_grid;
	Gtk::Label title_label;

	void update_cursor();
	void create_radius_cursor(float radius);

	void load_settings();
	void save_settings();
	bool scan_directory(const String &path, int scan_sub_levels, std::set<String> &out_files);
	void select_brush(Gtk::ToggleToolButton *button, String filename);

public:
	StateBrush2_Context(CanvasView* canvas_view);
	~StateBrush2_Context();

	void refresh_tool_options();
	Smach::event_result event_refresh_tool_options(const Smach::event& x);
	void create_image_layer_dialog();

	void update_overlay_preview(const synfig::Surface& surface, const synfig::Rect& rect);
	void clear_overlay_preview();

	void create_brushes_tab(Gtk::Notebook *notebook);
	void create_settings_tab(Gtk::Notebook *notebook);

	void draw_to(Vector pos, Real pressure);
	Layer_Bitmap::Handle find_or_create_layer();

	const CanvasView::Handle& get_canvas_view() const { return canvas_view_; }
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface() const { return canvas_view_->canvas_interface(); }
	synfig::Canvas::Handle get_canvas() const { return canvas_view_->get_canvas(); }
	WorkArea* get_work_area() const { return canvas_view_->get_work_area();}

	Smach::event_result event_mouse_down_handler(const Smach::event& x);
	Smach::event_result event_mouse_up_handler(const Smach::event& x);
	Smach::event_result event_mouse_draw_handler(const Smach::event& x);
	Smach::event_result event_stop_handler(const Smach::event& x);
};	// END of class StateBrush2_Context

/* === M E T H O D S ======================================================= */

const char * StateBrush2_Context::BrushConfig::setting_names[] = {
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

const char * StateBrush2_Context::BrushConfig::input_names[] = {
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

StateBrush2::StateBrush2() :
	Smach::state<StateBrush2_Context>("brush2", N_("Brush Tool 2"))
{
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN, &StateBrush2_Context::event_mouse_down_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,   &StateBrush2_Context::event_mouse_up_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG, &StateBrush2_Context::event_mouse_draw_handler));
	insert(event_def(EVENT_STOP,                       &StateBrush2_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,       &StateBrush2_Context::event_refresh_tool_options));
}

StateBrush2::~StateBrush2()
{
}

void* StateBrush2::enter_state(studio::CanvasView* machine_context) const
{
	return new StateBrush2_Context(machine_context);
}

void
StateBrush2_Context::BrushConfig::clear()
{
	filename.clear();
	for(int i = 0; i < BRUSH_SETTINGS_COUNT; ++i)
	{
		settings[i].base = 0;
		settings[i].inputs.clear();
	}
}

bool StateBrush2_Context::BrushConfig::read_space(const char **pos)
{
	// Skip whitespace characters
	while (**pos > 0 && **pos <= ' ') ++(*pos);
	return true;
}

bool StateBrush2_Context::BrushConfig::read_to_line_end(const char **pos)
{
	// Advance pointer to the end of the line
	while (**pos != 0 && **pos != '\n' && **pos != '\r') ++(*pos);
	if (**pos == 0) return false;
	if (**pos == '\n' && *(++(*pos)) == '\r') ++(*pos);
	if (**pos == '\r' && *(++(*pos)) == '\n') ++(*pos);
	return true;
}

bool StateBrush2_Context::BrushConfig::read_key(const char **pos, const char *key)
{
	// Check if the current position matches the given key string
	size_t l = strlen(key);
	if (strncmp(*pos, key, l) == 0)
		{ *pos += l; return true; }
	return false;
}

bool StateBrush2_Context::BrushConfig::read_word(const char **pos, String &out_value)
{
	// Read a word (a-z or '_') from the current position
	out_value.clear();
	const char *p = *pos;
	while ((*p >= 'a' && *p <= 'z') || *p == '_') ++p;
	if (p > *pos) { out_value.assign(*pos, p); *pos = p; return true; }
	return false;
}

bool StateBrush2_Context::BrushConfig::read_float(const char **pos, float &out_value)
{
	// Parse a floating point number from the current position
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

	if (negative) out_value = -out_value;
	*pos = p;
	return true;
}

bool StateBrush2_Context::BrushConfig::read_map_entry(const char **pos, MapEntry &out_value)
{
	// Parse a mapping entry in the form (x y)
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

bool StateBrush2_Context::BrushConfig::read_input_entry(const char **pos, InputEntry &out_value)
{
	// Parse a row in the brush config file, mapping settings and input entries
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
StateBrush2_Context::BrushConfig::read_row(const char **pos)
{
	// Parse a row in the brush config file, mapping settings and input entries
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
StateBrush2_Context::BrushConfig::load(const String &filename)
{
	// Load brush configuration from file and parse settings
	clear();

	char* buffer = nullptr;
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
	if (pos) while(read_row(&pos)) { }
	if (buffer) delete[] buffer;
	this->filename = filename;
}

void
StateBrush2_Context::BrushConfig::apply(brushlib::Brush &brush)
{
	// Apply all loaded settings and input mappings to the brush instance
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
StateBrush2_Context::load_settings()
{
	try
	{
		int brush_path_count = settings.get_value("brush.path_count", 0);
		if (brush_path_count > 0)
		{
			String value;
			App::brushes_path.clear();
			//int count = atoi(value.c_str());
			for(int i = 0; i < brush_path_count; ++i)
				if(settings.get_raw_value(strprintf("brush.path_%d", i),value))
					App::brushes_path.insert(value);
		}
		else
		{
			if (App::brushes_path.empty())
				App::brushes_path.insert(ResourceHelper::get_brush_path());
		}
		refresh_tool_options();

		std::string value;
		if (settings.get_raw_value("brush.selected_brush_filename", value))
			if (brush_buttons.count(value))
				brush_buttons[value]->set_active(true);

		eraser_checkbox.set_active(settings.get_value("brush.eraser", false));
	}
	catch(...)
	{
		synfig::warning("State Brush: Caught exception when attempting to load settings.");
	}
}

void
StateBrush2_Context::save_settings()
{
	try
	{
		settings.set_value("brush.path_count", (int)App::brushes_path.size());
		int j = 0;
		for (const auto& path : App::brushes_path)
			settings.set_value(strprintf("brush.path_%d", j++), path);

		settings.set_value("brush.selected_brush_filename", selected_brush_config.filename);
		settings.set_value("brush.eraser", eraser_checkbox.get_active());
	}
	catch(...)
	{
		synfig::warning("State Brush: Caught exception when attempting to save settings.");
	}
}

StateBrush2_Context::StateBrush2_Context(CanvasView* canvas_view) :
	canvas_view_(canvas_view),
	push_state(*get_work_area()),
	action_(nullptr),
	selected_brush_button(nullptr),
	settings(synfigapp::Main::get_selected_input_device()->settings()),
	eraser_checkbox(_("Eraser"))
{
	load_settings();
	App::dialog_tool_options->present();
	update_cursor();
	refresh_tool_options();
	App::dock_toolbox->refresh();
}

StateBrush2_Context::~StateBrush2_Context()
{
	if (action_)
	  action_ = nullptr;

	save_settings();
	brush_buttons.clear();
	setting_controls.clear();
	reset_buttons.clear();
	original_brush_config.clear();
	selected_brush_button = nullptr;
	clear_overlay_preview();
	get_work_area()->reset_cursor();
	App::dialog_tool_options->clear();
	App::dock_toolbox->refresh();
}

bool
StateBrush2_Context::scan_directory(const String &path, int scan_sub_levels, std::set<String> &out_files)
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
StateBrush2_Context::refresh_tool_options()
{
	brush_buttons.clear();
	setting_controls.clear();
	reset_buttons.clear();
	original_brush_config.clear();
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_local_name(_("Brush Tool"));
	App::dialog_tool_options->set_name("brush");

	Gtk::Notebook *notebook = Gtk::manage(new Gtk::Notebook());
	notebook->set_hexpand(true);
	notebook->set_vexpand(true);

	create_brushes_tab(notebook);
	create_settings_tab(notebook);
	notebook->show_all();
	App::dialog_tool_options->add(*notebook);
}

void
StateBrush2_Context::create_brushes_tab(Gtk::Notebook *notebook)
{
	// create the brush options container
	Gtk::Grid *brush_option_grid = Gtk::manage(new Gtk::Grid());
	brush_option_grid->set_orientation(Gtk::ORIENTATION_VERTICAL);

	// add options
	brush_option_grid->add(eraser_checkbox);

	// connect eraser checkbox signal to update brush
	eraser_checkbox.signal_toggled().connect([this]() {
		if (selected_brush_button && selected_brush_config.filename.length() > 0) {
			selected_brush_config.settings[BRUSH_ERASER].base = eraser_checkbox.get_active() ? 1.0 : 0.0;
			if (action_) {
				selected_brush_config.apply(action_->stroke.brush());
			}
		}
	});

	// create brushes scrollable palette
	Gtk::ToolItemGroup *tool_item_group = manage(new class Gtk::ToolItemGroup());
	gtk_tool_item_group_set_label(tool_item_group->gobj(), nullptr);

	Gtk::ToolPalette *palette = manage(new Gtk::ToolPalette());
	palette->add(*tool_item_group);
	palette->set_expand(*tool_item_group);
	palette->set_exclusive(*tool_item_group, true);
	palette->set_icon_size(Gtk::IconSize(BRUSH_ICON_SIZE));
	palette->add_events(Gdk::SCROLL_MASK);

	Gtk::ScrolledWindow *brushes_scroll = manage(new Gtk::ScrolledWindow());
	brushes_scroll->set_hexpand(true);
	brushes_scroll->set_vexpand(true);
	brushes_scroll->add(*palette);

	// load brushes files definition
	std::set<String> files;
	for (const auto& path : App::brushes_path)
		scan_directory(path.u8string(), 1, files);

	// run through brush definition and assign a button
	Gtk::ToggleToolButton* first_button = nullptr;
	String first_button_filename;
	for(std::set<String>::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		if (!brush_buttons.count(*i) && filesystem::Path::filename_extension(*i) == ".myb")
		{
			const String &brush_file = *i;
			const String icon_file = filesystem::Path::filename_sans_extension(brush_file) + "_prev.png";
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
					sigc::bind(sigc::mem_fun(*this, &StateBrush2_Context::select_brush), brush_button, brush_file) );

				// add the button to the palette
				tool_item_group->insert(*brush_button);

				// keep the first brush
				if (!first_button)
					first_button = brush_button;
					first_button_filename = brush_file;
			}
		}
	}
	brush_option_grid->add(*brushes_scroll);

	// add create image layer button
	Gtk::Button* create_image_layer_btn = Gtk::manage(new Gtk::Button(_("Create image layer")));
	brush_option_grid->add(*create_image_layer_btn);
	create_image_layer_btn->signal_clicked().connect(sigc::mem_fun(*this, &StateBrush2_Context::create_image_layer_dialog));
	brush_option_grid->show_all();

	Gtk::Label *tab_label = Gtk::manage(new Gtk::Label(_("Brushes")));
	notebook->append_page(*brush_option_grid, *tab_label);

	// select first brush
	if (first_button) {
		select_brush(first_button, first_button_filename);
		first_button->set_active(true);
	}
}

void
StateBrush2_Context::create_settings_tab(Gtk::Notebook *notebook)
{
	// create settings scrollable container
	Gtk::ScrolledWindow *settings_scroll = Gtk::manage(new Gtk::ScrolledWindow());
	settings_scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	settings_scroll->set_hexpand(true);
	settings_scroll->set_vexpand(true);

	// create settings grid
	Gtk::Grid *settings_grid = Gtk::manage(new Gtk::Grid());
	settings_grid->set_row_spacing(4);
	settings_grid->set_column_spacing(8);
	settings_grid->set_border_width(12);
	settings_grid->set_row_homogeneous(true);

	struct SettingInfo {
		const char* display_name;
		float min_value;
		float max_value;
		const char* tooltip;
	};
	const SettingInfo setting_info[BRUSH_SETTINGS_COUNT] = {
		{"Opacity", 0.0f, 2.0f, "0 means brush is transparent, 1 fully visible\n(also known as alpha or opacity)"},
		{"Opacity multiply", 0.0f, 2.0f, "This gets multiplied with opaque. You should only change the pressure input of this setting."},
		{"Opacity linearize", 0.0f, 2.0f, "Correct the nonlinearity introduced by blending multiple dabs on top of each other."},
		{"Radius", -2.0f, 5.0f, "Basic brush radius (logarithmic)\n 0.7 means 2 pixels\n 3.0 means 20 pixels"},
		{"Hardness", 0.0f, 1.0f, "Hard brush-circle borders (setting to zero will draw nothing)."},
		{"Anti-aliasing", 0.0f, 5.0f, "This setting decreases the hardness when necessary to prevent a pixel staircase effect."},
		{"Dabs per basic radius", 0.0f, 6.0f, "How many dabs to draw while the pointer moves a distance of one brush radius"},
		{"Dabs per actual radius", 0.0f, 6.0f, "Same as above, but the radius actually drawn is used, which can change dynamically"},
		{"Dabs per second", 0.0f, 80.0f, "Dabs to draw each second, no matter how far the pointer moves"},
		{"Radius by random", 0.0f, 1.5f, "Alter the radius randomly each dab."},
		{"Fine speed filter", 0.0f, 0.2f, "How slow the input fine speed is following the real speed"},
		{"Gross speed filter", 0.0f, 3.0f, "Same as 'fine speed filter', but note that the range is different"},
		{"Fine speed gamma", -8.0f, 8.0f, "This changes the reaction of the 'fine speed' input to extreme physical speed."},
		{"Gross speed gamma", -8.0f, 8.0f, "Same as 'fine speed gamma' for gross speed"},
		{"Jitter", 0.0f, 25.0f, "Add a random offset to the position where each dab is drawn"},
		{"Offset by speed", -3.0f, 3.0f, "Change position depending on pointer speed"},
		{"Offset by speed filter", 0.0f, 15.0f, "How slow the offset goes back to zero when the cursor stops moving"},
		{"Slow position tracking", 0.0f, 10.0f, "Slowdown pointer tracking speed. 0 disables it, higher values remove more jitter"},
		{"Slow tracking per dab", 0.0f, 10.0f, "Similar as above but at brushdab level"},
		{"Tracking noise", 0.0f, 12.0f, "Add randomness to the mouse pointer"},
		{"Color hue", 0.0f, 1.0f, "Color hue"},
		{"Color saturation", -0.5f, 1.5f, "Color saturation"},
		{"Color value", -0.5f, 1.5f, "Color value (brightness, intensity)"},
		{"Save color", 0.0f, 1.0f, "When selecting a brush, the color can be restored to the color that the brush was saved with."},
		{"Change color hue", -2.0f, 2.0f, "Change color hue."},
		{"Change color lightness (HSL)", -2.0f, 2.0f, "Change the color lightness (luminance) using the HSL color model."},
		{"Change color satur. (HSL)", -2.0f, 2.0f, "Change the color saturation using the HSL color model."},
		{"Change color value (HSV)", -2.0f, 2.0f, "Change the color value (brightness, intensity) using the HSV color model."},
		{"Change color satur. (HSV)", -2.0f, 2.0f, "Change the color saturation using the HSV color model."},
		{"Smudge", 0.0f, 1.0f, "Paint with the smudge color instead of the brush color."},
		{"Smudge length", 0.0f, 1.0f, "This controls how fast the smudge color becomes the color you are painting on."},
		{"Smudge radius", -1.6f, 1.6f, "This modifies the radius of the circle where color is picked up for smudging."},
		{"Eraser", 0.0f, 1.0f, "How much this tool behaves like an eraser"},
		{"Stroke threshold", 0.0f, 0.5f, "How much pressure is needed to start a stroke."},
		{"Stroke duration", -1.0f, 7.0f, "How far you have to move until the stroke input reaches 1.0."},
		{"Stroke hold time", 0.0f, 10.0f, "This defines how long the stroke input stays at 1.0."},
		{"Custom input", -5.0f, 5.0f, "Set the custom input to this value."},
		{"Custom input filter", 0.0f, 10.0f, "How slow the custom input actually follows the desired value."},
		{"Elliptical dab: ratio", 1.0f, 10.0f, "Aspect ratio of the dabs; must be >= 1.0"},
		{"Elliptical dab: angle", 0.0f, 180.0f, "Angle by which elliptical dabs are tilted"},
		{"Direction filter", 0.0f, 10.0f, "A low value will make the direction input adapt more quickly"},
		{"Lock alpha", 0.0f, 1.0f, "Do not modify the alpha channel of the layer"}
	};

	// build rows
	for (int i = 0; i < BRUSH_SETTINGS_COUNT; ++i) {
		const SettingInfo& info = setting_info[i];
		Gtk::Box *row_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 8));

		// label
		Gtk::Label *label = Gtk::manage(new Gtk::Label(_(info.display_name)));
		label->set_halign(Gtk::ALIGN_START);
		label->set_tooltip_text(_(info.tooltip));
		label->set_size_request(150, -1);
		label->set_ellipsize(Pango::ELLIPSIZE_END);
		label->set_xalign(0.0);
		row_box->pack_start(*label, false, false, 0);

		// slider
		Gtk::Scale *slider = Gtk::manage(new Gtk::Scale(Gtk::ORIENTATION_HORIZONTAL));
		slider->set_range(info.min_value, info.max_value);
		slider->set_digits(3);
		slider->set_draw_value(true);
		slider->set_value_pos(Gtk::POS_RIGHT);
		slider->set_tooltip_text(_(info.tooltip));
		slider->set_hexpand(true);

		float initial_value = selected_brush_button ? selected_brush_config.settings[i].base : info.min_value;
		slider->set_value(initial_value);
		// apply values changes to brush
		slider->signal_value_changed().connect([this, i, slider]() {
			selected_brush_config.settings[i].base = slider->get_value();
			if (action_) selected_brush_config.apply(action_->stroke.brush());
			if (i == BRUSH_RADIUS_LOGARITHMIC) {
				Real brush_radius = expf(selected_brush_config.settings[i].base);
				synfigapp::Main::set_bline_width(Distance(brush_radius, Distance::SYSTEM_UNITS));
				update_cursor();
			}
		});
		row_box->pack_start(*slider, true, true, 0);
		setting_controls[i] = slider;

		// reset button
		Gtk::Button *reset_btn = Gtk::manage(new Gtk::Button("↺"));
		reset_btn->set_valign(Gtk::ALIGN_CENTER);
		reset_btn->signal_clicked().connect([this, i, slider]() {
			slider->set_value(original_brush_config.settings[i].base);
		});
		row_box->pack_start(*reset_btn, false, false, 0);
		settings_grid->attach(*row_box, 0, i, 1, 1);
	}
	// add to tab
	settings_scroll->add(*settings_grid);
	Gtk::Label *tab_label = Gtk::manage(new Gtk::Label(_("Settings")));
	notebook->append_page(*settings_scroll, *tab_label);
	settings_scroll->show_all();
}

void
StateBrush2_Context::select_brush(Gtk::ToggleToolButton *button, String filename)
{
	if (button && button->get_active())
	{
		if (selected_brush_button)
			selected_brush_button->set_active(false);

		selected_brush_config.load(filename);
		eraser_checkbox.set_active(selected_brush_config.settings[BRUSH_ERASER].base > 0.0);
		selected_brush_button = button;
		original_brush_config = selected_brush_config;
		Real brush_radius = expf(selected_brush_config.settings[BRUSH_RADIUS_LOGARITHMIC].base);
		synfigapp::Main::set_bline_width(Distance(brush_radius, Distance::SYSTEM_UNITS));
		update_cursor();
		// apply brush settings to sliders
		for (int i = 0; i < BRUSH_SETTINGS_COUNT; ++i) {
			auto it = setting_controls.find(i);
			if (it != setting_controls.end()) {
				Gtk::Scale* slider = it->second;
				if (slider) {
					slider->set_value(selected_brush_config.settings[i].base);
				}
			}
		}
	}
}

void StateBrush2_Context::update_cursor()
{
	const float brush_radius_px = synfigapp::Main::get_bline_width();
	float display_radius = 10.0f;

	auto layer = find_or_create_layer();
	if (!layer || !layer->rendering_surface)
		return;
	rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(layer->rendering_surface);
	if (!lock || !lock->get_surface().is_valid())
		return;

	RendDesc& desc = get_canvas()->rend_desc();
	synfig::Rect layer_bounds(
		layer->get_param("tl").get(Point()),
		layer->get_param("br").get(Point()));

	TransformStack stack;
	build_transform_stack(get_canvas(), layer, get_canvas_view(), stack);
	synfig::Rect transformed_bounds = stack.perform(layer_bounds);
	int layer_pixel_width = lock->get_surface().get_w();
	float transformed_width = transformed_bounds.maxx - transformed_bounds.minx;
	float canvas_width = desc.get_br()[0] - desc.get_tl()[0];

	float scale_layer = transformed_width / layer_pixel_width;
	float scale_canvas = desc.get_w() / canvas_width;
	float scale_view = get_work_area()->get_zoom() * get_work_area()->get_scale_factor();

	display_radius = fabs(brush_radius_px * scale_layer * scale_canvas * scale_view);
	create_radius_cursor(display_radius);
}

void
StateBrush2_Context::create_radius_cursor(float radius)
{
	if (radius < 1.0f || radius > 200.0f) {
		get_work_area()->set_cursor(Gdk::PENCIL);
		return;
	}

	int diameter = static_cast<int>(ceil(radius * 2.0f));
	int size = diameter + 4;
	int hotspot = size / 2;

	auto pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, size, size);
	pixbuf->fill(0x00000000); // transparent

	auto surface = Cairo::ImageSurface::create(
		pixbuf->get_pixels(),
		Cairo::FORMAT_ARGB32,
		pixbuf->get_width(),
		pixbuf->get_height(),
		pixbuf->get_rowstride()
	);

	auto cr = Cairo::Context::create(surface);

	double center = hotspot + 0.5;
	cr->set_line_width(1.0);

	// Black outer circle
	cr->arc(center, center, radius, 0, 2 * M_PI);
	cr->set_source_rgba(0, 0, 0, 1);
	cr->stroke();

	// White inner circle
	cr->arc(center, center, radius - 1.0, 0, 2 * M_PI);
	cr->set_source_rgba(1, 1, 1, 1);
	cr->stroke();

	auto cursor = Gdk::Cursor::create(Gdk::Display::get_default(), pixbuf, hotspot, hotspot);
	get_work_area()->set_cursor(cursor);
}

void StateBrush2_Context::create_image_layer_dialog()
{
	// Create dialog window
	Gtk::Dialog dialog(_("Create Image Layer"), *App::main_window, true);
	dialog.add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
	dialog.add_button(_("Create"), Gtk::RESPONSE_OK);
	dialog.set_default_response(Gtk::RESPONSE_OK);

	// Create the layout grid
	Gtk::Grid* layout_grid = Gtk::manage(new Gtk::Grid());
	layout_grid->set_row_spacing(12);
	layout_grid->set_column_spacing(12);
	layout_grid->set_border_width(12);
	dialog.get_content_area()->pack_start(*layout_grid);

    // Get default width and height from the canvas
	auto canvas = get_canvas();
	int canvas_width = 512, canvas_height = 512;
	if (canvas) {
		canvas_width = canvas->rend_desc().get_w();
		canvas_height = canvas->rend_desc().get_h();
	}

	// Create spin buttons for width and height input
	auto width_input = Gtk::manage(new Gtk::SpinButton(Gtk::Adjustment::create(canvas_width, 1, 4096, 1, 10, 0)));
	auto height_input = Gtk::manage(new Gtk::SpinButton(Gtk::Adjustment::create(canvas_height, 1, 3112, 1, 10, 0)));
	width_input->set_numeric(true);
	height_input->set_numeric(true);

	// Presets
	auto preset_selector = Gtk::manage(new Gtk::ComboBoxText());
	preset_selector->append("custom", _("Custom Size"));
	preset_selector->append("4096x3112", _("4096x3112 4K"));
	preset_selector->append("2048x1556", _("2048x1556 2K"));
	preset_selector->append("1920x1080", _("1920x1080 HD"));
	preset_selector->append("1280x720", _("1280x720 HD"));
	preset_selector->append("720x576", _("720x576 PAL"));
	preset_selector->append("720x480", _("720x480 NTSC"));
	preset_selector->append("640x480", _("640x480 Web"));
	preset_selector->append("480x360", _("480x360 Web"));
	preset_selector->set_active_id("custom");

	// Add labels and widgets to the grid layout
	layout_grid->attach(*Gtk::manage(new Gtk::Label(_("Preset:"))), 0, 0, 1, 1);
	layout_grid->attach(*preset_selector, 1, 0, 2, 1);
	layout_grid->attach(*Gtk::manage(new Gtk::Label(_("Width:"))), 0, 1, 1, 1);
	layout_grid->attach(*width_input, 1, 1, 1, 1);
	layout_grid->attach(*Gtk::manage(new Gtk::Label(_("Height:"))), 0, 2, 1, 1);
	layout_grid->attach(*height_input, 1, 2, 1, 1);

	// Link width and height toggle button
	auto link_label = _("link width and height");
	auto unlink_label = _("Unlink width and height");
	auto aspect_link = Gtk::manage(new studio::Widget_Link(link_label, unlink_label));
	aspect_link->set_active(true);
	layout_grid->attach(*aspect_link, 2, 1, 1, 2);

	bool is_updating = false;
	double aspect_ratio = (double)canvas_width / (double)canvas_height;

	// Update aspect ratio value when toggling the link button
	aspect_link->signal_toggled().connect([&]() {
		if (aspect_link->get_active())
			aspect_ratio = (double)width_input->get_value() / (double)height_input->get_value();
	});

	// Automatically update height when width changes (if aspect ratio is locked)
	width_input->signal_value_changed().connect([&]() {
		if (is_updating || !aspect_link->get_active()) return;
		is_updating = true;
		height_input->set_value(std::max(1.0, width_input->get_value() / aspect_ratio));
		is_updating = false;
	});
	height_input->signal_value_changed().connect([&]() {
		if (is_updating || !aspect_link->get_active()) return;
		is_updating = true;
		width_input->set_value(std::max(1.0, height_input->get_value() * aspect_ratio));
		is_updating = false;
	});

	// Update width and height when preset is selected
	preset_selector->signal_changed().connect([&]() {
		std::string selected_preset = preset_selector->get_active_id();
		if (selected_preset == "custom") {
			width_input->set_sensitive(true);
			height_input->set_sensitive(true);
			return;
		}

		size_t separator = selected_preset.find('x');
		if (separator == std::string::npos) return;

		int preset_width = std::stoi(selected_preset.substr(0, separator));
		int preset_height = std::stoi(selected_preset.substr(separator + 1));

		is_updating = true;
		width_input->set_value(preset_width);
		height_input->set_value(preset_height);
		width_input->set_sensitive(false);
		height_input->set_sensitive(false);
		aspect_link->set_active(true);
		aspect_ratio = (double)preset_width / (double)preset_height;
		is_updating = false;
	});
	dialog.show_all();

	// Generates next unique image ID
	auto generate_next_id = [](std::string current_id) {
		int number = 1, digit_count = 0;
		if (!current_id.empty() && isdigit(current_id.back())) {
			while (digit_count < current_id.size() && isdigit(current_id[current_id.size() - 1 - digit_count]))
				digit_count++;
			std::string number_part = current_id.substr(current_id.size() - digit_count, digit_count);
			current_id = current_id.substr(0, current_id.size() - digit_count);
			number = atoi(number_part.c_str());
		}
		number++;
		char buffer[32];
		int min_digits = std::min(digit_count, 9);
		snprintf(buffer, sizeof(buffer), "%s%0*d", current_id.c_str(), min_digits, number);
		return std::string(buffer);
	};

	// Load previous brush image ID
	std::string image_layer_id = settings.get_value("brush.image_id", std::string("brush image001"));

    // Dialog event loop: creates image layer if user clicks OK
    while (dialog.run() == Gtk::RESPONSE_OK) {
        int final_width = width_input->get_value_as_int();
        int final_height = height_input->get_value_as_int();

        // Group canvas actions for undo
        synfigapp::Action::PassiveGrouper action_group(
            get_canvas_interface()->get_instance().get(), _("Create Image Layer")
        );

		Layer::Handle new_layer = get_canvas_interface()->add_layer_to("import", get_canvas(), 0);
		Layer_Bitmap::Handle bitmap_layer = etl::handle<synfig::Layer_Bitmap>::cast_dynamic(new_layer);

		bitmap_layer->set_description(image_layer_id);
		get_canvas_interface()->signal_layer_new_description()(bitmap_layer, bitmap_layer->get_description());

        if (canvas) {
            bitmap_layer->set_param("tl", canvas->rend_desc().get_tl());
            bitmap_layer->set_param("br", canvas->rend_desc().get_br());
            bitmap_layer->set_param("c", int(1));
        }

        // Initialize a blank surface to the layer
        if (final_width > 0 && final_height > 0) {
            Surface* blank_surface = new Surface(final_width, final_height);
            blank_surface->clear();
            bitmap_layer->rendering_surface = new rendering::SurfaceResource(
                new rendering::SurfaceSW(*blank_surface, true)
            );

				if (bitmap_layer->get_param_list().count("filename") != 0)
				{
					// generate name based on description
					String description, filename, filename_param;
					get_canvas_interface()
						->get_instance()
						->generate_new_name(
							bitmap_layer,
							description,
							filename,
							filename_param );

					// create and save surface
					get_canvas_interface()
						->get_instance()
						->save_surface(bitmap_layer->rendering_surface, filename);

				bitmap_layer->set_param("filename", filename_param);
				bitmap_layer->set_description(description);
			}
		}
		get_canvas_view()->get_selection_manager()->clear_selected_layers();
		get_canvas_view()->get_selection_manager()->set_selected_layer(bitmap_layer);
		image_layer_id = generate_next_id(image_layer_id);
		settings.set_value("brush.image_id", image_layer_id);
		break;
	}
}

Smach::event_result
StateBrush2_Context::event_refresh_tool_options(const Smach::event& /*x*/)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

void
StateBrush2_Context::update_overlay_preview(const synfig::Surface& surface, const synfig::Rect& rect)
{
	if (get_work_area() && get_work_area()->get_renderer_brush_overlay()) {
	  get_work_area()->get_renderer_brush_overlay()->set_overlay_surface(surface, rect);
	}
}

void
StateBrush2_Context::clear_overlay_preview()
{
	if (get_work_area() && get_work_area()->get_renderer_brush_overlay()) {
	  get_work_area()->get_renderer_brush_overlay()->clear_overlay();
	}
}

void
StateBrush2_Context::draw_to(Vector event_pos, Real pressure)
{
	Layer_Bitmap::Handle layer = find_or_create_layer();

	synfig::Rect layer_bounds = layer->get_bounding_rect();
	synfig::Rect full_layer_bounds;

	if (!transform_stack_.empty()) {
	  Point tl_transformed = transform_stack_.perform(Point(layer_bounds.minx, layer_bounds.miny));
	  Point br_transformed = transform_stack_.perform(Point(layer_bounds.maxx, layer_bounds.maxy));
	  full_layer_bounds = synfig::Rect(
	      std::min(tl_transformed[0], br_transformed[0]),
	      std::min(tl_transformed[1], br_transformed[1]),
	      std::max(tl_transformed[0], br_transformed[0]),
	      std::max(tl_transformed[1], br_transformed[1])
	  );
	} else {
	  full_layer_bounds = layer_bounds;
	}

	// transform coords
	Point pos = transform_stack_.unperform(event_pos);
	Point layer_tl = layer->get_param("tl").get(Point());
	Point layer_br = layer->get_param("br").get(Point());
	float surface_x = ((pos[0] - layer_tl[0]) / (layer_br[0] - layer_tl[0])) * overlay_surface_.get_w();
	float surface_y = ((pos[1] - layer_tl[1]) / (layer_br[1] - layer_tl[1])) * overlay_surface_.get_h();

	// calcluate dtime
	Glib::TimeVal current_time;
	current_time.assign_current_time();
	double dtime = (current_time - time_).as_double();
	time_ = current_time;

	// apply stroke point
	brushlib::SurfaceWrapper wrapper(&overlay_surface_);
	action_->stroke.brush().stroke_to(&wrapper, surface_x, surface_y, pressure, 0.0f, 0.0f, dtime);
	action_->stroke.add_point({surface_x, surface_y, pressure, dtime});
	update_overlay_preview(overlay_surface_, overlay_rect_);
}

Layer_Bitmap::Handle StateBrush2_Context::find_or_create_layer()
{
	// is the selected layer a bitmap ?
	Layer::Handle selected_layer = canvas_view_->get_selection_manager()->get_selected_layer();
	Layer_Bitmap::Handle layer = Layer_Bitmap::Handle::cast_dynamic(selected_layer);

	if (layer) {
	  return layer;
	}

	// is it a switch && active layer is a bitmap ?
	if (!layer) {
	  etl::handle<Layer_Switch> layer_switch = etl::handle<Layer_Switch>::cast_dynamic(selected_layer);
	  if (layer_switch) {
	      Layer::Handle current_layer = layer_switch->get_current_layer();
	      if (current_layer) {
	          layer = Layer_Bitmap::Handle::cast_dynamic(current_layer);
	          if (layer) {
	              return layer;
	          }
	      }
	  }
	}

	// creates a canvas size layer
	if (!layer) {
	  Canvas::Handle canvas = canvas_view_->get_canvas();
	  Layer::Handle new_layer = canvas_view_->canvas_interface()->add_layer_to("import", canvas);
	  if (!new_layer)
	      return Layer_Bitmap::Handle();

	  layer = etl::handle<synfig::Layer_Bitmap>::cast_dynamic(new_layer);
	  if (!layer)
	      return Layer_Bitmap::Handle();

	  layer->set_description(_("brush image"));

	  Point canvas_tl = canvas->rend_desc().get_tl();
	  Point canvas_br = canvas->rend_desc().get_br();

	  layer->set_param("tl", canvas_tl);
	  layer->set_param("br", canvas_br);
	  layer->set_param("c", int(1));

	  int width = canvas->rend_desc().get_w();
	  int height = canvas->rend_desc().get_h();

	  if (width > 0 && height > 0) {
	      Surface *initial_surface = new Surface(width, height);
	      initial_surface->clear();
	      layer->rendering_surface = new rendering::SurfaceResource(
	          new rendering::SurfaceSW(*initial_surface, true));

	      if (layer->get_param_list().count("filename") != 0)
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

	          layer->set_param("filename", filename_param);
	          layer->set_description(description);
	      }
	  }

	  canvas_view_->get_selection_manager()->clear_selected_layers();
	  canvas_view_->get_selection_manager()->set_selected_layer(layer);
	}

	return layer;
}

bool
StateBrush2_Context::build_transform_stack(
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
	 if(Layer_PasteCanvas::Handle layer_pastecanvas = Layer_PasteCanvas::Handle::cast_dynamic(*i))
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
StateBrush2_Context::event_mouse_down_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	if (event.button != BUTTON_LEFT)
	  return Smach::RESULT_OK;

	// clear any pending overlay clear signal
	if (clear_overlay_timer_.connected()) {
	  clear_overlay_timer_.disconnect();
	}

	Layer_Bitmap::Handle layer = find_or_create_layer();
	if (!layer) return Smach::RESULT_OK;

	layer->set_active(false);
	action_ = new synfigapp::Action::LayerBrush();
	action_->set_param("canvas", get_canvas());
	action_->set_param("canvas_interface", get_canvas_interface());
	action_->set_param("layer", Layer::Handle(layer));

	int undo_mode = synfigapp::Main::settings().get_value("pref.brush_undo_mode", 1);
	action_->stroke.set_undo_mode(static_cast<synfigapp::Action::LayerBrush::BrushStroke::UndoMode>(undo_mode));

	transform_stack_.clear();
	if (!build_transform_stack(canvas_view_->get_canvas(), layer, canvas_view_, transform_stack_)) {
	  transform_stack_.clear();
	}

	// Get actual layer bounds after transformation
	synfig::Rect layer_bounds = layer->get_bounding_rect();
	layer_bounds = transform_stack_.empty() ? layer_bounds : transform_stack_.perform(layer_bounds);
	overlay_rect_ = layer_bounds;

	// copy layer to overlay
	if (layer->rendering_surface) {
		rendering::SurfaceResource::LockRead<rendering::SurfaceSW> lock(layer->rendering_surface);
		if (lock && lock->get_surface().is_valid()) {
			const Surface& src = lock->get_surface();
			overlay_surface_ = synfig::Surface(src.get_w(), src.get_h());
			overlay_surface_.copy(src);
		}
	}

	selected_brush_config.apply(action_->stroke.brush());
	Color color = synfigapp::Main::get_outline_color();
	Real epsilon = 0.00000001;
	Real r(color.get_r()), g(color.get_g()), b(color.get_b());
	Real max_rgb = std::max(r, std::max(g, b));
	Real min_rgb = std::min(r, std::min(g, b));
	Real diff = max_rgb-min_rgb;

	Real val = max_rgb;
	Real sat = fabs(max_rgb) > epsilon ? 1.0 - (min_rgb / max_rgb) : 0;
	Real hue = fabs(diff) <= epsilon ?
			0 : max_rgb == r ?
				60.0 * fmod ((g - b)/(diff), 6.0) : max_rgb == g ?
					60.0 * (((b - r)/(diff))+2.0) : 60.0 * (((r - g)/(diff))+4.0);

	Real radius = synfigapp::Main::get_bline_width();
	action_->stroke.brush().set_base_value(BRUSH_COLOR_H, hue/360.0);
	action_->stroke.brush().set_base_value(BRUSH_COLOR_S, sat);
	action_->stroke.brush().set_base_value(BRUSH_COLOR_V, val);
	action_->stroke.brush().set_base_value(BRUSH_RADIUS_LOGARITHMIC, log(radius));
	action_->stroke.prepare();
	update_cursor();

	// paint first point
	Point pos = transform_stack_.unperform(event.pos);
	Point layer_tl = layer->get_param("tl").get(Point());
	Point layer_br = layer->get_param("br").get(Point());
	float surface_x = ((pos[0] - layer_tl[0]) / (layer_br[0] - layer_tl[0])) * overlay_surface_.get_w();
	float surface_y = ((pos[1] - layer_tl[1]) / (layer_br[1] - layer_tl[1])) * overlay_surface_.get_h();
	draw_to(event.pos , event.pressure);
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateBrush2_Context::event_mouse_draw_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	if (event.button != BUTTON_LEFT || !action_)
		return Smach::RESULT_OK;
	draw_to(event.pos, event.pressure);
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateBrush2_Context::event_mouse_up_handler(const Smach::event& x)
{
	const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));
	if (event.button != BUTTON_LEFT || !action_)
		return Smach::RESULT_OK;

	Layer_Bitmap::Handle layer = find_or_create_layer();
	if (!layer) return Smach::RESULT_OK;
	layer->set_active(true);

	if (action_->is_ready()) {
		get_canvas_interface()->get_instance()->perform_action(action_);
	}
	action_ = nullptr;
	transform_stack_.clear();

	if (layer && layer->rendering_surface) {
		// save the updated surface to file
		String filename_param = layer->get_param("filename").get(String());
	if (!filename_param.empty()) {
			get_canvas_interface()
				->get_instance()
				->save_surface(layer->rendering_surface, filename_param);
		}
	}
	// delay to allow the layer to render
	// todo : replace with an accurate signal
	clear_overlay_timer_ = Glib::signal_timeout().connect(
		[this]() -> bool {
			clear_overlay_preview();
			return false;
		},
		200
	);
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateBrush2_Context::event_stop_handler(const Smach::event& x)
{
	clear_overlay_preview();
	throw &state_normal;
	action_ = nullptr;
	return Smach::RESULT_OK;
}
