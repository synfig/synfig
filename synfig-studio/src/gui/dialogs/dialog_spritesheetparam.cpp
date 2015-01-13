/* === S Y N F I G ========================================================= */
/*!	\file dialog_spritesheetparam.cpp
**	\brief Implementation for the SpriteSheetParam Dialog
**
**	$Id$
**
**	\legal
**	Copyright (c) 2015 Denis Zdorovtsov
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

/* === H E A D E R S ======================================================= */
#include "dialog_spritesheetparam.h"
#include "general.h"

namespace studio
{

Dialog_SpriteSheetParam::Dialog_SpriteSheetParam(Gtk::Window &parent):
	Dialog_TargetParam(parent, _("Sprite sheet parameters"))
{

}

Dialog_SpriteSheetParam::~Dialog_SpriteSheetParam()
{
}

void
Dialog_SpriteSheetParam::init()
{

}

void
Dialog_SpriteSheetParam::write_tparam(synfig::TargetParam & tparam_)
{

}

}