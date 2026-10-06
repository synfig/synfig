/* === S Y N F I G ========================================================= */
/*!	\file action.h
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#ifndef __SYNFIG_APP_ACTION_H
#define __SYNFIG_APP_ACTION_H

/* === H E A D E R S ======================================================= */

#include "action_param.h"

/* === M A C R O S ========================================================= */

#define ACTION_MODULE_EXT public: \
	static const char name__[], local_name__[], version__[], task__[]; \
	static const synfigapp::Action::Category category__; \
	static const int priority__; \
	static synfigapp::Action::Base *create(); \
	virtual synfig::String get_name()const;	\
	virtual synfig::String get_local_name()const;


#define ACTION_SET_NAME(class,x) const char class::name__[]=x

#define ACTION_SET_CATEGORY(class,x) const synfigapp::Action::Category class::category__(x)

#define ACTION_SET_TASK(class,x) const char class::task__[]=x

#define ACTION_SET_PRIORITY(class,x) const int class::priority__=x

#define ACTION_SET_LOCAL_NAME(class,x) const char class::local_name__[]=x

#define ACTION_SET_VERSION(class,x) const char class::version__[]=x

//! don't define get_local_name() - allow the action code to define its own
#define ACTION_INIT_NO_GET_LOCAL_NAME(class)			  \
	synfigapp::Action::Base* class::create() { return new class(); } \
	synfig::String class::get_name()const { return name__; }

#define ACTION_INIT(class)				 \
	ACTION_INIT_NO_GET_LOCAL_NAME(class) \
	synfig::String class::get_local_name()const { return synfigapp_localize(local_name__); }

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
class ProgressCallback;
class Canvas;
}; // END of namespace synfig

namespace synfigapp {

class Instance;
class Main;

namespace Action {

class System;


//! Exception class, thrown when redoing or undoing an action
class Error
{
public:
	enum Type
	{
		TYPE_UNKNOWN,
		TYPE_UNABLE,
		TYPE_BADPARAM,
		TYPE_CRITICAL,
		TYPE_NOTREADY,
		TYPE_BUG,

		TYPE_END
	};
private:

	Type type_;
	synfig::String desc_;

public:

	Error(Type type, const char *format, ...):
		type_(type)
	{
		va_list args;
		va_start(args,format);
		desc_=synfig::vstrprintf(format,args);
		va_end(args);
	}

	Error(const char *format, ...):
		type_(TYPE_UNKNOWN)
	{
		va_list args;
		va_start(args,format);
		desc_=synfig::vstrprintf(format,args);
		va_end(args);
	}

	Error(Type type=TYPE_UNABLE):
		type_(type)
	{
	}

	Type get_type()const { return type_; }
	synfig::String get_desc()const { return desc_; }

}; // END of class Action::Error

class Param;
class ParamList;
class ParamDesc;
class ParamVocab;

// Action Category
enum Category
{
	CATEGORY_NONE			=0,
	CATEGORY_LAYER			=(1<<0),
	CATEGORY_CANVAS			=(1<<1),
	CATEGORY_WAYPOINT		=(1<<2),
	CATEGORY_ACTIVEPOINT	=(1<<3),
	CATEGORY_VALUEDESC		=(1<<4),
	CATEGORY_VALUENODE		=(1<<5),
	CATEGORY_KEYFRAME		=(1<<6),
	CATEGORY_GROUP			=(1<<7),
	CATEGORY_BEZIER			=(1<<8),

	CATEGORY_OTHER			=(1<<12),

	CATEGORY_DRAG			=(1<<24),

	CATEGORY_HIDDEN			=(1<<31),
	CATEGORY_ALL			=(~0)-(1<<31)		//!< All categories (EXCEPT HIDDEN)
}; // END of enum Category

inline Category operator|(Category lhs, Category rhs)
{ return static_cast<Category>(int(lhs)|int(rhs)); }



//! Top-level base class for all actions
/*!	An action should implement the following functions:
**	- static bool is_candidate(const ParamList &x);
**		- 	Checks the ParamList to see if this action could be performed.
**	- static ParamVocab get_param_vocab();
**		-	Yields the ParamVocab object which describes what
**			this action needs before it can perform the act.
**	- static Action::Base* create();
**		-	Factory for creating this action from a ParamList
**
*/
class Base : public etl::shared_object
{
protected:
	Base() { }

public:
	virtual ~Base() { };

	//! This function will throw an Action::Error() on failure
	virtual void perform()=0;

	virtual bool set_param(const synfig::String& /*name*/, const Param &) { return false; }
	virtual bool get_param(const synfig::String& /*name*/, Param &) { return false; }
	virtual bool is_ready()const=0;

	virtual synfig::String get_name()const =0;
	virtual synfig::String get_local_name()const { return get_name(); }

	void set_param_list(const ParamList &);

	static synfig::String get_layer_descriptions(const std::list<synfig::Layer::Handle> layers, synfig::String singular_prefix = "", synfig::String plural_prefix = "");
	static synfig::String get_layer_descriptions(const std::list<std::pair<synfig::Layer::Handle,int> > layers, synfig::String singular_prefix = "", synfig::String plural_prefix = "");
}; // END of class Action::Base

typedef Action::Base* (*Factory)();
typedef bool (*CandidateChecker)(const ParamList &x);
typedef ParamVocab (*GetParamVocab)();

typedef etl::handle<Base> Handle;

//! Undoable Action Base Class
class Undoable : public Base
{
	friend class System;
	bool active_;

protected:
	Undoable();

#ifdef _DEBUG
	~Undoable();
#endif

private:
	void set_active(bool x) { active_=x; }

public:
	typedef etl::handle<Undoable> Handle;

	//! This function will throw an Action::Error() on failure
	virtual void undo()=0;

	bool is_active()const { return active_; }

#ifdef _DEBUG
	virtual void ref() const noexcept;
	virtual void unref()const;
#endif
}; // END of class Action::Undoable

//! Action base class for canvas-specific actions
class CanvasSpecific
{
private:
	bool is_dirty_;
	EditMode	mode_;

	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_;
	synfig::Canvas::Handle canvas_;

protected:
	CanvasSpecific(const synfig::Canvas::Handle &canvas):is_dirty_(true),mode_(MODE_UNDEFINED),canvas_(canvas) { }
	CanvasSpecific():is_dirty_(true), mode_(MODE_UNDEFINED) { }

	virtual ~CanvasSpecific() { };


public:

	void set_canvas(synfig::Canvas::Handle x) { canvas_=x; }
	void set_canvas_interface(etl::loose_handle<synfigapp::CanvasInterface> x) { canvas_interface_=x; }

	const synfig::Canvas::Handle& get_canvas()const { return canvas_; }
	const etl::loose_handle<synfigapp::CanvasInterface>& get_canvas_interface()const { return canvas_interface_; }

	static ParamVocab get_param_vocab();
	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool get_param(const synfig::String& /*name*/, Param &) { return false; }
	virtual bool is_ready()const;

	EditMode get_edit_mode()const;

	void set_edit_mode(EditMode x) { mode_=x; }

	bool is_dirty()const { return is_dirty_; }
	void set_dirty(bool x=true) { is_dirty_=x; }

}; // END of class Action::CanvasSpecific

typedef std::list< Action::Undoable::Handle > ActionList;

/*!	\class synfigapp::Action::Super
**	\brief Super-Action base class for actions composed of several other actions.
**
**	Actions deriving from this class should only implement prepare(), and
**	NOT implement perform() or undo().
*/
class Super : public Undoable, public CanvasSpecific
{
	ActionList action_list_;

public:
	typedef etl::handle<Super> Handle;

	ActionList &action_list() { return action_list_; }
	const ActionList &action_list()const { return action_list_; }

	virtual void prepare()=0;

	void clear() { action_list().clear(); }

	bool first_time()const { return action_list_.empty(); }

	void add_action(Undoable::Handle action);
	void add_action_front(Undoable::Handle action);

	void add_action(Action::Handle action)
	{
		Undoable::Handle undoable = Undoable::Handle::cast_dynamic(action);
		assert(undoable);
		add_action(undoable);
	}

	void add_action_front(Action::Handle action)
	{
		Undoable::Handle undoable = Undoable::Handle::cast_dynamic(action);
		assert(undoable);
		add_action_front(undoable);
	}

	virtual void perform();
	virtual void undo();

}; // END of class Action::Super


class Group : public Super
{
	synfig::String name_;

	ActionList action_list_;
protected:
	bool ready_;
public:
	typedef etl::handle<Group> Handle;

	Group(const synfig::String &str="Group");
	virtual ~Group();

	virtual synfig::String get_name()const { return name_; }

	virtual void prepare() { };

	virtual bool set_param(const synfig::String& /*name*/, const Param &) { return false; }
	virtual bool is_ready()const { return ready_; }

	void set_name(std::string&x) { name_=x; }
}; // END of class Action::Group





struct BookEntry
{
	synfig::String 	name;
	synfig::String 	local_name;
	synfig::String 	version;
	synfig::String 	task;
	int 			priority;
	Category		category;
	Factory 		factory;
	CandidateChecker	is_candidate;
	GetParamVocab	get_param_vocab;

	bool operator<(const BookEntry &rhs)const { return priority<rhs.priority; }
}; // END of struct BookEntry

typedef std::map<synfig::String,BookEntry> Book;

class CandidateList : public std::list<BookEntry>
{
public:
	iterator find(const synfig::String& x);
	const_iterator find(const synfig::String& x)const { return const_cast<CandidateList*>(this)->find(x); }
};

Book& book();

Handle create(const synfig::String &name);

//! Compiles a list of potential candidate actions with the given \a param_list and \a category
CandidateList compile_candidate_list(const ParamList& param_list, Category category=CATEGORY_ALL);

/** Compiles a list of potential candidate actions, that are not CATEGORY_HIDDEN, with the given @a param_list and @a category */
CandidateList compile_visible_candidate_list(const ParamList& param_list, Category category = CATEGORY_ALL);

/*!	\class synfigapp::Action::Main
**	\brief \writeme
**
**	\writeme
*/
class Main
{
	friend class synfigapp::Main;

	Main();

public:
	~Main();

}; // END of class Action::Main

}; // END of namespace Action

}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
