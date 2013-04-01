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
#include <gui/canvasview.h>
#include <gtkmm/tooltip.h>
#include <gtkmm/alignment.h>
#include <gtkmm/comboboxentrytext.h>

#include <synfig/time.h>
#include <synfig/vector.h>
#include <synfig/general.h>
#include <synfig/renddesc.h>
#include <synfig/canvas.h>

#include "widgets/widget_sound.h"

#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class AsyncRenderer;

class Preview : public sigc::trackable, public etl::shared_object
{
public:
	class FlipbookElem
	{
	public:
		float t;
		Glib::RefPtr<Gdk::Pixbuf> buf; //at whatever resolution they are rendered at (resized at run time)
		cairo_surface_t* surface;
		FlipbookElem()
		{
			surface=NULL;
		}
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

	FlipBook			frames;

	studio::CanvasView::LooseHandle	canvasview;

	//synfig::RendDesc		description; //for rendering the preview...
	float	zoom,fps;
	float	begintime,endtime;
	bool 	overbegin,overend;
	bool	use_cairo;
	int		quality;

	float	global_fps;

	//expose the frame information etc.
	class Preview_Target;
	class Preview_Target_Cairo;
	void frame_finish(const Preview_Target *);

	sigc::signal0<void>	sig_changed;

public:

	Preview(const studio::CanvasView::LooseHandle &h = studio::CanvasView::LooseHandle(),
				float zoom = 0.5f, float fps = 15);
	~Preview();

	float 	get_zoom() const {return zoom;}
	void	set_zoom(float z){zoom = z;}

	float 	get_fps() const {return fps;}
	void	set_fps(float f){fps = f;}

	float 	get_global_fps() const {return global_fps;}
	void	set_global_fps(float f){global_fps = f;}

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

	bool get_use_cairo() const {return use_cairo;}
	void set_use_cairo(bool b) {use_cairo = b;}

	int		get_quality() const {return quality;}
	void	set_quality(int i)	{quality = i;}

	synfig::Canvas::Handle	get_canvas() const {return canvasview->get_canvas();}
	studio::CanvasView::Handle	get_canvasview() const {return canvasview;}

	void set_canvasview(const studio::CanvasView::LooseHandle &h);

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
	Gtk::Adjustment 	adj_time_scrub; //the adjustment for the managed scrollbar
	Gtk::HScale		scr_time_scrub;
	Gtk::ToggleButton	b_loop;
	Gtk::ScrolledWindow	preview_window;
	//Glib::RefPtr<Gdk::GC>		gc_area;
	Glib::RefPtr<Gdk::Pixbuf>	currentbuf;
	cairo_surface_t* current_surface;
	int				currentindex;
	//double			timeupdate;
	double				timedisp;
	double				audiotime;

	//sound stuff
	etl::handle<AudioContainer>	audio;
	sigc::connection	scrstartcon;
	sigc::connection	scrstopcon;
	sigc::connection	scrubcon;

	//preview encapsulation
	etl::handle<Preview>	preview;
	sigc::connection	prevchanged;

	Widget_Sound		disp_sound;
	Gtk::Adjustment		adj_sound;

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

	void slider_move(); //later to be a time_slider that's cooler
	bool play_update();
	void play_pause();
	//bool play_frameupdate();
	void update();

	void scrub_updated(double t);

	void repreview();

	void whenupdated();

	void eraseall();

	bool scroll_move_event(GdkEvent *);
	void disconnect_preview(Preview *);

	bool redraw(GdkEventExpose *heh = 0);
	void preview_draw();

	void hide_toolbar();
	void show_toolbar();

	sigc::signal<void,float>	signal_play_;
	sigc::signal<void>		signal_pause_;
	sigc::signal<void,float>	signal_seek_;

public:

	Widget_Preview();
	~Widget_Preview();

	//sets a signal to identify disconnection (so we don't hold onto it)...
	void set_preview(etl::handle<Preview> prev);
	void set_audioprofile(etl::handle<AudioProfile> p);
	void set_audio(etl::handle<AudioContainer> a);

	void clear();

	void play();
	void pause();
	void seek(float t);

	void on_play_pause_pressed();

	void seek_frame(int frames);

	void stoprender();

	sigc::signal<void,float>	&signal_play() {return signal_play_;}
	sigc::signal<void>	&signal_pause() {return signal_pause_;}
	sigc::signal<void,float>	&signal_seek() {return signal_seek_;}

	bool get_loop_flag() const {return b_loop.get_active();}
	void set_loop_flag(bool b) {return b_loop.set_active(b);}

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

	Gtk::ComboBoxEntry zoom_preview; 
	Glib::RefPtr<Gtk::ListStore> factor_refTreeModel;
	

private:

	Gtk::HBox *toolbar;
	Gtk::Button *play_pausebutton;
	bool on_key_pressed(GdkEventKey*);
	void on_zoom_entry_activated();
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
