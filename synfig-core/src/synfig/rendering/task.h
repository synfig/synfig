/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/task.h
**	\brief Task Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_TASK_H
#define __SYNFIG_RENDERING_TASK_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <set>

#include <synfig/rect.h>
#include <synfig/vector.h>

#include "surface.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Renderer;


// Helpers


template<typename T>
class Holder
{
public:
	typedef T Type;
	typedef etl::handle<Type> Handle;

private:
	Handle value;

public:
	Type& get()
		{ return *value; }
	const Type& get() const
		{ return *value; }
	const Handle& handle() const
		{ return value; }

	Holder():
		value(new Type()) { }
	Holder(const Holder &other):
		value(new Type(other.get())) { }
	Holder& operator=(const Holder &other)
		{ *value = other.get(); return *this; }
	Type& operator*()
		{ return get(); }
	const Type& operator*() const
		{ return get(); }
	Type* operator->()
		{ return &get(); }
	const Type* operator->() const
		{ return &get(); }
};


// Mode


class Mode {
public:
	static synfig::Token mode_token;
	virtual ~Mode() { }
	virtual Surface::Token::Handle get_mode_target_token() const
		{ return Surface::Token::Handle(); }
};


class ModeToken: public synfig::Token
{
public:
	typedef ConstRef<ModeToken> Handle;
	const String name;
	ModeToken(const String &name):
		synfig::Token(Mode::mode_token.handle()),
		name(name) { }
	inline Handle handle() const
		{ return Handle(*this); }
};


// Interfaces


class TaskInterfaceConstant
{
public:
	virtual bool is_constant()
		{ return true; }
	virtual ~TaskInterfaceConstant() { }
};


class TaskInterfaceSplit
{
public:
	virtual bool is_splittable()
		{ return true; }
	virtual ~TaskInterfaceSplit() { }
};


// Task


class Task: public etl::shared_object
{
public:
	typedef etl::handle<Task> Handle;
	typedef std::vector<Handle> List;
	typedef std::set<Handle> Set;

	typedef Task* (*Fabric)();
	typedef Task* (*CloneFabric)(const Task&);

	class Token: public synfig::Token
	{
	public:
		typedef ConstRef<Token> Handle;
		typedef std::map<ModeToken::Handle, Token::Handle> Map;

	private:
		Map alternatives_;

		inline Token& cast_const(Handle handle) const
			{ return *const_cast<Token*>(this); }

	protected:
		template<typename Type>
		Task* create_func()
			{ return new Type(); }

		template<typename Type, typename TypeAbstract = Type>
		Task* convert_func(const Task &other)
		{
			const TypeAbstract *orig = dynamic_cast<const TypeAbstract*>(&other);
			if (!orig) return NULL;
			Type *task = new Type();
			*task = *orig;
			return task;
		}

		virtual void unprepare_vfunc();
		virtual void prepare_vfunc();

	public:
		const String name;
		const ModeToken::Handle mode;
		const Handle abstract_task;
		const Fabric create;
		const CloneFabric clone;
		const CloneFabric convert;

		//! token for abstract task
		template<typename Type, typename TypeParent>
		Token(String name):
			synfig::Token(TypeParent::token.handle()),
			name(name),
			create(&create_func<Type>),
			clone(&convert_func<Type>),
			convert()
		{ }

		//! token for real task
		template<typename Type, typename TypeParent, typename TypeAbstract>
		Token(String name):
			synfig::Token(TypeParent::token.handle()),
			name(name),
			mode(Type::mode_token.handle()),
			abstract_task(TypeAbstract::token.handle()),
			create(&create_func<Type>),
			clone(&convert_func<Type>),
			convert(&convert_func<Type, TypeAbstract>)
		{ }

		const Map& alternatives() const
			{ return alternatives_; }
		inline bool is_abstract() const
			{ return !mode || !abstract_task || !convert; }
		inline Handle handle() const
			{ return Handle(*this); }
	};

	struct RunParams {
		const Renderer *renderer;
		mutable Task::List sub_queue;
		RunParams(): renderer(renderer) { }
	};

	struct RendererData
	{
		int index;
		int deps_count;
		Set deps; // always empty (deps_count is enough), used inside of Renderer::find_deps only
		Set back_deps;

		Set tmp_deps;
		Set tmp_back_deps;

		RunParams params;
		bool success;

		RendererData(): index(), deps_count(), success() { }
	};

	class LockReadBase: public SurfaceResource::LockReadBase
	{
	public:
		const Task::Handle task;
		explicit LockReadBase(
			const Task::Handle &task,
			Surface::Token::Handle token = Surface::Token::Handle()
		):
			SurfaceResource::LockReadBase(task->target_surface, token, task->target_rect),
			task(task) { }
	};

	class LockWriteBase: public SurfaceResource::SemiLockWriteBase
	{
	public:
		const Task::Handle task;
		explicit LockWriteBase( const Task::Handle &task ):
			SurfaceResource::LockWriteBase(task->target_surface, task->get_target_token(), task->target_rect),
			task(task) { }
	};

	template<typename T>
	class LockReadGeneric: public SurfaceResource::LockRead<T>
	{
	public:
		const Task::Handle task;
		explicit LockReadGeneric(const Task::Handle &task):
			SurfaceResource::LockRead<T>(task->target_surface, task->target_rect),
			task(task) { }
	};

	template<typename T>
	class LockWriteGeneric: public SurfaceResource::SemiLockWrite<T>
	{
	public:
		const Task::Handle task;
		explicit LockWriteGeneric(const Task::Handle &task):
			SurfaceResource::SemiLockWrite<T>(task->target_surface,	task->get_target_token(), task->target_rect),
			task(task) { }
	};


	static synfig::Token token;
	virtual Token::Handle get_token() const = 0;

private:
	static const etl::handle<Task> blank;

	mutable bool bounds_calculated;
	mutable Rect bounds;

public:
	Rect source_rect;
	RectInt target_rect;
	SurfaceResource::Handle target_surface;
	List sub_tasks;

	mutable RendererData renderer_data;

	Task();
	virtual ~Task();

	Surface::Token::Handle get_target_token() const {
		if (const Mode *mode = dynamic_cast<const Mode*>(this))
			return mode->get_mode_target_token();
		return Surface::Token::Handle();
	}

	Task::Handle& sub_task(int index) {
		assert(index >= 0);
		if (index >= (int)sub_tasks.size())
			sub_tasks.resize(index + 1);
		return sub_tasks[index];
	}

	const Task::Handle& sub_task(int index) const {
		assert(index >= 0);
		return index < (int)sub_tasks.size() ? sub_tasks[index] : blank;
	}

	bool can_convert_to(ModeToken::Handle mode) const;
	ModeToken::Handle get_mode() const;
	Task::Handle convert_to(ModeToken::Handle mode) const;
	Task::Handle convert_to_any() const;
	Task::Handle clone() const;
	Task::Handle clone_recursive() const;

	virtual Rect calc_bounds() const;
	void reset_bounds()
		{ bounds_calculated = false; }
	const Rect& get_bounds() const {
		if (!bounds_calculated) { bounds = calc_bounds(); bounds_calculated = true; }
		return bounds;
	}

	Vector get_pixels_per_unit() const;
	Vector get_units_per_pixel() const;

	void trunc_to_zero();
	void trunc_source_rect(const Rect &rect);
	void trunc_target_rect(const RectInt &rect);
	void trunc_by_bounds();
	void move_target_rect(const VectorInt &offset);
	void set_target_origin(const VectorInt &origin);

	bool Task::is_valid_coords_source() const
		{ return !source_rect.is_nan_or_inf() && && source_rect.is_valid(); }
	bool Task::is_valid_coords_target() const
		{ return !target_rect.is_valid(); }
	bool Task::is_valid_coords() const
		{ return is_valid_coords_source() && is_valid_coords_target(); }
	bool Task::is_valid_surface_size() const
		{ return target_surface
			  && target_surface->is_exists()
			  && etl::contains(RectInt(VectorInt::zero(), target_surface->get_size()), target_rect); }
	bool Task::is_valid() const
		{ return is_valid_coords() && is_valid_surface_size(); }

	void set_coords(const Rect &source_rect, const VectorInt target_size);
	void set_coords_zero();
	virtual void set_coords_sub_tasks();
	virtual bool run(RunParams &params) const;
};


// Special tasks


class TaskSurface: public Task
{
public:
	typedef etl::handle<TaskSurface> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token; }
};


class TaskLockSurface: public TaskSurface
{
public:
	typedef etl::handle<TaskLockSurface> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token; }

private:
	SurfaceResource::LockReadBase *lock_;

public:
	TaskLockSurface():
		lock_() { }
	explicit TaskLockSurface(const SurfaceResource::Handle &surface):
		lock_() { set_surface(surface); }
	TaskLockSurface(const TaskLockSurface& other):
		lock_() { *this = other; }
	~TaskLockSurface() { unlock(); }

	TaskLockSurface& operator=(const TaskLockSurface& other);

	void set_surface(const SurfaceResource::Handle &surface);
	void lock();
	void unlock();
};


class TaskNone: public Task, public TaskInterfaceConstant
{
public:
	typedef etl::handle<TaskNone> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token; }
	virtual bool run(RunParams &params) const
		{ return true; }
};


class TaskList: public Task
{
public:
	typedef etl::handle<TaskList> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token; }
	virtual bool run(RunParams &params) const
		{ return true; }
};


class TaskCallbackCond: public Task
{
public:
	typedef etl::handle<TaskCallbackCond> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token; }

	Glib::Threads::Cond *cond;
	Glib::Threads::Mutex *mutex;

	TaskCallbackCond(): cond(), mutex() { }

	virtual bool run(RunParams & /* params */) const {
		if (!cond || !mutex) return false;
		Glib::Threads::Mutex::Lock lock(*mutex);
		cond->signal();
		return true;
	}
};


} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
