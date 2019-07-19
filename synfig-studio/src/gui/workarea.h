/* === S Y N F I G ========================================================= */
/*!	\file workarea.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Nikita Kitaev
**  ......... ... 2018 Ivan Mahonin
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_WORKAREA_H
#define __SYNFIG_GTKMM_WORKAREA_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <map>
#include <set>

#include <ETL/smart_ptr>
#include <ETL/handle>

#include <gtkmm/drawingarea.h>
#include <gtkmm/table.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/image.h>
#include <gdkmm/pixbuf.h>
#include <gdkmm/cursor.h>
#include <gdkmm/device.h>

#include <synfig/time.h>
#include <synfig/vector.h>
#include <synfig/renddesc.h>
#include <synfig/canvas.h>

#include "dials/zoomdial.h"
#include "widgets/widget_ruler.h"
#include "duckmatic.h"
#include "instance.h"
#include "app.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Frame; };
namespace synfig { class Layer; };
namespace synfigapp { class CanvasInterface; };

namespace studio
{

class Instance;
class CanvasView;
class WorkArea;
class WorkAreaRenderer;
class AsyncRenderer;
class Renderer_Canvas;
class LockDucks;


class WorkArea : public Gtk::Table, public Duckmatic
{
public:
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

	enum DragMode
	{
		DRAG_NONE=0,
		DRAG_WINDOW,
		DRAG_DUCK,
		DRAG_GUIDE,
		DRAG_BOX,
		DRAG_BEZIER,
		DRAG_ZOOM_WINDOW,
		DRAG_ROTATE_WINDOW
	};

	/*! \class WorkArea::PushState
	**	Saves the current duck view and editing options
	**  Should be used by tools that hide ducks or change clickability settings */
	class PushState
	{
	public:
		WorkArea &workarea;
		const Type type_mask;
		const bool allow_duck_clicks;
		const bool allow_bezier_clicks;
		const bool allow_layer_clicks;
		PushState(WorkArea &workarea);
		~PushState();
	}; // END of class WorkArea::PushState

	/*! \class WorkArea::DirtyTrap
	**  While any DirtyTrap exists it accumulates WorkArea::queue_render() calls and blocks them.
	**  WorkArea::queue_render() will called once after all DirtyTrap's for this WorkArea destroyed. */
	class DirtyTrap
	{
	public:
		WorkArea &work_area;
		DirtyTrap(WorkArea &work_area);
		~DirtyTrap();
	};

	friend class DirtyTrap;
	friend class WorkAreaRenderer;
	friend class WorkAreaProgress;

private:
	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

	std::set<etl::handle<WorkAreaRenderer> > renderer_set_;

	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface;
	etl::handle<synfig::Canvas> canvas;
	etl::loose_handle<studio::Instance> instance;
	etl::loose_handle<studio::CanvasView> canvas_view;
	etl::handle<Renderer_Canvas> renderer_canvas;

	// Widgets
	Gtk::DrawingArea *drawing_area;
	Gtk::Frame *drawing_frame;
	Widget_Ruler *hruler;
	Widget_Ruler *vruler;
	Glib::RefPtr<Gtk::Adjustment> scrollx_adjustment;
	Glib::RefPtr<Gtk::Adjustment> scrolly_adjustment;
	ZoomDial *zoomdial;

	GdkDevice* curr_input_device;

	// Bleh!
	int	w;						//!< Width of the image (in pixels)
	int	h;						//!< Height of the image (in pixels)
	int	thumb_w;				//!< Width of the thumbnail image (in pixels)
	int	thumb_h;				//!< Height of the thumbnail image (in pixels)
	synfig::Real canvaswidth;	//!< Width of the canvas
	synfig::Real canvasheight;	//!< Height of the canvas
	synfig::Real pw;			//!< The width of a pixel
	synfig::Real ph;			//!< The height of a pixel
	synfig::Point window_tl;	//!< The (theoretical) top-left corner of the view window
	synfig::Point window_br;	//!< The (theoretical) bottom-right corner of the view window

	guint32 last_event_time;

	//! ???
	synfig::ProgressCallback *progresscallback;

	//! This flag is set if the user is dragging the video window
	/*! \see drag_point */
	DragMode drag_mode;

	etl::handle<Duckmatic::Duck> clicked_duck;
	etl::handle<Duckmatic::Duck> hover_duck;

	//! When dragging the viewport, this is set to the origin of the drag
	synfig::Point drag_point;

	synfig::Point curr_point;

	//! ???
	synfig::Point previous_focus;

	//! This flag is set if the grid should be drawn
	bool show_grid;

	//! This flag is set if the guides should be drawn
	bool show_guides;

	//! Checker background size
	synfig::Vector background_size;
	//! Checker background first color
	synfig::Color background_first_color;
	//! Checker background second color
	synfig::Color background_second_color;
	//! Checker background pattern (will generated by first request)
	mutable Cairo::RefPtr<Cairo::SurfacePattern> background_pattern;

	synfig::Time jack_offset;

	bool low_resolution;

	bool meta_data_lock;

	//! The coordinates of the focus the last time a part of the screen was refreshed
	synfig::Point last_focus_point;

	int low_res_pixel_size;

	int dirty_trap_count;
	int dirty_trap_queued;

	// This flag is set if onion skin is visible
	bool onion_skin;
	//! stores the future [1] and past [0] onion skins based on keyframes
	int onion_skins[2];

	// render future and past frames in background
	bool background_rendering;

	etl::loose_handle<synfig::ValueNode> selected_value_node_;

	bool allow_duck_clicks;
	bool allow_bezier_clicks;
	bool allow_layer_clicks;
	bool curr_guide_is_x;

	etl::handle<LockDucks> lock_ducks;

public:
	/*
 -- ** -- P U B L I C   D A T A -----------------------------------------------
	*/

	// used in renderer_ducks.cpp
	bool solid_lines;

	// used in renderer_guides.cpp
	GuideList::iterator curr_guide;

	// used in renderer_timecode.cpp
	int timecode_width, timecode_height;

	// used in renderer_bonesetup.cpp
	int bonesetup_width, bonesetup_height;

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/
private:
	sigc::signal<void> signal_rendering_;
	sigc::signal<void, synfig::Time> signal_rendering_tile_finished_;
	sigc::signal<void> signal_cursor_moved_;
	sigc::signal<void> signal_view_window_changed_;
	sigc::signal<void> signal_meta_data_changed_;
	sigc::signal<void, GdkDevice*> signal_input_device_changed_;
	sigc::signal<void> signal_popup_menu_;
	sigc::signal<void, synfig::Point> signal_user_click_[5]; //!< One signal per button
	sigc::signal<void, etl::handle<synfig::Layer> > signal_layer_selected_; //!< Signal for when the user clicks on a layer

public:
	sigc::signal<void>& signal_rendering() { return signal_rendering_; }
	sigc::signal<void, synfig::Time>& signal_rendering_tile_finished() { return signal_rendering_tile_finished_; }
	sigc::signal<void>& signal_cursor_moved() { return signal_cursor_moved_; }
	sigc::signal<void>& signal_view_window_changed() { return signal_view_window_changed_; }
	sigc::signal<void>& signal_meta_data_changed() { return signal_meta_data_changed_; }
	sigc::signal<void, GdkDevice*>& signal_input_device_changed() { return signal_input_device_changed_; }
	sigc::signal<void> &signal_popup_menu() { return signal_popup_menu_; }
	sigc::signal<void, synfig::Point> &signal_user_click(int button=0){ return signal_user_click_[button]; } //!< One signal per button (5 buttons)
	sigc::signal<void, etl::handle<synfig::Layer> >& signal_layer_selected() { return signal_layer_selected_; }

private:
	/*
 -- ** -- P R I V A T E   M E T H O D S -----------------------------------------
	*/

	void set_drag_mode(DragMode mode);

public:
	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

	WorkArea(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface);
	virtual ~WorkArea();

	void view_window_changed() { signal_view_window_changed()(); }

	const etl::loose_handle<synfig::ValueNode>& get_selected_value_node() { return  selected_value_node_; }
	const synfig::Point& get_drag_point()const { return drag_point; }

	synfig::VectorInt get_windows_offset() const;
	synfig::RectInt get_window_rect() const;

	bool get_allow_layer_clicks() { return allow_layer_clicks; }
	void set_allow_layer_clicks(bool value) { allow_layer_clicks=value; }

	bool get_allow_duck_clicks() { return allow_duck_clicks; }
	void set_allow_duck_clicks(bool value) { allow_duck_clicks=value; }

	bool get_allow_bezier_clicks() { return allow_bezier_clicks; }
	void set_allow_bezier_clicks(bool value) { allow_bezier_clicks=value; }

	void insert_renderer(const etl::handle<WorkAreaRenderer> &x);
	void insert_renderer(const etl::handle<WorkAreaRenderer> &x,int priority);
	void erase_renderer(const etl::handle<WorkAreaRenderer> &x);
	void resort_render_set();

	void set_onion_skin(bool x);
	bool get_onion_skin() const { return onion_skin; }
	void set_onion_skins(int *onions);
	int const * get_onion_skins() const { return onion_skins; }

	void set_background_rendering(bool x);
	bool get_background_rendering() const { return background_rendering; }

	void set_selected_value_node(etl::loose_handle<synfig::ValueNode> x);

	DragMode get_drag_mode() { return drag_mode; }
	bool is_dragging() { return get_drag_mode() != DRAG_NONE; }

	void set_cursor(const Glib::RefPtr<Gdk::Cursor> &x);
	void set_cursor(Gdk::CursorType x);

	const synfig::Point& get_cursor_pos()const { return curr_point; }

	Glib::RefPtr<Gtk::Adjustment> get_scrollx_adjustment() { return scrollx_adjustment; }
	Glib::RefPtr<Gtk::Adjustment> get_scrolly_adjustment() { return scrolly_adjustment; }
	Glib::RefPtr<const Gtk::Adjustment> get_scrollx_adjustment() const { return scrollx_adjustment; }
	Glib::RefPtr<const Gtk::Adjustment> get_scrolly_adjustment() const { return scrolly_adjustment; }

	void set_instance(etl::loose_handle<studio::Instance> x) { instance=x; }
	void set_canvas(etl::handle<synfig::Canvas> x) { canvas=x; }
	void set_canvas_view(etl::loose_handle<studio::CanvasView> x) { canvas_view=x; }
	const etl::handle<synfig::Canvas>& get_canvas() const { return canvas; }
	const etl::loose_handle<studio::Instance>& get_instance() const { return instance; }
	const etl::loose_handle<studio::CanvasView>& get_canvas_view() const { return canvas_view; }
	const etl::handle<Renderer_Canvas>& get_renderer_canvas() const { return renderer_canvas; }

	void refresh_dimension_info();

	//! Enables showing of the grid
	void enable_grid();
	//! Disables showing of the grid
	void disable_grid();
	//! Toggles the showing of the grid
	void toggle_grid();
	//! Returns the state of the show_grid flag
	bool grid_status()const { return show_grid; }
	//! Toggles the snap of the grid
	void toggle_grid_snap();
	//! Sets the size of the grid
	void set_grid_size(const synfig::Vector &s);
	//! Sets the color of the grid
	void set_grid_color(const synfig::Color &c);
	//! Returns the color of the grid
	const synfig::Color &get_grid_color()const { return Duckmatic::get_grid_color();}

	//! Returns the state of the show_guides flag
	bool get_show_guides()const { return show_guides; }
	//! Sets the showing of the grid
	void set_show_guides(bool x);
	//! Toggles the showing of the guides
	void toggle_show_guides() { set_show_guides(!get_show_guides()); }
	//! Toggles the snap of the guides
	void toggle_guide_snap();
	//! Sets the color of the guides
	void set_guides_color(const synfig::Color &c);
	//! Returns the color of the guides
	const synfig::Color &get_guides_color()const { return Duckmatic::get_guides_color();}

	synfig::Time get_jack_offset()const { return jack_offset; }
	void set_jack_offset(const synfig::Time &x);

	//! Sets the size of the checker background
	void set_background_size(const synfig::Vector &s);
	//! Sets the first color of the checker background
	void set_background_first_color(const synfig::Color &c);
	//! Sets the second color of the checker background
	void set_background_second_color(const synfig::Color &c);
	//! Sets the size of the checker background
	const synfig::Vector& get_background_size() const { return background_size;}
	//! Returns the first color of the checker background
	const synfig::Color& get_background_first_color() const { return background_first_color;}
	//! Returns the second color of the checker background
	const synfig::Color& get_background_second_color() const { return background_second_color;}
	//! Returns the Cairo Pattern for background
	const Cairo::RefPtr<Cairo::SurfacePattern>& get_background_pattern() const;

	bool get_low_resolution_flag()const { return low_resolution; }
	void set_low_resolution_flag(bool x);
	void toggle_low_resolution_flag();

	//! ???
	void queue_scroll();

	//! ??
	void popup_menu();

	int get_low_res_pixel_size()const { return low_res_pixel_size; }
	synfig::String get_renderer() const;

	void set_low_res_pixel_size(int x);

	int get_w()const { return w; }
	int get_h()const { return h; }
	int get_thumb_w()const { return thumb_w; }
	int get_thumb_h()const { return thumb_h; }

	//! Converts screen coords (ie: pixels) to composition coordinates
	synfig::Point screen_to_comp_coords(synfig::Point pos)const;

	//! Converts composition coordinates to screen coords (ie: pixels)
	synfig::Point comp_to_screen_coords(synfig::Point pos)const;

	float get_pw()const { return pw; }
	float get_ph()const { return ph; }

	const synfig::Point &get_window_tl()const { return window_tl; }
	const synfig::Point &get_window_br()const { return window_br; }

	//! initiate background rendering of canvas and wait
	void sync_render(bool refresh = true);

	//! initiate background rendering of canvas
	void queue_render(bool refresh = true);

	void zoom_in();
	void zoom_out();
	void zoom_fit();
	void zoom_norm();
	void zoom_edit();
	float get_zoom()const { return zoom; } // zoom is declared in Duckmatic
	void set_zoom(float z);

	void set_progress_callback(synfig::ProgressCallback *x)	{ progresscallback=x; }
	synfig::ProgressCallback *get_progress_callback() { return progresscallback; }

	void set_focus_point(const synfig::Point &x);
	synfig::Point get_focus_point()const;

	bool refresh(const Cairo::RefPtr<Cairo::Context> &cr);

	void reset_cursor();
	void refresh_cursor();

	void save_meta_data();
	void load_meta_data();
	//! Test initial meta data values
	bool have_meta_data();

	void grab_focus();

private:
	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

	bool on_key_press_event(GdkEventKey* event);
	bool on_key_release_event(GdkEventKey* event);
	bool on_drawing_area_event(GdkEvent* event);
	bool on_hruler_event(GdkEvent* event);
	bool on_vruler_event(GdkEvent* event);
	void on_duck_selection_single(const etl::handle<Duck>& duck_guid);
}; // END of class WorkArea


}; // END of namespace studio

/* === E N D =============================================================== */

#endif
