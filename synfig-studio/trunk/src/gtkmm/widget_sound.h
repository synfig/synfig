/* === S Y N F I G ========================================================= */
/*!	\file widget_sound.h
**	\brief Widget Sound Header
**
**	$Id: widget_sound.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_WIDGET_SOUND_H
#define __SYNFIG_WIDGET_SOUND_H

/* === H E A D E R S ======================================================= */
#include <ETL/handle>

#include <gtkmm/drawingarea.h>

#include "widget_timeslider.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
	
class AudioProfile;
class AudioContainer;

/*	What can widget sound do?
	Options:
	1. Just draw the sound
	2. Scroll time and draw the sound
	3. Play, stop, and scrub the sound... (full interaction...)
	4. Provide hooks for scrubbing to work... (and possibly play and stop in the future)

	Going with 4 for now...
*/
class Widget_Sound : public Widget_Timeslider
{
	etl::handle<AudioProfile>	audioprof;
	
	//event override interface
	virtual bool on_expose_event(GdkEventExpose *heh = 0);
	
	//for scrubbing... (click is start, drag is scrub, and release is stop...)
	virtual bool on_motion_notify_event(GdkEventMotion* event);
	virtual bool on_button_press_event(GdkEventButton *event);
	virtual bool on_button_release_event(GdkEventButton *event);
	
	//Might want a signal setup for scrubbing... and here it is
	sigc::signal1<void,double>	signal_start_scrubbing_;
	sigc::signal1<void,double>	signal_scrub_;
	sigc::signal0<void>			signal_stop_scrubbing_;
	
public: //structors
	Widget_Sound();
	~Widget_Sound();

public: //accessors
	bool set_profile(etl::handle<AudioProfile>	p);
	etl::handle<AudioProfile>	get_profile() const;

	//for signal interface
	sigc::signal1<void,double>	&	signal_start_scrubbing()	{return signal_start_scrubbing_;}
	sigc::signal1<void,double>	&	signal_scrub()				{return signal_scrub_;}
	sigc::signal0<void>			&	signal_stop_scrubbing()		{return signal_stop_scrubbing_;}

public: //get set interface
	void set_position(double t);
	double get_position() const;

public: //interface
	void draw();
	
	void clear();
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
