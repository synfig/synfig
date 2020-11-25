/* === S Y N F I G ========================================================= */
/*!	\file layer_filtergroup.cpp
**	\brief Implementation of the "Filter Group" layer
**
**	$Id$
**
**	\legal
**	......... ... 2017 Ivan Mahonin
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

#include "layer_filtergroup.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/paramdesc.h>
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

SYNFIG_LAYER_INIT(Layer_FilterGroup);
SYNFIG_LAYER_SET_NAME(Layer_FilterGroup,"filter_group");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_FilterGroup,N_("Filter Group"));
SYNFIG_LAYER_SET_CATEGORY(Layer_FilterGroup,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_FilterGroup,"0.1");

/* === M E T H O D S ======================================================= */

Layer_FilterGroup::Layer_FilterGroup():
	Layer_PasteCanvas(1.0, Color::BLEND_STRAIGHT)
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

String
Layer_FilterGroup::get_local_name()const
{
	String s = Layer_PasteCanvas::get_local_name();
	return s.empty() ? _("Filter Group") : '[' + s + ']';
}

Layer::Vocab
Layer_FilterGroup::get_param_vocab()const
{
	Layer::Vocab vocab(Layer_PasteCanvas::get_param_vocab());

	String hide[] = {
		"time_dilation",
		"time_offset",
		"children_lock",
		"outline_grow" };
	for(Layer::Vocab::iterator i = vocab.begin(); i != vocab.end(); ++i)
		for(int j = 0; j < (int)(sizeof(hide)/sizeof(hide[0])); ++j)
			if (i->get_name() == hide[j])
				i->hidden();

	return vocab;
}

ValueBase
Layer_FilterGroup::get_param(const String& param)const
{
	EXPORT_NAME();
	EXPORT_VERSION();
	return Layer_PasteCanvas::get_param(param);
}

Context
Layer_FilterGroup::build_context_queue(Context context, CanvasBase &out_queue)const
{
	if (!get_sub_canvas())
		return context;
	if (!*context)
		return get_sub_canvas()->get_context_sorted(context.get_params(), out_queue);

	CanvasBase sub_queue;
	for(Context c = get_sub_canvas()->get_context_sorted(context.get_params(), sub_queue); *c; ++c)
		out_queue.push_back(*c);
	for(Context c = context; *c; ++c)
		out_queue.push_back(*c);
	out_queue.push_back(Layer::Handle());
	return Context(out_queue.begin(), context.get_params());
}
