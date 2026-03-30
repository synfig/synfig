/* === S Y N F I G ========================================================= */
/*!	\file gtest_file_io_sifz.cpp
**	\brief Tests for .sifz round-trip consistency
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

#include <cstdio>

#include "test_utils.h"

using namespace synfig;

class SifzRoundTripTest : public ::testing::Test {
protected:
	static std::unique_ptr<Main> main_instance;

	static void SetUpTestSuite() {
		if (!main_instance)
			main_instance = std::make_unique<Main>(".");
	}

	/// Helper to create a temporary file path for .sifz output
	static std::string temp_sifz_path(const std::string& name) {
		return "fixtures/tmp_" + name + ".sifz";
	}

	void TearDown() override {
		// Clean up any temp files created during tests
		std::remove(temp_sifz_path("gradient").c_str());
		std::remove(temp_sifz_path("basic_shapes").c_str());
		std::remove(temp_sifz_path("animation_keyframes").c_str());
	}
};

std::unique_ptr<Main> SifzRoundTripTest::main_instance;

TEST_F(SifzRoundTripTest, SaveGradientAsSifz) {
	String errors, warnings;
	std::string src_path = test::fixture_path("gradient.sif");
	FileSystem::Identifier src_id(FileSystemNative::instance(), src_path);
	Canvas::Handle canvas = open_canvas_as(src_id, src_path, errors, warnings);
	ASSERT_NE(canvas, nullptr) << "Failed to load source: " << errors;

	std::string dst_path = temp_sifz_path("gradient");
	FileSystem::Identifier dst_id(FileSystemNative::instance(), dst_path);
	bool saved = save_canvas(dst_id, canvas);
	EXPECT_TRUE(saved) << "Failed to save canvas as .sifz";
}

TEST_F(SifzRoundTripTest, RoundTripGradientPreservesDimensions) {
	String errors, warnings;
	std::string src_path = test::fixture_path("gradient.sif");
	FileSystem::Identifier src_id(FileSystemNative::instance(), src_path);
	Canvas::Handle original = open_canvas_as(src_id, src_path, errors, warnings);
	ASSERT_NE(original, nullptr);

	// Save as .sifz
	std::string dst_path = temp_sifz_path("gradient");
	FileSystem::Identifier dst_id(FileSystemNative::instance(), dst_path);
	ASSERT_TRUE(save_canvas(dst_id, original));

	// Reload the .sifz
	String errors2, warnings2;
	FileSystem::Identifier reload_id(FileSystemNative::instance(), dst_path);
	Canvas::Handle reloaded = open_canvas_as(reload_id, dst_path, errors2, warnings2);
	ASSERT_NE(reloaded, nullptr) << "Failed to reload .sifz: " << errors2;

	EXPECT_EQ(original->get_w(), reloaded->get_w());
	EXPECT_EQ(original->get_h(), reloaded->get_h());
}

TEST_F(SifzRoundTripTest, RoundTripGradientPreservesLayerCount) {
	String errors, warnings;
	std::string src_path = test::fixture_path("gradient.sif");
	FileSystem::Identifier src_id(FileSystemNative::instance(), src_path);
	Canvas::Handle original = open_canvas_as(src_id, src_path, errors, warnings);
	ASSERT_NE(original, nullptr);

	std::string dst_path = temp_sifz_path("gradient");
	FileSystem::Identifier dst_id(FileSystemNative::instance(), dst_path);
	ASSERT_TRUE(save_canvas(dst_id, original));

	String errors2, warnings2;
	FileSystem::Identifier reload_id(FileSystemNative::instance(), dst_path);
	Canvas::Handle reloaded = open_canvas_as(reload_id, dst_path, errors2, warnings2);
	ASSERT_NE(reloaded, nullptr);

	EXPECT_EQ(original->size(), reloaded->size());
}

TEST_F(SifzRoundTripTest, RoundTripBasicShapes) {
	String errors, warnings;
	std::string src_path = test::fixture_path("basic_shapes.sif");
	FileSystem::Identifier src_id(FileSystemNative::instance(), src_path);
	Canvas::Handle original = open_canvas_as(src_id, src_path, errors, warnings);
	ASSERT_NE(original, nullptr);

	std::string dst_path = temp_sifz_path("basic_shapes");
	FileSystem::Identifier dst_id(FileSystemNative::instance(), dst_path);
	ASSERT_TRUE(save_canvas(dst_id, original));

	String errors2, warnings2;
	FileSystem::Identifier reload_id(FileSystemNative::instance(), dst_path);
	Canvas::Handle reloaded = open_canvas_as(reload_id, dst_path, errors2, warnings2);
	ASSERT_NE(reloaded, nullptr);

	EXPECT_EQ(original->get_w(), reloaded->get_w());
	EXPECT_EQ(original->get_h(), reloaded->get_h());
	EXPECT_EQ(original->size(), reloaded->size());
}

TEST_F(SifzRoundTripTest, RoundTripAnimationKeyframes) {
	String errors, warnings;
	std::string src_path = test::fixture_path("animation_keyframes.sif");
	FileSystem::Identifier src_id(FileSystemNative::instance(), src_path);
	Canvas::Handle original = open_canvas_as(src_id, src_path, errors, warnings);
	ASSERT_NE(original, nullptr);

	std::string dst_path = temp_sifz_path("animation_keyframes");
	FileSystem::Identifier dst_id(FileSystemNative::instance(), dst_path);
	ASSERT_TRUE(save_canvas(dst_id, original));

	String errors2, warnings2;
	FileSystem::Identifier reload_id(FileSystemNative::instance(), dst_path);
	Canvas::Handle reloaded = open_canvas_as(reload_id, dst_path, errors2, warnings2);
	ASSERT_NE(reloaded, nullptr);

	EXPECT_EQ(original->get_w(), reloaded->get_w());
	EXPECT_EQ(original->get_h(), reloaded->get_h());
	EXPECT_EQ(original->size(), reloaded->size());
}
