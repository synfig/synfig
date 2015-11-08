/* === S Y N F I G ========================================================= */
/*!	\file layer_switch.cpp
**	\brief Implementation of the "Switch" layer
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

#include "layer_switch.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>


#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === C L A S S E S ======================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Switch);
SYNFIG_LAYER_SET_NAME(Layer_Switch,"switch");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Switch,N_("Switch"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Switch,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_Switch,"0.0");
SYNFIG_LAYER_SET_CVS_ID(Layer_Switch,"$Id$");

/* === M E T H O D S ======================================================= */

Layer_Switch::Layer_Switch()
{
	param_layer_name=ValueBase(String());
	set_param("children_lock",ValueBase(true));

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer_Switch::~Layer_Switch()
{
}

String
Layer_Switch::get_local_name()const
{
	String s = Layer_PasteCanvas::get_local_name();
	return s.empty() ? _("Switch") : _("Switch") + (" [" + s + ']');
}

Layer::Vocab
Layer_Switch::get_param_vocab()const
{
	Layer::Vocab ret(Layer_PasteCanvas::get_param_vocab());

	ret.push_back(ParamDesc("layer_name")
		.set_local_name(_("Active Layer Name"))
		.set_description(_("Only layer with specified name are visible"))
		.set_hint("sublayer_name")
	);

	return ret;
}

bool
Layer_Switch::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_layer_name);
	return Layer_PasteCanvas::set_param(param,value);
}

ValueBase
Layer_Switch::get_param(const String& param)const
{
	EXPORT_VALUE(param_layer_name);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_PasteCanvas::get_param(param);
}

Layer::Handle
Layer_Switch::get_current_layer()const
{
	Canvas::Handle canvas = get_sub_canvas();
	String n = param_layer_name.get(String());
	if (canvas)
		for(IndependentContext i = canvas->get_independent_context(); *i; i++)
			if ((*i)->get_description() == n)
				return *i;
	return NULL;
}


void
Layer_Switch::apply_z_range_to_params(ContextParams &cp)const
{
	if (optimized()) return; // z_range already applied while optimizxation

	Layer::Handle layer = get_current_layer();
	if (layer) {
		cp.z_range=true;
		cp.z_range_position=layer->get_depth();
		cp.z_range_depth=0;
		cp.z_range_blur=0;
		return;
	}

	cp.z_range=true;
	cp.z_range_position=0;
	cp.z_range_depth=-1;
	cp.z_range_blur=0;
}
