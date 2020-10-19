/* === S Y N F I G ========================================================= */
/*!	\file stroboscope.cpp
**	\brief Implementation of the "Stroboscope" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009 Ray Frederikson
**	Copyright (c) 2012-2013 Carlos López
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

#include "stroboscope.h"

#include <synfig/localization.h>

#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/value.h>

#endif

using namespace etl;
using namespace synfig;
using namespace modules;
using namespace lyr_std;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Stroboscope);
SYNFIG_LAYER_SET_NAME(Layer_Stroboscope,"stroboscope");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Stroboscope,N_("Stroboscope"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Stroboscope,N_("Time"));
SYNFIG_LAYER_SET_VERSION(Layer_Stroboscope,"0.1");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Layer_Stroboscope::Layer_Stroboscope()
{
	param_frequency=ValueBase(float(2.0));
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer_Stroboscope::~Layer_Stroboscope()
{
}

bool
Layer_Stroboscope::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE(param_frequency);

	return Layer::set_param(param,value);
}

ValueBase
Layer_Stroboscope::get_param(const String & param)const
{
	EXPORT_VALUE(param_frequency);
	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer::get_param(param);
}

Layer::Vocab
Layer_Stroboscope::get_param_vocab()const
{
	Layer::Vocab ret(Layer::get_param_vocab());

	ret.push_back(ParamDesc("frequency")
		.set_local_name(_("Frequency"))
		.set_description(_("Frequency of the Strobe in times per second"))
	);

	return ret;
}

void
Layer_Stroboscope::set_time_vfunc(IndependentContext context, Time t)const
{
	float frequency=param_frequency.get(float());
	
	Time ret_time=Time::begin();
	if(frequency > 0.0)
		ret_time = Time(1.0)/frequency*floor(t*frequency);

	context.set_time(ret_time);
}

