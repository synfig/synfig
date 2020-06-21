/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_interpolationenum.h
**	\brief Widget for interpolation selection
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**            (c) 2020 Rodolfo Ribeiro Gomes
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
/* ========================================================================= */

#ifndef SYNFIG_STUDIO_WIDGET_INTERPOLATIONENUM_H
#define SYNFIG_STUDIO_WIDGET_INTERPOLATIONENUM_H

#include <gui/widgets/widget_enum.h>

namespace studio {

class Widget_InterpolationEnum : public Widget_Enum
{
public:
	enum Side {SIDE_BEFORE, SIDE_AFTER, SIDE_BOTH};
	Widget_InterpolationEnum(Side side);

protected:
	void set_icons();
};

}
#endif // SYNFIG_STUDIO_WIDGET_INTERPOLATIONENUM_H
