/* === S Y N F I G ========================================================= */
/*!	\file freetimr.cpp
**	\brief Implementation of the "Free Time" layer
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/localization.h>

#include <synfig/context.h>
#include <synfig/value.h>
#include <synfig/paramdesc.h>

#include "freetime.h"

#endif

using namespace std;
using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_FreeTime);
SYNFIG_LAYER_SET_NAME(Layer_FreeTime,"freetime");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_FreeTime,N_("Free Time"));
SYNFIG_LAYER_SET_CATEGORY(Layer_FreeTime,N_("Time"));
SYNFIG_LAYER_SET_VERSION(Layer_FreeTime,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Layer_FreeTime::Layer_FreeTime()
{
	param_time=ValueBase(Time(0));

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer_FreeTime::~Layer_FreeTime()
	{ }

bool
Layer_FreeTime::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE(param_time);
	return Layer::set_param(param,value);
}

ValueBase
Layer_FreeTime::get_param(const String & param)const
{
	EXPORT_VALUE(param_time);
	EXPORT_NAME();
	EXPORT_VERSION();
	return Layer::get_param(param);
}

Layer::Vocab
Layer_FreeTime::get_param_vocab()const
{
	Layer::Vocab ret(Layer::get_param_vocab());

	ret.push_back(ParamDesc("time")
		.set_local_name(_("Time"))
		.set_description(_("Current time for next layers"))
	);

	return ret;
}

void
Layer_FreeTime::set_time_vfunc(IndependentContext context, Time /* t */)const
{
	context.set_time( param_time.get(Time()) );
}

