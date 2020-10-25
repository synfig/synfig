/* === S Y N F I G ========================================================= */
/*!	\file mptr_ffmpeg.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_MPTR_FFMPEG_H
#define __SYNFIG_MPTR_FFMPEG_H

/* === H E A D E R S ======================================================= */

#include <synfig/importer.h>
#include <sys/types.h>
#include <cstdio>
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif

#include <synfig/surface.h>
/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class ffmpeg_mptr : public synfig::Importer
{
	SYNFIG_IMPORTER_MODULE_EXT
public:
private:
#ifdef HAVE_FORK
	pid_t pid = -1;
#endif
	FILE *file;
	int cur_frame;
	synfig::Surface frame;
	float fps;
#ifdef HAVE_TERMIOS_H
	struct termios oldtty;
#endif

	bool seek_to(const synfig::Time& time);
	bool grab_frame(void);

public:
	ffmpeg_mptr(const synfig::FileSystem::Identifier &identifier);
	~ffmpeg_mptr();

	virtual bool is_animated();

	virtual bool get_frame(synfig::Surface &surface, const synfig::RendDesc &renddesc, synfig::Time time, synfig::ProgressCallback *callback);
};

/* === E N D =============================================================== */

#endif
