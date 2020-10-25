/* === S Y N F I G ========================================================= */
/*!	\file tool/joblistprocessor.h
**	\brief Synfig Tool Rendering Job List Processor Class
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2012 Diego Barrios Romero
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

#ifndef __SYNFIG_JOBLISTPROCESSOR_H
#define __SYNFIG_JOBLISTPROCESSOR_H

#include <list>
#include <synfig/targetparam.h>
#include "job.h"

/// Process a Job list setting up and processing each job
void process_job_list(std::list<Job>& job_list,
						const synfig::TargetParam& target_parameters);

/// Prepare a job to be processed
/// \return whether the preparation was OK or not
bool setup_job(Job& job, const synfig::TargetParam& target_parameters);

/// Process an individual job
void process_job(Job& job);

std::string get_absolute_path(std::string relative_path);

#endif // __SYNFIG_JOBLISTPROCESSOR_H
