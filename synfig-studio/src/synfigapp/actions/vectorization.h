/* === S Y N F I G ========================================================= */
/*!	\file vectorization.h
**	\brief Vectorization Action File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_APP_ACTION_VECTORIZATION_H
#define __SYNFIG_APP_ACTION_VECTORIZATION_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/action.h>
#include <synfig/layer.h>
#include "vectorizer/centerlinevectorizer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Instance;

namespace Action {

class Vectorization :
	public Undoable,
	public CanvasSpecific
{
private:
	synfig::Layer::Handle layer,reference_layer, new_layer;
	synfig::String v_mode; 
    int threshold, penalty, despeckling, maxthickness;
    bool pparea, addborder,isOutline;
	studio::CenterlineConfiguration getCenterlineConfiguration() const;
  	studio::NewOutlineConfiguration getOutlineConfiguration() const;

  	studio::VectorizerConfiguration *getCurrentConfiguration() const {
    	return isOutline ? (studio::VectorizerConfiguration *)new studio::NewOutlineConfiguration(getOutlineConfiguration())
                       : (studio::VectorizerConfiguration *)new studio::CenterlineConfiguration(getCenterlineConfiguration());
  	}

       
public:

	Vectorization();
	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void perform();
	virtual void undo();

	ACTION_MODULE_EXT	
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
