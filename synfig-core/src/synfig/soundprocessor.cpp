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

#endif

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class SoundProcessor::Internal {
public:
	std::vector<PlayOptions> stack;
	// TODO:
};

SoundProcessor::SoundProcessor():
	internal(new Internal())
{
	internal->stack.push_back(PlayOptions());
	// TODO:
}

SoundProcessor::~SoundProcessor() { delete internal; }

void SoundProcessor::clear()
{
	set_playing(false);
	// TODO:
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
	// TODO:
}

Time SoundProcessor::get_position() const
{
	// TODO:
	return Time();
}

void SoundProcessor::set_position(Time value)
{
	// TODO:
}

bool SoundProcessor::get_playing() const
{
	// TODO:
	return false;
}

void SoundProcessor::set_playing(bool value)
{
	// TODO:
}

/* === E N T R Y P O I N T ================================================= */
