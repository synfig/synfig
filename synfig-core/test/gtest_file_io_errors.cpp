/* === S Y N F I G ========================================================= */
/*!	\file gtest_file_io_errors.cpp
**	\brief Tests for error handling with malformed files
**
**	\legal
**	Copyright (c) 2024 Synfig contributors
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
/* ========================================================================= */

#include <gtest/gtest.h>

#include <synfig/general.h>
#include <synfig/main.h>
#include <synfig/canvas.h>
#include <synfig/loadcanvas.h>
#include <synfig/filesystemnative.h>

#include <cstdio>
#include <fstream>
#include <string>

using namespace synfig;

class FileIOErrorTest : public ::testing::Test {
protected:
	static std::unique_ptr<Main> main_instance;
	std::vector<std::string> temp_files;

	static void SetUpTestSuite() {
		if (!main_instance)
			main_instance = std::make_unique<Main>(".");
	}

	/// Write content to a temporary file and track it for cleanup
	std::string write_temp_file(const std::string& name, const std::string& content) {
		std::string path = "fixtures/tmp_malformed_" + name;
		std::ofstream ofs(path, std::ios::binary);
		ofs << content;
		ofs.close();
		temp_files.push_back(path);
		return path;
	}

	void TearDown() override {
		for (const auto& f : temp_files)
			std::remove(f.c_str());
	}
};

std::unique_ptr<Main> FileIOErrorTest::main_instance;

TEST_F(FileIOErrorTest, EmptyFileReturnsNull) {
	std::string path = write_temp_file("empty.sif", "");
	String errors, warnings;
	FileSystem::Identifier id(FileSystemNative::instance(), path);
	Canvas::Handle canvas = open_canvas_as(id, path, errors, warnings);
	EXPECT_EQ(canvas, nullptr);
}

TEST_F(FileIOErrorTest, InvalidXmlReturnsNull) {
	std::string path = write_temp_file("invalid_xml.sif",
		"<<<this is not valid xml at all>>>");
	String errors, warnings;
	FileSystem::Identifier id(FileSystemNative::instance(), path);
	Canvas::Handle canvas = open_canvas_as(id, path, errors, warnings);
	EXPECT_EQ(canvas, nullptr);
}

TEST_F(FileIOErrorTest, TruncatedXmlReturnsNull) {
	std::string path = write_temp_file("truncated.sif",
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<canvas version=\"1.2\" width=\"480\" height=\"270\">\n"
		"  <layer type=\"circle\">\n"
		"    <param name=\"z_depth\">\n");
		// Intentionally truncated - no closing tags
	String errors, warnings;
	FileSystem::Identifier id(FileSystemNative::instance(), path);
	Canvas::Handle canvas = open_canvas_as(id, path, errors, warnings);
	EXPECT_EQ(canvas, nullptr);
}

TEST_F(FileIOErrorTest, WrongRootElementReturnsNull) {
	std::string path = write_temp_file("wrong_root.sif",
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<html><body>Not a synfig file</body></html>");
	String errors, warnings;
	FileSystem::Identifier id(FileSystemNative::instance(), path);
	Canvas::Handle canvas = open_canvas_as(id, path, errors, warnings);
	EXPECT_EQ(canvas, nullptr);
}

TEST_F(FileIOErrorTest, BinaryGarbageReturnsNull) {
	std::string garbage(256, '\0');
	for (int i = 0; i < 256; ++i)
		garbage[i] = static_cast<char>(i);
	std::string path = write_temp_file("binary_garbage.sif", garbage);
	String errors, warnings;
	FileSystem::Identifier id(FileSystemNative::instance(), path);
	Canvas::Handle canvas = open_canvas_as(id, path, errors, warnings);
	EXPECT_EQ(canvas, nullptr);
}

TEST_F(FileIOErrorTest, NonExistentFileProducesErrors) {
	String errors, warnings;
	std::string path = "fixtures/this_file_does_not_exist_12345.sif";
	FileSystem::Identifier id(FileSystemNative::instance(), path);
	Canvas::Handle canvas = open_canvas_as(id, path, errors, warnings);
	EXPECT_EQ(canvas, nullptr);
}
