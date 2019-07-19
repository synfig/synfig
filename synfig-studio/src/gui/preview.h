/* === S Y N F I G ========================================================= */
/*!	\file preview.h
**	\brief Previews an animation
**
**	$Id$
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_PREVIEW_H
#define __SYNFIG_PREVIEW_H

/* === H E A D E R S ======================================================= */
#include <ETL/handle>
#include <ETL/clock> /* indirectly includes winnt.h on WIN32 - needs to be included before gtkmm headers, which fix this */

#include <gtkmm/drawingarea.h>
#include <gtkmm/table.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/image.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/dialog.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/tooltip.h>
#include <gtkmm/alignment.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/hvscale.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/liststore.h>

#include <glibmm/dispatcher.h>

#include <synfig/time.h>
#include <synfig/vector.h>
#include <synfig/renddesc.h>
#include <synfig/canvas.h>

#include "dials/jackdial.h"

#include <vector>

#ifdef WITH_JACK
#include <jack/jack.h>
#include <jack/transport.h>
#endif

#include <synfig/soundprocessor.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class AsyncRenderer;
class CanvasView;

class Preview : public sigc::trackable, public etl::shared_object
{
public:
	class FlipbookElem
	{
	public:
		float t;
		Glib::RefPtr<Gdk::Pixbuf> buf; //at whatever resolution they are rendered at (resized at run time)
		cairo_surface_t* surface;
		FlipbookElem(): t(), surface(NULL) { }
		//Copy constructor
		FlipbookElem(const FlipbookElem& other): t(other.t) ,buf(other.buf), surface(cairo_surface_reference(other.surface))
		{
		}
		~FlipbookElem()
		{
			if(surface)
				cairo_surface_destroy(surface);
		}
	};

	etl::handle<studio::AsyncRenderer>	renderer;

	sigc::signal<void, Preview *>	signal_destroyed_;	//so things can reference us without fear

	typedef std::vector<FlipbookElem>	 FlipBook;
private:

	FlipBook frames;

	etl::loose_handle<CanvasView> canvasview;

	//synfig::RendDesc		description; //for rendering the preview...
	float	zoom,fps;
	float	begintime,endtime;
	float	jack_offset;
	bool 	overbegin,overend;
	int		quality;

	float	global_fps;

	//expose the frame information etc.
	class Preview_Target;
	class Preview_Target_Cairo;
	void frame_finish(const Preview_Target *);

	sigc::signal0<void>	sig_changed;

public:

	explicit Preview(const etl::loose_handle<CanvasView> &h = etl::loose_handle<CanvasView>(),
				float zoom = 0.5f, float fps = 15);
	~Preview();

	float 	get_zoom() const {return zoom;}
	void	set_zoom(float z){zoom = z;}

	float 	get_fps() const {return fps;}
	void	set_fps(float f){fps = f;}

	float 	get_global_fps() const {return global_fps;}
	void	set_global_fps(float f){global_fps = f;}

	float   get_jack_offset() const {return jack_offset;}
	void	set_jack_offset(float t){jack_offset = t;}

	float	get_begintime() const
	{
		if(overbegin)
			return begintime;
		else if(canvasview)
			return get_canvas()->rend_desc().get_time_start();
		else return -1;
	}

	float	get_endtime() const
	{
		if(overend)
			return endtime;
		else if(canvasview)
			return get_canvas()->rend_desc().get_time_end();
		else return -1;
	}

	void	set_begintime(float t)	{begintime = t;}
	void	set_endtime(float t) 	{endtime = t;}

	bool get_overbegin() const {return overbegin;}
	void set_overbegin(bool b) {overbegin = b;}

	bool get_overend() const {return overend;}
	void set_overend(bool b) {overend = b;}

	int		get_quality() const {return quality;}
	void	set_quality(int i)	{quality = i;}

	const etl::handle<synfig::Canvas>& get_canvas() const;
	const etl::loose_handle<CanvasView>& get_canvasview() const;

	void set_canvasview(const etl::loose_handle<CanvasView> &h);

	//signal interface
	sigc::signal<void, Preview *> &	signal_destroyed() { return signal_destroyed_; }
	//sigc::signal<void, const synfig::RendDesc &>	&signal_desc_change() {return signal_desc_change_;}

	//functions for exposing iterators through the preview
	FlipBook::iterator	begin() 	{return frames.begin();}
	FlipBook::iterator	end() 		{return frames.end();}

	FlipBook::const_iterator	begin() const {return frames.begin();}
	FlipBook::const_iterator	end() const	  {return frames.end();}
	void push_back(FlipbookElem fe) { frames.push_back(fe); }
	// Used to clear the FlipBook. Do not use directly the std::vector<>::clear member
	// because the cairo_surface_t* wouldn't be destroyed.
	void clear();
	
	unsigned int				numframes() const  {return frames.size();}

	void render();

	sigc::signal0<void>	&signal_changed() { return sig_changed; }
};

class Widget_Preview : public Gtk::Table
{
	Gtk::DrawingArea	draw_area;
	Glib::RefPtr<Gtk::Adjustment> adj_time_scrub; //the adjustment for the managed scrollbar
	Gtk::HScale		scr_time_scrub;
	Gtk::ToggleButton	b_loop;
	Gtk::ScrolledWindow	preview_window;
	//Glib::RefPtr<Gdk::GC>		gc_area;
	Glib::RefPtr<Gdk::Pixbuf>	currentbuf;
	int					currentindex;
	//double			timeupdate;
	double				timedisp;
	double				audiotime;

	//preview encapsulation
	etl::handle<Preview>	preview;
	sigc::connection	prevchanged;

	Gtk::ToggleButton *jackbutton;
	Widget_Time *offset_widget;
	Glib::RefPtr<Gtk::Adjustment> adj_sound;

	Gtk::Label		l_lasttime;
	Gtk::Label		l_currenttime;

	//only for internal stuff, doesn't set anything
	bool 	playing;
	bool	singleframe;
	bool	toolbarisshown;

	//for accurate time tracking
	etl::clock	timer;

	//int		curindex; //for later
	sigc::connection	timecon;

	synfig::SoundProcessor soundProcessor;

	void slider_move(); //later to be a time_slider that's cooler
	bool play_update();

	//bool play_frameupdate();
	void update();

	void scrub_updated(double t);

	void repreview();

	void whenupdated();

	void eraseall();

	bool scroll_move_event(GdkEvent *);
	void disconnect_preview(Preview *);

	bool redraw(const Cairo::RefPtr<Cairo::Context> &cr);
	void preview_draw();

	void hide_toolbar();
	void show_toolbar();

public:

	Widget_Preview();
	~Widget_Preview();

	//sets a signal to identify disconnection (so we don't hold onto it)...
	void set_preview(etl::handle<Preview> prev);

	void clear();

	void play();
	void pause();
	void seek(const synfig::Time &t);
	synfig::Time get_position() const;
	synfig::Time get_time_start() const;
	synfig::Time get_time_end() const;


	void on_play_pause_pressed();

	void seek_frame(int frames);

	void stoprender();

	bool get_loop_flag() const {return b_loop.get_active();}
	void set_loop_flag(bool b) {return b_loop.set_active(b);}

	void on_dialog_show();
	void on_dialog_hide();

protected:

	class ModelColumns : public Gtk::TreeModel::ColumnRecord
	{
	public:

		ModelColumns()
		{ 
			add(factor_id); 
			add(factor_value);
		}

		Gtk::TreeModelColumn<Glib::ustring> factor_id;
		Gtk::TreeModelColumn<Glib::ustring> factor_value;

	};

	ModelColumns factors;

	Gtk::ComboBoxText zoom_preview;
	Glib::RefPtr<Gtk::ListStore> factor_refTreeModel;
	
private:

	Gtk::HBox *toolbar;
	Gtk::Button *play_button;
	Gtk::Button *pause_button;
	bool on_key_pressed(GdkEventKey*);
	void on_zoom_entry_activated();

	bool is_time_equal_to_current_frame(const synfig::Time &time);

	JackDial *jackdial;
	bool jack_enabled;
	bool jack_is_playing;
	synfig::Time jack_time;
	synfig::Time jack_offset;
	synfig::Time jack_initial_time;

	bool get_jack_enabled() { return jack_enabled; }
	void set_jack_enabled(bool value);

#ifdef WITH_JACK
	void toggle_jack_button();
	void on_jack_offset_changed();
	Glib::Dispatcher jack_dispatcher;
	jack_client_t *jack_client;
	bool jack_synchronizing;
	void on_jack_sync();
	static int jack_sync_callback(jack_transport_state_t state, jack_position_t *pos, void *arg);
#endif
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
