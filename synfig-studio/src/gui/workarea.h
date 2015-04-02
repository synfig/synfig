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
#include <synfig/general.h>
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

/*
namespace etl {

template <typename T_, typename C_=std::less<T_,T_> >
class dereferenced_compare
{
public:
	typedef etl::loose_handle<T_> first_argument_type;
	typedef etl::loose_handle<T_> second_argument_type;
	typedef bool result_type;

}
};
*/

namespace synfigapp { class CanvasInterface; };

namespace synfig { class Layer; };
namespace Gtk { class Frame; };

namespace studio
{
class WorkAreaTarget;
class WorkAreaTarget_Full;
class WorkAreaTarget_Cairo;
class WorkAreaTarget_Cairo_Tile;

class Instance;
class CanvasView;
class WorkArea;
class WorkAreaRenderer;
class AsyncRenderer;
class DirtyTrap
{
	friend class WorkArea;
	WorkArea *work_area;
public:
	DirtyTrap(WorkArea *work_area);
	~DirtyTrap();
};


class WorkArea : public Gtk::Table, public Duckmatic
{
	friend class WorkAreaTarget;
	friend class WorkAreaTarget_Full;
	friend class WorkAreaTarget_Cairo;
	friend class WorkAreaTarget_Cairo_Tile;
	friend class DirtyTrap;
	friend class WorkAreaRenderer;
	friend class WorkAreaProgress;

	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	class PushState;
	friend class PushState;

	void insert_renderer(const etl::handle<WorkAreaRenderer> &x);
	void insert_renderer(const etl::handle<WorkAreaRenderer> &x,int priority);
	void erase_renderer(const etl::handle<WorkAreaRenderer> &x);
	void resort_render_set();

	enum DragMode
	{
		DRAG_NONE=0,
		DRAG_WINDOW,
		DRAG_DUCK,
		DRAG_GUIDE,
		DRAG_BOX,
		DRAG_BEZIER
	};
	// Class used to store the cairo surface
	class SurfaceElement
	{
	public:
		cairo_surface_t* surface;
		int refreshes;
		SurfaceElement()
		{
			surface=NULL;
			refreshes=0;
		}
		//Copy constructor
		SurfaceElement(const SurfaceElement& other): surface(cairo_surface_reference(other.surface)), refreshes(other.refreshes)
		{
		}
		~SurfaceElement()
		{
			if(surface)
				cairo_surface_destroy(surface);
		}
	};

	typedef std::vector<SurfaceElement>	 SurfaceBook;

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	std::set<etl::handle<WorkAreaRenderer> > renderer_set_;

	etl::handle<studio::AsyncRenderer> async_renderer;


	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface;
	etl::handle<synfig::Canvas> canvas;
	etl::loose_handle<studio::Instance> instance;
	etl::loose_handle<studio::CanvasView> canvas_view;

	// Widgets
	Gtk::DrawingArea *drawing_area;
	Glib::RefPtr<Gtk::Adjustment> scrollx_adjustment;
	Glib::RefPtr<Gtk::Adjustment> scrolly_adjustment;
	// TODO: Implement Rulers
	Widget_Ruler *vruler;
	Widget_Ruler *hruler;
	Gtk::Button *menubutton;
	Gtk::Frame *drawing_frame;

	GdkDevice* curr_input_device;

	// Bleh!
	int	w;						//!< Width of the image (in pixels)
	int	h;						//!< Height of the image (in pixels)
	synfig::Real	canvaswidth;	//!< Width of the canvas
	synfig::Real	canvasheight;	//!< Height of the canvas
	synfig::Real	pw;				//!< The width of a pixel
	synfig::Real	ph;				//!< The height of a pixel
	// float zoom and prev_zoom are declared in Duckmatic
	synfig::Point window_tl;		//!< The (theoretical) top-left corner of the view window
	synfig::Point window_br;		//!< The (theoretical) bottom-right corner of the view window

	guint32 last_event_time;

	int bpp;
	//unsigned char *buffer;

	//! ???
	synfig::ProgressCallback *progresscallback;

	//! ???
	synfig::RendDesc desc;

	//! This flag is set if the user is dragging the video window
	/*! \see drag_point */
	DragMode dragging;

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

	synfig::Time jack_offset;

	bool low_resolution;

	bool meta_data_lock;

	//! This flag is set if the entire frame is rendered rather than using tiles
	bool full_frame;

	//Glib::RefPtr<Gdk::Pixbuf> pix_buf;

	//! This vector holds all of the tiles for this frame
	std::vector< std::pair<Glib::RefPtr<Gdk::Pixbuf>,int> > tile_book;
	// This vector holds all the cairo surfaces for the frame 
	SurfaceBook cairo_book;

	//! This integer describes the total times that the work area has been refreshed
	int refreshes;

	//! This list holds the queue of tiles that need to be rendered
	//std::list<int> tile_queue;

	int tile_w, tile_h;

	gint render_idle_func_id;

	//! The coordinates of the focus the last time a part of the screen was refreshed
	synfig::Point last_focus_point;

	bool canceled_;

	int quality;
	int low_res_pixel_size;

	bool dirty_trap_enabled;

	int dirty_trap_queued;

	// This flag is set if onion skin is visible
	bool onion_skin;
	//! stores the future [1] and past [0] onion skins based on keyframes
	int onion_skins[2];

	etl::loose_handle<synfig::ValueNode> selected_value_node_;

	bool allow_duck_clicks;
	bool allow_bezier_clicks;
	bool allow_layer_clicks;
	bool cancel;
	bool curr_guide_is_x;
	bool dirty;
	bool queued;
	bool rendering;
	
#ifdef SINGLE_THREADED
	/* resize bug workaround */
	int old_window_width;
	int old_window_height;
#endif

	/*
 -- ** -- P U B L I C   D A T A -----------------------------------------------
	*/

public:

	const etl::loose_handle<synfig::ValueNode>& get_selected_value_node() { return  selected_value_node_; }
	const synfig::Point& get_drag_point()const { return drag_point; }
	std::vector< std::pair<Glib::RefPtr<Gdk::Pixbuf>,int> >& get_tile_book(){ return tile_book; }
	SurfaceBook& get_cairo_book() { return cairo_book; }
	int get_refreshes()const { return refreshes; }
	bool get_canceled()const { return canceled_; }
	bool get_queued()const { return queued; }
	bool get_rendering()const { return rendering; }
#ifdef SINGLE_THREADED
	bool get_updating()const;
	void stop_updating(bool cancel = false);
#endif
	bool get_full_frame()const { return full_frame; }
	//int get_w()const { return w; }
	//int get_h()const { return h; }

	int get_tile_w()const { return tile_w; }
	int get_tile_h()const { return tile_h; }

	bool get_allow_layer_clicks() { return allow_layer_clicks; }
	void set_allow_layer_clicks(bool value) { allow_layer_clicks=value; }

	bool get_allow_duck_clicks() { return allow_duck_clicks; }
	void set_allow_duck_clicks(bool value) { allow_duck_clicks=value; }

	bool get_allow_bezier_clicks() { return allow_bezier_clicks; }
	void set_allow_bezier_clicks(bool value) { allow_bezier_clicks=value; }

	// used in renderer_ducks.cpp
	bool solid_lines;

	// used in renderer_guides.cpp
	GuideList::iterator curr_guide;

	// used in renderer_timecode.cpp
	int timecode_width, timecode_height;

	// used in renderer_bonesetup.cpp
	int bonesetup_width, bonesetup_height;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	//unsigned char *get_buffer() { return buffer; }
	bool set_wh(int w, int h,int chan=3);

	int next_unrendered_tile(int refreshes)const;
	int next_unrendered_tile()const { return next_unrendered_tile(refreshes); }

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

private:

	sigc::signal<void,GdkDevice* > signal_input_device_changed_;

	//! One signal per button
	sigc::signal<void,synfig::Point> signal_user_click_[5];

	sigc::signal<void> signal_popup_menu_;

	sigc::signal<void> signal_cursor_moved_;
	sigc::signal<void> signal_rendering_;

	//! Signal for when the user clicks on a layer
	sigc::signal<void, etl::handle<synfig::Layer> > signal_layer_selected_;

	sigc::signal<void> signal_view_window_changed_;

	sigc::signal<void> signal_meta_data_changed_;

public:

	sigc::signal<void>& signal_rendering() { return signal_rendering_; }

	sigc::signal<void>& signal_cursor_moved() { return signal_cursor_moved_; }

	sigc::signal<void>& signal_view_window_changed() { return signal_view_window_changed_; }

	sigc::signal<void>& signal_meta_data_changed() { return signal_meta_data_changed_; }

	void view_window_changed() { signal_view_window_changed()(); }

	sigc::signal<void,GdkDevice* >& signal_input_device_changed() { return signal_input_device_changed_; }

	sigc::signal<void> &signal_popup_menu() { return signal_popup_menu_; }

	//! One signal per button (5 buttons)
	sigc::signal<void,synfig::Point> &signal_user_click(int button=0){ return signal_user_click_[button]; }

	sigc::signal<void, etl::handle<synfig::Layer> >& signal_layer_selected() { return signal_layer_selected_; }

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	void set_onion_skin(bool x);
	bool get_onion_skin()const;
	void toggle_onion_skin() { set_onion_skin(!get_onion_skin()); }
	void set_onion_skins(int *onions);
	int const * get_onion_skins()const;

	void set_selected_value_node(etl::loose_handle<synfig::ValueNode> x);

	bool is_dragging() { return dragging!=DRAG_NONE; }

	DragMode get_dragging_mode() { return dragging; }

	WorkArea(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface);
	virtual ~WorkArea();

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
	etl::handle<synfig::Canvas> get_canvas()const { return canvas; }
	etl::handle<studio::Instance> get_instance()const { return instance; }
	etl::loose_handle<studio::CanvasView> get_canvas_view()const { return canvas_view; }

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
	const synfig::Vector &get_background_size()const { return background_size;}
	//! Returns the first color of the checker background
	const synfig::Color &get_background_first_color()const { return background_first_color;}
	//! Returns the second color of the checker background
	const synfig::Color &get_background_second_color()const { return background_second_color;}

	bool get_low_resolution_flag()const { return low_resolution; }
	void set_low_resolution_flag(bool x);
	void toggle_low_resolution_flag();

	//! ???
	void queue_scroll();

	//! ??
	void popup_menu();

	int get_quality()const { return quality; }
	int get_low_res_pixel_size()const { return low_res_pixel_size; }

	void set_quality(int x);
	void set_low_res_pixel_size(int x);


	int get_w()const { return w; }
	int get_h()const { return h; }
	int get_bpp()const { return bpp; }

	//! ??
	const synfig::RendDesc &get_rend_desc()const { return desc; }

	//! ??
	void set_rend_desc(const synfig::RendDesc &x) { desc=x; }

	//! Converts screen coords (ie: pixels) to composition coordinates
	synfig::Point screen_to_comp_coords(synfig::Point pos)const;

	//! Converts composition coordinates to screen coords (ie: pixels)
	synfig::Point comp_to_screen_coords(synfig::Point pos)const;

	float get_pw()const { return pw; }
	float get_ph()const { return ph; }

	const synfig::Point &get_window_tl()const { return window_tl; }
	const synfig::Point &get_window_br()const { return window_br; }


	bool async_update_preview();
	void async_update_finished();
	void async_render_preview(synfig::Time time);
	void async_render_preview();

	bool sync_update_preview();
	bool sync_render_preview(synfig::Time time);
	bool sync_render_preview();
	void sync_render_preview_hook();

	void queue_render_preview();


	void queue_draw_preview();

	void zoom_in();
	void zoom_out();
	void zoom_fit();
	void zoom_norm();
	float get_zoom()const { return zoom; } // zoom is declared in Duckmatic

	void set_zoom(float z);


	void set_progress_callback(synfig::ProgressCallback *x)	{ progresscallback=x; }
	synfig::ProgressCallback *get_progress_callback() { return progresscallback; }

	void set_focus_point(const synfig::Point &x);

	synfig::Point get_focus_point()const;

	void done_rendering();

#ifdef SINGLE_THREADED
	/* resize bug workaround */
	void refresh_second_check();
#endif
	bool refresh(const Cairo::RefPtr<Cairo::Context> &cr);

	void reset_cursor();
	void refresh_cursor();

	void save_meta_data();
	void load_meta_data();
	//! Test initial meta data values
	bool have_meta_data();

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:
	bool on_key_press_event(GdkEventKey* event);
	bool on_key_release_event(GdkEventKey* event);
	bool on_drawing_area_event(GdkEvent* event);
	bool on_hruler_event(GdkEvent* event);
	bool on_vruler_event(GdkEvent* event);

	/*
 -- ** -- S T A T I C   P U B L I C   M E T H O D S ---------------------------
	*/

public:

	/*
 -- ** -- S T A T I C   P R I V A T E   M E T H O D S -------------------------
	*/

private:

	static gboolean __render_preview(gpointer data);
#ifdef SINGLE_THREADED
	/* resize bug workaround */
	static gboolean __refresh_second_check(gpointer data);
#endif

}; // END of class WorkArea

/*! \class WorkArea::PushState
**	Saves the current duck view and editing options
**  Should be used by tools that hide ducks or change clickability settings */
class WorkArea::PushState
{
	WorkArea *workarea_;
	Type type_mask;
	bool allow_duck_clicks;
	bool allow_bezier_clicks;
	bool allow_layer_clicks;

	bool needs_restore;

public:
	PushState(WorkArea *workarea_);
	~PushState();
	void restore();
}; // END of class WorkArea::PushState

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
