/* === S Y N F I G ========================================================= */
/*!	\file audiocontainer.h
**	\brief Sound info header
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

#ifndef __SYNFIG_AUDIOCONTAINER_H
#define __SYNFIG_AUDIOCONTAINER_H

/* === H E A D E R S ======================================================= */
#include <ETL/handle>

#include <sigc++/sigc++.h>
#include <string>
#include <vector>

/* === M A C R O S ========================================================= */
const float DEF_DISPLAYSAMPLERATE = 400;
/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class AudioContainer;

//Note: Might want to abstract something to share data between profile and parent
class AudioProfile : public etl::shared_object
{
public:
	typedef std::vector<char>	SampleProfile;

private:
	SampleProfile	samples;
	double			samplerate; //samples / second of the profile

	//reference our parent for any native sound info
	std::shared_ptr<AudioContainer>	parent;

public:	//samples interface

	SampleProfile::const_iterator	begin() const 	{return samples.begin();}
	SampleProfile::const_iterator	end() const 	{return samples.end();}

	void clear();
	unsigned int size() const {return samples.size();}

	char operator[](int i) const
	{
		if(i >= 0 && i < (int)samples.size()) return samples[i];
		else return 0;
	}

public: //

	double get_samplerate() const {return samplerate;}
	void set_samplerate(double f) {samplerate = f;}

	double get_offset() const;

	etl::handle<AudioContainer>	get_parent() const;
	void set_parent(etl::handle<AudioContainer> i);
	friend class AudioContainer;
};

/*	Audio container actually implements all the cool stuff
	Note: May be a bit too monolithic...
*/
class AudioContainer : public sigc::trackable, public etl::shared_object
{
	etl::handle<AudioProfile>	prof;

	struct	AudioImp;
	AudioImp *imp;

	bool	profilevalid; //this is only half useful
		//it makes it so we don't always have to realloc memory when the file switches...

public: //structors

	AudioContainer();
	~AudioContainer();

public: //accessor interface
	void set_offset(const double &s);
	double get_offset() const;

public: //info gather interface
	etl::handle<AudioProfile>	get_profile(float samplerate = DEF_DISPLAYSAMPLERATE);
	bool get_current_time(double &out);

public: //operational interface
	bool load(const std::string &filename, const std::string &filedirectory = "");
	void clear();

	//play functions...
	void play(double t);
	void stop();
	//Note: this refers to the wrapper concept of the audio, the actual sound may or may not be playing...
	bool is_playing() const;

	//scrubbing functions...
	void start_scrubbing(double t);
	void stop_scrubbing();
	void scrub(double t); //!< if we are not currently scrubbing this will not work
	bool is_scrubbing() const;

	double scrub_time() const;

	bool isRunning() const;
	bool isPaused() const;
};

} // END of namespace studio

/* === E N D =============================================================== */

#endif
