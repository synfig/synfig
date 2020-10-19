/*! ========================================================================
** Extended Template and Library Test Suite
** Angle Class Test
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
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
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <ETL/smach>
#include <cstdio>

/* === M A C R O S ========================================================= */

using namespace etl;

/* === C L A S S E S ======================================================= */

enum EventKey
{
	EVENT_1,
	EVENT_2,
	EVENT_3,
	EVENT_4
};



struct MachineContext
{
	smach<MachineContext,EventKey> machine;

	MachineContext():machine(this)
	{
	}
};

typedef smach<MachineContext,EventKey> Smach;

class Event1 : public Smach::event
{
public:
	Event1():Smach::event(EVENT_1) { }
};


class DefaultStateContext
{
	MachineContext *context;
public:
	DefaultStateContext(MachineContext *context):context(context) { printf("Entered Default State\n"); }
	~DefaultStateContext() { printf("Left Default State\n"); }

	Smach::event_result event1_handler(const Smach::event& x)
	{
		printf("DEFAULT STATE: Received Event 1\n");
		return Smach::RESULT_ACCEPT;
	}
};

class DefaultState : public Smach::state<DefaultStateContext>
{
public:
	DefaultState():Smach::state<DefaultStateContext>("DefaultState")
	{
		insert(event_def(EVENT_1,&DefaultStateContext::event1_handler));
	}

} default_state;







class State1Context
{
	MachineContext *context;
public:
	State1Context(MachineContext *context):context(context) { printf("Entered State 1\n"); }
	~State1Context() { printf("Left State 1\n"); }

	Smach::event_result event1_handler(const Smach::event& x)
	{
		printf("STATE1: Received Event 1\n");
		return Smach::RESULT_OK;
	}

	Smach::event_result event3_handler(const Smach::event& x);
};

class State1 : public Smach::state<State1Context>
{
public:
	State1():Smach::state<State1Context>("State1")
	{
		insert(event_def(EVENT_1,&State1Context::event1_handler));
		insert(event_def(EVENT_3,&State1Context::event3_handler));
	}

} state_1;


class State2Context
{
	MachineContext *context;
public:
	State2Context(MachineContext *context):context(context) { printf("Entered State 2\n"); }
	~State2Context() { printf("Left State 2\n"); }

	Smach::event_result event1_handler(const Smach::event& /*x*/)
	{
		printf("STATE2: Received Event 1\n");
		return Smach::RESULT_OK;
	}

	Smach::event_result event2_handler(const Smach::event& /*x*/)
	{
		printf("STATE2: Received Event 2\n");
		return Smach::RESULT_OK;
	}

	Smach::event_result event3_handler(const Smach::event& /*x*/)
	{
		printf("STATE2: Received Event 3\n");
		return Smach::RESULT_OK;
	}
};

class State2 : public Smach::state<State2Context>
{
public:
	State2():Smach::state<State2Context>("State2")
	{
		insert(event_def(EVENT_1,&State2Context::event1_handler));
		insert(event_def(EVENT_2,&State2Context::event2_handler));
		insert(event_def(EVENT_3,&State2Context::event3_handler));
	}

} state_2;

Smach::event_result
State1Context::event3_handler(const Smach::event& x)
{
	printf("STATE1: Received Event 3, throwing state to change to...\n");

	throw &state_2;
//	context->machine.enter(&state_2);
//	return Smach::RESULT_ACCEPT;
}

/* === G L O B A L S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

int main()
{
	int error=0;

	MachineContext context;
	try
	{
		Smach& state_machine(context.machine);

		state_machine.set_default_state(&default_state);

		state_machine.enter(&state_1);

		state_machine.process_event(Event1());
		state_machine.process_event(EVENT_1);
		state_machine.process_event(EVENT_2);
		state_machine.process_event(EVENT_3);

		state_machine.process_event(Event1());
		state_machine.process_event(EVENT_1);
		state_machine.process_event(EVENT_2);
		state_machine.process_event(EVENT_3);

		state_machine.process_event(Event1());
		state_machine.process_event(EVENT_1);
		state_machine.process_event(EVENT_2);
		state_machine.process_event(EVENT_3);
	}
	catch(...)
	{
		printf("Uncaught exception\n");
		error++;
	}

	return error;
}
