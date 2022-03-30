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

#include <cstdlib> // std::abs
#include <iostream> // std::cerr
#include <sstream>

#include <synfig/general.h> // synfig::error , synfig::info
#include <synfig/vector.h>

struct SynfigTestException : public std::exception
{
	std::string function;
	int line;
	std::string message;

	SynfigTestException(std::string function, int line, std::string message)
		: function(function), line(line), message(message)
	{}

	const char* what() const noexcept override { return message.c_str(); }
};

std::ostream& operator<<(std::ostream& os, const synfig::Vector& v)
{
	os << '(' << v[0] << ',' << v[1] << ')';
	return os;
}

#define ERROR_MESSAGE_TWO_VALUES(a, b) \
	std::ostringstream oss; \
	oss.precision(8); \
	oss << "\t - expected " << a << ", but got " << b << std::endl; \
	throw SynfigTestException{__FUNCTION__, __LINE__, oss.str()}; \

#define ASSERT(value) {\
	if (!(value)) { \
		std::ostringstream oss; \
		oss << "\t - not TRUE: " << #value << std::endl; \
		throw SynfigTestException{__FUNCTION__, __LINE__, oss.str()}; \
	} \
}

#define ASSERT_FALSE(value) {\
	if (value) { \
		std::ostringstream oss; \
		oss << "\t - not FALSE: " << #value << std::endl; \
		throw SynfigTestException{__FUNCTION__, __LINE__, oss.str()}; \
	} \
}

#define ASSERT_EQUAL(expected, value) {\
	if (expected != value) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
	} \
}

#define ASSERT_NOT_EQUAL(not_acceptable, value) {\
	if (not_acceptable == value) { \
		std::ostringstream oss; \
		oss.precision(8); \
		oss << "\t - must not be equal: " #value << " vs. " << not_acceptable << std::endl; \
		throw SynfigTestException{__FUNCTION__, __LINE__, oss.str()}; \
	} \
}

#define ASSERT_APPROX_EQUAL(expected, value) {\
	if (!synfig::approximate_equal(expected, value)) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
	} \
}

#define ASSERT_APPROX_EQUAL_MICRO(expected, value) {\
	if (std::abs(expected - value) > 1e-6) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
	} \
}

#define ASSERT_VECTOR_APPROX_EQUAL_MICRO(expected, value) {\
	if (std::abs(expected[0] - value[0]) > 2e-6 || std::abs(expected[1] - value[1]) > 2e-6) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
	} \
}

#define ASSERT_EXCEPTION_THROWN(expected, action) {\
	try {\
		action; \
		std::ostringstream oss; \
		oss << "\t - expected exception " << #expected << " was not thrown" << std::endl; \
		throw SynfigTestException{__FUNCTION__, __LINE__, oss.str()}; \
	} catch (expected &ex) { \
	} \
}

#define ASSERT_EXCEPTION_NOT_THROWN(not_expected, action) {\
	try {\
		action; \
	} catch (not_expected &ex) { \
		std::ostringstream oss; \
		oss << "\t - it should not throw exception " << #not_expected << std::endl; \
		throw SynfigTestException{__FUNCTION__, __LINE__, oss.str()}; \
	} \
}

#define ASSERT_NO_EXCEPTION_THROWN(action) {\
	try {\
		action; \
	} catch (...) { \
		std::ostringstream oss; \
		oss << "\t - it should not throw any exception" << std::endl; \
		throw SynfigTestException{__FUNCTION__, __LINE__, oss.str()}; \
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
		std::ostringstream oss; \
		oss << "\t - expected signal " << #SIGNAL_NAME << " was not called" << std::endl; \
		throw SynfigTestException{__FUNCTION__, __LINE__, oss.str()}; \
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
		std::ostringstream oss; \
		oss << "\t - not-expected signal was called: " << #SIGNAL_NAME << std::endl; \
		throw SynfigTestException{__FUNCTION__, __LINE__, oss.str()}; \
	} \
}

#define TEST_FUNCTION(function_name) {\
	std::string error_msg; \
	try { \
		function_name(); \
		tst_statistics__.successes++; \
		std::cout << "."; \
	} catch (SynfigTestException& exc) { \
		tst_statistics__.failures++; \
		std::cout << "F"; \
		tst_statistics__.errors.push_back(exc); \
	} catch (std::logic_error& exc) { \
		error_msg = "<Unexpected std::logic_error thrown>: "; \
		error_msg += exc.what(); \
	} catch (std::runtime_error& exc) { \
		error_msg = "<Unexpected std::runtime_error thrown>: "; \
		error_msg += exc.what(); \
	} catch (std::exception& exc) { \
		error_msg = "<Unexpected std::exception thrown>: "; \
		error_msg += exc.what(); \
	} catch (...) { \
		error_msg = "<Unexpected and unknown exception thrown>"; \
	} \
\
	if (!error_msg.empty()) { \
		tst_statistics__.exception_thrown++; \
		std::cout << "X"; \
		tst_statistics__.errors.push_back({#function_name, -1, error_msg}); \
	} \
}

#define TEST_SUITE_BEGIN() \
	int tst_exit_status = 0; \
	{ \
		struct tst_statistics__ { \
			int successes = 0; \
			int failures = 0; \
			int exception_thrown = 0; \
			std::vector<SynfigTestException> errors; \
		} tst_statistics__; \
		{

#define TEST_SUITE_END() \
		} \
		std::cout << std::endl; \
		for (const auto& err : tst_statistics__.errors) { \
			if (err.line < 0) \
				synfig::error("ERROR:\n %s", err.function.c_str()); \
			else \
				synfig::warning("FAILURE:\n %s:%i", err.function.c_str(), err.line); \
			std::cerr << err.message << std::endl; \
		} \
		std::cerr << std::endl << "========================================================================" << std::endl; \
		if (tst_statistics__.exception_thrown) \
			synfig::error("Statistics:\n %i tests were interrupted by unexpected exceptions thrown.\n %i tests failed.\n %i successful tests)", \
				tst_statistics__.exception_thrown, tst_statistics__.failures, tst_statistics__.successes); \
		else if (tst_statistics__.failures) \
			synfig::warning("Statistics:\n %i tests failed.\n %i successful tests", \
				tst_statistics__.failures, tst_statistics__.successes); \
		else \
			synfig::info("Success (%i tests)", tst_statistics__.successes); \
		std::cerr << "========================================================================" << std::endl; \
		tst_exit_status = tst_statistics__.exception_thrown? 2 : (tst_statistics__.failures ? 1 : 0); \
	}

#endif // SYNFIG_TESTBASE_H
