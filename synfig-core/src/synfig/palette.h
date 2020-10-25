/* === S Y N F I G ========================================================= */
/*!	\file palette.h
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_PALETTE_H
#define __SYNFIG_PALETTE_H

/* === H E A D E R S ======================================================= */

#include "color.h"
#include "string.h"
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Surface;

struct PaletteItem
{
	Color color;
	String name;
	int weight;

	PaletteItem():weight(1) { }

	PaletteItem(const Color& color, const String& name, int weight=1):
		color(color),name(name),weight(weight) { }

	PaletteItem(const Color& color, int weight=1):
		color(color),weight(weight) { }

	void add(const Color& x, int weight=1);

	bool operator<(const PaletteItem& rhs)const { return weight<rhs.weight; }
}; // END of struct PaletteItem

class Palette : public std::vector<PaletteItem>
{
	String name_;

public:
	Palette();
	Palette(const String& name_);

	/*! Generates a palette for the given
	**	surface
	*/
	Palette(const Surface& surface, int size, const Gamma &gamma);

	iterator find_closest(const Color& color, const Gamma &gamma, float* dist = 0);
	const_iterator find_closest(const Color& color, const Gamma &gamma, float* dist = 0)const;

	iterator find_heavy();

	iterator find_light();

	static Palette grayscale(int steps, ColorReal gamma);

	void save_to_file(const synfig::String& filename)const;

	static Palette load_from_file(const synfig::String& filename);
}; // END of class Palette

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
