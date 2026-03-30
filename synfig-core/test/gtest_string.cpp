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

#include <gtest/gtest.h>

using namespace synfig;

/* === P R O C E D U R E S ================================================= */

TEST(StringTest, test_remove_trailing_zeroes_without_trailing_zeroes_returns_unchanged_string)
{
	std::string res = remove_trailing_zeroes("10.0305");
	EXPECT_EQ("10.0305", res)
}

TEST(StringTest, test_remove_trailing_zeroes_without_trailing_zeroes_and_no_decimal_point_appends_dot_zero)
{
	std::string res = remove_trailing_zeroes("102");
	EXPECT_EQ("102.0", res)
}

TEST(StringTest, test_remove_trailing_zeroes_with_trailing_zeroes_but_no_decimal_point_appends_dot_zero)
{
	std::string res = remove_trailing_zeroes("100");
	EXPECT_EQ("100.0", res)
}

TEST(StringTest, test_remove_trailing_zeroes_without_trailing_zeroes_and_no_decimal_point_does_not_appends_dot_zero_if_requested)
{
	std::string res = remove_trailing_zeroes("102", false);
	EXPECT_EQ("102", res)
}

TEST(StringTest, test_remove_trailing_zeroes_with_trailing_zeroes_but_no_decimal_point_does_not_appends_dot_zero_if_requested)
{
	std::string res = remove_trailing_zeroes("100", false);
	EXPECT_EQ("100", res)
}
TEST(StringTest, test_remove_trailing_zeroes_with_real_trailing_zeroes_does_remove_them)
{
	std::string res = remove_trailing_zeroes("100.3000");
	EXPECT_EQ("100.3", res)
}

TEST(StringTest, test_remove_trailing_zeroes_with_only_trailing_zeroes_keeps_the_decimal_point_by_default)
{
	std::string res = remove_trailing_zeroes("100.000");
	EXPECT_EQ("100.0", res)
}

TEST(StringTest, test_left_trim_does_remove_all_leading_whitespaces)
{
	EXPECT_EQ("Something", left_trim(" \t\v\n\rSomething"))
}

TEST(StringTest, test_left_trim_does_not_remove_any_trailing_whitespaces)
{
	EXPECT_EQ("Something \t\v\n\r", left_trim("Something \t\v\n\r"))
}

TEST(StringTest, test_left_trim_does_not_remove_any_intermediate_whitespaces)
{
	EXPECT_EQ("Some \t\v\n\rthing", left_trim("Some \t\v\n\rthing"))
}

TEST(StringTest, test_left_trim_keeps_string_unmodified_if_no_leading_whitespace)
{
	EXPECT_EQ("Something", left_trim("Something"))
}

TEST(StringTest, test_right_trim_does_remove_all_trailing_whitespaces)
{
	EXPECT_EQ("Something", right_trim("Something \t\v\n\r"))
}

TEST(StringTest, test_right_trim_does_not_remove_any_leading_whitespaces)
{
	EXPECT_EQ(" \t\v\n\rSomething", right_trim(" \t\v\n\rSomething"))
}

TEST(StringTest, test_right_trim_does_not_remove_any_intermediate_whitespaces)
{
	EXPECT_EQ("Some \t\v\n\rthing", right_trim("Some \t\v\n\rthing"))
}

TEST(StringTest, test_right_trim_keeps_string_unmodified_if_no_trailing_whitespace)
{
	EXPECT_EQ("Something", right_trim("Something"))
}

TEST(StringTest, test_trim_does_remove_all_leading_or_trailing_whitespaces)
{
	EXPECT_EQ("Something", trim(" \t\v\n\rSomething \t\v\n\r"))
}

TEST(StringTest, test_trim_does_not_remove_any_intermediate_whitespaces)
{
	EXPECT_EQ("Some \t\v\n\rthing", trim("Some \t\v\n\rthing"))
}

TEST(StringTest, test_trim_keeps_string_unmodified_if_no_leading_or_trailing_whitespace)
{
	EXPECT_EQ("Something", trim("Something"))
}

TEST(StringTest, test_strprintf)
{
	char mystring[80]{"My formatted string!"};

	EXPECT_EQ("This is a test of >>My formatted string!<<.", strprintf("This is a test of >>%s<<.", mystring));
}

TEST(StringTest, test_strscanf)
{
	std::string myinputstring{"5 6.75 George 7"};
	int i,i2;
	float f;
	char mystring[80];

	EXPECT_EQ(4, strscanf(myinputstring,"%d %f %s %d",&i, &f, mystring, &i2));
	EXPECT_EQ(5, i);
	EXPECT_EQ(6.75, f);
	EXPECT_EQ(std::string("George"), mystring);
	EXPECT_EQ(7, i2);
}

TEST(StringTest, test_stratof)
{
	EXPECT_EQ(32.5849, stratof(std::string("32.5849")));
}
