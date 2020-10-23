/* === S Y N F I G ========================================================= */
/*!	\file main_win32.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2015 Konstantin Dmitriev
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

#ifndef MAIN_WIN32_H
#define	MAIN_WIN32_H

#ifdef _WIN32
void redirectIOToConsole();
bool consoleOptionEnabled(int argc, char* argv[]);
#endif /* WIN32 */

#endif	/* MAIN_WIN32_H */

