/* === S Y N F I G ========================================================= */
/*!	\file preview.cpp
**	\brief Preview implementation file
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2011 Nikita Kitaev
**	Copyright (c) 2012 Yu Chen
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

#include "preview.h"
#include "app.h"
#include "audiocontainer.h"
#include <gtkmm/stock.h>
#include <gtkmm/separator.h>
#include <gdkmm/general.h>

#include <synfig/target_scanline.h>
#include <synfig/target_cairo.h>
#include <synfig/surface.h>

#include <algorithm>
#include "asyncrenderer.h"
#include "canvasview.h"

#include <cmath>
#include <cassert>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <synfig/string.h>
#include <gui/helpers.h>

#include <gui/localization.h>
#include <cairomm-1.0/cairomm/context.h>
#include <cairomm-1.0/cairomm/enums.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

#define tolower ::tolower

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

class studio::Preview::Preview_Target : public Target_Scanline
{
	Surface	surface;

	sigc::signal<void, const Preview_Target *>		signal_frame_done_;

	int scanline;

	double	tbegin,tend;

	int		nframes,curframe;

public:

	Preview_Target()
	{
		set_alpha_mode(TARGET_ALPHA_MODE_FILL);
		tbegin   = tend     = 0;
		scanline            = 0;
		nframes  = curframe = 0;
	}

	const RendDesc &get_rend_desc() const { return desc; }

	virtual bool set_rend_desc(RendDesc *r)
	{
		if(Target_Scanline::set_rend_desc(r))
		{
			/*synfig::warning("Succeeded in setting the desc to new one: %d x %d, %.2f fps [%.2f,%.2f]",
							desc.get_w(),desc.get_h(),desc.get_frame_rate(),
					(float)desc.get_time_start(),(float)desc.get_time_end());*/

			surface.set_wh(desc.get_w(), desc.get_h());

			curframe = 0;
			nframes  = (int)floor((desc.get_time_end() - desc.get_time_start()) * desc.get_frame_rate());

			tbegin   = desc.get_time_start();
			tend     = tbegin + nframes / desc.get_frame_rate();

			return true;
		}
		return false;
	}

	virtual bool start_frame(ProgressCallback */*cb*/=NULL)
	{
		return true;
	}

	virtual void end_frame()
	{
		//ok... notify our subscribers...
		signal_frame_done_(this);
		curframe += 1;
		//synfig::warning("Finished the frame stuff, and changed time to %.3f",t);
	}

	virtual Color * start_scanline(int scanline)
	{
		return surface[scanline];
	}

	virtual bool end_scanline() {return true;}

	sigc::signal<void, const Preview_Target *>	&signal_frame_done() {return signal_frame_done_;}

	const Surface &get_surface() const {return surface;}

	float get_time() const
	{
		double time = ((nframes-curframe) / (double)nframes) * tbegin
		            + ((curframe)         / (double)nframes) * tend;
		return time;
	}
};

studio::Preview::Preview(const etl::loose_handle<CanvasView> &h, float zoom, float f):
	canvasview(h),
	zoom(zoom),
	fps(f),
	begintime(),
	endtime(),
	jack_offset(),
	overbegin(false),
	overend(false),
	quality(),
	global_fps()
{ }

void studio::Preview::set_canvasview(const etl::loose_handle<CanvasView> &h)
{
	canvasview = h;

	if(canvasview)
	{
		//perhaps reset override values...
		const RendDesc &r = canvasview->get_canvas()->rend_desc();
		if(r.get_frame_rate())
		{
			float rate = 1/r.get_frame_rate();
			overbegin = false; begintime = r.get_time_start() + r.get_frame_start() * rate;
			overend   = false; endtime   = r.get_time_start() + r.get_frame_end()   * rate;
		}
	}
}

studio::Preview::~Preview()
{
	signal_destroyed_(this); //tell anything that attached to us, we're dying
}

void studio::Preview::render()
{
	if(canvasview)
	{

		//render description
		RendDesc desc = get_canvas()->rend_desc();

		//set the global fps of the preview
		set_global_fps(desc.get_frame_rate());

		desc.clear_flags();

		int   neww = (int)floor(desc.get_w() * zoom + 0.5),
		      newh = (int)floor(desc.get_h() * zoom + 0.5);
		float newfps = fps;

		/*synfig::warning("Setting the render description: %d x %d, %f fps, [%f,%f]",
						neww,newh,newfps, overbegin?begintime:(float)desc.get_time_start(),
						overend?endtime:(float)desc.get_time_end());*/
		desc.set_w(neww);
		desc.set_h(newh);
		desc.set_frame_rate(newfps);
		desc.set_render_excluded_contexts(false);
		desc.set_bg_color(App::preview_background_color); //#636

		if(overbegin)
		{
			desc.set_time_start(begintime);
			//synfig::warning("Set start time to %.2f...",(float)desc.get_time_start());
		}
		if(overend)
		{
			desc.set_time_end(endtime);
			//synfig::warning("Set end time to %.2f...",(float)desc.get_time_end());
		}

		//setting the description

		//HACK - add on one extra frame because the renderer can't render the last frame
		// Maybe this can be removed now because the next_time(&t) was refacgorized to consider the last frame too
		//TODO: do not use get_time on Preview_Target
		desc.set_time_end(desc.get_time_end() + 1.000001/fps);

		// Render using a Preview target
		etl::handle<Preview_Target> target = new Preview_Target;
		target->signal_frame_done().connect(sigc::mem_fun(*this, &Preview::frame_finish));

		//set the options
		target->set_canvas(get_canvas());
		target->set_quality(quality);
		// Set the render description
		target->set_rend_desc(&desc);

		//... first we must clear our current selves of space
		frames.resize(0);

		//now tell it to go... with inherited prog. reporting...
		if(renderer) renderer->stop();
		renderer = new AsyncRenderer(target);
		renderer->start();
	}
}

void studio::Preview::clear()
{
	frames.clear();
}

const etl::handle<synfig::Canvas>&
studio::Preview::get_canvas() const
	{return canvasview->get_canvas();}

const etl::loose_handle<CanvasView>&
studio::Preview::get_canvasview() const
	{return canvasview;}

static void free_guint8(const guint8 *mem)
{
	free((void*)mem);
}

void studio::Preview::frame_finish(const Preview_Target *targ)
{
	//copy image with time to next frame (can just push back)
	FlipbookElem	fe;
	float           time = targ->get_time();
	const Surface&  surf = targ->get_surface();
	const RendDesc& r    = targ->get_rend_desc();

	//synfig::warning("Finished a frame at %f s",time);

	//copy EVERYTHING!
	PixelFormat pf(PF_RGB);

	const int total_bytes(r.get_w() * r.get_h() * synfig::pixel_size(pf));

	//synfig::warning("Creating a buffer");
	unsigned char *buffer((unsigned char*)malloc(total_bytes));

	if(!buffer)
		return;

	//convert all the pixels to the pixbuf... buffer... thing...
	//synfig::warning("Converting...");
	color_to_pixelformat(buffer, surf[0], pf, &App::gamma, surf.get_w(), surf.get_h());

	//load time
	fe.t = time;
	//uses and manages the memory for the buffer...
	//synfig::warning("Create a pixmap...");
	fe.buf =
	Gdk::Pixbuf::create_from_data(
		buffer,	                               // pointer to the data
		Gdk::COLORSPACE_RGB,                   // the colorspace
		((pf & PF_A) == PF_A),                 // has alpha?
		8,                                     // bits per sample
		surf.get_w(),                          // width
		surf.get_h(),                          // height
		surf.get_w() * synfig::pixel_size(pf), // stride (pitch)
		sigc::ptr_fun(free_guint8)
	);

	//add the flipbook element to the list (assume time is correct)
	//synfig::info("Prev: Adding %f s to the list", time);
	frames.push_back(fe);

	signal_changed()();
}

#define IMAGIFY_BUTTON(button,stockid,tooltip) \
        icon = manage(new Gtk::Image(Gtk::StockID(stockid), Gtk::ICON_SIZE_BUTTON)); \
	button->set_tooltip_text(tooltip); \
        button->add(*icon); \
        button->set_relief(Gtk::RELIEF_NONE); \
        button->show(); \
	icon->set_padding(0,0); \
	icon->show();

Widget_Preview::Widget_Preview():
	Gtk::Table(1, 5),
	adj_time_scrub(Gtk::Adjustment::create(0, 0, 1000, 0, 10, 0)),
	scr_time_scrub(adj_time_scrub),
	b_loop(/*_("Loop")*/),
	currentindex(-100000),//TODO get the value from canvas setting or preview option
	timedisp(-1),
	audiotime(0),
	jackbutton(),
	offset_widget(),
	adj_sound(Gtk::Adjustment::create(0, 0, 4)),
	l_lasttime("0s"),
	playing(false),
	singleframe(),
	toolbarisshown(),
	zoom_preview(true),
	toolbar(),
	play_button(),
	pause_button(),
	jackdial(NULL),
	jack_enabled(false),
	jack_is_playing(false),
	jack_time(0),
	jack_offset(0),
	jack_initial_time(0)
#ifdef WITH_JACK
	,
	jack_client(NULL),
	jack_synchronizing(false)
#endif
{
	//catch key press event for shortcut keys
	signal_key_press_event().connect(sigc::mem_fun(*this, &Widget_Preview::on_key_pressed));

	//connect to expose events
	//signal_expose_event().connect(sigc::mem_fun(*this, &studio::Widget_Preview::redraw));

	//manage all the change in values etc...

	//1st row: preview content
	preview_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	//pack preview content into scrolled window
	preview_window.add(draw_area);

	//preview window background color - Not working anymore after version 3.16!
	//https://developer.gnome.org/gtk3/unstable/GtkWidget.html#gtk-widget-override-background-color
	Gdk::RGBA bg_color;
	bg_color.set_red(54*256);
	bg_color.set_blue(59*256);
	bg_color.set_green(59*256);
	draw_area.override_background_color(bg_color);

	adj_time_scrub->signal_value_changed().connect(sigc::mem_fun(*this, &Widget_Preview::slider_move));
	scr_time_scrub.signal_event().connect(sigc::mem_fun(*this, &Widget_Preview::scroll_move_event));
	draw_area.signal_draw().connect(sigc::mem_fun(*this, &Widget_Preview::redraw));

	scr_time_scrub.set_draw_value(0);

	Gtk::Button *button = 0;
	Gtk::Image  *icon   = 0;

	#if 1

	//2nd row: prevframe play/pause nextframe loop | halt-render re-preview erase-all
	toolbar = Gtk::manage(new class Gtk::HBox(false, 0));

	//prev rendered frame
	Gtk::Button *prev_framebutton;
	Gtk::Image *icon0 = manage(new Gtk::Image(Gtk::StockID("synfig-animate_seek_prev_frame"), Gtk::ICON_SIZE_BUTTON));
	prev_framebutton = manage(new class Gtk::Button());
	prev_framebutton->set_tooltip_text(_("Prev frame"));
	icon0->set_padding(0,0);
	icon0->show();
	prev_framebutton->add(*icon0);
	prev_framebutton->set_relief(Gtk::RELIEF_NONE);
	prev_framebutton->show();
	prev_framebutton->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &Widget_Preview::seek_frame), -1));

	toolbar->pack_start(*prev_framebutton, Gtk::PACK_SHRINK, 0);

	{ //play
		Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID("synfig-animate_play"), Gtk::ICON_SIZE_BUTTON));
		play_button = manage(new class Gtk::Button());
		play_button->set_tooltip_text(_("Play"));
		icon->set_padding(0,0);
		icon->show();
		play_button->add(*icon);
		play_button->set_relief(Gtk::RELIEF_NONE);
		play_button->show();
		play_button->signal_clicked().connect(sigc::mem_fun(*this, &Widget_Preview::on_play_pause_pressed));
		toolbar->pack_start(*play_button, Gtk::PACK_SHRINK, 0);
	}

	{ //pause
		Gtk::Image *icon = manage(new Gtk::Image(Gtk::StockID("synfig-animate_pause"), Gtk::ICON_SIZE_BUTTON));
		pause_button = manage(new class Gtk::Button());
		pause_button->set_tooltip_text(_("Pause"));
		icon->set_padding(0,0);
		icon->show();
		pause_button->add(*icon);
		pause_button->set_relief(Gtk::RELIEF_NONE);
		pause_button->signal_clicked().connect(sigc::mem_fun(*this, &Widget_Preview::on_play_pause_pressed));
		toolbar->pack_start(*pause_button, Gtk::PACK_SHRINK, 0);
	}


	//next rendered frame
	Gtk::Button *next_framebutton;
	Gtk::Image *icon2 = manage(new Gtk::Image(Gtk::StockID("synfig-animate_seek_next_frame"), Gtk::ICON_SIZE_BUTTON));
	next_framebutton = manage(new class Gtk::Button());
	next_framebutton->set_tooltip_text(_("Next frame"));
	icon2->set_padding(0,0);
	icon2->show();
	next_framebutton->add(*icon2);
	next_framebutton->set_relief(Gtk::RELIEF_NONE);
	next_framebutton->show();
	next_framebutton->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &Widget_Preview::seek_frame), 1));

	toolbar->pack_start(*next_framebutton, Gtk::PACK_SHRINK, 0);

	//spacing
	Gtk::Alignment *space = Gtk::manage(new Gtk::Alignment());
	space->set_size_request(8);
	toolbar->pack_start(*space, false, true);


	//loop
	button = &b_loop;
	IMAGIFY_BUTTON(button,"synfig-animate_loop", _("Loop"));
	toolbar->pack_start(b_loop, Gtk::PACK_SHRINK,0);

	//spacing
	Gtk::Alignment *space1 = Gtk::manage(new Gtk::Alignment());
	space1->set_size_request(24);
	toolbar->pack_start(*space1, false, true);


	//halt render
	button = manage(new Gtk::Button(/*_("Halt Render")*/));
	button->signal_clicked().connect(sigc::mem_fun(*this, &Widget_Preview::stoprender));
	IMAGIFY_BUTTON(button,Gtk::Stock::STOP, _("Halt render"));

	toolbar->pack_start(*button, Gtk::PACK_SHRINK, 0);

	//re-preview
	button = manage(new Gtk::Button(/*_("Re-Preview")*/));
	button->signal_clicked().connect(sigc::mem_fun(*this, &Widget_Preview::repreview));
	IMAGIFY_BUTTON(button, Gtk::Stock::EDIT, _("Re-preview"));

	toolbar->pack_start(*button, Gtk::PACK_SHRINK, 0);

	//erase all
	button = manage(new Gtk::Button(/*_("Erase All")*/));
	button->signal_clicked().connect(sigc::mem_fun(*this, &Widget_Preview::eraseall));
	IMAGIFY_BUTTON(button, Gtk::Stock::CLEAR, _("Erase all rendered frame(s)"));

	toolbar->pack_start(*button, Gtk::PACK_SHRINK, 0);

	//spacing
	Gtk::Alignment *space2 = Gtk::manage(new Gtk::Alignment());
	space1->set_size_request(24);
	toolbar->pack_start(*space2, false, true);

	//jack
	jackdial = Gtk::manage(new JackDial());
#ifdef WITH_JACK
	jack_dispatcher.connect(sigc::mem_fun(*this, &Widget_Preview::on_jack_sync));
	jack_dispatcher.connect(sigc::mem_fun(*this, &Widget_Preview::on_jack_sync));

	jackbutton = jackdial->get_toggle_jackbutton();
	jackdial->signal_toggle_jack().connect(sigc::mem_fun(*this, &studio::Widget_Preview::toggle_jack_button));
	jackdial->signal_offset_changed().connect(sigc::mem_fun(*this, &studio::Widget_Preview::on_jack_offset_changed));
#endif
	//FIXME: Hardcoded FPS!
	jackdial->set_fps(24.f);
	jackdial->set_offset(jack_offset);
	if ( !getenv("SYNFIG_DISABLE_JACK") )
		jackdial->show();
	toolbar->pack_start(*jackdial, false, true);

	//zoom preview
	factor_refTreeModel = Gtk::ListStore::create(factors);
	zoom_preview.set_model(factor_refTreeModel);
	zoom_preview.property_has_frame() = true;
	zoom_preview.signal_changed().connect(sigc::mem_fun(*this, &Widget_Preview::preview_draw));

	Gtk::TreeModel::Row row = *(factor_refTreeModel->append());
	row[factors.factor_id] = "1";
	row[factors.factor_value] = "25%";

	row = *(factor_refTreeModel->append());
	row[factors.factor_id] = "2";
	row[factors.factor_value] = "50%";

	row = *(factor_refTreeModel->append());
	row[factors.factor_id] = "3";
	row[factors.factor_value] = "100%";

	row = *(factor_refTreeModel->append());
	row[factors.factor_id]  = "4";
	row[factors.factor_value] = "200%";

	row = *(factor_refTreeModel->append());
	row[factors.factor_id] = "5";
	row[factors.factor_value] = _("Fit");
	zoom_preview.set_entry_text_column(factors.factor_value);

	Gtk::Entry* entry = zoom_preview.get_entry();
	entry->set_text("100%"); //default zoom level
	entry->set_icon_from_stock(Gtk::StockID("synfig-zoom"));
	entry->signal_activate().connect(sigc::mem_fun(*this, &Widget_Preview::on_zoom_entry_activated));

	//set the zoom widget width
	zoom_preview.set_size_request(100, -1);
	zoom_preview.show();

	toolbar->pack_end(zoom_preview, Gtk::PACK_SHRINK, 0);

	show_toolbar();

	//3rd row: previewing frame numbering and rendered frame numbering
	Gtk::HBox *status = manage(new Gtk::HBox);
	status->pack_start(l_currenttime, Gtk::PACK_SHRINK, 5);
	Gtk::Label *separator = manage(new Gtk::Label(" / "));
	status->pack_start(*separator, Gtk::PACK_SHRINK, 0);
	status->pack_start(l_lasttime, Gtk::PACK_SHRINK, 5);

	status->show_all();

	// attach all widgets
	attach(preview_window, 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0);
	attach(scr_time_scrub, 0, 1, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	attach(*toolbar,       0, 1, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL);
	attach(*status,        0, 1, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	preview_window.show_all();
	scr_time_scrub.show_all();

	//if(draw_area.get_window()) gc_area = Gdk::GC::create(draw_area.get_window());
	#endif
}

studio::Widget_Preview::~Widget_Preview()
{
	clear();
}

void studio::Widget_Preview::update()
{
	//the meat goes in this locker...
	double time = adj_time_scrub->get_value();

	//find the frame and display it...
	if(preview)
	{
		#ifdef WITH_JACK
		if (jack_enabled && !jack_synchronizing && !is_time_equal_to_current_frame(jack_time - jack_offset))
		{
			jack_nframes_t sr = jack_get_sample_rate(jack_client);
			jack_nframes_t nframes = ((double)sr * (time + jack_offset));
			jack_transport_locate(jack_client, nframes);
		}
		#endif

		if (fabs(soundProcessor.get_position() - time) > 0.5)
			soundProcessor.set_position(time);

		//synfig::warning("Updating at %.3f s",time);

		//use time to find closest frame...
		studio::Preview::FlipBook::const_iterator 	beg = preview->begin(),end = preview->end();
		studio::Preview::FlipBook::const_iterator 	i;

		i = beg;

		//go to current hint if need be...
		if(currentindex >= 0 && currentindex < (int)preview->numframes())
		{
			i = beg+currentindex;
		}

		//we can't have a picture if there are none to get
		if(beg != end)
		{
			//don't bother with binary search it will just be slower...

			//synfig::info("Search for time %f",time);

			//incrementally go in either direction
			//(bias downward towards beg, because that's what we want)
			for(;i != end;++i)
			{
				//synfig::info("Look at %f",i->t);
				if(i->t > time) break;
				//synfig::info("Go past...");
			}

			//if(i!=beg)--i;

			//bias down, so we can't be at end... and it still is valid...
			for(;i != beg;)
			{
				--i;
				//synfig::info("Look at %f",i->t);
				if(i->t <= time) break;
				//synfig::info("Go past...");
			}

			/*i = preview->begin(); end = preview->end();
			if(i == end) return;

			j = i;
			for(;i != end; j = i++)
			{
				if(i->t > time) break;
			}*/

			//we should be at a valid edge since we biased downward

			//don't get the closest, round down... (if we can)
			if(i == end)
			{
				synfig::error("i == end....");
				//assert(0);
				currentbuf.clear();
				currentindex = 0;
				timedisp = -1;
			}else
			{
				currentbuf = i->buf;
				currentindex = i-beg;
				if(timedisp != i->t)
				{
					timedisp = i->t;
					//synfig::warning("Update at: %f seconds (%f s)",time,timedisp);
					preview_draw();
					//synfig::warning("success!");
				}
			}
		}
	}

	//current frame in previewing
	Glib::ustring timecode(Time((double)timedisp).round(preview->get_global_fps())
                .get_string(preview->get_global_fps(), App::get_time_format()));
	l_currenttime.set_text(timecode);

}
void studio::Widget_Preview::preview_draw()
{
	draw_area.queue_draw();//on_expose_event();
}

bool studio::Widget_Preview::redraw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	//And render the drawing area
	Glib::RefPtr<Gdk::Pixbuf> pxnew, px = currentbuf;

	int dw = draw_area.get_width();
	int dh = draw_area.get_height();

	if(!px)
		return true;
	//made not need this line
	//if ( draw_area.get_height() == 0 || px->get_height() == 0 || px->get_width() == 0)
	//	return true;

	//figure out the scaling factors...
	float sx, sy;
	float q = 1 / preview->get_zoom();
	int nw, nh;
	int w,h;

	// grab the source dimensions
	w = px->get_width();
	h = px->get_height();

	Gtk::Entry* entry = zoom_preview.get_entry();
	String str(entry->get_text());
	Glib::ustring text = str;
	const char *c = text.c_str();

	if (text == _("Fit") || text == "fit")
	{
		sx = dw / (float)w;
		sy = dh / (float)h;

		//synfig::info("widget_preview redraw: now to scale the bitmap: %.3f x %.3f",sx,sy);

		//round to smallest scale (fit entire thing in window without distortion)
		if(sx > sy) sx = sy;

		//cleanup previous size request
		draw_area.set_size_request();
	}

	//limit zoom level from 0.01 to 10 times
	else if (atof(c) > 1000)
	{
		sx = sy = 10 * q;
	}

	else if (atof(c) <= 0 )
	{
		sx = sy = 0 ;
		draw_area.set_size_request(0, 0);
	}

	else sx = sy = atof(c) / 100 * q;

	//scale to a new pixmap and then copy over to the window
	nw = (int)(w * sx);
	nh = (int)(h * sx);

	if(nw == 0 || nh == 0)return true;

	pxnew = px->scale_simple(nw, nh, Gdk::INTERP_NEAREST);

	//except "Fit" or "fit", we need to set size request for scrolled window
	if (text != _("Fit") && text != "fit")
	{
		draw_area.set_size_request(nw, nh);
		dw = draw_area.get_width();
		dh = draw_area.get_height();
	}

	//synfig::info("Now to draw to the window...");
	//copy to window
	Glib::RefPtr<Gdk::Window> wind = draw_area.get_window();
	if(!wind) synfig::warning("The destination window is broken...");

	/* Options for drawing...
		1) store with alpha, then clear and render with alpha every frame
			- more time consuming
			+ more expandable
		2) store with just pixel info
			- less expandable
			+ faster
			+ better memory footprint
	*/
	//px->composite(const Glib::RefPtr<Gdk::Pixbuf>& dest, int dest_x, int dest_y, int dest_width, int dest_height, double offset_x, double offset_y, double scale_x, double scale_y, InterpType interp_type, int overall_alpha) const

	cr->save();
	Gdk::Cairo::set_source_pixbuf(
		cr,    //cairo context
		pxnew, //pixbuf
		//coordinates to place center of the preview window
		(dw - nw) / 2, (dh - nh) / 2
		);
	cr->paint();
	cr->restore();

	//synfig::warning("Refresh the draw area");
	//make sure the widget refreshes

	return false;
}

bool studio::Widget_Preview::play_update()
{
	float diff = timer.pop_time();
	//synfig::info("Play update: diff = %.2f",diff);

	if(playing)
	{
		//we go to the next one...
		double time = adj_time_scrub->get_value() + diff;

		if (jack_enabled)
		{
#ifdef WITH_JACK
			jack_position_t pos;
			jack_transport_state_t state = jack_transport_query(jack_client, &pos);
			if (state != JackTransportRolling && state != JackTransportStarting)
				{ on_jack_sync(); return true; }
			jack_time = Time((Time::value_type)pos.frame/(Time::value_type)pos.frame_rate);
			time = jack_time - jack_offset;
#endif
		}
		else
		{
			//time = soundProcessor.get_position();
		}

		if (fabs(soundProcessor.get_position() - time) > 0.5)
			soundProcessor.set_position(time);

		//Looping conditions...
		if(time >= adj_time_scrub->get_upper())
		{
			if(get_loop_flag())
			{
				time = adj_time_scrub->get_lower();// + time-adj_time_scrub.get_upper();
				currentindex = 0;
			}else
			{
				time = adj_time_scrub->get_upper();
				adj_time_scrub->set_value(time);
				pause();
				update();

				//synfig::info("Play Stopped: time set to %f",adj_time_scrub.get_value());
				return false;
			}
		}

		//set the new time...
		ConfigureAdjustment(adj_time_scrub)
			.set_value(time)
			.finish();

		//update the window to the correct image we might want to do this later...
		//update();
		//synfig::warning("Did update pu");
	}
	return true;
}

void studio::Widget_Preview::slider_move()
{
	//if(!playing)
	{
		update();
		//synfig::warning("Did update sm");
	}
}

//for other things updating the value changed signal...
void studio::Widget_Preview::scrub_updated(double t)
{
	if (playing) on_play_pause_pressed();

	//synfig::info("Scrubbing to %.3f, setting adj to %.3f",oldt,t);

	ConfigureAdjustment(adj_time_scrub)
		.set_value(t)
		.finish();
}

void studio::Widget_Preview::disconnect_preview(Preview *prev)
{
	if(prev == preview)
	{
		preview = 0;
		//prevchanged.disconnect();
		soundProcessor.clear();
	}
}

void studio::Widget_Preview::set_preview(etl::handle<Preview>	prev)
{
	disconnect_preview(preview.get());

	preview = prev;

	synfig::info("Setting preview");

	//stop playing the mini animation...
	pause();

	if (preview) {
		//set the internal values
		float rate = preview->get_fps();
		jackdial->set_fps(rate);
		jackdial->set_offset(preview->get_jack_offset());
		synfig::info("	FPS = %f",rate);
		if (rate) {
			float start = preview->get_begintime();
			float end = preview->get_endtime();

			rate = 1/rate;

			adj_time_scrub->configure(start, start, end, rate, 10.0*rate, 0.0);

			//if the begin time and the end time are the same there is only a single frame
			singleframe = end==start;
		} else {
			adj_time_scrub->configure(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
			singleframe = true;
		}
		scr_time_scrub.unset_adjustment();
		scr_time_scrub.hide();
		scr_time_scrub.set_adjustment(adj_time_scrub);
		scr_time_scrub.show();
		scr_time_scrub.show_all();

		preview->get_canvas()->fill_sound_processor(soundProcessor);
		set_jack_enabled( preview && preview->get_canvasview()->get_jack_enabled_in_preview() );

		//connect so future information will be found...
		prevchanged = prev->signal_changed().connect(sigc::mem_fun(*this,&Widget_Preview::whenupdated));
		prev->signal_destroyed().connect(sigc::mem_fun(*this,&Widget_Preview::disconnect_preview));
		update();
		//synfig::warning("Did update sp");
		queue_draw();
	}
}

void studio::Widget_Preview::whenupdated()
{
	l_lasttime.set_text((Time((double)(--preview->end())->t)
							.round(preview->get_global_fps())
							.get_string(preview->get_global_fps(),App::get_time_format())));
	update();
}

void studio::Widget_Preview::clear()
{
	disconnect_preview(preview.get());
	set_jack_enabled(false);
}

void studio::Widget_Preview::play()
{
	if (playing) return;
	if(preview)
	{
		if (!jack_enabled && get_position() == get_time_end()) seek(get_time_start());
		soundProcessor.set_position(get_position());
		soundProcessor.set_playing(true);

		//synfig::info("Playing at %lf",adj_time_scrub->get_value());
		//audiotime = adj_time_scrub->get_value();
		playing = true;

		play_button->hide();
		pause_button->show();

		//adj_time_scrub->set_value(adj_time_scrub->get_lower());
		update(); //we don't want to call play update because that will try to advance the timer

		//approximate length of time in seconds, right?
		double rate = /*std::min(*/adj_time_scrub->get_step_increment()/*,1/30.0)*/;
		int timeout = (int)floor(1000*rate);

		//synfig::info("	rate = %.3lfs = %d ms",rate,timeout);

		//signal_play_(adj_time_scrub->get_value());

		timecon = Glib::signal_timeout().connect(sigc::mem_fun(*this,&Widget_Preview::play_update),timeout);
		timer.reset();
	}
}

void studio::Widget_Preview::pause()
{
	//synfig::warning("stopping");
	timecon.disconnect();
	playing = false;
	pause_button->hide();
	play_button->show();
	soundProcessor.set_playing(false);
}

void studio::Widget_Preview::on_play_pause_pressed()
{
	bool play_flag;
	//! Commented out , build warnings
//	float begin = preview->get_begintime();
//	float end = preview->get_endtime();
//	float current = adj_time_scrub->get_value();
//	Gtk::Image *icon;

	play_flag = !playing;

#ifdef WITH_JACK
	if (jack_enabled)
	{
		if (jack_is_playing) {
			jack_transport_stop(jack_client);
			on_jack_sync();
		} else
			jack_transport_start(jack_client);
		return;
	}
#endif

	if(play_flag) play(); else pause();
}

void studio::Widget_Preview::seek_frame(int frames)
{
//	if(!frames)	return;

	if(playing) on_play_pause_pressed();	//pause playing when seek frame called

	double fps = preview->get_fps();

	double currenttime = adj_time_scrub->get_value();
	int currentframe = (int)floor(currenttime * fps);
	Time newtime(double((currentframe + frames + 0.5) / fps));

	adj_time_scrub->set_value(newtime);
}

bool studio::Widget_Preview::scroll_move_event(GdkEvent *event)
{
	switch(event->type)
	{
		case GDK_BUTTON_PRESS:
		{
			if(event->button.button == 1 || event->button.button == 3)
			{
				pause();
			}
			break;
		}

		default: break;
	}

	return false;
}

synfig::Time studio::Widget_Preview::get_position() const
	{ return adj_time_scrub->get_value(); }
synfig::Time studio::Widget_Preview::get_time_start() const
	{ return adj_time_scrub->get_lower(); }
synfig::Time studio::Widget_Preview::get_time_end() const
	{ return adj_time_scrub->get_upper(); }

void studio::Widget_Preview::seek(const synfig::Time &t)
{
	pause();
	adj_time_scrub->set_value(t);
}

void studio::Widget_Preview::repreview()
{
	if(preview)
	{
		stoprender();
		pause();
		preview->get_canvasview()->preview_option();
	}
}

void studio::Widget_Preview::stoprender()
{
	if(preview)
	{
		// don't crash if the render has already been stopped
		if (!preview->renderer)
			return;

#ifdef SINGLE_THREADED
		if (preview->renderer->updating)
			preview->renderer->pause();
		else
#endif
			preview->renderer.detach();
	}
}

void studio::Widget_Preview::eraseall()
{
	pause();
	stoprender();

	currentbuf.clear();
	currentindex = 0;
	timedisp = 0;
	queue_draw();

	if(preview)
	{
		preview->clear();
	}
}

void Widget_Preview::on_zoom_entry_activated()
{
	Gtk::Entry* entry = zoom_preview.get_entry();
	String str(entry->get_text());
	string digi = "0123456789";
	size_t first = str.find_first_of(digi);

	if (first == string::npos)
	{
		entry->set_text(_("Fit"));

		//release the focus to enable accelerator keys
		preview_window.grab_focus();

		return ;
	}

	size_t last = str.find_first_not_of(digi);

	if (last == string::npos)
	{
		last = str.find_last_of(digi) + 1;
	}

	if (first > last)
	{
		entry->set_text (_("Fit"));
	}

	else entry->set_text(str.substr(first, last - first) + "%");

	//release the focus to enable accelerator keys
	preview_window.grab_focus();
}

void Widget_Preview::hide_toolbar()
{
	toolbar->hide();
	toolbarisshown = 0;

	//release the focus to enable accelerator keys
	preview_window.grab_focus();
}

void Widget_Preview::show_toolbar()
{
	toolbar->show();
	toolbarisshown = 1;
	toolbar->grab_focus();
}

//shortcut keys TODO: customizable shortcut keys would be awesome.
bool studio::Widget_Preview::on_key_pressed(GdkEventKey *ev)
{
	//hide and show toolbar
	if (ev->keyval == gdk_keyval_from_name("h"))
	{
		if (toolbarisshown) hide_toolbar();
		else show_toolbar();
		return true;
	}

	//previous rendered frame
	if (ev->keyval == gdk_keyval_from_name("a"))
	{
		if(playing) pause();
		seek_frame(-1);
		return true;
	}

	//play/pause
	if (ev->keyval == gdk_keyval_from_name("s"))
	{
		on_play_pause_pressed();
		return true;
	}

	//next render frame
	if (ev->keyval == gdk_keyval_from_name("d"))
	{
		if(playing) pause();
		seek_frame(+1);
		return true;
	}

	//loop
	if (ev->keyval == gdk_keyval_from_name("f"))
	{
		if(get_loop_flag()) set_loop_flag(false);
		else set_loop_flag(true);
		return true;
	}

	//zoom level switching
	//zoom to 25%
	Gtk::Entry* entry = zoom_preview.get_entry();
	Glib::ustring text = entry->get_text();

	if (ev->keyval == gdk_keyval_from_name("1"))
	{
		if(entry->get_text() != "25%")
		{
			entry->set_text("25%");
		}
		return true;
	}

	if (ev->keyval == gdk_keyval_from_name("2"))
	{
		if(entry->get_text() != "50%")
		{
			entry->set_text("50%");
		}
		return true;
	}

	if (ev->keyval == gdk_keyval_from_name("3"))
	{
		if(entry->get_text() != "100%")
		{
			entry->set_text("100%");
		}
		return true;
	}

	if (ev->keyval == gdk_keyval_from_name("4"))
	{
		if(entry->get_text() != "200%")
		{
			entry->set_text("200%");
		}
		return true;
	}

	if (ev->keyval == gdk_keyval_from_name("5"))
	{
		if(entry->get_text() != _("Fit"))
		{
			entry->set_text(_("Fit"));
		}
		return true;
	}

	return false;
}

bool
Widget_Preview::is_time_equal_to_current_frame(const synfig::Time &time)
{
	float fps = preview ? preview->get_fps() : 25.f;
	Time starttime = get_time_start();
	Time endtime = get_time_end();

	synfig::Time t0 = get_position();
	synfig::Time t1 = time;

	if (fps != 0.f) {
		t0 = t0.round(fps);
		t1 = t1.round(fps);
	}

	t0 = std::max(starttime, std::min(endtime, t0));
	t1 = std::max(starttime, std::min(endtime, t1));

	return t0.is_equal(t1);
}

void Widget_Preview::on_dialog_show()
{
	set_jack_enabled( preview && preview->get_canvasview()->get_jack_enabled_in_preview() );
}

void Widget_Preview::on_dialog_hide()
{
	if (preview)
	{
		bool enabled = get_jack_enabled();
		set_jack_enabled(false);
		preview->get_canvasview()->set_jack_enabled_in_preview(enabled);
	}
	pause();
	stoprender();
}

void Widget_Preview::set_jack_enabled(bool value) {
	if (jack_enabled == value) return;

#ifdef WITH_JACK
	if (playing) pause();
	jack_enabled = value;
	if (jack_enabled)
	{
		// lock jack in canvas views
		App::jack_lock();

		// initialize jack
		jack_client = jack_client_open("synfigstudiopreview", JackNullOption, 0);
		jack_set_sync_callback(jack_client, jack_sync_callback, this);
		if (jack_activate(jack_client) != 0)
		{
			jack_client_close(jack_client);
			jack_client = NULL;
			jack_enabled = false;
			App::jack_unlock();
		} else {
			// remember time
			on_jack_sync();
			jack_initial_time = jack_time;
		}
	}
	else
	{
		// restore time
		jack_nframes_t sr = jack_get_sample_rate(jack_client);
		jack_nframes_t nframes = ((double)sr * (jack_initial_time));
		jack_transport_locate(jack_client, nframes);

		// deinitialize jack
		jack_deactivate(jack_client);
		jack_client_close(jack_client);
		jack_client = NULL;

		// unlock jack in canvas views
		App::jack_unlock();
	}

	//jackdial->toggle_enable_jack(jack_enabled);

	Gtk::IconSize iconsize=Gtk::IconSize::from_name("synfig-small_icon_16x16");
	Gtk::Image *icon;
	offset_widget = jackdial->get_offsetwidget();

	if (jackbutton->get_active())
	{
		icon = manage(new Gtk::Image(Gtk::StockID("synfig-jack"),iconsize));
		jackbutton->remove();
		jackbutton->add(*icon);
		jackbutton->set_tooltip_text(_("Disable JACK"));
		icon->set_padding(0,0);
		icon->show();

		offset_widget->show();
	}
	else
	{
		icon = manage(new Gtk::Image(Gtk::StockID("synfig-jack"),iconsize));
		jackbutton->remove();
		jackbutton->add(*icon);
		jackbutton->set_tooltip_text(_("Enable JACK"));
		icon->set_padding(0,0);
		icon->show();

		offset_widget->hide();
	}
#endif

	if (preview) preview->get_canvasview()->set_jack_enabled_in_preview( get_jack_enabled() );
}


#ifdef WITH_JACK
void Widget_Preview::toggle_jack_button()
{
	set_jack_enabled(!get_jack_enabled());
}

void Widget_Preview::on_jack_offset_changed() {
	jack_offset = jackdial->get_offset();
	if (get_jack_enabled()) on_jack_sync();
}

void Widget_Preview::on_jack_sync() {
	jack_position_t pos;
	jack_transport_state_t state = jack_transport_query(jack_client, &pos);

	jack_is_playing = state == JackTransportRolling || state == JackTransportStarting;
	jack_time = Time((Time::value_type)pos.frame/(Time::value_type)pos.frame_rate);

	if (playing != jack_is_playing)
	{
		if (jack_is_playing)
			play();
		else
			pause();
	}

	if (!is_time_equal_to_current_frame(jack_time - jack_offset))
	{
		jack_synchronizing = true;
		seek(jack_time - jack_offset);
		jack_synchronizing = false;
	}
}

int Widget_Preview::jack_sync_callback(jack_transport_state_t /* state */, jack_position_t * /* pos */, void *arg) {
	Widget_Preview *widget_preview = static_cast<Widget_Preview*>(arg);
	widget_preview->jack_dispatcher.emit();
	return 1;
}
#endif
