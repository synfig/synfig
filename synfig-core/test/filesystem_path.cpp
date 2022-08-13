/* === S Y N F I G ========================================================= */
/*! \file filesystem_path.cpp
**  \brief Test synfig::filesystem::Path methods
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

#include <synfig/filesystem.h>

#include "test_base.h"

using namespace synfig::filesystem;

/* === P R O C E D U R E S ================================================= */

void
test_default_path_constructor_is_empty()
{
	Path p;
	ASSERT(p.empty())
}

void
test_path_with_empty_string_is_empty()
{
	Path p("");
	ASSERT(p.empty())
}

void
test_path_with_non_empty_string_is_not_empty()
{
	Path p("file");
	ASSERT_FALSE(p.empty())
}

void
test_fetch_path_filename_regular_case()
{
	Path p1("/foo/bar.txt");
	ASSERT(p1.has_filename())
	ASSERT_EQUAL("bar.txt", p1.filename().u8string());

	Path p2("foo/bar.txt");
	ASSERT(p2.has_filename())
	ASSERT_EQUAL("bar.txt", p2.filename().u8string());

	Path p3("bar.txt");
	ASSERT(p3.has_filename())
	ASSERT_EQUAL("bar.txt", p3.filename().u8string());
}

void
test_does_not_fetch_path_filename_when_path_is_empty()
{
	Path p;
	ASSERT_FALSE(p.has_filename())
	ASSERT_EQUAL("", p.filename().u8string());
}

void
test_does_not_fetch_path_filename_when_path_is_root()
{
	Path p1("/");
	ASSERT_FALSE(p1.has_filename())
	ASSERT_EQUAL("", p1.filename().u8string());

	Path p2("C:\\");
	ASSERT_FALSE(p2.has_filename())
	ASSERT_EQUAL("", p2.filename().u8string());
}

void
test_fetch_path_filename_when_path_does_not_have_any_extension()
{
	Path p1("/foo/bar");
	ASSERT(p1.has_filename())
	ASSERT_EQUAL("bar", p1.filename().u8string());

	Path p2("foo/bar");
	ASSERT(p2.has_filename())
	ASSERT_EQUAL("bar", p2.filename().u8string());
}

void
test_fetch_path_filename_when_path_filename_starts_with_dot_char()
{
	Path p1("/foo/.bar");
	ASSERT(p1.has_filename())
	ASSERT_EQUAL(".bar", p1.filename().u8string());

	Path p2("foo/.bar");
	ASSERT(p2.has_filename())
	ASSERT_EQUAL(".bar", p2.filename().u8string());}

void
test_fetch_path_filename_when_path_filename_ends_with_dot_char()
{
	Path p("/foo/bar.");
	ASSERT(p.has_filename())
	ASSERT_EQUAL("bar.", p.filename().u8string());
}

void
test_fetch_path_filename_when_is_dot_filename()
{
	Path p1("/foo/.");
	ASSERT(p1.has_filename())
	ASSERT_EQUAL(".", p1.filename().u8string());

	Path p2(".");
	ASSERT(p2.has_filename())
	ASSERT_EQUAL(".", p2.filename().u8string());
}

void
test_fetch_path_filename_when_is_dot_dot_filename()
{
	Path p1("/foo/..");
	ASSERT(p1.has_filename())
	ASSERT_EQUAL("..", p1.filename().u8string());

	Path p2("..");
	ASSERT(p2.has_filename())
	ASSERT_EQUAL("..", p2.filename().u8string());
}

void
test_fetch_path_stem_regular_case()
{
	Path p1("/foo/bar.txt");
	ASSERT(p1.has_stem())
	ASSERT_EQUAL("bar", p1.stem().u8string());

	Path p2("foo/bar.txt");
	ASSERT(p2.has_stem())
	ASSERT_EQUAL("bar", p2.stem().u8string());
}

void
test_does_not_fetch_path_stem_when_path_is_empty()
{
	Path p;
	ASSERT_FALSE(p.has_stem())
	ASSERT_EQUAL("", p.stem().u8string());
}

void
test_does_not_fetch_path_stem_when_path_is_root()
{
	Path p1("/");
	ASSERT_FALSE(p1.has_stem())
	ASSERT_EQUAL("", p1.stem().u8string());

	Path p2("C:\\");
	ASSERT_FALSE(p2.has_stem())
	ASSERT_EQUAL("", p2.stem().u8string());
}

void
test_fetch_path_stem_when_path_does_not_have_any_extension()
{
	Path p1("/foo/bar");
	ASSERT(p1.has_stem())
	ASSERT_EQUAL("bar", p1.stem().u8string());

	Path p2("foo/bar");
	ASSERT(p2.has_stem())
	ASSERT_EQUAL("bar", p2.stem().u8string());
}

void
test_fetch_path_stem_when_path_filename_starts_with_dot_char()
{
	Path p1("/foo/.bar");
	ASSERT(p1.has_stem())
	ASSERT_EQUAL(".bar", p1.stem().u8string());

	Path p2("foo/.bar");
	ASSERT(p2.has_stem())
	ASSERT_EQUAL(".bar", p2.stem().u8string());
}

void
test_fetch_path_stem_when_path_filename_ends_with_dot_char()
{
	Path p("/foo/bar.");
	ASSERT(p.has_stem())
	ASSERT_EQUAL("bar", p.stem().u8string());
}

void
test_fetch_path_stem_when_is_dot_filename()
{
	Path p1("/foo/.");
	ASSERT(p1.has_stem())
	ASSERT_EQUAL(".", p1.stem().u8string());

	Path p2(".");
	ASSERT(p2.has_stem())
	ASSERT_EQUAL(".", p2.stem().u8string());
}

void
test_fetch_path_stem_when_is_dot_dot_filename()
{
	Path p1("/foo/..");
	ASSERT(p1.has_stem())
	ASSERT_EQUAL("..", p1.stem().u8string());

	Path p2("..");
	ASSERT(p2.has_stem())
	ASSERT_EQUAL("..", p2.stem().u8string());
}

void
test_fetch_path_stem_correctly_when_intermediate_path_has_dot_char()
{
	Path p1("/foo/bar.txt/play.dd");
	ASSERT(p1.has_stem())
	ASSERT_EQUAL("play", p1.stem().u8string());

	Path p2("/foo/bar.txt/play.");
	ASSERT(p2.has_stem())
	ASSERT_EQUAL("play", p2.stem().u8string());

	Path p3("/foo/bar.txt/play");
	ASSERT(p3.has_stem())
	ASSERT_EQUAL("play", p3.stem().u8string());
}

void
test_fetch_path_stem_correctly_when_weird_case_dotchar_dotchar_extension()
{
	Path p1("/foo/..weird");
	ASSERT(p1.has_stem())
	ASSERT_EQUAL(".", p1.stem().u8string());

	Path p2("..weird");
	ASSERT(p2.has_stem())
	ASSERT_EQUAL(".", p2.stem().u8string());
}
void
test_fetch_path_extension_regular_case()
{
	Path p("/foo/bar.txt");
	ASSERT(p.has_extension())
	ASSERT_EQUAL(".txt", p.extension().u8string());
}

void
test_does_not_fetch_path_extension_when_path_is_empty()
{
	Path p;
	ASSERT_FALSE(p.has_extension())
	ASSERT_EQUAL("", p.extension().u8string());
}

void
test_does_not_fetch_path_extension_when_path_is_root()
{
	Path p1("/");
	ASSERT_FALSE(p1.has_extension())
	ASSERT_EQUAL("", p1.extension().u8string());

	Path p2("C:\\");
	ASSERT_FALSE(p2.has_extension())
	ASSERT_EQUAL("", p2.extension().u8string());
}

void
test_does_not_fetch_path_extension_when_path_does_not_have_any_extension()
{
	Path p("/foo/bar");
	ASSERT_FALSE(p.has_extension())
	ASSERT_EQUAL("", p.extension().u8string());
}

void
test_does_not_fetch_path_extension_when_path_filename_starts_with_dot_char()
{
	Path p("/foo/.bar");
	ASSERT_FALSE(p.has_extension())
	ASSERT_EQUAL("", p.extension().u8string());
}

void
test_fetch_path_extension_when_path_filename_ends_with_dot_char()
{
	Path p("/foo/bar.");
	ASSERT(p.has_extension())
	ASSERT_EQUAL(".", p.extension().u8string());
}

void
test_does_not_fetch_path_extension_when_is_dot_filename()
{
	Path p1("/foo/.");
	ASSERT_FALSE(p1.has_extension())
	ASSERT_EQUAL("", p1.extension().u8string());

	Path p2(".");
	ASSERT_FALSE(p2.has_extension())
	ASSERT_EQUAL("", p2.extension().u8string());
}

void
test_does_not_fetch_path_extension_when_is_dot_dot_filename()
{
	Path p1("/foo/..");
	ASSERT_FALSE(p1.has_extension())
	ASSERT_EQUAL("", p1.extension().u8string());

	Path p2("..");
	ASSERT_FALSE(p2.has_extension())
	ASSERT_EQUAL("", p2.extension().u8string());
}

void
test_fetch_path_extension_correctly_when_intermediate_path_has_dot_char()
{
	Path p1("/foo/bar.txt/play.dd");
	ASSERT(p1.has_extension())
	ASSERT_EQUAL(".dd", p1.extension().u8string());

	Path p2("/foo/bar.txt/play.");
	ASSERT(p2.has_extension())
	ASSERT_EQUAL(".", p2.extension().u8string());

	Path p3("/foo/bar.txt/play");
	ASSERT_FALSE(p3.has_extension())
	ASSERT_EQUAL("", p3.extension().u8string());
}

void
test_fetch_path_extension_correctly_when_weird_case_dotchar_dotchar_extension()
{
	Path p1("/foo/..weird");
	ASSERT(p1.has_extension())
	ASSERT_EQUAL(".weird", p1.extension().u8string());

	Path p2("..weird");
	ASSERT(p2.has_extension())
	ASSERT_EQUAL(".weird", p2.extension().u8string());
}

/* === E N T R Y P O I N T ================================================= */

int main() {

	TEST_SUITE_BEGIN()

	TEST_FUNCTION(test_default_path_constructor_is_empty)
	TEST_FUNCTION(test_path_with_empty_string_is_empty)
	TEST_FUNCTION(test_path_with_non_empty_string_is_not_empty)

	TEST_FUNCTION(test_fetch_path_filename_regular_case)
	TEST_FUNCTION(test_does_not_fetch_path_filename_when_path_is_empty)
	TEST_FUNCTION(test_does_not_fetch_path_filename_when_path_is_root)
	TEST_FUNCTION(test_fetch_path_filename_when_path_does_not_have_any_extension)
	TEST_FUNCTION(test_fetch_path_filename_when_path_filename_starts_with_dot_char)
	TEST_FUNCTION(test_fetch_path_filename_when_path_filename_ends_with_dot_char)
	TEST_FUNCTION(test_fetch_path_filename_when_is_dot_filename)
	TEST_FUNCTION(test_fetch_path_filename_when_is_dot_dot_filename)

	TEST_FUNCTION(test_fetch_path_stem_regular_case)
	TEST_FUNCTION(test_does_not_fetch_path_stem_when_path_is_empty)
	TEST_FUNCTION(test_does_not_fetch_path_stem_when_path_is_root)
	TEST_FUNCTION(test_fetch_path_stem_when_path_does_not_have_any_extension)
	TEST_FUNCTION(test_fetch_path_stem_when_path_filename_starts_with_dot_char)
	TEST_FUNCTION(test_fetch_path_stem_when_path_filename_ends_with_dot_char)
	TEST_FUNCTION(test_fetch_path_stem_when_is_dot_filename)
	TEST_FUNCTION(test_fetch_path_stem_when_is_dot_dot_filename)
	TEST_FUNCTION(test_fetch_path_stem_correctly_when_intermediate_path_has_dot_char)
	TEST_FUNCTION(test_fetch_path_stem_correctly_when_weird_case_dotchar_dotchar_extension)

	TEST_FUNCTION(test_fetch_path_extension_regular_case)
	TEST_FUNCTION(test_does_not_fetch_path_extension_when_path_is_empty)
	TEST_FUNCTION(test_does_not_fetch_path_extension_when_path_is_root)
	TEST_FUNCTION(test_does_not_fetch_path_extension_when_path_does_not_have_any_extension)
	TEST_FUNCTION(test_does_not_fetch_path_extension_when_path_filename_starts_with_dot_char)
	TEST_FUNCTION(test_fetch_path_extension_when_path_filename_ends_with_dot_char)
	TEST_FUNCTION(test_does_not_fetch_path_extension_when_is_dot_filename)
	TEST_FUNCTION(test_does_not_fetch_path_extension_when_is_dot_dot_filename)
	TEST_FUNCTION(test_fetch_path_extension_correctly_when_intermediate_path_has_dot_char)
	TEST_FUNCTION(test_fetch_path_extension_correctly_when_weird_case_dotchar_dotchar_extension)

	TEST_SUITE_END()

	return tst_exit_status;
}

