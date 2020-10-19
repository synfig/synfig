/* === S Y N F I G ========================================================= */
/*!	\file freetime.h
**	\brief Header file for implementation of the "Free Time" layer
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_FREETIME_H
#define __SYNFIG_FREETIME_H

/* === H E A D E R S ======================================================= */

#include <synfig/layers/layer_invisible.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace modules
{
namespace lyr_std
{

class Layer_FreeTime : public Layer_Invisible
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//! Parameter: (Time)
	ValueBase param_time;

protected:
	Layer_FreeTime();

public:
	~Layer_FreeTime();

	virtual bool set_param(const String & param, const ValueBase &value);
	virtual ValueBase get_param(const String & param)const;
	virtual Vocab get_param_vocab()const;
	virtual void set_time_vfunc(IndependentContext context, Time time)const;
};

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
