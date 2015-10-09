/* === S Y N F I G ========================================================= */
/*!	\file layer_sound.cpp
**	\brief Implementation of the "Sound" layer
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

#include "layer_sound.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/real.h>
#include <synfig/soundprocessor.h>
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/value.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Sound);
SYNFIG_LAYER_SET_NAME(Layer_Sound,"sound");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Sound,N_("Sound"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Sound,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_Sound,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_Sound,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Layer_Sound::Layer_Sound():
	Layer_Composite(0.0),
	param_filename(String()),
	param_delay(Time()),
	param_volume(Real(1.0))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

bool
Layer_Sound::set_param(const String &param, const ValueBase &value)
{
	IMPORT_VALUE(param_filename);
	IMPORT_VALUE(param_delay);
	IMPORT_VALUE(param_volume);

	return Layer::set_param(param,value);
}

ValueBase
Layer_Sound::get_param(const String &param)const
{
	EXPORT_VALUE(param_filename);
	EXPORT_VALUE(param_delay);
	EXPORT_VALUE(param_volume);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer::get_param(param);
}

Layer::Vocab
Layer_Sound::get_param_vocab()const
{
	Layer::Vocab ret(Layer::get_param_vocab());

	ret.push_back(ParamDesc("filename")
		.set_local_name(_("Filename"))
		.set_description(_("Path to sound file"))
		.set_static(true)
		.set_hint("filename")
	);

	ret.push_back(ParamDesc("delay")
		.set_local_name(_("Delay"))
		.set_description(_("Delay before play"))
		.set_static(true)
	);

	ret.push_back(ParamDesc("volume")
		.set_local_name(_("Volume"))
		.set_description(_("Volume of sound"))
		.set_static(true)
	);

	return ret;
}

void
Layer_Sound::fill_sound_processor(SoundProcessor &soundProcessor) const
{
	String filename = param_filename.get(String());
	Time delay = param_delay.get(Time());
	Real volume = param_volume.get(Real());
	if (!filename.empty())
		soundProcessor.addSound(SoundProcessor::PlayOptions(delay, volume), SoundProcessor::Sound(filename));
}

