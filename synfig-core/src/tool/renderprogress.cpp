/*!	\file tool/renderprogress.cpp
**	\brief Implementation of the functions from the render progress class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2014 Diego Barrios Romero
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

#include "renderprogress.h"

RenderProgress::RenderProgress()
    : last_scanline_(0),
      start_timepoint_(Clock::now()), last_timepoint_(Clock::now())
{ }

bool RenderProgress::task(const std::string& taskname)
{
    taskname_ = taskname;
    return true;
}

bool RenderProgress::error(const std::string& task)
{
    std::cout << _("error") << ": " << task << std::endl;
    return true;
}

bool RenderProgress::warning(const std::string& task)
{
    std::cout << _("warning") << ": " << task << std::endl;
    return true;
}

bool RenderProgress::amount_complete(int scanline, int height)
{
    if(SynfigToolGeneralOptions::instance()->should_be_quiet())
    {
        return true;
    }

    if(scanline != height)
    {
        // avoid reporting the progress too often
        Duration time_since_last_call(Clock::now() - last_timepoint_);
        if (time_since_last_call.count() < 0.2)
        {
            return true;
        }
        last_timepoint_ = Clock::now();

        std::cerr << "\r"
                  << taskname_ << ": " << _("Line") << " "
                  << scanline << _(" of ") << height << ". "
                  << _("Remaining time: ");

        if (scanline != last_scanline_)
        {
            remaining_rendered_proportion_ =
                double(height-scanline)/(scanline-last_scanline_);
        }
        Duration time_since_start(Clock::now() - start_timepoint_);
        double remaining_seconds =
            time_since_start.count() * remaining_rendered_proportion_;

        printRemainingTime(remaining_seconds);
    }
    else
        std::cerr << "\r" << taskname_ << ": " << _("DONE")
                  << std::endl;
    return true;
}

void RenderProgress::printRemainingTime(double remaining_seconds)
{
    int weeks=0,days=0,hours=0,minutes=0,seconds=0;

    seconds = remaining_seconds;

    while(seconds>=60)
        minutes++,seconds-=60;
    while(minutes>=60)
        hours++,minutes-=60;
    while(hours>=24)
        days++,hours-=24;
    while(days>=7)
        weeks++,days-=7;

    printRemainingTime(seconds, minutes, hours, days, weeks);
}

void RenderProgress::printRemainingTime(const int seconds, const int minutes,
                                         const int hours, const int days,
                                         const int weeks)
{
    if(weeks != 0)
    {
        /// TRANSLATORS This "w" stands for weeks
        std::cerr << weeks << _("w ");
    }
    if(days != 0)
    {
        /// TRANSLATORS This "d" stands for days
        std::cerr << days << _("d ");
    }
    if(hours != 0)
    {
        /// TRANSLATORS This "h" stands for hours
        std::cerr << hours << _("h ");
    }
    if(minutes != 0)
    {
        /// TRANSLATORS This "m" stands for minutes
        std::cerr << minutes << _("m ");
    }
    /// TRANSLATORS This "s" stands for seconds
    std::cerr << seconds << _("s ");
}
