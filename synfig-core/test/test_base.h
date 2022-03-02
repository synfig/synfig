/* === S Y N F I G ========================================================= */
/*!	\file test_base.h
**	\brief Macros for unit test writing
**
**	$Id$
**
**	\legal
**	Copyright (c) 2021 Synfig contributors
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
#ifndef SYNFIG_TESTBASE_H
#define SYNFIG_TESTBASE_H

#include <iostream> // std::cerr
#include <cstdlib> // std::abs

#include <synfig/general.h> // synfig::error , synfig::info
#include <synfig/vector.h>

std::ostream& operator<<(std::ostream& os, const synfig::Vector& v)
{
	os << '(' << v[0] << ',' << v[1] << ')';
	return os;
}

#define ERROR_MESSAGE_TWO_VALUES(a, b) \
	std::cerr.precision(8); \
	std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - expected " << a << ", but got " << b << std::endl;

#define ASSERT(value) {\
	if (!(value)) { \
		std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - not TRUE: " << #value << std::endl; \
		return true; \
	} \
}

#define ASSERT_FALSE(value) {\
	if (value) { \
		std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - not FALSE: " << #value << std::endl; \
		return true; \
	} \
}

#define ASSERT_EQUAL(expected, value) {\
	if (expected != value) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
		return true; \
	} \
}

#define ASSERT_NOT_EQUAL(not_acceptable, value) {\
	if (not_acceptable == value) { \
		std::cerr.precision(8); \
		std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - must not be equal" << std::endl; \
		return true; \
	} \
}

#define ASSERT_APPROX_EQUAL(expected, value) {\
	if (!synfig::approximate_equal(expected, value)) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
		return true; \
	} \
}

#define ASSERT_APPROX_EQUAL_MICRO(expected, value) {\
	if (std::abs(expected - value) > 1e-6) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
		return true; \
	} \
}

#define ASSERT_VECTOR_APPROX_EQUAL_MICRO(expected, value) {\
	if (std::abs(expected[0] - value[0]) > 2e-6 || std::abs(expected[1] - value[1]) > 2e-6) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
		return true; \
	} \
}

#define ASSERT_EXCEPTION_THROWN(expected, action) {\
	try {\
		action; \
		std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - expected exception " << #expected << " was not thrown" << std::endl; \
		return true; \
	} catch (expected &ex) { \
	} \
}

#define ASSERT_EXCEPTION_NOT_THROWN(not_expected, action) {\
	try {\
		action; \
	} catch (not_expected &ex) { \
		std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - it should not throw exception " << #not_expected << std::endl; \
		return true; \
	} \
}

#define ASSERT_NO_EXCEPTION_THROWN(action) {\
	try {\
		action; \
	} catch (...) { \
		std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - it should not throw any exception" << std::endl; \
		return true; \
	} \
}

#define ASSERT_SIGNAL_EMITTED(OBJECT,SIGNAL_NAME,SIGNAL_CALL_PARAMS,SIGNAL_SLOT_PARAMS,SIGNAL_SLOT_RETURN,ACTION) {\
	bool tst_signal_emitted = false; \
	sigc::connection tst_conn = OBJECT->SIGNAL_NAME(SIGNAL_CALL_PARAMS).connect( \
		[=, &tst_signal_emitted](SIGNAL_SLOT_PARAMS) -> SIGNAL_SLOT_RETURN {\
			tst_signal_emitted = true; \
		} \
	);\
	ACTION; \
	tst_conn.disconnect(); \
	if (!tst_signal_emitted) { \
		std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - expected signal " << #SIGNAL_NAME << " was not called" << std::endl; \
		return true; \
	} \
}

#define ASSERT_SIGNAL_NOT_EMITTED(OBJECT,SIGNAL_NAME,SIGNAL_CALL_PARAMS,SIGNAL_SLOT_PARAMS,SIGNAL_SLOT_RETURN,ACTION) {\
	bool tst_signal_emitted = false; \
	sigc::connection tst_conn = OBJECT->SIGNAL_NAME(SIGNAL_CALL_PARAMS).connect( \
		[=, &tst_signal_emitted](SIGNAL_SLOT_PARAMS) -> SIGNAL_SLOT_RETURN {\
			tst_signal_emitted = true; \
		} \
	);\
	ACTION; \
	tst_conn.disconnect(); \
	if (tst_signal_emitted) { \
		std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - expected signal " << #SIGNAL_NAME << " was called" << std::endl; \
		return true; \
	} \
}

#define TEST_FUNCTION(function_name) {\
	bool fail = function_name(); \
	if (fail) { \
		synfig::error("%s FAILED", #function_name); \
		tst_failures++; \
	} else { \
		tst_successes++; \
	} \
}

#define TEST_SUITE_BEGIN() \
	int tst_exit_status = 0; \
	{ \
		int tst_successes = 0; \
		int tst_failures = 0; \
		bool tst_exception_thrown = false; \
		try {

#define TEST_SUITE_END() \
		} catch (...) { \
			synfig::error("Some exception has been thrown."); \
			tst_exception_thrown = true; \
		} \
		if (tst_exception_thrown) \
			synfig::error("Test interrupted due to an exception thrown (%i errors and %i successful tests until then)", tst_failures, tst_successes); \
		else if (tst_failures) \
			synfig::error("Test finished with %i errors and %i successful tests", tst_failures, tst_successes); \
		else \
			synfig::info("Success (%i tests)", tst_successes); \
		tst_exit_status = tst_exception_thrown? 2 : (tst_failures ? 1 : 0); \
	}

#endif // SYNFIG_TESTBASE_H
