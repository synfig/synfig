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

#include <cmath>

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include "layer_switch.h"

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
	param_layer_depth=ValueBase(int(-1));
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

	ret.push_back(ParamDesc("layer_depth")
		.set_local_name(_("Active Layer Depth"))
		.set_description(_("Uses when layer name is empty. Only layer with specified depth are visible"))
	);

	return ret;
}

bool
Layer_Switch::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_layer_name);
	IMPORT_VALUE(param_layer_depth);
	return Layer_PasteCanvas::set_param(param,value);
}

ValueBase
Layer_Switch::get_param(const String& param)const
{
	EXPORT_VALUE(param_layer_name);
	EXPORT_VALUE(param_layer_depth);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_PasteCanvas::get_param(param);
}

Layer::Handle
Layer_Switch::get_current_layer()const
{
	Canvas::Handle canvas = get_sub_canvas();
	if (canvas) {
		String name = param_layer_name.get(String());
		if (name.empty()) {
			int depth = param_layer_depth.get(int());
			for(IndependentContext i = canvas->get_independent_context(); *i; i++)
				if ((*i)->get_depth() == depth)
					return *i;
		} else {
			for(IndependentContext i = canvas->get_independent_context(); *i; i++)
				if ((*i)->get_description() == name)
					return *i;
		}
	}
	return Layer::Handle();
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

void
Layer_Switch::possible_layers_changed()
{
	on_possible_layers_changed();
	signal_possible_layers_changed_();
}

void
Layer_Switch::on_childs_changed()
{
	Layer_PasteCanvas::on_childs_changed();
	std::set<String> a(last_existant_layers), b;
	get_existant_layers(b);
	if (a != b)
		possible_layers_changed();
}

void
Layer_Switch::on_static_param_changed(const String &param)
{
	Layer_PasteCanvas::on_static_param_changed(param);
	if (param == "layer_name")
	{
		std::set<String> a(last_possible_layers), b;
		get_possible_layers(b);
		if (a != b)
			possible_layers_changed();
	}
}

void
Layer_Switch::on_dynamic_param_changed(const String &param)
{
	Layer_PasteCanvas::on_dynamic_param_changed(param);
	if (param == "layer_name")
	{
		std::set<String> a(last_possible_layers), b;
		get_possible_layers(b);
		if (a != b)
			possible_layers_changed();
	}
}

void
Layer_Switch::get_existant_layers(std::set<String> &x) const
{
	if (!get_sub_canvas()) return;
	for(IndependentContext i = get_sub_canvas()->get_independent_context(); *i; ++i)
		x.insert((*i)->get_description());
	last_existant_layers = x;
}

void
Layer_Switch::get_possible_layers(std::set<String> &x) const
{
	if (dynamic_param_list().count("layer_name"))
	{
		std::set<ValueBase> v;
		dynamic_param_list().find("layer_name")->second->get_values(v);
		for(std::set<ValueBase>::const_iterator i = v.begin(); i != v.end(); ++i)
			if (!i->get(String()).empty())
				x.insert(i->get(String()));
	}
	else
	{
		if (!param_layer_name.get(String()).empty())
			x.insert(param_layer_name.get(String()));
	}
	last_possible_layers = x;
}

void
Layer_Switch::get_possible_new_layers(std::set<String> &x) const
{
	std::set<String> possible;
	get_possible_layers(possible);

	std::set<String> existant;
	get_existant_layers(existant);

	for(std::set<String>::const_iterator i = possible.begin(); i != possible.end(); ++i)
		if (!existant.count(*i)) x.insert(*i);
}

void
Layer_Switch::get_impossible_existant_layers(std::set<String> &x) const
{
	std::set<String> possible;
	get_possible_layers(possible);

	std::set<String> existant;
	get_existant_layers(existant);

	for(std::set<String>::const_iterator i = existant.begin(); i != existant.end(); ++i)
		if (!possible.count(*i)) x.insert(*i);
}

