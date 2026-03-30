/* === S Y N F I G ========================================================= */
/*!	\file gtest_export_format.cpp
**	\brief Tests for export format registration and round-trip
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
#include <synfig/savecanvas.h>
#include <synfig/filesystemnative.h>
#include <synfig/target.h>

#include "test_utils.h"

using namespace synfig;

class ExportFormatTest : public ::testing::Test {
protected:
	static std::unique_ptr<Main> main_instance;

	static void SetUpTestSuite() {
		if (!main_instance)
			main_instance = std::make_unique<Main>(".");
	}

	Canvas::Handle load_fixture(const std::string& filename) {
		String errors, warnings;
		std::string path = test::fixture_path(filename);
		FileSystem::Identifier id(FileSystemNative::instance(), path);
		return open_canvas_as(id, path, errors, warnings);
	}
};

std::unique_ptr<Main> ExportFormatTest::main_instance;

// --- Target module registration tests ---

TEST_F(ExportFormatTest, TargetBookIsNotEmpty) {
	const Target::Book& book = Target::book();
	EXPECT_GT(book.size(), 0u) << "Target book should have registered targets";
}

TEST_F(ExportFormatTest, ExtBookIsNotEmpty) {
	const Target::ExtBook& ebook = Target::ext_book();
	EXPECT_GT(ebook.size(), 0u) << "Extension book should have registered extensions";
}

TEST_F(ExportFormatTest, PngTargetRegistered) {
	const Target::ExtBook& ebook = Target::ext_book();
	bool found = ebook.find("png") != ebook.end();
	EXPECT_TRUE(found) << "PNG target should be registered in ext_book";
}

TEST_F(ExportFormatTest, PpmTargetRegistered) {
	// PPM is a built-in target that should always be available
	const Target::ExtBook& ebook = Target::ext_book();
	bool found = ebook.find("ppm") != ebook.end();
	EXPECT_TRUE(found) << "PPM target should be registered in ext_book";
}

TEST_F(ExportFormatTest, TargetBookEntriesHaveFactories) {
	const Target::Book& book = Target::book();
	for (auto it = book.begin(); it != book.end(); ++it) {
		EXPECT_NE(it->second.factory, nullptr)
			<< "Target '" << it->first << "' should have a valid factory";
	}
}

TEST_F(ExportFormatTest, TargetBookEntriesHaveExtensions) {
	const Target::Book& book = Target::book();
	for (auto it = book.begin(); it != book.end(); ++it) {
		// file_extension can be empty for some targets (e.g. null target)
		// but most should have one
		if (it->first != "null") {
			EXPECT_FALSE(it->second.file_extension.empty())
				<< "Target '" << it->first << "' should have a file extension";
		}
	}
}

// --- Canvas save/load round-trip tests ---

TEST_F(ExportFormatTest, SaveAndReloadPreservesDimensions) {
	Canvas::Handle original = load_fixture("gradient.sif");
	ASSERT_NE(original, nullptr);

	int orig_w = original->get_w();
	int orig_h = original->get_h();

	// Save to a temporary .sif file
	std::string tmp_path = "fixtures/tmp_roundtrip_test.sif";
	FileSystem::Identifier tmp_id(FileSystemNative::instance(), tmp_path);
	bool saved = save_canvas(tmp_id, original);
	ASSERT_TRUE(saved) << "Failed to save canvas to " << tmp_path;

	// Reload the saved file
	String errors, warnings;
	FileSystem::Identifier reload_id(FileSystemNative::instance(), tmp_path);
	Canvas::Handle reloaded = open_canvas_as(reload_id, tmp_path, errors, warnings);
	ASSERT_NE(reloaded, nullptr) << "Failed to reload saved canvas: " << errors;

	EXPECT_EQ(reloaded->get_w(), orig_w) << "Width should be preserved after round-trip";
	EXPECT_EQ(reloaded->get_h(), orig_h) << "Height should be preserved after round-trip";

	// Cleanup
	std::remove(tmp_path.c_str());
}

TEST_F(ExportFormatTest, SaveAndReloadPreservesFrameRate) {
	Canvas::Handle original = load_fixture("gradient.sif");
	ASSERT_NE(original, nullptr);

	float orig_fps = original->rend_desc().get_frame_rate();

	std::string tmp_path = "fixtures/tmp_roundtrip_fps_test.sif";
	FileSystem::Identifier tmp_id(FileSystemNative::instance(), tmp_path);
	bool saved = save_canvas(tmp_id, original);
	ASSERT_TRUE(saved);

	String errors, warnings;
	FileSystem::Identifier reload_id(FileSystemNative::instance(), tmp_path);
	Canvas::Handle reloaded = open_canvas_as(reload_id, tmp_path, errors, warnings);
	ASSERT_NE(reloaded, nullptr);

	EXPECT_NEAR(reloaded->rend_desc().get_frame_rate(), orig_fps, 0.01f)
		<< "Frame rate should be preserved after round-trip";

	std::remove(tmp_path.c_str());
}

TEST_F(ExportFormatTest, SaveAndReloadPreservesLayerCount) {
	Canvas::Handle original = load_fixture("basic_shapes.sif");
	ASSERT_NE(original, nullptr);

	size_t orig_layer_count = original->size();

	std::string tmp_path = "fixtures/tmp_roundtrip_layers_test.sif";
	FileSystem::Identifier tmp_id(FileSystemNative::instance(), tmp_path);
	bool saved = save_canvas(tmp_id, original);
	ASSERT_TRUE(saved);

	String errors, warnings;
	FileSystem::Identifier reload_id(FileSystemNative::instance(), tmp_path);
	Canvas::Handle reloaded = open_canvas_as(reload_id, tmp_path, errors, warnings);
	ASSERT_NE(reloaded, nullptr);

	EXPECT_EQ(reloaded->size(), orig_layer_count)
		<< "Layer count should be preserved after round-trip";

	std::remove(tmp_path.c_str());
}

// --- File extension mapping tests ---

TEST_F(ExportFormatTest, ExtBookMapsExtensionsToTargetNames) {
	const Target::ExtBook& ebook = Target::ext_book();
	const Target::Book& book = Target::book();

	for (auto it = ebook.begin(); it != ebook.end(); ++it) {
		// Each extension should map to a valid target name in the book
		EXPECT_NE(book.find(it->second), book.end())
			<< "Extension '" << it->first << "' maps to target '" << it->second
			<< "' which should exist in the target book";
	}
}
