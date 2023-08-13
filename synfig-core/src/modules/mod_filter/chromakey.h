/* === S Y N F I G ========================================================= */
/*!	\file chromakey.h
**	\brief Header file for implementation of the "Chroma Key" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2022 Rodolfo Ribeiro Gomes
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
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_CHROMAKEY_H_H
#define __SYNFIG_CHROMAKEY_H_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/rendering/common/task/taskpixelprocessor.h>
#include <synfig/rendering/software/task/tasksw.h>
#include <synfig/rendering/opengl/task/taskgl.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

class TaskChromaKey: public rendering::TaskPixelProcessor
{
public:
	typedef etl::handle<TaskChromaKey> Handle;
	SYNFIG_EXPORT static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	Color key_color;
	Real lower_bound;
	Real upper_bound;
	bool desaturate;

	TaskChromaKey();

	bool is_transparent() const;
};


class TaskChromaKeySW: public TaskChromaKey, public rendering::TaskSW
{
public:
	typedef etl::handle<TaskChromaKeySW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual bool run(RunParams &params) const;
};

class TaskChromaKeyGL: public TaskChromaKey, public rendering::TaskGL
{
public:
	typedef etl::handle<TaskChromaKeyGL> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual bool run(RunParams &params) const;
};

class ChromaKey : public Layer
{
	SYNFIG_LAYER_MODULE_EXT
private:

	//!Parameter: (Color) the key color to be masked out
	ValueBase param_key_color;
	//!Parameter: (Real) below this value, the pixel become fully transparent
	ValueBase param_lower_bound;
	//!Parameter: (Real) above this value, the color is kept as it is
	ValueBase param_upper_bound;
	//!Parameter (int)
	ValueBase param_supersample_width, param_supersample_height;
	//!Parameter (bool) dessaturate the color between lower and upper bound
	ValueBase param_desaturate;

public:
	ChromaKey();

	virtual bool set_param(const String & param, const ValueBase &value);
	virtual ValueBase get_param(const String & param) const;
	virtual Vocab get_param_vocab() const;

	virtual Color get_color(Context context, const Point &pos) const;

	virtual Rect get_bounding_rect(Context context) const;

	virtual bool reads_context() const { return true; }

protected:
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context) const;
}; // END of class ChromaKey

}

#endif // __SYNFIG_CHROMAKEY_H

/* === E N D =============================================================== */
