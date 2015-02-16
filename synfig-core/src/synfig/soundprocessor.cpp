/* === S Y N F I G ========================================================= */
/*!	\file synfig/soundprocessor.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "soundprocessor.h"
#include <Mlt.h>
#include <cmath>
#include <vector>

#endif

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class SoundProcessor::Internal
{
public:
	static bool initialized;
	std::vector<PlayOptions> stack;
	Mlt::Profile profile;
	Mlt::Producer *last_track;
	Mlt::Consumer *consumer;
	Time position;

	void clear()
	{
		if (last_track != NULL) { delete last_track; last_track = NULL; }
		if (consumer != NULL) { consumer->stop(); delete consumer; consumer = NULL; }
		stack.clear();
		stack.push_back(PlayOptions());
	}

	Internal(): last_track(), consumer(), position(0.0) { clear(); }
	~Internal() { clear(); }
};

bool SoundProcessor::Internal::initialized = false;

SoundProcessor::SoundProcessor()
{
	if (!Internal::initialized)
	{
		Mlt::Factory::init();
		Internal::initialized = true;
	}
	internal = new Internal();
}
SoundProcessor::~SoundProcessor() { delete internal; }

void SoundProcessor::clear()
{
	internal->clear();
}

void SoundProcessor::beginGroup(const PlayOptions &playOptions)
{
	internal->stack.push_back(PlayOptions(
		internal->stack.back().delay + playOptions.delay,
		internal->stack.back().volume * playOptions.volume ));
}

void SoundProcessor::endGroup()
{
	assert(internal->stack.size() > 1);
	if (internal->stack.size() > 1)
		internal->stack.pop_back();
}

void SoundProcessor::addSound(const PlayOptions &playOptions, const Sound &sound)
{
	PlayOptions options(
			internal->stack.back().delay + playOptions.delay,
			internal->stack.back().volume * playOptions.volume );
	if (options.volume <= 0.0) return;

	// Create track
	String filename;
	filename = String("avformat:")+sound.filename;
	
	Mlt::Producer *track = new Mlt::Producer(internal->profile, filename.c_str());
	if (track->get_producer() == NULL) { delete track; return; }
	int delay = (int)round(options.delay*internal->profile.fps());
	if (-delay >= track->get_length()) { delete track; return; }
	if (delay < 0) {
		// cut
		track->set_in_and_out(-delay, -1);
	} else
	if (delay > 0) {
		Mlt::Playlist *playlist = new Mlt::Playlist();
		playlist->blank(delay);
		playlist->append(*track);
		delete track;
		track = playlist;
	}

	if (internal->last_track == NULL)
		{ internal->last_track = track; return; }

	set_position(0.0);

	// Combine tracks
	Mlt::Tractor *tractor = new Mlt::Tractor();

	Mlt::Multitrack *multitrack = tractor->multitrack();
	multitrack->connect(*internal->last_track, 0);
	multitrack->connect(*track, 1);
	delete multitrack;

	Mlt::Transition transition(internal->profile, "mix");
	transition.set("combine", 1);
	Mlt::Field *field = tractor->field();
	field->plant_transition(transition, 0, 1);
	delete field;

	delete internal->last_track;
	internal->last_track = tractor;
}

Time SoundProcessor::get_position() const
{
	return Time(internal->last_track == NULL ? 0.0 :
				(double)internal->last_track->position()/internal->profile.fps() );
}

void SoundProcessor::set_position(Time value)
{
	if (internal->last_track != NULL)
	{
		internal->last_track->seek( (int)round(value*internal->profile.fps()) );
		internal->last_track->set_speed(1.0);
	}
}

bool SoundProcessor::get_playing() const
{
	return internal->consumer != NULL;
}

void SoundProcessor::set_playing(bool value)
{
	if (value)
	{
		if (internal->consumer == NULL)
		{
			internal->consumer = new Mlt::Consumer(internal->profile, "sdl_audio");
			if (internal->last_track != NULL)
			{
				internal->consumer->connect(*internal->last_track);
				internal->consumer->start();
			}
		}
	}
	else
	{
		if (internal->consumer != NULL)
		{
			internal->consumer->stop();
			delete internal->consumer;
			internal->consumer = NULL;
		}
	}
}

bool SoundProcessor::subsys_init() { return Mlt::Factory::init(); }
bool SoundProcessor::subsys_stop() { return true; }


/* === E N T R Y P O I N T ================================================= */
