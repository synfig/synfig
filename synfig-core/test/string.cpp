/* === S Y N F I G ========================================================= */
/*! \file string.cpp
**  \brief Test String helper methods
**
**  \legal
**  Copyright (c) 2022 Synfig contributors
**
**  This file is part of Synfig.
**
**  Synfig is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  Synfig is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**  \endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#include <synfig/string_helper.h>

#include "test_base.h"

using namespace synfig;

/* === P R O C E D U R E S ================================================= */

void
test_remove_trailing_zeroes_without_trailing_zeroes_returns_unchanged_string()
{
	std::string res = remove_trailing_zeroes("10.0305");
	ASSERT_EQUAL("10.0305", res)
}

void
test_remove_trailing_zeroes_without_trailing_zeroes_and_no_decimal_point_appends_dot_zero()
{
	std::string res = remove_trailing_zeroes("102");
	ASSERT_EQUAL("102.0", res)
}

void
test_remove_trailing_zeroes_with_trailing_zeroes_but_no_decimal_point_appends_dot_zero()
{
	std::string res = remove_trailing_zeroes("100");
	ASSERT_EQUAL("100.0", res)
}

void
test_remove_trailing_zeroes_without_trailing_zeroes_and_no_decimal_point_does_not_appends_dot_zero_if_requested()
{
	std::string res = remove_trailing_zeroes("102", false);
	ASSERT_EQUAL("102", res)
}

void
test_remove_trailing_zeroes_with_trailing_zeroes_but_no_decimal_point_does_not_appends_dot_zero_if_requested()
{
	std::string res = remove_trailing_zeroes("100", false);
	ASSERT_EQUAL("100", res)
}
void
test_remove_trailing_zeroes_with_real_trailing_zeroes_does_remove_them()
{
	std::string res = remove_trailing_zeroes("100.3000");
	ASSERT_EQUAL("100.3", res)
}

void
test_remove_trailing_zeroes_with_only_trailing_zeroes_keeps_the_decimal_point_by_default()
{
	std::string res = remove_trailing_zeroes("100.000");
	ASSERT_EQUAL("100.0", res)
}

void
test_left_trim_does_remove_all_leading_whitespaces()
{
	ASSERT_EQUAL("Something", left_trim(" \t\v\n\rSomething"))
}

void
test_left_trim_does_not_remove_any_trailing_whitespaces()
{
	ASSERT_EQUAL("Something \t\v\n\r", left_trim("Something \t\v\n\r"))
}

void
test_left_trim_does_not_remove_any_intermediate_whitespaces()
{
	ASSERT_EQUAL("Some \t\v\n\rthing", left_trim("Some \t\v\n\rthing"))
}

void
test_left_trim_keeps_string_unmodified_if_no_leading_whitespace()
{
	ASSERT_EQUAL("Something", left_trim("Something"))
}

void
test_right_trim_does_remove_all_trailing_whitespaces()
{
	ASSERT_EQUAL("Something", right_trim("Something \t\v\n\r"))
}

void
test_right_trim_does_not_remove_any_leading_whitespaces()
{
	ASSERT_EQUAL(" \t\v\n\rSomething", right_trim(" \t\v\n\rSomething"))
}

void
test_right_trim_does_not_remove_any_intermediate_whitespaces()
{
	ASSERT_EQUAL("Some \t\v\n\rthing", right_trim("Some \t\v\n\rthing"))
}

void
test_right_trim_keeps_string_unmodified_if_no_trailing_whitespace()
{
	ASSERT_EQUAL("Something", right_trim("Something"))
}

void
test_trim_does_remove_all_leading_or_trailing_whitespaces()
{
	ASSERT_EQUAL("Something", trim(" \t\v\n\rSomething \t\v\n\r"))
}

void
test_trim_does_not_remove_any_intermediate_whitespaces()
{
	ASSERT_EQUAL("Some \t\v\n\rthing", trim("Some \t\v\n\rthing"))
}

void
test_trim_keeps_string_unmodified_if_no_leading_or_trailing_whitespace()
{
	ASSERT_EQUAL("Something", trim("Something"))
}

void
test_strprintf()
{
	char mystring[80]{"My formatted string!"};

	ASSERT_EQUAL("This is a test of >>My formatted string!<<.", strprintf("This is a test of >>%s<<.", mystring));
}

void
test_strscanf()
{
	std::string myinputstring{"5 6.75 George 7"};
	int i,i2;
	float f;
	char mystring[80];

	ASSERT_EQUAL(4, strscanf(myinputstring,"%d %f %s %d",&i, &f, mystring, &i2));
	ASSERT_EQUAL(5, i);
	ASSERT_EQUAL(6.75, f);
	ASSERT_EQUAL(std::string("George"), mystring);
	ASSERT_EQUAL(7, i2);
}

void
test_stratof()
{
	ASSERT_EQUAL(32.5849, stratof(std::string("32.5849")));
}

/* === E N T R Y P O I N T ================================================= */

int main() {

	TEST_SUITE_BEGIN()

	TEST_FUNCTION(test_remove_trailing_zeroes_without_trailing_zeroes_returns_unchanged_string)
	TEST_FUNCTION(test_remove_trailing_zeroes_without_trailing_zeroes_and_no_decimal_point_appends_dot_zero)
	TEST_FUNCTION(test_remove_trailing_zeroes_with_trailing_zeroes_but_no_decimal_point_appends_dot_zero)
	TEST_FUNCTION(test_remove_trailing_zeroes_without_trailing_zeroes_and_no_decimal_point_does_not_appends_dot_zero_if_requested)
	TEST_FUNCTION(test_remove_trailing_zeroes_with_trailing_zeroes_but_no_decimal_point_does_not_appends_dot_zero_if_requested)
	TEST_FUNCTION(test_remove_trailing_zeroes_with_real_trailing_zeroes_does_remove_them)
	TEST_FUNCTION(test_remove_trailing_zeroes_with_only_trailing_zeroes_keeps_the_decimal_point_by_default)

	TEST_FUNCTION(test_left_trim_does_remove_all_leading_whitespaces)
	TEST_FUNCTION(test_left_trim_does_not_remove_any_trailing_whitespaces)
	TEST_FUNCTION(test_left_trim_does_not_remove_any_intermediate_whitespaces)
	TEST_FUNCTION(test_left_trim_keeps_string_unmodified_if_no_leading_whitespace)

	TEST_FUNCTION(test_right_trim_does_remove_all_trailing_whitespaces)
	TEST_FUNCTION(test_right_trim_does_not_remove_any_leading_whitespaces)
	TEST_FUNCTION(test_right_trim_does_not_remove_any_intermediate_whitespaces)
	TEST_FUNCTION(test_right_trim_keeps_string_unmodified_if_no_trailing_whitespace)

	TEST_FUNCTION(test_trim_does_remove_all_leading_or_trailing_whitespaces)
	TEST_FUNCTION(test_trim_does_not_remove_any_intermediate_whitespaces)
	TEST_FUNCTION(test_trim_keeps_string_unmodified_if_no_leading_or_trailing_whitespace)

	TEST_FUNCTION(test_strprintf)
	TEST_FUNCTION(test_strscanf)
	TEST_FUNCTION(test_stratof)

	TEST_SUITE_END()

	return tst_exit_status;
}

