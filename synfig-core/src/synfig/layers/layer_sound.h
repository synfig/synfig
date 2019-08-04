/* === S Y N F I G ========================================================= */
/*!	\file layer_sound.h
**	\brief Header file for implementation of the "Sound" layer
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_LAYER_SOUND_H
#define __SYNFIG_LAYER_SOUND_H

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include <synfig/color.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Layer_Sound : public Layer_Composite, public Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT
private:
	ValueBase param_filename;
	ValueBase param_delay;
	ValueBase param_volume;

public:
	Layer_Sound();
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	virtual ValueBase get_param(const String & param)const;
	virtual Vocab get_param_vocab()const;
	virtual void fill_sound_processor(SoundProcessor &soundProcessor) const;
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context)const;
}; // END of class Layer_SolidColor

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
