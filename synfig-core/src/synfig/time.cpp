/* === S Y N F I G ========================================================= */
/*!	\file time.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2008 Gerco Ballintijn
**  Copyright (c) 2008 Carlos López
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

#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdio>

#include <algorithm>

#include <ETL/stringf>
#include <ETL/misc>

#include "general.h"
#include "real.h"

#include "time.h"

#include <synfig/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

#define tolower ::tolower

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */

Time::Time(const String &str_, float fps):
	value_(0)
{
	String str(str_);
	std::transform(str.begin(),str.end(),str.begin(),&tolower);

	// Start/Begin Of Time
	if(str=="sot" || str=="bot")
	{
		operator=(begin());
		return;
	}
	// End Of Time
	if(str=="eot")
	{
		operator=(end());
		return;
	}


	unsigned int pos=0;
	int read;
	float amount;

	// Now try to read it in the letter-abbreviated format
	while(pos<str.size() && sscanf(String(str,pos).c_str(),"%f%n",&amount,&read))
	{
		pos+=read;
		if(pos>=str.size() || read==0)
		{
			// Throw up a warning if there are no units
			// and the amount isn't zero. There is no need
			// to warn about units if the value is zero
			// it is the only case where units are irrelevant.
			if(amount!=0 && fps)
			{
				synfig::warning(_("Time(): No unit provided in time code, assuming FRAMES (\"%s\")"),str.c_str());
				value_+=amount/fps;
			}
			else
			{
				synfig::warning(_("Time(): No unit provided in time code and frame rate is unknown! Assuming SECONDS"));
				value_+=amount;
			}
			return;
		}
		switch(str[pos])
		{
			case 'h':
			case 'H':
				value_+=amount*3600;
				break;
			case 'm':
			case 'M':
				value_+=amount*60;
				break;
			case 's':
			case 'S':
				value_+=amount;
				break;
			case 'f':
			case 'F':
				if(fps)
					value_+=amount/fps;
				else
					synfig::warning("Time(): Individual frames referenced, but frame rate is unknown");
				break;
			case ':':
				// try to read it in as a traditional time format
				{
					int hour,minute,second;
					float frame;
					if(fps && sscanf(str.c_str(),"%d:%d:%d.%f",&hour,&minute,&second,&frame)==4)
					{
							value_=frame/fps+(hour*3600+minute*60+second);
							return;
					}

					if(sscanf(str.c_str(),"%d:%d:%d",&hour,&minute,&second)==3)
					{
						value_=hour*3600+minute*60+second;
						return;
					}
				}
				synfig::warning("Time(): Bad time format");
				break;

			default:
				value_+=amount;
				synfig::warning("Time(): Unexpected character '%c' when parsing time string \"%s\"",str[pos],str.c_str());
				break;
		}
		pos++;
		amount=0;
	}
}

// This functions suggests what time is in seconds
std::string Time::get_string(Time::Format format) const
{
	Time time(*this);
	if (time <= begin())
		return "SOT";	// Start Of Time
	if (time >= end())
		return "EOT";	// End Of Time

	if(format <= FORMAT_NORMAL)
	{
		synfig::ChangeLocale change_locale(LC_NUMERIC, "C");
		return strprintf("%.3f", (float)time);
	}


	if(format<=FORMAT_VIDEO)
	{
		int hours, minutes, seconds, microseconds;
		hours = time / 3600;
		time -= hours*3600;

		minutes = time / 60;
		time -= minutes*60;

		seconds=time;
		time -= seconds;

		microseconds = time*1000;

		return strprintf("%02d:%02d:%02d.%02d", hours, minutes, seconds, microseconds);
	}

	synfig::error(_("Translating Time to unknown format (not implemented)"));

	return "";
}

String
Time::get_string(float fps, Time::Format format)const
{
	Time time(*this);

	if(time<=begin())
		return "SOT";	// Start Of Time
	if(time>=end())
		return "EOT";	// End Of Time

	if(fps<0)fps=0;

	if(ceil(time.value_)-time.value_<epsilon_())
		time.value_=ceil(time.value_);

	int hour = 0, minute = 0;
	if(!(format<=FORMAT_FRAMES))
	{
		hour=time/3600;time-=hour*3600;
		minute=time/60;time-=minute*60;
	}
	// <= is redefined, so this means "is the FORMAT_VIDEO bit set in the format?"
	if(format<=FORMAT_VIDEO)
	{
		int second;
		second=time;time-=second;

		if(fps && fps>1)
		{
			int frame;
			frame=round_to_int(time*fps);

			return strprintf("%02d:%02d:%02d.%02d",hour,minute,second,frame);
		}
		else
			return strprintf("%02d:%02d:%02d",hour,minute,second);
	}

	if (format <= FORMAT_FRAMES)
	{
		if (fps && fps>0)
			return strprintf("%df", round_to_int(time * fps));
		else
			return strprintf("%ds", round_to_int(time * 1));
	}

	String ret;
	bool started = false;

	if(format<=FORMAT_FULL || hour)
	{
		ret+=strprintf("%dh",hour);
		started = true;
	}

	if(format<=FORMAT_FULL || minute)
	{
		if (!(format<=FORMAT_NOSPACES) && started)
			ret += " ";

		ret += strprintf("%dm", minute);
		started = true;
	}

	if(fps && fps>1)
	{
		int second;
		float frame;
		second=time;time-=second;
		frame=time*fps;

		if(format<=FORMAT_FULL || second)
		{
			if (!(format<=FORMAT_NOSPACES) && started)
				ret += " ";

			ret += strprintf("%ds", (int)second);
			started = true;
		}

		if(format<=FORMAT_FULL || abs(frame) > epsilon_() || !started)
		{
			if (!(format<=FORMAT_NOSPACES) && started)
				ret += " ";

			if (fabs(frame-floor(frame)) >= epsilon_())
				ret += strprintf("%0.3ff", frame);
			else
				ret += strprintf("%0.0ff", frame);
		}
	}
	else
	{
		float second;
		second=time;
		if(format<=FORMAT_FULL || second || !started)
		{
			if (!(format<=FORMAT_NOSPACES) && started)
				ret += " ";

			if(abs(second-floor(second))>=epsilon_())
			{
				String seconds(strprintf("%0.8f",second));

				// skip trailing zeros
				int count = 0;
				String::reverse_iterator i = seconds.rbegin();
				for ( ; (*i) == '0'; i++)
					count++;

				// if we removed too many, go back one place, leaving one zero
				if (*i < '0' || *i > '9') count--;

				ret += seconds.substr(0, seconds.size()-count) + "s";
			}
			else
				ret+=strprintf("%0.0fs",second);
		}
	}

	return ret;
}

Time
Time::round(float fps)const
{
	// the aim is to make results for the same frame absolutely identical
	assert(approximate_greater_lp(fps, 0.f));
	if (!approximate_greater_lp(fps, 0.f)) return *this;
	return Time(floor(value_*fps + 0.5)/fps);
}

#ifdef _DEBUG
const char *
Time::c_str()const
{
	return get_string().c_str();
}
#endif

//! \writeme
bool
Time::is_valid()const
{
	return !std::isnan(value_);
}
