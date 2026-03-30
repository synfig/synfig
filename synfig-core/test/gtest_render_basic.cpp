/* === S Y N F I G ========================================================= */
/*!	\file gtest_render_basic.cpp
**	\brief Tests for basic shape rendering correctness
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
#include <synfig/layer.h>
#include <synfig/surface.h>

#include "test_utils.h"

using namespace synfig;

class RenderBasicTest : public ::testing::Test {
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

std::unique_ptr<Main> RenderBasicTest::main_instance;

// --- Canvas layer enumeration tests ---

TEST_F(RenderBasicTest, GradientCanvasHasLayers) {
	Canvas::Handle canvas = load_fixture("gradient.sif");
	ASSERT_NE(canvas, nullptr);
	EXPECT_GT(canvas->size(), 0u) << "gradient.sif should contain at least one layer";
}

TEST_F(RenderBasicTest, BasicShapesCanvasHasLayers) {
	Canvas::Handle canvas = load_fixture("basic_shapes.sif");
	ASSERT_NE(canvas, nullptr);
	EXPECT_GT(canvas->size(), 0u) << "basic_shapes.sif should contain at least one layer";
}

TEST_F(RenderBasicTest, AnimationKeyframesCanvasHasLayers) {
	Canvas::Handle canvas = load_fixture("animation_keyframes.sif");
	ASSERT_NE(canvas, nullptr);
	EXPECT_GT(canvas->size(), 0u) << "animation_keyframes.sif should contain at least one layer";
}

// --- Layer type identification tests ---

TEST_F(RenderBasicTest, LayersHaveNonEmptyNames) {
	Canvas::Handle canvas = load_fixture("basic_shapes.sif");
	ASSERT_NE(canvas, nullptr);

	for (auto it = canvas->begin(); it != canvas->end(); ++it) {
		Layer::Handle layer = *it;
		EXPECT_FALSE(layer->get_name().empty())
			<< "Each layer should have a non-empty type name";
	}
}

TEST_F(RenderBasicTest, LayersHaveNonEmptyLocalNames) {
	Canvas::Handle canvas = load_fixture("basic_shapes.sif");
	ASSERT_NE(canvas, nullptr);

	for (auto it = canvas->begin(); it != canvas->end(); ++it) {
		Layer::Handle layer = *it;
		EXPECT_FALSE(layer->get_local_name().empty())
			<< "Each layer should have a non-empty localized name";
	}
}

TEST_F(RenderBasicTest, GradientContainsGradientLayer) {
	Canvas::Handle canvas = load_fixture("gradient.sif");
	ASSERT_NE(canvas, nullptr);

	bool found_gradient = false;
	for (auto it = canvas->begin(); it != canvas->end(); ++it) {
		if ((*it)->get_name().find("gradient") != String::npos) {
			found_gradient = true;
			break;
		}
	}
	EXPECT_TRUE(found_gradient) << "gradient.sif should contain a gradient-type layer";
}

// --- Canvas dimension and FPS property tests ---

TEST_F(RenderBasicTest, CanvasDimensionsArePositive) {
	Canvas::Handle canvas = load_fixture("gradient.sif");
	ASSERT_NE(canvas, nullptr);
	EXPECT_GT(canvas->get_w(), 0) << "Canvas width should be positive";
	EXPECT_GT(canvas->get_h(), 0) << "Canvas height should be positive";
}

TEST_F(RenderBasicTest, CanvasDimensionsAreReasonable) {
	Canvas::Handle canvas = load_fixture("basic_shapes.sif");
	ASSERT_NE(canvas, nullptr);
	EXPECT_LE(canvas->get_w(), 10000) << "Canvas width should be reasonable";
	EXPECT_LE(canvas->get_h(), 10000) << "Canvas height should be reasonable";
}

TEST_F(RenderBasicTest, CanvasHasValidFrameRate) {
	Canvas::Handle canvas = load_fixture("gradient.sif");
	ASSERT_NE(canvas, nullptr);

	float fps = canvas->rend_desc().get_frame_rate();
	EXPECT_GT(fps, 0.0f) << "Frame rate should be positive";
	EXPECT_LE(fps, 120.0f) << "Frame rate should be reasonable (<=120)";
}

TEST_F(RenderBasicTest, AnimationCanvasHasValidFrameRate) {
	Canvas::Handle canvas = load_fixture("animation_keyframes.sif");
	ASSERT_NE(canvas, nullptr);

	float fps = canvas->rend_desc().get_frame_rate();
	EXPECT_GT(fps, 0.0f) << "Animation canvas should have a positive frame rate";
}

// --- Surface construction tests ---

TEST_F(RenderBasicTest, SurfaceCanBeCreatedWithDimensions) {
	Surface surface(64, 48);
	EXPECT_EQ(surface.get_w(), 64);
	EXPECT_EQ(surface.get_h(), 48);
}

TEST_F(RenderBasicTest, SurfaceFillProducesUniformColor) {
	Surface surface(8, 8);
	Color red(1.0f, 0.0f, 0.0f, 1.0f);
	surface.fill(red);

	for (int y = 0; y < surface.get_h(); ++y) {
		for (int x = 0; x < surface.get_w(); ++x) {
			EXPECT_NEAR(surface[y][x].get_r(), 1.0f, 1e-5f);
			EXPECT_NEAR(surface[y][x].get_g(), 0.0f, 1e-5f);
			EXPECT_NEAR(surface[y][x].get_b(), 0.0f, 1e-5f);
			EXPECT_NEAR(surface[y][x].get_a(), 1.0f, 1e-5f);
		}
	}
}

TEST_F(RenderBasicTest, IdenticalSurfacesHaveInfinitePSNR) {
	Surface a(16, 16);
	Surface b(16, 16);
	Color c(0.5f, 0.5f, 0.5f, 1.0f);
	a.fill(c);
	b.fill(c);

	double psnr = test::compute_psnr(a, b);
	EXPECT_TRUE(std::isinf(psnr)) << "Identical surfaces should have infinite PSNR";
}

TEST_F(RenderBasicTest, DifferentSurfacesHaveFinitePSNR) {
	Surface a(16, 16);
	Surface b(16, 16);
	a.fill(Color(1.0f, 0.0f, 0.0f, 1.0f));
	b.fill(Color(0.0f, 1.0f, 0.0f, 1.0f));

	double psnr = test::compute_psnr(a, b);
	EXPECT_FALSE(std::isinf(psnr)) << "Different surfaces should have finite PSNR";
	EXPECT_GT(psnr, 0.0) << "PSNR should be positive for valid surfaces";
}

TEST_F(RenderBasicTest, MismatchedSurfaceSizesReturnZeroPSNR) {
	Surface a(16, 16);
	Surface b(32, 32);
	a.fill(Color(1.0f, 1.0f, 1.0f, 1.0f));
	b.fill(Color(1.0f, 1.0f, 1.0f, 1.0f));

	double psnr = test::compute_psnr(a, b);
	EXPECT_DOUBLE_EQ(psnr, 0.0) << "Mismatched sizes should return 0 PSNR";
}
