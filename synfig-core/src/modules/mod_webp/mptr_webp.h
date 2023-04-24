/* === S Y N F I G ========================================================= */
/*!	\file mptr_webp.h
**	\brief Header for WEBP (via FFMPEG) Importer (webp_mptr)
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
 *	Copyright (c) 2022 BobSynfig
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
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_MPTR_WEBP_H
#define __SYNFIG_MPTR_WEBP_H

/* === H E A D E R S ======================================================= */

#include <synfig/importer.h>
#include <synfig/os.h>
#include <synfig/surface.h>

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class webp_mptr : public synfig::Importer
{
	SYNFIG_IMPORTER_MODULE_EXT
private:
	synfig::OS::RunPipe::Handle pipe;
	int                         cur_frame;
	synfig::Surface             frame;
	float                       fps;
#ifdef HAVE_TERMIOS_H
	struct termios oldtty;
#endif

	bool seek_to   (const synfig::Time& time);
	bool grab_frame(void);

public:
	webp_mptr(const synfig::FileSystem::Identifier &identifier);
	~webp_mptr();

	virtual bool is_animated();

	virtual bool get_frame(      synfig::Surface          &surface,
	                       const synfig::RendDesc         &renddesc,
	                             synfig::Time              time,
	                             synfig::ProgressCallback *callback);
};

/* === E N D =============================================================== */

#endif
