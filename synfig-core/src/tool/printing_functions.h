/* === S Y N F I G ========================================================= */
/*!	\file tool/printing_functions.h
**	\brief Printing functions
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2014 Diego Barrios Romero
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

#ifndef __SYNFIG_PRINTING_FUNCTIONS_H
#define __SYNFIG_PRINTING_FUNCTIONS_H

void print_usage();

/// Print canvases' children IDs in cascade
void print_child_canvases(const std::string& prefix,
						  const synfig::Canvas::Handle& canvas);

void print_canvas_info(const Job& job);

#endif // __SYNFIG_PRINTING_FUNCTIONS_H
