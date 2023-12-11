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

#include <synfig/filesystem_path.h>

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
test_append_path_simple_case()
{
	ASSERT_EQUAL("a/b", (Path("a") / Path("b")).u8string())
	ASSERT_EQUAL("/foo/bar/", (Path("/foo") / Path("bar/")).u8string())
}

void
test_append_path_does_not_duplicate_slash()
{
	ASSERT_EQUAL("a/b", (Path("a/") / Path("b")).u8string())
	ASSERT_EQUAL("/bar/", (Path("/") / Path("bar/")).u8string())
}

void
test_append_path_with_root_path_replaces()
{
	ASSERT_EQUAL("/b", (Path("a") / Path("/b")).u8string())
	ASSERT_EQUAL("/bar/", (Path("/foo") / Path("/bar/")).u8string())
}

void
test_append_path_cppreference_examples()
{
	// https://en.cppreference.com/w/cpp/filesystem/path/append
	ASSERT_EQUAL("\\\\host/foo", (Path("\\\\host")  / Path("foo")).u8string())
	ASSERT_EQUAL("\\\\host/foo", (Path("\\\\host/") / Path("foo")).u8string())

	// On POSIX,
	ASSERT_EQUAL("foo/", (Path("foo") / Path("")).u8string())
	ASSERT_EQUAL("/bar", (Path("foo") / Path("/bar")).u8string())

#ifdef _WIN32
	// On Windows,
	ASSERT_EQUAL("C:/bar", (Path("foo") / Path("C:/bar")).u8string())
	ASSERT_EQUAL("C:", (Path("foo") / Path("C:")).u8string())
	ASSERT_EQUAL("C:", (Path("C:") / Path("")).u8string())
	ASSERT_EQUAL("C:/bar", (Path("C:foo") / Path("/bar")).u8string())
	ASSERT_EQUAL("C:foo/bar", (Path("C:foo") / Path("C:bar")).u8string())
#endif
}

void
test_concat_path_cppreference_examples()
{
	Path p1; // an empty path
	p1 += std::string("var"); // does not insert a separator
	ASSERT_EQUAL("var", p1.u8string())
	ASSERT_EQUAL("varlib", (p1 + std::string("lib")).u8string())
	p1 += std::string("lib");
	p1.concat("123456");
	ASSERT_EQUAL("varlib123456", p1.u8string())
}

void
test_remove_filename()
{
	ASSERT_EQUAL("foo/", Path("foo/bar").remove_filename().u8string())
	ASSERT_EQUAL("foo/", Path("foo/").remove_filename().u8string())
	ASSERT_EQUAL("/", Path("/foo").remove_filename().u8string())
	ASSERT_EQUAL("/", Path("/").remove_filename().u8string())
}

void
test_replace_filename()
{
	ASSERT_EQUAL("/bar", Path("/foo").replace_filename(Path("bar")).u8string())
	ASSERT_EQUAL("/bar", Path("/").replace_filename(Path("bar")).u8string())
	ASSERT_EQUAL("bar", Path("foo").replace_filename(Path("bar")).u8string())
	ASSERT_EQUAL("pub", Path("").replace_filename(Path("pub")).u8string())
}

void
test_replace_extension()
{
	// examples from https://en.cppreference.com/w/cpp/filesystem/path/replace_extension
	ASSERT_EQUAL("/foo/bar.png", Path("/foo/bar.jpg").replace_extension(Path(".png")).u8string())
	ASSERT_EQUAL("/foo/bar.png", Path("/foo/bar.jpg").replace_extension(Path("png")).u8string())
	ASSERT_EQUAL("/foo/bar.",    Path("/foo/bar.jpg").replace_extension(Path(".")).u8string())
	ASSERT_EQUAL("/foo/bar",     Path("/foo/bar.jpg").replace_extension(Path("")).u8string())
	ASSERT_EQUAL("/foo/bar.png", Path("/foo/bar.").replace_extension(Path("png")).u8string())
	ASSERT_EQUAL("/foo/bar.png", Path("/foo/bar").replace_extension(Path(".png")).u8string())
	ASSERT_EQUAL("/foo/bar.png", Path("/foo/bar").replace_extension(Path("png")).u8string())
	ASSERT_EQUAL("/foo/bar.",    Path("/foo/bar").replace_extension(Path(".")).u8string())
	ASSERT_EQUAL("/foo/bar",     Path("/foo/bar").replace_extension(Path("")).u8string())
	ASSERT_EQUAL("/foo/..png",   Path("/foo/.").replace_extension(Path(".png")).u8string())
	ASSERT_EQUAL("/foo/..png",   Path("/foo/.").replace_extension(Path("png")).u8string())
	ASSERT_EQUAL("/foo/..",      Path("/foo/.").replace_extension(Path(".")).u8string())
	ASSERT_EQUAL("/foo/.",       Path("/foo/.").replace_extension(Path("")).u8string())
	ASSERT_EQUAL("/foo/.png",    Path("/foo/").replace_extension(Path(".png")).u8string())
	ASSERT_EQUAL("/foo/.png",    Path("/foo/").replace_extension(Path("png")).u8string())
}

void
test_add_suffix()
{
	ASSERT_EQUAL("/foo/bar-000.jpg", Path("/foo/bar.jpg").add_suffix("-000").u8string())
	ASSERT_EQUAL("foo/bar-000.jpg", Path("foo/bar.jpg").add_suffix("-000").u8string())
	ASSERT_EQUAL("/bar-000.jpg", Path("/bar.jpg").add_suffix("-000").u8string())
	ASSERT_EQUAL("bar-000.jpg", Path("bar.jpg").add_suffix("-000").u8string())

	ASSERT_EQUAL(".hidden-000", Path(".hidden").add_suffix("-000").u8string())
	ASSERT_EQUAL(".hidden-000.txt", Path(".hidden.txt").add_suffix("-000").u8string())

	ASSERT_EQUAL("/foo/bar-000.", Path("/foo/bar.").add_suffix("-000").u8string())
	ASSERT_EQUAL("foo/bar-000.", Path("foo/bar.").add_suffix("-000").u8string())
	ASSERT_EQUAL("/bar-000.", Path("/bar.").add_suffix("-000").u8string())
	ASSERT_EQUAL("bar-000.", Path("bar.").add_suffix("-000").u8string())

	ASSERT_EQUAL("/foo/bar-000", Path("/foo/bar").add_suffix("-000").u8string())
	ASSERT_EQUAL("foo/bar-000", Path("foo/bar").add_suffix("-000").u8string())
	ASSERT_EQUAL("/bar-000", Path("/bar").add_suffix("-000").u8string())
	ASSERT_EQUAL("bar-000", Path("bar").add_suffix("-000").u8string())
	ASSERT_EQUAL("-000", Path().add_suffix("-000").u8string())

	ASSERT_EQUAL("bar-000-001.jpg", Path("bar.jpg").add_suffix("-000").add_suffix("-001").u8string())
}

void
test_try_to_add_suffix_to_dot_file_does_not_work()
{
	ASSERT_EQUAL(".", Path(".").add_suffix("-000").u8string())
	ASSERT_EQUAL("/.", Path("/.").add_suffix("-000").u8string())
	ASSERT_EQUAL("a/.", Path("a/.").add_suffix("-000").u8string())
}

void
test_try_to_add_suffix_to_dotdot_file_does_not_work()
{
	ASSERT_EQUAL("..", Path("..").add_suffix("-000").u8string())
	ASSERT_EQUAL("/..", Path("/..").add_suffix("-000").u8string())
	ASSERT_EQUAL("a/..", Path("a/..").add_suffix("-000").u8string())
}

void
test_compare_path_both_empty()
{
	Path p1, p2;
	ASSERT_EQUAL(0, p1.compare(p2))
}

void
test_compare_path_different_root_name()
{
#ifdef _WIN32
	Path p1("C:");
	Path p2("C:");
	ASSERT_EQUAL(0, p1.compare(p2))

	Path p3("D:");
	ASSERT(0 > p1.compare(p3))
	ASSERT(0 < p3.compare(p1))

	Path p4("A:");
	ASSERT(0 < p1.compare(p4))
	ASSERT(0 > p4.compare(p1))
#endif
}

void
test_compare_path_different_root_directory()
{
	Path q1("/");
	Path q2("/");
	ASSERT_EQUAL(0, q1.compare(q2))

	Path q3("a");
	ASSERT(0 > q1.compare(q3))
	ASSERT(0 < q3.compare(q1))
#ifdef _WIN32
	Path p1("C:/");
	Path p2("C:/");
	ASSERT_EQUAL(0, p1.compare(p2))

	Path p3("C:");
	ASSERT(0 > p1.compare(p3))
	ASSERT(0 < p3.compare(p1))
#endif
}

void
test_compare_path_with_different_relative_path_size()
{
	Path q1("/a/b");
	Path q2("/a/b/c");
	ASSERT(0 > q1.compare(q2))
	ASSERT(0 < q2.compare(q1))

	ASSERT(0 > Path("/aa/ab/").compare(Path("/aa/ac/dd")))
	ASSERT(0 < Path("/aa/ac/dd").compare(Path("/aa/ab/")))

	ASSERT(0 > Path("/aa/ac/dd").compare(Path("/d")))
	ASSERT(0 < Path("/d").compare(Path("/aa/ac/dd")))

	ASSERT(0 > Path("aa/abb").compare(Path("aa/ac/dd")))
	ASSERT(0 < Path("aa/ac/dd").compare(Path("aa/abb")))
}

void
test_compare_path_both_has_same_relative_path_size()
{
	Path q1("/a/b");
	Path q2("/a/c");
	ASSERT_EQUAL(0, q1.compare(q1))
	ASSERT_EQUAL(-1, q1.compare(q2))
	ASSERT_EQUAL(1, q2.compare(q1))

	ASSERT(0 > Path("/aa/").compare(Path("/ac/")))
	ASSERT(0 < Path("/ac").compare(Path("/aa")))

	ASSERT(0 > Path("aa/abb").compare(Path("aa/c")))
	ASSERT(0 < Path("aa/c").compare(Path("aa/abb")))

	ASSERT(0 > Path("aa").compare(Path("ac")))
	ASSERT(0 < Path("ac").compare(Path("aa")))
}

void
test_fetch_path_root_name_regular_drive_on_windows()
{
#ifdef _WIN32
	Path p1("C:");
	ASSERT(p1.has_root_name())
	ASSERT_EQUAL("C:", p1.root_name().u8string());

	Path p2("D:/");
	ASSERT(p2.has_root_name())
	ASSERT_EQUAL("D:", p2.root_name().u8string());

	Path p3("E:/bar.txt");
	ASSERT(p3.has_root_name())
	ASSERT_EQUAL("E:", p3.root_name().u8string());

	Path p4("F:/foo/bar.txt");
	ASSERT(p4.has_root_name())
	ASSERT_EQUAL("F:", p4.root_name().u8string());
#endif
}

void
test_fetch_path_root_name_network_samba_folder()
{
	Path p1("\\\\host");
	ASSERT(p1.has_root_name())
	ASSERT_EQUAL("\\\\host", p1.root_name().u8string());

	Path p2("\\\\host2/");
	ASSERT(p2.has_root_name())
	ASSERT_EQUAL("\\\\host2", p2.root_name().u8string());

	Path p3("\\\\host3\\bar.txt");
	ASSERT(p3.has_root_name())
	ASSERT_EQUAL("\\\\host3", p3.root_name().u8string());

	Path p4("\\\\host4\\foo\\bar.txt");
	ASSERT(p4.has_root_name())
	ASSERT_EQUAL("\\\\host4", p4.root_name().u8string());
}

void
test_does_not_fetch_root_name_when_path_is_empty()
{
	Path p;
	ASSERT_FALSE(p.has_root_name())
	ASSERT_EQUAL("", p.root_name().u8string())
}

void
test_does_not_fetch_root_name_when_not_absolute()
{
	Path p1("foo");
	ASSERT_FALSE(p1.has_root_name())
	ASSERT_EQUAL("", p1.root_name().u8string())

	Path p2("foo.bar");
	ASSERT_FALSE(p2.has_root_name())
	ASSERT_EQUAL("", p2.root_name().u8string())

	Path p3("foo/bar");
	ASSERT_FALSE(p3.has_root_name())
	ASSERT_EQUAL("", p3.root_name().u8string())

	Path p4("foo/");
	ASSERT_FALSE(p4.has_root_name())
	ASSERT_EQUAL("", p4.root_name().u8string())
}

void
test_fetch_path_root_directory_regular_drive_on_windows()
{
#ifdef _WIN32
	Path p1("C:");
	ASSERT_FALSE(p1.has_root_directory())
	ASSERT_EQUAL("", p1.root_directory().u8string());

	Path p2("D:/");
	ASSERT(p2.has_root_directory())
	ASSERT_EQUAL("/", p2.root_directory().u8string());

	Path p3("E:/bar.txt");
	ASSERT(p3.has_root_directory())
	ASSERT_EQUAL("/", p3.root_directory().u8string());

	Path p4("F:/foo/bar.txt");
	ASSERT(p4.has_root_directory())
	ASSERT_EQUAL("/", p4.root_directory().u8string());
#endif
}

void
test_fetch_path_root_directory_network_samba_folder()
{
	Path p1("\\\\host");
	ASSERT_FALSE(p1.has_root_directory())
	ASSERT_EQUAL("", p1.root_directory().u8string());

	Path p2("\\\\host2/");
	ASSERT(p2.has_root_directory())
	ASSERT_EQUAL("/", p2.root_directory().u8string());

	Path p3("\\\\host3\\bar.txt");
	ASSERT(p3.has_root_directory())
	ASSERT_EQUAL("\\", p3.root_directory().u8string());

	Path p4("\\\\host4\\foo\\bar.txt");
	ASSERT(p4.has_root_directory())
	ASSERT_EQUAL("\\", p4.root_directory().u8string());
}

void
test_fetch_path_root_directory_prepended_slash()
{
	Path p1("/file");
	ASSERT(p1.has_root_directory())
	ASSERT_EQUAL("/", p1.root_directory().u8string());

	Path p2("/foo/bar");
	ASSERT(p2.has_root_directory())
	ASSERT_EQUAL("/", p2.root_directory().u8string());

	Path p3("/foo/bar.txt");
	ASSERT(p3.has_root_directory())
	ASSERT_EQUAL("/", p3.root_directory().u8string());

	Path p4("/");
	ASSERT(p4.has_root_directory())
	ASSERT_EQUAL("/", p4.root_directory().u8string());
}

void
test_does_not_fetch_root_directory_when_path_is_empty()
{
	Path p;
	ASSERT_FALSE(p.has_root_directory())
	ASSERT_EQUAL("", p.root_directory().u8string())
}

void
test_does_not_fetch_root_directory_when_not_absolute()
{
	Path p1("foo");
	ASSERT_FALSE(p1.has_root_directory())
	ASSERT_EQUAL("", p1.root_directory().u8string())

	Path p2("foo.bar");
	ASSERT_FALSE(p2.has_root_directory())
	ASSERT_EQUAL("", p2.root_directory().u8string())

	Path p3("foo/bar");
	ASSERT_FALSE(p3.has_root_directory())
	ASSERT_EQUAL("", p3.root_directory().u8string())

	Path p4("foo/");
	ASSERT_FALSE(p4.has_root_directory())
	ASSERT_EQUAL("", p4.root_directory().u8string())
}

void
test_fetch_relative_path_regular_case()
{
	Path p1("/foo/bar.txt");
	ASSERT(p1.has_relative_path())
	ASSERT_EQUAL("foo/bar.txt", p1.relative_path().u8string());

	Path p2("foo/bar.txt");
	ASSERT(p2.has_relative_path())
	ASSERT_EQUAL("foo/bar.txt", p2.relative_path().u8string());

	Path p3("bar.txt");
	ASSERT(p3.has_relative_path())
	ASSERT_EQUAL("bar.txt", p3.relative_path().u8string());

	Path p4("bar/");
	ASSERT(p4.has_relative_path())
	ASSERT_EQUAL("bar/", p4.relative_path().u8string());

	Path p5("/bar.txt");
	ASSERT(p5.has_relative_path())
	ASSERT_EQUAL("bar.txt", p5.relative_path().u8string());
}

void
test_fetch_relative_path_root_with_multiple_separators()
{
	Path p1("//foo/bar.txt");
	ASSERT(p1.has_relative_path())
	ASSERT_EQUAL("foo/bar.txt", p1.relative_path().u8string());
}

void
test_fetch_relative_path_regular_case_on_windows()
{
#ifdef _WIN32
	Path p1("C:/foo/bar.txt");
	ASSERT(p1.has_relative_path())
	ASSERT_EQUAL("foo/bar.txt", p1.relative_path().u8string());

	Path p2("D:/foo/bar.txt");
	ASSERT(p2.has_relative_path())
	ASSERT_EQUAL("foo/bar.txt", p2.relative_path().u8string());

	Path p3("E:/bar.txt");
	ASSERT(p3.has_relative_path())
	ASSERT_EQUAL("bar.txt", p3.relative_path().u8string());

	Path p4("F:\\bar/");
	ASSERT(p4.has_relative_path())
	ASSERT_EQUAL("bar/", p4.relative_path().u8string());
#endif
}

void
test_fetch_relative_path_with_network_samba_folder()
{
	Path p1("\\\\host/foo/bar.txt");
	ASSERT(p1.has_relative_path())
	ASSERT_EQUAL("foo/bar.txt", p1.relative_path().u8string());

	Path p2("\\\\host2/foo/bar.txt");
	ASSERT(p2.has_relative_path())
	ASSERT_EQUAL("foo/bar.txt", p2.relative_path().u8string());

	Path p3("\\\\host3/bar.txt");
	ASSERT(p3.has_relative_path())
	ASSERT_EQUAL("bar.txt", p3.relative_path().u8string());

	Path p4("\\\\host4\\bar/");
	ASSERT(p4.has_relative_path())
	ASSERT_EQUAL("bar/", p4.relative_path().u8string());
}

void
test_does_not_fetch_relative_path_when_path_is_empty()
{
	Path p;
	ASSERT_FALSE(p.has_relative_path())
	ASSERT_EQUAL("", p.relative_path().u8string())
}

void
test_does_not_fetch_relative_path_when_path_is_root()
{
	Path p0("/");
	ASSERT_FALSE(p0.has_relative_path())
	ASSERT_EQUAL("", p0.relative_path().u8string());

	Path p1("//");
	ASSERT_FALSE(p1.has_relative_path())
	ASSERT_EQUAL("", p1.relative_path().u8string());

#ifdef _WIN32
	Path p2("C:\\");
	ASSERT_FALSE(p2.has_relative_path())
	ASSERT_EQUAL("", p2.relative_path().u8string());
#endif

	Path p3("\\\\host");
	ASSERT_FALSE(p3.has_relative_path())
	ASSERT_EQUAL("", p3.relative_path().u8string());

	Path p4("\\\\host\\");
	ASSERT_FALSE(p4.has_relative_path())
	ASSERT_EQUAL("", p4.relative_path().u8string());
}

void
test_fetch_relative_path_when_path_ends_with_slash()
{
	Path p1("/foo/");
	ASSERT(p1.has_relative_path())
	ASSERT_EQUAL("foo/", p1.relative_path().u8string());

	Path p2("foo/");
	ASSERT(p2.has_relative_path())
	ASSERT_EQUAL("foo/", p2.relative_path().u8string());
}

void
test_fetch_parent_path_regular_case()
{
	Path p1("/var/tmp/example.txt");
	ASSERT(p1.has_parent_path())
	ASSERT_EQUAL("/var/tmp", p1.parent_path().u8string())

	Path p2("var/tmp/example.txt");
	ASSERT(p2.has_parent_path())
	ASSERT_EQUAL("var/tmp", p2.parent_path().u8string())

	Path p3("var/tmp");
	ASSERT(p3.has_parent_path())
	ASSERT_EQUAL("var", p3.parent_path().u8string())
}

void
test_fetch_parent_path_regular_case_on_windows()
{
	// bug originally reported in PR #2762
#ifdef _WIN32
	Path p1("C:\\foo.bar");
	ASSERT(p1.has_parent_path())
	ASSERT_EQUAL("C:\\", p1.parent_path().u8string())
#endif
}

void
test_fetch_parent_path_with_network_samba_folder_on_windows()
{

}

void
test_does_not_fetch_parent_path_when_path_is_empty()
{
	Path p1;
	ASSERT_FALSE(p1.has_parent_path())
	ASSERT_EQUAL("", p1.parent_path().u8string())
}

void
test_parent_path_returns_copy_when_path_is_root()
{
	Path p1("/");
	ASSERT(p1.has_parent_path())
	ASSERT_EQUAL("/", p1.parent_path().u8string())

	Path p0("//");
	ASSERT(p0.has_parent_path())
	ASSERT_EQUAL("//", p0.parent_path().u8string())

#ifdef _WIN32
	Path p2("C:\\");
	ASSERT(p2.has_parent_path())
	ASSERT_EQUAL("C:\\", p2.parent_path().u8string())
#endif

	Path p3("\\\\host\\");
	ASSERT(p3.has_parent_path())
	ASSERT_EQUAL("\\\\host\\", p3.parent_path().u8string())

	Path p4("\\\\host");
	ASSERT(p4.has_parent_path())
	ASSERT_EQUAL("\\\\host", p4.parent_path().u8string())
}

void
test_fetch_parent_path_when_path_ends_with_slash()
{
	Path p1("/var/tmp/example/");
	ASSERT(p1.has_parent_path())
	ASSERT_EQUAL("/var/tmp/example", p1.parent_path().u8string())
}

void
test_fetch_parent_path_when_it_ends_with_dot_filename()
{
	Path p1("/var/tmp/.");
	ASSERT(p1.has_parent_path())
	ASSERT_EQUAL("/var/tmp", p1.parent_path().u8string())
}

void
test_fetch_parent_path_when_it_ends_with_dot_dot_filename()
{
	Path p1("/var/tmp/..");
	ASSERT(p1.has_parent_path())
	ASSERT_EQUAL("/var/tmp", p1.parent_path().u8string())
}

void
test_fetch_parent_path_skips_consecutive_separators()
{
	Path p1("/var/tmp//example.txt");
	ASSERT(p1.has_parent_path())
	ASSERT_EQUAL("/var/tmp", p1.parent_path().u8string())

	Path p2("/var/tmp///example.txt");
	ASSERT(p2.has_parent_path())
	ASSERT_EQUAL("/var/tmp", p2.parent_path().u8string())

	Path p3("/var/tmp//example//");
	ASSERT(p3.has_parent_path())
	ASSERT_EQUAL("/var/tmp//example", p3.parent_path().u8string())
}

void
test_fetch_parent_path_when_relative_path_has_only_one_component_and_it_is_not_an_absolute_path()
{
	Path p1("var");
	ASSERT_FALSE(p1.has_parent_path())
	ASSERT_EQUAL("", p1.parent_path().u8string())

	Path p2(".");
	ASSERT_FALSE(p2.has_parent_path())
	ASSERT_EQUAL("", p2.parent_path().u8string())

	Path p3("..");
	ASSERT_FALSE(p3.has_parent_path())
	ASSERT_EQUAL("", p3.parent_path().u8string())
}

void
test_fetch_parent_path_when_relative_path_has_only_one_component_and_it_is_an_absolute_path()
{
	Path p1("/var");
	ASSERT(p1.has_parent_path())
	ASSERT_EQUAL("/", p1.parent_path().u8string())

	Path p2("/.");
	ASSERT(p2.has_parent_path())
	ASSERT_EQUAL("/", p2.parent_path().u8string())

	Path p3("/var.txt");
	ASSERT(p3.has_parent_path())
	ASSERT_EQUAL("/", p3.parent_path().u8string())
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
test_does_not_fetch_path_filename_when_path_ends_with_slash()
{
	Path p1("/foo/bar/");
	ASSERT_FALSE(p1.has_filename())
	ASSERT_EQUAL("", p1.filename().u8string());
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
test_does_not_fetch_path_stem_when_path_ends_with_slash()
{
	Path p1("/foo/bar/");
	ASSERT_FALSE(p1.has_stem())
	ASSERT_EQUAL("", p1.stem().u8string());
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
test_does_not_fetch_path_extension_when_path_ends_with_slash()
{
	Path p("/foo/bar/");
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

void
test_is_absolute()
{
#ifndef _WIN32
	Path p1("/");
	ASSERT(p1.is_absolute())
	ASSERT_FALSE(p1.is_relative())

	Path p2("/a");
	ASSERT(p2.is_absolute())
	ASSERT_FALSE(p2.is_relative())

	Path p3("/.");
	ASSERT(p3.is_absolute())
	ASSERT_FALSE(p3.is_relative())

	Path p4("/..");
	ASSERT(p4.is_absolute())
	ASSERT_FALSE(p4.is_relative())

	Path p5("/foo/");
	ASSERT(p5.is_absolute())
	ASSERT_FALSE(p5.is_relative())

	Path p6("/foo/bar");
	ASSERT(p6.is_absolute())
	ASSERT_FALSE(p6.is_relative())
#endif

	Path p7(".");
	ASSERT_FALSE(p7.is_absolute())
	ASSERT(p7.is_relative())

	Path p8("a");
	ASSERT_FALSE(p8.is_absolute())
	ASSERT(p8.is_relative())

	Path p9("./");
	ASSERT_FALSE(p9.is_absolute())
	ASSERT(p9.is_relative())

	Path p10("..");
	ASSERT_FALSE(p10.is_absolute())
	ASSERT(p10.is_relative())

	Path p11("foo/");
	ASSERT_FALSE(p11.is_absolute())
	ASSERT(p11.is_relative())

	Path p12("foo/bar");
	ASSERT_FALSE(p12.is_absolute())
	ASSERT(p12.is_relative())
}

void
test_is_not_absolute_on_windows_if_path_starts_with_root_directory()
{
#ifdef _WIN32
	Path p1("/");
	ASSERT_FALSE(p1.is_absolute())
	ASSERT(p1.is_relative())

	Path p2("/a");
	ASSERT_FALSE(p2.is_absolute())
	ASSERT(p2.is_relative())

	Path p3("/.");
	ASSERT_FALSE(p3.is_absolute())
	ASSERT(p3.is_relative())

	Path p4("/..");
	ASSERT_FALSE(p4.is_absolute())
	ASSERT(p4.is_relative())

	Path p5("/foo/");
	ASSERT_FALSE(p5.is_absolute())
	ASSERT(p5.is_relative())

	Path p6("/foo/bar");
	ASSERT_FALSE(p6.is_absolute())
	ASSERT(p6.is_relative())
#endif
}

void
test_is_absolute_on_windows_with_drive()
{
#ifdef _WIN32
	Path p1("C:\\");
	ASSERT(p1.is_absolute())
	ASSERT_FALSE(p1.is_relative())

	Path p2("C:\\a");
	ASSERT(p2.is_absolute())
	ASSERT_FALSE(p2.is_relative())

	Path p3("C:\\.");
	ASSERT(p3.is_absolute())
	ASSERT_FALSE(p3.is_relative())

	Path p4("C:\\..");
	ASSERT(p4.is_absolute())
	ASSERT_FALSE(p4.is_relative())

	Path p5("C:\\foo/");
	ASSERT(p5.is_absolute())
	ASSERT_FALSE(p5.is_relative())

	Path p6("C:\\foo/bar");
	ASSERT(p6.is_absolute())
	ASSERT_FALSE(p6.is_relative())
#endif
}

void
test_is_absolute_on_windows_with_network_samba_folder()
{
	Path p1("\\\\host\\");
	ASSERT(p1.is_absolute())
	ASSERT_FALSE(p1.is_relative())

	Path p2("\\\\host\\a");
	ASSERT(p2.is_absolute())
	ASSERT_FALSE(p2.is_relative())

	Path p3("\\\\host\\.");
	ASSERT(p3.is_absolute())
	ASSERT_FALSE(p3.is_relative())

	Path p4("\\\\host\\..");
	ASSERT(p4.is_absolute())
	ASSERT_FALSE(p4.is_relative())

	Path p5("\\\\host\\foo/");
	ASSERT(p5.is_absolute())
	ASSERT_FALSE(p5.is_relative())

	Path p6("\\\\host\\foo/bar");
	ASSERT(p6.is_absolute())
	ASSERT_FALSE(p6.is_relative())
}

void
test_normalize_empty_path_keeps_empty()
{
	ASSERT(Path().lexically_normal().empty())
}

void
test_normalize_path_converts_backlash_to_slash()
{
	ASSERT_EQUAL("/a/b/c", Path("\\a\\b\\c").lexically_normal().u8string())
	ASSERT_EQUAL("/a/b/c/", Path("\\a\\b\\c\\").lexically_normal().u8string())
	ASSERT_EQUAL("a/b/c", Path("a\\b\\c").lexically_normal().u8string())
	ASSERT_EQUAL("a/b/c/", Path("a\\b\\c\\").lexically_normal().u8string())
	ASSERT_EQUAL("/", Path("\\").lexically_normal().u8string())
	ASSERT_EQUAL("C:/", Path("C:\\").lexically_normal().u8string())
}

void
test_normalize_does_not_change_double_slash_of_windows_samba_network_shared_host()
{
	ASSERT_EQUAL("\\\\host", Path("\\\\host").lexically_normal().u8string())
	ASSERT_EQUAL("\\\\host/", Path("\\\\host\\").lexically_normal().u8string())
}

void
test_normalize_dot_path_is_dot_without_trailing_slash()
{
	ASSERT_EQUAL(".", Path("./").lexically_normal().u8string())
}

void
test_normalize_path_with_trailing_dot_dot_slash_removes_slash()
{
	ASSERT_EQUAL("..", Path("../").lexically_normal().u8string())
}

void
test_normalize_removes_double_slashes()
{
	ASSERT_EQUAL("a/b", Path("a//b").lexically_normal().u8string())
	ASSERT_EQUAL("/a/b", Path("//a//b").lexically_normal().u8string())
	ASSERT_EQUAL("a/b/", Path("a//b//").lexically_normal().u8string())
	ASSERT_EQUAL("a/b", Path("a//b").lexically_normal().u8string())
}

void
test_normalize_removes_intermediate_special_dot_folder()
{
	ASSERT_EQUAL("a/b", Path("a/./b").lexically_normal().u8string())
	ASSERT_EQUAL("a", Path("./././a").lexically_normal().u8string())
	ASSERT_EQUAL("b", Path("./b").lexically_normal().u8string())
	ASSERT_EQUAL("/a/b", Path("/./a/./b").lexically_normal().u8string())
	ASSERT_EQUAL("/a", Path("/./././a").lexically_normal().u8string())
	ASSERT_EQUAL("/b", Path("/./b").lexically_normal().u8string())
	ASSERT_EQUAL("a/", Path("a/.").lexically_normal().u8string())
	ASSERT_EQUAL("a/", Path("a/././.").lexically_normal().u8string())
	ASSERT_EQUAL("a/", Path("a/./").lexically_normal().u8string())
}

void
test_normalize_removes_intermediate_special_dot_dot_folder()
{
	ASSERT_EQUAL("/b", Path("/a/../b").lexically_normal().u8string())
	ASSERT_EQUAL("/a/", Path("/a/b/..").lexically_normal().u8string())
	ASSERT_EQUAL("/a/", Path("/a/b/../").lexically_normal().u8string())
	ASSERT_EQUAL("/b", Path("/a/../../b").lexically_normal().u8string())
	ASSERT_EQUAL("/c", Path("/a/b/../../c").lexically_normal().u8string())
	ASSERT_EQUAL("/c", Path("/a/../b/../c").lexically_normal().u8string())

	ASSERT_EQUAL("b", Path("a/../b").lexically_normal().u8string())
	ASSERT_EQUAL("a/", Path("a/b/..").lexically_normal().u8string())
	ASSERT_EQUAL("a/", Path("a/b/../").lexically_normal().u8string())
	ASSERT_EQUAL("../b", Path("a/../../b").lexically_normal().u8string())
	ASSERT_EQUAL("c", Path("a/b/../../c").lexically_normal().u8string())
	ASSERT_EQUAL("../../a", Path("../../a").lexically_normal().u8string())
}

void
test_normalize_does_not_remove_component_with_prepended_dot()
{
	ASSERT_EQUAL(".foo", Path(".foo").lexically_normal().u8string())
	ASSERT_EQUAL("/.foo", Path("/.foo").lexically_normal().u8string())
	ASSERT_EQUAL(".a", Path(".a").lexically_normal().u8string())
	ASSERT_EQUAL("foo/.bar", Path("foo/.bar").lexically_normal().u8string())
	ASSERT_EQUAL(".bar", Path(".foo/../.bar").lexically_normal().u8string())
	ASSERT_EQUAL(".foo/.bar/", Path(".foo/./.bar/.").lexically_normal().u8string())
	ASSERT_EQUAL("foo/.b", Path("foo/.b").lexically_normal().u8string())
	ASSERT_EQUAL("foo/.bar/end", Path("foo/.bar/end").lexically_normal().u8string())
}

void
test_normalize_does_not_remove_component_with_prepended_dot_dot()
{
	ASSERT_EQUAL("..foo", Path("..foo").lexically_normal().u8string())
	ASSERT_EQUAL("/..foo", Path("/..foo").lexically_normal().u8string())
	ASSERT_EQUAL("..a", Path("..a").lexically_normal().u8string())
	ASSERT_EQUAL("foo/..bar", Path("foo/..bar").lexically_normal().u8string())
	ASSERT_EQUAL("..bar", Path(".foo/../..bar").lexically_normal().u8string())
	ASSERT_EQUAL(".foo/..bar/", Path(".foo/./..bar/.").lexically_normal().u8string())
	ASSERT_EQUAL("foo/..b", Path("foo/..b").lexically_normal().u8string())
	ASSERT_EQUAL("foo/..bar/end", Path("foo/..bar/end").lexically_normal().u8string())
}

void
test_normalize_remove_special_dot_dot_right_after_root()
{
	ASSERT_EQUAL("/", Path("/..").lexically_normal().u8string())
	ASSERT_EQUAL("/a", Path("/../../a").lexically_normal().u8string())
	ASSERT_EQUAL("/c", Path("/../b/../c").lexically_normal().u8string())
}

void
test_normalize_examples_from_cpp_reference_dot_com()
{
	// https://en.cppreference.com/w/cpp/filesystem/path/lexically_normal
	ASSERT_EQUAL("a/", Path("a/./b/..").lexically_normal().u8string());
	ASSERT_EQUAL("a/", Path("a/.///b/../").lexically_normal().u8string());
}

void
test_relative()
{
	ASSERT_EQUAL("../../d", Path("/a/d").lexically_relative(Path("/a/b/c")).u8string());
	ASSERT_EQUAL("../b/c", Path("/a/b/c").lexically_relative(Path("/a/d")).u8string());
	ASSERT_EQUAL("b/c", Path("a/b/c").lexically_relative(Path("a")).u8string());
	ASSERT_EQUAL("../..", Path("a/b/c").lexically_relative(Path("a/b/c/x/y")).u8string());
	ASSERT_EQUAL(".", Path("a/b/c").lexically_relative(Path("a/b/c")).u8string());
	ASSERT_EQUAL("../../a/b", Path("a/b").lexically_relative(Path("c/d")).u8string());
	ASSERT_EQUAL("", Path("a/b").lexically_relative(Path("/a/b")).u8string());
}

void
test_relative_to_empty_path_returns_itself()
{
	ASSERT_EQUAL("a/e.fg", Path("a/e.fg").lexically_relative(Path()).u8string());
	ASSERT_EQUAL("e.fg", Path("e.fg").lexically_relative(Path()).u8string());
	// exception: absolute path
#ifdef _WIN32
	ASSERT_EQUAL("", Path("C:/a/d").lexically_relative(Path()).u8string());
#else
	ASSERT_EQUAL("", Path("/a/d").lexically_relative(Path()).u8string());
#endif
}

void
test_empty_path_relative_to_another_returns_as_it_was_special_dot_file()
{
	ASSERT_EQUAL("../..", Path().lexically_relative(Path("a/e.fg")).u8string());
	ASSERT_EQUAL("..", Path().lexically_relative(Path("e.fg")).u8string());
	// exception: absolute path
#ifdef _WIN32
	ASSERT_EQUAL("", Path().lexically_relative(Path("C:/a/d")).u8string());
#else
	ASSERT_EQUAL("", Path().lexically_relative(Path("/a/d")).u8string());
#endif
}

void
test_relative_ported_from_old_etl_stringf()
{
	ASSERT_EQUAL("myfile.txt", Path("/home/darco/projects/voria/myfile.txt").lexically_relative(Path("/home/darco/projects/voria")).u8string())

	ASSERT_EQUAL("files/myfile.txt", Path("/home/darco/projects/voria/files/myfile.txt").lexically_relative(Path("/home/darco/projects/voria")).u8string())

	ASSERT_EQUAL("../../share", Path("/usr/share").lexically_relative(Path("/usr/local/bin/.")).u8string())
}

void
test_fake_relative_from_cpp_reference_dot_com()
{
	// https://en.cppreference.com/w/cpp/filesystem/relative
#ifdef _WIN32
	ASSERT_EQUAL("c", Path("C:/a/b/c").relative_to(Path("C:/a/b")).u8string());
	ASSERT_EQUAL("../c", Path("C:/a/c").relative_to(Path("C:/a/b")).u8string());
	ASSERT_EQUAL("", Path("c").relative_to(Path("C:/a/b")).u8string());
	ASSERT_EQUAL("", Path("C:/a/b").relative_to(Path("c")).u8string());
#else
	ASSERT_EQUAL("c", Path("/a/b/c").relative_to(Path("/a/b")).u8string());
	ASSERT_EQUAL("../c", Path("/a/c").relative_to(Path("/a/b")).u8string());
	ASSERT_EQUAL("", Path("c").relative_to(Path("/a/b")).u8string());
	ASSERT_EQUAL("", Path("/a/b").relative_to(Path("c")).u8string());
#endif
}

void
test_relative_between_different_root_paths()
{
#ifdef _WIN32
	ASSERT_EQUAL("", Path("D:/a/b/c").relative_to(Path("C:/a/b")).u8string());
	ASSERT_EQUAL("", Path("D:/a/c").relative_to(Path("C:/a/b")).u8string());
#endif
}

void
test_lexically_proximate_from_cpp_reference_dot_com()
{
	// https://en.cppreference.com/w/cpp/filesystem/path/lexically_normal
	ASSERT_EQUAL("a/b", Path("a/b").lexically_proximate(Path("/a/b")).u8string());
}

void
test_fake_proximate_from_cpp_reference_dot_com()
{
	// https://en.cppreference.com/w/cpp/filesystem/relative
#ifdef _WIN32
	ASSERT_EQUAL("c", Path("C:/a/b/c").proximate_to(Path("C:/a/b")).u8string());
	ASSERT_EQUAL("../c", Path("C:/a/c").proximate_to(Path("C:/a/b")).u8string());
	ASSERT_EQUAL("c", Path("c").proximate_to(Path("C:/a/b")).u8string());
	ASSERT_EQUAL("C:/a/b", Path("C:/a/b").proximate_to(Path("c")).u8string());
#else
	ASSERT_EQUAL("c", Path("/a/b/c").proximate_to(Path("/a/b")).u8string());
	ASSERT_EQUAL("../c", Path("/a/c").proximate_to(Path("/a/b")).u8string());
	ASSERT_EQUAL("c", Path("c").proximate_to(Path("/a/b")).u8string());
	ASSERT_EQUAL("/a/b", Path("/a/b").proximate_to(Path("c")).u8string());
#endif
}

void
test_proximate_between_different_root_paths()
{
#ifdef _WIN32
	ASSERT_EQUAL("D:/a/b/c", Path("D:/a/b/c").proximate_to(Path("C:/a/b")).u8string());
	ASSERT_EQUAL("D:/a/c", Path("D:/a/c").proximate_to(Path("C:/a/b")).u8string());
#endif
}

void
test_from_uri()
{
	ASSERT_EQUAL("/path/to/file", Path::from_uri("file:///path/to/file").u8string());
	ASSERT_EQUAL("/path/to/file", Path::from_uri("file:/path/to/file").u8string());
	ASSERT_EQUAL("/path/to/file", Path::from_uri("file://localhost/path/to/file").u8string());

	ASSERT_EQUAL("c:/path/to/file", Path::from_uri("file:c:/path/to/file").u8string());
	ASSERT_EQUAL("c:/path/to/file", Path::from_uri("file:///c:/path/to/file").u8string());
	ASSERT_EQUAL("c:/path/to/file", Path::from_uri("file:/c:/path/to/file").u8string());
	ASSERT_EQUAL("c:/path/to/file", Path::from_uri("file://localhost/c:/path/to/file").u8string());

	ASSERT_EQUAL("c:/path/to/file", Path::from_uri("file:c|/path/to/file").u8string());
	ASSERT_EQUAL("c:/path/to/file", Path::from_uri("file:///c|/path/to/file").u8string());
	ASSERT_EQUAL("c:/path/to/file", Path::from_uri("file:/c|/path/to/file").u8string());
	ASSERT_EQUAL("c:/path/to/file", Path::from_uri("file://localhost/c|/path/to/file").u8string());

	ASSERT_EQUAL("\\\\host.example.com/path/to/file", Path::from_uri("file://host.example.com/path/to/file").u8string());
	ASSERT_EQUAL("\\\\host.example.com/path/to/file", Path::from_uri("file:////host.example.com/path/to/file").u8string());
	ASSERT_EQUAL("\\\\host.example.com/path/to/file", Path::from_uri("file://///host.example.com/path/to/file").u8string());
}

/* === E N T R Y P O I N T ================================================= */

int main() {

	TEST_SUITE_BEGIN()

	TEST_FUNCTION(test_default_path_constructor_is_empty)
	TEST_FUNCTION(test_path_with_empty_string_is_empty)
	TEST_FUNCTION(test_path_with_non_empty_string_is_not_empty)

	TEST_FUNCTION(test_append_path_simple_case)
	TEST_FUNCTION(test_append_path_does_not_duplicate_slash)
	TEST_FUNCTION(test_append_path_with_root_path_replaces)
	TEST_FUNCTION(test_append_path_cppreference_examples)

	TEST_FUNCTION(test_concat_path_cppreference_examples)

	TEST_FUNCTION(test_remove_filename)

	TEST_FUNCTION(test_replace_filename)

	TEST_FUNCTION(test_replace_extension)

	TEST_FUNCTION(test_add_suffix)
	TEST_FUNCTION(test_try_to_add_suffix_to_dot_file_does_not_work)
	TEST_FUNCTION(test_try_to_add_suffix_to_dotdot_file_does_not_work)

	TEST_FUNCTION(test_compare_path_both_empty)
	TEST_FUNCTION(test_compare_path_different_root_name)
	TEST_FUNCTION(test_compare_path_different_root_directory)
	TEST_FUNCTION(test_compare_path_with_different_relative_path_size)
	TEST_FUNCTION(test_compare_path_both_has_same_relative_path_size)

	TEST_FUNCTION(test_fetch_path_root_name_regular_drive_on_windows)
	TEST_FUNCTION(test_fetch_path_root_name_network_samba_folder)
	TEST_FUNCTION(test_does_not_fetch_root_name_when_path_is_empty)
	TEST_FUNCTION(test_does_not_fetch_root_name_when_not_absolute)

	TEST_FUNCTION(test_fetch_path_root_directory_regular_drive_on_windows)
	TEST_FUNCTION(test_fetch_path_root_directory_network_samba_folder)
	TEST_FUNCTION(test_fetch_path_root_directory_prepended_slash)
	TEST_FUNCTION(test_does_not_fetch_root_directory_when_path_is_empty)
	TEST_FUNCTION(test_does_not_fetch_root_directory_when_not_absolute)

	TEST_FUNCTION(test_fetch_relative_path_regular_case)
	TEST_FUNCTION(test_fetch_relative_path_root_with_multiple_separators)
	TEST_FUNCTION(test_fetch_relative_path_regular_case_on_windows)
	TEST_FUNCTION(test_fetch_relative_path_with_network_samba_folder)
	TEST_FUNCTION(test_does_not_fetch_relative_path_when_path_is_empty)
	TEST_FUNCTION(test_does_not_fetch_relative_path_when_path_is_root)
	TEST_FUNCTION(test_fetch_relative_path_when_path_ends_with_slash)

	TEST_FUNCTION(test_fetch_parent_path_regular_case)
	TEST_FUNCTION(test_fetch_parent_path_regular_case_on_windows)
	TEST_FUNCTION(test_fetch_parent_path_with_network_samba_folder_on_windows)
	TEST_FUNCTION(test_does_not_fetch_parent_path_when_path_is_empty)
	TEST_FUNCTION(test_parent_path_returns_copy_when_path_is_root)
	TEST_FUNCTION(test_fetch_parent_path_when_path_ends_with_slash)
	TEST_FUNCTION(test_fetch_parent_path_when_it_ends_with_dot_filename)
	TEST_FUNCTION(test_fetch_parent_path_when_it_ends_with_dot_dot_filename)
	TEST_FUNCTION(test_fetch_parent_path_skips_consecutive_separators)
	TEST_FUNCTION(test_fetch_parent_path_when_relative_path_has_only_one_component_and_it_is_not_an_absolute_path)
	TEST_FUNCTION(test_fetch_parent_path_when_relative_path_has_only_one_component_and_it_is_an_absolute_path)

	TEST_FUNCTION(test_fetch_path_filename_regular_case)
	TEST_FUNCTION(test_does_not_fetch_path_filename_when_path_is_empty)
	TEST_FUNCTION(test_does_not_fetch_path_filename_when_path_is_root)
	TEST_FUNCTION(test_fetch_path_filename_when_path_does_not_have_any_extension)
	TEST_FUNCTION(test_does_not_fetch_path_filename_when_path_ends_with_slash)
	TEST_FUNCTION(test_fetch_path_filename_when_path_filename_starts_with_dot_char)
	TEST_FUNCTION(test_fetch_path_filename_when_path_filename_ends_with_dot_char)
	TEST_FUNCTION(test_fetch_path_filename_when_is_dot_filename)
	TEST_FUNCTION(test_fetch_path_filename_when_is_dot_dot_filename)

	TEST_FUNCTION(test_fetch_path_stem_regular_case)
	TEST_FUNCTION(test_does_not_fetch_path_stem_when_path_is_empty)
	TEST_FUNCTION(test_does_not_fetch_path_stem_when_path_is_root)
	TEST_FUNCTION(test_fetch_path_stem_when_path_does_not_have_any_extension)
	TEST_FUNCTION(test_does_not_fetch_path_stem_when_path_ends_with_slash)
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
	TEST_FUNCTION(test_does_not_fetch_path_extension_when_path_ends_with_slash)
	TEST_FUNCTION(test_does_not_fetch_path_extension_when_path_filename_starts_with_dot_char)
	TEST_FUNCTION(test_fetch_path_extension_when_path_filename_ends_with_dot_char)
	TEST_FUNCTION(test_does_not_fetch_path_extension_when_is_dot_filename)
	TEST_FUNCTION(test_does_not_fetch_path_extension_when_is_dot_dot_filename)
	TEST_FUNCTION(test_fetch_path_extension_correctly_when_intermediate_path_has_dot_char)
	TEST_FUNCTION(test_fetch_path_extension_correctly_when_weird_case_dotchar_dotchar_extension)

	TEST_FUNCTION(test_is_absolute)
	TEST_FUNCTION(test_is_not_absolute_on_windows_if_path_starts_with_root_directory)
	TEST_FUNCTION(test_is_absolute_on_windows_with_drive)
	TEST_FUNCTION(test_is_absolute_on_windows_with_network_samba_folder)

	TEST_FUNCTION(test_normalize_empty_path_keeps_empty)
	TEST_FUNCTION(test_normalize_path_converts_backlash_to_slash)
	TEST_FUNCTION(test_normalize_does_not_change_double_slash_of_windows_samba_network_shared_host)
	TEST_FUNCTION(test_normalize_dot_path_is_dot_without_trailing_slash)
	TEST_FUNCTION(test_normalize_path_with_trailing_dot_dot_slash_removes_slash)
	TEST_FUNCTION(test_normalize_removes_double_slashes)
	TEST_FUNCTION(test_normalize_removes_intermediate_special_dot_folder)
	TEST_FUNCTION(test_normalize_removes_intermediate_special_dot_dot_folder)
	TEST_FUNCTION(test_normalize_does_not_remove_component_with_prepended_dot)
	TEST_FUNCTION(test_normalize_does_not_remove_component_with_prepended_dot_dot)
	TEST_FUNCTION(test_normalize_remove_special_dot_dot_right_after_root)
	TEST_FUNCTION(test_normalize_examples_from_cpp_reference_dot_com)

	TEST_FUNCTION(test_relative)
	TEST_FUNCTION(test_relative_to_empty_path_returns_itself)
	TEST_FUNCTION(test_empty_path_relative_to_another_returns_as_it_was_special_dot_file)
	TEST_FUNCTION(test_relative_ported_from_old_etl_stringf)
	TEST_FUNCTION(test_fake_relative_from_cpp_reference_dot_com)
	TEST_FUNCTION(test_relative_between_different_root_paths)

	TEST_FUNCTION(test_lexically_proximate_from_cpp_reference_dot_com)
	TEST_FUNCTION(test_fake_proximate_from_cpp_reference_dot_com)
	TEST_FUNCTION(test_proximate_between_different_root_paths)

	TEST_FUNCTION(test_from_uri)

	TEST_SUITE_END()

	return tst_exit_status;
}
