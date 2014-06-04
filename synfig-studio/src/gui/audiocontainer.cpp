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

#include <algorithm>
#include <sigc++/signal.h>

#include <ETL/stringf>
#include <ETL/clock>
//#include <ETL/thread>
#include <glibmm/thread.h>

#include <synfig/general.h>

#include <glibmm/main.h>

#include "audiocontainer.h"

#include <cstdio>
#include <sys/stat.h>
#include <errno.h>

#include <set>
#include <vector>

#ifdef WITH_FMOD
#include <fmod.h>
#endif

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */
#ifdef __WIN32
#else //linux...
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

#ifdef WITH_FMOD
bool build_profile(FSOUND_SAMPLE *sample, double &samplerate, std::vector<char> &samples)
#else
bool build_profile(FSOUND_SAMPLE */*sample*/, double &/*samplerate*/, std::vector<char> &/*samples*/)
#endif
{
#ifdef WITH_FMOD

	float sps = samplerate;

	//trivial rejection...
	if(!sample || sps < 1)
	{
		synfig::warning("build_profile: Sample rate was too low or sample was invalid");
		return false;
	}

	//lock for all samples and process them into a subset
	unsigned int mode = FSOUND_Sample_GetMode(sample);

	//make sure that it's 8 bit... I hope this works...

	//sample rate of the actual song...
	int allsamplerate = 0;
	FSOUND_Sample_GetDefaults(sample,&allsamplerate,0,0,0);

	//get the size of the sample defaults from the mode
	int channels = 1;
	int channelsize = 1; //number of bytes

	if(mode & FSOUND_16BITS) channelsize = 2; //this shouldn't happen
	if(mode & FSOUND_STEREO) channels = 2;

	//Get the sample information
	int samplesize = channels*channelsize; //the only two things that increase samplesize
	int numsamples = FSOUND_Sample_GetLength(sample); //number of samples in the sound
	int sizeall = samplesize*numsamples; //should be the size of the entire song...

	if(sizeall <= 0)
	{
		synfig::warning("ProfileAudio: Sample buffer cannot be size smaller than 1 (%X)",FSOUND_GetError());
		return false;
	}

	//be sure that the new sample rate is less than or equal to the original
	if(sps > allsamplerate) sps = allsamplerate;

	float stride = allsamplerate/(float)sps;

	//down sampling to 8 bit min/max values
	synfig::warning("About to downsample from %d Hz to %.1f Hz, sample stride: %f", allsamplerate, sps, stride);

	char *sampledata=0,*useless = 0;
	unsigned int len1,len2;
	// vector<char>	samples;
	{
		if(!FSOUND_Sample_Lock(sample,0,sizeall,(void**)&sampledata,(void**)&useless,&len1,&len2))
		{
			synfig::warning("ProfileAudio: Unable to lock the sound buffer... (%X)",FSOUND_GetError());
			return false;
		}
		synfig::warning("Locked: %X: %d bytes, %X: %d bytes",sampledata,len1,useless,len2);

		if(channelsize == 1)
		{
			//process the data
			char *iter = sampledata;
			char *end = iter + sizeall;

			float curaccum = 0;
			float numinc = sps/(float)allsamplerate;

			/* Loop per sample DDA alg.
			*/

			int i = 0;

			//HACK - to prevent if statement inside inner loop
			//synfig::warning("wo baby wo baby, inc: %d, stride: %f, size: %d", inc, stride, sizeall);
			while(iter < end)
			{
				int maxs = 0, mins = 0;

				for(;curaccum < 1; curaccum += numinc)
				{
					for(i = 0; iter < end && i < channels; ++i, iter += channelsize)
					{
						maxs = std::max(maxs,(int)*iter);
						mins = std::min(mins,(int)*iter);
					}
				}
				//insert onto new list
				samples.push_back(maxs);
				samples.push_back(mins);

				//and flush all the used samples for curaccum
				curaccum -= 1;
			}
		}else if(channelsize == 2)
		{
			//process the data
			char *iter = sampledata;
			char *end = iter + sizeall;

			float curaccum = 0;
			float numinc = sps/(float)allsamplerate;

			/* Loop per sample DDA alg.
			*/

			int i = 0;

			//HACK - to prevent if statement inside inner loop
			//synfig::warning("wo baby wo baby, inc: %d, stride: %f, size: %d", inc, stride, sizeall);
			while(iter < end)
			{
				int maxs = 0, mins = 0;

				for(;curaccum < 1; curaccum += numinc)
				{
					for(i = 0; iter < end && i < channels; ++i, iter += channelsize)
					{
						maxs = std::max(maxs,(int)*(short*)iter);
						mins = std::min(mins,(int)*(short*)iter);
					}
				}
				//insert onto new list
				samples.push_back(maxs / 256);
				samples.push_back(mins / 256);

				//and flush all the used samples for curaccum
				curaccum -= 1;
			}
		}
	}

	synfig::warning("Stats: %f seconds with %d bytes now %d bytes", (samples.size()/2)/sps, sizeall, samples.size());
	synfig::warning("		%f seconds before", numsamples/(float)allsamplerate);

	//we're done yay!, unlock
	FSOUND_Sample_Unlock(sample,sampledata,useless,len1,len2);
	synfig::info("Unlocked");

	//FSOUND_PlaySound(FSOUND_FREE,sound); //test

	//we're done
	samplerate = sps*2; //it must be x2 because we are sampling max and min

	return true;

	#else

	return false;

	#endif
}


//FMOD Systemwide Specific data mostly here...

struct scrubinfo;

#ifdef WITH_FMOD
static double	buffer_length_sec = 0;

//------- Scrubbing --------------
/* Scrubbing works as follows:

	The sound is played using PlaySoundEx
		we specify a user created DSP for scrubbing
		set it initially to inactive

	When the program initiates it
		we set the initial data in the shared structure and activate the dsp unit
		then for each cursor update we get we set the value in the shared structure
*/

/* Things to check:
	If IsPlaying just governs the channel play/stop value or if it also concerns the pause state

*/

//so we can know where to create all this stuff
struct scrubinfo
{
	/*	Linearly fit the frequency to hit the desired zero point...
	*/
	/*struct scrubelement
	{
		double	pos;
		double	dt;
		//the amount of time left til the cursor hits this one
		//	it's incremental so that the cursor must pass previous
		//	ones before decrementing this value
	};
	*/

	//the time it should take to get to the next position...

	//to prevent from writing to the same location at once... (pos, deltatime, delaystart)
	//Glib::Mutex	lock;

	//the queue system would provide a more accurate representation...
	volatile double pos;
	volatile double deltatime;

	volatile double delaystart; //the amount of time we need to go before we start interpolating...

	volatile int	channel;

	/*std::list<scrubelement>	queue;

	volatile int	channel;

	//current position is FSOUND_GetCurrentPosition and current time is always 0...

	void add(const scrubelement &elem)
	{
		lock.LockWrite();

		queue.push_back(elem);

		lock.UnlockWrite();
	}

	//Function to safely get rid of all the old samples (dt < 0)
	void flush()
	{
		lock.LockWrite();

		while(queue.size() && queue.front().dt < 0)
		{
			queue.pop_front();
		}

		lock.UnlockWrite();
	}*/

	void Lock()
	{
		//lock.lock();
	}

	void Unlock()
	{
		//lock.unlock();
	}

	//All parameters and state should be set by the time we get here...
	void scrub_dsp_process()
	{
		const double epsilon = 1e-5;

		//Trivial reject... we go nowhere if we aren't playing (hit boundary...)
		if(!FSOUND_IsPlaying(channel)) return;

		//Get rid of all the old samples
		//flush();

		//Trivial reject #2 - We also go nowhere with no future samples (pause)
		/*if(queue.size() <= 0)
		{
			FSOUND_SetPaused(channel,true);
			return;
		}*/

		double dt = buffer_length_sec;

		//Lock ourselves so we don't die
		Lock();

		//printf("DSP data: delay = %.3f s, pos = %d, dt = %.3f\n", delaystart, (int)pos, deltatime);

		//Check delay
		if(delaystart > 0)
		{
			delaystart -= dt;

			if(delaystart < 0)
			{
				dt = -delaystart; //add time back...
				delaystart = 0;
			}
		}

		//Trivial reject for if we're past current sample...
		if(delaystart > 0 || deltatime <= 0)
		{
			FSOUND_SetPaused(channel,true);
			Unlock();
			return;
		}

		//Calculate stretched frequency based on delayed future sample...

		//NOTE: BY NOT TRACKING POSITION AS A FLOAT AND JUST USING THE SOUNDS VALUE
		//		WE ARE LOSING A TINY AMOUNT OF PRECISION ACCURACY EVERY UPDATE
		//		(THIS SHOULDN'T BE A PROBLEM)
		const double p0 = FSOUND_GetCurrentPosition(channel);
		double curdp = 0;

		if(!FSOUND_GetPaused(channel))
		{
			curdp = FSOUND_GetFrequency(channel) * deltatime;
		}

		//need to rescale derivative...

		//Extrapolate from difference in position and deltatime vs dt...
		const double pa = p0 + curdp/2;

		const double p1 = pos;

		//const double pb = p0/3 + p1*2/3;

		//will extrapolate if needed... (could be funky on a curve)
		double t = 0;
		if(deltatime > epsilon)
		{
			t = dt / deltatime;
		}

		//Decrement deltatime (we may have gone past but that's what happens when we don't get input...)
		deltatime -= dt;

		//we don't need to look at the current variables anymore...
		Unlock();

		const double invt = 1-t;
		//double deltapos = (p1-p0)*t; //linear version
		double deltapos = invt*invt*p0 + 2*t*invt*pa + t*t*p1 - p0; //quadratic smoothing version

		//Attempted cubic smoothing
		//const double invt2 = invt*invt;
		//const double t2 = t*t;
		//double deltapos = invt2*invt*p0 + 3*t*invt2*pa + 3*t2*invt*pb + t2*t*p1;
		//double deltapos = p0 + t*(3*(pa-p0) + t*(3*(p0+2*pa+pb) + t*((p1-3*pb+3*ba-p0)))); //unwound cubic

		//printf("\ttime = %.2f; p(%d,%d,%d) dp:%d - delta = %d\n",t,(int)p0,(int)p1,(int)p2,(int)curdp,(int)deltapos);

		//Based on the delta info calculate the stretched frequency
		const int dest_samplesize = FSOUND_DSP_GetBufferLength();

		//rounded to nearest frequency... (hopefully...)
		int freq = (int)(deltapos * FSOUND_GetOutputRate() / (double)dest_samplesize);

		//NOTE: WE MIGHT WANT TO DO THIS TO BE MORE ACCURATE BUT YEAH... ISSUES WITH SMALL NUMBERS
		//double newdp = deltapos / t;

		//printf("\tfreq = %d Hz\n", freq);

		// !If I failed... um assume we have to pause it... ?
		if(abs(freq) < 100)
		{
			FSOUND_SetPaused(channel,true);
		}else
		{
			//synfig::info("DSP f = %d Hz", freq);
			FSOUND_SetPaused(channel,false);
			if(!FSOUND_SetFrequency(channel,freq))
			{
				//ERROR WILL ROBINSON!!!...
				printf("Error in Freq... what do I do?\n");
			}
		}
	}
};

struct scrubuserdata
{
	/* //for use with multiple
	//each one is a 'handle' to a pointer that will be effected by something else
	typedef scrubinfo**	value_type;
	typedef std::set< value_type > scrubslist;
	scrubslist		scrubs;

	//so we can lock access to the list...
	ReadWriteLock	lock;

	void AddScrub(scrubinfo **i)
	{
		lock.LockWrite();
		scrubs.insert(i);
		lock.UnLockWrite();
	}

	void RemoveScrub(scrubinfo **i)
	{
		lock.LockWrite();
		scrubs.erase(i);
		lock.UnLockWrite();
	}*/

	scrubinfo * volatile *	scrub;
};

//Scrubbing data structures
static const int 		default_scrub_priority = 5; //between clear and sfx/music mix
static scrubuserdata	g_scrubdata = {0};
static FSOUND_DSPUNIT	*scrubdspunit = 0;

void * scrubdspwrap(void *originalbuffer, void *newbuffer, int length, void *userdata)
{
	//std::string	dsp = "DSP";
	if(userdata)
	{
		scrubuserdata &sd = *(scrubuserdata*)userdata;

		/* //For use with multiple scrubs...
		//Lock so no one can write to it while we're reading from it...
		sd.lock.LockRead();

		//make a copy of it...
		std::vector<scrubinfo**>	v(sd.scrubs.begin(),sd.scrubs.end());

		//other things can do stuff with it again...
		sd.lock.UnLockRead();

		//loop through the list and process all the active scrub units
		std::vector<scrubinfo**>::iterator	i = v.begin(),
											end = v.end();
		for(;i != end; ++i)
		{
			//check to make sure this object is active...
			if(*i && **i)
			{
				(**i)->scrub_dsp_process();
			}
		}
		*/

		if(sd.scrub && *sd.scrub)
		{
			//dsp += " processing...";
			scrubinfo * info = (*sd.scrub);
			info->scrub_dsp_process();
		}
	}

	//synfig::info(dsp);

	return newbuffer;
}

//------- Class for loading fmod on demand -------

class FMODInitializer
{
	bool loaded;
	int	refcount;

public:
	FMODInitializer():loaded(false),refcount(0) {}
	~FMODInitializer()
	{
		clear();
	}

	void addref()
	{
		if(!loaded)
		{
			#ifdef WITH_FMOD
			synfig::info("Initializing FMOD on demand...");

			{
				FSOUND_SetOutput(AUDIO_OUTPUT);

				/*int numdrivers = FSOUND_GetNumDrivers();
				synfig::info("Num FMOD drivers = %d",numdrivers);
				synfig::info("Current Driver is #%d", FSOUND_GetDriver());

				for(int i = 0; i < numdrivers; ++i)
				{
					unsigned int caps = 0;
					FSOUND_GetDriverCaps(i,&caps);

					synfig::info("   Caps for driver %d (%s) = %x",i,FSOUND_GetDriverName(i),caps);
				}

				FSOUND_SetDriver(0);*/

				//Modify buffer size...
				//FSOUND_SetBufferSize(100);

				if(!FSOUND_Init(44100, 32, 0))
				{
					synfig::warning("Unable to load FMOD");
				}else
				{
					loaded = true;

					//Create the DSP for processing scrubbing...
					scrubdspunit = FSOUND_DSP_Create(&scrubdspwrap,default_scrub_priority,&g_scrubdata);

					//Load the number of sec per buffer into the global variable...
					buffer_length_sec = FSOUND_DSP_GetBufferLength() / (double)FSOUND_GetOutputRate();
				}
			}
			#endif
		}

		//add to the refcount
		++refcount;
		//synfig::info("Audio: increment fmod refcount %d", refcount);
	}

	void decref()
	{
		if(refcount <= 0)
		{
			synfig::warning("FMOD refcount is already 0...");
		}else
		{
			--refcount;
			//synfig::info("Audio: decrement fmod refcount %d", refcount);

			//NOTE: UNCOMMENT THIS IF YOU WANT FMOD TO UNLOAD ITSELF WHEN IT ISN'T NEEDED ANYMORE...
			flush();
		}
	}

	bool is_loaded() const { return loaded; }

	void clear()
	{
		refcount = 0;
		flush();
	}

	void flush()
	{
		if(loaded && refcount <= 0)
		{
			#ifdef WITH_FMOD
			synfig::info("Unloading FMOD");
			if(scrubdspunit) FSOUND_DSP_Free(scrubdspunit);
			FSOUND_Close();
			#endif
			loaded = false;
		}
	}
};

//The global counter for FMOD....
FMODInitializer		fmodinit;

#endif

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

void studio::AudioProfile::set_parent(etl::handle<AudioContainer> i)
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
	etl::clock			timer;	//for getting the time diff between scrub input points

	//Scrubbing information...
	//the current position of the sound will be sufficient for normal stuff...
	#ifdef WITH_FMOD
	scrubinfo			scrinfo;
	#endif

	scrubinfo			*scrptr;

	bool is_scrubbing() const {return scrptr != 0;}
#ifdef WITH_FMOD
	void set_scrubbing(bool s)
#else
	void set_scrubbing(bool /*s*/)
#endif
	{
		#ifdef WITH_FMOD
		if(s)
			scrptr = &scrinfo;
		else
		#endif
		scrptr = 0;
	}

	//helper to make sure we are actually playing (and to get a new channel...)
	bool init_play()
	{
		#ifdef WITH_FMOD
		if(!FSOUND_IsPlaying(channel))
		{
			if(sample)
			{
				//play sound paused etc.
				channel = FSOUND_PlaySoundEx(FSOUND_FREE,sample,0,true);
				if(channel < 0 || FSOUND_GetError() != FMOD_ERR_NONE)
				{
					synfig::warning("Could not play the sample...");
					return false;
				}
			}
		}else
		{
			FSOUND_SetPaused(channel,true);
			FSOUND_SetFrequency(channel,sfreq);
		}
		return true;

		#else

		return false;

		#endif
	}

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
	{
		//reuse the channel...
		#ifdef WITH_FMOD
		channel = FSOUND_FREE;
		#endif
	}

	~AudioImp()
	{
		clear();
	}

public: //helper/accessor funcs
	bool start_playing_now() //callback for timer...
	{
		#ifdef WITH_FMOD
		if(playing)
		{
			//Make sure the sound is playing and if it is un pause it...
			if(init_play())
				FSOUND_SetPaused(channel,false);
		}
		#endif

		return false; //so the timer doesn't repeat itself
	}

	bool isRunning()
	{
		#ifdef WITH_FMOD
		return FSOUND_IsPlaying(channel);
		#else
		return false;
		#endif
	}

	bool isPaused()
	{
#ifdef WITH_FMOD
		return FSOUND_GetPaused(channel);
#else
		return false;
#endif
	}


public: //forward interface

	//Accessors for the offset - in seconds
	const double &get_offset() const {return offset;}
	void set_offset(const double &d)
	{
		offset = d;
	}

	//Will override the parameter timevalue if the sound is running, and not if it's not...
#ifdef WITH_FMOD
	bool get_current_time(double &out)
#else
	bool get_current_time(double &/*out*/)
#endif
	{
		if(isRunning())
		{
			#ifdef WITH_FMOD
			unsigned int pos = FSOUND_GetCurrentPosition(channel);

			//adjust back by 1 frame... HACK....
			//pos -= FSOUND_DSP_GetBufferLength();

			//set the position
			out = pos/(double)sfreq + offset;
			#endif

			return true;
		}
		return false;
	}

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
	{
		return curscrubpos;
	}
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

#ifdef WITH_FMOD
handle<studio::AudioProfile> studio::AudioContainer::get_profile(float samplerate)
#else
handle<studio::AudioProfile> studio::AudioContainer::get_profile(float /*samplerate*/)
#endif
{
	#ifdef WITH_FMOD

	//if we already have done our work, then we're good
	if(profilevalid && prof)
	{
		//synfig::info("Using already built profile");
		return prof;
	}

	//synfig::info("Before profile");
	//make a new profile at current sample rate

	//NOTE: We might want to reuse the structure already there...
	prof = new AudioProfile;
	prof->set_parent(this); //Our parent is THIS!!!

	if(!prof)
	{
		synfig::warning("Couldn't allocate audioprofile...");
		return handle<studio::AudioProfile>();
	}

	//setting the info for the sample rate
	//synfig::info("Setting info...");

	synfig::info("Building Profile...");
	prof->samplerate = samplerate;
	if(build_profile(imp->sample,prof->samplerate,prof->samples))
	{
		synfig::info("	Success!");
		profilevalid = true;
		return prof;
	}else
	{
		return handle<studio::AudioProfile>();
	}

	#else

	return handle<studio::AudioProfile>();

	#endif
}

void studio::AudioContainer::clear()
{
	if(imp)
	{
		delete imp;
		imp = 0;
	}

	profilevalid = false;
}

void studio::AudioContainer::play(double t)
{
	if(imp) imp->play(t);
}

void studio::AudioContainer::stop()
{
	if(imp) imp->stop();
}

bool studio::AudioContainer::get_current_time(double &out)
{
	if(imp) return imp->get_current_time(out);
	else return false;
}

void AudioContainer::set_offset(const double &s)
{
	if(imp) imp->set_offset(s);
}

double AudioContainer::get_offset() const
{
	static double zero = 0;
	if(imp)
		return imp->get_offset();
	return zero;
}

bool AudioContainer::is_playing() const
{
	if(imp)
		return imp->playing;
	return false;
}

bool AudioContainer::is_scrubbing() const
{
	if(imp)
		return imp->is_scrubbing();
	return false;
}

void AudioContainer::start_scrubbing(double t)
{
	if(imp) imp->start_scrubbing(t);
}

void AudioContainer::stop_scrubbing()
{
	if(imp) imp->stop_scrubbing();
}

void AudioContainer::scrub(double t)
{
	if(imp) imp->scrub(t);
}

double AudioContainer::scrub_time() const
{
	if(imp) return imp->scrub_time();
	else return 0;
}

bool AudioContainer::isRunning() const
{
	if(imp) return imp->isRunning();
	else return false;
}

bool AudioContainer::isPaused() const
{
	if(imp) return imp->isPaused();
	else return false;
}

//----------- Audio imp information -------------------

#ifdef WITH_FMOD
bool studio::AudioContainer::AudioImp::load(const std::string &filename,
											const std::string &filedirectory)
#else
bool studio::AudioContainer::AudioImp::load(const std::string &/*filename*/,
											const std::string &/*filedirectory*/)
#endif
{
	clear();

	#ifdef WITH_FMOD

	//And continue with the sound loading...
	string 	file = filename;

	//Trivial reject... (fixes stat call problem... where it just looks at directory and not file...)
	if(file.length() == 0) return false;

	//we don't need the file directory?
	if(!is_absolute_path(file))
	{
		file=filedirectory+filename;
		synfig::warning("Not absolute hoooray");
	}
	synfig::info("Loading Audio file: %s", file.c_str());

	//check to see if file exists
	{
		struct stat	s;
		if(stat(file.c_str(),&s) == -1 && errno == ENOENT)
		{
			synfig::info("There was no audio file...");
			return false;
		}
	}

	//load fmod if we can...
	//synfig::warning("I'm compiled with FMOD!");
	fmodinit.addref();

	//load the stream
	int ch = FSOUND_FREE;
	FSOUND_SAMPLE *sm = FSOUND_Sample_Load(FSOUND_FREE,file.c_str(),FSOUND_LOOP_OFF|FSOUND_MPEGACCURATE,0,0);

	if(!sm)
	{
		synfig::warning("Could not open the audio file as a sample: %s",file.c_str());
		goto error;
	}

	//synfig::warning("Opened a file as a sample! :)");

	/*{
		int bufferlen = FSOUND_DSP_GetBufferLength();
		synfig::info("Buffer length = %d samples, %.3lf s",bufferlen, bufferlen / (double)FSOUND_GetOutputRate());
	}*/

	//set all the variables since everything has worked out...
	//get the length of the stream
	{
		length = FSOUND_Sample_GetLength(sm);

		int volume = 0;
		FSOUND_Sample_GetDefaults(sm,&sfreq,&volume,0,0);

		//double len = length / (double)sfreq;
		//synfig::info("Sound info: %.2lf s long, %d Hz, %d Vol",(double)length,sfreq,volume);
	}

	//synfig::warning("Got all info, and setting up everything, %.2f sec.", length);
	//synfig::warning("	BigSample: composed of %d samples", FSOUND_Sample_GetLength(sm));
	synfig::info("Successfully opened %s as a sample and initialized it.",file.c_str());

	//set up the playable info
	sample = sm;
	channel = ch;

	//the length and sfreq params have already been initialized

	return true;

error:
	if(sm) FSOUND_Sample_Free(sm);
	file = "";

	fmodinit.decref();

	return false;

	#else
	return false;
	#endif
}

#ifdef WITH_FMOD
void studio::AudioContainer::AudioImp::play(double t)
#else
void studio::AudioContainer::AudioImp::play(double /*t*/)
#endif
{
	#ifdef WITH_FMOD
	if(!sample) return;

	//stop scrubbing if we are...
	if(is_scrubbing()) stop_scrubbing();

	//t -= offset;
	t -= get_offset();
	playing = true;

	if(t < 0)
	{
		unsigned int timeout = (int)floor(-t * 1000 + 0.5);
		//synfig::info("Playing audio delayed by %d ms",timeout);
		//delay for t seconds...
		delaycon = Glib::signal_timeout().connect(
						sigc::mem_fun(*this,&studio::AudioContainer::AudioImp::start_playing_now),timeout);

		init_play();
		FSOUND_SetFrequency(channel,sfreq);
		FSOUND_SetCurrentPosition(channel,0);
		return;
	}

	unsigned int position = (int)floor(t*sfreq + 0.5);

	if(position >= FSOUND_Sample_GetLength(sample))
	{
		synfig::warning("Can't play audio when past length...");
		return;
	}

	init_play();
	FSOUND_SetFrequency(channel,sfreq);
	FSOUND_SetCurrentPosition(channel,position);
	FSOUND_SetPaused(channel,false);

	//synfig::info("Playing audio with position %d samples",position);

	#endif
}

void studio::AudioContainer::AudioImp::stop()
{
	delaycon.disconnect();

	#ifdef WITH_FMOD
	if(fmodinit.is_loaded() && playing && isRunning())
	{
		FSOUND_SetPaused(channel,true);
	}
	#endif

	playing = false;
}

void studio::AudioContainer::AudioImp::clear()
{
	#ifdef WITH_FMOD
	delaycon.disconnect();

	stop();
	stop_scrubbing();

	if(sample)
	{
		if(FSOUND_IsPlaying(channel))
		{
			FSOUND_StopSound(channel);
		}
		channel = FSOUND_FREE;
		FSOUND_Sample_Free(sample);
		fmodinit.decref();
	}

	playing = false;

	#else
	channel = 0;
	#endif

	sample = 0;
	playing = false;
}

#ifdef WITH_FMOD
void studio::AudioContainer::AudioImp::start_scrubbing(double t)
#else
void studio::AudioContainer::AudioImp::start_scrubbing(double /*t*/)
#endif
{
	//synfig::info("Start scrubbing: %lf", t);
	if(playing) stop();

	set_scrubbing(true);

	#ifdef WITH_FMOD
	//make sure the other one is not scrubbing...
	if(g_scrubdata.scrub)
	{
		*g_scrubdata.scrub = 0; //nullify the pointer...
	}

	//Set up the initial state for the delayed audio position
	scrinfo.delaystart = 0;
	scrinfo.pos = 0;
	scrinfo.deltatime = 0;

	//set it to point to our pointer (dizzy...)
	g_scrubdata.scrub = &scrptr;

	//setup position info so we can know what to do on boundary conditions...
	curscrubpos = (t - get_offset()) * sfreq;

	//So we can get an accurate difference...
	timer.reset();

	//reposition the sound if it won't be when scrubbed (if it's already in the range...)
	int curi = (int)curscrubpos;
	if(curi >= 0 && curi < length)
	{
		init_play();
		FSOUND_SetCurrentPosition(channel,curi);

		//Set the values...
		scrinfo.pos = curscrubpos;
		scrinfo.delaystart = delay_factor*buffer_length_sec;

		//synfig::info("\tStarting at %d samps, with %d p %.3f delay",
		//				FSOUND_GetCurrentPosition(channel), (int)scrinfo.pos, scrinfo.delaystart);
	}



	//enable the dsp...
	//synfig::info("\tActivating DSP");
	FSOUND_DSP_SetActive(scrubdspunit,true);
	#endif
}

void studio::AudioContainer::AudioImp::stop_scrubbing()
{
	//synfig::info("Stop scrubbing");

	if(is_scrubbing())
	{
		set_scrubbing(false);

		#ifdef WITH_FMOD
		g_scrubdata.scrub = 0;

		//stop the dsp...
		//synfig::info("\tDeactivating DSP");
		FSOUND_DSP_SetActive(scrubdspunit,false);
		if(FSOUND_IsPlaying(channel)) FSOUND_SetPaused(channel,true);
		#endif
	}

	curscrubpos = 0;
}

#ifdef WITH_FMOD
void studio::AudioContainer::AudioImp::scrub(double t)
#else
void studio::AudioContainer::AudioImp::scrub(double /*t*/)
#endif
{
	#ifdef WITH_FMOD
	//synfig::info("Scrub to %lf",t);
	if(is_scrubbing())
	{
		//What should we do?

		/* Different special cases
			All outside, all inside,
			coming in (left or right),
			going out (left or right)
		*/
		double oldpos = curscrubpos;
		double newpos = (t - get_offset()) * sfreq;

		curscrubpos = newpos;

		//Ok the sound is running, now we need to tweak it
		if(newpos > oldpos)
		{
			//Outside so completely stopped...
			if(newpos < 0 || oldpos >= length)
			{
				//synfig::info("\tOut +");
				if(FSOUND_IsPlaying(channel))
				{
					FSOUND_SetPaused(channel,true);
				}

				//Zero out the data!
				scrinfo.Lock();
				scrinfo.delaystart = 0;
				scrinfo.deltatime = 0;
				scrinfo.Unlock();

				return;
			}

			//going in? - start the sound at the beginning...
			/*else if(oldpos < 0)
			{
				//Set up the sound to be playing paused at the start...
				init_play();
				FSOUND_SetCurrentPosition(channel,0);

				synfig::info("\tIn + %d", FSOUND_GetCurrentPosition(channel));

				scrinfo.Lock();
				scrinfo.pos = 0;
				scrinfo.delaystart = delay_factor*buffer_length_sec;
				scrinfo.deltatime = 0;
				scrinfo.Unlock();
			}*/
			//don't need to deal with leaving... automatically dealt with...

			else //We're all inside...
			{
				//Set new position and decide what to do with time...
				scrinfo.Lock();
				scrinfo.pos = newpos;

				//should we restart the delay cycle... (is it done?)
				if(!isRunning() || (scrinfo.delaystart <= 0 && scrinfo.deltatime <= 0 && isPaused()))
				{
					//synfig::info("Starting + at %d",(int)newpos);
					scrinfo.deltatime = 0;
					scrinfo.delaystart = delay_factor*buffer_length_sec;
					scrinfo.Unlock();

					//Set up the sound paused at the current position
					init_play();
					int setpos = min(max((int)newpos,0),length);
					FSOUND_SetCurrentPosition(channel,setpos);
					timer.reset();
					return;
				}

				//No! just increment the time delta...
				scrinfo.deltatime += timer.pop_time();

				//Nope... continue and just increment the deltatime and reset position...
				scrinfo.Unlock();

				//set channel and unpause
				FSOUND_SetPaused(channel,false);
				scrinfo.channel = channel;

			}
		}else if(newpos < oldpos)
		{
			//completely stopped...
			if(newpos >= length || oldpos < 0)
			{
				//synfig::info("Out -");
				if(FSOUND_IsPlaying(channel))
				{
					FSOUND_SetPaused(channel,true);
				}

				//Zero out the data!
				scrinfo.Lock();
				scrinfo.delaystart = 0;
				scrinfo.deltatime = 0;
				scrinfo.Unlock();
			}

			//going in? - start going backwards at the end...
			/*else if(oldpos >= length)
			{
				synfig::info("In -");
				//Set up the sound to be playing paused at the start...
				init_play();
				FSOUND_SetCurrentPosition(channel,length-1);

				scrinfo.Lock();
				scrinfo.pos = length-1;
				scrinfo.delaystart = delay_factor*buffer_length_sec;
				scrinfo.deltatime = 0;
				scrinfo.Unlock();
			}*/
			//we don't have to worry about the leaving case...

			else //We're all inside...
			{
				//Set new position and decide what to do with time...
				scrinfo.Lock();
				scrinfo.pos = newpos;

				//should we restart the delay cycle... (is it done?)
				if(!isRunning() ||(scrinfo.delaystart <= 0 && scrinfo.deltatime <= 0 && isPaused()))
				{
					//synfig::info("Starting - at %d",(int)newpos);
					scrinfo.deltatime = 0;
					scrinfo.delaystart = delay_factor*buffer_length_sec;
					scrinfo.Unlock();

					//reset timing so next update will be a valid diff...
					init_play();
					int setpos = min(max((int)newpos,0),length);
					FSOUND_SetCurrentPosition(channel,setpos);
					timer.reset();
					return;
				}

				//No! just increment the time delta...
				scrinfo.deltatime += timer.pop_time();

				//Nope... continue and just increment the deltatime and reset position...
				scrinfo.Unlock();

				//set channel and unpause
				FSOUND_SetPaused(channel,false);
				scrinfo.channel = channel;
			}
		}
	}
	#endif
}
