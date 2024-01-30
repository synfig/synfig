/* === S Y N F I G ========================================================= */
/*!	\file mptr_magickpp.h
**	\brief Header for Magick++ Importer (magickpp_mptr)
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2024      Synfig Contributors
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

#ifndef SYNFIG_MPTR_MAGICKPP_H
#define SYNFIG_MPTR_MAGICKPP_H

/* === H E A D E R S ======================================================= */

#include <synfig/importer.h>
#include <synfig/surface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

/**
 * Import static images, animated images or, possibly, videos
 * by using Magick++ library
 */
class magickpp_mptr : public synfig::Importer
{
	SYNFIG_IMPORTER_MODULE_EXT

public:
	magickpp_mptr(const synfig::FileSystem::Identifier& identifier);

	~magickpp_mptr();

	bool is_animated() override;

	bool get_frame(synfig::Surface& surface, const synfig::RendDesc& renddesc, synfig::Time time, synfig::ProgressCallback* callback) override;

private:
	// Info for animations
	/** number of repetitions. Zero means infinity */
	ssize_t animation_repetitions_;
	/** Initial time of each frame */
	std::vector<synfig::Time> frame_time_list_;
	/** Total duration of an animation cycle */
	synfig::Time animation_length_;
};

/* === E N D =============================================================== */

#endif
