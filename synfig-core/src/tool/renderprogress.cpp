/*!	\file tool/renderprogress.cpp
**	\brief Implementation of the functions from the render progress class
**
**	\legal
**	Copyright (c) 2014, 2015 Diego Barrios Romero
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

#include <cmath>
#include <iostream>
#include "renderprogress.h"
#include <ETL/stringf>
#include <sstream>

RenderProgress::RenderProgress()
    : last_frame_(0), last_printed_line_length_(0),
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

bool RenderProgress::amount_complete(int current_frame, int frames_count)
{
    if(SynfigToolGeneralOptions::instance()->should_be_quiet())
    {
        return true;
    }

    std::ostringstream outputStream;

    const bool isFinished = (current_frame == frames_count);
    if (!isFinished)
    {
        // avoid reporting the progress too often
        Duration time_since_last_call(Clock::now() - last_timepoint_);
        if (time_since_last_call.count() < 0.2)
        {
            return true;
        }
        last_timepoint_ = Clock::now();

        int percentage_completed = 100;
        if (frames_count > 0)
        {
            percentage_completed = 100 * current_frame / frames_count;
        }

        outputStream << "\r"
                     << etl::strprintf(_("%s: Frame %d of %d (%d%%). Remaining time: "), 
                        taskname_.c_str(), current_frame, frames_count, percentage_completed);

        if (current_frame != last_frame_)
        {
            remaining_rendered_proportion_ =
                double(frames_count-current_frame)/(current_frame-last_frame_);
        }
        Duration time_since_start(Clock::now() - start_timepoint_);
        double remaining_seconds =
            time_since_start.count() * remaining_rendered_proportion_;

        printRemainingTime(outputStream, remaining_seconds);
    }
    else
    {
        outputStream << "\r" << taskname_ << ": " << _("DONE");
    }

    const std::string line = outputStream.str();
    const std::string extendedLine =
        extendLineToClearRest(line, last_printed_line_length_);
    last_printed_line_length_ = line.size();

    std::cerr << extendedLine;
    if (isFinished)
    {
        std::cerr << std::endl;
    }

    return true;
}

void RenderProgress::printRemainingTime(std::ostream& os,
                                        double remaining_seconds) const
{
    int weeks, days, hours, minutes, seconds;

    seconds = remaining_seconds;

    minutes = floor(seconds/60);
    seconds %= 60;

    hours = floor(minutes/60);
    minutes %= 60;

    days = floor(hours/24);
    hours %= 24;

    weeks = floor(days/7);
    days %= 7;

    printRemainingTime(os, seconds, minutes, hours, days, weeks);
}

void RenderProgress::printRemainingTime(std::ostream& os,
                                        const int seconds, const int minutes,
                                        const int hours, const int days,
                                        const int weeks) const
{
    if(weeks != 0)
    {
        /// TRANSLATORS This "w" stands for weeks
        os << weeks << _("w ");
    }
    if(days != 0)
    {
        /// TRANSLATORS This "d" stands for days
        os << days << _("d ");
    }
    if(hours != 0)
    {
        /// TRANSLATORS This "h" stands for hours
        os << hours << _("h ");
    }
    if(minutes != 0)
    {
        /// TRANSLATORS This "m" stands for minutes
        os << minutes << _("m ");
    }
    /// TRANSLATORS This "s" stands for seconds
    os << seconds << _("s ");
}

std::string RenderProgress::extendLineToClearRest(std::string line,
                                                  size_t last_line_length) const
{
    if (line.size() < last_line_length)
    {
        line.append(last_line_length - line.size(), ' ');
    }

    return line;
}
