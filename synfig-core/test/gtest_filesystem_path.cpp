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

#include <gtest/gtest.h>

using namespace synfig::filesystem;

/* === P R O C E D U R E S ================================================= */

TEST(FilesystemPathTest, test_default_path_constructor_is_empty)
{
	Path p;
	EXPECT_TRUE(p.empty())
}

TEST(FilesystemPathTest, test_path_with_empty_string_is_empty)
{
	Path p("");
	EXPECT_TRUE(p.empty())
}

TEST(FilesystemPathTest, test_path_with_non_empty_string_is_not_empty)
{
	Path p("file");
	EXPECT_FALSE(p.empty())
}

TEST(FilesystemPathTest, test_append_path_simple_case)
{
	EXPECT_EQ("a/b", (Path("a") / Path("b")).u8string())
	EXPECT_EQ("/foo/bar/", (Path("/foo") / Path("bar/")).u8string())
}

TEST(FilesystemPathTest, test_append_path_does_not_duplicate_slash)
{
	EXPECT_EQ("a/b", (Path("a/") / Path("b")).u8string())
	EXPECT_EQ("/bar/", (Path("/") / Path("bar/")).u8string())
}

TEST(FilesystemPathTest, test_append_path_with_root_path_replaces)
{
	EXPECT_EQ("/b", (Path("a") / Path("/b")).u8string())
	EXPECT_EQ("/bar/", (Path("/foo") / Path("/bar/")).u8string())
}

TEST(FilesystemPathTest, test_append_path_cppreference_examples)
{
	// https://en.cppreference.com/w/cpp/filesystem/path/append
	EXPECT_EQ("\\\\host/foo", (Path("\\\\host")  / Path("foo")).u8string())
	EXPECT_EQ("\\\\host/foo", (Path("\\\\host/") / Path("foo")).u8string())

	// On POSIX,
	EXPECT_EQ("foo/", (Path("foo") / Path("")).u8string())
	EXPECT_EQ("/bar", (Path("foo") / Path("/bar")).u8string())

#ifdef _WIN32
	// On Windows,
	EXPECT_EQ("C:/bar", (Path("foo") / Path("C:/bar")).u8string())
	EXPECT_EQ("C:", (Path("foo") / Path("C:")).u8string())
	EXPECT_EQ("C:", (Path("C:") / Path("")).u8string())
	EXPECT_EQ("C:/bar", (Path("C:foo") / Path("/bar")).u8string())
	EXPECT_EQ("C:foo/bar", (Path("C:foo") / Path("C:bar")).u8string())
#endif
}

TEST(FilesystemPathTest, test_concat_path_cppreference_examples)
{
	Path p1; // an empty path
	p1 += std::string("var"); // does not insert a separator
	EXPECT_EQ("var", p1.u8string())
	EXPECT_EQ("varlib", (p1 + std::string("lib")).u8string())
	p1 += std::string("lib");
	p1.concat("123456");
	EXPECT_EQ("varlib123456", p1.u8string())
}

TEST(FilesystemPathTest, test_remove_filename)
{
	EXPECT_EQ("foo/", Path("foo/bar").remove_filename().u8string())
	EXPECT_EQ("foo/", Path("foo/").remove_filename().u8string())
	EXPECT_EQ("/", Path("/foo").remove_filename().u8string())
	EXPECT_EQ("/", Path("/").remove_filename().u8string())
}

TEST(FilesystemPathTest, test_replace_filename)
{
	EXPECT_EQ("/bar", Path("/foo").replace_filename(Path("bar")).u8string())
	EXPECT_EQ("/bar", Path("/").replace_filename(Path("bar")).u8string())
	EXPECT_EQ("bar", Path("foo").replace_filename(Path("bar")).u8string())
	EXPECT_EQ("pub", Path("").replace_filename(Path("pub")).u8string())
}

TEST(FilesystemPathTest, test_replace_extension)
{
	// examples from https://en.cppreference.com/w/cpp/filesystem/path/replace_extension
	EXPECT_EQ("/foo/bar.png", Path("/foo/bar.jpg").replace_extension(Path(".png")).u8string())
	EXPECT_EQ("/foo/bar.png", Path("/foo/bar.jpg").replace_extension(Path("png")).u8string())
	EXPECT_EQ("/foo/bar.",    Path("/foo/bar.jpg").replace_extension(Path(".")).u8string())
	EXPECT_EQ("/foo/bar",     Path("/foo/bar.jpg").replace_extension(Path("")).u8string())
	EXPECT_EQ("/foo/bar.png", Path("/foo/bar.").replace_extension(Path("png")).u8string())
	EXPECT_EQ("/foo/bar.png", Path("/foo/bar").replace_extension(Path(".png")).u8string())
	EXPECT_EQ("/foo/bar.png", Path("/foo/bar").replace_extension(Path("png")).u8string())
	EXPECT_EQ("/foo/bar.",    Path("/foo/bar").replace_extension(Path(".")).u8string())
	EXPECT_EQ("/foo/bar",     Path("/foo/bar").replace_extension(Path("")).u8string())
	EXPECT_EQ("/foo/..png",   Path("/foo/.").replace_extension(Path(".png")).u8string())
	EXPECT_EQ("/foo/..png",   Path("/foo/.").replace_extension(Path("png")).u8string())
	EXPECT_EQ("/foo/..",      Path("/foo/.").replace_extension(Path(".")).u8string())
	EXPECT_EQ("/foo/.",       Path("/foo/.").replace_extension(Path("")).u8string())
	EXPECT_EQ("/foo/.png",    Path("/foo/").replace_extension(Path(".png")).u8string())
	EXPECT_EQ("/foo/.png",    Path("/foo/").replace_extension(Path("png")).u8string())
}

TEST(FilesystemPathTest, test_add_suffix)
{
	EXPECT_EQ("/foo/bar-000.jpg", Path("/foo/bar.jpg").add_suffix("-000").u8string())
	EXPECT_EQ("foo/bar-000.jpg", Path("foo/bar.jpg").add_suffix("-000").u8string())
	EXPECT_EQ("/bar-000.jpg", Path("/bar.jpg").add_suffix("-000").u8string())
	EXPECT_EQ("bar-000.jpg", Path("bar.jpg").add_suffix("-000").u8string())

	EXPECT_EQ(".hidden-000", Path(".hidden").add_suffix("-000").u8string())
	EXPECT_EQ(".hidden-000.txt", Path(".hidden.txt").add_suffix("-000").u8string())

	EXPECT_EQ("/foo/bar-000.", Path("/foo/bar.").add_suffix("-000").u8string())
	EXPECT_EQ("foo/bar-000.", Path("foo/bar.").add_suffix("-000").u8string())
	EXPECT_EQ("/bar-000.", Path("/bar.").add_suffix("-000").u8string())
	EXPECT_EQ("bar-000.", Path("bar.").add_suffix("-000").u8string())

	EXPECT_EQ("/foo/bar-000", Path("/foo/bar").add_suffix("-000").u8string())
	EXPECT_EQ("foo/bar-000", Path("foo/bar").add_suffix("-000").u8string())
	EXPECT_EQ("/bar-000", Path("/bar").add_suffix("-000").u8string())
	EXPECT_EQ("bar-000", Path("bar").add_suffix("-000").u8string())
	EXPECT_EQ("-000", Path().add_suffix("-000").u8string())

	EXPECT_EQ("bar-000-001.jpg", Path("bar.jpg").add_suffix("-000").add_suffix("-001").u8string())
}

TEST(FilesystemPathTest, test_try_to_add_suffix_to_dot_file_does_not_work)
{
	EXPECT_EQ(".", Path(".").add_suffix("-000").u8string())
	EXPECT_EQ("/.", Path("/.").add_suffix("-000").u8string())
	EXPECT_EQ("a/.", Path("a/.").add_suffix("-000").u8string())
}

TEST(FilesystemPathTest, test_try_to_add_suffix_to_dotdot_file_does_not_work)
{
	EXPECT_EQ("..", Path("..").add_suffix("-000").u8string())
	EXPECT_EQ("/..", Path("/..").add_suffix("-000").u8string())
	EXPECT_EQ("a/..", Path("a/..").add_suffix("-000").u8string())
}

TEST(FilesystemPathTest, test_compare_path_both_empty)
{
	Path p1, p2;
	EXPECT_EQ(0, p1.compare(p2))
}

TEST(FilesystemPathTest, test_compare_path_different_root_name)
{
#ifdef _WIN32
	Path p1("C:");
	Path p2("C:");
	EXPECT_EQ(0, p1.compare(p2))

	Path p3("D:");
	EXPECT_TRUE(0 > p1.compare(p3))
	EXPECT_TRUE(0 < p3.compare(p1))

	Path p4("A:");
	EXPECT_TRUE(0 < p1.compare(p4))
	EXPECT_TRUE(0 > p4.compare(p1))
#endif
}

TEST(FilesystemPathTest, test_compare_path_different_root_directory)
{
	Path q1("/");
	Path q2("/");
	EXPECT_EQ(0, q1.compare(q2))

	Path q3("a");
	EXPECT_TRUE(0 > q1.compare(q3))
	EXPECT_TRUE(0 < q3.compare(q1))
#ifdef _WIN32
	Path p1("C:/");
	Path p2("C:/");
	EXPECT_EQ(0, p1.compare(p2))

	Path p3("C:");
	EXPECT_TRUE(0 > p1.compare(p3))
	EXPECT_TRUE(0 < p3.compare(p1))
#endif
}

TEST(FilesystemPathTest, test_compare_path_with_different_relative_path_size)
{
	Path q1("/a/b");
	Path q2("/a/b/c");
	EXPECT_TRUE(0 > q1.compare(q2))
	EXPECT_TRUE(0 < q2.compare(q1))

	EXPECT_TRUE(0 > Path("/aa/ab/").compare(Path("/aa/ac/dd")))
	EXPECT_TRUE(0 < Path("/aa/ac/dd").compare(Path("/aa/ab/")))

	EXPECT_TRUE(0 > Path("/aa/ac/dd").compare(Path("/d")))
	EXPECT_TRUE(0 < Path("/d").compare(Path("/aa/ac/dd")))

	EXPECT_TRUE(0 > Path("aa/abb").compare(Path("aa/ac/dd")))
	EXPECT_TRUE(0 < Path("aa/ac/dd").compare(Path("aa/abb")))
}

TEST(FilesystemPathTest, test_compare_path_both_has_same_relative_path_size)
{
	Path q1("/a/b");
	Path q2("/a/c");
	EXPECT_EQ(0, q1.compare(q1))
	EXPECT_TRUE(0 > q1.compare(q2))
	EXPECT_TRUE(0 < q2.compare(q1))

	EXPECT_TRUE(0 > Path("/aa/").compare(Path("/ac/")))
	EXPECT_TRUE(0 < Path("/ac/").compare(Path("/aa/")))

	EXPECT_TRUE(0 > Path("/aa").compare(Path("/ac")))
	EXPECT_TRUE(0 < Path("/ac").compare(Path("/aa")))

	EXPECT_TRUE(0 > Path("aa/abb").compare(Path("aa/c")))
	EXPECT_TRUE(0 < Path("aa/c").compare(Path("aa/abb")))

	EXPECT_TRUE(0 > Path("aa").compare(Path("ac")))
	EXPECT_TRUE(0 < Path("ac").compare(Path("aa")))
}

TEST(FilesystemPathTest, test_fetch_path_root_name_regular_drive_on_windows)
{
#ifdef _WIN32
	Path p1("C:");
	EXPECT_TRUE(p1.has_root_name())
	EXPECT_EQ("C:", p1.root_name().u8string());

	Path p2("D:/");
	EXPECT_TRUE(p2.has_root_name())
	EXPECT_EQ("D:", p2.root_name().u8string());

	Path p3("E:/bar.txt");
	EXPECT_TRUE(p3.has_root_name())
	EXPECT_EQ("E:", p3.root_name().u8string());

	Path p4("F:/foo/bar.txt");
	EXPECT_TRUE(p4.has_root_name())
	EXPECT_EQ("F:", p4.root_name().u8string());
#endif
}

TEST(FilesystemPathTest, test_fetch_path_root_name_network_samba_folder)
{
	Path p1("\\\\host");
	EXPECT_TRUE(p1.has_root_name())
	EXPECT_EQ("\\\\host", p1.root_name().u8string());

	Path p2("\\\\host2/");
	EXPECT_TRUE(p2.has_root_name())
	EXPECT_EQ("\\\\host2", p2.root_name().u8string());

	Path p3("\\\\host3\\bar.txt");
	EXPECT_TRUE(p3.has_root_name())
	EXPECT_EQ("\\\\host3", p3.root_name().u8string());

	Path p4("\\\\host4\\foo\\bar.txt");
	EXPECT_TRUE(p4.has_root_name())
	EXPECT_EQ("\\\\host4", p4.root_name().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_root_name_when_path_is_empty)
{
	Path p;
	EXPECT_FALSE(p.has_root_name())
	EXPECT_EQ("", p.root_name().u8string())
}

TEST(FilesystemPathTest, test_does_not_fetch_root_name_when_not_absolute)
{
	Path p1("foo");
	EXPECT_FALSE(p1.has_root_name())
	EXPECT_EQ("", p1.root_name().u8string())

	Path p2("foo.bar");
	EXPECT_FALSE(p2.has_root_name())
	EXPECT_EQ("", p2.root_name().u8string())

	Path p3("foo/bar");
	EXPECT_FALSE(p3.has_root_name())
	EXPECT_EQ("", p3.root_name().u8string())

	Path p4("foo/");
	EXPECT_FALSE(p4.has_root_name())
	EXPECT_EQ("", p4.root_name().u8string())
}

TEST(FilesystemPathTest, test_fetch_path_root_directory_regular_drive_on_windows)
{
#ifdef _WIN32
	Path p1("C:");
	EXPECT_FALSE(p1.has_root_directory())
	EXPECT_EQ("", p1.root_directory().u8string());

	Path p2("D:/");
	EXPECT_TRUE(p2.has_root_directory())
	EXPECT_EQ("/", p2.root_directory().u8string());

	Path p3("E:/bar.txt");
	EXPECT_TRUE(p3.has_root_directory())
	EXPECT_EQ("/", p3.root_directory().u8string());

	Path p4("F:/foo/bar.txt");
	EXPECT_TRUE(p4.has_root_directory())
	EXPECT_EQ("/", p4.root_directory().u8string());
#endif
}

TEST(FilesystemPathTest, test_fetch_path_root_directory_network_samba_folder)
{
	Path p1("\\\\host");
	EXPECT_FALSE(p1.has_root_directory())
	EXPECT_EQ("", p1.root_directory().u8string());

	Path p2("\\\\host2/");
	EXPECT_TRUE(p2.has_root_directory())
	EXPECT_EQ("/", p2.root_directory().u8string());

	Path p3("\\\\host3\\bar.txt");
	EXPECT_TRUE(p3.has_root_directory())
	EXPECT_EQ("\\", p3.root_directory().u8string());

	Path p4("\\\\host4\\foo\\bar.txt");
	EXPECT_TRUE(p4.has_root_directory())
	EXPECT_EQ("\\", p4.root_directory().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_root_directory_prepended_slash)
{
	Path p1("/file");
	EXPECT_TRUE(p1.has_root_directory())
	EXPECT_EQ("/", p1.root_directory().u8string());

	Path p2("/foo/bar");
	EXPECT_TRUE(p2.has_root_directory())
	EXPECT_EQ("/", p2.root_directory().u8string());

	Path p3("/foo/bar.txt");
	EXPECT_TRUE(p3.has_root_directory())
	EXPECT_EQ("/", p3.root_directory().u8string());

	Path p4("/");
	EXPECT_TRUE(p4.has_root_directory())
	EXPECT_EQ("/", p4.root_directory().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_root_directory_when_path_is_empty)
{
	Path p;
	EXPECT_FALSE(p.has_root_directory())
	EXPECT_EQ("", p.root_directory().u8string())
}

TEST(FilesystemPathTest, test_does_not_fetch_root_directory_when_not_absolute)
{
	Path p1("foo");
	EXPECT_FALSE(p1.has_root_directory())
	EXPECT_EQ("", p1.root_directory().u8string())

	Path p2("foo.bar");
	EXPECT_FALSE(p2.has_root_directory())
	EXPECT_EQ("", p2.root_directory().u8string())

	Path p3("foo/bar");
	EXPECT_FALSE(p3.has_root_directory())
	EXPECT_EQ("", p3.root_directory().u8string())

	Path p4("foo/");
	EXPECT_FALSE(p4.has_root_directory())
	EXPECT_EQ("", p4.root_directory().u8string())
}

TEST(FilesystemPathTest, test_fetch_relative_path_regular_case)
{
	Path p1("/foo/bar.txt");
	EXPECT_TRUE(p1.has_relative_path())
	EXPECT_EQ("foo/bar.txt", p1.relative_path().u8string());

	Path p2("foo/bar.txt");
	EXPECT_TRUE(p2.has_relative_path())
	EXPECT_EQ("foo/bar.txt", p2.relative_path().u8string());

	Path p3("bar.txt");
	EXPECT_TRUE(p3.has_relative_path())
	EXPECT_EQ("bar.txt", p3.relative_path().u8string());

	Path p4("bar/");
	EXPECT_TRUE(p4.has_relative_path())
	EXPECT_EQ("bar/", p4.relative_path().u8string());

	Path p5("/bar.txt");
	EXPECT_TRUE(p5.has_relative_path())
	EXPECT_EQ("bar.txt", p5.relative_path().u8string());
}

TEST(FilesystemPathTest, test_fetch_relative_path_root_with_multiple_separators)
{
	Path p1("//foo/bar.txt");
	EXPECT_TRUE(p1.has_relative_path())
	EXPECT_EQ("foo/bar.txt", p1.relative_path().u8string());
}

TEST(FilesystemPathTest, test_fetch_relative_path_regular_case_on_windows)
{
#ifdef _WIN32
	Path p1("C:/foo/bar.txt");
	EXPECT_TRUE(p1.has_relative_path())
	EXPECT_EQ("foo/bar.txt", p1.relative_path().u8string());

	Path p2("D:/foo/bar.txt");
	EXPECT_TRUE(p2.has_relative_path())
	EXPECT_EQ("foo/bar.txt", p2.relative_path().u8string());

	Path p3("E:/bar.txt");
	EXPECT_TRUE(p3.has_relative_path())
	EXPECT_EQ("bar.txt", p3.relative_path().u8string());

	Path p4("F:\\bar/");
	EXPECT_TRUE(p4.has_relative_path())
	EXPECT_EQ("bar/", p4.relative_path().u8string());
#endif
}

TEST(FilesystemPathTest, test_fetch_relative_path_with_network_samba_folder)
{
	Path p1("\\\\host/foo/bar.txt");
	EXPECT_TRUE(p1.has_relative_path())
	EXPECT_EQ("foo/bar.txt", p1.relative_path().u8string());

	Path p2("\\\\host2/foo/bar.txt");
	EXPECT_TRUE(p2.has_relative_path())
	EXPECT_EQ("foo/bar.txt", p2.relative_path().u8string());

	Path p3("\\\\host3/bar.txt");
	EXPECT_TRUE(p3.has_relative_path())
	EXPECT_EQ("bar.txt", p3.relative_path().u8string());

	Path p4("\\\\host4\\bar/");
	EXPECT_TRUE(p4.has_relative_path())
	EXPECT_EQ("bar/", p4.relative_path().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_relative_path_when_path_is_empty)
{
	Path p;
	EXPECT_FALSE(p.has_relative_path())
	EXPECT_EQ("", p.relative_path().u8string())
}

TEST(FilesystemPathTest, test_does_not_fetch_relative_path_when_path_is_root)
{
	Path p0("/");
	EXPECT_FALSE(p0.has_relative_path())
	EXPECT_EQ("", p0.relative_path().u8string());

	Path p1("//");
	EXPECT_FALSE(p1.has_relative_path())
	EXPECT_EQ("", p1.relative_path().u8string());

#ifdef _WIN32
	Path p2("C:\\");
	EXPECT_FALSE(p2.has_relative_path())
	EXPECT_EQ("", p2.relative_path().u8string());
#endif

	Path p3("\\\\host");
	EXPECT_FALSE(p3.has_relative_path())
	EXPECT_EQ("", p3.relative_path().u8string());

	Path p4("\\\\host\\");
	EXPECT_FALSE(p4.has_relative_path())
	EXPECT_EQ("", p4.relative_path().u8string());
}

TEST(FilesystemPathTest, test_fetch_relative_path_when_path_ends_with_slash)
{
	Path p1("/foo/");
	EXPECT_TRUE(p1.has_relative_path())
	EXPECT_EQ("foo/", p1.relative_path().u8string());

	Path p2("foo/");
	EXPECT_TRUE(p2.has_relative_path())
	EXPECT_EQ("foo/", p2.relative_path().u8string());
}

TEST(FilesystemPathTest, test_fetch_parent_path_regular_case)
{
	Path p1("/var/tmp/example.txt");
	EXPECT_TRUE(p1.has_parent_path())
	EXPECT_EQ("/var/tmp", p1.parent_path().u8string())

	Path p2("var/tmp/example.txt");
	EXPECT_TRUE(p2.has_parent_path())
	EXPECT_EQ("var/tmp", p2.parent_path().u8string())

	Path p3("var/tmp");
	EXPECT_TRUE(p3.has_parent_path())
	EXPECT_EQ("var", p3.parent_path().u8string())
}

TEST(FilesystemPathTest, test_fetch_parent_path_regular_case_on_windows)
{
	// bug originally reported in PR #2762
#ifdef _WIN32
	Path p1("C:\\foo.bar");
	EXPECT_TRUE(p1.has_parent_path())
	EXPECT_EQ("C:\\", p1.parent_path().u8string())
#endif
}

TEST(FilesystemPathTest, test_fetch_parent_path_with_network_samba_folder_on_windows)
{

}

TEST(FilesystemPathTest, test_does_not_fetch_parent_path_when_path_is_empty)
{
	Path p1;
	EXPECT_FALSE(p1.has_parent_path())
	EXPECT_EQ("", p1.parent_path().u8string())
}

TEST(FilesystemPathTest, test_parent_path_returns_copy_when_path_is_root)
{
	Path p1("/");
	EXPECT_TRUE(p1.has_parent_path())
	EXPECT_EQ("/", p1.parent_path().u8string())

	Path p0("//");
	EXPECT_TRUE(p0.has_parent_path())
	EXPECT_EQ("//", p0.parent_path().u8string())

#ifdef _WIN32
	Path p2("C:\\");
	EXPECT_TRUE(p2.has_parent_path())
	EXPECT_EQ("C:\\", p2.parent_path().u8string())
#endif

	Path p3("\\\\host\\");
	EXPECT_TRUE(p3.has_parent_path())
	EXPECT_EQ("\\\\host\\", p3.parent_path().u8string())

	Path p4("\\\\host");
	EXPECT_TRUE(p4.has_parent_path())
	EXPECT_EQ("\\\\host", p4.parent_path().u8string())
}

TEST(FilesystemPathTest, test_fetch_parent_path_when_path_ends_with_slash)
{
	Path p1("/var/tmp/example/");
	EXPECT_TRUE(p1.has_parent_path())
	EXPECT_EQ("/var/tmp/example", p1.parent_path().u8string())
}

TEST(FilesystemPathTest, test_fetch_parent_path_when_it_ends_with_dot_filename)
{
	Path p1("/var/tmp/.");
	EXPECT_TRUE(p1.has_parent_path())
	EXPECT_EQ("/var/tmp", p1.parent_path().u8string())
}

TEST(FilesystemPathTest, test_fetch_parent_path_when_it_ends_with_dot_dot_filename)
{
	Path p1("/var/tmp/..");
	EXPECT_TRUE(p1.has_parent_path())
	EXPECT_EQ("/var/tmp", p1.parent_path().u8string())
}

TEST(FilesystemPathTest, test_fetch_parent_path_skips_consecutive_separators)
{
	Path p1("/var/tmp//example.txt");
	EXPECT_TRUE(p1.has_parent_path())
	EXPECT_EQ("/var/tmp", p1.parent_path().u8string())

	Path p2("/var/tmp///example.txt");
	EXPECT_TRUE(p2.has_parent_path())
	EXPECT_EQ("/var/tmp", p2.parent_path().u8string())

	Path p3("/var/tmp//example//");
	EXPECT_TRUE(p3.has_parent_path())
	EXPECT_EQ("/var/tmp//example", p3.parent_path().u8string())
}

TEST(FilesystemPathTest, test_fetch_parent_path_when_relative_path_has_only_one_component_and_it_is_not_an_absolute_path)
{
	Path p1("var");
	EXPECT_FALSE(p1.has_parent_path())
	EXPECT_EQ("", p1.parent_path().u8string())

	Path p2(".");
	EXPECT_FALSE(p2.has_parent_path())
	EXPECT_EQ("", p2.parent_path().u8string())

	Path p3("..");
	EXPECT_FALSE(p3.has_parent_path())
	EXPECT_EQ("", p3.parent_path().u8string())
}

TEST(FilesystemPathTest, test_fetch_parent_path_when_relative_path_has_only_one_component_and_it_is_an_absolute_path)
{
	Path p1("/var");
	EXPECT_TRUE(p1.has_parent_path())
	EXPECT_EQ("/", p1.parent_path().u8string())

	Path p2("/.");
	EXPECT_TRUE(p2.has_parent_path())
	EXPECT_EQ("/", p2.parent_path().u8string())

	Path p3("/var.txt");
	EXPECT_TRUE(p3.has_parent_path())
	EXPECT_EQ("/", p3.parent_path().u8string())
}

TEST(FilesystemPathTest, test_fetch_path_filename_regular_case)
{
	Path p1("/foo/bar.txt");
	EXPECT_TRUE(p1.has_filename())
	EXPECT_EQ("bar.txt", p1.filename().u8string());

	Path p2("foo/bar.txt");
	EXPECT_TRUE(p2.has_filename())
	EXPECT_EQ("bar.txt", p2.filename().u8string());

	Path p3("bar.txt");
	EXPECT_TRUE(p3.has_filename())
	EXPECT_EQ("bar.txt", p3.filename().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_filename_when_path_is_empty)
{
	Path p;
	EXPECT_FALSE(p.has_filename())
	EXPECT_EQ("", p.filename().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_filename_when_path_is_root)
{
	Path p1("/");
	EXPECT_FALSE(p1.has_filename())
	EXPECT_EQ("", p1.filename().u8string());

	Path p2("C:\\");
	EXPECT_FALSE(p2.has_filename())
	EXPECT_EQ("", p2.filename().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_filename_when_path_does_not_have_any_extension)
{
	Path p1("/foo/bar");
	EXPECT_TRUE(p1.has_filename())
	EXPECT_EQ("bar", p1.filename().u8string());

	Path p2("foo/bar");
	EXPECT_TRUE(p2.has_filename())
	EXPECT_EQ("bar", p2.filename().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_filename_when_path_ends_with_slash)
{
	Path p1("/foo/bar/");
	EXPECT_FALSE(p1.has_filename())
	EXPECT_EQ("", p1.filename().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_filename_when_path_filename_starts_with_dot_char)
{
	Path p1("/foo/.bar");
	EXPECT_TRUE(p1.has_filename())
	EXPECT_EQ(".bar", p1.filename().u8string());

	Path p2("foo/.bar");
	EXPECT_TRUE(p2.has_filename())
	EXPECT_EQ(".bar", p2.filename().u8string());}

TEST(FilesystemPathTest, test_fetch_path_filename_when_path_filename_ends_with_dot_char)
{
	Path p("/foo/bar.");
	EXPECT_TRUE(p.has_filename())
	EXPECT_EQ("bar.", p.filename().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_filename_when_is_dot_filename)
{
	Path p1("/foo/.");
	EXPECT_TRUE(p1.has_filename())
	EXPECT_EQ(".", p1.filename().u8string());

	Path p2(".");
	EXPECT_TRUE(p2.has_filename())
	EXPECT_EQ(".", p2.filename().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_filename_when_is_dot_dot_filename)
{
	Path p1("/foo/..");
	EXPECT_TRUE(p1.has_filename())
	EXPECT_EQ("..", p1.filename().u8string());

	Path p2("..");
	EXPECT_TRUE(p2.has_filename())
	EXPECT_EQ("..", p2.filename().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_stem_regular_case)
{
	Path p1("/foo/bar.txt");
	EXPECT_TRUE(p1.has_stem())
	EXPECT_EQ("bar", p1.stem().u8string());

	Path p2("foo/bar.txt");
	EXPECT_TRUE(p2.has_stem())
	EXPECT_EQ("bar", p2.stem().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_stem_when_path_is_empty)
{
	Path p;
	EXPECT_FALSE(p.has_stem())
	EXPECT_EQ("", p.stem().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_stem_when_path_is_root)
{
	Path p1("/");
	EXPECT_FALSE(p1.has_stem())
	EXPECT_EQ("", p1.stem().u8string());

	Path p2("C:\\");
	EXPECT_FALSE(p2.has_stem())
	EXPECT_EQ("", p2.stem().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_stem_when_path_does_not_have_any_extension)
{
	Path p1("/foo/bar");
	EXPECT_TRUE(p1.has_stem())
	EXPECT_EQ("bar", p1.stem().u8string());

	Path p2("foo/bar");
	EXPECT_TRUE(p2.has_stem())
	EXPECT_EQ("bar", p2.stem().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_stem_when_path_ends_with_slash)
{
	Path p1("/foo/bar/");
	EXPECT_FALSE(p1.has_stem())
	EXPECT_EQ("", p1.stem().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_stem_when_path_filename_starts_with_dot_char)
{
	Path p1("/foo/.bar");
	EXPECT_TRUE(p1.has_stem())
	EXPECT_EQ(".bar", p1.stem().u8string());

	Path p2("foo/.bar");
	EXPECT_TRUE(p2.has_stem())
	EXPECT_EQ(".bar", p2.stem().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_stem_when_path_filename_ends_with_dot_char)
{
	Path p("/foo/bar.");
	EXPECT_TRUE(p.has_stem())
	EXPECT_EQ("bar", p.stem().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_stem_when_is_dot_filename)
{
	Path p1("/foo/.");
	EXPECT_TRUE(p1.has_stem())
	EXPECT_EQ(".", p1.stem().u8string());

	Path p2(".");
	EXPECT_TRUE(p2.has_stem())
	EXPECT_EQ(".", p2.stem().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_stem_when_is_dot_dot_filename)
{
	Path p1("/foo/..");
	EXPECT_TRUE(p1.has_stem())
	EXPECT_EQ("..", p1.stem().u8string());

	Path p2("..");
	EXPECT_TRUE(p2.has_stem())
	EXPECT_EQ("..", p2.stem().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_stem_correctly_when_intermediate_path_has_dot_char)
{
	Path p1("/foo/bar.txt/play.dd");
	EXPECT_TRUE(p1.has_stem())
	EXPECT_EQ("play", p1.stem().u8string());

	Path p2("/foo/bar.txt/play.");
	EXPECT_TRUE(p2.has_stem())
	EXPECT_EQ("play", p2.stem().u8string());

	Path p3("/foo/bar.txt/play");
	EXPECT_TRUE(p3.has_stem())
	EXPECT_EQ("play", p3.stem().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_stem_correctly_when_weird_case_dotchar_dotchar_extension)
{
	Path p1("/foo/..weird");
	EXPECT_TRUE(p1.has_stem())
	EXPECT_EQ(".", p1.stem().u8string());

	Path p2("..weird");
	EXPECT_TRUE(p2.has_stem())
	EXPECT_EQ(".", p2.stem().u8string());
}
TEST(FilesystemPathTest, test_fetch_path_extension_regular_case)
{
	Path p("/foo/bar.txt");
	EXPECT_TRUE(p.has_extension())
	EXPECT_EQ(".txt", p.extension().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_extension_when_path_is_empty)
{
	Path p;
	EXPECT_FALSE(p.has_extension())
	EXPECT_EQ("", p.extension().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_extension_when_path_is_root)
{
	Path p1("/");
	EXPECT_FALSE(p1.has_extension())
	EXPECT_EQ("", p1.extension().u8string());

	Path p2("C:\\");
	EXPECT_FALSE(p2.has_extension())
	EXPECT_EQ("", p2.extension().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_extension_when_path_does_not_have_any_extension)
{
	Path p("/foo/bar");
	EXPECT_FALSE(p.has_extension())
	EXPECT_EQ("", p.extension().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_extension_when_path_ends_with_slash)
{
	Path p("/foo/bar/");
	EXPECT_FALSE(p.has_extension())
	EXPECT_EQ("", p.extension().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_extension_when_path_filename_starts_with_dot_char)
{
	Path p("/foo/.bar");
	EXPECT_FALSE(p.has_extension())
	EXPECT_EQ("", p.extension().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_extension_when_path_filename_ends_with_dot_char)
{
	Path p("/foo/bar.");
	EXPECT_TRUE(p.has_extension())
	EXPECT_EQ(".", p.extension().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_extension_when_is_dot_filename)
{
	Path p1("/foo/.");
	EXPECT_FALSE(p1.has_extension())
	EXPECT_EQ("", p1.extension().u8string());

	Path p2(".");
	EXPECT_FALSE(p2.has_extension())
	EXPECT_EQ("", p2.extension().u8string());
}

TEST(FilesystemPathTest, test_does_not_fetch_path_extension_when_is_dot_dot_filename)
{
	Path p1("/foo/..");
	EXPECT_FALSE(p1.has_extension())
	EXPECT_EQ("", p1.extension().u8string());

	Path p2("..");
	EXPECT_FALSE(p2.has_extension())
	EXPECT_EQ("", p2.extension().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_extension_correctly_when_intermediate_path_has_dot_char)
{
	Path p1("/foo/bar.txt/play.dd");
	EXPECT_TRUE(p1.has_extension())
	EXPECT_EQ(".dd", p1.extension().u8string());

	Path p2("/foo/bar.txt/play.");
	EXPECT_TRUE(p2.has_extension())
	EXPECT_EQ(".", p2.extension().u8string());

	Path p3("/foo/bar.txt/play");
	EXPECT_FALSE(p3.has_extension())
	EXPECT_EQ("", p3.extension().u8string());
}

TEST(FilesystemPathTest, test_fetch_path_extension_correctly_when_weird_case_dotchar_dotchar_extension)
{
	Path p1("/foo/..weird");
	EXPECT_TRUE(p1.has_extension())
	EXPECT_EQ(".weird", p1.extension().u8string());

	Path p2("..weird");
	EXPECT_TRUE(p2.has_extension())
	EXPECT_EQ(".weird", p2.extension().u8string());
}

TEST(FilesystemPathTest, test_is_absolute)
{
#ifndef _WIN32
	Path p1("/");
	EXPECT_TRUE(p1.is_absolute())
	EXPECT_FALSE(p1.is_relative())

	Path p2("/a");
	EXPECT_TRUE(p2.is_absolute())
	EXPECT_FALSE(p2.is_relative())

	Path p3("/.");
	EXPECT_TRUE(p3.is_absolute())
	EXPECT_FALSE(p3.is_relative())

	Path p4("/..");
	EXPECT_TRUE(p4.is_absolute())
	EXPECT_FALSE(p4.is_relative())

	Path p5("/foo/");
	EXPECT_TRUE(p5.is_absolute())
	EXPECT_FALSE(p5.is_relative())

	Path p6("/foo/bar");
	EXPECT_TRUE(p6.is_absolute())
	EXPECT_FALSE(p6.is_relative())
#endif

	Path p7(".");
	EXPECT_FALSE(p7.is_absolute())
	EXPECT_TRUE(p7.is_relative())

	Path p8("a");
	EXPECT_FALSE(p8.is_absolute())
	EXPECT_TRUE(p8.is_relative())

	Path p9("./");
	EXPECT_FALSE(p9.is_absolute())
	EXPECT_TRUE(p9.is_relative())

	Path p10("..");
	EXPECT_FALSE(p10.is_absolute())
	EXPECT_TRUE(p10.is_relative())

	Path p11("foo/");
	EXPECT_FALSE(p11.is_absolute())
	EXPECT_TRUE(p11.is_relative())

	Path p12("foo/bar");
	EXPECT_FALSE(p12.is_absolute())
	EXPECT_TRUE(p12.is_relative())
}

TEST(FilesystemPathTest, test_is_not_absolute_on_windows_if_path_starts_with_root_directory)
{
#ifdef _WIN32
	Path p1("/");
	EXPECT_FALSE(p1.is_absolute())
	EXPECT_TRUE(p1.is_relative())

	Path p2("/a");
	EXPECT_FALSE(p2.is_absolute())
	EXPECT_TRUE(p2.is_relative())

	Path p3("/.");
	EXPECT_FALSE(p3.is_absolute())
	EXPECT_TRUE(p3.is_relative())

	Path p4("/..");
	EXPECT_FALSE(p4.is_absolute())
	EXPECT_TRUE(p4.is_relative())

	Path p5("/foo/");
	EXPECT_FALSE(p5.is_absolute())
	EXPECT_TRUE(p5.is_relative())

	Path p6("/foo/bar");
	EXPECT_FALSE(p6.is_absolute())
	EXPECT_TRUE(p6.is_relative())
#endif
}

TEST(FilesystemPathTest, test_is_absolute_on_windows_with_drive)
{
#ifdef _WIN32
	Path p1("C:\\");
	EXPECT_TRUE(p1.is_absolute())
	EXPECT_FALSE(p1.is_relative())

	Path p2("C:\\a");
	EXPECT_TRUE(p2.is_absolute())
	EXPECT_FALSE(p2.is_relative())

	Path p3("C:\\.");
	EXPECT_TRUE(p3.is_absolute())
	EXPECT_FALSE(p3.is_relative())

	Path p4("C:\\..");
	EXPECT_TRUE(p4.is_absolute())
	EXPECT_FALSE(p4.is_relative())

	Path p5("C:\\foo/");
	EXPECT_TRUE(p5.is_absolute())
	EXPECT_FALSE(p5.is_relative())

	Path p6("C:\\foo/bar");
	EXPECT_TRUE(p6.is_absolute())
	EXPECT_FALSE(p6.is_relative())
#endif
}

TEST(FilesystemPathTest, test_is_absolute_on_windows_with_network_samba_folder)
{
	Path p1("\\\\host\\");
	EXPECT_TRUE(p1.is_absolute())
	EXPECT_FALSE(p1.is_relative())

	Path p2("\\\\host\\a");
	EXPECT_TRUE(p2.is_absolute())
	EXPECT_FALSE(p2.is_relative())

	Path p3("\\\\host\\.");
	EXPECT_TRUE(p3.is_absolute())
	EXPECT_FALSE(p3.is_relative())

	Path p4("\\\\host\\..");
	EXPECT_TRUE(p4.is_absolute())
	EXPECT_FALSE(p4.is_relative())

	Path p5("\\\\host\\foo/");
	EXPECT_TRUE(p5.is_absolute())
	EXPECT_FALSE(p5.is_relative())

	Path p6("\\\\host\\foo/bar");
	EXPECT_TRUE(p6.is_absolute())
	EXPECT_FALSE(p6.is_relative())
}

TEST(FilesystemPathTest, test_normalize_empty_path_keeps_empty)
{
	EXPECT_TRUE(Path().lexically_normal().empty())
}

TEST(FilesystemPathTest, test_normalize_path_converts_backlash_to_slash)
{
	EXPECT_EQ("/a/b/c", Path("\\a\\b\\c").lexically_normal().u8string())
	EXPECT_EQ("/a/b/c/", Path("\\a\\b\\c\\").lexically_normal().u8string())
	EXPECT_EQ("a/b/c", Path("a\\b\\c").lexically_normal().u8string())
	EXPECT_EQ("a/b/c/", Path("a\\b\\c\\").lexically_normal().u8string())
	EXPECT_EQ("/", Path("\\").lexically_normal().u8string())
	EXPECT_EQ("C:/", Path("C:\\").lexically_normal().u8string())
}

TEST(FilesystemPathTest, test_normalize_does_not_change_double_slash_of_windows_samba_network_shared_host)
{
	EXPECT_EQ("\\\\host", Path("\\\\host").lexically_normal().u8string())
	EXPECT_EQ("\\\\host/", Path("\\\\host\\").lexically_normal().u8string())
}

TEST(FilesystemPathTest, test_normalize_dot_path_is_dot_without_trailing_slash)
{
	EXPECT_EQ(".", Path("./").lexically_normal().u8string())
}

TEST(FilesystemPathTest, test_normalize_path_with_trailing_dot_dot_slash_removes_slash)
{
	EXPECT_EQ("..", Path("../").lexically_normal().u8string())
}

TEST(FilesystemPathTest, test_normalize_removes_double_slashes)
{
	EXPECT_EQ("a/b", Path("a//b").lexically_normal().u8string())
	EXPECT_EQ("/a/b", Path("//a//b").lexically_normal().u8string())
	EXPECT_EQ("a/b/", Path("a//b//").lexically_normal().u8string())
	EXPECT_EQ("a/b", Path("a//b").lexically_normal().u8string())
}

TEST(FilesystemPathTest, test_normalize_removes_intermediate_special_dot_folder)
{
	EXPECT_EQ("a/b", Path("a/./b").lexically_normal().u8string())
	EXPECT_EQ("a", Path("./././a").lexically_normal().u8string())
	EXPECT_EQ("b", Path("./b").lexically_normal().u8string())
	EXPECT_EQ("/a/b", Path("/./a/./b").lexically_normal().u8string())
	EXPECT_EQ("/a", Path("/./././a").lexically_normal().u8string())
	EXPECT_EQ("/b", Path("/./b").lexically_normal().u8string())
	EXPECT_EQ("a/", Path("a/.").lexically_normal().u8string())
	EXPECT_EQ("a/", Path("a/././.").lexically_normal().u8string())
	EXPECT_EQ("a/", Path("a/./").lexically_normal().u8string())
}

TEST(FilesystemPathTest, test_normalize_removes_intermediate_special_dot_dot_folder)
{
	EXPECT_EQ("/b", Path("/a/../b").lexically_normal().u8string())
	EXPECT_EQ("/a/", Path("/a/b/..").lexically_normal().u8string())
	EXPECT_EQ("/a/", Path("/a/b/../").lexically_normal().u8string())
	EXPECT_EQ("/b", Path("/a/../../b").lexically_normal().u8string())
	EXPECT_EQ("/c", Path("/a/b/../../c").lexically_normal().u8string())
	EXPECT_EQ("/c", Path("/a/../b/../c").lexically_normal().u8string())

	EXPECT_EQ("b", Path("a/../b").lexically_normal().u8string())
	EXPECT_EQ("a/", Path("a/b/..").lexically_normal().u8string())
	EXPECT_EQ("a/", Path("a/b/../").lexically_normal().u8string())
	EXPECT_EQ("../b", Path("a/../../b").lexically_normal().u8string())
	EXPECT_EQ("c", Path("a/b/../../c").lexically_normal().u8string())
	EXPECT_EQ("../../a", Path("../../a").lexically_normal().u8string())
}

TEST(FilesystemPathTest, test_normalize_does_not_remove_component_with_prepended_dot)
{
	EXPECT_EQ(".foo", Path(".foo").lexically_normal().u8string())
	EXPECT_EQ("/.foo", Path("/.foo").lexically_normal().u8string())
	EXPECT_EQ(".a", Path(".a").lexically_normal().u8string())
	EXPECT_EQ("foo/.bar", Path("foo/.bar").lexically_normal().u8string())
	EXPECT_EQ(".bar", Path(".foo/../.bar").lexically_normal().u8string())
	EXPECT_EQ(".foo/.bar/", Path(".foo/./.bar/.").lexically_normal().u8string())
	EXPECT_EQ("foo/.b", Path("foo/.b").lexically_normal().u8string())
	EXPECT_EQ("foo/.bar/end", Path("foo/.bar/end").lexically_normal().u8string())
}

TEST(FilesystemPathTest, test_normalize_does_not_remove_component_with_prepended_dot_dot)
{
	EXPECT_EQ("..foo", Path("..foo").lexically_normal().u8string())
	EXPECT_EQ("/..foo", Path("/..foo").lexically_normal().u8string())
	EXPECT_EQ("..a", Path("..a").lexically_normal().u8string())
	EXPECT_EQ("foo/..bar", Path("foo/..bar").lexically_normal().u8string())
	EXPECT_EQ("..bar", Path(".foo/../..bar").lexically_normal().u8string())
	EXPECT_EQ(".foo/..bar/", Path(".foo/./..bar/.").lexically_normal().u8string())
	EXPECT_EQ("foo/..b", Path("foo/..b").lexically_normal().u8string())
	EXPECT_EQ("foo/..bar/end", Path("foo/..bar/end").lexically_normal().u8string())
}

TEST(FilesystemPathTest, test_normalize_remove_special_dot_dot_right_after_root)
{
	EXPECT_EQ("/", Path("/..").lexically_normal().u8string())
	EXPECT_EQ("/a", Path("/../../a").lexically_normal().u8string())
	EXPECT_EQ("/c", Path("/../b/../c").lexically_normal().u8string())
}

TEST(FilesystemPathTest, test_normalize_examples_from_cpp_reference_dot_com)
{
	// https://en.cppreference.com/w/cpp/filesystem/path/lexically_normal
	EXPECT_EQ("a/", Path("a/./b/..").lexically_normal().u8string());
	EXPECT_EQ("a/", Path("a/.///b/../").lexically_normal().u8string());
}

TEST(FilesystemPathTest, test_relative)
{
	EXPECT_EQ("../../d", Path("/a/d").lexically_relative(Path("/a/b/c")).u8string());
	EXPECT_EQ("../b/c", Path("/a/b/c").lexically_relative(Path("/a/d")).u8string());
	EXPECT_EQ("b/c", Path("a/b/c").lexically_relative(Path("a")).u8string());
	EXPECT_EQ("../..", Path("a/b/c").lexically_relative(Path("a/b/c/x/y")).u8string());
	EXPECT_EQ(".", Path("a/b/c").lexically_relative(Path("a/b/c")).u8string());
	EXPECT_EQ("../../a/b", Path("a/b").lexically_relative(Path("c/d")).u8string());
	EXPECT_EQ("", Path("a/b").lexically_relative(Path("/a/b")).u8string());
}

TEST(FilesystemPathTest, test_relative_to_empty_path_returns_itself)
{
	EXPECT_EQ("a/e.fg", Path("a/e.fg").lexically_relative(Path()).u8string());
	EXPECT_EQ("e.fg", Path("e.fg").lexically_relative(Path()).u8string());
	// exception: absolute path
#ifdef _WIN32
	EXPECT_EQ("", Path("C:/a/d").lexically_relative(Path()).u8string());
#else
	EXPECT_EQ("", Path("/a/d").lexically_relative(Path()).u8string());
#endif
}

TEST(FilesystemPathTest, test_empty_path_relative_to_another_returns_as_it_was_special_dot_file)
{
	EXPECT_EQ("../..", Path().lexically_relative(Path("a/e.fg")).u8string());
	EXPECT_EQ("..", Path().lexically_relative(Path("e.fg")).u8string());
	// exception: absolute path
#ifdef _WIN32
	EXPECT_EQ("", Path().lexically_relative(Path("C:/a/d")).u8string());
#else
	EXPECT_EQ("", Path().lexically_relative(Path("/a/d")).u8string());
#endif
}

TEST(FilesystemPathTest, test_relative_ported_from_old_etl_stringf)
{
	EXPECT_EQ("myfile.txt", Path("/home/darco/projects/voria/myfile.txt").lexically_relative(Path("/home/darco/projects/voria")).u8string())

	EXPECT_EQ("files/myfile.txt", Path("/home/darco/projects/voria/files/myfile.txt").lexically_relative(Path("/home/darco/projects/voria")).u8string())

	EXPECT_EQ("../../share", Path("/usr/share").lexically_relative(Path("/usr/local/bin/.")).u8string())
}

TEST(FilesystemPathTest, test_fake_relative_from_cpp_reference_dot_com)
{
	// https://en.cppreference.com/w/cpp/filesystem/relative
#ifdef _WIN32
	EXPECT_EQ("c", Path("C:/a/b/c").relative_to(Path("C:/a/b")).u8string());
	EXPECT_EQ("../c", Path("C:/a/c").relative_to(Path("C:/a/b")).u8string());
	EXPECT_EQ("", Path("c").relative_to(Path("C:/a/b")).u8string());
	EXPECT_EQ("", Path("C:/a/b").relative_to(Path("c")).u8string());
#else
	EXPECT_EQ("c", Path("/a/b/c").relative_to(Path("/a/b")).u8string());
	EXPECT_EQ("../c", Path("/a/c").relative_to(Path("/a/b")).u8string());
	EXPECT_EQ("", Path("c").relative_to(Path("/a/b")).u8string());
	EXPECT_EQ("", Path("/a/b").relative_to(Path("c")).u8string());
#endif
}

TEST(FilesystemPathTest, test_relative_between_different_root_paths)
{
#ifdef _WIN32
	EXPECT_EQ("", Path("D:/a/b/c").relative_to(Path("C:/a/b")).u8string());
	EXPECT_EQ("", Path("D:/a/c").relative_to(Path("C:/a/b")).u8string());
#endif
}

TEST(FilesystemPathTest, test_lexically_proximate_from_cpp_reference_dot_com)
{
	// https://en.cppreference.com/w/cpp/filesystem/path/lexically_normal
	EXPECT_EQ("a/b", Path("a/b").lexically_proximate(Path("/a/b")).u8string());
}

TEST(FilesystemPathTest, test_fake_proximate_from_cpp_reference_dot_com)
{
	// https://en.cppreference.com/w/cpp/filesystem/relative
#ifdef _WIN32
	EXPECT_EQ("c", Path("C:/a/b/c").proximate_to(Path("C:/a/b")).u8string());
	EXPECT_EQ("../c", Path("C:/a/c").proximate_to(Path("C:/a/b")).u8string());
	EXPECT_EQ("c", Path("c").proximate_to(Path("C:/a/b")).u8string());
	EXPECT_EQ("C:/a/b", Path("C:/a/b").proximate_to(Path("c")).u8string());
#else
	EXPECT_EQ("c", Path("/a/b/c").proximate_to(Path("/a/b")).u8string());
	EXPECT_EQ("../c", Path("/a/c").proximate_to(Path("/a/b")).u8string());
	EXPECT_EQ("c", Path("c").proximate_to(Path("/a/b")).u8string());
	EXPECT_EQ("/a/b", Path("/a/b").proximate_to(Path("c")).u8string());
#endif
}

TEST(FilesystemPathTest, test_proximate_between_different_root_paths)
{
#ifdef _WIN32
	EXPECT_EQ("D:/a/b/c", Path("D:/a/b/c").proximate_to(Path("C:/a/b")).u8string());
	EXPECT_EQ("D:/a/c", Path("D:/a/c").proximate_to(Path("C:/a/b")).u8string());
#endif
}

TEST(FilesystemPathTest, test_from_uri)
{
	EXPECT_EQ("/path/to/file", Path::from_uri("file:///path/to/file").u8string());
	EXPECT_EQ("/path/to/file", Path::from_uri("file:/path/to/file").u8string());
	EXPECT_EQ("/path/to/file", Path::from_uri("file://localhost/path/to/file").u8string());

	EXPECT_EQ("c:/path/to/file", Path::from_uri("file:c:/path/to/file").u8string());
	EXPECT_EQ("c:/path/to/file", Path::from_uri("file:///c:/path/to/file").u8string());
	EXPECT_EQ("c:/path/to/file", Path::from_uri("file:/c:/path/to/file").u8string());
	EXPECT_EQ("c:/path/to/file", Path::from_uri("file://localhost/c:/path/to/file").u8string());

	EXPECT_EQ("c:/path/to/file", Path::from_uri("file:c|/path/to/file").u8string());
	EXPECT_EQ("c:/path/to/file", Path::from_uri("file:///c|/path/to/file").u8string());
	EXPECT_EQ("c:/path/to/file", Path::from_uri("file:/c|/path/to/file").u8string());
	EXPECT_EQ("c:/path/to/file", Path::from_uri("file://localhost/c|/path/to/file").u8string());

	EXPECT_EQ("\\\\host.example.com/path/to/file", Path::from_uri("file://host.example.com/path/to/file").u8string());
	EXPECT_EQ("\\\\host.example.com/path/to/file", Path::from_uri("file:////host.example.com/path/to/file").u8string());
	EXPECT_EQ("\\\\host.example.com/path/to/file", Path::from_uri("file://///host.example.com/path/to/file").u8string());
}

TEST(FilesystemPathTest, test_from_uri_with_escapes)
{
	EXPECT_EQ("/path/to/file with spaces", Path::from_uri("file:///path/to/file%20with%20spaces").u8string());

	EXPECT_EQ("/path/to/file.a", Path::from_uri("file:///path/to/file%2ea").u8string());
	EXPECT_EQ("/path/to/file.b", Path::from_uri("file:///path/to/file%2Eb").u8string());
	EXPECT_EQ("/path/to/.file", Path::from_uri("file:///path/to/%2Efile").u8string());

	EXPECT_EQ("/path/to/.file", Path::from_uri("file:///path/to/%2Efile").u8string());

	EXPECT_EQ("/path/to/file\xc3\xb3", Path::from_uri("file:///path/to/file%c3%B3").u8string());
}
