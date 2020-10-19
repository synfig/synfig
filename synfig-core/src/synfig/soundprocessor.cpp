/* === S Y N F I G ========================================================= */
/*!	\file synfig/soundprocessor.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#include <vector>

#ifndef WITHOUT_MLT
#include <Mlt.h>
#endif

#include "general.h"
#include "soundprocessor.h"

#endif

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
#ifndef WITHOUT_MLT
	Mlt::Profile profile;
	Mlt::Producer *last_track;
	Mlt::Consumer *consumer;
#endif
	bool playing;
	Time position;

	void clear() {
#ifndef WITHOUT_MLT
		if (last_track != NULL) { delete last_track; last_track = NULL; }
		if (consumer != NULL) { consumer->stop(); delete consumer; consumer = NULL; }
		stack.clear();
		stack.push_back(PlayOptions());
#endif
	}

	Internal():
#ifndef WITHOUT_MLT
		last_track(), consumer(),
#endif
		playing(), position(0.0) { clear(); }
	~Internal() { clear(); }
};

bool SoundProcessor::Internal::initialized = false;

SoundProcessor::SoundProcessor()
{
	assert(Internal::initialized);
	internal = new Internal();
	infinite = true;
}

SoundProcessor::~SoundProcessor() { delete internal; }

void SoundProcessor::clear()
	{ internal->clear(); }

void SoundProcessor::beginGroup(const PlayOptions &playOptions)
{
	internal->stack.push_back(PlayOptions(
		internal->stack.back().delay + playOptions.delay,
		internal->stack.back().volume * playOptions.volume ));
}

void SoundProcessor::endGroup()
{
#ifndef WITHOUT_MLT
	assert(internal->stack.size() > 1);
	if (internal->stack.size() > 1)
		internal->stack.pop_back();
#endif
}

void SoundProcessor::addSound(const PlayOptions &playOptions, const Sound &sound)
{
#ifndef WITHOUT_MLT
	PlayOptions options(
			internal->stack.back().delay + playOptions.delay,
			internal->stack.back().volume * playOptions.volume );
	if (options.volume <= 0.0) return;

	// Create track
	Mlt::Producer *track = new Mlt::Producer(internal->profile, (String("avformat:") + sound.filename).c_str());
	if (track->get_producer() == NULL || track->get_length() <= 0) {
		delete track;
		track = new Mlt::Producer(internal->profile, (String("vorbis:") + sound.filename).c_str());
		if (track->get_producer() == NULL || track->get_length() <= 0) { delete track; return; }
	}

	int delay = (int)round(options.delay*internal->profile.fps());
	if (-delay >= track->get_length()) { delete track; return; }

	// set volume
	if (!approximate_equal(options.volume, Real(1))) {
		Mlt::Filter *filter = new Mlt::Filter(internal->profile, "volume");
		filter->set("gain", options.volume);
		track->attach(*filter);
	}

	// set delay
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

	if (internal->last_track == NULL) {

		if (!infinite) {
			internal->last_track = track;
			return;
		}

		// create background infinite track,
		// show (and track position) must go on even after all sounds finished
		bool success = true;
		Mlt::Producer *infinite_track = new Mlt::Producer(internal->profile, "tone");
		if (success && !infinite_track->get_producer()) {
			success = false;
			error("SoundProcessor: cannot create the 'tone' MLT producer for fake infinite track");
		}
		if (success && infinite_track->set("frequency", 1.0)) {
			success = false;
			error("SoundProcessor: cannot set frequency of the 'tone' MLT producer for fake infinite track");
		}
		if (success && infinite_track->set("level", -1000000.0)) {
			success = false;
			error("SoundProcessor: cannot mute the 'tone' MLT producer for fake infinite track");
		}
		
		if (infinite_track->get_producer() == NULL) {
			delete infinite_track;
			internal->last_track = track;
			return;
		}
		internal->last_track = infinite_track;
	}

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
#endif
}

void SoundProcessor::set_infinite(bool value)
{
	infinite = value;
}

Time SoundProcessor::get_position() const
{
#ifndef WITHOUT_MLT
	return Time(internal->last_track == NULL ? 0.0 :
				(double)internal->last_track->position()/internal->profile.fps() );
#else
	return Time();
#endif
}

void SoundProcessor::set_position(Time value)
{
#ifndef WITHOUT_MLT
	Time dt = value - get_position();
	if (dt >= Time(-0.01) && dt <= Time(0.01))
		return;
	if (internal->last_track != NULL) {
		bool restart = internal->playing && internal->consumer;
		if (restart) set_playing(false);
		internal->last_track->seek( (int)round(value*internal->profile.fps()) );
		if (restart) set_playing(true);
	}
#endif
}

bool SoundProcessor::get_playing() const
	{ return internal->playing; }

void SoundProcessor::set_playing(bool value)
{
#ifndef WITHOUT_MLT
	if (value == internal->playing) return;
	internal->playing = value;
	if (internal->playing) {
		if (internal->last_track != NULL) {
			internal->last_track->set_speed(1.0);
			internal->consumer = new Mlt::Consumer(internal->profile, "sdl_audio");
			internal->consumer->connect(*internal->last_track);
			internal->consumer->start();
		}
	} else {
		if (internal->consumer) {
			internal->consumer->stop();
			delete internal->consumer;
			internal->consumer = NULL;
		}
	}
#endif
}

void SoundProcessor::do_export(String path)
{
#ifndef WITHOUT_MLT
	if (internal->last_track != NULL) {
		internal->last_track->set_speed(1.0);
		internal->consumer = new Mlt::Consumer(internal->profile, "avformat");
		internal->consumer->connect(*internal->last_track);
		internal->consumer->set("target", path.c_str());
		internal->consumer->run();
	}
#endif
}

bool SoundProcessor::subsys_init() {
	if (!Internal::initialized)
#ifndef WITHOUT_MLT
		Internal::initialized = Mlt::Factory::init();
#else
		Internal::initialized = true;
#endif
	return Internal::initialized;
}

bool SoundProcessor::subsys_stop()
{
#ifndef WITHOUT_MLT
	Mlt::Factory::close();
#endif
	return true;
}


/* === E N T R Y P O I N T ================================================= */
