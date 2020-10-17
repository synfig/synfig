/* === S Y N F I G ========================================================= */
/*!	\file synfig/synfig_export.h
**	\brief Export definition for MSVC
**
**	$Id$
**
**	\legal
**	......... ... 2020 Artem Konoplin
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