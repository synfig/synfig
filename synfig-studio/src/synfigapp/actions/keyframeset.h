/* === S Y N F I G ========================================================= */
/*!	\file keyframeset.h
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_APP_ACTION_KEYFRAMESET_H
#define __SYNFIG_APP_ACTION_KEYFRAMESET_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/action.h>
#include <synfig/keyframe.h>
#include <synfig/time.h>
#include <synfig/guid.h>
#include <set>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Instance;

namespace Action {

class KeyframeSet :
	public Super
{
private:

	synfig::Keyframe keyframe;
	synfig::Keyframe old_keyframe;
	synfig::Time old_time;
	synfig::Time new_time;

	synfig::Time keyframe_prev,keyframe_next;

	std::set<synfig::GUID> guid_set;

	void process_value_desc(const synfigapp::ValueDesc& value_desc);

	int scale_activepoints(const synfigapp::ValueDesc& value_desc,const synfig::Time& old_begin,const synfig::Time& old_end,const synfig::Time& new_begin,const synfig::Time& new_end);
	int scale_waypoints(const synfigapp::ValueDesc& value_desc,const synfig::Time& old_begin,const synfig::Time& old_end,const synfig::Time& new_begin,const synfig::Time& new_end);

public:

	KeyframeSet();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();
	virtual void perform();
	virtual void undo();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
