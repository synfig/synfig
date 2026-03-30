/* === S Y N F I G ========================================================= */
/*!	\file gtest_file_io_sif.cpp
**	\brief Tests for .sif XML file loading
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

#include "test_utils.h"

using namespace synfig;

class SifFileIOTest : public ::testing::Test {
protected:
	static std::unique_ptr<Main> main_instance;

	static void SetUpTestSuite() {
		if (!main_instance)
			main_instance = std::make_unique<Main>(".");
	}
};

std::unique_ptr<Main> SifFileIOTest::main_instance;

TEST_F(SifFileIOTest, LoadGradientSif) {
	String errors, warnings;
	std::string path = test::fixture_path("gradient.sif");
	FileSystem::Identifier id(FileSystemNative::instance(), path);
	Canvas::Handle canvas = open_canvas_as(id, path, errors, warnings);
	ASSERT_NE(canvas, nullptr) << "Failed to load gradient.sif: " << errors;
	EXPECT_GT(canvas->get_w(), 0);
	EXPECT_GT(canvas->get_h(), 0);
}

TEST_F(SifFileIOTest, LoadBasicShapesSif) {
	String errors, warnings;
	std::string path = test::fixture_path("basic_shapes.sif");
	FileSystem::Identifier id(FileSystemNative::instance(), path);
	Canvas::Handle canvas = open_canvas_as(id, path, errors, warnings);
	ASSERT_NE(canvas, nullptr) << "Failed to load basic_shapes.sif: " << errors;
	EXPECT_GT(canvas->get_w(), 0);
	EXPECT_GT(canvas->get_h(), 0);
}

TEST_F(SifFileIOTest, LoadAnimationKeyframesSif) {
	String errors, warnings;
	std::string path = test::fixture_path("animation_keyframes.sif");
	FileSystem::Identifier id(FileSystemNative::instance(), path);
	Canvas::Handle canvas = open_canvas_as(id, path, errors, warnings);
	ASSERT_NE(canvas, nullptr) << "Failed to load animation_keyframes.sif: " << errors;
	EXPECT_GT(canvas->get_w(), 0);
	EXPECT_GT(canvas->get_h(), 0);
}

TEST_F(SifFileIOTest, LoadedCanvasHasLayers) {
	String errors, warnings;
	std::string path = test::fixture_path("gradient.sif");
	FileSystem::Identifier id(FileSystemNative::instance(), path);
	Canvas::Handle canvas = open_canvas_as(id, path, errors, warnings);
	ASSERT_NE(canvas, nullptr);
	EXPECT_GT(canvas->size(), 0u) << "Canvas should have at least one layer";
}

TEST_F(SifFileIOTest, NonExistentFileReturnsNull) {
	String errors, warnings;
	std::string path = "fixtures/nonexistent.sif";
	FileSystem::Identifier id(FileSystemNative::instance(), path);
	Canvas::Handle canvas = open_canvas_as(id, path, errors, warnings);
	EXPECT_EQ(canvas, nullptr);
}

TEST_F(SifFileIOTest, LoadedCanvasHasValidDimensions) {
	String errors, warnings;
	std::string path = test::fixture_path("gradient.sif");
	FileSystem::Identifier id(FileSystemNative::instance(), path);
	Canvas::Handle canvas = open_canvas_as(id, path, errors, warnings);
	ASSERT_NE(canvas, nullptr);
	// Typical canvas dimensions should be reasonable
	EXPECT_LE(canvas->get_w(), 10000);
	EXPECT_LE(canvas->get_h(), 10000);
}
