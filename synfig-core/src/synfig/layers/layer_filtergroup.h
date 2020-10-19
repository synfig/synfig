/* === S Y N F I G ========================================================= */
/*!	\file layer_filtergroup.h
**	\brief Header file for implementation of the "Filter Group" layer
**
**	$Id$
**
**	\legal
**	......... ... 2017 Ivan Mahonin
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

#ifndef __SYNFIG_LAYER_FILTERGROUP_H
#define __SYNFIG_LAYER_FILTERGROUP_H

/* === H E A D E R S ======================================================= */

#include "layer_pastecanvas.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
/*!	\class Layer_FilterGroup
**	\brief Class of the Filter Group layer.
*/
class Layer_FilterGroup : public Layer_PasteCanvas
{
	//! Layer module: defines the needed members to belong to a layer's factory.
	SYNFIG_LAYER_MODULE_EXT

public:
	//! Default constructor
	Layer_FilterGroup();
	//! Returns a string with the localized name of this layer
	virtual String get_local_name()const;
	//! Gets the parameter vocabulary
	virtual Vocab get_param_vocab()const;
	//! Get the value of the specified parameter. \see Layer::get_param
	virtual ValueBase get_param(const String & param)const;

protected:
	virtual Context build_context_queue(Context context, CanvasBase &queue)const;
}; // END of class Layer_FilterGroup

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
