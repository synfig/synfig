/*!	\file test/test_json.cpp
**	\brief Tests for JSON parser
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2025 Synfig authors
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

#include "test_base.h"
#include <gui/json.h>

void
test_parse_empty_object()
{
	auto d = JSON::Parser::parse("{}");
	ASSERT_EQUAL(0, d.size());
}

void
test_parse_simple_object_with_string_value()
{
	auto d = JSON::Parser::parse("{\"key\":\"value\"}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("value", d.at("key"));
}

void
test_parse_simple_object_with_int_value()
{
	auto d = JSON::Parser::parse("{\"key\":32}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("32", d.at("key"));
}

void
test_parse_simple_object_with_negative_int_value()
{
	auto d = JSON::Parser::parse("{\"key\":-32}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("-32", d.at("key"));
}

void
test_parse_simple_object_with_boolean_value()
{
	auto d = JSON::Parser::parse("{\"key\":true}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("true", d.at("key"));

	d = JSON::Parser::parse("{\"key\":false}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("false", d.at("key"));
}

void
test_parse_simple_object_with_null_value()
{
	auto d = JSON::Parser::parse("{\"key\":null}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("null", d.at("key"));
}

void
test_parse_simple_object_with_whitespaces()
{
	auto d = JSON::Parser::parse(" \t\n { \t\n \"key\" \t\n : \t\r null \t\r } \t\t");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("null", d.at("key"));
}

void
test_parse_simple_object_with_whitespaces_in_key()
{
	auto d = JSON::Parser::parse("{\"key \":null} \t\t");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("null", d.at("key "));

	d = JSON::Parser::parse("{\"ke y \":null} \t\t");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("null", d.at("ke y "));

	d = JSON::Parser::parse("{\" ke y \":null} \t\t");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("null", d.at(" ke y "));
}

void
test_parse_object_with_two_keys()
{
	auto d = JSON::Parser::parse("{\"key\":\"Hello\",\"door\":\"World\"}");
	ASSERT_EQUAL(2, d.size());
	ASSERT_EQUAL("Hello", d.at("key"));
	ASSERT_EQUAL("World", d.at("door"));
}

void
test_parse_object_with_two_keys_with_whitespaces_between_them()
{
	auto d = JSON::Parser::parse("{\"key\":\"Hello\" \t , \t \"door\":\"World\"}");
	ASSERT_EQUAL(2, d.size());
	ASSERT_EQUAL("Hello", d.at("key"));
	ASSERT_EQUAL("World", d.at("door"));
}

void
test_does_not_parse_object_with_extra_comma()
{
	ASSERT_EXCEPTION_THROWN(std::runtime_error, JSON::Parser::parse("{\"key\":\"Hello\",,\"door\":\"World\"}"));
}

void
test_does_not_parse_object_with_extra_comma_at_end()
{
	ASSERT_EXCEPTION_THROWN(std::runtime_error, JSON::Parser::parse("{\"key\":\"Hello\",\"door\":\"World\",}"));
}

void
test_ignore_array_in_object()
{
	auto d = JSON::Parser::parse("{\"key\":[]}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("", d.at("key"));

	d = JSON::Parser::parse("{\"key\":[1,2,3,4]}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("", d.at("key"));

	d = JSON::Parser::parse("{\"key\": [1, 2,3,\"hi\"] }");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("", d.at("key"));

	d = JSON::Parser::parse("{\"key\":[1, 2,3,[]]}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("", d.at("key"));
}

void
test_ignore_nested_object()
{
	auto d = JSON::Parser::parse("{\"key\":{}}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("", d.at("key"));

	d = JSON::Parser::parse("{\"key\":{\"inner\":\"value\"}}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("", d.at("key"));

	d = JSON::Parser::parse("{\"key\":{\"inner\":[]}}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("", d.at("key"));

	d = JSON::Parser::parse("{\"key\":{\"inner\":{\"even more inner\":\"value\"}}}");
	ASSERT_EQUAL(1, d.size());
	ASSERT_EQUAL("", d.at("key"));
}

void
test_parse_simple_string()
{
	auto d = JSON::Parser::parse("{\"key\":\"Hello, World\"}");
	ASSERT_EQUAL("Hello, World", d.at("key"));
}

void
test_parse_string_basic_backslash_escape()
{
	auto d = JSON::Parser::parse("{\"key\":\"Hello, \\\"World\\\"\"}");
	ASSERT_EQUAL("Hello, \"World\"", d.at("key"));

	d = JSON::Parser::parse("{\"key\":\"Testing\\\"\\\\\\/\\b\\f\\n\\r\\t\"}");
	ASSERT_EQUAL("Testing\"\\/\b\f\n\r\t", d.at("key"));
}

void
test_parse_string_basic_backslash_escape_at_the_beginning_of_string()
{
	auto d = JSON::Parser::parse("{\"key\":\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"}");
	ASSERT_EQUAL("\"\\/\b\f\n\r\t", d.at("key"));
}

void
test_parse_string_incomplete_basic_backslash_escape()
{
	ASSERT_EXCEPTION_THROWN(std::runtime_error, JSON::Parser::parse("{\"key\":\"Incomplete\\\"}"));
}

void
test_parse_string_inexistent_basic_backslash_escape()
{
	auto d = JSON::Parser::parse("{\"key\":\"In\\existent\"}");
	ASSERT_EQUAL(u8"In�xistent", d.at("key"));
}

int main(int argc, const char* argv[])
{
	TEST_SUITE_BEGIN();

	TEST_FUNCTION(test_parse_empty_object);
	TEST_FUNCTION(test_parse_simple_object_with_string_value);
	TEST_FUNCTION(test_parse_simple_object_with_int_value);
	TEST_FUNCTION(test_parse_simple_object_with_negative_int_value);
	TEST_FUNCTION(test_parse_simple_object_with_boolean_value);
	TEST_FUNCTION(test_parse_simple_object_with_null_value);
	TEST_FUNCTION(test_parse_simple_object_with_string_value);
	TEST_FUNCTION(test_parse_simple_object_with_whitespaces);
	TEST_FUNCTION(test_parse_simple_object_with_whitespaces_in_key);
	TEST_FUNCTION(test_parse_object_with_two_keys);
	TEST_FUNCTION(test_parse_object_with_two_keys_with_whitespaces_between_them);
	TEST_FUNCTION(test_does_not_parse_object_with_extra_comma);
	TEST_FUNCTION(test_does_not_parse_object_with_extra_comma_at_end);

	TEST_FUNCTION(test_ignore_array_in_object);
	TEST_FUNCTION(test_ignore_nested_object);

	TEST_FUNCTION(test_parse_simple_string);
	TEST_FUNCTION(test_parse_string_basic_backslash_escape);
	TEST_FUNCTION(test_parse_string_basic_backslash_escape_at_the_beginning_of_string);
	TEST_FUNCTION(test_parse_string_incomplete_basic_backslash_escape);
	TEST_FUNCTION(test_parse_string_inexistent_basic_backslash_escape);

	TEST_SUITE_END();

	return tst_exit_status;
}
