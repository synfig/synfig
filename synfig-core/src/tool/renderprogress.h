/* === S Y N F I G ========================================================= */
/*!	\file tool/renderprogress.h
**	\brief RenderProgress class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#ifndef __SYNFIG_RENDERPROGRESS_H
#define __SYNFIG_RENDERPROGRESS_H

using namespace std;
using namespace etl;
using namespace synfig;

#include <synfig/string.h>
#include "definitions.h"

class RenderProgress : public synfig::ProgressCallback
{
	string taskname;

	etl::clock clk;
	int clk_scanline; // The scanline at which the clock was reset
	etl::clock clk2;

	float last_time;
public:

	RenderProgress():clk_scanline(0), last_time(0) { }

	virtual bool
	task(const String &thetask)
	{
		taskname=thetask;
		return true;
	}

	virtual bool
	error(const String &task)
	{
		std::cout<<_("error")<<": "<<task.c_str()<<std::endl;
		return true;
	}

	virtual bool
	warning(const String &task)
	{
		std::cout<<_("warning")<<": "<<task.c_str()<<std::endl;
		return true;
	}

	virtual bool
	amount_complete(int scanline, int h)
	{
		if(be_quiet)return true;
		if(scanline!=h)
		{
			const float time(clk()*(float)(h-scanline)/(float)(scanline-clk_scanline));
			const float delta(time-last_time);

			int weeks=0,days=0,hours=0,minutes=0,seconds=0;

			last_time=time;

			if(clk2()<0.2)
				return true;
			clk2.reset();

			if(scanline)
				seconds=(int)time+1;
			else
			{
				//cerr<<"reset"<<endl;
				clk.reset();
				clk_scanline=scanline;
			}

			if(seconds<0)
			{
				clk.reset();
				clk_scanline=scanline;
				seconds=0;
			}
			while(seconds>=60)
				minutes++,seconds-=60;
			while(minutes>=60)
				hours++,minutes-=60;
			while(hours>=24)
				days++,hours-=24;
			while(days>=7)
				weeks++,days-=7;

			cerr<<taskname.c_str()<<": "<<_("Line")<<" "<<scanline<<_(" of ")<<h<<" -- ";
			//cerr<<time/(h-clk_scanline)<<" ";
			/*
			if(delta>=-time/(h-clk_scanline)  )
				cerr<<">";
			*/
			if(delta>=0 && clk()>4.0 && scanline>clk_scanline+200)
			{
				//cerr<<"reset"<<endl;
				clk.reset();
				clk_scanline=scanline;
			}

			if(weeks)
				/// TRANSLATORS This "w" stands for weeks
				cerr<<weeks<<_("w ");
			if(days)
				/// TRANSLATORS This "d" stands for days
				cerr<<days<<_("d ");
			if(hours)
				/// TRANSLATORS This "h" stands for hours
				cerr<<hours<<_("h ");
			if(minutes)
				/// TRANSLATORS This "m" stands for minutes
				cerr<<minutes<<_("m ");
			if(seconds)
				/// TRANSLATORS This "s" stands for seconds
				cerr<<seconds<<_("s ");

			cerr<<"           \r";
		}
		else
			cerr<<taskname.c_str()<<": "<<_("DONE")<<"                        "<<endl;;
		return true;
	}
};

#endif
