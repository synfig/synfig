/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/task.cpp
**	\brief Task
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#include <synfig/general.h>

#include "task.h"
#include "renderer.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


synfig::Token Mode::mode_token;
SYNFIG_EXPORT synfig::Token Task::token;

SYNFIG_EXPORT const Task::Handle Task::blank;

SYNFIG_EXPORT Task::Token TaskSurface::token(
	DescSpecial<TaskSurface>("Surface") );
Task::Token TaskLockSurface::token(
	DescSpecial<TaskLockSurface>("LoskSurface") );
Task::Token TaskList::token(
	DescSpecial<TaskList>("List") );
SYNFIG_EXPORT Task::Token TaskEvent::token(
	DescSpecial<TaskEvent>("Event") );


// Task

void Task::Token::unprepare_vfunc()
	{ alternatives_.clear(); }

void
Task::Token::prepare_vfunc()
{
	assert( is_abstract()
		  ? !mode && !abstract_task && !convert
		  : mode && abstract_task && abstract_task->is_abstract() && convert );
	if (!is_abstract() && abstract_task->is_abstract()) {
		assert(!abstract_task->alternatives_.count(mode));
		abstract_task->cast_const().alternatives_[mode] = Handle(*this);
	}
}


Task::RunParams::RunParams(const Renderer::Handle &renderer):
	rendererHolder(renderer), renderer(renderer.get()) { }


Task::Task():
	bounds_calculated(false),
	bounds(Rect::infinite()),
	source_rect(Rect::infinite()),
	target_rect(RectInt::zero())
{ }

Task::~Task()
{ }

void
Task::assign_target(const Task &other) {
	source_rect = other.source_rect;
	target_rect = other.target_rect;
	target_surface = other.target_surface;
}

void
Task::assign(const Task &other) {
	assign_target(other);
	sub_tasks = other.sub_tasks;
	renderer_data = other.renderer_data; // TODO: remove renderer_data from task
}

Task&
Task::operator=(const Task &other)
	{ assign(other); return *this; }

bool
Task::can_convert_to(ModeToken::Handle mode) const
{
	if (!mode) return false;

	Token::Handle token = get_token();
	if (!token->is_abstract())
		token = token->abstract_task;

	return token->alternatives().count(mode);
}

ModeToken::Handle
Task::get_mode() const
	{ return get_token()->mode; }

Task::Handle
Task::convert_to(ModeToken::Handle mode) const
{
	if (!mode)
		return Task::Handle();

	Token::Handle token = get_token();
	if (!token->is_abstract())
		return Task::Handle();

	Token::Map::const_iterator i = token->alternatives().find(mode);
	if (i == token->alternatives().end())
		return Task::Handle();

	return Task::Handle(i->second->convert(*this));
}

Task::Handle
Task::convert_to_any() const
{
	Token::Handle token = get_token();
	if (!token->is_abstract())
		return Task::Handle();

	Task::Handle task;
	for(Token::Map::const_iterator i = token->alternatives().begin(); i != token->alternatives().end(); ++i) {
		task = std::shared_ptr<Task>(i->second->convert(*this));
		if (task) return task;
	}

	return Task::Handle();
}

Task::Handle
Task::clone() const {
	Task *t = get_token()->clone(*this);
	assert(t);
	return Task::Handle(t);
}

Task::Handle
Task::clone_recursive() const
{
	Task::Handle task = clone();
	if (task)
		for(List::iterator i = task->sub_tasks.begin(); i != task->sub_tasks.end(); ++i)
			if (*i) (*i) = (*i)->clone_recursive();
	return task;
}

Vector
Task::get_pixels_per_unit() const
{
	if (!is_valid_coords())
		return Vector();
	return Vector(
		(Real)target_rect.get_width()/source_rect.get_width(),
		(Real)target_rect.get_height()/source_rect.get_height() );
}

Vector
Task::get_units_per_pixel() const
{
	if (!is_valid_coords())
		return Vector();
	return Vector(
		source_rect.get_width()/(Real)target_rect.get_width(),
		source_rect.get_height()/(Real)target_rect.get_height() );
}

void
Task::trunc_to_zero()
{
	source_rect = Rect::zero();
	target_rect = RectInt::zero();
}

void
Task::trunc_source_rect(const Rect &rect)
{
	if (!rect.is_valid() || !is_valid_coords())
		{ trunc_to_zero(); return; }
	Rect sr = source_rect & rect;
	if (!sr.is_valid())
		{ trunc_to_zero(); return; }
	Vector ppu = get_pixels_per_unit();
	RectInt tr = target_rect;
	tr.minx += (int)floor(ppu[0]*(sr.minx - source_rect.minx));
	tr.miny += (int)floor(ppu[1]*(sr.miny - source_rect.miny));
	tr.maxx -= (int)floor(ppu[0]*(source_rect.maxx - sr.maxx));
	tr.maxy -= (int)floor(ppu[1]*(source_rect.maxy - sr.maxy));
	trunc_target_rect(tr);
}

void
Task::trunc_target_rect(const RectInt &rect)
{
	if (!rect.is_valid() || !is_valid_coords())
		{ trunc_to_zero(); return; }
	RectInt tr = target_rect & rect;
	if (!tr.is_valid())
		{ trunc_to_zero(); return; }
	Vector upp = get_units_per_pixel();
	source_rect.minx += upp[0]*(tr.minx - target_rect.minx);
	source_rect.miny += upp[1]*(tr.miny - target_rect.miny);
	source_rect.maxx -= upp[0]*(target_rect.maxx - tr.maxx);
	source_rect.maxy -= upp[1]*(target_rect.maxy - tr.maxy);
	target_rect = tr;
}

void
Task::trunc_by_bounds()
	{ trunc_source_rect(get_bounds()); }

void
Task::move_target_rect(const VectorInt &offset)
	{ if (is_valid_coords()) target_rect += offset; }

void
Task::set_target_origin(const VectorInt &origin)
	{ if (is_valid_coords()) move_target_rect(origin - target_rect.get_min()); }

Rect
Task::calc_bounds() const
	{ return Rect::infinite(); }

void
Task::set_coords(const Rect &source_rect, const VectorInt &target_size)
{
	if (this->source_rect.is_full_infinite()) {
		this->source_rect = source_rect;
		this->target_rect = RectInt(VectorInt(), target_size);
		if (!is_valid_coords())
			trunc_to_zero();
	} else {
		trunc_source_rect(source_rect);
	}

	if (!target_surface)
		target_surface = std::make_shared<SurfaceResource>();

	// allocate surface by incoming target_size without truncation,
	// it's significant for transformation antialiasing
	if (!target_surface->is_exists())
		target_surface->create(target_rect.maxx, target_rect.maxy);

	trunc_by_bounds();
	set_coords_sub_tasks();
}

bool
Task::allow_run_before(Task &other) const {
	if (!is_valid() || !other.is_valid())
		return true;
	if (!get_allow_multithreading() && !other.get_allow_multithreading())
		return false;
	if (target_surface == other.target_surface)
		if ( !get_mode_allow_simultaneous_write()
		  || !other.get_mode_allow_simultaneous_write()
		  || get_target_token() != other.get_target_token()
		  || rect_intersect(target_rect, other.target_rect) )
			return false;
	for(Task::List::const_iterator i = sub_tasks.begin(); i != sub_tasks.end(); ++i)
		if ( *i && (*i)->is_valid()
		  && (*i)->target_surface == other.target_surface
		  && rect_intersect((*i)->target_rect, other.target_rect) ) return false;
	return true;
}

void
Task::set_coords_zero()
	{ set_coords(Rect::zero(), VectorInt::zero()); }

void
Task::touch_coords()
	{ set_coords(Rect(source_rect), target_rect.get_size()); }


void
Task::set_coords_sub_tasks()
{
	// by default set the same coords for all childs
	for(List::iterator i = sub_tasks.begin(); i != sub_tasks.end(); ++i)
		if (*i) (*i)->set_coords(source_rect, target_rect.get_size());
}

bool
Task::run(RunParams & /* params */) const
	{ return false; }


// TaskList

VectorInt
TaskList::calc_target_offset(const Task &a, const Task &b)
{
	Vector ppuA = a.get_pixels_per_unit();
	Vector ppuB = b.get_pixels_per_unit();
	if (ppuA != ppuB)
		warning( "Different pixel-per-unit value while calculation of target offset. a: %s, b: %s",
				 a.get_token()->name.c_str(),
				 b.get_token()->name.c_str() );
	Vector offset = (b.source_rect.get_min() - a.source_rect.get_min()).multiply_coords(a.get_pixels_per_unit());
	return b.target_rect.get_min() - a.target_rect.get_min() - VectorInt((int)round(offset[0]), (int)round(offset[1]));
}


// TaskLockSurface

void
TaskLockSurface::set_surface(const SurfaceResource::Handle &surface)
{
	unlock();
	target_surface = surface;
	source_rect = Rect(0.0, 0.0, 1.0, 1.0);
	target_rect = RectInt(
		VectorInt::zero(),
		target_surface ? target_surface->get_size() : VectorInt::zero() );
	lock();
}

TaskLockSurface&
TaskLockSurface::operator=(const TaskLockSurface& other) {
	*(TaskSurface*)this = other;
	if (other.lock_ && other.lock_->resource != other.target_surface)
		lock();
	return *this;
}

void
TaskLockSurface::lock() {
	if (lock_ && lock_->resource != target_surface)
		unlock();
	if (target_surface)
		lock_ = new SurfaceResource::LockReadBase(target_surface);
}

void
TaskLockSurface::unlock() {
	if (lock_) {
		delete lock_;
		lock_ = NULL;
	}
}


// TaskEvent

TaskEvent&
TaskEvent::operator=(const TaskEvent &other)
{
	*(Task*)(this) = other;
	done = (int)other.done;
	cancelled = (int)other.cancelled;
	signal_finished = other.signal_finished;
	return *this;
}

bool
TaskEvent::is_done() const
{
	std::lock_guard<std::mutex> lock(mutex);
	return done;
}

bool
TaskEvent::is_cancelled() const
{
	std::lock_guard<std::mutex> lock(mutex);
	return cancelled;
}

bool
TaskEvent::is_finished() const
{
	std::lock_guard<std::mutex> lock(mutex);
	return done || cancelled;
}

void
TaskEvent::finish(bool success)
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (done || cancelled) return;
		(success ? done : cancelled) = true;
	}
	signal_finished(success);
	cond.notify_one();
}

void
TaskEvent::wait()
{
	std::unique_lock<std::mutex> lock(mutex);
	if (done || cancelled) return;
	cond.wait(lock);
}

bool
TaskEvent::run(RunParams & /* params */) const
{
	const_cast<TaskEvent*>(this)->finish(true);
	return true;
}

/* === E N T R Y P O I N T ================================================= */
