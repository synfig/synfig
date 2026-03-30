/* === S Y N F I G ========================================================= */
/*!	\file gtest_render_blend.cpp
**	\brief Tests for layer blend mode rendering verification
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
#include <synfig/color/color.h>
#include <synfig/surface.h>

#include "test_utils.h"

using namespace synfig;

class RenderBlendTest : public ::testing::Test {
protected:
	static std::unique_ptr<Main> main_instance;

	static void SetUpTestSuite() {
		if (!main_instance)
			main_instance = std::make_unique<Main>(".");
	}

	// Helper: compare color channels with tolerance
	static void expect_color_near(const Color& actual, float r, float g, float b, float a, float tol = 1e-3f) {
		EXPECT_NEAR(actual.get_r(), r, tol) << "Red channel mismatch";
		EXPECT_NEAR(actual.get_g(), g, tol) << "Green channel mismatch";
		EXPECT_NEAR(actual.get_b(), b, tol) << "Blue channel mismatch";
		EXPECT_NEAR(actual.get_a(), a, tol) << "Alpha channel mismatch";
	}
};

std::unique_ptr<Main> RenderBlendTest::main_instance;

// --- BLEND_COMPOSITE tests ---

TEST_F(RenderBlendTest, CompositeOpaqueOverOpaque) {
	// Opaque red composited onto opaque blue at full amount -> red
	Color a(1.0f, 0.0f, 0.0f, 1.0f); // foreground: red
	Color b(0.0f, 0.0f, 1.0f, 1.0f); // background: blue
	Color result = Color::blend(a, b, 1.0f, Color::BLEND_COMPOSITE);
	expect_color_near(result, 1.0f, 0.0f, 0.0f, 1.0f);
}

TEST_F(RenderBlendTest, CompositeTransparentOverOpaque) {
	// Fully transparent foreground composited onto blue -> blue
	Color a(1.0f, 0.0f, 0.0f, 0.0f); // foreground: transparent red
	Color b(0.0f, 0.0f, 1.0f, 1.0f); // background: blue
	Color result = Color::blend(a, b, 1.0f, Color::BLEND_COMPOSITE);
	expect_color_near(result, 0.0f, 0.0f, 1.0f, 1.0f);
}

TEST_F(RenderBlendTest, CompositeHalfAmount) {
	// Opaque red composited at 50% amount onto opaque blue
	Color a(1.0f, 0.0f, 0.0f, 1.0f);
	Color b(0.0f, 0.0f, 1.0f, 1.0f);
	Color result = Color::blend(a, b, 0.5f, Color::BLEND_COMPOSITE);
	// At 50% amount with opaque colors, expect a mix
	EXPECT_GT(result.get_r(), 0.0f);
	EXPECT_GT(result.get_b(), 0.0f);
	EXPECT_NEAR(result.get_a(), 1.0f, 1e-3f);
}

TEST_F(RenderBlendTest, CompositeZeroAmount) {
	// Zero amount should leave background unchanged
	Color a(1.0f, 0.0f, 0.0f, 1.0f);
	Color b(0.0f, 0.0f, 1.0f, 1.0f);
	Color result = Color::blend(a, b, 0.0f, Color::BLEND_COMPOSITE);
	expect_color_near(result, 0.0f, 0.0f, 1.0f, 1.0f);
}

// --- BLEND_STRAIGHT tests ---

TEST_F(RenderBlendTest, StraightFullAmount) {
	// Straight blend at full amount: A replaces B
	Color a(1.0f, 0.0f, 0.0f, 1.0f);
	Color b(0.0f, 0.0f, 1.0f, 1.0f);
	Color result = Color::blend(a, b, 1.0f, Color::BLEND_STRAIGHT);
	expect_color_near(result, 1.0f, 0.0f, 0.0f, 1.0f);
}

TEST_F(RenderBlendTest, StraightZeroAmount) {
	// Straight blend at zero amount: B unchanged
	Color a(1.0f, 0.0f, 0.0f, 1.0f);
	Color b(0.0f, 0.0f, 1.0f, 1.0f);
	Color result = Color::blend(a, b, 0.0f, Color::BLEND_STRAIGHT);
	expect_color_near(result, 0.0f, 0.0f, 1.0f, 1.0f);
}

TEST_F(RenderBlendTest, StraightHalfAmountInterpolates) {
	// Straight blend at 50%: linear interpolation between A and B
	Color a(1.0f, 0.0f, 0.0f, 1.0f);
	Color b(0.0f, 0.0f, 1.0f, 1.0f);
	Color result = Color::blend(a, b, 0.5f, Color::BLEND_STRAIGHT);
	EXPECT_NEAR(result.get_r(), 0.5f, 0.05f);
	EXPECT_NEAR(result.get_b(), 0.5f, 0.05f);
}

// --- BLEND_ADD tests ---

TEST_F(RenderBlendTest, AddBlendSumsColors) {
	Color a(0.3f, 0.2f, 0.1f, 1.0f);
	Color b(0.2f, 0.3f, 0.4f, 1.0f);
	Color result = Color::blend(a, b, 1.0f, Color::BLEND_ADD);
	// Add blend: B + A * amount
	EXPECT_NEAR(result.get_r(), 0.5f, 0.05f);
	EXPECT_NEAR(result.get_g(), 0.5f, 0.05f);
	EXPECT_NEAR(result.get_b(), 0.5f, 0.05f);
}

TEST_F(RenderBlendTest, AddBlendZeroAmountUnchanged) {
	Color a(0.5f, 0.5f, 0.5f, 1.0f);
	Color b(0.2f, 0.3f, 0.4f, 1.0f);
	Color result = Color::blend(a, b, 0.0f, Color::BLEND_ADD);
	expect_color_near(result, 0.2f, 0.3f, 0.4f, 1.0f);
}

// --- BLEND_MULTIPLY tests ---

TEST_F(RenderBlendTest, MultiplyBlendMultipliesColors) {
	Color a(0.5f, 0.5f, 0.5f, 1.0f);
	Color b(1.0f, 1.0f, 1.0f, 1.0f);
	Color result = Color::blend(a, b, 1.0f, Color::BLEND_MULTIPLY);
	// Multiply: result channels should be around A*B = 0.5
	EXPECT_NEAR(result.get_r(), 0.5f, 0.1f);
	EXPECT_NEAR(result.get_g(), 0.5f, 0.1f);
	EXPECT_NEAR(result.get_b(), 0.5f, 0.1f);
}

TEST_F(RenderBlendTest, MultiplyByZeroProducesBlack) {
	Color a(0.0f, 0.0f, 0.0f, 1.0f);
	Color b(1.0f, 1.0f, 1.0f, 1.0f);
	Color result = Color::blend(a, b, 1.0f, Color::BLEND_MULTIPLY);
	EXPECT_NEAR(result.get_r(), 0.0f, 0.05f);
	EXPECT_NEAR(result.get_g(), 0.0f, 0.05f);
	EXPECT_NEAR(result.get_b(), 0.0f, 0.05f);
}

TEST_F(RenderBlendTest, MultiplyByOnePreservesColor) {
	Color a(1.0f, 1.0f, 1.0f, 1.0f);
	Color b(0.7f, 0.3f, 0.5f, 1.0f);
	Color result = Color::blend(a, b, 1.0f, Color::BLEND_MULTIPLY);
	EXPECT_NEAR(result.get_r(), 0.7f, 0.1f);
	EXPECT_NEAR(result.get_g(), 0.3f, 0.1f);
	EXPECT_NEAR(result.get_b(), 0.5f, 0.1f);
}

// --- BLEND_SUBTRACT tests ---

TEST_F(RenderBlendTest, SubtractBlendSubtractsColors) {
	Color a(0.3f, 0.2f, 0.1f, 1.0f);
	Color b(0.5f, 0.5f, 0.5f, 1.0f);
	Color result = Color::blend(a, b, 1.0f, Color::BLEND_SUBTRACT);
	// Subtract: B - A * amount
	EXPECT_NEAR(result.get_r(), 0.2f, 0.05f);
	EXPECT_NEAR(result.get_g(), 0.3f, 0.05f);
	EXPECT_NEAR(result.get_b(), 0.4f, 0.05f);
}

// --- BLEND_ONTO tests ---

TEST_F(RenderBlendTest, OntoBlendPreservesBackgroundAlpha) {
	Color a(1.0f, 0.0f, 0.0f, 1.0f);
	Color b(0.0f, 0.0f, 1.0f, 0.5f);
	Color result = Color::blend(a, b, 1.0f, Color::BLEND_ONTO);
	// BLEND_ONTO should preserve B's alpha
	EXPECT_NEAR(result.get_a(), 0.5f, 0.05f);
}

// --- BLEND_BEHIND tests ---

TEST_F(RenderBlendTest, BehindBlendCompositesBOntoA) {
	// BLEND_BEHIND: B is composited onto A (reversed order)
	Color a(1.0f, 0.0f, 0.0f, 0.5f); // semi-transparent red
	Color b(0.0f, 0.0f, 1.0f, 1.0f); // opaque blue
	Color result = Color::blend(a, b, 1.0f, Color::BLEND_BEHIND);
	// Since B is opaque and behind, result should mix
	EXPECT_GE(result.get_a(), 0.5f);
}

// --- Surface-level blend verification ---

TEST_F(RenderBlendTest, BlendedSurfacesDiffer) {
	Surface base(16, 16);
	base.fill(Color(0.0f, 0.0f, 1.0f, 1.0f));

	Surface blended(16, 16);
	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 16; ++x) {
			Color fg(1.0f, 0.0f, 0.0f, 1.0f);
			blended[y][x] = Color::blend(fg, base[y][x], 0.5f, Color::BLEND_COMPOSITE);
		}
	}

	EXPECT_FALSE(test::surfaces_match(base, blended, 60.0))
		<< "Blended surface should differ from the original";
}

TEST_F(RenderBlendTest, BlendMethodEnumRange) {
	// Verify that the blend method enum values are in expected range
	EXPECT_EQ(static_cast<int>(Color::BLEND_COMPOSITE), 0);
	EXPECT_EQ(static_cast<int>(Color::BLEND_STRAIGHT), 1);
	EXPECT_EQ(static_cast<int>(Color::BLEND_ADD), 4);
	EXPECT_EQ(static_cast<int>(Color::BLEND_SUBTRACT), 5);
	EXPECT_EQ(static_cast<int>(Color::BLEND_MULTIPLY), 6);
	EXPECT_LT(static_cast<int>(Color::BLEND_END), 100)
		<< "BLEND_END should be a reasonable sentinel value";
}
