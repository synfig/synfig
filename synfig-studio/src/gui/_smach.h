/*! ========================================================================
** Extended Template and Library
** \file _smach.h
** \brief State Machine Abstraction Class Implementation
** \internal
**
** \legal
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2008 Chris Moore
**
** This file is part of Synfig.
**
** Synfig is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** Synfig is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
** \endlegal
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__SMACH_H_
#define __ETL__SMACH_H_

/* === H E A D E R S ======================================================= */

#include <cassert>
#include <vector>
#include <algorithm>
#include <stdexcept>

/* === M A C R O S ========================================================= */

#define SMACH_STATE_STACK_SIZE		(32)


/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

/*! ========================================================================
** \class	smach
** \brief	Templatized State Machine
**
** A more detailed description needs to be written.
*/
template <typename CON, typename K=int>
class smach
{
public:

	typedef K event_key;
	typedef CON context_type;


	struct egress_exception { };
	struct pop_exception { };


	//! Result type for event processing
	enum event_result
	{
		// These values are returned by the event
		// handlers cast to state pointers.
		RESULT_ERROR,		//!< General error or malfunction
		RESULT_OK,			//!< Event has been processed
		RESULT_ACCEPT,		//!< The event has been explicitly accepted.
		RESULT_REJECT,		//!< The event has been explicitly rejected.

		RESULT_END			//!< Not a valid result
	};

	//template<typename T> class state;

	//! Event base class
	struct event
	{
		event_key key;

		event() { }
		event(const event_key& key):key(key) { }

		operator event_key()const { return key; }
	};

	//! Event definition class
	template<typename T>
	class event_def_internal
	{
		// List our friends
		friend class smach;
		//friend class state<T>;

	public:
		typedef T state_context_type;

		//! Event function type
		typedef event_result (T::*funcptr)(const event&);

	//private:

		event_key id;		//<! Event ID
		funcptr handler;	//<! Pointer event handler

	public:

		//! Less-than operator for sorting. Based on event_key value.
		bool operator<(const event_def_internal &rhs)const
			{ return id<rhs.id; }

		//! Equal-to operator. Based on event_key value.
		bool operator==(const event_def_internal &rhs)const
			{ return id==rhs.id; }

		//! Less-than operator for finding.
		bool operator<(const event_key &rhs)const
			{ return id<rhs; }

		//! Equal-to operator. Based on event_key value.
		bool operator==(const event_key &rhs)const
			{ return id==rhs; }

		//! Trivial Constructor
		event_def_internal() { }

		//! Constructor for creating an event_def_internal from the given key and function reference.
		event_def_internal(event_key a, funcptr b):id(a),handler(b) { }

		//! Copy constructor
		event_def_internal(const event_def_internal &x):id(x.id),handler(x.handler) { }

	};

	class state_base
	{
		// Our parent is our friend
		friend class smach;
	public:
		virtual ~state_base() { }

		virtual void* enter_state(context_type* machine_context)const=0;

		virtual bool leave_state(void* state_context)const=0;

		virtual event_result process_event(void* state_context,const event& id)const=0;

		virtual const char *get_name() const=0;
	};

	//! State class
	template<typename T>
	class state : public state_base
	{
		// Our parent is our friend
		friend class smach;

	public:
		typedef event_def_internal<T> event_def;
		typedef T state_context_type;


	private:

		std::vector<event_def> event_list;

		smach *nested;		//! Nested machine
		event_key low,high;	//! Lowest and Highest event values
		const char *name;	//! Name of the state
		typename event_def::funcptr default_handler;	//! Default handler for unknown key

	public:

		//! Constructor
		state(const char *n, smach* nest=0):
			nested(nest),name(n),default_handler(NULL)
		{ }

		virtual ~state() { }

		//! Setup a nested state machine
		/*! A more detailed explanation needs to be written */
		void set_nested_machine(smach *sm) { nested=sm; }

		//! Sets the default handler
		void set_default_handler(const typename event_def::funcptr &x) { default_handler=x; }

		//! Returns given the name of the state
		virtual const char *get_name() const { return name; }

		//! Adds an event_def onto the list and then make sure it is sorted correctly.
		void
		insert(const event_def &x)
		{
			// If this is our first event_def,
			// setup the high and low values.
			if(!event_list.size())
				low=high=x.id;

			// Sort the event_def onto the list
			event_list.push_back(x);
			sort(event_list.begin(),event_list.end());

			// Update the low and high markers
			if(x.id<low)
				low=x.id;
			if(high<x.id)
				high=x.id;
		}

		typename std::vector<event_def>::iterator find(const event_key &x) { return binary_find(event_list.begin(),event_list.end(),x); }
		typename std::vector<event_def>::const_iterator find(const event_key &x)const { return binary_find(event_list.begin(),event_list.end(),x); }

	protected:

		/*
		This part compiles differently in gcc/clang and MSVC compilers.
		Since the behavior of the compiler is not specified for virtual methods
		(C++11 standard says: § 14.7.1.10. "It is unspecified whether or not an
		implementation implicitly instantiates a virtual member function of a
		class template if the virtual member function would not otherwise be instantiated").

		Unlike GCC/Clang, MSVC expects the template class to be fully defined
		at the moment, so this code does not compile. Therefore, we move
		the `enter_state` method to derived classes.

		Minimal working example:
		```
		template <class T>
		class A {
		virtual void* Run() {
			return new T();
		}
		};

		class C;

		class B : A<C> {
		
		};
		```

		MSVC fails with the error:
		```
		<source>(4): error C2027: use of undefined type 'C'
		<source>(8): note: see declaration of 'C'
		<source>(3): note: while compiling class template member function 'void *A<C>::Run(void)'
		<source>(10): note: see reference to class template instantiation 'A<C>' being compiled
		Compiler returned: 2
		```

		This code can be tested online at https://godbolt.org		
		*/

		/*virtual void* enter_state(context_type* machine_context)const
		{
			return new state_context_type(machine_context);
		}*/

		virtual bool leave_state(void* x)const
		{
			state_context_type* state_context(reinterpret_cast<state_context_type*>(x));
			delete state_context;
			return true;
		}

		virtual event_result
		process_event(void* x,const event& id)const
		{
			state_context_type* state_context(reinterpret_cast<state_context_type*>(x));

			// Check for nested machine in state
			if(nested)
			{
				const event_result ret(nested->process_event(id));
				if(ret!=RESULT_OK)
					return ret;
			}

			// Quick test to make sure that the
			// given event is in the state
			if(id.key<low || high<id.key)
				return RESULT_OK;

			// Look for the event
			typename std::vector<event_def>::const_iterator iter(find(id.key));

			// If search results were negative, fail.
			if(iter->id!=id.key)
				return RESULT_OK;

			// Execute event function
			event_result ret((state_context->*(iter->handler))(id));

			if(ret==RESULT_OK && default_handler)
				ret=(state_context->*(default_handler))(id);

			return ret;
		}
	};
	
	class guard {
	private:
		bool *flag;
	public:
		explicit guard(bool &flag): flag() {
			assert(!flag);
			if (!flag) { flag = true; this->flag = &flag; }
		}
		~guard()
			{ if (flag) *flag = false;}
		operator bool() const
			{ return flag != nullptr; }
	};


private:

	// Machine data
	const state_base* curr_state;  //!< Current state of the machine
	smach* child;                  //!< Child machine
	void* state_context;           //!< State Context
	context_type* machine_context; //!< Machine Context
	const state_base* default_state;
	void* default_context;

	//! State stack data
	const state_base* 	state_stack[SMACH_STATE_STACK_SIZE];
	void* 				state_context_stack[SMACH_STATE_STACK_SIZE] = {0};
	int 				states_on_stack;
	
	bool changing_state;
	bool changing_default_state;

public:

	//! Gets the name of the currently active state
	const char *
	get_state_name()const
	{
		if(curr_state)
			return curr_state->get_name();
		if(default_state)
			return default_state->get_name();
		return 0;
	}

	//! Determines if a given event result is an error
	/*! This function allows us to quickly see
		if an event_result contained an error */
	static bool
	event_error(const event_result &rhs)
		{ return rhs<=RESULT_ERROR; }

	bool
	set_default_state(const state_base *nextstate)
	{
		guard lock(changing_default_state);
		if (!lock) return false;
		
		// Keep track of the current state unless
		// the state switch fails
		const state_base *prev_state = default_state;
		void *prev_context = default_context;

		default_state = 0;
		default_context = 0;
		
		// If we are already in a state, leave it
		if (prev_state && prev_context)
			prev_state->leave_state(prev_context);
		
		// Attempt to enter the state
		default_state=nextstate;
		if (default_state) {
			default_context = default_state->enter_state(machine_context);
			if (default_context)
				return true;
		}

		// We failed, so attempt to return to previous state
		default_state = prev_state;
		if (default_state) {
			default_context = default_state->enter_state(machine_context);
			if (!default_context)
				default_state = 0;
		}

		// At this point we are not in the
		// requested state, so return failure
		return false;
	}

	//! Leaves the current state
	/*! Effectively makes the state_depth() function return zero. */
	bool
	egress()
	{
		// Try to pop all states off the state stack, before lock
		while(states_on_stack)
			if (!pop_state()) return false;

		guard lock(changing_state);
		if (!lock) return false;
		
		// If we are not in a state, then I guess
		// we were successful.
		if (!curr_state)
			return true;

		const state_base* old_state = curr_state;
		void *old_context = state_context;

		// Clear out the current state and its state_context
		curr_state = 0;
		state_context = 0;

		// Leave the state
		if (old_state && old_context)
			old_state->leave_state(old_context);

		return true;
	}

	//! State entry function
	/*! Attempts to enter the given state,
		popping off all states on the stack
		in the process. */
	bool
	enter(const state_base *nextstate)
	{
		guard lock(changing_state);
		if (!lock) return false;

		// Keep track of the current state unless
		// the state switch fails
		const state_base *prev_state=curr_state;

		// If we are already in a state, leave it and
		// collapse the state stack
		changing_state = false;
		egress();
		assert(!state_context);
		state_context = 0;
		changing_state = true;

		// Attempt to enter the state
		curr_state = nextstate;
		if (curr_state) {
			state_context = curr_state->enter_state(machine_context);
			if (state_context)
				return true;
		}
		
		// If we had a previous state, enter it
		curr_state = prev_state;
		if (curr_state) {
			state_context = curr_state->enter_state(machine_context);
			if (!state_context)
				curr_state = 0;
		}

		// At this point we are not in the
		// requested state, so return failure
		return false;
	}

	//! Pushes state onto state stack
	/*! This allows you to enter a state without
		leaving your current state.
		\param   nextstate Pointer to the state to enter
		\sa      pop_state()
	*/
	bool
	push_state(const state_base *nextstate)
	{
		// If there is no current state,
		// just go ahead and enter the given state.
		// before locking
		if (!curr_state)
			return enter(nextstate);
		
		guard lock(changing_state);
		if (!lock) return false;

		// If there are not enough slots, then throw something.
		if (states_on_stack==SMACH_STATE_STACK_SIZE)
			throw(std::overflow_error("smach<>::push_state(): state stack overflow!"));

		// Push the current state onto the stack
		state_stack[states_on_stack] = curr_state;
		state_context_stack[states_on_stack++] = state_context;

		// Try to enter the next state
		state_context = 0;
		curr_state = nextstate;
		if (curr_state) {
			state_context = curr_state->enter_state(machine_context);
			if (state_context)
				return true;
		}

		// Unable to push state, return to old one
		curr_state = state_stack[--states_on_stack];
		state_context = state_context_stack[states_on_stack];
		return false;
	}

	//! Pops state off of state stack
	/*! Decreases state depth */
	bool
	pop_state()
	{
		{ // scope for locking
			guard lock(changing_state);
			if (!lock) return false;
			
			// If we aren't in a state, then there is nothing
			// to do.
			if (!curr_state && !states_on_stack)
				throw(std::underflow_error("smach<>::pop_state(): stack is empty!"));

			if (states_on_stack) {
				const state_base* old_state = curr_state;
				void *old_context = state_context;

				// Pop previous state off of stack
				--states_on_stack;
				curr_state = state_stack[states_on_stack];
				state_context = state_context_stack[states_on_stack];

				if (old_state && old_context)
					old_state->leave_state(old_context);
				return true;
			}
		}
		
		// call egress with changing_state unlocked
		return egress();
	}

	//! State Machine Constructor
	/*! A more detailed description needs to be written */
	smach(context_type* machine_context=0):
		curr_state(0),
		child(0),
		state_context(0),
		machine_context(machine_context),
		default_state(0),
		default_context(0),
		states_on_stack(0),
		changing_state(false),
		changing_default_state(false)
	{ }

	//! The destructor
	~smach()
	{
		assert(!changing_state);
		changing_state = false;
		egress();

		// reset default state
		assert(!changing_default_state);
		changing_default_state = false;
		const state_base *prev_state = default_state;
		void* prev_context = default_context;
		default_state=0;
		default_context=0;
		if (prev_state && prev_context)
			prev_state->leave_state(prev_context);

		assert(!default_state && !default_context);
		assert(!curr_state && !state_context);
	}

	//! Sets up a child state machine
	/*! A child state machine runs in parallel with
		its parent, and gets event priority. This
		mechanism is useful in cases where an inherited
		object has its own state machine. */
	void set_child(smach *x)
	{
		child=x;
	}

	//! Returns the number states currently active
	int
	state_depth()
		{ return curr_state?states_on_stack+1:0; }

	event_result
	process_event(const event_key& id) { return process_event(event(id)); }

	//! Process an event
	event_result
	process_event(const event& id)
	{
		event_result ret(RESULT_OK);

		// Check for child machine
		if(child)
		{
			ret=child->process_event(id);
			if(ret!=RESULT_OK)
				return ret;
		}

		try
		{
			if(curr_state && state_context)
				ret=curr_state->process_event(state_context,id);

			if(ret==RESULT_OK && default_state && default_context)
				ret=default_state->process_event(default_context,id);

			return ret;
		}
		catch(const egress_exception&) {
			if (egress()) {
				ret=RESULT_ACCEPT;
			} else {
				ret=RESULT_ERROR;
			}
		}
		catch(const pop_exception&) { pop_state(); return RESULT_ACCEPT; }
		catch(const state_base* state) { return enter(state)?RESULT_ACCEPT:RESULT_ERROR; }
		return ret;
	}

}; // END of template class smach

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
