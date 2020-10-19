/* === S Y N F I G ========================================================= */
/*!	\file tool/renderprogress.h
**	\brief RenderProgress class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2014, 2015 Diego Barrios Romero
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

#ifndef __SYNFIG_RENDERPROGRESS_H
#define __SYNFIG_RENDERPROGRESS_H

#include <string>
#include <iosfwd>
#include <chrono>
#include <synfig/progresscallback.h>
#include "definitions.h"


//! Prints the progress and estimated time left to the console
class RenderProgress : public synfig::ProgressCallback
{
public:

    RenderProgress();

    virtual bool task(const std::string& taskname);

    virtual bool error(const std::string& task);

    virtual bool warning(const std::string& task);

    virtual bool amount_complete(int scanline, int height);
private:
    std::string taskname_;
    int last_frame_;
    size_t last_printed_line_length_;

    typedef std::chrono::system_clock Clock;
    typedef std::chrono::duration<double> Duration;
    Clock::time_point start_timepoint_;
    Clock::time_point last_timepoint_;
    double remaining_rendered_proportion_;

    void printRemainingTime(std::ostream& os, double remaining_seconds) const;

    void printRemainingTime(std::ostream& os,
                            const int seconds, const int minutes,
                            const int hours, const int days,
                            const int weeks) const;
    std::string extendLineToClearRest(std::string line,
                                      size_t last_line_length) const;
};

#endif
