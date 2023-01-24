/* === S Y N F I G ========================================================= */
/*!	\file synfig/soundprocessor.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_SOUNDPROCESSOR_H
#define __SYNFIG_SOUNDPROCESSOR_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <map>
#include <limits>

#include "time.h"
#include "real.h"
#include "filesystem.h"


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

class SoundProcessor
{
public:
	class PlayOptions {
	public:
		Time delay;
		Real volume;
		PlayOptions(): delay(0.0), volume(1.0) { }
		explicit PlayOptions(Time delay, Real volume): delay(delay), volume(volume) { }
	};

	class Sound {
	public:
		String filename;
		Sound(): filename() { }
		explicit Sound(const String &filename): filename(FileSystem::fix_slashes(filename)) { }
	};

private:
	class Internal;
	Internal *internal;
	bool infinite;

public:
	SoundProcessor();
	~SoundProcessor();

	void clear();

	void beginGroup(const PlayOptions &playOptions);
	void endGroup();

	void addSound(const PlayOptions &playOptions, const Sound &sound);

	Time get_position() const;
	void set_position(Time value);

	void set_infinite(bool value);

	bool get_playing() const;
	void set_playing(bool value);

	void do_export(String path);

	static bool subsys_init();
	static bool subsys_stop();
};

}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
