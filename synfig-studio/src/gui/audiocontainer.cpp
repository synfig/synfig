/* === S Y N F I G ========================================================= */
/*!	\file audiocontainer.cpp
**	\brief Audio Container implementation File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "audiocontainer.h"

#include <ETL/clock>

#endif

/* === U S I N G =========================================================== */

using namespace etl;

/* === M A C R O S ========================================================= */
#ifndef __WIN32
#define AUDIO_OUTPUT	FSOUND_OUTPUT_OSS
#endif

/* === G L O B A L S ======================================================= */
//const double delay_factor = 3;
//Warning: Unused variable delay_factor
/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

//Help constructing stuff
struct FSOUND_SAMPLE;
using studio::AudioContainer;

bool build_profile(FSOUND_SAMPLE */*sample*/, double &/*samplerate*/, std::vector<char> &/*samples*/)
	{ return false; } // not implemented


struct scrubinfo {};

//----- AudioProfile Implementation -----------
void studio::AudioProfile::clear()
{
	samplerate = 0;
	samples.clear();
}

handle<AudioContainer>	studio::AudioProfile::get_parent() const
{
	return parent;
}

void studio::AudioProfile::set_parent(std::shared_ptr<AudioContainer> i)
{
	parent = i;
}

double studio::AudioProfile::get_offset() const
{
	if(parent)
		return parent->get_offset();
	return 0;
}

//---------- AudioContainer definitions ---------------------

struct studio::AudioContainer::AudioImp
{
	//Sample load time information
	FSOUND_SAMPLE *		sample;
	int					channel;
	int					sfreq;
	int					length;

	//Time information
	double				offset; //time offset for playing...

	//We don't need it now that we've adopted the play(t) time schedule...
	//current time... and playing info....
	//float				seekpost;
	//bool				useseekval;

	//Make sure to sever our delayed start if we are stopped prematurely
	sigc::connection	delaycon;

	//Action information
	bool				playing;
	double				curscrubpos;

	//Scrubbing information...
	//the current position of the sound will be sufficient for normal stuff...
	scrubinfo			*scrptr;

	bool is_scrubbing() const {return scrptr != 0;}
	void set_scrubbing(bool /*s*/) { scrptr = 0; }

	//helper to make sure we are actually playing (and to get a new channel...)
	bool init_play() { return false; }

public: //structors
	AudioImp():
		sample(0),
		channel(0),
		sfreq(0),
		length(0),
		offset(0),
		playing(false),
		curscrubpos(),
		scrptr(0)
	{ } //reuse the channel...

	~AudioImp()
		{ clear(); }

public: //helper/accessor funcs
	bool start_playing_now() //callback for timer...
		{ return false; } //so the timer doesn't repeat itself
	bool isRunning()
		{ return false; }
	bool isPaused()
		{ return false; }

public: //forward interface

	//Accessors for the offset - in seconds
	const double &get_offset() const
		{return offset;}
	void set_offset(const double &d)
		{ offset = d; }

	//Will override the parameter timevalue if the sound is running, and not if it's not...
	bool get_current_time(double &/*out*/)
		{ return isRunning(); }

	//Big implementation functions...
	bool load(const std::string &filename, const std::string &filedirectory);
	void clear();

	//playing functions
	void play(double t);
	void stop();

	//scrubbing functions
	void start_scrubbing(double t);
	void scrub(double t);
	void stop_scrubbing();

	double scrub_time()
		{ return curscrubpos; }
};

//--------------- Audio Container definitions --------------------------
studio::AudioContainer::AudioContainer():
	imp(NULL),
	profilevalid()
{ }

studio::AudioContainer::~AudioContainer()
{
	if(imp) delete (imp);
}

bool studio::AudioContainer::load(const std::string &filename,const std::string &filedirectory)
{
	if(!imp)
	{
		imp = new AudioImp;
	}

	profilevalid = false;
	return imp->load(filename,filedirectory);
}

handle<studio::AudioProfile> studio::AudioContainer::get_profile(float /*samplerate*/)
	{ return handle<studio::AudioProfile>(); }

void studio::AudioContainer::clear()
{
	if (imp) { delete imp; imp = 0; }
	profilevalid = false;
}

void studio::AudioContainer::play(double t)
	{ if(imp) imp->play(t); }

void studio::AudioContainer::stop()
	{ if(imp) imp->stop(); }

bool studio::AudioContainer::get_current_time(double &out)
{
	if(imp) return imp->get_current_time(out);
	else return false;
}

void AudioContainer::set_offset(const double &s)
	{ if(imp) imp->set_offset(s); }

double AudioContainer::get_offset() const
{
	static double zero = 0;
	return imp ? imp->get_offset() : zero;
}

bool AudioContainer::is_playing() const
	{ return imp && imp->playing; }

bool AudioContainer::is_scrubbing() const
	{ return imp && imp->is_scrubbing(); }

void AudioContainer::start_scrubbing(double t)
	{ if (imp) imp->start_scrubbing(t); }

void AudioContainer::stop_scrubbing()
	{ if(imp) imp->stop_scrubbing(); }

void AudioContainer::scrub(double t)
	{ if(imp) imp->scrub(t); }

double AudioContainer::scrub_time() const
	{ return imp ? imp->scrub_time() : 0; }

bool AudioContainer::isRunning() const
	{ return imp && imp->isRunning(); }

bool AudioContainer::isPaused() const
	{ return imp && imp->isPaused(); }

//----------- Audio imp information -------------------

bool studio::AudioContainer::AudioImp::load(const std::string &/*filename*/,
											const std::string &/*filedirectory*/)
{
	clear();
	return false;
}

void studio::AudioContainer::AudioImp::play(double /*t*/)
	{ }

void studio::AudioContainer::AudioImp::stop()
{
	delaycon.disconnect();
	playing = false;
}

void studio::AudioContainer::AudioImp::clear()
{
	channel = 0;
	sample = 0;
	playing = false;
}

void studio::AudioContainer::AudioImp::start_scrubbing(double /*t*/)
{
	if (playing) stop();
	set_scrubbing(true);
}

void studio::AudioContainer::AudioImp::stop_scrubbing()
{
	if (is_scrubbing()) set_scrubbing(false);
	curscrubpos = 0;
}

void studio::AudioContainer::AudioImp::scrub(double /*t*/)
	{ }
