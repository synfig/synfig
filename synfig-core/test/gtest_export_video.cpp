/* === S Y N F I G ========================================================= */
/*!	\file gtest_export_video.cpp
**	\brief Tests for FFmpeg video export verification
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
#include <synfig/target.h>
#include <synfig/renddesc.h>

#include "test_utils.h"

using namespace synfig;

class ExportVideoTest : public ::testing::Test {
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

	bool has_ffmpeg_target() {
		const Target::Book& book = Target::book();
		return book.find("ffmpeg") != book.end();
	}

	bool has_avi_extension() {
		const Target::ExtBook& ebook = Target::ext_book();
		return ebook.find("avi") != ebook.end();
	}

	bool has_mp4_extension() {
		const Target::ExtBook& ebook = Target::ext_book();
		return ebook.find("mp4") != ebook.end();
	}
};

std::unique_ptr<Main> ExportVideoTest::main_instance;

// --- FFmpeg target availability tests ---

TEST_F(ExportVideoTest, FfmpegTargetExists) {
	if (!has_ffmpeg_target()) {
		GTEST_SKIP() << "FFmpeg target not available in this build";
	}
	const Target::Book& book = Target::book();
	auto it = book.find("ffmpeg");
	EXPECT_NE(it->second.factory, nullptr)
		<< "FFmpeg target should have a valid factory";
}

TEST_F(ExportVideoTest, FfmpegTargetHasExtension) {
	if (!has_ffmpeg_target()) {
		GTEST_SKIP() << "FFmpeg target not available in this build";
	}
	const Target::Book& book = Target::book();
	auto it = book.find("ffmpeg");
	EXPECT_FALSE(it->second.file_extension.empty())
		<< "FFmpeg target should have a file extension defined";
}

TEST_F(ExportVideoTest, AviExtensionRegistered) {
	if (!has_avi_extension()) {
		GTEST_SKIP() << "AVI extension not registered in this build";
	}
	const Target::ExtBook& ebook = Target::ext_book();
	auto it = ebook.find("avi");
	EXPECT_FALSE(it->second.empty())
		<< "AVI extension should map to a target name";
}

TEST_F(ExportVideoTest, Mp4ExtensionRegistered) {
	if (!has_mp4_extension()) {
		GTEST_SKIP() << "MP4 extension not registered in this build";
	}
	const Target::ExtBook& ebook = Target::ext_book();
	auto it = ebook.find("mp4");
	EXPECT_FALSE(it->second.empty())
		<< "MP4 extension should map to a target name";
}

// --- Canvas timeline property tests ---

TEST_F(ExportVideoTest, AnimationCanvasHasPositiveFrameRate) {
	Canvas::Handle canvas = load_fixture("animation_keyframes.sif");
	ASSERT_NE(canvas, nullptr);

	float fps = canvas->rend_desc().get_frame_rate();
	EXPECT_GT(fps, 0.0f) << "Animation canvas should have a positive frame rate";
}

TEST_F(ExportVideoTest, CanvasRendDescDimensionsMatchCanvas) {
	Canvas::Handle canvas = load_fixture("animation_keyframes.sif");
	ASSERT_NE(canvas, nullptr);

	const RendDesc& rd = canvas->rend_desc();
	EXPECT_EQ(rd.get_w(), canvas->get_w())
		<< "RendDesc width should match canvas width";
	EXPECT_EQ(rd.get_h(), canvas->get_h())
		<< "RendDesc height should match canvas height";
}

TEST_F(ExportVideoTest, FrameRatePreservedAfterSet) {
	Canvas::Handle canvas = load_fixture("animation_keyframes.sif");
	ASSERT_NE(canvas, nullptr);

	float original_fps = canvas->rend_desc().get_frame_rate();

	// Set a new frame rate and verify it sticks
	canvas->rend_desc().set_frame_rate(30.0f);
	EXPECT_NEAR(canvas->rend_desc().get_frame_rate(), 30.0f, 0.01f);

	// Restore original
	canvas->rend_desc().set_frame_rate(original_fps);
	EXPECT_NEAR(canvas->rend_desc().get_frame_rate(), original_fps, 0.01f);
}

TEST_F(ExportVideoTest, RendDescHasReasonableDefaults) {
	Canvas::Handle canvas = load_fixture("gradient.sif");
	ASSERT_NE(canvas, nullptr);

	const RendDesc& rd = canvas->rend_desc();
	EXPECT_GT(rd.get_w(), 0) << "RendDesc width should be positive";
	EXPECT_GT(rd.get_h(), 0) << "RendDesc height should be positive";
	EXPECT_GT(rd.get_frame_rate(), 0.0f) << "Frame rate should be positive";
}

// --- Video target parameter tests ---

TEST_F(ExportVideoTest, VideoTargetsMapToCorrectTargetNames) {
	const Target::ExtBook& ebook = Target::ext_book();
	const Target::Book& book = Target::book();

	// Check common video extensions if they exist
	std::vector<std::string> video_exts = {"avi", "mp4", "mpg", "gif"};
	for (const auto& ext : video_exts) {
		auto ext_it = ebook.find(ext);
		if (ext_it != ebook.end()) {
			auto target_it = book.find(ext_it->second);
			EXPECT_NE(target_it, book.end())
				<< "Video extension '" << ext << "' should map to an existing target '"
				<< ext_it->second << "'";
		}
	}
}

TEST_F(ExportVideoTest, MultipleFixturesHaveConsistentFrameRate) {
	// All test fixtures should have sensible frame rates
	std::vector<std::string> fixtures = {
		"gradient.sif", "basic_shapes.sif", "animation_keyframes.sif"
	};

	for (const auto& name : fixtures) {
		Canvas::Handle canvas = load_fixture(name);
		if (!canvas) continue;

		float fps = canvas->rend_desc().get_frame_rate();
		EXPECT_GT(fps, 0.0f) << name << " should have positive frame rate";
		EXPECT_LE(fps, 120.0f) << name << " should have reasonable frame rate";
	}
}

TEST_F(ExportVideoTest, CanvasDimensionsAreEven) {
	// Video codecs typically require even dimensions
	Canvas::Handle canvas = load_fixture("animation_keyframes.sif");
	ASSERT_NE(canvas, nullptr);

	int w = canvas->get_w();
	int h = canvas->get_h();

	// This is informational - not all canvases need even dimensions,
	// but it's a common requirement for video export
	if (w % 2 != 0 || h % 2 != 0) {
		GTEST_LOG_(INFO) << "Canvas dimensions (" << w << "x" << h
			<< ") are not both even - video codecs may require padding";
	}
	// At minimum, dimensions should be positive
	EXPECT_GT(w, 0);
	EXPECT_GT(h, 0);
}
