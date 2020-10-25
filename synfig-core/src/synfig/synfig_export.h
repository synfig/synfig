/* === S Y N F I G ========================================================= */
/*!	\file synfig/synfig_export.h
**	\brief Export definition for MSVC
**
**	\legal
**	......... ... 2020 Artem Konoplin
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

#ifndef __SYNFIG_EXPORT_H
#define __SYNFIG_EXPORT_H

#ifdef _MSC_VER
// We need this export only for MSVC. Even on MinGW, it breaks linkning.
#ifdef synfig_EXPORTS
/* We are building this library */
#define SYNFIG_EXPORT __declspec(dllexport)
#else
/* We are using this library */
#define SYNFIG_EXPORT __declspec(dllimport)
#endif
#else
#define SYNFIG_EXPORT
#endif

#endif