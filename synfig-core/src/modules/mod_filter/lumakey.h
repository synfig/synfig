/* === S Y N F I G ========================================================= */
/*!	\file lumakey.h
**	\brief Header file for implementation of the "Luma Key" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_LUMAKEY_H
#define __SYNFIG_LUMAKEY_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/rendering/common/task/taskpixelprocessor.h>
#include <synfig/rendering/software/task/tasksw.h>
#include <synfig/rendering/opengl/task/taskgl.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;

/// The pixel luma (Y) changes its alpha channel
/// In other words, LumaKey converts a [r g b a] pixel into [r' g' b' a*y]
class TaskLumaKey: public rendering::TaskPixelProcessor
{
public:
	typedef etl::handle<TaskLumaKey> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	TaskLumaKey();

protected:
	/// An intermediate matrix to help computation
	/// matrix * pixel results in [r' g' b' a Y]
	/// Implementations must perform the a*Y computation
	ColorMatrix matrix;
};


class TaskLumaKeySW: public TaskLumaKey, public rendering::TaskSW
{
public:
	typedef etl::handle<TaskLumaKeySW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual bool run(RunParams &params) const;
};

class TaskLumaKeyGL: public TaskLumaKey, public rendering::TaskGL
{
public:
	typedef etl::handle<TaskLumaKeyGL> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual bool run(RunParams &params) const;
};

class LumaKey : public Layer
{
	SYNFIG_LAYER_MODULE_EXT
private:

public:
	LumaKey();

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual Rect get_bounding_rect(Context context)const;

	virtual bool reads_context()const { return true; }

protected:
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context) const;
}; // END of class LumaKey

/* === E N D =============================================================== */

#endif
