/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_interpolation.h
**	\brief Widget for interpolation selection
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2022      Rodolfo R. Gomes
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

#ifndef SYNFIG_STUDIO_WIDGET_INTERPOLATION_H
#define SYNFIG_STUDIO_WIDGET_INTERPOLATION_H

/* === H E A D E R S ======================================================= */

#include <gui/widgets/widget_enum.h>

namespace studio {

/// A combobox that shows supported Interpolation types
class Widget_Interpolation : public studio::Widget_Enum
{
public:
	enum Side {SIDE_BEFORE, SIDE_AFTER, SIDE_BOTH};
	/// \param side What side of waypoint the interpolation affects
	Widget_Interpolation(Side side);

protected:
	void set_icons();
};

}

#endif // WIDGET_INTERPOLATION_H
