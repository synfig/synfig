/* === S Y N F I G ========================================================= */
/*!	\file layerpaint.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "layerpaint.h"
#include <synfigapp/canvasinterface.h>
#include <synfigapp/general.h>
#include <synfig/layer_pastecanvas.h>
#include <synfig/valuenode_composite.h>

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT(Action::LayerPaint);
ACTION_SET_NAME(Action::LayerPaint,"LayerPaint");
ACTION_SET_LOCAL_NAME(Action::LayerPaint,N_("Paint"));
ACTION_SET_TASK(Action::LayerPaint,"paint");
ACTION_SET_CATEGORY(Action::LayerPaint,Action::CATEGORY_NONE);
ACTION_SET_PRIORITY(Action::LayerPaint,0);
ACTION_SET_VERSION(Action::LayerPaint,"0.0");
ACTION_SET_CVS_ID(Action::LayerPaint,"$Id$");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerPaint::LayerPaint():
	applied(false)
{
}

Action::ParamVocab
Action::LayerPaint::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());
	return ret;
}

bool
Action::LayerPaint::is_candidate(const ParamList &x)
{
	return false;
}

bool
Action::LayerPaint::set_param(const synfig::String& name, const Action::Param &param)
{
	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::LayerPaint::is_ready()const
{
	if(!layer)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerPaint::mark_as_already_applied(
	etl::handle<synfig::Layer_Bitmap> layer,
	const synfig::Surface &undo_surface,
	const synfig::Point &undo_tl,
	const synfig::Point &undo_br )
{
	assert(!applied);

	Mutex::Lock lock(layer->mutex);

	this->layer = layer;

	this->undo_surface.set_wh(undo_surface.get_w(), undo_surface.get_h());
	this->undo_surface.copy(undo_surface);
	this->undo_tl = undo_tl;
	this->undo_br = undo_br;

	surface.set_wh(layer->surface.get_w(), layer->surface.get_h());
	surface.copy(layer->surface);
	tl = layer->get_param("tl").get(Point());
	br = layer->get_param("br").get(Point());

	applied = true;
}

void
Action::LayerPaint::perform()
{
	if (!applied)
	{
		{
			Mutex::Lock lock(layer->mutex);
			layer->surface.set_wh(surface.get_w(), surface.get_h());
			layer->surface.copy(surface);
		}
		layer->changed();
		layer->set_param("tl", tl);
		layer->set_param("br", br);
		applied = true;
	}
}

void
Action::LayerPaint::undo()
{
	applied = false;
	{
		Mutex::Lock lock(layer->mutex);
		layer->surface.set_wh(undo_surface.get_w(), undo_surface.get_h());
		layer->surface.copy(undo_surface);
	}
	layer->changed();
	layer->set_param("tl", undo_tl);
	layer->set_param("br", undo_br);
}
